#include "StdAfx.h"
#include "Configuration.h"
#include <set>
#include <SpectralEvaluation/StringUtils.h>

/** The global instance of configuration settings */
CConfigurationSetting g_settings;


void CConfigurationSetting::Clear()
{
    for (int i = 0; i < MAX_NUMBER_OF_SCANNING_INSTRUMENTS; ++i)
    {
        scanner[i].Clear();
    }

    scannerNum = 0;
}

/** ------------- Constructor for the communication-settings ----------------- */
CConfigurationSetting::CommunicationSetting::CommunicationSetting()
{
    Clear();

    // The following settings had a different default value from the value in 'Clear'
    // unclear why. TODO: Investigate what values are suitable defaults and unify.
    baudrate = 115200;
    queryPeriod = 10 * 60; // <-- query every 10 minutes
    timeout = 30000;
}

void CConfigurationSetting::CommunicationSetting::Clear()
{
    // initialize the communication
    // Serial settings
    baudrate = 57600;
    connectionType = FTP_CONNECTION;
    flowControl = 1;
    port = 1;
    medium = MEDIUM_CABLE;

    // Freewave radio modems
    radioID = "0";

    // FTP - Settings
    ftpHostName = "192.168.0.1";
    ftpPort = 21;
    ftpUserName = "novac";
    ftpPassword = "1225";
    ftpAdminUserName = "administrator";
    ftpAdminPassword = "1225";

    // directory polling
    directory = "";

    // General Settings
    queryPeriod = 8000;
    timeout = 5000;
    sleepTime.hour = 20;
    sleepTime.minute = 0;
    sleepTime.second = 0;
    wakeupTime.hour = 6;
    wakeupTime.minute = 0;
    wakeupTime.second = 0;
}

/** ------------- Constructor for the motor-settings ----------------- */
void CConfigurationSetting::MotorSetting::Clear()
{
    motorStepsComp = 85;
    stepsPerRound = 200;
}

void CConfigurationSetting::ScanningInstrumentSetting::Clear() {
    tilt = 0.0;
    coneAngle = 90.0;
    compass = 0;
    observatory = "";
    site = "";
    specNum = 0;
    volcano = "";
    electronicsBox = BOX_MOXA;
    for (int i = 0; i < MAX_SPECTROMETERS_PER_SCANNER; ++i)
    {
        spec[i].Clear();
    }
    plotColumn = 0;
    plotColumnHistory = 0;
    minColumn = 0;
    maxColumn = 500;
    plotFluxHistory = 0;
    minFlux = 0;
    maxFlux = 500;
    windSettings.Clear();
    scSettings.Clear();
    motor[0].Clear();
    motor[1].Clear();
}

/** --------- Constructor for automatic calibrations for the spectrometer ------------- */

void CConfigurationSetting::AutomaticCalibrationSetting::Clear()
{
    enable = false;
    solarSpectrumFile = "";
    initialCalibrationFile = "";
    instrumentLineshapeFile = "";
    initialCalibrationType = 0;
    instrumentLineShapeFitOption = 0;
    instrumentLineShapeFitRegion = novac::WavelengthRange(330.0, 350.0);
}


/** --------- Constructor for the spectrometer channel settings ------------- */

void CConfigurationSetting::SpectrometerChannelSetting::Clear()
{
    fitWindow.name = "";
    fitWindow.reference.clear();
    fitWindow.polyOrder = 5;
    fitWindow.specLength = 2048;

    darkSettings.Clear();
}

/** ------------- Constructor for the spectrometer-settings ----------------- */

void CConfigurationSetting::SpectrometerSetting::Clear()
{
    channelNum = 0;
    serialNumber = "";
    modelName = "S2000";
    for (int i = 0; i < MAX_CHANNEL_NUM; ++i)
    {
        channel[i].Clear();
    }
}

CConfigurationSetting::CFTPSetting::CFTPSetting()
    : ftpStatus(0), ftpAddress(""), userName(""), password(""), protocol("SFTP"), port(22), ftpStartTime(0), ftpStopTime(86400)
{
}

void CConfigurationSetting::CFTPSetting::SetFTPStatus(int status)
{
    ftpStatus = status;
}


CConfigurationSetting::CWindFieldDataSettings::CWindFieldDataSettings()
{
    // By default the wind-field file is on the FTP-server in the 'wind' directory
    windFieldFile = "sftp://ors20.see.chalmers.se/Wind/";
    windFileReloadInterval = 360; // re-read the file every 6 hours
}

void CConfigurationSetting::WindSpeedMeasurementSetting::Clear()
{
    automaticWindMeasurements = true;
    duration = 20 * 60;		// 20 minutes duration
    interval = 60 * 60;		// measurement once every hour
    maxAngle = 30;			// only make wind measurements if the plume centre is within 30 degrees from zenith
    stablePeriod = 3;		// only measure if the plume is stable over the last three scans.
    minPeakColumn = 50;		// only measure if the peak column is at least 50 ppmm (over the offset-level)
    desiredAngle = 5;		// the inital desired angle
    useCalculatedPlumeParameters = 0; // the initial settings for using calculated wind-field
    SwitchRange = 50.0;
}

void CConfigurationSetting::SetupChangeSetting::Clear()
{
    automaticSetupChange = 0;
    useCalculatedPlumeParameters = 0;
    windDirectionTolerance = 20.0;
    mode = CHANGEMODE_FAST;
}

std::vector<std::string> ListMonitoredVolcanoes(const CConfigurationSetting& settings)
{
    std::set<std::string> allVolcanoes;

    for (unsigned int k = 0; k < settings.scannerNum; ++k)
    {
        std::string name = settings.scanner[k].volcano;
        allVolcanoes.insert(name);
    }

    std::vector<std::string> volcanoNames{ begin(allVolcanoes), end(allVolcanoes) };

    return volcanoNames;
}


bool IdentifySpectrometer(const CConfigurationSetting& settings, const std::string& serial, int& scannerIdx, int& spectrometerIdx)
{
    scannerIdx = 0;
    spectrometerIdx = 0;

    for (unsigned long ii = 0; ii < g_settings.scannerNum; ++ii)
    {
        for (unsigned long jj = 0; jj < g_settings.scanner[ii].specNum; ++jj)
        {
            const std::string thisSerial = std::string(g_settings.scanner[ii].spec[jj].serialNumber);
            if (EqualsIgnoringCase(thisSerial, serial))
            {
                scannerIdx = ii;
                spectrometerIdx = jj;
                return true;
            }
        }
    }

    return false;
}

