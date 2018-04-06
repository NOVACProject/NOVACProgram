// RemoteConfiguration.cpp : implementation file
//

#include "stdafx.h"
#include "../NovacMasterProgram.h"
#include "RemoteConfigurationDlg.h"
#include "../Common/Common.h"
using namespace ConfigurationDialog;

// CRemoteConfiguration dialog

#define CONFIGURATION_FILE _T("CFG.TXT")

extern CConfigurationSetting g_settings;

IMPLEMENT_DYNAMIC(CRemoteConfigurationDlg, CPropertyPage)
CRemoteConfigurationDlg::CRemoteConfigurationDlg()
	: CSystemConfigurationPage(CRemoteConfigurationDlg::IDD)
{
	m_scannerTree = NULL;
	m_SerialController = new CSerialControllerWithTx();
	
	m_cfgLog.Format("%s\\cfgLog.txt", (LPCSTR)g_settings.outputDirectory);

	m_remoteConf.m_compass = m_remoteConf.m_debugflag = m_remoteConf.m_delay = 0;
	m_remoteConf.m_ftpIP[0] = m_remoteConf.m_ftpIP[1] = m_remoteConf.m_ftpIP[2] = m_remoteConf.m_ftpIP[3] = 0;
	m_remoteConf.m_ftpPassword.Format("");
	m_remoteConf.m_ftpUserName.Format("");
	m_remoteConf.m_ftpTimeOut = m_remoteConf.m_realTime = 0;
	m_remoteConf.m_measNum = m_remoteConf.m_motorStepComp = 0;
	m_remoteConf.m_percent = m_remoteConf.m_maxExptime = m_remoteConf.m_pitch = 0;
	m_remoteConf.m_roll = m_remoteConf.m_temp = 0;
	m_remoteConf.m_skipmotor = m_remoteConf.m_startChannel = m_remoteConf.m_stopChannel = 0;
	m_remoteConf.m_stepsPerRound = 0;
	for(int i = 0; i < MAX_SPEC_PER_SCAN; ++i){
		m_remoteConf.m_meas[i].chn = 0;
		m_remoteConf.m_meas[i].flag = 0;
		m_remoteConf.m_meas[i].intsum = m_remoteConf.m_meas[i].extsum = 0;
		m_remoteConf.m_meas[i].inttime = m_remoteConf.m_meas[i].pos = 0;
		m_remoteConf.m_meas[i].rep = 0;
		memset(m_remoteConf.m_meas[i].name, 0, 30*sizeof(char));
	}
}

CRemoteConfigurationDlg::~CRemoteConfigurationDlg()
{
	if(m_SerialController != NULL){
		delete(m_SerialController);
		m_SerialController = NULL;
	}
}

void CRemoteConfigurationDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	// the ftp-server settings
	DDX_Control(pDX, IDC_FTP_SERVER_IP,       m_ftpServerIp);
	DDX_Control(pDX, IDC_FTP_SERVER_USERNAME, m_ftpServerUserName);
	DDX_Control(pDX, IDC_FTP_SERVER_PASSWORD, m_ftpServerPassword);
	DDX_Text(pDX, IDC_FTP_SERVER_USERNAME, m_remoteConf.m_ftpUserName);
	DDX_Text(pDX, IDC_FTP_SERVER_PASSWORD, m_remoteConf.m_ftpPassword);

	// the spectrum transfer settings
	DDX_Control(pDX, IDC_SPECTRUM_CHANNEL_START,	m_channelStart);
	DDX_Control(pDX, IDC_SPECTRUM_CHANNEL_STOP,		m_channelStop);
	DDX_Control(pDX, IDC_CHECK_REALTIME,					m_realTime);
	DDX_Text(pDX, IDC_SPECTRUM_CHANNEL_START,		m_remoteConf.m_startChannel);
	DDX_Text(pDX, IDC_SPECTRUM_CHANNEL_STOP,		m_remoteConf.m_stopChannel);
	DDX_Check(pDX, IDC_CHECK_REALTIME,					m_remoteConf.m_realTime);

	// the motor settings
	DDX_Control(pDX, IDC_MOTOR_STEPSPERROUND,	m_motorStepsPerRound);
	DDX_Control(pDX, IDC_MOTOR_STEPCOMP,		m_motorStepComp);
	DDX_Control(pDX, IDC_MOTOR_DELAY,			m_motorDelay);
	DDX_Text(pDX, IDC_MOTOR_STEPSPERROUND,		m_remoteConf.m_stepsPerRound);
	DDX_Text(pDX, IDC_MOTOR_STEPCOMP,			m_remoteConf.m_motorStepComp);
	DDX_Text(pDX, IDC_MOTOR_DELAY,				m_remoteConf.m_delay);

	// the exposure time settings
	DDX_Control(pDX, IDC_EXPTIME_MAX,			m_exposureTimeMax);
	DDX_Control(pDX, IDC_EXPTIME_PERCENT,		m_exposureTimePercent);
	DDX_Control(pDX, IDC_MEASUREMENTS_FRAME,	m_measurementFrame);
	DDX_Text(pDX, IDC_EXPTIME_MAX,				m_remoteConf.m_maxExptime);
	DDX_Text(pDX, IDC_EXPTIME_PERCENT,			m_remoteConf.m_percent);

	// The geometry
	DDX_Text(pDX, IDC_GEOM_COMPASS,				m_remoteConf.m_compass);
	DDX_Text(pDX, IDC_GEOM_PITCH,				m_remoteConf.m_pitch);
	DDX_Text(pDX, IDC_GEOM_ROLL,				m_remoteConf.m_roll);

	// The upload/download buttons
	DDX_Control(pDX, IDC_UPLOADCFGBTN,			m_uploadConfiguration);
	DDX_Control(pDX, IDC_DNLDCFGBTN,			m_downConfiguration);

}


