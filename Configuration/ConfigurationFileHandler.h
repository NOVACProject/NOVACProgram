#pragma once

#include "Configuration.h"
#include "../Common/XMLFileReader.h"
#include <SpectralEvaluation/Exceptions.h>

/** A <b>CConfigurationFileHandler</b> object is capable of reading
    and writing to the configuration file. */
namespace FileHandler
{

class ConfigurationFileException : public std::exception
{
private:
    const std::string m_msg = "";

public:
    ConfigurationFileException(const char* msg) :
        m_msg(msg)
    {}

    ConfigurationFileException(std::string msg) :
        m_msg(msg.c_str())
    {}

    const char* what() const noexcept override final { return m_msg.c_str(); }
};

class CConfigurationFileHandler : public CXMLFileReader
{
public:
    CConfigurationFileHandler();
    ~CConfigurationFileHandler();

    /** Reads the configuration file and stores the found values in the
        supplied reference 'configuration'. If fileName is not-null the reader will
        read from that file, otherwise it will read from 'configuration.xml'
        in the same directory as the executable.
        @throws novac::FileIoException if the file could not be opened (e.g. if it doesn't exist)
        @throws ConfigurationFileException if the reading fails for some reason */
    void ReadConfigurationFile(CConfigurationSetting& configuration, const CString* fileName = nullptr);

    /** Reads the FTP login configuration file and stores the found values in the
        supplied reference 'conf'. If fileName is not-null the reader will
        read from that file, otherwise it will read from 'ftplogin.xml'
        in the same directory as the executable.
        @throws novac::FileIoException if the file could not be opened (e.g. if it doesn't exist)
        @throws ConfigurationFileException if the reading fails for some reason */
    void ReadFtpLoginConfigurationFile(CConfigurationSetting& configuration, const CString* fileName = nullptr);

    /** Writes the configuration file using the values stored in the
        supplied reference 'conf'. If fileName is not-null the reader will
        write to that file, otherwise it will write to 'configuration.xml'
        in the same directory as the executable.
        @throws ConfigurationFileException if the writing fails for some reason */
    void WriteConfigurationFile(const CConfigurationSetting& configuration, const CString* fileName = nullptr) const;

    /** Writes the FTP login configuration file using the values stored in the
        supplied reference 'conf'. If fileName is not-null the reader will
        write to that file, otherwise it will write to 'ftplogin.xml'
        in the same directory as the executable.
        @throws ConfigurationFileException if the writing fails for some reason */
    void WriteFtpLoginConfigurationFile(const CConfigurationSetting& configuration, const CString* fileName = nullptr) const;

private:
    // ---------------------- PRIVATE DATA ----------------------------
    /** A pointer to the current scanning instrument.
        This is used while reading and parsing a configuration file, and should in all instances point to the instrument currently being read in.
        Storing this as a member is error prone, but has at least proven to work so far.. */
    CConfigurationSetting::ScanningInstrumentSetting* curScanner = nullptr;

    /** A pointer to the configuration object that we are filling in data into.
        This is used while reading and parsing a configuration file.
        Storing this as a member is error prone, but has at least proven to work so far.. */
    CConfigurationSetting* conf = nullptr;

    // -------------------- PRIVATE METHODS --------------------------

    /** Makes a sanity check of the settings just read in */
    static void CheckSettings(CConfigurationSetting& configuration);

    /** Starts the parsing */
    int Parse();

    /** Parses one 'scanningInstrument' section */
    int Parse_ScanningInstrument();

    /** Parses one 'spectrometer' section */
    int Parse_Spectrometer();

    /** Parses one custom spectrometer model section */
    int Parse_CustomSpectrometerModel(CConfigurationSetting::SpectrometerSetting* curSpec);

    /** Parses one 'channel' section */
    int Parse_Channel();

    /** Parses one 'calibration' section */
    int Parse_Calibration(CConfigurationSetting::SpectrometerChannelSetting* curSpec);

    /** Parses one 'communication' section */
    int Parse_Communication();

    /** Parses one 'windmeasurement' section */
    int	Parse_WindMeasurement();

    /** Parses one 'realtimesetup' section */
    int Parse_RealTimeSetup();

    /** Parses the 'Specie' section. Deprecated!, replaced by 'reference' */
    int Parse_Specie(CConfigurationSetting::SpectrometerChannelSetting* curSpec);

    /** Parses a shift or squeeze section */
    int Parse_ShiftOrSqueeze(const CString& label, novac::SHIFT_TYPE& option, double& lowValue, double& highValue);

    /** Parses the 'Reference'-section. */
    int Parse_Reference(CConfigurationSetting::SpectrometerChannelSetting* curChannel);

    /** Parses the 'outputDir' section */
    int Parse_OutputDir();

    /** Parses the 'sourceInfo' section */
    int Parse_SourceInfo();

    /** Parses the 'localPublishDir' - section */
    int Parse_LocalPublishDir();

    /** Parses the 'windImport' - section */
    int Parse_WindImport();

    /** Parses the 'medium' section */
    int Parse_Medium(CConfigurationSetting::CommunicationSetting* curComm);
};
}