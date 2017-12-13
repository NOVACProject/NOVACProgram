// ConfigureCOMPortDlg.cpp : implementation file
//

#include "stdafx.h"
#include "../NovacMasterProgram.h"
#include "ConfigureCOMPortDlg.h"

using namespace ConfigurationDialog;


// CConfigureCOMPortDlg dialog

IMPLEMENT_DYNAMIC(CConfigureCOMPortDlg, CPropertyPage)
CConfigureCOMPortDlg::CConfigureCOMPortDlg()
	: CSystemConfigurationPage(CConfigureCOMPortDlg::IDD)
{
	m_configuration = NULL;

	m_curSetting = 2;

	m_availableBaudrates[0] = 9600;
	m_availableBaudrates[1] = 19200;
	m_availableBaudrates[2] = 38400;
	m_availableBaudrates[3] = 57600;
	m_availableBaudrates[4] = 115200;

	this->m_intervalHr = 0;
	this->m_intervalMin = 0;
	this->m_intervalSec = 0;
	this->m_sleepHr     = 18;
	this->m_sleepMin    = 0;
	this->m_sleepSec    = 0;
	this->m_wakeHr      = 6;
	this->m_wakeMin     = 0;
	this->m_wakeSec     = 0;
}

CConfigureCOMPortDlg::~CConfigureCOMPortDlg()
{
}

void CConfigureCOMPortDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	DDX_Radio(pDX, IDC_RADIO_SERIAL, m_curSetting);

	// The ftp-settings
	DDX_Control(pDX, IDC_FTP_IPADDRESS, m_IPAddress);
	DDX_Text(pDX, IDC_FTP_USERNAME, m_ftpUserName);
	DDX_Control(pDX, IDC_FTP_USERNAME, m_editUserName);
	DDX_Text(pDX, IDC_FTP_PASSWORD, m_ftpPassword);
	DDX_Control(pDX, IDC_FTP_PASSWORD, m_editPassword);

	// The serial settings
	DDX_Control(pDX, IDC_COMPORT_COMBO, m_comPort);
	DDX_Control(pDX, IDC_BAUDRATE_COMBO, m_baudrate);
	DDX_Control(pDX, IDC_HANDSHAKE_COMBO, m_handshake);

	// The Freewave callbook/radioID
	DDX_Control(pDX, IDC_EDIT_RADIOID, m_editRadioID);
	DDX_Text(pDX, IDC_EDIT_RADIOID, m_radioID);


	// The time out
	DDX_Text(pDX, IDC_TIMEOUT_EDIT, m_timeout);
	DDX_Control(pDX, IDC_TIMEOUT_EDIT, m_editTimeOut);

	// The connection interval
	DDX_Text(pDX, IDC_INTERVAL_HR,  m_intervalHr);
	DDX_Text(pDX, IDC_INTERVAL_MIN, m_intervalMin);
	DDX_Text(pDX, IDC_INTERVAL_SEC, m_intervalSec);
	DDX_Control(pDX, IDC_STATIC_CONNECTION_INTERVAL, m_connectionIntervalLabel);

	// The sleep from...
	DDX_Text(pDX, IDC_SLEEPHR, m_sleepHr);
	DDV_MinMaxInt(pDX, m_sleepHr, 0, 23);
	DDX_Control(pDX, IDC_SLEEPHR, m_editSleepHr);

	DDX_Text(pDX, IDC_SLEEPMIN, m_sleepMin);
	DDV_MinMaxInt(pDX, m_sleepMin, 0, 59);
	DDX_Control(pDX, IDC_SLEEPMIN, m_editSleepMin);

	DDX_Text(pDX, IDC_SLEEPSEC, m_sleepSec);
	DDV_MinMaxInt(pDX, m_sleepSec, 0, 59);
	DDX_Control(pDX, IDC_SLEEPSEC, m_editSleepSec);

	// ... and the sleep to
	DDX_Text(pDX, IDC_WAKEHR, m_wakeHr);
	DDV_MinMaxInt(pDX, m_wakeHr, 0, 23);
	DDX_Control(pDX, IDC_WAKEHR, m_editWakeHr);

	DDX_Text(pDX, IDC_WAKEMIN, m_wakeMin);
	DDV_MinMaxInt(pDX, m_wakeMin, 0, 59);
	DDX_Control(pDX, IDC_WAKEMIN, m_editWakeMin);

	DDX_Text(pDX, IDC_WAKESEC, m_wakeSec);
	DDV_MinMaxInt(pDX, m_wakeSec, 0, 59);
	DDX_Control(pDX, IDC_WAKESEC, m_editWakeSec);

	// All the labels
	DDX_Control(pDX, IDC_LABEL_SLEEPFROM, m_labelSleepFrom);
	DDX_Control(pDX, IDC_LABEL_SLEEPTO,   m_labelSleepTo);
	DDX_Control(pDX, IDC_LABEL1, m_label1);
	DDX_Control(pDX, IDC_LABEL2, m_label2);
	DDX_Control(pDX, IDC_LABEL3, m_label3);
	DDX_Control(pDX, IDC_LABEL4, m_label4);
	DDX_Control(pDX, IDC_LABEL5, m_label5);
	DDX_Control(pDX, IDC_LABEL_RADIOID, m_labelRadioID);
}