BEGIN_MESSAGE_MAP(CRemoteConfigurationDlg, CPropertyPage)
	ON_BN_CLICKED(IDC_DNLDCFGBTN, OnDownloadRemoteCfg)
	ON_BN_CLICKED(IDC_UPLOADCFGBTN, OnUploadRemoteCfg)

	ON_COMMAND(ID__INSERTROW, OnInsertMeasurement)
	ON_COMMAND(ID__DELETEROW, OnDeleteMeasurement)
END_MESSAGE_MAP()


// CRemoteConfiguration message handlers

BOOL CRemoteConfigurationDlg::OnInitDialog(){
	CDialog::OnInitDialog();

	// the path of the configuration file
	m_cfgPath.Format("%sTemp\\", (LPCSTR)g_settings.outputDirectory);
	m_cfgFile.Format("%s%s", (LPCSTR)m_cfgPath, CONFIGURATION_FILE);

	// it should not be able to upload a configuration file before a configuration 
	//	file has been downloaded
	m_uploadConfiguration.EnableWindow(FALSE);

	// initialize and update the measurement grid
	this->InitMeasGrid();
	this->PopulateMeasGrid();

	// show the updated settings
	this->UpdateData(FALSE);
	GetComSetting();

	if(m_toolTip.m_hWnd != NULL)
		return TRUE;

	// Enable the tool tips
	if(!m_toolTip.Create(this)){
		TRACE0("Failed to create tooltip control\n"); 
	}
	m_toolTip.AddTool(&m_ftpServerIp,       TEXT("The IP-number of the FTP-server that will be informed every time the scanning instrument changes IP-address"));
	m_toolTip.AddTool(&m_ftpServerUserName, TEXT("The user name in the FTP-server that will be informed every time the scanning instrument changes IP-address"));
	m_toolTip.AddTool(&m_ftpServerPassword, TEXT("The password of the FTP-server that will be informed every time the scanning instrument changes IP-address"));
	m_toolTip.AddTool(&m_channelStart,      TEXT("The first pixel in the spectrum that will be transferred"));
	m_toolTip.AddTool(&m_channelStop,       TEXT("The last pixel in the spectrum that will be transferred"));
	m_toolTip.AddTool(&m_realTime,          TEXT("If this box is checked the spectra will be evaluated one at a time. If not checked they will be evaluated one a full scan has been collected"));
	m_toolTip.AddTool(&m_motorStepsPerRound,TEXT("The number of steps the motor divides one round into"));
	m_toolTip.AddTool(&m_motorStepComp,		TEXT("The position of the switch"));
	m_toolTip.AddTool(&m_motorDelay,		TEXT("The delay between each motor step, in ms"));
	m_toolTip.AddTool(&m_exposureTimeMax,   TEXT("The maximum exposure time (in milli seconds)"));
	m_toolTip.AddTool(&m_exposureTimePercent, TEXT("The desired intensity of the spectra"));
	m_toolTip.AddTool(&m_downConfiguration,		TEXT("Downloads the configuration file in the currently selected scanning instrument."));
	m_toolTip.AddTool(&m_uploadConfiguration, TEXT("Uploads the active configuration file to the currently selected scanning instrument."));
	m_toolTip.SetMaxTipWidth(SHRT_MAX);
	m_toolTip.Activate(TRUE);

	return TRUE;
}

