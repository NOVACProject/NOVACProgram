#include "stdafx.h"
#include "../NovacMasterProgram.h"
#include "WindConfigurationDlg.h"


// CWindConfigurationDlg dialog

using namespace ConfigurationDialog;


IMPLEMENT_DYNAMIC(CWindConfigurationDlg, CSystemConfigurationPage)
CWindConfigurationDlg::CWindConfigurationDlg()
	: CSystemConfigurationPage(CWindConfigurationDlg::IDD)
{
}

CWindConfigurationDlg::~CWindConfigurationDlg()
{
}

void CWindConfigurationDlg::DoDataExchange(CDataExchange* pDX)
{
	CSystemConfigurationPage::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_EDIT_INTERVAL,			m_editInterval);
	DDX_Control(pDX, IDC_EDIT_DURATION,			m_editDuration);
	DDX_Control(pDX, IDC_EDIT_MAXANGLE,			m_editMaxAngle);
	DDX_Control(pDX, IDC_EDIT_STABLEPERIOD,		m_editStablePeriod);
	DDX_Control(pDX, IDC_EDIT_MINPEAKCOLUMN,	m_editMinPeakColumn);

	DDX_Check(pDX, IDC_CHECK_DOWINDSPEED,		automatic);
	DDX_Text(pDX, IDC_EDIT_INTERVAL,			interval);
	DDX_Text(pDX, IDC_EDIT_DURATION,			duration);
	DDX_Text(pDX, IDC_EDIT_MAXANGLE,			maxAngle);
	DDX_Text(pDX, IDC_EDIT_STABLEPERIOD,		stablePeriod);
	DDX_Text(pDX, IDC_EDIT_MINPEAKCOLUMN,		minPeakColumn);

}


BEGIN_MESSAGE_MAP(CWindConfigurationDlg, CPropertyPage)
	ON_COMMAND(IDC_CHECK_DOWINDSPEED, OnOnOff)

	// immediately save all changes made in the dialog
	ON_EN_CHANGE(IDC_EDIT_INTERVAL, SaveData)
	ON_EN_CHANGE(IDC_EDIT_DURATION, SaveData)
	ON_EN_CHANGE(IDC_EDIT_MAXANGLE, SaveData)
	ON_EN_CHANGE(IDC_EDIT_STABLEPERIOD, SaveData)
	ON_EN_CHANGE(IDC_EDIT_MINPEAKCOLUMN, SaveData)

END_MESSAGE_MAP()


// CWindConfigurationDlg message handlers

BOOL CWindConfigurationDlg::OnInitDialog()
{
	CSystemConfigurationPage::OnInitDialog();

	// setup the tool tips
	InitToolTips();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CWindConfigurationDlg::InitToolTips(){
	// Don't initialize the tool tips twice
	if(m_toolTip.m_hWnd != NULL)
		return;

	// Enable the tool tips
	if(!m_toolTip.Create(this)){
		TRACE0("Failed to create tooltip control\n"); 
	}
//	m_toolTip.AddTool(&m_labelVolcano,							IDC_EDIT_VOLCANO);
//	m_toolTip.AddTool(&m_editSerial,								"The serial number of the spectrometer");

	m_toolTip.SetMaxTipWidth(SHRT_MAX);
	m_toolTip.Activate(TRUE);
}

BOOL CWindConfigurationDlg::PreTranslateMessage(MSG* pMsg)
{
	m_toolTip.RelayEvent(pMsg);

	return CPropertyPage::PreTranslateMessage(pMsg);
}

void CWindConfigurationDlg::OnChangeScanner(){
	if(UpdateData(TRUE)){ // <-- first save the data in the dialog

		// Then change the settings so that we're using the newly selected scanner
		CSystemConfigurationPage::OnChangeScanner();

		if(m_curScanner == NULL)
			return;

		CConfigurationSetting::WindSpeedMeasurementSetting &wind = m_curScanner->windSettings;

		this->interval	= wind.interval / 60;
		this->duration	= wind.duration / 60;
		this->maxAngle	= wind.maxAngle;
		this->stablePeriod=wind.stablePeriod;
		this->minPeakColumn= wind.minPeakColumn;
		this->automatic	= wind.automaticWindMeasurements;

		if(automatic){
			m_editDuration.EnableWindow(TRUE);
			m_editInterval.EnableWindow(TRUE);
			m_editMaxAngle.EnableWindow(TRUE);
			m_editStablePeriod.EnableWindow(TRUE);
			m_editMinPeakColumn.EnableWindow(TRUE);
		}else{
			m_editDuration.EnableWindow(FALSE);
			m_editInterval.EnableWindow(FALSE);
			m_editMaxAngle.EnableWindow(FALSE);
			m_editStablePeriod.EnableWindow(FALSE);
			m_editMinPeakColumn.EnableWindow(FALSE);
		}

		// Finally, update the screen to reflect the changes
		UpdateData(FALSE);
	}
}

void CWindConfigurationDlg::OnOnOff(){
	SaveData(); // <-- first save the data in the dialog
	if(automatic){
		m_editDuration.EnableWindow(TRUE);
		m_editInterval.EnableWindow(TRUE);
		m_editMaxAngle.EnableWindow(TRUE);
		m_editStablePeriod.EnableWindow(TRUE);
	}else{
		m_editDuration.EnableWindow(FALSE);
		m_editInterval.EnableWindow(FALSE);
		m_editMaxAngle.EnableWindow(FALSE);
		m_editStablePeriod.EnableWindow(FALSE);
	}
}

void CWindConfigurationDlg::SaveData(){
	if(UpdateData(TRUE)){ // <-- first save the data in the dialog

		if(m_curScanner == NULL)
			return;

		CConfigurationSetting::WindSpeedMeasurementSetting &wind = m_curScanner->windSettings;

		wind.interval		= interval * 60;
		wind.duration		= duration * 60;
		wind.maxAngle		= maxAngle;
		wind.stablePeriod	= stablePeriod;
		wind.minPeakColumn	= minPeakColumn;

		wind.automaticWindMeasurements = (automatic)? true : false;
	}
}

/** Called when the user has changed between using angles or distances */
void CWindConfigurationDlg::OnChangeAngleDistance(){
	if(UpdateData(TRUE)){ // <-- first save the data in the dialog
		if(useAngle == 0){
			m_editAngle.EnableWindow(TRUE);
			m_editDistance.EnableWindow(FALSE);
		}else{
			m_editAngle.EnableWindow(FALSE);
			m_editDistance.EnableWindow(TRUE);
		}
	}
}