BEGIN_MESSAGE_MAP(CConfigureCOMPortDlg, CPropertyPage)

	// Changing the serial parameters
	ON_CBN_SELCHANGE(IDC_COMPORT_COMBO, SaveData)
	ON_CBN_SELCHANGE(IDC_BAUDRATE_COMBO, SaveData)
	ON_CBN_SELCHANGE(IDC_HANDSHAKE_COMBO, SaveData)

	// The RadioID/Callbook number
	ON_EN_CHANGE(IDC_EDIT_RADIOID, SaveData)

	// The timeout
	ON_EN_CHANGE(IDC_TIMEOUT_EDIT, SaveData)

	// The connection interval
	ON_EN_CHANGE(IDC_INTERVAL_HR, SaveData)
	ON_EN_CHANGE(IDC_INTERVAL_MIN, SaveData)
	ON_EN_CHANGE(IDC_INTERVAL_SEC, SaveData)

	// The sleeping time
	ON_EN_CHANGE(IDC_SLEEPHR, SaveData)
	ON_EN_CHANGE(IDC_SLEEPMIN, SaveData)
	ON_EN_CHANGE(IDC_SLEEPSEC, SaveData)

	// The wake up time
	ON_EN_CHANGE(IDC_WAKEHR, SaveData)
	ON_EN_CHANGE(IDC_WAKEMIN, SaveData)
	ON_EN_CHANGE(IDC_WAKESEC, SaveData)

	// The FTP-Settings
	ON_EN_CHANGE(IDC_FTP_IPADDRESS, SaveData)
	ON_EN_CHANGE(IDC_FTP_USERNAME, SaveData)
	ON_EN_CHANGE(IDC_FTP_PASSWORD, SaveData)

	// The FTP/cable/radio radio-box
	ON_BN_CLICKED(IDC_RADIO_FTP, OnChangeMethod)
	ON_BN_CLICKED(IDC_RADIO_SERIAL, OnChangeMethod)
	ON_BN_CLICKED(IDC_RADIO_FREEWAVE_SERIAL, OnChangeMethod)
END_MESSAGE_MAP()


// CConfigureCOMPortDlg message handlers

BOOL ConfigurationDialog::CConfigureCOMPortDlg::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	CString port, bdr;
	Common common;

	// Default is to use FTP connection
	m_curSetting = 2;
	ShowFTP();

	/** Initialize the controls */
	for(int i = 1; i < 22; ++i){
		port.Format("COM%1d", i);
		m_comPort.AddString(port);
	}

	for(int i = 0; i < sizeof(m_availableBaudrates)/sizeof(int); ++i){
		bdr.Format("%d", m_availableBaudrates[i]);
		m_baudrate.AddString(bdr);
	}

	m_handshake.AddString("No");
	m_handshake.AddString("Hardware");

	// set the currently selected values
	OnChangeScanner();

	// set up the tool tips
	InitToolTips();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CConfigureCOMPortDlg::OnChangeScanner(){
	UpdateDlg();

	if(this->m_hWnd != NULL)
		OnChangeMethod();
}