BOOL CRemoteConfigurationDlg::PreTranslateMessage(MSG* pMsg){
	m_toolTip.RelayEvent(pMsg);

	return CPropertyPage::PreTranslateMessage(pMsg);
}

void CRemoteConfigurationDlg::InitMeasGrid(){
	float columnNo = 9.5;
	CRect rect;
	m_measurementFrame.GetWindowRect(&rect);
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;

	if(m_measGrid.m_hWnd != NULL)
		return;
	
	rect.top = 20;
	rect.left = 10;
	rect.right = width - 20;
	rect.bottom = height - 10;
	this->m_measGrid.Create(rect, &m_measurementFrame, 0);
	this->m_measGrid.parent = this;

	m_measGrid.InsertColumn("");
	m_measGrid.SetColumnWidth(0, (int)(rect.right / (2*columnNo)));

	m_measGrid.InsertColumn("Position");
	m_measGrid.SetColumnWidth(1, (int)(rect.right / columnNo));

	m_measGrid.InsertColumn("Exp. Time");
	m_measGrid.SetColumnWidth(2, (int)(rect.right / columnNo));

	m_measGrid.InsertColumn("Sum1");
	m_measGrid.SetColumnWidth(3, (int)(rect.right / columnNo));

	m_measGrid.InsertColumn("Sum2");
	m_measGrid.SetColumnWidth(4, (int)(rect.right / columnNo));

	m_measGrid.InsertColumn("Channel");
	m_measGrid.SetColumnWidth(5, (int)(rect.right / columnNo));

	m_measGrid.InsertColumn("Name");
	m_measGrid.SetColumnWidth(6, (int)(rect.right / columnNo));

	m_measGrid.InsertColumn("Repetitions");
	m_measGrid.SetColumnWidth(7, (int)(rect.right / columnNo));

	m_measGrid.InsertColumn("Flag");
	m_measGrid.SetColumnWidth(8, (int)(rect.right / columnNo));

	m_measGrid.SetFixedRowCount(1);
	m_measGrid.SetFixedColumnCount(1);

	m_measGrid.SetRowCount(3);
	m_measGrid.SetEditable(TRUE); /* make sure the user can edit the positions */
}

void CRemoteConfigurationDlg::PopulateMeasGrid(){

	m_measGrid.SetRowCount(max(2, 1 + m_remoteConf.m_measNum));

	for(int i = 0; i < m_remoteConf.m_measNum ; ++i){
		m_measGrid.SetItemTextFmt(i+1, 0, "%d", i);
		m_measGrid.SetItemTextFmt(i+1, 1, "%d", m_remoteConf.m_meas[i].pos);
		m_measGrid.SetItemTextFmt(i+1, 2, "%d", m_remoteConf.m_meas[i].inttime);
		m_measGrid.SetItemTextFmt(i+1, 3, "%d", m_remoteConf.m_meas[i].intsum);
		m_measGrid.SetItemTextFmt(i+1, 4, "%d", m_remoteConf.m_meas[i].extsum);
		m_measGrid.SetItemTextFmt(i+1, 5, "%d", m_remoteConf.m_meas[i].chn);
		m_measGrid.SetItemTextFmt(i+1, 6, "%s", m_remoteConf.m_meas[i].name);
		m_measGrid.SetItemTextFmt(i+1, 7, "%d", m_remoteConf.m_meas[i].rep);
		m_measGrid.SetItemTextFmt(i+1, 8, "%d", m_remoteConf.m_meas[i].flag);
	}
}

