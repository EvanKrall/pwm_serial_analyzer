#ifndef PWM_SERIAL_ANALYZER_RESULTS
#define PWM_SERIAL_ANALYZER_RESULTS

#include <AnalyzerResults.h>

#define PULSE_TOO_SHORT_ERROR_FLAG ( 1 << 0 )
#define PULSE_TOO_LONG_ERROR_FLAG ( 1 << 1 )
#define TIMEOUT_ERROR_FLAG ( 1 << 2 )

class PWMSerialAnalyzer;
class PWMSerialAnalyzerSettings;

class PWMSerialAnalyzerResults : public AnalyzerResults
{
  public:
    PWMSerialAnalyzerResults( PWMSerialAnalyzer* analyzer, PWMSerialAnalyzerSettings* settings );
    virtual ~PWMSerialAnalyzerResults();

    virtual void GenerateBubbleText( U64 frame_index, Channel& channel, DisplayBase display_base );
    virtual void GenerateExportFile( const char* file, DisplayBase display_base, U32 export_type_user_id );

    virtual void GenerateFrameTabularText( U64 frame_index, DisplayBase display_base );
    virtual void GeneratePacketTabularText( U64 packet_id, DisplayBase display_base );
    virtual void GenerateTransactionTabularText( U64 transaction_id, DisplayBase display_base );

  protected: // functions
  protected: // vars
    PWMSerialAnalyzerSettings* mSettings;
    PWMSerialAnalyzer* mAnalyzer;
};

#endif // PWM_SERIAL_ANALYZER_RESULTS
