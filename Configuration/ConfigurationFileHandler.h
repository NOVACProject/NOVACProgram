#pragma once

#include "Configuration.h"
#include "../Common/XMLFileReader.h"

/** A <b>CConfigurationFileHandler</b> object is capable of reading
    and writing to the configuration file. */
namespace FileHandler
{
  class CConfigurationFileHandler : public CXMLFileReader
  {
  public:
    CConfigurationFileHandler(void);
    ~CConfigurationFileHandler(void);

    /** Reads the configuration file and stores the found values in the 
        supplied reference 'conf'. If fileName is not-null the reader will 
        read from that file, otherwise it will read from 'configuration.xml' 
        in the same directory as the executable. 
        @return 0 on sucess. @return 1 if any error occurs */
    int ReadConfigurationFile(CConfigurationSetting &configuration, const CString *fileName = NULL);

	/** Reads the FTP login configuration file and stores the found values in the
	supplied reference 'conf'. If fileName is not-null the reader will
	read from that file, otherwise it will read from 'ftplogin.xml'
	in the same directory as the executable.
	@return 0 on sucess. @return 1 if any error occurs */
	int ReadFtpLoginConfigurationFile(CConfigurationSetting &configuration, const CString *fileName = NULL);

    /** Writes the configuration file using the values stored in the 
        supplied reference 'conf'. If fileName is not-null the reader will 
        write to that file, otherwise it will write to 'configuration.xml' 
        in the same directory as the executable. 
        @return 0 on sucess. @return 1 if any error occurs */
	int WriteConfigurationFile(CConfigurationSetting &configuration, const CString *fileName = NULL);

	/** Writes the FTP login configuration file using the values stored in the
	supplied reference 'conf'. If fileName is not-null the reader will
	write to that file, otherwise it will write to 'ftplogin.xml'
	in the same directory as the executable.
	@return 0 on sucess. @return 1 if any error occurs */
	int WriteFtpLoginConfigurationFile(CConfigurationSetting &configuration, const CString *fileName = NULL);

  private:
    // ---------------------- PRIVATE DATA ----------------------------
    /** A Pointer to the current scanning instrument */
    CConfigurationSetting::ScanningInstrumentSetting *curScanner;

    /** A Pointer to the configuration object that we are filling in or reading from */
    CConfigurationSetting *conf;

    // -------------------- PRIVATE METHODS --------------------------

		/** Makes a sanity check of the settings just read in */
		int CheckSettings();

    /** Starts the parsing */
    int Parse();

    /** Parses one 'scanningInstrument' section */
    int Parse_ScanningInstrument();

    /** Parses one 'spectrometer' section */
    int Parse_Spectrometer();

    /** Parses one 'channel' section */
    int Parse_Channel();
		
		/** Parses one 'communication' section */
    int Parse_Communication();

		/** Parses one 'windmeasurement' section */
		int	Parse_WindMeasurement();

		/** Parses one 'realtimesetup' section */
		int Parse_RealTimeSetup();

    /** Parses the 'Specie' section. Deprecated!, replaced by 'reference' */
    int Parse_Specie(CConfigurationSetting::SpectrometerChannelSetting *curSpec);

		/** Parses a shift or squeeze section */
		int Parse_ShiftOrSqueeze(const CString &label, Evaluation::SHIFT_TYPE &option, double &lowValue, double &highValue);

		/** Parses the 'Reference'-section. */
		int Parse_Reference(CConfigurationSetting::SpectrometerChannelSetting *curChannel);

    /** Parses the 'outputDir' section */
    int Parse_OutputDir();

    /** Parses the 'sourceInfo' section */
    int Parse_SourceInfo();

		/** Parses the 'localPublishDir' - section */
		int Parse_LocalPublishDir();

		/** Parses the 'windImport' - section */
		int Parse_WindImport();

		/** Parses the 'motor' - section */
		int Parse_Motor();

		/** Parses the 'medium' section */
		int Parse_Medium(CConfigurationSetting::CommunicationSetting *curComm);
  };
}