void CRemoteConfigurationDlg::SaveMeasGrid(){
	int nRows = m_measGrid.GetRowCount() - 1;
	CGridCellBase *cell;

	for(int i = 0; i < nRows; ++i){
		cell = m_measGrid.GetCell(1+i, 1);
		m_remoteConf.m_meas[i].pos = atoi(cell->GetText());

		cell = m_measGrid.GetCell(1+i, 2);
		m_remoteConf.m_meas[i].inttime = atoi(cell->GetText());

		cell = m_measGrid.GetCell(1+i, 3);
		m_remoteConf.m_meas[i].intsum = atoi(cell->GetText());

		cell = m_measGrid.GetCell(1+i, 4);
		m_remoteConf.m_meas[i].extsum = atoi(cell->GetText());

		cell = m_measGrid.GetCell(1+i, 5);
		m_remoteConf.m_meas[i].chn = atoi(cell->GetText());

		cell = m_measGrid.GetCell(1+i, 6);
		sprintf(m_remoteConf.m_meas[i].name, "%s", cell->GetText());

		cell = m_measGrid.GetCell(1+i, 7);
		m_remoteConf.m_meas[i].rep = atoi(cell->GetText());

		cell = m_measGrid.GetCell(1+i, 8);
		m_remoteConf.m_meas[i].flag = atoi(cell->GetText());
	}
}

/** Uploading the configuration file */
void CRemoteConfigurationDlg::OnUploadRemoteCfg(){
	CString message, cfgFile;
	CString exeFilePath; //path of txzm.exe
	Common commonObject;
	commonObject.GetExePath();
	exeFilePath.Format("%stxzm.exe", (LPCSTR)commonObject.m_exePath);

	m_SerialController->SetSerialPort(m_currentScanner,
		g_settings.scanner[m_currentScanner].comm.port,
		g_settings.scanner[m_currentScanner].comm.baudrate,
		NOPARITY,
		8,
		ONESTOPBIT,
		g_settings.scanner[m_currentScanner].comm.flowControl);

	// A local copy of the string is necessary since the GetBuffer-function
	//		might change the variable
	cfgFile.Format("%s", (LPCSTR)m_cfgFile);

	/**Write cfg.txt*/
	WriteCfg(m_cfgFile, false);

	/**Write cfgLog.txt*/
	WriteCfg(m_cfgFile, true);

	/**Delete cfg.txt in remote PC*//**Upload new cfg.txt*/
	if(FAIL != m_SerialController->DelFile(CONFIGURATION_FILE,'B'))
	{
		if(m_SerialController->UploadFile(exeFilePath, cfgFile.GetBuffer(256), 'B'))
			message = _T("Begin to upload configuration file cfg.txt");
		
		if(IsExistingFile(m_cfgFile)==1)	//delete existing cfg.txt
			DeleteFile(m_cfgFile);
	}
	else
		MessageBox("Can not delete cfg.txt in remote system,\n please try again");
	ShowMessage(message);
}

/** Downloading the configuration file */
void CRemoteConfigurationDlg::OnDownloadRemoteCfg(){
	CString message;

	// If there's already a downloaded file, delete it
	if(IsExistingFile(m_cfgFile))
		DeleteFile((LPCTSTR)m_cfgFile);

	GetComSetting();
	m_SerialController->SetSerialPort(m_currentScanner,
		g_settings.scanner[m_currentScanner].comm.port,
		g_settings.scanner[m_currentScanner].comm.baudrate,
		NOPARITY,
		8,
		ONESTOPBIT,
		g_settings.scanner[m_currentScanner].comm.flowControl);
	
	/**download remote cfg*/
	if(SUCCESS == m_SerialController->DownloadFile(CONFIGURATION_FILE, m_cfgPath, 'B'))
	{
		ReadCfg(m_cfgFile);
		// When the remote configuration file has been downloaded and read, then
		//	shall it also be possible to upload it again.
		m_uploadConfiguration.EnableWindow(TRUE);
	}else{
		message = _T("Can not download configuration file.");
		ShowMessage(message);	
		MessageBox(message);
	}


}

