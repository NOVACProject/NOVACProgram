#include "StdAfx.h"
#include "configuration.h"

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
    // Serial settings
    this->baudrate = 115200;
    this->connectionType = FTP_CONNECTION;
    this->flowControl = 1;
    this->port = 1;
    medium = MEDIUM_CABLE;

    // Freewave radio modems
    radioID = "0";

    // FTP - Settings
    ftpHostName = "192.168.0.1";
    ftpUserName = "novac";
    ftpPassword = "1225";
    ftpAdminUserName = "administrator";
    ftpAdminPassword = "1225";

    // General Settings
    this->queryPeriod = 10 * 60; // <-- query every 10 minutes
    this->timeout = 30000;
    this->sleepTime.hour = 20;
    this->sleepTime.minute = 0;
    this->sleepTime.second = 0;
    this->wakeupTime.hour = 6;
    this->wakeupTime.minute = 0;
    this->wakeupTime.second = 0;
}

void CConfigurationSetting::CommunicationSetting::Clear()
{
    // initialize the communication
    // Serial settings
    this->baudrate = 57600;
    this->connectionType = FTP_CONNECTION;
    this->flowControl = 1;
    this->port = 1;
    medium = MEDIUM_CABLE;

    // Freewave radio modems
    radioID = "0";

    // FTP - Settings
    ftpHostName = "192.168.0.1";
    ftpUserName = "novac";
    ftpPassword = "1225";
    ftpAdminUserName = "administrator";
    ftpAdminPassword = "1225";


    // General Settings
    this->queryPeriod = 8000;
    this->timeout = 5000;
    this->sleepTime.hour = 20;
    this->sleepTime.minute = 0;
    this->sleepTime.second = 0;
    this->wakeupTime.hour = 6;
    this->wakeupTime.minute = 0;
    this->wakeupTime.second = 0;
}

/** ------------- Constructor for the motor-settings ----------------- */
void CConfigurationSetting::MotorSetting::Clear()
{
    this->motorStepsComp = 85;
    this->stepsPerRound = 200;
}

CConfigurationSetting::MotorSetting& CConfigurationSetting::MotorSetting::operator =(const CConfigurationSetting::MotorSetting &motor2)
{
    this->motorStepsComp = motor2.motorStepsComp;
    this->stepsPerRound = motor2.stepsPerRound;
    return *this;
}

/** ------------- Constructor for the scanner-settings ----------------- */
CConfigurationSetting::ScanningInstrumentSetting::ScanningInstrumentSetting()
{
    this->Clear();
}

void CConfigurationSetting::ScanningInstrumentSetting::Clear() {
    tilt = 0.0;
    coneAngle = 90.0;
    compass = 0;
    observatory = "";
    site = "";
    specNum = 0;
    volcano = "";
    electronicsBox = BOX_VERSION_2;
    for (int i = 0; i < MAX_SPECTROMETERS_PER_SCANNER; ++i)
    {
        spec[i].Clear();
    }
    plotColumn = 0;
    plotColumnHistory = 0;
    minColumn = 0;
    maxColumn = 500;
    windSettings.Clear();
    scSettings.Clear();
    motor[0].Clear();
    motor[1].Clear();
}

/** --------- Constructor for the spectrometer channel settings ------------- */

void CConfigurationSetting::SpectrometerChannelSetting::Clear()
{
    fitWindow.name = "";
    fitWindow.nRef = 0;
    fitWindow.polyOrder = 5;
    fitWindow.specLength = 2048;

    m_darkSettings.Clear();
}

// assignment
CConfigurationSetting::SpectrometerChannelSetting &CConfigurationSetting::SpectrometerChannelSetting::operator =(const CConfigurationSetting::SpectrometerChannelSetting &spec2)
{
    fitWindow = spec2.fitWindow;

    m_darkSettings = spec2.m_darkSettings;
    return *this;
}

/** ------------- Constructor for the spectrometer-settings ----------------- */

void CConfigurationSetting::SpectrometerSetting::Clear()
{
    this->channelNum = 0;
    this->serialNumber = "";
    this->modelName = "S2000";
    for (int i = 0; i < MAX_CHANNEL_NUM; ++i)
    {
        this->channel[i].Clear();
    }
}

CConfigurationSetting::SpectrometerSetting &CConfigurationSetting::SpectrometerSetting::operator=(const CConfigurationSetting::SpectrometerSetting& other)
{
    this->channelNum = other.channelNum;

    for (int i = 0; i < MAX_CHANNEL_NUM; ++i)
    {
        this->channel[i] = other.channel[i];
    }

    this->serialNumber = other.serialNumber;
    this->modelName = other.modelName;

    return *this;
}

