// DarkSettingsDialog.cpp : implementation file
//

#include "stdafx.h"
#include "../NovacMasterProgram.h"
#include "DarkSettingsDialog.h"

using namespace Dialogs;

// CDarkSettingsDialog dialog

IMPLEMENT_DYNAMIC(CDarkSettingsDialog, CDialog)
CDarkSettingsDialog::CDarkSettingsDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CDarkSettingsDialog::IDD, pParent)
{
}

CDarkSettingsDialog::~CDarkSettingsDialog()
{
}

void CDarkSettingsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

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
	DDX_Control(pDX, IDC_LABEL_DARKOFFSETCORR,	m_labelDarkOffsetCorr);

	// The edit-boxes
	DDX_Control(pDX, IDC_EDIT_OFFSETSPECTRUM,					m_editOffset1);
	DDX_Control(pDX, IDC_EDIT_OFFSETSPECTRUM2,				m_editOffset2);
	DDX_Control(pDX, IDC_EDIT_DARKCURRENT_SPECTRUM,		m_editDC1);
	DDX_Control(pDX, IDC_EDIT_DARKCURRENT_SPECTRUM2,	m_editDC2);

	DDX_Text(pDX, IDC_EDIT_OFFSETSPECTRUM,				m_offsetPath1);
	DDX_Text(pDX, IDC_EDIT_OFFSETSPECTRUM2,				m_offsetPath2);
	DDX_Text(pDX, IDC_EDIT_DARKCURRENT_SPECTRUM,	m_dcPath1);
	DDX_Text(pDX, IDC_EDIT_DARKCURRENT_SPECTRUM2,	m_dcPath2);

	DDX_Text(pDX,	IDC_EDIT_USER_SUPPLIED_DARK_SPECTRUM,	m_darkSpectrum_UserSupplied);

	// The Combo-boxes
	DDX_Control(pDX, IDC_COMBO_OFFSET,					m_comboOffset);
	DDX_Control(pDX, IDC_COMBO_DARKCURRENT,			m_comboDC);

	// The buttons
	DDX_Control(pDX,	IDC_BUTTON_BROWSE_OFFSET,				m_btnMasterOff);
	DDX_Control(pDX,	IDC_BUTTON_BROWSE_OFFSET2,			m_btnSlaveOff);
	DDX_Control(pDX,	IDC_BUTTON_BROWSE_DARKCURRENT,	m_btnMasterDC);
	DDX_Control(pDX,	IDC_BUTTON_BROWSE_DARKCURRENT2,	m_btnSlaveDC);	
}


BEGIN_MESSAGE_MAP(CDarkSettingsDialog, CDialog)
	// Changing the edit-boxes
	ON_EN_CHANGE(IDC_EDIT_OFFSETSPECTRUM,					SaveData)
	ON_EN_CHANGE(IDC_EDIT_OFFSETSPECTRUM2,				SaveData)
	ON_EN_CHANGE(IDC_EDIT_DARKCURRENT_SPECTRUM,		SaveData)
	ON_EN_CHANGE(IDC_EDIT_DARKCURRENT_SPECTRUM2,	SaveData)

	// Changing the selection in the combo-boxes
	ON_CBN_SELCHANGE(IDC_COMBO_OFFSET,						SaveData)
	ON_CBN_SELCHANGE(IDC_COMBO_DARKCURRENT,				SaveData)

	// Pressing any of the buttons
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_OFFSET,				OnBrowseOffset1)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_OFFSET2,			OnBrowseOffset2)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_DARKCURRENT,	OnBrowseDC1)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_DARKCURRENT2,	OnBrowseDC2)

	// Changing the radio-buttons
	ON_BN_CLICKED(IDC_RADIO_MEASURE_DARK,					OnChangeDarkSpecOption)
	ON_BN_CLICKED(IDC_RADIO_MODEL_SOMETIMES,			OnChangeDarkSpecOption)
	ON_BN_CLICKED(IDC_RADIO_MODEL_ALWAYS,					OnChangeDarkSpecOption)
	ON_BN_CLICKED(IDC_RADIO_DARK_USER_SUPPLIED,		OnChangeDarkSpecOption)

END_MESSAGE_MAP()



// CDarkSettingsDialog message handlers

void CDarkSettingsDialog::OnCancel()
{
	// TODO: Add your specialized code here and/or call the base class

	CDialog::OnCancel();
}

void CDarkSettingsDialog::OnOK()
{
	// Save the changes to the dialog
	UpdateData(TRUE);

	// Save the changes the user has made
	if(this->m_darkSettings == NULL)
		return; // shouldn't happen...

	// Initialize the values of the dialog
	m_darkSettings->m_darkSpecOption			= (Configuration::DARK_SPEC_OPTION)m_darkSpecOption;
	m_darkSettings->m_darkCurrentOption			= (Configuration::DARK_MODEL_OPTION)m_dcSpecOption;
	m_darkSettings->m_offsetOption				= (Configuration::DARK_MODEL_OPTION)m_offsetSpecOption;

	// the files
	if(m_darkSettings->m_darkSpecOption	== Configuration::DARK_SPEC_OPTION::USER_SUPPLIED){
		m_darkSettings->m_offsetSpec = std::string((LPCSTR)m_darkSpectrum_UserSupplied);
	}else{
		m_darkSettings->m_offsetSpec = std::string((LPCSTR)m_offsetPath1);
		m_darkSettings->m_darkCurrentSpec = std::string((LPCSTR)m_dcPath1);
	}

	CDialog::OnOK();
}