void CConfigureCOMPortDlg::UpdateDlg(){
	CString bdr;
	int currentScanner, currentSpec;

	if(m_hWnd == NULL)
		return;

	HTREEITEM hTree = m_scannerTree->GetSelectedItem();
	GetScanAndSpec(currentScanner, currentSpec);

	if(currentSpec == -1){
		if((currentScanner == -1) || (m_configuration->scanner[currentScanner].specNum > 1))
		return;
		else{
		hTree = m_scannerTree->GetChildItem(hTree);
		currentSpec = 0;
		}
	}

	// --- Update the controls ---
	CConfigurationSetting::CommunicationSetting &comm = m_configuration->scanner[currentScanner].comm;

	// 1. The settings for the serial communication

	// 1a. The com-port
	m_comPort.SetCurSel(comm.port - 1);

	// 1b. The baudrate
	for(int i = 0; i < sizeof(m_availableBaudrates)/sizeof(int); ++i){
		if(m_availableBaudrates[i] == comm.baudrate){
		m_baudrate.SetCurSel(i);
		break;
		}
	}
	// 1c. Handshake
	m_handshake.SetCurSel(comm.flowControl);

	// 1d. The communication medium
	if(comm.connectionType == FTP_CONNECTION){
		m_curSetting = 2;
	}else{
		switch(comm.medium){
			case MEDIUM_CABLE: m_curSetting = 0; break;
			case MEDIUM_FREEWAVE_SERIAL_MODEM: m_curSetting = 1; break;
		}
	}

	// 2. The Freewave - radiomodem settings
	m_radioID.Format("%s", comm.radioID);
	
	// 3. The time out
	m_timeout = comm.timeout / 1000;

	// 4. The Query period
	int t = comm.queryPeriod;
	m_intervalHr  = (int)(t / 3600);
	m_intervalMin = (int)((t - m_intervalHr*3600) / 60);
	m_intervalSec = (int)(t % 60);

	// 5. sleep period 
	m_sleepHr   = comm.sleepTime.hour;
	m_sleepMin = comm.sleepTime.minute;
	m_sleepSec = comm.sleepTime.second;
	m_wakeHr   = comm.wakeupTime.hour;
	m_wakeMin  = comm.wakeupTime.minute;
	m_wakeSec  = comm.wakeupTime.second;

	// 6. Update the screen
	if(m_hWnd != NULL){
		UpdateData(FALSE);
	}

	// 7. The FTP-settings
	m_ftpPassword.Format("%s", comm.ftpPassword);
	m_ftpUserName.Format("%s", comm.ftpUserName);

	// 8. Update the screen
	if(m_hWnd != NULL){
		UpdateData(FALSE);
	}
	m_IPAddress.SetAddress(comm.ftpIP[0], comm.ftpIP[1], comm.ftpIP[2], comm.ftpIP[3]);
}

void CConfigureCOMPortDlg::OnChangeMethod(){
	if(m_hWnd != NULL)
		UpdateData(TRUE); // <-- Save the data in the dialog

	SaveData(); // <-- Save the remaining data in the dialog

	switch(m_curSetting){
		case 0:	ShowSerialCable(); break;
		case 1:	ShowFreewaveSerial(); break;
		case 2:	ShowFTP(); break;
		default: ShowFTP();
	}

	UpdateDlg();
}