/**Write cfg.txt file to upload*/
int CRemoteConfigurationDlg::WriteCfg(const CString &fileName, bool append)
{
	int currentSpec;
	GetScanAndSpec(m_currentScanner, currentSpec);
	if(currentSpec == -1){
		MessageBox("No spectrometer selected. Please select one and try again");
		return 1;
	}

	// -1. Get the data from the dialog
	UpdateData(TRUE);
	SaveMeasGrid();	// <-- Save the contents in the measurement grid

	// 0. Open the file
	FILE *f = NULL;
	if(append)
		f = fopen(fileName, "a+");
	else
		f = fopen(fileName, "w");
	if(f == NULL){
		MessageBox("Could not write to cfg.txt");
		return 1;
	}

	// 1. Write the header
	CString dateTime;
	Common commonObject;
	commonObject.GetDateTimeText(dateTime);
	fprintf(f, "%%-------------Modified at %s------------\n\n", (LPCSTR)dateTime);

	fprintf(f, "%%Questions? email mattias.johansson@chalmers.se\n\n");

	// 2. Write the instrument name
	fprintf(f, "%% This name will be written in the spectrum-files (maximum 16 characters)\nIt also defines the name for the file containing the IP number uploaded to the server\n");
	fprintf(f, "INSTRUMENTNAME=%s\n\n", (LPCSTR)m_configuration->scanner[m_currentScanner].spec[currentSpec].serialNumber);


	// 3. Write the ftp - server information
	BYTE byte1, byte2, byte3, byte4;
	m_ftpServerIp.GetAddress(byte1, byte2, byte3, byte4);
	if(byte1 != 0 || byte2 != 0 || byte3 != 0 || byte4 != 0){
		fprintf(f, "%% The following adress are notified with ftp by our current IP adress at startup\n%% and at every time  our dynamic IP adress is changed.");
		fprintf(f, "SERVER=%d.%d.%d.%d %s %s\n", byte1, byte2, byte3, byte4, 
			(LPCSTR)m_remoteConf.m_ftpUserName, (LPCSTR)m_remoteConf.m_ftpPassword);
		fprintf(f, "%% timeout in miliseconds for accessing the server\nFTPTIMEOUT=%d\n\n", m_remoteConf.m_ftpTimeOut);
	}

	// 4. Write the Spectrum transfer information
	fprintf(f, "%% The following channels defines which channels in the spectra that will be transferred\n");
	fprintf(f, "STARTCHN=%d\n",   m_remoteConf.m_startChannel);
	fprintf(f, "STOPCHN=%d\n\n",	m_remoteConf.m_stopChannel);

	fprintf(f, "%% If Realtime=1 then the spectra will be added to work.pak one at a time.\n");
	fprintf(f, "%% If RealTime=0 then the spectra will be added to work.pak one scan at a time\n");
	fprintf(f, "REALTIME=%d\n\n",			m_remoteConf.m_realTime);

	// 5. Write the motor information
	fprintf(f, "%% StepsPerRound defines the number of steps the steppermotor divides one round into\n");
	fprintf(f, "STEPSPERROUND=%d\n",	m_remoteConf.m_stepsPerRound);
	fprintf(f, "MOTORSTEPCOMP=%d\n",	m_remoteConf.m_motorStepComp);
	fprintf(f, "%% If Skipmotor=1 then the scanner will not be used. ONLY FOR TESTING PURPOSES\n");
	fprintf(f, "SKIPMOTOR=%d\n",				m_remoteConf.m_skipmotor);
	fprintf(f, "DELAY=%d\n\n",          m_remoteConf.m_delay);

	// 6. Write the geometry (compass, tilt...)
	fprintf(f, "%% The geometry: compassDirection  tiltX(=roll)  tiltY(=pitch)  temperature\n");
	fprintf(f, "COMPASS=%.1lf %.1lf %.1lf %.1lf \n\n", 
		m_remoteConf.m_compass,
		m_remoteConf.m_roll,
		m_remoteConf.m_pitch,
		m_remoteConf.m_temp);

	// 7. Write other things
	fprintf(f, "%% Percent defines how big part of the spectrometers dynamic range we want to use\n");
	fprintf(f,  "PERCENT=%.2lf\n\n",			m_remoteConf.m_percent);
	fprintf(f, "%% The maximum integration time that we allow the spectrometer to use. In milli seconds\n");
	fprintf(f,	"MAXINTTIME=%.0lf\n\n",	m_remoteConf.m_maxExptime);
	fprintf(f, "%% The debug-level, the higher number the more output will be created\n");
	fprintf(f,  "DEBUG=%d\n\n",         m_remoteConf.m_debugflag);

	// 8. Write the measurement information
	fprintf(f, "%% sum1 is inside the specrometer [1 to 15]\n%%-----pos----time-sum1-sum2--chn--basename----- repetitions\n");

	for(int i = 0; i < m_measGrid.GetRowCount() - 1; ++i)
	{
		fprintf(f, "MEAS=%3d %d %d %d",
			m_remoteConf.m_meas[i].pos, 
			m_remoteConf.m_meas[i].inttime, 
			m_remoteConf.m_meas[i].intsum, 
			m_remoteConf.m_meas[i].extsum);

		fprintf(f, " %d %s %d %d\n",
			m_remoteConf.m_meas[i].chn, 
			m_remoteConf.m_meas[i].name, 
			m_remoteConf.m_meas[i].rep, 
			m_remoteConf.m_meas[i].flag);
	}

	fclose(f);
	return 0;
}

