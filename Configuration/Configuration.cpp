#include "StdAfx.h"
#include "configuration.h"

/** The global instance of configuration settings */
CConfigurationSetting g_settings;

CConfigurationSetting::CConfigurationSetting(void)
{
	this->outputDirectory.Format("");

	this->scannerNum  = 0;
	this->startup     = STARTUP_MANUAL;
}

CConfigurationSetting::~CConfigurationSetting(void)
{
}

void CConfigurationSetting::Clear(void)
{
	int i;
	for(i = 0; i < MAX_NUMBER_OF_SCANNING_INSTRUMENTS; ++i){
		scanner[i].Clear();
	}

	scannerNum = 0;
}


/** ------------- Constructor for the communication-settings ----------------- */
CConfigurationSetting::CommunicationSetting::CommunicationSetting(){
	// initialize the communication

	// Serial settings
	this->baudrate = 115200;
	this->connectionType = FTP_CONNECTION;
	this->flowControl = 1;
	this->port = 1;
	medium = MEDIUM_CABLE;

	// Freewave radio modems
	radioID.Format("0");

	// FTP - Settings
	ftpIP[0]= 192;
	ftpIP[1]= 168;
	ftpIP[2]= 0;
	ftpIP[3]= 1;
	ftpUserName.Format("novac");
	ftpPassword.Format("1225");
	ftpAdminUserName.Format("administrator");
	ftpAdminPassword.Format("1225");

	// General Settings
	this->queryPeriod = 10*60; // <-- query every 10 minutes
	this->timeout = 5000;
	this->sleepTime.hour = 20;
	this->sleepTime.minute = 0;
	this->sleepTime.second = 0;
	this->wakeupTime.hour = 6;
	this->wakeupTime.minute = 0;
	this->wakeupTime.second = 0;
}

CConfigurationSetting::CommunicationSetting::~CommunicationSetting(){
}

