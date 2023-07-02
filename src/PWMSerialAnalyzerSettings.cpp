#include "PWMSerialAnalyzerSettings.h"

#include <AnalyzerHelpers.h>
#include <sstream>
#include <cstring>

#pragma warning( disable : 4800 ) // warning C4800: 'U32' : forcing value to bool 'true' or 'false' (performance warning)

PWMSerialAnalyzerSettings::PWMSerialAnalyzerSettings()
    : mInputChannel( UNDEFINED_CHANNEL ),
      mMinimumPulseWidthNanoseconds( 1 ),
      mThresholdPulseWidthNanoseconds( 75000 ),
      mMaximumPulseWidthNanoseconds( 400000 ),
      mHighIsIdle( true ),
      mOneIsShort( true )
{
    mInputChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
    mInputChannelInterface->SetTitleAndTooltip( "Input Channel", "PWM Serial" );
    mInputChannelInterface->SetChannel( mInputChannel );

    mMinimumPulseWidthNanosecondsInterface.reset( new AnalyzerSettingInterfaceInteger() );
    mMinimumPulseWidthNanosecondsInterface->SetTitleAndTooltip( "Minimum pulse width (nanoseconds)", "Specify the minimum pulse width in nanoseconds. Pulses shorter than this length are treated as errors." );
    mMinimumPulseWidthNanosecondsInterface->SetMax( 1000000000 );
    mMinimumPulseWidthNanosecondsInterface->SetMin( 1 );
    mMinimumPulseWidthNanosecondsInterface->SetInteger( mMinimumPulseWidthNanoseconds );

    mThresholdPulseWidthNanosecondsInterface.reset( new AnalyzerSettingInterfaceInteger() );
    mThresholdPulseWidthNanosecondsInterface->SetTitleAndTooltip( "Threshold pulse width (nanoseconds)", "Threshold time (in nanoseconds) between 0 and 1. " );
    mThresholdPulseWidthNanosecondsInterface->SetMax( 1000000000 );
    mThresholdPulseWidthNanosecondsInterface->SetMin( 1 );
    mThresholdPulseWidthNanosecondsInterface->SetInteger( mThresholdPulseWidthNanoseconds );

    mMaximumPulseWidthNanosecondsInterface.reset( new AnalyzerSettingInterfaceInteger() );
    mMaximumPulseWidthNanosecondsInterface->SetTitleAndTooltip( "Maximum pulse width (nanoseconds)", "Specify the maximum pulse width in nanoseconds. Pulses longer than this length are treated as errors." );
    mMaximumPulseWidthNanosecondsInterface->SetMax( 1000000000 );
    mMaximumPulseWidthNanosecondsInterface->SetMin( 1 );
    mMaximumPulseWidthNanosecondsInterface->SetInteger( mMaximumPulseWidthNanoseconds );

    mHighIsIdleInterface.reset( new AnalyzerSettingInterfaceBool() );
    mHighIsIdleInterface->SetTitleAndTooltip(
        "", "Check this if a high level means idle (pulses are low, e.g. HDQ or 1-Wire)"
    );
    mHighIsIdleInterface->SetCheckBoxText( "High is idle" );
    mHighIsIdleInterface->SetValue( mHighIsIdle );

    mOneIsShortInterface.reset( new AnalyzerSettingInterfaceBool() );
    mOneIsShortInterface->SetTitleAndTooltip(
        "", "Check this if a one bit is indicated by a short pulse. If unchecked, short pulses indicate 0."
    );
    mOneIsShortInterface->SetCheckBoxText( "Short pulses are 1" );
    mOneIsShortInterface->SetValue( mOneIsShort );

    AddInterface( mInputChannelInterface.get() );
    AddInterface( mMinimumPulseWidthNanosecondsInterface.get() );
    AddInterface( mThresholdPulseWidthNanosecondsInterface.get() );
    AddInterface( mMaximumPulseWidthNanosecondsInterface.get() );
    AddInterface( mHighIsIdleInterface.get() );
    AddInterface( mOneIsShortInterface.get() );

    // AddExportOption( 0, "Export as text/csv file", "text (*.txt);;csv (*.csv)" );
    AddExportOption( 0, "Export as text/csv file" );
    AddExportExtension( 0, "text", "txt" );
    AddExportExtension( 0, "csv", "csv" );

    ClearChannels();
    AddChannel( mInputChannel, "PWM Serial", false );
}

PWMSerialAnalyzerSettings::~PWMSerialAnalyzerSettings()
{
}

bool PWMSerialAnalyzerSettings::SetSettingsFromInterfaces()
{
    mInputChannel =  mInputChannelInterface->GetChannel();
    mMinimumPulseWidthNanoseconds =  mMinimumPulseWidthNanosecondsInterface->GetInteger();
    mThresholdPulseWidthNanoseconds =  mThresholdPulseWidthNanosecondsInterface->GetInteger();
    mMaximumPulseWidthNanoseconds =  mMaximumPulseWidthNanosecondsInterface->GetInteger();
    mHighIsIdle =  mHighIsIdleInterface->GetValue();
    mOneIsShort =  mOneIsShortInterface->GetValue();

    ClearChannels();
    AddChannel( mInputChannel, "PWM Serial", true );

    return true;
}

void PWMSerialAnalyzerSettings::UpdateInterfacesFromSettings()
{
    mInputChannelInterface->SetChannel( mInputChannel );
    mMinimumPulseWidthNanosecondsInterface->SetInteger( mMinimumPulseWidthNanoseconds );
    mThresholdPulseWidthNanosecondsInterface->SetInteger( mThresholdPulseWidthNanoseconds );
    mMaximumPulseWidthNanosecondsInterface->SetInteger( mMaximumPulseWidthNanoseconds );
    mHighIsIdleInterface->SetValue( mHighIsIdle );
    mOneIsShortInterface->SetValue( mOneIsShort );
}

void PWMSerialAnalyzerSettings::LoadSettings( const char* settings )
{
    SimpleArchive text_archive;
    text_archive.SetString( settings );

    const char* name_string; // the first thing in the archive is the name of the protocol analyzer that the data belongs to.
    text_archive >> &name_string;
    if( strcmp( name_string, "SaleaePWMSerialAnalyzer" ) != 0 )
        AnalyzerHelpers::Assert( "SaleaePWMSerialAnalyzer: Provided with a settings string that doesn't belong to us;" );

    text_archive >> mInputChannel;

    text_archive >> mInputChannel;
    text_archive >> mMinimumPulseWidthNanoseconds;
    text_archive >> mThresholdPulseWidthNanoseconds;
    text_archive >> mMaximumPulseWidthNanoseconds;
    text_archive >> mHighIsIdle;
    text_archive >> mOneIsShort;

    ClearChannels();
    AddChannel( mInputChannel, "PWM Serial", true );

    UpdateInterfacesFromSettings();
}

const char* PWMSerialAnalyzerSettings::SaveSettings()
{
    SimpleArchive text_archive;

    text_archive << "SaleaePWMSerialAnalyzer";
    text_archive << mInputChannel;
    text_archive << mMinimumPulseWidthNanoseconds;
    text_archive << mThresholdPulseWidthNanoseconds;
    text_archive << mMaximumPulseWidthNanoseconds;
    text_archive << mHighIsIdle;
    text_archive << mOneIsShort;

    return SetReturnString( text_archive.GetString() );
}