int CRemoteConfigurationDlg::GetComSetting()
{
	if(m_configuration == NULL)
	{
		return -1;
	}
	int currentSpec;
	GetScanAndSpec(m_currentScanner, currentSpec);

	//CConfigurationSetting::CommunicationSetting &comm = m_configuration->scanner[currentScanner].comm;

	return m_currentScanner;

}
void CRemoteConfigurationDlg::WriteALineInFile(CString filename, CString line)
{
	FILE *f;
	f=fopen(filename,"a+");

	if(f<(FILE*)1)
		return;

	fprintf(f,line+"\n");  

	fclose(f);
}

/** Reads a cfg.txt - file */
int CRemoteConfigurationDlg::ReadCfg(const CString &filename){
	char *pt;
	FILE *fil;
	char nl[2]={ 0x0a, 0 };
	char lf[2]={ 0x0d, 0 };
	long i;
	int flag;
	char txt[256];
	m_remoteConf.m_measNum = 0;
	CString instrumentName, str;
		CString log;
	fil = fopen(filename, "r");
	if(fil<(FILE *)1){
		printf("Could not open file %s\n", (LPCSTR)filename);
		return 1;
	}

	while (fgets(txt, sizeof(txt) - 1, fil)) {
		if (strlen(txt) > 4 && txt[0] != '%') {
			pt = txt;
			if (pt = strstr(txt, nl)) {
				pt[0] = 0;
			}

			pt = txt;
			if (pt = strstr(txt, lf)) {
				pt[0] = 0;
			}

			//if(pt = strstr(txt,"INSTRUMENTNAME=")){
			//  pt = strstr(txt,"=");
			//  sscanf(&pt[1],"%s",temp);
			//  strncpy(instrumentname, temp, 16);
			//}

			if (pt = strstr(txt, "SERVER=")) {
				char username[1024], password[1024];
				int byte1, byte2, byte3, byte4;
				pt = strstr(txt, "=");
				if (sscanf(&pt[1], "%d.%d.%d.%d %s %s", &byte1, &byte2, &byte3,
					&byte4, username, password) == 6) {
					// Update the window
					m_remoteConf.m_ftpIP[0] = byte1;
					m_remoteConf.m_ftpIP[1] = byte2;
					m_remoteConf.m_ftpIP[2] = byte3;
					m_remoteConf.m_ftpIP[3] = byte4;
					this->m_ftpServerIp.SetAddress(byte1, byte2, byte3, byte4);
					m_remoteConf.m_ftpUserName.Format("%s", username);
					m_remoteConf.m_ftpPassword.Format("%s", password);

					// Update the log file
					log.Format("SERVER=%d.%d.%d.%d %s %s", byte1, byte2, byte3, byte4, username, password);
					WriteALineInFile(m_cfgLog, log);
				}
			}

			if (pt = strstr(txt, "FTPTIMEOUT=")) {
				pt = strstr(txt, "=");
				if (sscanf(&pt[1], "%d", &m_remoteConf.m_ftpTimeOut) == 1) {
					// Update the log file
					log.Format("FTPTIMEOUT=%d", m_remoteConf.m_ftpTimeOut);
					WriteALineInFile(m_cfgLog, log);
				}
			}

			if (pt = strstr(txt, "STEPSPERROUND=")) {
				pt = strstr(txt, "=");
				if (sscanf(&pt[1], "%d", &m_remoteConf.m_stepsPerRound) == 1) {
					// Update the log file
					log.Format("STEPSPERROUND=%d", m_remoteConf.m_stepsPerRound);
					WriteALineInFile(m_cfgLog, log);
				}
			}

			if (pt = strstr(txt, "MOTORSTEPSCOMP=")) {
				pt = strstr(txt, "=");
				if (sscanf(&pt[1], "%d", &m_remoteConf.m_motorStepComp) == 1) {
					// Update the log file
					log.Format("MOTORSTEPSCOMP=%d", m_remoteConf.m_motorStepComp);
					WriteALineInFile(m_cfgLog, log);
				}
			}

			if (pt = strstr(txt, "DELAY=")) {
				pt = strstr(txt, "=");
				if (sscanf(&pt[1], "%d", &m_remoteConf.m_delay) == 1) {
					// Update the log file
					log.Format("DELAY=%d", m_remoteConf.m_delay);
					WriteALineInFile(m_cfgLog, log);
				}
			}

			if (pt = strstr(txt, "COMPASS=")) {
				pt = strstr(txt, "=");
				if (sscanf(&pt[1], "%lf %lf %lf %lf",
					&m_remoteConf.m_compass, &m_remoteConf.m_roll, &m_remoteConf.m_pitch, &m_remoteConf.m_temp) == 4) {
					// Update the log file
					log.Format("COMPASS=%.1lf %.1lf %.1lf %.1lf ",
						m_remoteConf.m_compass, m_remoteConf.m_roll, m_remoteConf.m_pitch, m_remoteConf.m_temp);
					WriteALineInFile(m_cfgLog, log);
				}
			}

			if (pt = strstr(txt, "STARTCHN=")) {
				pt = strstr(txt, "=");
				if (sscanf(&pt[1], "%d", &m_remoteConf.m_startChannel) == 1) {
					// Update the log file
					log.Format("STARTCHN=%d", m_remoteConf.m_startChannel);
					WriteALineInFile(m_cfgLog, log);
				}
			}

			if (pt = strstr(txt, "STOPCHN=")) {
				pt = strstr(txt, "=");
				if (sscanf(&pt[1], "%d", &m_remoteConf.m_stopChannel) == 1) {
					// Update the log file
					log.Format("STOPCHN=%d", m_remoteConf.m_stopChannel);
					WriteALineInFile(m_cfgLog, log);
				}
			}

			if (pt = strstr(txt, "DEBUG=")) {
				pt = strstr(txt, "=");
				if (sscanf(&pt[1], "%d", &m_remoteConf.m_debugflag) == 1) {
					log.Format("DEBUG=%d", m_remoteConf.m_debugflag);
					WriteALineInFile(m_cfgLog, log);
				}
			}

			if (pt = strstr(txt, "REALTIME=")) {
				pt = strstr(txt, "=");
				if (sscanf(&pt[1], "%d", &m_remoteConf.m_realTime)) {
					// update the log file
					log.Format("REALTIME=%d", m_remoteConf.m_realTime);
					WriteALineInFile(m_cfgLog, log);
				}
			}

			if (pt = strstr(txt, "PERCENT=")) {
				pt = strstr(txt, "=");
				if (sscanf(&pt[1], "%lf", &m_remoteConf.m_percent) == 1) {
					log.Format("PERCENT=%.3lf", m_remoteConf.m_percent);
					WriteALineInFile(m_cfgLog, log);
				}
			}

			if (pt = strstr(txt, "MAXINTTIME="))
			{
				pt = strstr(txt, "=");
				if (pt) {
					if (sscanf(&pt[1], "%lf", &m_remoteConf.m_maxExptime) == 1) {
						log.Format("MAXINTTIME=%.0lf", m_remoteConf.m_maxExptime);
						WriteALineInFile(m_cfgLog, log);
					}
				}
			}

			if (pt = strstr(txt, "MEAS=")) {
				char nameBuffer[2048];
				pt = strstr(txt, "=");
				i = m_remoteConf.m_measNum;
				if (i < 61) {
					m_remoteConf.m_meas[i].rep = 1;
					flag = 0;
					if (pt) {
						if (sscanf(pt + 1, "%d %d %d %d %hd %s %d %d",
							&m_remoteConf.m_meas[i].pos,
							&m_remoteConf.m_meas[i].inttime,
							&m_remoteConf.m_meas[i].intsum,
							&m_remoteConf.m_meas[i].extsum,
							&m_remoteConf.m_meas[i].chn,
							nameBuffer,
							&m_remoteConf.m_meas[i].rep,
							&flag) == 8) {
							log.Format("MEAS=%d %d %d %d %d %s %d %d",
								m_remoteConf.m_meas[i].pos, m_remoteConf.m_meas[i].inttime, m_remoteConf.m_meas[i].intsum,
								m_remoteConf.m_meas[i].extsum, m_remoteConf.m_meas[i].chn, nameBuffer, m_remoteConf.m_meas[i].rep, flag);
							WriteALineInFile(m_cfgLog, log);
						}
					}


					nameBuffer[30] = 0;
					sprintf(m_remoteConf.m_meas[i].name, "%s", nameBuffer);
					m_remoteConf.m_meas[i].flag = (unsigned char)flag;
					++m_remoteConf.m_measNum;
				}
				else {
					puts("Maximum measurements exceeded");
				}
			}

			/*if(pt=strstr(txt,"SKIPMOTOR=")){
			pt=strstr(txt,"=");
			sscanf(&pt[1],"%d",&skipmotor);
				log.Format("SKIPMOTOR=%d",skipmotor);
				WriteALineInFile(m_cfgLog,log);
		}*/
		}
	}
	fclose(fil);

	// update the measurement table
	PopulateMeasGrid();

	// update all fields
	UpdateData(FALSE);
	return 0;
}