void CConfigureCOMPortDlg::SaveData(){
	int currentScanner, currentSpec;

	if(m_hWnd == NULL)
		return;

	HTREEITEM hTree = m_scannerTree->GetSelectedItem();
	GetScanAndSpec(currentScanner, currentSpec);

	if(currentSpec == -1){
		return;
	}

	// ------- Save the change ----------- 
	if(UpdateData(TRUE))
	{
		CConfigurationSetting::CommunicationSetting &comm = m_configuration->scanner[currentScanner].comm;

		// 1. The serial settings

		// 1a. The com-port
		comm.port = m_comPort.GetCurSel() + 1;

		// 1b. The baudrate
		comm.baudrate = m_availableBaudrates[m_baudrate.GetCurSel()];

		// 1c. The Handshake
		comm.flowControl = m_handshake.GetCurSel();

		// 1d. The communication medium
		switch(m_curSetting){
			case 0:
				comm.connectionType = SERIAL_CONNECTION;
				comm.medium = MEDIUM_CABLE; break;
			case 1:
				comm.connectionType = SERIAL_CONNECTION;
				comm.medium = MEDIUM_FREEWAVE_SERIAL_MODEM; break;
			case 2:
				comm.connectionType = FTP_CONNECTION;
				break;
			default:
				comm.connectionType = FTP_CONNECTION;
				break;
		}

		// 2. The Freewave - radiomodem settings
		comm.radioID.Format("%s", m_radioID);
		
		// 3. The FTP-settings
		m_IPAddress.GetAddress(comm.ftpIP[0], comm.ftpIP[1], comm.ftpIP[2], comm.ftpIP[3]);
		comm.ftpPassword.Format("%s", m_ftpPassword);
		comm.ftpUserName.Format("%s", m_ftpUserName);

		// 4. The Timeout
		comm.timeout = m_timeout * 1000;

		// 5. The Query period
		comm.queryPeriod = (m_intervalHr * 3600 + m_intervalMin * 60 + m_intervalSec);

		// 6. The sleep period
		comm.sleepTime.hour			= m_sleepHr;
		comm.sleepTime.minute		= m_sleepMin;
		comm.sleepTime.second		= m_sleepSec;
		comm.wakeupTime.hour		= m_wakeHr;
		comm.wakeupTime.minute	= m_wakeMin;
		comm.wakeupTime.second	= m_wakeSec;

	}

}
BOOL ConfigurationDialog::CConfigureCOMPortDlg::PreTranslateMessage(MSG* pMsg){
	m_toolTip.RelayEvent(pMsg);

	return CPropertyPage::PreTranslateMessage(pMsg);
}

BOOL ConfigurationDialog::CConfigureCOMPortDlg::OnSetActive()
{
	UpdateDlg();

	return CSystemConfigurationPage::OnSetActive();
}


/** Showing the settings for connecting through a serial cable */
void CConfigureCOMPortDlg::ShowSerialCable(){
	// 1. Set the labels
	SetDlgItemText(IDC_LABEL1, "COM-Port");
	SetDlgItemText(IDC_LABEL2, "Baudrate");
	SetDlgItemText(IDC_LABEL3, "Handshake");
	SetDlgItemText(IDC_LABEL4, "Timeout");

	// 2. Show the controls we want to use
	m_comPort.ShowWindow(SW_SHOW);
	m_baudrate.ShowWindow(SW_SHOW);
	m_handshake.ShowWindow(SW_SHOW);
	m_editTimeOut.ShowWindow(SW_SHOW);
	m_label5.ShowWindow(SW_SHOW);

	// 3. Hide the controls we don't want to use
	m_IPAddress.ShowWindow(SW_HIDE);
	m_editUserName.ShowWindow(SW_HIDE);
	m_editPassword.ShowWindow(SW_HIDE);	
	m_editRadioID.ShowWindow(SW_HIDE);
	m_labelRadioID.ShowWindow(SW_HIDE);
}

