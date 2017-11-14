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

	// The special controls for the Heidelberg-instrument
	DDX_Radio(pDX,   IDC_RADIO_ANGLE1,			useAngle);
	DDX_Control(pDX, IDC_EDIT_ANGLE,			m_editAngle);
	DDX_Control(pDX, IDC_EDIT_DISTANCE,			m_editDistance);
	DDX_Control(pDX, IDC_EDIT_SWITCHRANGE,      m_editSwitchRange);
	DDX_Control(pDX, IDC_LABEL_SWITCHRANGE,     m_labelSwitchRange);
	DDX_Control(pDX, IDC_RADIO_ANGLE1,			m_radioAngle);
	DDX_Control(pDX, IDC_RADIO_ANGLE2,			m_radioDistance);
	DDX_Control(pDX, IDC_LABEL_DEGREES,			m_labelDeg);
	DDX_Control(pDX, IDC_LABEL_METERS,			m_labelMeters);
	DDX_Control(pDX, IDC_FRAME_DISTANCE,		m_frameDistance);
	DDX_Control(pDX, IDC_CHECK_USE_AUTOMATIC_PLUMEPARAM2, m_useCalcWind);
	DDX_Check(pDX, IDC_CHECK_USE_AUTOMATIC_PLUMEPARAM2, useCalcWind);
	DDX_Text(pDX,	IDC_EDIT_ANGLE,				desiredAngle);
	DDX_Text(pDX,	IDC_EDIT_DISTANCE,			desiredDistance);
	DDX_Text(pDX,   IDC_EDIT_SWITCHRANGE,       desiredSwitchRange);
}


