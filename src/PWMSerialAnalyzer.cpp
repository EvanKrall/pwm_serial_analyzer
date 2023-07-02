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

    mPWMSerial = GetAnalyzerChannelData( mSettings->mInputChannel );

    if( mPWMSerial->GetBitState() == mActive )
        mPWMSerial->AdvanceToNextEdge();

    for( ;; )
    {
        // we're starting idle. Find the next transition to active.
        mPWMSerial->AdvanceToNextEdge();

        // we're now at the beginning of the start bit.  We can start collecting the data.
        U64 pulse_starting_sample = mPWMSerial->GetSampleNumber();

        bool value = false;
        bool pulse_too_short = false;
        bool pulse_too_long = false;

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

        // ok now record the value!
        // note that we're not using the mData2 or mType fields for anything, so we won't bother to set them.
        Frame frame;
        frame.mStartingSampleInclusive = pulse_starting_sample;
        frame.mEndingSampleInclusive = mPWMSerial->GetSampleNumber();
        frame.mData1 = value;
        frame.mFlags = 0;
        if( pulse_too_short == true )
            frame.mFlags |= PULSE_TOO_SHORT_ERROR_FLAG | DISPLAY_AS_ERROR_FLAG;

        if( pulse_too_long == true )
            frame.mFlags |= PULSE_TOO_LONG_ERROR_FLAG | DISPLAY_AS_ERROR_FLAG;

        mResults->AddFrame( frame );

        FrameV2 framev2;

        framev2.AddBoolean( "value", value );

        if( pulse_too_short )
        {
            framev2.AddString( "error", "pulse too short" );
        }
        else if( pulse_too_long )
        {
            framev2.AddString( "error", "pulse too long" );
        }

        mResults->AddFrameV2( framev2, "data", pulse_starting_sample, mPWMSerial->GetSampleNumber() );

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
