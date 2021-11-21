#pragma once

#include "../Common/Common.h"
#include <SpectralEvaluation/GPSData.h>
#include <SpectralEvaluation/Evaluation/ReferenceFile.h>
#include <SpectralEvaluation/Evaluation/FitWindow.h>
#include <SpectralEvaluation/Configuration/DarkSettings.h>

#ifndef _CCONFIGURATIONSETTINGS_H_
#define _CCONFIGURATIONSETTINGS_H_

/** The <b>CCConfigurationSetting</b> holds the configuration of the scanning Instruments
    that is found in the configuration.xml-file. */
class CConfigurationSetting
{
public:
    class CommunicationSetting
    {
    public:
        CommunicationSetting();

        /** Resets all values to default */
        void Clear();

        /** What type of connection is it to the instrument */
        int connectionType;

        // ----------- The settings for serial communication --------------
        /** The port number to use, only useful if connection type is serial */
        long port;

        /** The baudrate to use for communication, only useful if connection type is serial */
        long baudrate;

        /** Which type of flow control to use, only useful if connection type is serial */
        int flowControl;

        /** The timeout for communication */
        long timeout;

        /** The medium through which the communciation occurs.
                MEDIUM_CABLE corresponds to a cable,
                MEDIUM_FREEWAVE_SERIAL_MODEM corresponds to a Freewave radio modem. */
        int medium;

        // ----- The additional settings for the serial Freewave communication -----

        /** The RadioID OR callbook number */
        CString radioID;

        // ----- Settings for FTP communication -----

        /** The IP-number / Host name of the scanning instrument */
        CString ftpHostName;

        /** Port to use for FTP, if not default (21). */
        int ftpPort;

        /** The username at the scanning instrument */
        CString ftpUserName;

        /** The password at the scanning instrument */
        CString ftpPassword;

        /** The administrator-username at the scanning instrument */
        CString ftpAdminUserName;

        /** The administrator-password at the scanning instrument */
        CString ftpAdminPassword;

        // ----- Settings for Directory Polling -----

        /** The directory to check for pak files */
        CString directory;

        // ----------- The general settings for the communication --------------

        /** How often the scanning Instrument should be queried for new data */
        long queryPeriod;

        /** Time to start sleeping */
        struct timeStruct sleepTime;

        /** Time to wake up */
        struct timeStruct wakeupTime;

        /** Assignment operator */
        CommunicationSetting& operator=(const CommunicationSetting& comm2);
    };

    class SpectrometerChannelSetting {
    public:
        /** Resets all values to default */
        void Clear();

        /** The fit settings that are defined for this spectrometer */
        novac::CFitWindow fitWindow;

        /** The settings for how to get the dark-spectrum */
        Configuration::CDarkSettings m_darkSettings;

        /** Assignment operator */
        SpectrometerChannelSetting& operator=(const SpectrometerChannelSetting& spec2);
    };

    class SpectrometerSetting
    {
    public:
        /** Resets all values to default */
        void Clear();

        /** The serial number of this spectrometer */
        CString serialNumber = "";

        /** The model-number of this spectrometer */
        std::string modelName = "S2000";

        /** The number of channels defined in this spectrometer */
        unsigned char channelNum = 0;

        SpectrometerChannelSetting channel[MAX_CHANNEL_NUM];

        /** Assignment operator */
        SpectrometerSetting& operator=(const SpectrometerSetting& spec2);
    };

    class WindSpeedMeasurementSetting {
    public:
        WindSpeedMeasurementSetting();

        /** Resets all values to default */
        void Clear();

        /** True if this system is supposed to make wind speed measurements automatically */
        bool automaticWindMeasurements;

        /** The preferred interval between each measurement [in seconds] */
        int interval;

        /** The preferred duration of each measurement [in seconds] */
        int duration;

        /** Measurements will only be performed if the centre of the
                plume is within +-maxAngle from zenith */
        double maxAngle;

        /** Measurements will only be performed if the centre of the plume
                is relatively stable over the last 'stablePeriod' number of scans */
        int stablePeriod;

        /** Measurements will only be performed if the peak-column (minus the offset)
                is larger than 'minPeakColumn' ppmm */
        double minPeakColumn;

        /** The desired angle [degrees] between the two measurement directions
                OR the desired distance [meters] between the two measurements at plume-height.
                The value is an angle if > 0 and a distance if < 0
                */
        double desiredAngle;

        /** This is true if we should trust and use the values for
                wind-direction and plume-height calculated by the program */
        int useCalculatedPlumeParameters;

        /** */
        double SwitchRange;

        /** Assignment operator */
        WindSpeedMeasurementSetting& operator=(const WindSpeedMeasurementSetting& ws2);
    };

    class SetupChangeSetting {
    public:
        SetupChangeSetting();

        /** Resets all values to default */
        void Clear();

        /** This is true if we should let the program change the setup of the
                instrument automatically. This only works for Heidelberg (V-II) instruments */
        int automaticSetupChange;

        /** This is true if we should trust and use the values for
                wind-direction and plume-height calculated by the program */
        int useCalculatedPlumeParameters;

        /** The tolerance for varying wind-directions. No changes will be done as long
                as the changes of the wind-direction is less than this value */
        double windDirectionTolerance;

        /** How brave we are on using the scanner, can be either of;
                CHANGEMODE_FAST or
                CHANGEMODE_SAFE  */
        int mode;

        /** Assignment operator */
        SetupChangeSetting& operator=(const SetupChangeSetting& ws2);
    };

    class MotorSetting
    {
    public:
        /** Resets all values to default */
        void Clear();