BEGIN_MESSAGE_MAP(CWindConfigurationDlg, CPropertyPage)
	ON_COMMAND(IDC_CHECK_DOWINDSPEED, OnOnOff)

	// immediately save all changes made in the dialog
	ON_EN_CHANGE(IDC_EDIT_INTERVAL, SaveData)
	ON_EN_CHANGE(IDC_EDIT_DURATION, SaveData)
	ON_EN_CHANGE(IDC_EDIT_MAXANGLE, SaveData)
	ON_EN_CHANGE(IDC_EDIT_STABLEPERIOD, SaveData)
	ON_EN_CHANGE(IDC_EDIT_MINPEAKCOLUMN, SaveData)
	ON_EN_CHANGE(IDC_EDIT_ANGLE, SaveData)
	ON_EN_CHANGE(IDC_EDIT_DISTANCE, SaveData)
	ON_EN_CHANGE(IDC_EDIT_SWITCHRANGE, SaveData)

	ON_BN_CLICKED(IDC_RADIO_ANGLE1, OnChangeAngleDistance)
	ON_BN_CLICKED(IDC_RADIO_ANGLE2, OnChangeAngleDistance)
	ON_BN_CLICKED(IDC_CHECK_USE_AUTOMATIC_PLUMEPARAM2, SaveData)

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

		if(m_curScanner->instrumentType == INSTR_HEIDELBERG){
			useCalcWind = wind.useCalculatedPlumeParameters;
			if(wind.desiredAngle > 0){
				useAngle			= 0;
				desiredAngle		= wind.desiredAngle;
				desiredDistance		= 0.0;
			}else{
				useAngle			= 1;
				desiredAngle		= 0.0;
				desiredDistance		= -wind.desiredAngle;
			}
			desiredSwitchRange = wind.SwitchRange;
		}

		if(automatic){
			m_editDuration.EnableWindow(TRUE);
			m_editInterval.EnableWindow(TRUE);
			m_editMaxAngle.EnableWindow(TRUE);
			m_editStablePeriod.EnableWindow(TRUE);
			m_editMinPeakColumn.EnableWindow(TRUE);

			if(m_curScanner->instrumentType == INSTR_HEIDELBERG){
				m_editDistance.ShowWindow(SW_SHOW);
				m_editDistance.EnableWindow(TRUE);
				m_editAngle.ShowWindow(SW_SHOW);
				m_labelDeg.ShowWindow(SW_SHOW);
				if(useAngle == 0){
					m_editAngle.EnableWindow(TRUE);
					m_editDistance.EnableWindow(FALSE);
				}else{
					m_editAngle.EnableWindow(FALSE);
					m_editDistance.EnableWindow(TRUE);
				}
				m_labelMeters.ShowWindow(SW_SHOW);
				m_labelMeters.EnableWindow(TRUE);
				m_radioDistance.ShowWindow(SW_SHOW);
				m_radioDistance.EnableWindow(TRUE);
				m_radioAngle.ShowWindow(SW_SHOW);
				m_radioAngle.EnableWindow(TRUE);
				m_useCalcWind.ShowWindow(SW_SHOW);
				m_useCalcWind.EnableWindow(TRUE);
				m_editSwitchRange.ShowWindow(SW_SHOW);
				m_editSwitchRange.EnableWindow(TRUE);
				m_labelSwitchRange.ShowWindow(SW_SHOW);
				m_labelSwitchRange.EnableWindow(TRUE);
				m_frameDistance.ShowWindow(SW_SHOW);
			}else{
				m_editDistance.ShowWindow(SW_HIDE);
				m_editAngle.ShowWindow(SW_HIDE);
				m_labelDeg.ShowWindow(SW_HIDE);
				m_labelMeters.ShowWindow(SW_HIDE);
				m_radioDistance.ShowWindow(SW_HIDE);
				m_radioAngle.ShowWindow(SW_HIDE);
				m_frameDistance.ShowWindow(SW_HIDE);
				m_useCalcWind.ShowWindow(SW_HIDE);
				m_editSwitchRange.ShowWindow(SW_HIDE);
				m_labelSwitchRange.ShowWindow(SW_HIDE);
			}
		}else{
			m_editDuration.EnableWindow(FALSE);
			m_editInterval.EnableWindow(FALSE);
			m_editMaxAngle.EnableWindow(FALSE);
			m_editStablePeriod.EnableWindow(FALSE);
			m_editMinPeakColumn.EnableWindow(FALSE);

			if(m_curScanner->instrumentType == INSTR_HEIDELBERG){
				m_editDistance.ShowWindow(SW_SHOW);
				m_editDistance.EnableWindow(FALSE);
				m_editAngle.ShowWindow(SW_SHOW);
				m_editAngle.EnableWindow(FALSE);
				m_labelDeg.ShowWindow(SW_SHOW);
				m_labelDeg.EnableWindow(FALSE);
				m_labelMeters.ShowWindow(SW_SHOW);
				m_labelMeters.EnableWindow(FALSE);
				m_radioDistance.ShowWindow(SW_SHOW);
				m_radioDistance.EnableWindow(FALSE);
				m_radioAngle.ShowWindow(SW_SHOW);
				m_radioAngle.EnableWindow(FALSE);
				m_frameDistance.ShowWindow(SW_SHOW);
				m_useCalcWind.ShowWindow(SW_SHOW);
				m_useCalcWind.EnableWindow(FALSE);
				m_editSwitchRange.ShowWindow(SW_SHOW);
				m_editSwitchRange.EnableWindow(FALSE);
				m_labelSwitchRange.ShowWindow(SW_SHOW);
				m_labelSwitchRange.EnableWindow(FALSE);
			}else{
				m_editDistance.ShowWindow(SW_HIDE);
				m_editAngle.ShowWindow(SW_HIDE);
				m_labelDeg.ShowWindow(SW_HIDE);
				m_labelMeters.ShowWindow(SW_HIDE);
				m_radioDistance.ShowWindow(SW_HIDE);
				m_radioAngle.ShowWindow(SW_HIDE);
				m_frameDistance.ShowWindow(SW_HIDE);
				m_useCalcWind.ShowWindow(SW_HIDE);
				m_editSwitchRange.ShowWindow(SW_HIDE);
				m_labelSwitchRange.ShowWindow(SW_HIDE);
			}
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

		if(m_curScanner->instrumentType == INSTR_HEIDELBERG){
			if(useAngle == 0){
				m_editAngle.EnableWindow(TRUE);
				m_editDistance.EnableWindow(FALSE);
			}else{
				m_editAngle.EnableWindow(FALSE);
				m_editDistance.EnableWindow(TRUE);
			}
			m_labelDeg.EnableWindow(TRUE);
			m_labelMeters.EnableWindow(TRUE);
			m_radioDistance.EnableWindow(TRUE);
			m_radioAngle.EnableWindow(TRUE);
			m_useCalcWind.EnableWindow(TRUE);
			m_editSwitchRange.EnableWindow(TRUE);
			m_labelSwitchRange.EnableWindow(TRUE);
		}
	}else{
		m_editDuration.EnableWindow(FALSE);
		m_editInterval.EnableWindow(FALSE);
		m_editMaxAngle.EnableWindow(FALSE);
		m_editStablePeriod.EnableWindow(FALSE);

		if(m_curScanner->instrumentType == INSTR_HEIDELBERG){
			m_editAngle.EnableWindow(FALSE);
			m_editDistance.EnableWindow(FALSE);
			m_labelDeg.EnableWindow(FALSE);
			m_labelMeters.EnableWindow(FALSE);
			m_radioDistance.EnableWindow(FALSE);
			m_radioAngle.EnableWindow(FALSE);
			m_useCalcWind.EnableWindow(FALSE);
			m_editSwitchRange.EnableWindow(FALSE);
			m_labelSwitchRange.EnableWindow(FALSE);
		}
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


		if(m_curScanner->instrumentType == INSTR_HEIDELBERG){
			wind.useCalculatedPlumeParameters = useCalcWind;
			if(useAngle == 0){
				wind.desiredAngle = desiredAngle;
			}else{
				wind.desiredAngle = -desiredDistance;
			}
			wind.SwitchRange = desiredSwitchRange;
		}
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