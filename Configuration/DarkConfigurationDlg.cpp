#include "stdafx.h"
#include "../NovacMasterProgram.h"
#include "DarkConfigurationDlg.h"

using namespace ConfigurationDialog;

// CDarkConfigurationDlg dialog

IMPLEMENT_DYNAMIC(CDarkConfigurationDlg, CSystemConfigurationPage)
CDarkConfigurationDlg::CDarkConfigurationDlg()
	: CSystemConfigurationPage(CDarkConfigurationDlg::IDD)
{
	m_configuration = NULL;
	m_scannerTree = NULL;
	m_parent = NULL;

	m_initialized = false;
}

CDarkConfigurationDlg::~CDarkConfigurationDlg()
{
	m_configuration = NULL;
	m_scannerTree = NULL;
	m_parent = NULL;
}

void CDarkConfigurationDlg::DoDataExchange(CDataExchange* pDX)
{
	CSystemConfigurationPage::DoDataExchange(pDX);

	// The radio
	DDX_Radio(pDX,	IDC_RADIO_MEASURE_DARK,		m_darkSpecOption);

	// The values of the combo-boxes
	DDX_CBIndex(pDX,	 IDC_COMBO_DARKCURRENT,		m_dcSpecOption);
	DDX_CBIndex(pDX,	 IDC_COMBO_OFFSET,				m_offsetSpecOption);

	// The labels
	DDX_Control(pDX, IDC_LABEL_MASTER1,					m_labelMaster1);
	DDX_Control(pDX, IDC_LABEL_MASTER2,					m_labelMaster2);
	DDX_Control(pDX, IDC_LABEL_SLAVE1,					m_labelSlave1);
	DDX_Control(pDX, IDC_LABEL_SLAVE2,					m_labelSlave2);
	DDX_Control(pDX, IDC_LABEL_DARKOFFSETCORR,			m_labelDarkOffsetCorr);

	// The edit-boxes
	DDX_Control(pDX, IDC_EDIT_OFFSETSPECTRUM,			m_editOffset1);
	DDX_Control(pDX, IDC_EDIT_OFFSETSPECTRUM2,			m_editOffset2);
	DDX_Control(pDX, IDC_EDIT_DARKCURRENT_SPECTRUM,		m_editDC1);
	DDX_Control(pDX, IDC_EDIT_DARKCURRENT_SPECTRUM2,	m_editDC2);

	DDX_Text(pDX, IDC_EDIT_OFFSETSPECTRUM,				m_offsetPath1);
	DDX_Text(pDX, IDC_EDIT_OFFSETSPECTRUM2,				m_offsetPath2);
	DDX_Text(pDX, IDC_EDIT_DARKCURRENT_SPECTRUM,		m_dcPath1);
	DDX_Text(pDX, IDC_EDIT_DARKCURRENT_SPECTRUM2,		m_dcPath2);

	// The Combo-boxes
	DDX_Control(pDX, IDC_COMBO_OFFSET,					m_comboOffset);
	DDX_Control(pDX, IDC_COMBO_DARKCURRENT,				m_comboDC);

	// The buttons
	DDX_Control(pDX,	IDC_BUTTON_BROWSE_OFFSET,		m_btnMasterOff);
	DDX_Control(pDX,	IDC_BUTTON_BROWSE_OFFSET2,		m_btnSlaveOff);
	DDX_Control(pDX,	IDC_BUTTON_BROWSE_DARKCURRENT,	m_btnMasterDC);
	DDX_Control(pDX,	IDC_BUTTON_BROWSE_DARKCURRENT2,	m_btnSlaveDC);	
}


BEGIN_MESSAGE_MAP(CDarkConfigurationDlg, CPropertyPage)
	// Changing the edit-boxes
	ON_EN_CHANGE(IDC_EDIT_OFFSETSPECTRUM,			SaveData)
	ON_EN_CHANGE(IDC_EDIT_OFFSETSPECTRUM2,			SaveData)
	ON_EN_CHANGE(IDC_EDIT_DARKCURRENT_SPECTRUM,		SaveData)
	ON_EN_CHANGE(IDC_EDIT_DARKCURRENT_SPECTRUM2,	SaveData)

	// Changing the selection in the combo-boxes
	ON_CBN_SELCHANGE(IDC_COMBO_OFFSET,				SaveData)
	ON_CBN_SELCHANGE(IDC_COMBO_DARKCURRENT,			SaveData)

	// Pressing any of the buttons
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_OFFSET,			OnBrowseOffset1)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_OFFSET2,		OnBrowseOffset2)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_DARKCURRENT,	OnBrowseDC1)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_DARKCURRENT2,	OnBrowseDC2)

	// Changing the radio-buttons
	ON_BN_CLICKED(IDC_RADIO_MEASURE_DARK,			OnChangeDarkSpecOption)
	ON_BN_CLICKED(IDC_RADIO_MODEL_SOMETIMES,		OnChangeDarkSpecOption)
	ON_BN_CLICKED(IDC_RADIO_MODEL_ALWAYS,			OnChangeDarkSpecOption)