        /** The number of steps in one round */
        int stepsPerRound = 200;

        /** The motor-steps-compensation */
        int motorStepsComp = 85;

        /** Assignment operator */
        MotorSetting& operator=(const MotorSetting& other);
    };

    class ScanningInstrumentSetting
    {
    public:
        ScanningInstrumentSetting();

        /** Resets all values to default */
        void Clear();

        /** The volcano on which the instrument is measuring */
        CString volcano;

        /** The observatory which owns the scanning instrument */
        CString observatory;

        /** The site at which the scanning instrument is situated */
        CString site;

        /** The direction in which the instrument points (in degrees from north) */
        double  compass;

        /** The opening angle of the cone that the scanner measures in (in degrees).
        This is 90 degrees for the old scanner, and typically 30 or 45 degrees for the new. */
        double  coneAngle;

        /** The tilt of the system, in the direction of the scanner. */
        double tilt;

        /** The gps-coordinates for the scanning instrument */
        novac::CGPSData  gps;

        /** The communication settings for the scanning instrument */
        CommunicationSetting comm;

        /** The spectrometer inside the scanning instrument */
        SpectrometerSetting spec[MAX_SPECTROMETERS_PER_SCANNER];

        /** The number of spectrometers configured */
        unsigned long specNum;

        /** The settings for automatic wind speed measurements */
        WindSpeedMeasurementSetting windSettings;

        /** The settings for automatically changing the parameters of the scan */
        SetupChangeSetting scSettings;

        /** The settings for the motor(s) */
        MotorSetting motor[2];

        /** The type of the electronics box */
        ELECTRONICS_BOX electronicsBox;

        /** Plot column under Today's Data. */
        int plotColumn;

        /** Plot column history. */
        int plotColumnHistory;
        int minColumn;
        int maxColumn;

        /** Plot flux history. */
        int plotFluxHistory;
        int minFlux;
        int maxFlux;

        /** Assignment operator */
        ScanningInstrumentSetting& operator=(const ScanningInstrumentSetting& scanner2);
    };

    /** Settings for uploading the produced data to the NOVAC server. */
    class CFTPSetting
    {
    public:
        CFTPSetting();
        void SetFTPStatus(int status);
        CString ftpAddress;     // the name of the FTP-server
        int port;               // FTP port, in case not default
        CString userName;       // the user name at the FTP-server
        CString password;       // the password at the FTP-server
        CString protocol;       // the protocol to use, must be either FTP or SFTP
        int     ftpStatus;      // not used?
        int     ftpStartTime;   // the time of day when to start uploading (seconds since midnight)
        int     ftpStopTime;    // the time of day when to stop uploading (seconds since midnight)
    };

    /** Settings for publishing the results on a web - page */
    class WebSettings
    {
    public:
        int     publish = 0;           // 0 if don't publish, otherwise publish
        CString localDirectory = "";   // set to a local directory if the output is to be stored on the same computer
        CString imageFormat = ".png";  // the format of the images to save, can be .bmp, .gif, .jpg or .png
    };

    /** Settings for calling of external programs when receiving scans */
    class CExternalCallSettings
    {
    public:
        CString fullScanScript = ""; // the path of one shell-command/excetuable file to exceute when receiving a complete scan
        CString imageScript = "";    // the path of one shell-command/executable file to execute when one image has been generated
    };

    /** Settings for retrieval of the wind-field from external sources */
    class CWindFieldDataSettings {
    public:
        CWindFieldDataSettings();
        CString windFieldFile; // the path and file-name of the file which is the source of the wind-field data
        long windFileReloadInterval; // the reload inteval of the wind-field fiel (in minutes) - zero corresponds to never reload the file
        int enabled; // 1 if enabled; 0 if not
    };

    CConfigurationSetting() = default;

    /** Resets all values to default */
    void Clear();

    // ----------------------------------------------------------------
    // -------------------- PUBLIC CONSTANTS --------------------------
    // ----------------------------------------------------------------
    static const int STARTUP_MANUAL = 0;
    static const int STARTUP_AUTOMATIC = 1;

    static const int CHANGEMODE_SAFE = 0;
    static const int CHANGEMODE_FAST = 1;

    // ----------------------------------------------------------------
    // ----------------------- PUBLIC DATA ----------------------------
    // ----------------------------------------------------------------

    /** The configured scanning instruments */
    ScanningInstrumentSetting scanner[MAX_NUMBER_OF_SCANNING_INSTRUMENTS];

    /** How many scanning instruments that are defined */
    unsigned long scannerNum = 0;

    /** The output directory */
    CString outputDirectory = "";

    /** Settings for web - publishing the results */
    WebSettings webSettings;

    /** Startup method */
    int startup = STARTUP_MANUAL;

    /**The ftp server setting*/
    CFTPSetting ftpSetting;

    /** The settings for calling of external programs */
    CExternalCallSettings externalSetting;

    /** The settings for retrieving the wind-field from external sources */
    CWindFieldDataSettings windSourceSettings;
};

// --------------------------------------------------------------------------------------------------------- 
// --------------------------- Free functions working on the settings --------------------------------------
// --------------------------------------------------------------------------------------------------------- 

/** Lists the name of all volcanoes monitored by instruments connected to this computer. */
std::vector<std::string> ListMonitoredVolcanoes(const CConfigurationSetting& settings);

/** Extracts the name of the volcano which the provided instrument monitors.
    Returns empty string if the scanner could not be found. */
std::string GetVolcanoMonitoredByScanner(const CConfigurationSetting& settings, const std::string& serialNumber);

#endif