void CRemoteConfigurationDlg::OnInsertMeasurement(){
	CMeasurement tmp_meas[100];
	int i;

	CCellRange cellRange = m_measGrid.GetSelectedCellRange();
	int minRow  = cellRange.GetMinRow() - 1;
	int nRows   = cellRange.GetRowSpan();

	if(nRows <= 0) /* nothing selected*/
		return;

	if(m_measGrid.GetColumnCount() != cellRange.GetColSpan()+1) /* not a full row selected */
		return;

	// before we do anything, save the data that the user has typed in
	SaveMeasGrid();

	for(i = minRow; i < (minRow + nRows); ++i){
		memcpy(tmp_meas, &m_remoteConf.m_meas[i], (m_remoteConf.m_measNum - i)*sizeof(CMeasurement)); /* backup */
		memcpy(&m_remoteConf.m_meas[i+1], tmp_meas, (m_remoteConf.m_measNum - i)*sizeof(CMeasurement));
		++m_remoteConf.m_measNum;
	}

	/* Redraw the table */
	m_measGrid.DeleteNonFixedRows();
	PopulateMeasGrid();
}

void CRemoteConfigurationDlg::OnDeleteMeasurement(){
	CMeasurement tmp_meas[100];
	int i;

	CCellRange cellRange = m_measGrid.GetSelectedCellRange();
	int minRow = cellRange.GetMinRow() - 1;
	int nRows = cellRange.GetRowSpan();

	if(nRows <= 0) /* nothing selected*/
		return;

	if(m_measGrid.GetColumnCount() != cellRange.GetColSpan()+1) /* not a full row selected */
		return;

	// before we do anything, save the data that the user has typed in
	SaveMeasGrid();

	for(i = 0; i < nRows; ++i){
		memcpy(tmp_meas, &m_remoteConf.m_meas[minRow+1], (m_remoteConf.m_measNum - minRow)*sizeof(CMeasurement)); /* backup */
		memcpy(&m_remoteConf.m_meas[minRow], tmp_meas, (m_remoteConf.m_measNum - minRow)*sizeof(CMeasurement));
		--m_remoteConf.m_measNum;
	}

	/* Redraw the table */
	m_measGrid.DeleteNonFixedRows();
	this->PopulateMeasGrid();
}

void CRemoteConfigurationDlg::OnChangeScanner(){
	// Clear everything...
	m_ftpServerIp.ClearAddress();
	m_ftpServerUserName.Clear();
	m_ftpServerPassword.Clear();

	m_channelStart.Clear();
	m_channelStop.Clear();
	m_motorStepsPerRound.Clear();
	m_motorStepComp.Clear();
	m_motorDelay.Clear();
	m_exposureTimeMax.Clear();
	m_exposureTimePercent.Clear();

	m_uploadConfiguration.EnableWindow(FALSE);
	m_downConfiguration.EnableWindow(TRUE);
}