END_MESSAGE_MAP()


// CDarkConfigurationDlg message handlers

BOOL CDarkConfigurationDlg::OnInitDialog(){
	CDialog::OnInitDialog();

	// setup the tool tips
	InitToolTips();

	m_initialized = true;

	// Update the variables
	OnChangeScanner();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDarkConfigurationDlg::InitToolTips(){
	// Don't initialize the tool tips twice
	if(m_toolTip.m_hWnd != NULL)
		return;

	// Enable the tool tips
	if(!m_toolTip.Create(this)){
		TRACE0("Failed to create tooltip control\n"); 
	}

//	m_toolTip.AddTool(&m_labelVolcano, IDC_EDIT_VOLCANO);
}

BOOL CDarkConfigurationDlg::OnKillActive(){

	return CPropertyPage::OnKillActive();
}

void CDarkConfigurationDlg::SaveData(){
	int i;
	CString str;

	if(m_hWnd == NULL)
		return;

	// ------- Save the change ----------- 
	if(UpdateData(TRUE))
	{
		for(i = 0; i < m_curSpec->channelNum; ++i){
			// The option for how to get the dark-spectrum
			m_curSpec->channel[i].m_darkSettings.m_darkSpecOption = (DARK_SPEC_OPTION)m_darkSpecOption;

			// The option for how to get the offset-spectrum
			m_curSpec->channel[i].m_darkSettings.m_offsetOption			= (DARK_MODEL_OPTION)m_offsetSpecOption;

			// The option for how to get the dark-current spectrum
			m_curSpec->channel[i].m_darkSettings.m_darkCurrentOption = (DARK_MODEL_OPTION)m_dcSpecOption;
		}

		// The path of the offset spectrum
		m_curSpec->channel[0].m_darkSettings.m_offsetSpec.Format(m_offsetPath1);
		if(m_curSpec->channelNum > 1){
			m_curSpec->channel[1].m_darkSettings.m_offsetSpec.Format(m_offsetPath2);
		}

		// The path of the dark-current spectrum
		m_curSpec->channel[0].m_darkSettings.m_darkCurrentSpec.Format(m_dcPath1);
		if(m_curSpec->channelNum > 1){
			m_curSpec->channel[1].m_darkSettings.m_darkCurrentSpec.Format(m_dcPath2);
		}

		// Show or hide the label telling that the user must himself offset correct the
		//	dark current spectrum
		if(m_curSpec->channel[0].m_darkSettings.m_darkCurrentOption	== USER_SUPPLIED){
			m_labelDarkOffsetCorr.ShowWindow(SW_SHOW);
		}else{
			m_labelDarkOffsetCorr.ShowWindow(SW_HIDE);
		}
	}
}

void CDarkConfigurationDlg::UpdateDlg(){
	UpdateData(FALSE);
}

BOOL CDarkConfigurationDlg::PreTranslateMessage(MSG* pMsg){
	m_toolTip.RelayEvent(pMsg);

	return CPropertyPage::PreTranslateMessage(pMsg);
}

void CDarkConfigurationDlg::OnChangeScanner(){
	if(!m_initialized)
		return;

	if(m_hWnd == NULL)
		return; // happens if this tab just disappeared

	if(UpdateData(TRUE)){ // <-- first save the data in the dialog

		// Then change the settings so that we're using the newly selected scanner
		CSystemConfigurationPage::OnChangeScanner();

		if(m_curScanner == NULL)
			return;
	
		// The option for how to get the dark-spectrum
		m_darkSpecOption		= (int)m_curSpec->channel[0].m_darkSettings.m_darkSpecOption;

		// The option for how to use the offset spectrum
		m_offsetSpecOption	= (int)m_curSpec->channel[0].m_darkSettings.m_offsetOption;

		// The option for how to use the dark-current spectrum
		m_dcSpecOption			= (int)m_curSpec->channel[0].m_darkSettings.m_darkCurrentOption;

		// Show the path of the offset spectrum
		if(strlen(m_curSpec->channel[0].m_darkSettings.m_offsetSpec) > 1){
			m_offsetPath1.Format(m_curSpec->channel[0].m_darkSettings.m_offsetSpec);
		}
		if(strlen(m_curSpec->channel[1].m_darkSettings.m_offsetSpec) > 1 && m_curSpec->channelNum > 1){
			m_offsetPath2.Format(m_curSpec->channel[1].m_darkSettings.m_offsetSpec);
		}

		// Show the path of the dark-current spectrum
		if(strlen(m_curSpec->channel[0].m_darkSettings.m_darkCurrentSpec) > 1){
			m_dcPath1.Format(m_curSpec->channel[0].m_darkSettings.m_darkCurrentSpec);
		}
		if(strlen(m_curSpec->channel[1].m_darkSettings.m_darkCurrentSpec) > 1 && m_curSpec->channelNum > 1){
			m_dcPath2.Format(m_curSpec->channel[1].m_darkSettings.m_darkCurrentSpec);
		}

		// Enable the controls that should be enabled and disable the controls
		//	that should be disabled
		UpdateControls();

		// Finally, update the screen
		UpdateData(FALSE);
	}
}

void CDarkConfigurationDlg::OnChangeDarkSpecOption(){
	if(!m_initialized)
		return;

	if(UpdateData(TRUE)){ // <-- first save the data in the dialog

		// Enable the controls that should be enabled and disable the controls
		//	that should be disabled
		UpdateControls();

		// Save the data in the dialog
		SaveData();
	}
}

void CDarkConfigurationDlg::UpdateControls(){
	// if there is only one channel on this spectrometer, then hide
	//	the edit-boxes and labels that belongs to the slave
	if(m_curSpec->channelNum == 1){
		m_labelSlave1.ShowWindow(SW_HIDE);
		m_labelSlave2.ShowWindow(SW_HIDE);
		m_editOffset2.ShowWindow(SW_HIDE);
		m_editDC2.ShowWindow(SW_HIDE);
		m_btnSlaveOff.ShowWindow(SW_HIDE);
		m_btnSlaveDC.ShowWindow(SW_HIDE);
	}else{
		m_labelSlave1.ShowWindow(SW_SHOW);
		m_labelSlave2.ShowWindow(SW_SHOW);
		m_editOffset2.ShowWindow(SW_SHOW);
		m_editDC2.ShowWindow(SW_SHOW);
		m_btnSlaveOff.ShowWindow(SW_SHOW);
		m_btnSlaveDC.ShowWindow(SW_SHOW);
	}
	
	// If the user only wants to use measured spectra, then
	//	the controls on the screed should be disabled
	BOOL state = TRUE;
	if(m_darkSpecOption == 0){
		state = FALSE;
	}else{
		state = TRUE;
	}
	m_labelMaster1.EnableWindow(state);
	m_labelMaster2.EnableWindow(state);
	m_labelSlave1.EnableWindow(state);
	m_labelSlave2.EnableWindow(state);

	/** The edit-boxes */
	m_editOffset1.EnableWindow(state);
	m_editOffset2.EnableWindow(state);
	m_editDC1.EnableWindow(state);
	m_editDC2.EnableWindow(state);

	/** The combo-boxes */
	m_comboOffset.EnableWindow(state);
	m_comboDC.EnableWindow(state);

	/** The buttons */
	m_btnMasterOff.EnableWindow(state);
	m_btnSlaveOff.EnableWindow(state);
	m_btnMasterDC.EnableWindow(state);
	m_btnSlaveDC.EnableWindow(state);
}

// The 'Browse' buttons
void CDarkConfigurationDlg::OnBrowseOffset1(){
	BrowseFile(IDC_EDIT_OFFSETSPECTRUM);
}
void CDarkConfigurationDlg::OnBrowseOffset2(){
	BrowseFile(IDC_EDIT_OFFSETSPECTRUM2);
}
void CDarkConfigurationDlg::OnBrowseDC1(){
	BrowseFile(IDC_EDIT_DARKCURRENT_SPECTRUM);
}
void CDarkConfigurationDlg::OnBrowseDC2(){
	BrowseFile(IDC_EDIT_DARKCURRENT_SPECTRUM2);
}

/** Browsing for a file, the result will be saved in the edit box 'editbox' */
void CDarkConfigurationDlg::BrowseFile(int editBox){
	CString fileName;
	TCHAR filter[512];
	int n = _stprintf(filter, "Spectrum files\0");
	n += _stprintf(filter + n + 1, "*.txt;*.xs;*.std\0");
	filter[n + 2] = 0;
	Common common;

	if(m_curSpec == NULL)
		return;
	
	// Let the user browse for the file
	fileName.Format("");
	if(!common.BrowseForFile(filter, fileName)){
		return;
	}

	// Update the window
	SetDlgItemText(editBox, fileName);
}