/** Showing the settings for connecting using a Freewave serial modem */
void CConfigureCOMPortDlg::ShowFreewaveSerial(){
	// 1. Set the labels
	SetDlgItemText(IDC_LABEL1, "COM-Port");
	SetDlgItemText(IDC_LABEL2, "Baudrate");
	SetDlgItemText(IDC_LABEL3, "Handshake");
	SetDlgItemText(IDC_LABEL4, "Timeout");

	// 2. Show the controls we want to use
	m_comPort.ShowWindow(SW_SHOW);
	m_baudrate.ShowWindow(SW_SHOW);
	m_handshake.ShowWindow(SW_SHOW);
	m_editTimeOut.ShowWindow(SW_SHOW);
	m_label5.ShowWindow(SW_SHOW);
	m_editRadioID.ShowWindow(SW_SHOW);
	m_labelRadioID.ShowWindow(SW_SHOW);

	// 3. Hide the controls we don't want to use
	m_IPAddress.ShowWindow(SW_HIDE);
	m_editUserName.ShowWindow(SW_HIDE);
	m_editPassword.ShowWindow(SW_HIDE);
}

/** Showing the settings for connecting using FTP-Protocol*/
void CConfigureCOMPortDlg::ShowFTP(){
	// 1. Set the labels
	SetDlgItemText(IDC_LABEL1, "IP-Address");
	SetDlgItemText(IDC_LABEL2, "Username");
	SetDlgItemText(IDC_LABEL3, "Password");
	SetDlgItemText(IDC_LABEL4, "");

	// 2. Show the controls we want to use
	m_IPAddress.ShowWindow(SW_SHOW);
	m_editUserName.ShowWindow(SW_SHOW);
	m_editPassword.ShowWindow(SW_SHOW);

	// 3. Hide the controls we don't want to use
	m_comPort.ShowWindow(SW_HIDE);
	m_baudrate.ShowWindow(SW_HIDE);
	m_handshake.ShowWindow(SW_HIDE);
	m_editTimeOut.ShowWindow(SW_HIDE);
	m_label5.ShowWindow(SW_HIDE);
	m_editRadioID.ShowWindow(SW_HIDE);
	m_labelRadioID.ShowWindow(SW_HIDE);
}

void CConfigureCOMPortDlg::InitToolTips(){
	if(m_toolTip.m_hWnd != NULL)
		return;

	// Enable the tool tips
	if(!m_toolTip.Create(this)){
		TRACE0("Failed to create tooltip control\n"); 
	}
	m_toolTip.AddTool(&m_connectionIntervalLabel, IDC_INTERVAL_HR);
	m_toolTip.AddTool(&m_editSleepHr,						IDC_LABEL_SLEEPFROM);
	m_toolTip.AddTool(&m_editSleepMin,						IDC_LABEL_SLEEPFROM);
	m_toolTip.AddTool(&m_editSleepSec,						IDC_LABEL_SLEEPFROM);
	m_toolTip.AddTool(&m_labelSleepFrom,					IDC_LABEL_SLEEPFROM);
	m_toolTip.AddTool(&m_editWakeHr,						IDC_LABEL_SLEEPTO);
	m_toolTip.AddTool(&m_editWakeMin,						IDC_LABEL_SLEEPTO);
	m_toolTip.AddTool(&m_editWakeSec,						IDC_LABEL_SLEEPTO);
	m_toolTip.AddTool(&m_labelSleepTo,						IDC_LABEL_SLEEPTO);
	m_toolTip.AddTool(&m_comPort,							"The Serial Port to use");
	m_toolTip.AddTool(&m_baudrate,							"The Baudrate to use");
	m_toolTip.AddTool(&m_handshake,							"Which kind of handshake to use (default is Hardware)");
	m_toolTip.AddTool(&m_editTimeOut,						"The time-out for communication with this scanning instrument");
	m_toolTip.AddTool(&m_editRadioID,						"The radioID/Callbook number for this scanning instrument");
	m_toolTip.AddTool(&m_IPAddress,							"The IP-Address for the scanning instrument");
	m_toolTip.AddTool(&m_editUserName,						"The user name to log into the scanning instrument");
	m_toolTip.AddTool(&m_editPassword,						"The password to log into the scanning instrument");
	m_toolTip.SetMaxTipWidth(SHRT_MAX);
	m_toolTip.Activate(TRUE);
}