CConfigurationSetting::ScanningInstrumentSetting &CConfigurationSetting::ScanningInstrumentSetting::operator=(const CConfigurationSetting::ScanningInstrumentSetting &scanner2) {

    comm = scanner2.comm;
    compass = scanner2.compass;
    coneAngle = scanner2.coneAngle;
    gps = scanner2.gps;

    observatory.Format("%s", (LPCSTR)scanner2.observatory);
    site.Format("%s", (LPCSTR)scanner2.site);

    for (int i = 0; i < MAX_SPECTROMETERS_PER_SCANNER; ++i)
        spec[i] = scanner2.spec[i];

    specNum = scanner2.specNum;
    volcano.Format("%s", (LPCSTR)scanner2.volcano);

    windSettings = scanner2.windSettings;
    scSettings = scanner2.scSettings;
    motor[0] = scanner2.motor[0];
    motor[1] = scanner2.motor[1];
    return *this;
}


CConfigurationSetting::CommunicationSetting &CConfigurationSetting::CommunicationSetting::operator=(const CConfigurationSetting::CommunicationSetting &comm2) {
    // Serial settings
    this->baudrate = comm2.baudrate;
    this->connectionType = comm2.connectionType;
    this->flowControl = comm2.flowControl;
    this->port = comm2.port;
    this->medium = comm2.medium;

    // Freewave radio modems
    radioID.Format("%s", (LPCSTR)comm2.radioID);

    // FTP - Settings
    ftpHostName.Format("%s", (LPCSTR)comm2.ftpHostName);
    ftpUserName.Format("%s", (LPCSTR)comm2.ftpUserName);
    ftpPassword.Format("%s", (LPCSTR)comm2.ftpPassword);

    // General Settings
    this->queryPeriod = comm2.queryPeriod;
    this->timeout = comm2.timeout;
    this->sleepTime = comm2.sleepTime;
    this->wakeupTime = comm2.wakeupTime;

    return *this;
}
/** ------------- Constructor for the ftp-settings ----------------- */
CConfigurationSetting::CFTPSetting::CFTPSetting()
    : ftpStatus(0), ftpAddress(""), userName(""), password(""), protocol("SFTP"), ftpStartTime(0), ftpStopTime(86400)
{
}

void CConfigurationSetting::CFTPSetting::SetFTPStatus(int status)
{
    ftpStatus = status;
}


CConfigurationSetting::CWindFieldDataSettings::CWindFieldDataSettings()
{
    // By default the wind-field file is on the FTP-server in the 'wind' directory
    windFieldFile = "ftp://129.16.35.206/wind/";
    windFileReloadInterval = 360; // re-read the file every 6 hours
}

CConfigurationSetting::WindSpeedMeasurementSetting::WindSpeedMeasurementSetting()
{
    this->Clear();
}

void CConfigurationSetting::WindSpeedMeasurementSetting::Clear()
{
    automaticWindMeasurements = false;
    duration = 20 * 60;		// 20 minutes duration
    interval = 60 * 60;		// measurement once every hour
    maxAngle = 30;				// only make wind measurements if the plume centre is within 30 degrees from zenith
    stablePeriod = 3;			// only measure if the plume is stable over the last three scans.
    minPeakColumn = 50;		// only measure if the peak column is at least 50 ppmm (over the offset-level)
    desiredAngle = 5;		// the inital desired angle
    useCalculatedPlumeParameters = 0; // the initial settings for using calculated wind-field
    SwitchRange = 50.0;
}
/** Assignment operator */

CConfigurationSetting::WindSpeedMeasurementSetting& CConfigurationSetting::WindSpeedMeasurementSetting::operator=(const WindSpeedMeasurementSetting &ws2) {
    this->automaticWindMeasurements = ws2.automaticWindMeasurements;
    this->duration = ws2.duration;
    this->interval = ws2.interval;
    this->maxAngle = ws2.maxAngle;
    this->minPeakColumn = ws2.minPeakColumn;
    this->stablePeriod = ws2.stablePeriod;
    this->desiredAngle = ws2.desiredAngle;
    this->useCalculatedPlumeParameters = ws2.useCalculatedPlumeParameters;
    this->SwitchRange = ws2.SwitchRange;
    return *this;
}

CConfigurationSetting::SetupChangeSetting::SetupChangeSetting() {
    this->Clear();
}

void CConfigurationSetting::SetupChangeSetting::Clear()
{
    automaticSetupChange = 0;
    useCalculatedPlumeParameters = 0;
    windDirectionTolerance = 20.0;
    mode = CHANGEMODE_FAST;
}
/** Assignment operator */

CConfigurationSetting::SetupChangeSetting& CConfigurationSetting::SetupChangeSetting::operator=(const SetupChangeSetting &scs2) {
    this->automaticSetupChange = scs2.automaticSetupChange;
    this->useCalculatedPlumeParameters = scs2.useCalculatedPlumeParameters;
    this->windDirectionTolerance = scs2.windDirectionTolerance;
    this->mode = scs2.mode;

    return *this;
}
