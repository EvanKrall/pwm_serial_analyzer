#include "PWMSerialAnalyzerResults.h"
#include <AnalyzerHelpers.h>
#include "PWMSerialAnalyzer.h"
#include "PWMSerialAnalyzerSettings.h"
#include <iostream>
#include <sstream>
#include <stdio.h>

PWMSerialAnalyzerResults::PWMSerialAnalyzerResults( PWMSerialAnalyzer* analyzer, PWMSerialAnalyzerSettings* settings )
    : AnalyzerResults(), mSettings( settings ), mAnalyzer( analyzer )
{
}

PWMSerialAnalyzerResults::~PWMSerialAnalyzerResults()
{
}

void PWMSerialAnalyzerResults::GenerateBubbleText( U64 frame_index, Channel& /*channel*/,
                                                DisplayBase display_base ) // unrefereced vars commented out to remove warnings.
{
    // we only need to pay attention to 'channel' if we're making bubbles for more than one channel (as set by
    // AddChannelBubblesWillAppearOn)
    ClearResultStrings();
    Frame frame = GetFrame( frame_index );

    bool pulse_too_short = false;
    if( ( frame.mFlags & PULSE_TOO_SHORT_ERROR_FLAG ) != 0 )
        pulse_too_short = true;

    bool pulse_too_long = false;
    if( ( frame.mFlags & PULSE_TOO_LONG_ERROR_FLAG ) != 0 )
        pulse_too_long = true;

    bool timeout = false;
    if( ( frame.mFlags & TIMEOUT_ERROR_FLAG ) != 0 )
        timeout = true;

    char number_str[ 128 ];
    AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 1, number_str, 128 );

    char result_str[ 128 ];

    // normal case:
    if( timeout || pulse_too_long || pulse_too_short )
    {
        AddResultString( "!" );

        snprintf( result_str, sizeof( result_str ), "%s (error)", number_str );
        AddResultString( result_str );

        if ( timeout == true )
            snprintf( result_str, sizeof( result_str ), "%s (inter-bit timeout)", number_str );
        else if( pulse_too_long == true )
            snprintf( result_str, sizeof( result_str ), "%s (pulse too long)", number_str );
        else if( pulse_too_short == true )
            snprintf( result_str, sizeof( result_str ), "%s (pulse too short)", number_str );
        AddResultString( result_str );
    }
    else
    {
        AddResultString( number_str );
    }
}

void PWMSerialAnalyzerResults::GenerateExportFile( const char* file, DisplayBase display_base, U32 /*export_type_user_id*/ )
{
    // export_type_user_id is only important if we have more than one export type.
    std::stringstream ss;

    U64 trigger_sample = mAnalyzer->GetTriggerSample();
    U32 sample_rate = mAnalyzer->GetSampleRate();
    U64 num_frames = GetNumFrames();

    void* f = AnalyzerHelpers::StartFile( file );

    // Normal case -- not MP mode.
    ss << "Time [s],Value,Inter-bit timeout error,Pulse too long error,Pulse too short error" << std::endl;

    for( U32 i = 0; i < num_frames; i++ )
    {
        Frame frame = GetFrame( i );

        // static void GetTimeString( U64 sample, U64 trigger_sample, U32 sample_rate_hz, char* result_string, U32
        // result_string_max_length );
        char time_str[ 128 ];
        AnalyzerHelpers::GetTimeString( frame.mStartingSampleInclusive, trigger_sample, sample_rate, time_str, 128 );

        char number_str[ 128 ];
        AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 1, number_str, 128 );

        ss << time_str << "," << number_str;

        ss << ",";

        if( ( frame.mFlags & TIMEOUT_ERROR_FLAG ) != 0 )
            ss << "Error";

        ss << ",";

        if( ( frame.mFlags & PULSE_TOO_LONG_ERROR_FLAG ) != 0 )
            ss << "Error";

        ss << ",";

        if( ( frame.mFlags & PULSE_TOO_SHORT_ERROR_FLAG ) != 0 )
            ss << "Error";


        ss << std::endl;

        AnalyzerHelpers::AppendToFile( ( U8* )ss.str().c_str(), ss.str().length(), f );
        ss.str( std::string() );

        if( UpdateExportProgressAndCheckForCancel( i, num_frames ) == true )
        {
            AnalyzerHelpers::EndFile( f );
            return;
        }
    }

    UpdateExportProgressAndCheckForCancel( num_frames, num_frames );
    AnalyzerHelpers::EndFile( f );
}

void PWMSerialAnalyzerResults::GenerateFrameTabularText( U64 frame_index, DisplayBase display_base )
{
    ClearTabularText();
    Frame frame = GetFrame( frame_index );

    bool pulse_too_short = false;
    if( ( frame.mFlags & PULSE_TOO_SHORT_ERROR_FLAG ) != 0 )
        pulse_too_short = true;

    bool pulse_too_long = false;
    if( ( frame.mFlags & PULSE_TOO_LONG_ERROR_FLAG ) != 0 )
        pulse_too_long = true;

    bool timeout = false;
    if( ( frame.mFlags & TIMEOUT_ERROR_FLAG ) != 0 )
        timeout = true;

    char number_str[ 128 ];
    AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 1, number_str, 128 );

    char result_str[ 128 ];

    // normal case:
    if( ( pulse_too_long == true ) || ( pulse_too_short == true ) || (timeout == true) )
    {
        if( timeout == true )
            snprintf( result_str, sizeof( result_str ), "%s (timeout)", number_str);
        else if( pulse_too_long == true )
            snprintf( result_str, sizeof( result_str ), "%s (pulse too long)", number_str );
        else if( pulse_too_short == true )
            snprintf( result_str, sizeof( result_str ), "%s (pulse too short)", number_str );

        AddTabularText( result_str );
    }
    else
    {
        AddTabularText( number_str );
    }
}

void PWMSerialAnalyzerResults::GeneratePacketTabularText( U64 /*packet_id*/,
                                                       DisplayBase /*display_base*/ ) // unrefereced vars commented out to remove warnings.
{
    ClearResultStrings();
    AddResultString( "not supported" );
}

void PWMSerialAnalyzerResults::GenerateTransactionTabularText(
    U64 /*transaction_id*/, DisplayBase /*display_base*/ ) // unrefereced vars commented out to remove warnings.
{
    ClearResultStrings();
    AddResultString( "not supported" );
}
