#include "PWMSerialSimulationDataGenerator.h"
#include "PWMSerialAnalyzerSettings.h"

PWMSerialSimulationDataGenerator::PWMSerialSimulationDataGenerator()
{
}

PWMSerialSimulationDataGenerator::~PWMSerialSimulationDataGenerator()
{
}

void PWMSerialSimulationDataGenerator::Initialize( U32 simulation_sample_rate, PWMSerialAnalyzerSettings* settings )
{
    mSimulationSampleRateHz = simulation_sample_rate;
    mSettings = settings;

    mClockGenerator.Init( 1e9, simulation_sample_rate );
    mPWMSerialSimulationData.SetChannel( mSettings->mInputChannel );
    mPWMSerialSimulationData.SetSampleRate( simulation_sample_rate );


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

    float shortPulse = (mSettings->mMinimumPulseWidthNanoseconds + mSettings->mThresholdPulseWidthNanoseconds) / 2 / 1e9;
    float longPulse = (mSettings->mMaximumPulseWidthNanoseconds + mSettings->mThresholdPulseWidthNanoseconds) / 2 / 1e9;

    if( mSettings->mOneIsShort == true )
    {
        mDurationZero = longPulse;
        mDurationOne = shortPulse;
    }
    else
    {
        mDurationZero = shortPulse;
        mDurationOne = longPulse;
    }

    mPWMSerialSimulationData.SetInitialBitState( mIdle );
    mPWMSerialSimulationData.Advance( mClockGenerator.AdvanceByTimeS( mSettings->mMaximumPulseWidthNanoseconds / 1e9 ) ); // Start with idle for mMaximumPulseWidthNanoseconds.

    mValue = 0;
}

U32 PWMSerialSimulationDataGenerator::GenerateSimulationData( U64 largest_sample_requested, U32 sample_rate,
                                                           SimulationChannelDescriptor** simulation_channels )
{
    U64 adjusted_largest_sample_requested =
        AnalyzerHelpers::AdjustSimulationTargetSample( largest_sample_requested, sample_rate, mSimulationSampleRateHz );

    while( mPWMSerialSimulationData.GetCurrentSampleNumber() < adjusted_largest_sample_requested )
    {
        CreatePWMSerialByte( mValue++ );
        mPWMSerialSimulationData.Advance( mClockGenerator.AdvanceByHalfPeriod( 10.0 ) ); // insert 10 bit-periods of idle
    }

    *simulation_channels = &mPWMSerialSimulationData;

    return 1; // we are retuning the size of the SimulationChannelDescriptor array.  In our case, the "array" is length 1.
}

void PWMSerialSimulationDataGenerator::CreatePWMSerialByte( U64 value )
{
    // assume we start idle

    U32 num_bits = 8;

    for ( U32 i = 0; i < num_bits; i++ ) {
        mPWMSerialSimulationData.Transition();  // active-going edge for start of bit
        if ( value & (1 << i) ) {
            mPWMSerialSimulationData.Advance( mClockGenerator.AdvanceByTimeS(mDurationOne) );
            mPWMSerialSimulationData.Transition();
            mPWMSerialSimulationData.Advance( mClockGenerator.AdvanceByTimeS(mDurationZero) );
        } else {
            mPWMSerialSimulationData.Advance( mClockGenerator.AdvanceByTimeS(mDurationZero) );
            mPWMSerialSimulationData.Transition();
            mPWMSerialSimulationData.Advance( mClockGenerator.AdvanceByTimeS(mDurationOne) );
        }
    }
}
