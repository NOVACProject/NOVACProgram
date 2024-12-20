#pragma once

#include "../Common/Common.h"
#include <SpectralEvaluation/GPSData.h>
#include <SpectralEvaluation/Evaluation/ReferenceFile.h>
#include <SpectralEvaluation/Evaluation/FitWindow.h>
#include <SpectralEvaluation/Configuration/DarkSettings.h>
#include <SpectralEvaluation/Spectra/WavelengthRange.h>

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

        /** How often the scanning Instrument should be queried for new data.
            This is the time between polls in seconds */
        long queryPeriod;

        /** Time to start sleeping */
        struct timeStruct sleepTime;

        /** Time to wake up */
        struct timeStruct wakeupTime;

        /** Assignment operator */
        CommunicationSetting& operator=(const CommunicationSetting& other) = default;
        CommunicationSetting(const CommunicationSetting& other) = default;
    };

    class AutomaticCalibrationSetting
    {
    public:
        AutomaticCalibrationSetting() { Clear(); }

        /** If enabled then the automatic calibration will run at the defined intervals. */
        BOOL enable = FALSE;

        /** If enabled then new references will be generated and replace the user-configured references. */
        BOOL generateReferences = FALSE;

        /** Set to true to high-pass filter the created references (and convert them into ppmm). */
        BOOL filterReferences = TRUE;

        /** The number of hours which needs to pass between each calibration */
        int intervalHours = 2;

        /** The time of day when we can start performing calibrations. In seconds since midnight UTC.
            This time is compared against the time of the scan and hence needs to be in UTC.
            Notice that it is totally valid to have intervalTimeOfDayLow > intervalTimeOfDayHigh for locations far from Europe.
            Default value is at 9 o'clock (9 * 60 * 60) */
        int intervalTimeOfDayLow = 32400;

        /** The time of day when we can start performing calibrations. In seconds since midnight UTC.
            This time is compared against the time of the scan and hence needs to be in UTC.
            Default value is at 15 o'clock (15 * 60 * 60) */
        int intervalTimeOfDayHigh = 54000;

        /** The full path to the high resolved solar spectrum */
        CString solarSpectrumFile;

        /** Path to the intial calibration file (either .std, .clb or .xs).
            If this is a file in the extended std format then it may also contain the instrument line shape
            (and hence make the instrumentLineshapeFile unnecessary). */
        CString initialCalibrationFile;

        /** Path to the initial instrument line shape file (.slf) if any is provided.
            Ususally not set if m_initialCalibrationFile is .std. */
        CString instrumentLineshapeFile;

        /** The type of file for the initialCalibrationFile and instrumentLineshapeFile
        *   (only used for displaying the correct options in the user interface).
            0 corresponds to both initialCalibrationFile and instrumentLineshapeFile provided
            1 corresponds to only initialCalibrationFile */
        int initialCalibrationType = 0;

        /** The option for if an instrument line shape should be fitted as well during
        *   the retrieval of the pixel-to-wavelength calibration.
        *   0 corresponds to no fitting of an instrument line shape,
        *   1 corresponds to fitting a super-gaussian instrument line shape.  */
        int instrumentLineShapeFitOption = 1;

        /** The wavelength region in which the instrument line shape should be fitted (in nm).  */
        novac::WavelengthRange instrumentLineShapeFitRegion = novac::WavelengthRange(330.0, 350.0);

        void Clear();

        AutomaticCalibrationSetting(const AutomaticCalibrationSetting& other) = default;
        AutomaticCalibrationSetting& operator=(const AutomaticCalibrationSetting& other) = default;
    };

    class SpectrometerChannelSetting
    {
    public:
        SpectrometerChannelSetting() { Clear(); }

        /** Resets all values to default */
        void Clear();

        /** The fit settings that are defined for this spectrometer */
        novac::CFitWindow fitWindow;

        /** The settings for how to get the dark-spectrum */
        Configuration::CDarkSettings darkSettings;

        /** Specifies how and if the instrument should perform self-calibration */
        AutomaticCalibrationSetting autoCalibration;

        /** Assignment */
        SpectrometerChannelSetting& operator=(const SpectrometerChannelSetting& other) = default;
        SpectrometerChannelSetting(const SpectrometerChannelSetting& other) = default;
    };

    class SpectrometerSetting
    {
    public:
        SpectrometerSetting() { Clear(); }

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
        SpectrometerSetting& operator=(const SpectrometerSetting& other) = default;
        SpectrometerSetting(const SpectrometerSetting& other) = default;
    };

    class WindSpeedMeasurementSetting
    {
    public:
        WindSpeedMeasurementSetting() { Clear(); }

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

        /** Assignment */
        WindSpeedMeasurementSetting& operator=(const WindSpeedMeasurementSetting& other) = default;
        WindSpeedMeasurementSetting(const WindSpeedMeasurementSetting& other) = default;
    };

    class SetupChangeSetting
    {
    public:
        SetupChangeSetting() { Clear(); }

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

        /** Assignment */
        SetupChangeSetting& operator=(const SetupChangeSetting& other) = default;
        SetupChangeSetting(const SetupChangeSetting& other) = default;
    };

    class MotorSetting
    {
    public:
        MotorSetting() = default;

        /** Resets all values to default */
        void Clear();

        /** The number of steps in one round */
        int stepsPerRound = 200;

        /** The motor-steps-compensation */
        int motorStepsComp = 85;

        /** Assignment operator */
        MotorSetting& operator=(const MotorSetting& other) = default;
        MotorSetting(const MotorSetting& other) = default;
    };

    class ScanningInstrumentSetting
    {
    public:
        ScanningInstrumentSetting() { Clear(); }

        /** Resets all values to default */
        void Clear();

        /** The volcano on which the instrument is measuring */
        CString volcano;

        /** The observatory which owns the scanning instrument */
        CString observatory;

        /** The site at which the scanning instrument is situated */
        CString site;

        /** The direction in which the instrument points (in degrees from north) */
        double compass;

        /** The opening angle of the cone that the scanner measures in (in degrees).
            This is 90 degrees for the flat scanner, and typically 60 degrees for the conical. */
        double coneAngle;

        /** The tilt of the system, in the direction of the scanner. */
        double tilt;

        /** The gps-coordinates for the scanning instrument */
        novac::CGPSData gps;

        /** The communication settings for the scanning instrument */
        CommunicationSetting comm;

        /** The spectrometer inside the scanning instrument.
            TODO: This could benefit from being a std::vector instead */
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

        /** Assignment */
        ScanningInstrumentSetting& operator=(const ScanningInstrumentSetting& other) = default;
        ScanningInstrumentSetting(const ScanningInstrumentSetting& other) = default;
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
    class CWindFieldDataSettings
    {
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

/** Identifies the spectrometer with the provided serial in the current settings.
    The returned scannerIdx and spectrometerIdx identifies the found spectrometer.
        I.e., settings.scanner[scannerIdx].spec[spectrometerIdx].serialNumber will equal the requested serial.
    @return true if the spectrometer could be found, otherwise false.
    @param settings The current software settings
    @param serial The serial number of the spectrometer to locate. Will return the first one if duplicates exist.
    @param scannerIdx The index of the scanner.
    @param spectrometerIdx The index of the spectrometer */
bool IdentifySpectrometer(const CConfigurationSetting& settings, const std::string& serial, int& scannerIdx, int& spectrometerIdx);

#endif