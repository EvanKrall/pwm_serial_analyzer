#ifndef PWM_SERIAL_ANALYZER_SETTINGS
#define PWM_SERIAL_ANALYZER_SETTINGS

#include <AnalyzerSettings.h>
#include <AnalyzerTypes.h>

namespace PWMSerialAnalyzerEnums
{
};

class PWMSerialAnalyzerSettings : public AnalyzerSettings
{
  public:
    PWMSerialAnalyzerSettings();
    virtual ~PWMSerialAnalyzerSettings();

    virtual bool SetSettingsFromInterfaces();
    void UpdateInterfacesFromSettings();
    virtual void LoadSettings( const char* settings );
    virtual const char* SaveSettings();


    Channel mInputChannel;
    U32 mMinimumPulseWidthNanoseconds;
    U32 mThresholdPulseWidthNanoseconds;
    U32 mMaximumPulseWidthNanoseconds;
    bool mHighIsIdle;
    bool mOneIsShort;

  protected:
    std::auto_ptr<AnalyzerSettingInterfaceChannel> mInputChannelInterface;
    std::auto_ptr<AnalyzerSettingInterfaceInteger> mMinimumPulseWidthNanosecondsInterface;
    std::auto_ptr<AnalyzerSettingInterfaceInteger> mThresholdPulseWidthNanosecondsInterface;
    std::auto_ptr<AnalyzerSettingInterfaceInteger> mMaximumPulseWidthNanosecondsInterface;
    std::auto_ptr<AnalyzerSettingInterfaceBool> mHighIsIdleInterface;
    std::auto_ptr<AnalyzerSettingInterfaceBool> mOneIsShortInterface;
};

#endif // PWM_SERIAL_ANALYZER_SETTINGS
