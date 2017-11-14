#include "stdafx.h"
#include "../NovacMasterProgram.h"
#include "VIIAdvancedConfigurationDlg.h"


// CVIIAdvancedConfigurationDlg dialog

using namespace ConfigurationDialog;

IMPLEMENT_DYNAMIC(CVIIAdvancedConfigurationDlg, CSystemConfigurationPage)
CVIIAdvancedConfigurationDlg::CVIIAdvancedConfigurationDlg()
	: CSystemConfigurationPage(CVIIAdvancedConfigurationDlg::IDD)
{
}

CVIIAdvancedConfigurationDlg::~CVIIAdvancedConfigurationDlg()
{
}

void CVIIAdvancedConfigurationDlg::DoDataExchange(CDataExchange* pDX)
{
	CSystemConfigurationPage::DoDataExchange(pDX);

	// Dialog data
	DDX_Check(pDX,		IDC_CHECK_ENABLE_REALTIME_CFG,			m_automaticSetupChange);
	DDX_Check(pDX,		IDC_CHECK_USE_AUTOMATIC_PLUMEPARAM,		m_useCalculatedPlumeParameters);
	DDX_Text(pDX,		IDC_EDIT_WD_TOLERANCE,					m_windDirectionTolerance);
	DDX_Radio(pDX,		IDC_RADIO_MODE1,						m_mode);

	// Dialog components
	DDX_Control(pDX, IDC_EDIT_WD_TOLERANCE,						m_editTolerance);
	DDX_Control(pDX, IDC_RADIO_MODE1,							m_radioMode1);
	DDX_Control(pDX, IDC_RADIO_MODE2,							m_radioMode2);
	DDX_Control(pDX, IDC_CHECK_USE_AUTOMATIC_PLUMEPARAM,		m_checkUseAutoValues);
}


BEGIN_MESSAGE_MAP(CVIIAdvancedConfigurationDlg, CPropertyPage)
	ON_EN_CHANGE(IDC_EDIT_WD_TOLERANCE,					SaveData)
	ON_BN_CLICKED(IDC_CHECK_USE_AUTOMATIC_PLUMEPARAM,	SaveData)
	ON_BN_CLICKED(IDC_RADIO_MODE1,						SaveData)
	ON_BN_CLICKED(IDC_RADIO_MODE2,						SaveData)

	ON_BN_CLICKED(IDC_CHECK_ENABLE_REALTIME_CFG,		OnOnOff)
END_MESSAGE_MAP()


// CVIIAdvancedConfigurationDlg message handlers

BOOL CVIIAdvancedConfigurationDlg::OnInitDialog()
{
	CSystemConfigurationPage::OnInitDialog();

	// setup the tool tips
	InitToolTips();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CVIIAdvancedConfigurationDlg::InitToolTips(){
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

BOOL CVIIAdvancedConfigurationDlg::PreTranslateMessage(MSG* pMsg)
{
	m_toolTip.RelayEvent(pMsg);

	return CPropertyPage::PreTranslateMessage(pMsg);
}

void CVIIAdvancedConfigurationDlg::OnChangeScanner(){
	if(UpdateData(TRUE)){ // <-- first save the data in the dialog

		// Then change the settings so that we're using the newly selected scanner
		CSystemConfigurationPage::OnChangeScanner();

		if(m_curScanner == NULL)
			return;

		CConfigurationSetting::SetupChangeSetting &scs = m_curScanner->scSettings;

		this->m_automaticSetupChange			= scs.automaticSetupChange;
		this->m_useCalculatedPlumeParameters	= scs.useCalculatedPlumeParameters;
		this->m_windDirectionTolerance			= scs.windDirectionTolerance;
		this->m_mode							= scs.mode;


		// enable the controls that should be enabled and disable the controls that should be disabled
		BOOL show = (m_automaticSetupChange == 0) ? FALSE : TRUE;
		m_editTolerance.EnableWindow(show);
		m_radioMode1.EnableWindow(show);
		m_radioMode2.EnableWindow(show);
		m_checkUseAutoValues.EnableWindow(show);

		// Finally, update the screen to reflect the changes
		UpdateData(FALSE);
	}
}

void CVIIAdvancedConfigurationDlg::OnOnOff(){
	SaveData(); // <-- first save the data in the dialog

	BOOL show = (m_automaticSetupChange == 0) ? FALSE: TRUE;
	m_editTolerance.EnableWindow(show);
	m_radioMode1.EnableWindow(show);
	m_radioMode2.EnableWindow(show);
	m_checkUseAutoValues.EnableWindow(show);
}

void CVIIAdvancedConfigurationDlg::SaveData(){
	if(UpdateData(TRUE)){ // <-- first save the data in the dialog

		if(m_curScanner == NULL)
			return;

		CConfigurationSetting::SetupChangeSetting &scs = m_curScanner->scSettings;

		scs.automaticSetupChange					= this->m_automaticSetupChange;
		scs.useCalculatedPlumeParameters			= this->m_useCalculatedPlumeParameters;
		scs.windDirectionTolerance					= this->m_windDirectionTolerance;
		scs.mode									= this->m_mode;
	}
}