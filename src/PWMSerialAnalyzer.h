#ifndef PWM_SERIAL_ANALYZER_H
#define PWM_SERIAL_ANALYZER_H

#include <Analyzer.h>
#include "PWMSerialAnalyzerResults.h"
#include "PWMSerialSimulationDataGenerator.h"

class PWMSerialAnalyzerSettings;
class PWMSerialAnalyzer : public Analyzer2
{
  public:
    PWMSerialAnalyzer();
    virtual ~PWMSerialAnalyzer();
    virtual void SetupResults();
    virtual void WorkerThread();

    virtual U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels );
    virtual U32 GetMinimumSampleRateHz();

    virtual const char* GetAnalyzerName() const;
    virtual bool NeedsRerun();


#pragma warning( push )
#pragma warning(                                                                                                                           \
    disable : 4251 ) // warning C4251: 'PWMSerialAnalyzer::<...>' : class <...> needs to have dll-interface to be used by clients of class

  protected: // functions

  protected: // vars
    std::auto_ptr<PWMSerialAnalyzerSettings> mSettings;
    std::auto_ptr<PWMSerialAnalyzerResults> mResults;
    AnalyzerChannelData* mPWMSerial;

    PWMSerialSimulationDataGenerator mSimulationDataGenerator;
    bool mSimulationInitilized;

    // PWMSerial analysis vars:
    U32 mSampleRateHz;

    BitState mIdle;
    BitState mActive;

    BitState mShort;
    BitState mLong;
#pragma warning( pop )
};

extern "C" ANALYZER_EXPORT const char* __cdecl GetAnalyzerName();
extern "C" ANALYZER_EXPORT Analyzer* __cdecl CreateAnalyzer();
extern "C" ANALYZER_EXPORT void __cdecl DestroyAnalyzer( Analyzer* analyzer );

#endif // PWM_SERIAL_ANALYZER_H
