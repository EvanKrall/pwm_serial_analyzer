#include "PWMSerialAnalyzer.h"
#include "PWMSerialAnalyzerSettings.h"
#include <AnalyzerChannelData.h>


PWMSerialAnalyzer::PWMSerialAnalyzer() : Analyzer2(), mSettings( new PWMSerialAnalyzerSettings() ), mSimulationInitilized( false )
{
    SetAnalyzerSettings( mSettings.get() );
    UseFrameV2();
}

PWMSerialAnalyzer::~PWMSerialAnalyzer()
{
    KillThread();
}


void PWMSerialAnalyzer::SetupResults()
{
    // Unlike the worker thread, this function is called from the GUI thread
    // we need to reset the Results object here because it is exposed for direct access by the GUI, and it can't be deleted from the
    // WorkerThread

    mResults.reset( new PWMSerialAnalyzerResults( this, mSettings.get() ) );
    SetAnalyzerResults( mResults.get() );
    mResults->AddChannelBubblesWillAppearOn( mSettings->mInputChannel );
}

void PWMSerialAnalyzer::WorkerThread()
{
    mSampleRateHz = GetSampleRate();

    U32 minimum_pulse_width_nanoseconds = mSettings->mMinimumPulseWidthNanoseconds;
    U32 threshold_pulse_width_nanoseconds = mSettings->mThresholdPulseWidthNanoseconds;
    U32 maximum_pulse_width_nanoseconds = mSettings->mMaximumPulseWidthNanoseconds;


    if( mSettings->mHighIsIdle == true )
    {
        mIdle = BIT_HIGH;
        mActive = BIT_LOW;
    }
    else
    {
        mIdle = BIT_LOW;
        mActive = BIT_HIGH;
    }

    if( mSettings->mOneIsShort == true )
    {
        mShort = BIT_HIGH;
        mLong = BIT_LOW;
    }
    else
    {
        mShort = BIT_LOW;
        mLong = BIT_HIGH;
    }


    U32 bits_per_transfer = mSettings->mBitsPerTransfer;

    mPWMSerial = GetAnalyzerChannelData( mSettings->mInputChannel );

    if( mPWMSerial->GetBitState() == mActive )
        mPWMSerial->AdvanceToNextEdge();

    for( ;; )
    {

        bool pulse_too_short = false;
        bool pulse_too_long = false;
        bool timeout = false;

        U64 data = 0;
        DataBuilder data_builder;
        data_builder.Reset( &data, mSettings->mShiftOrder, bits_per_transfer );

        U64 frame_starting_sample = 0;

        for (int i=0; i<bits_per_transfer; i++) {
            // if not on the first bit, check whether the delay until the start of this bit is longer than the maximum inter-bit period
            if ( i != 0 ) {
                U64 delay_until_next_edge_samples = mPWMSerial->GetSampleOfNextEdge() - mPWMSerial->GetSampleNumber();
                U64 delay_until_next_edge_ns = delay_until_next_edge_samples * 1e9 / mSampleRateHz;

                if (delay_until_next_edge_ns > mSettings->mMaximumInterBitPeriodNanoseconds) {
                    timeout = true;
                    break;
                }
            }

            // we're starting idle. Find the next transition to active.
            mPWMSerial->AdvanceToNextEdge();

            // we're now at the beginning of the start bit.  We can start collecting the data.
            U64 pulse_starting_sample = mPWMSerial->GetSampleNumber();
            if (i==0) {
                frame_starting_sample = pulse_starting_sample;
            }

            BitState value;

            mPWMSerial->AdvanceToNextEdge();
            U64 pulse_end_sample = mPWMSerial->GetSampleNumber();

            U64 pulse_width_samples = pulse_end_sample - pulse_starting_sample;

            U64 pulse_width_nanoseconds = pulse_width_samples * 1e9 / mSampleRateHz;

            if ( pulse_width_nanoseconds < minimum_pulse_width_nanoseconds ) {
                pulse_too_short = true;
            } else if ( pulse_width_nanoseconds > maximum_pulse_width_nanoseconds ) {
                pulse_too_long = true;
            }

            if ( pulse_width_nanoseconds < threshold_pulse_width_nanoseconds ) {
                value = mShort;
            } else {
                value = mLong;
            }

            data_builder.AddBit( value );
            if (pulse_too_long || pulse_too_short || timeout) {
                break;
            }
        }

        // ok now record the value!
        // note that we're not using the mData2 or mType fields for anything, so we won't bother to set them.
        Frame frame;
        frame.mStartingSampleInclusive = frame_starting_sample;
        frame.mEndingSampleInclusive = mPWMSerial->GetSampleNumber();
        frame.mData1 = data;
        frame.mFlags = 0;

        if( timeout == true )
            frame.mFlags |= TIMEOUT_ERROR_FLAG | DISPLAY_AS_ERROR_FLAG;

        if( pulse_too_short == true )
            frame.mFlags |= PULSE_TOO_SHORT_ERROR_FLAG | DISPLAY_AS_ERROR_FLAG;

        if( pulse_too_long == true )
            frame.mFlags |= PULSE_TOO_LONG_ERROR_FLAG | DISPLAY_AS_ERROR_FLAG;

        mResults->AddFrame( frame );

        FrameV2 framev2;

        const U32 bytes_per_transfer = ( mSettings->mBitsPerTransfer + 7 ) / 8;
        U8 bytes[ 8 ];
        for( int i = 0; i < bytes_per_transfer; ++i )
        {
            auto bit_offset = ( bytes_per_transfer - i - 1 ) * 8;
            bytes[ i ] = data >> bit_offset;
        }
        framev2.AddByteArray( "data", bytes, bytes_per_transfer );

        if( timeout )
        {
            framev2.AddString( "error", "inter-bit timeout" );
        }
        else if( pulse_too_short )
        {
            framev2.AddString( "error", "pulse too short" );
        }
        else if( pulse_too_long )
        {
            framev2.AddString( "error", "pulse too long" );
        }

        mResults->AddFrameV2( framev2, "data", frame_starting_sample, mPWMSerial->GetSampleNumber() );

        mResults->CommitResults();

        ReportProgress( frame.mEndingSampleInclusive );
        CheckIfThreadShouldExit();
    }
}

bool PWMSerialAnalyzer::NeedsRerun()
{
    return false;
}


U32 PWMSerialAnalyzer::GenerateSimulationData( U64 minimum_sample_index, U32 device_sample_rate,
                                            SimulationChannelDescriptor** simulation_channels )
{
    if( mSimulationInitilized == false )
    {
        mSimulationDataGenerator.Initialize( GetSimulationSampleRate(), mSettings.get() );
        mSimulationInitilized = true;
    }

    return mSimulationDataGenerator.GenerateSimulationData( minimum_sample_index, device_sample_rate, simulation_channels );
}

U32 PWMSerialAnalyzer::GetMinimumSampleRateHz()
{
    return (4e9 / mSettings->mThresholdPulseWidthNanoseconds);
}

const char* PWMSerialAnalyzer::GetAnalyzerName() const
{
    return "PWM Serial";
}

const char* GetAnalyzerName()
{
    return "PWM Serial";
}

Analyzer* CreateAnalyzer()
{
    return new PWMSerialAnalyzer();
}

void DestroyAnalyzer( Analyzer* analyzer )
{
    delete analyzer;
}
