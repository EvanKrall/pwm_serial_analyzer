#ifndef PWM_SERIAL_SIMULATION_DATA_GENERATOR
#define PWM_SERIAL_SIMULATION_DATA_GENERATOR

#include <AnalyzerHelpers.h>

class PWMSerialAnalyzerSettings;

class PWMSerialSimulationDataGenerator
{
  public:
    PWMSerialSimulationDataGenerator();
    ~PWMSerialSimulationDataGenerator();

    void Initialize( U32 simulation_sample_rate, PWMSerialAnalyzerSettings* settings );
    U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channelss );

  protected:
    PWMSerialAnalyzerSettings* mSettings;
    U32 mSimulationSampleRateHz;

    U64 mValue;

    BitState mIdle;
    BitState mActive;

    float mDurationZero;
    float mDurationOne;


  protected: // PWMSerial specific
    void CreatePWMSerialByte( U64 value );
    ClockGenerator mClockGenerator;
    SimulationChannelDescriptor mPWMSerialSimulationData; // if we had more than one channel to simulate, they would need to be in an array
};
#endif // UNIO_SIMULATION_DATA_GENERATOR