BOOL CDarkSettingsDialog::OnInitDialog(){
  CDialog::OnInitDialog();

	// setup the tool tips
	InitToolTips();

	// Initialize the values
	if(this->m_darkSettings == NULL)
		return FALSE; // shouldn't happen...

	// Initialize the values of the dialog
	m_darkSpecOption				= (int)m_darkSettings->m_darkSpecOption;
	m_dcSpecOption					= (int)m_darkSettings->m_darkCurrentOption;
	m_offsetSpecOption				= (int)m_darkSettings->m_offsetOption;

	// the files
	if(m_darkSpecOption == (int)Configuration::DARK_SPEC_OPTION::USER_SUPPLIED){
		m_darkSpectrum_UserSupplied.Format("%s", m_darkSettings->m_offsetSpec.c_str());
	}else{
		m_offsetPath1.Format("%s", m_darkSettings->m_offsetSpec.c_str());
		m_dcPath1.Format("%s", m_darkSettings->m_darkCurrentSpec.c_str());
	}

	// Enable and disable the controls accoring to the settings
	UpdateControls();

	// Fill in the values in the dialog
	UpdateData(FALSE);

  return TRUE;  // return TRUE unless you set the focus to a control
  // EXCEPTION: OCX Property Pages should return FALSE
}

void CDarkSettingsDialog::InitToolTips(){
	// Don't initialize the tool tips twice
	if(m_toolTip.m_hWnd != NULL)
		return;

	// Enable the tool tips
  if(!m_toolTip.Create(this)){
    TRACE0("Failed to create tooltip control\n"); 
  }

//	m_toolTip.AddTool(&m_labelVolcano,							IDC_EDIT_VOLCANO);
}

void CDarkSettingsDialog::SaveData(){
	CString str;

	if(m_hWnd == NULL)
		return;

  // ------- Save the change ----------- 
	if(UpdateData(TRUE))
	{
	}
}

void CDarkSettingsDialog::UpdateDlg(){
	UpdateData(FALSE);
}

BOOL CDarkSettingsDialog::PreTranslateMessage(MSG* pMsg){
  m_toolTip.RelayEvent(pMsg);

  return CDialog::PreTranslateMessage(pMsg);
}


void CDarkSettingsDialog::UpdateControls(){
	// if there is only one channel on this spectrometer, then hide
	//	the edit-boxes and labels that belongs to the slave
	//if(m_curSpec->channelNum == 1){
	//	m_labelSlave1.ShowWindow(SW_HIDE);
	//	m_labelSlave2.ShowWindow(SW_HIDE);
	//	m_editOffset2.ShowWindow(SW_HIDE);
	//	m_editDC2.ShowWindow(SW_HIDE);
	//	m_btnSlaveOff.ShowWindow(SW_HIDE);
	//	m_btnSlaveDC.ShowWindow(SW_HIDE);
	//}else{
	//	m_labelSlave1.ShowWindow(SW_SHOW);
	//	m_labelSlave2.ShowWindow(SW_SHOW);
	//	m_editOffset2.ShowWindow(SW_SHOW);
	//	m_editDC2.ShowWindow(SW_SHOW);
	//	m_btnSlaveOff.ShowWindow(SW_SHOW);
	//	m_btnSlaveDC.ShowWindow(SW_SHOW);
	//}
	
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
void CDarkSettingsDialog::OnBrowseOffset1(){
	if(1 == BrowseFile(IDC_EDIT_OFFSETSPECTRUM))
		m_offsetSpecOption = (int)Configuration::DARK_SPEC_OPTION::USER_SUPPLIED;

	UpdateData(FALSE);
}
void CDarkSettingsDialog::OnBrowseOffset2(){
	if(1 == BrowseFile(IDC_EDIT_OFFSETSPECTRUM2))
		m_offsetSpecOption = (int)Configuration::DARK_SPEC_OPTION::USER_SUPPLIED;

	UpdateData(FALSE);
}
void CDarkSettingsDialog::OnBrowseDC1(){
	if(1 == BrowseFile(IDC_EDIT_DARKCURRENT_SPECTRUM))
		m_dcSpecOption = (int)Configuration::DARK_SPEC_OPTION::USER_SUPPLIED;

	UpdateData(FALSE);
}
void CDarkSettingsDialog::OnBrowseDC2(){
	if(1 == BrowseFile(IDC_EDIT_DARKCURRENT_SPECTRUM2))
		m_dcSpecOption = (int)Configuration::DARK_SPEC_OPTION::USER_SUPPLIED;

	UpdateData(FALSE);
}

/** Browsing for a file, the result will be saved in the edit box 'editbox' */
int	CDarkSettingsDialog::BrowseFile(int editBox){
  CString fileName;
  TCHAR filter[512];
  int n = _stprintf(filter, "Spectrum files\0");
  n += _stprintf(filter + n + 1, "*.txt;*.xs;*.std\0");
  filter[n + 2] = 0;
  Common common;

  // Let the user browse for the file
	fileName.Format("");
  if(!common.BrowseForFile(filter, fileName)){
    return 0;
  }

	// Update the window
	SetDlgItemText(editBox, fileName);

	return 1;
}

void CDarkSettingsDialog::OnChangeDarkSpecOption(){
	if(UpdateData(TRUE)){ // <-- first save the data in the dialog

		// Enable the controls that should be enabled and disable the controls
		//	that should be disabled
		UpdateControls();

		// Save the data in the dialog
		SaveData();
	}
}