void CConfigurationSetting::CommunicationSetting::Clear(){
	// initialize the communication
	// Serial settings
	this->baudrate = 57600;
	this->connectionType = FTP_CONNECTION;
	this->flowControl = 1;
	this->port = 1;
	medium = MEDIUM_CABLE;

	// Freewave radio modems
	radioID.Format("0");

	// FTP - Settings
	ftpIP[0]= 192;
	ftpIP[1]= 168;
	ftpIP[2]= 0;
	ftpIP[3]= 1;
	ftpUserName.Format("novac");
	ftpPassword.Format("1225");
	ftpAdminUserName.Format("administrator");
	ftpAdminPassword.Format("1225");


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
CConfigurationSetting::MotorSetting::MotorSetting(){
	this->Clear();
}
CConfigurationSetting::MotorSetting::~MotorSetting(){
}
void CConfigurationSetting::MotorSetting::Clear(){
	this->motorStepsComp	= 85;
	this->stepsPerRound		= 200;
}
CConfigurationSetting::MotorSetting& CConfigurationSetting::MotorSetting::operator =(const CConfigurationSetting::MotorSetting &motor2){
	this->motorStepsComp	= motor2.motorStepsComp;
	this->stepsPerRound		= motor2.stepsPerRound;
	return *this;
}

/** ------------- Constructor for the scanner-settings ----------------- */
CConfigurationSetting::ScanningInstrumentSetting::ScanningInstrumentSetting(){
	this->Clear();
}

CConfigurationSetting::ScanningInstrumentSetting::~ScanningInstrumentSetting(){
}

void CConfigurationSetting::ScanningInstrumentSetting::Clear(){
	tilt = 0.0;
	coneAngle = 90.0;
	compass = 0;
	observatory.Format("");
	site.Format("");
	specNum = 0;
	volcano.Format("");
	instrumentType = INSTR_GOTHENBURG;
	electronicsBox = BOX_VERSION_2;
	for(int i = 0; i < MAX_SPECTROMETERS_PER_SCANNER; ++i)
		spec[i].Clear();

	windSettings.Clear();
	scSettings.Clear();
	motor[0].Clear();
	motor[1].Clear();
}

CConfigurationSetting::DarkSettings::DarkSettings(){
	m_darkSpecOption		= MEASURE;

	m_darkCurrentOption = MEASURED;
	m_darkCurrentSpec.Format("");

	m_offsetOption		  = MEASURED;
	m_offsetSpec.Format("");
}
CConfigurationSetting::DarkSettings::~DarkSettings(){

}
void CConfigurationSetting::DarkSettings::Clear(){
	m_darkSpecOption		= MEASURE;

	m_darkCurrentOption = MEASURED;
	m_darkCurrentSpec.Format("");

	m_offsetOption		  = MEASURED;
	m_offsetSpec.Format("");
}

CConfigurationSetting::DarkSettings& CConfigurationSetting::DarkSettings::operator =(const CConfigurationSetting::DarkSettings &dark2){
	m_darkSpecOption		= dark2.m_darkSpecOption;

	m_darkCurrentOption = dark2.m_darkCurrentOption;
	m_offsetOption			= dark2.m_offsetOption;

	m_darkCurrentSpec.Format(dark2.m_darkCurrentSpec);
	m_offsetSpec.Format(dark2.m_offsetSpec);

	return *this;
}

/** --------- Constructor for the spectrometer channel settings ------------- */
CConfigurationSetting::SpectrometerChannelSetting::SpectrometerChannelSetting(){
	detectorSize = 2048; // default;
}

CConfigurationSetting::SpectrometerChannelSetting::~SpectrometerChannelSetting(){
}
void CConfigurationSetting::SpectrometerChannelSetting::Clear(){
	detectorSize = 2048;
	fitWindow.name.Format("");
	fitWindow.nRef = 0;
	fitWindow.polyOrder = 5;
	fitWindow.specLength = 2048;

	m_darkSettings.Clear();
}

// assignment
CConfigurationSetting::SpectrometerChannelSetting &CConfigurationSetting::SpectrometerChannelSetting::operator =(const CConfigurationSetting::SpectrometerChannelSetting &spec2){
	detectorSize = spec2.detectorSize;
	fitWindow = spec2.fitWindow;

	m_darkSettings = spec2.m_darkSettings;
	return *this;
}

/** ------------- Constructor for the spectrometer-settings ----------------- */
CConfigurationSetting::SpectrometerSetting::SpectrometerSetting(){
	channelNum = 0;
	serialNumber.Format("");
	model = S2000;
}

CConfigurationSetting::SpectrometerSetting::~SpectrometerSetting(){
}

void CConfigurationSetting::SpectrometerSetting::Clear(){
	channelNum = 0;
	serialNumber.Format("");
	model = S2000;
	for(int i = 0 ; i < MAX_CHANNEL_NUM; ++i)
		channel[i].Clear();
}

CConfigurationSetting::SpectrometerSetting &CConfigurationSetting::SpectrometerSetting::operator=(const CConfigurationSetting::SpectrometerSetting &spec2){
	channelNum = spec2.channelNum;

	for(int i = 0; i < MAX_CHANNEL_NUM; ++i)
		channel[i] = spec2.channel[i];

	serialNumber.Format("%s", (LPCSTR)spec2.serialNumber);
	model = spec2.model;

	return *this;
}

CConfigurationSetting::ScanningInstrumentSetting &CConfigurationSetting::ScanningInstrumentSetting::operator=(const CConfigurationSetting::ScanningInstrumentSetting &scanner2){

	comm		= scanner2.comm;
	compass		= scanner2.compass;
	coneAngle	= scanner2.coneAngle;
	gps			= scanner2.gps;

	observatory.Format("%s", (LPCSTR)scanner2.observatory);
	site.Format("%s", (LPCSTR)scanner2.site);

	for(int i = 0; i < MAX_SPECTROMETERS_PER_SCANNER; ++i)
		spec[i] = scanner2.spec[i];

	specNum		= scanner2.specNum;
	volcano.Format("%s", (LPCSTR)scanner2.volcano);

	instrumentType = scanner2.instrumentType;

	windSettings	= scanner2.windSettings;
	scSettings		= scanner2.scSettings;
	motor[0]		= scanner2.motor[0];
	motor[1]		= scanner2.motor[1];
	return *this;
}


CConfigurationSetting::CommunicationSetting &CConfigurationSetting::CommunicationSetting::operator=(const CConfigurationSetting::CommunicationSetting &comm2){
	// Serial settings
	this->baudrate				= comm2.baudrate;
	this->connectionType		= comm2.connectionType;
	this->flowControl			= comm2.flowControl;
	this->port					= comm2.port;
	this->medium				= comm2.medium;

	// Freewave radio modems
	radioID.Format("%s", (LPCSTR)comm2.radioID);

	// FTP - Settings
	ftpIP[0] = comm2.ftpIP[0];
	ftpIP[1] = comm2.ftpIP[1];
	ftpIP[2] = comm2.ftpIP[2];
	ftpIP[3] = comm2.ftpIP[3];
	ftpUserName.Format("%s", (LPCSTR)comm2.ftpUserName);
	ftpPassword.Format("%s", (LPCSTR)comm2.ftpPassword);

	// General Settings
	this->queryPeriod			= comm2.queryPeriod;
	this->timeout				= comm2.timeout;
	this->sleepTime				= comm2.sleepTime;
	this->wakeupTime			= comm2.wakeupTime;

	return *this;
}
/** ------------- Constructor for the ftp-settings ----------------- */
CConfigurationSetting::CFTPSetting::CFTPSetting()
 : ftpStatus(0), ftpAddress(""), userName(""), password(""), ftpStartTime(0), ftpStopTime(86400)
{
}
CConfigurationSetting::CFTPSetting::~CFTPSetting()
{
}
void CConfigurationSetting::CFTPSetting::SetFTPStatus(int status)
{
	ftpStatus = status;
}

CConfigurationSetting::WebSettings::WebSettings(){
	this->publish = 0;		// default is to not publish the results
	imageFormat.Format(".png");
	this->localDirectory.Format("");
}

CConfigurationSetting::WebSettings::~WebSettings(){
}

CConfigurationSetting::CExternalCallSettings::CExternalCallSettings(){
	// by default, nothing is called
	fullScanScript.Format("");
	imageScript.Format("");
}

CConfigurationSetting::CExternalCallSettings::~CExternalCallSettings(){
	// clean the file-names
	fullScanScript.Format("");
	imageScript.Format("");
}

CConfigurationSetting::CWindFieldDataSettings::CWindFieldDataSettings(){
	Common common;
	common.GetExePath();

	// By default the wind-field file is on the FTP-server in the 'wind' directory
	windFieldFile.Format("ftp://129.16.35.206/wind/");
	windFileReloadInterval	= 360; // re-read the file every 6 hours
}

CConfigurationSetting::CWindFieldDataSettings::~CWindFieldDataSettings(){
	windFieldFile.Format("");
	windFileReloadInterval	= 0;
}

CConfigurationSetting::WindSpeedMeasurementSetting::WindSpeedMeasurementSetting(){
	this->Clear();
}
CConfigurationSetting::WindSpeedMeasurementSetting::~WindSpeedMeasurementSetting(){

}
void CConfigurationSetting::WindSpeedMeasurementSetting::Clear(){
	automaticWindMeasurements = false;
	duration = 20 * 60;		// 20 minutes duration
	interval = 60 * 60;		// measurement once every hour
	maxAngle = 30;				// only make wind measurements if the plume centre is within 30 degrees from zenith
	stablePeriod = 3;			// only measure if the plume is stable over the last three scans.
	minPeakColumn	= 50;		// only measure if the peak column is at least 50 ppmm (over the offset-level)
	desiredAngle	= 5;		// the inital desired angle
	useCalculatedPlumeParameters = 0; // the initial settings for using calculated wind-field
	SwitchRange = 50.0; 
}
/** Assignment operator */

CConfigurationSetting::WindSpeedMeasurementSetting& CConfigurationSetting::WindSpeedMeasurementSetting::operator=(const WindSpeedMeasurementSetting &ws2){
	this->automaticWindMeasurements    = ws2.automaticWindMeasurements;
	this->duration                     = ws2.duration;
	this->interval                     = ws2.interval;
	this->maxAngle                     = ws2.maxAngle;
	this->minPeakColumn                = ws2.minPeakColumn;
	this->stablePeriod                 = ws2.stablePeriod;
	this->desiredAngle                 = ws2.desiredAngle;
	this->useCalculatedPlumeParameters = ws2.useCalculatedPlumeParameters;
	this->SwitchRange                  = ws2.SwitchRange;
	return *this;
}

CConfigurationSetting::SetupChangeSetting::SetupChangeSetting(){
	this->Clear();
}
CConfigurationSetting::SetupChangeSetting::~SetupChangeSetting(){

}
void CConfigurationSetting::SetupChangeSetting::Clear(){
	automaticSetupChange			= 0;
	useCalculatedPlumeParameters	= 0;
	windDirectionTolerance			= 20.0;
	mode							= CHANGEMODE_FAST;
}
/** Assignment operator */

CConfigurationSetting::SetupChangeSetting& CConfigurationSetting::SetupChangeSetting::operator=(const SetupChangeSetting &scs2){
	this->automaticSetupChange			= scs2.automaticSetupChange;
	this->useCalculatedPlumeParameters	= scs2.useCalculatedPlumeParameters;
	this->windDirectionTolerance		= scs2.windDirectionTolerance;
	this->mode							= scs2.mode;

	return *this;
}
