// LocationConfigurationDlg.cpp : implementation file
//

#include "stdafx.h"
#include "../NovacMasterProgram.h"
#include "LocationConfigurationDlg.h"
#include "ScannerConfiguration.h"
#include "../VolcanoInfo.h"
#include "../ObservatoryInfo.h"
#include "../Common/SpectrometerModel.h"

#include "../Dialogs/QueryStringDialog.h"

using namespace ConfigurationDialog;

extern CVolcanoInfo			g_volcanoes;
//extern CObservatoryInfo g_observatories;

// CLocationConfigurationDlg dialog

IMPLEMENT_DYNAMIC(CLocationConfigurationDlg, CSystemConfigurationPage)

CLocationConfigurationDlg::CLocationConfigurationDlg()
	: CSystemConfigurationPage(CLocationConfigurationDlg::IDD)
{
	m_configuration = NULL;
	m_scannerTree = NULL;
	m_parent = NULL;
}

CLocationConfigurationDlg::~CLocationConfigurationDlg()
{
	m_configuration = NULL;
	m_scannerTree = NULL;
	m_parent = NULL;
}

void CLocationConfigurationDlg::DoDataExchange(CDataExchange* pDX)
{	CSystemConfigurationPage::DoDataExchange(pDX);

	// The labels (only used for showing tooltips);
	DDX_Control(pDX, IDC_LABEL_VOLCANO,					m_labelVolcano);
	DDX_Control(pDX, IDC_LABEL_SITE,					m_labelSite);
	DDX_Control(pDX, IDC_LABEL_OBSERVATORY,				m_labelObservatory);

	// The edits (only used for showing tooltips);
	DDX_Control(pDX, IDC_EDIT_SITE,						m_editSite);
	DDX_Control(pDX, IDC_EDIT_OBSERVATORY,				m_editObservatory);
	DDX_Control(pDX, IDC_EDIT_SERIALNUMBER,				m_editSerial);

	DDX_Control(pDX, IDC_EDIT_STEPSPERROUND1,			m_editSPR1);
	DDX_Control(pDX, IDC_EDIT_STEPSPERROUND2,			m_editSPR2);
	DDX_Control(pDX, IDC_EDIT_MOTORSTEPSCOMPENSATION1,	m_editMSC1);
	DDX_Control(pDX, IDC_EDIT_MOTORSTEPSCOMPENSATION2,	m_editMSC2);
	DDX_Control(pDX, IDC_LABEL_STEPSPERROUND,			m_labelSPR);

	// The combo boxes
	DDX_Control(pDX, IDC_COMBO_VOLCANO,					m_comboVolcano);
	DDX_Control(pDX, IDC_COMBO_ELECTRONICS,				m_comboElectronics);
	DDX_Control(pDX, IDC_COMBO_INSTRUMENTTYPE,			m_comboInstrumentType);
	DDX_Control(pDX, IDC_COMBO_SPECTROMETERMODEL,		m_comboSpectrometerModel);
	DDX_Control(pDX, IDC_COMBO_CHANNELS,				m_comboSpectrometerChannels);

	if(m_curScanner != NULL){
		DDX_Text(pDX, IDC_EDIT_SITE,					m_curScanner->site);
		DDX_Text(pDX, IDC_EDIT_OBSERVATORY,			m_curScanner->observatory);
		DDX_Text(pDX, IDC_EDIT_SERIALNUMBER,			m_curScanner->spec[0].serialNumber);

		//DDX_Text(pDX, IDC_EDIT_STEPSPERROUND1,			m_curScanner->motor[0].stepsPerRound);
		//DDX_Text(pDX, IDC_EDIT_STEPSPERROUND2,			m_curScanner->motor[1].stepsPerRound);
		//DDX_Text(pDX, IDC_EDIT_MOTORSTEPSCOMPENSATION1,	m_curScanner->motor[0].motorStepsComp);
		//DDX_Text(pDX, IDC_EDIT_MOTORSTEPSCOMPENSATION2,	m_curScanner->motor[1].motorStepsComp);

	}else{
		CString site, observatory, serialNumber;
		DDX_Text(pDX, IDC_EDIT_SITE,					site);
		DDX_Text(pDX, IDC_EDIT_OBSERVATORY,			observatory);
		DDX_Text(pDX, IDC_EDIT_SERIALNUMBER,			serialNumber);
	}
}


BEGIN_MESSAGE_MAP(CLocationConfigurationDlg, CPropertyPage)
	// immediately save all changes made in the dialog
	ON_EN_CHANGE(IDC_EDIT_SITE,											SaveData)
	ON_EN_CHANGE(IDC_EDIT_OBSERVATORY,				SaveData)
	ON_EN_CHANGE(IDC_EDIT_SERIALNUMBER,					SaveData)
	ON_EN_CHANGE(IDC_EDIT_STEPSPERROUND1,				SaveData)
	ON_EN_CHANGE(IDC_EDIT_STEPSPERROUND2,				SaveData)
	ON_EN_CHANGE(IDC_EDIT_MOTORSTEPSCOMPENSATION1,		SaveData)
	ON_EN_CHANGE(IDC_EDIT_MOTORSTEPSCOMPENSATION2,		SaveData)

	ON_CBN_SELCHANGE(IDC_COMBO_VOLCANO,					OnChangeVolcano)
	ON_CBN_SELCHANGE(IDC_COMBO_SPECTROMETERMODEL,		OnChangeModel)
	//ON_CBN_SELCHANGE(IDC_COMBO_OBSERVATORY,				SaveData)
	ON_CBN_SELCHANGE(IDC_COMBO_CHANNELS,				OnChangeChannelNum)
	ON_CBN_SELCHANGE(IDC_COMBO_INSTRUMENTTYPE,			OnChangeType)
	ON_CBN_SELCHANGE(IDC_COMBO_ELECTRONICS, &CLocationConfigurationDlg::OnChangeElectronics)
END_MESSAGE_MAP()


BOOL CLocationConfigurationDlg::OnInitDialog()
{
	Common common;
	CString str, model;

	CDialog::OnInitDialog();

	// The volcanoes - combo box 
	UpdateVolcanoList();

	// The observatories - combo box 
	//m_comboObservatory.ResetContent();
	//for(unsigned int k = 0; k < g_observatories.m_observatoryNum; ++k){
	//	str.Format("%s", g_observatories.m_name[k]);
	//	m_comboObservatory.AddString(str);
	//}

	// The spectrometer models combo box
	m_comboSpectrometerModel.ResetContent();
	for(int j = 0; j < CSpectrometerModel::GetNumSpectrometerModels()-1; ++j){
		CSpectrometerModel::ToString((SPECTROMETER_MODEL)j, model);
		m_comboSpectrometerModel.AddString(model);
	}

	// The channels combo box
	m_comboSpectrometerChannels.ResetContent();
	for(int k = 0; k < MAX_CHANNEL_NUM; ++k){
		str.Format("%d", k+1);
		m_comboSpectrometerChannels.AddString(str);
	}

	// The electronics combo-box
	m_comboElectronics.ResetContent();
	m_comboElectronics.AddString("Version 1");
	m_comboElectronics.AddString("Version 3");

	// The instrument-type combo-box
	m_comboInstrumentType.ResetContent();
	m_comboInstrumentType.AddString("Gothenburg");
	//m_comboInstrumentType.AddString("Heidelberg");

	UpdateData(FALSE);

	// setup the tool tips
	InitToolTips();

	// Setup the label.
	str.Format("* The serial number is used to identify spectra\n\n* Data from connected scanners will only be evaluated if the serial-number is configured\n");
	SetDlgItemText(IDC_LABEL_MESSAGE, str);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CLocationConfigurationDlg::InitToolTips(){
	// Don't initialize the tool tips twice
	if(m_toolTip.m_hWnd != NULL)
		return;

	// Enable the tool tips
	if(!m_toolTip.Create(this)){
		TRACE0("Failed to create tooltip control\n"); 
	}
	m_toolTip.AddTool(&m_labelVolcano,					IDC_EDIT_VOLCANO);
	m_toolTip.AddTool(&m_comboVolcano,					IDC_EDIT_VOLCANO);
	m_toolTip.AddTool(&m_labelSite,						IDC_EDIT_SITE);
	m_toolTip.AddTool(&m_editSite,						IDC_EDIT_SITE);
	m_toolTip.AddTool(&m_labelObservatory,				IDC_EDIT_OBSERVATORY);
	m_toolTip.AddTool(&m_editObservatory,				IDC_EDIT_OBSERVATORY);
	m_toolTip.AddTool(&m_comboSpectrometerModel,		IDC_COMBO_SPECTROMETERMODEL);
	m_toolTip.AddTool(&m_comboSpectrometerChannels,		IDC_COMBO_CHANNELS);
	m_toolTip.AddTool(&m_editSerial,					IDC_EDIT_SERIALNUMBER);

	m_toolTip.SetMaxTipWidth(SHRT_MAX);
	m_toolTip.Activate(TRUE);
}

BOOL CLocationConfigurationDlg::OnKillActive(){

	//SaveData();

	//HTREEITEM hTree = m_scannerTree->GetSelectedItem();
	//int currentScanner = GetScanner(m_scannerTree->GetItemText(hTree));
	//int currentSpec    = GetSpectrometer(m_scannerTree->GetItemText(hTree));

	//if(currentSpec == -1){
	//  if((currentScanner == -1) || (m_configuration->scanner[currentScanner].specNum > 1))
	//    return CPropertyPage::OnKillActive();
	//  else{
	//    hTree = m_scannerTree->GetChildItem(hTree);
	//    currentSpec = 0;
	//  }
	//}

	//// Check if the site and observatory are defined
	//if(strlen(m_configuration->scanner[currentScanner].observatory) == 0){
	//  MessageBox("Please write the name of the observatory");
	//  return 0;
	//}
	//if(strlen(m_configuration->scanner[currentScanner].site) == 0){
	//  MessageBox("Please write the name of the site where the scanner is");
	//  return 0;
	//}

	return CPropertyPage::OnKillActive();
}

void CLocationConfigurationDlg::SaveData(){
	UpdateData(TRUE);

	// save the observatory-name
	//if(m_curScanner != NULL){
	//	int sel = m_comboObservatory.GetCurSel();
	//	if(sel >= 0){
	//		m_curScanner->observatory.Format("%s", g_observatories.m_name[sel]);
	//	}else{
	//		m_curScanner->observatory.Format("chalmers");
	//	}
	//}else{
	//	
	//}
}

void CLocationConfigurationDlg::UpdateDlg(){
	UpdateData(FALSE);
}

BOOL CLocationConfigurationDlg::PreTranslateMessage(MSG* pMsg){
	m_toolTip.RelayEvent(pMsg);

	return CPropertyPage::PreTranslateMessage(pMsg);
}

void CLocationConfigurationDlg::OnChangeScanner(){
	if(UpdateData(TRUE)){ // <-- first save the data in the dialog

		// Then change the settings so that we're using the newly selected scanner
		CSystemConfigurationPage::OnChangeScanner();

		if(m_curScanner == NULL){
			return;
		}

		// Then Update the volcano combo-box
		for(unsigned int i = 0; i < g_volcanoes.m_volcanoNum; ++i){
			if(Equals(g_volcanoes.m_name[i], m_curScanner->volcano)){
				m_comboVolcano.SetCurSel(i);
				break;
			}
		}

		// Then update the spectrometer model
		m_comboSpectrometerModel.SetCurSel((int)m_curScanner->spec[0].model);

		// Then update the channel numbers
		m_comboSpectrometerChannels.SetCurSel(m_curScanner->spec[0].channelNum - 1);
		
		// Update the electronics
		m_comboElectronics.SetCurSel((int)m_curScanner->electronicsBox);

		// Update the instrument-type
		m_comboInstrumentType.SetCurSel((int)m_curScanner->instrumentType);

		// If the type is 'Heidelberg' then we can only have one channel
		if(m_curScanner->instrumentType == INSTR_HEIDELBERG){
			m_curScanner->spec[0].channelNum = 1;
			m_comboSpectrometerChannels.SetCurSel(0);
			m_comboSpectrometerChannels.EnableWindow(FALSE);
			m_editSPR1.ShowWindow(TRUE);
			m_editSPR2.ShowWindow(TRUE);
			m_labelSPR.ShowWindow(TRUE);
			m_editMSC2.ShowWindow(TRUE);

		}else if(m_curScanner->instrumentType == INSTR_GOTHENBURG){
			m_comboSpectrometerChannels.EnableWindow(TRUE);
			m_editSPR1.ShowWindow(FALSE);
			m_editSPR2.ShowWindow(FALSE);
			m_labelSPR.ShowWindow(FALSE);
			m_editMSC2.ShowWindow(FALSE);
		}

		// Finally, update the screen to reflect the changes
		UpdateData(FALSE);
	}
}

BOOL ConfigurationDialog::CLocationConfigurationDlg::OnSetActive()
{
	UpdateData(FALSE);
	return CSystemConfigurationPage::OnSetActive();
}

void CLocationConfigurationDlg::OnChangeVolcano(){

	if(m_curScanner == NULL)
		return;

	int curSel = m_comboVolcano.GetCurSel();
	if(curSel < 0)
		return;

	if(curSel == g_volcanoes.m_volcanoNum){
		// The user has selected the 'Other' - volcano, add one more volcano to the list
		AddAVolcano();
	}

	curSel = m_comboVolcano.GetCurSel();
	m_curScanner->volcano.Format("%s", g_volcanoes.m_name[curSel]);

	m_scannerTree->UpdateTree();

	// Also update the name of the file that the program should read from the server...
	m_configuration->windSourceSettings.windFieldFile.Format("ftp://129.16.35.206/wind/wind_%s.txt", g_volcanoes.m_simpleName[curSel]);
}

/** The user has changed the model of the spectrometer */
void CLocationConfigurationDlg::OnChangeModel(){
	if(m_curScanner == NULL)
		return;

	int curSel = m_comboSpectrometerModel.GetCurSel();
	if(curSel < 0)
		return;

	m_curScanner->spec[0].model = (SPECTROMETER_MODEL)curSel;
}

/** The user has changed the type of the instrument */
void CLocationConfigurationDlg::OnChangeType(){
	if(m_curScanner == NULL)
		return;

	int curSel = m_comboInstrumentType.GetCurSel();
	if(curSel < 0)
		return;

	m_curScanner->instrumentType = (INSTRUMENT_TYPE)curSel;

	// If the type is 'Heidelberg' then we can only have one channel
	if(m_curScanner->instrumentType == INSTR_HEIDELBERG){
		m_curScanner->spec[0].channelNum = 1;
		m_comboSpectrometerChannels.SetCurSel(0);
		m_comboSpectrometerChannels.EnableWindow(FALSE);

		m_editSPR1.ShowWindow(TRUE);
		m_editSPR2.ShowWindow(TRUE);
		m_labelSPR.ShowWindow(TRUE);
		m_editMSC2.ShowWindow(TRUE);
	}else if(m_curScanner->instrumentType == INSTR_GOTHENBURG){
		m_comboSpectrometerChannels.EnableWindow(TRUE);

		m_editSPR1.ShowWindow(FALSE);
		m_editSPR2.ShowWindow(FALSE);
		m_labelSPR.ShowWindow(FALSE);
		m_editMSC2.ShowWindow(FALSE);
	}

	// Call the 'onchangescanner' function to determine if
	//	we want to change the number of tabs...
	((CScannerConfiguration *)m_parent)->OnChangeScanner();
}

/** The user has changed the number of channels in the spectrometer */
void CLocationConfigurationDlg::OnChangeChannelNum(){
	if(m_curScanner == NULL)
		return;

	int curSel = m_comboSpectrometerChannels.GetCurSel();
	if(curSel < 0)
		return;

	m_curScanner->spec[0].channelNum = curSel + 1;

	((CScannerConfiguration *)m_parent)->OnChangeScanner();
}


/** Adds a volcano to the list of volcanoes */
void	CLocationConfigurationDlg::AddAVolcano(){
	CString name, tempStr;
	Common common;
	double latitude, longitude;
	long	altitude;

	// 1. Ask the user about the name of the volcano
	Dialogs::CQueryStringDialog nameDlg;
	nameDlg.m_windowText.Format("What is the name of the source?");
	nameDlg.m_inputString = &name;
	INT_PTR ret = nameDlg.DoModal();

	if(IDCANCEL == ret)
		return;

	// 2. Ask the user about the latitude, longitude and altitude of the source
	nameDlg.m_windowText.Format("The latitude of the source?");
	nameDlg.m_inputString = &tempStr;
	tempStr.Format("0.0");
	ret = nameDlg.DoModal();

	if(IDCANCEL == ret || 1 != sscanf(tempStr, "%lf", &latitude))
		return;

	nameDlg.m_windowText.Format("The longitude of the source?");
	tempStr.Format("0.0");
	ret = nameDlg.DoModal();

	if(IDCANCEL == ret || 1 != sscanf(tempStr, "%lf", &longitude))
		return;

	nameDlg.m_windowText.Format("The altitude of the source?");
	tempStr.Format("0");
	ret = nameDlg.DoModal();

	if(IDCANCEL == ret || 1 != sscanf(tempStr, "%ld", &altitude))
		return;

	// 3. Add the user-given source to the list of volcanoes
	int index = g_volcanoes.m_volcanoNum;
	g_volcanoes.m_name[index].Format(name);
	g_volcanoes.m_simpleName[index].Format(common.SimplifyString(name));
	g_volcanoes.m_peakLatitude[index]		= latitude;
	g_volcanoes.m_peakLongitude[index]		= longitude;
	g_volcanoes.m_peakHeight[index]			= altitude;
	g_volcanoes.m_hoursToGMT[index]			= 0;
	g_volcanoes.m_observatory[index]		= 1;
	++g_volcanoes.m_volcanoNum;

	// Update the list of volcanoes
	UpdateVolcanoList();

	// Set the new volcano to be the currently selected
	m_comboVolcano.SetCurSel(g_volcanoes.m_volcanoNum - 1);
}

/** (Re-)initializes the list of volcanoes */
void CLocationConfigurationDlg::UpdateVolcanoList(){
	CString str;

	m_comboVolcano.ResetContent();
	for(unsigned int k = 0; k < g_volcanoes.m_volcanoNum; ++k){
		str.Format("%s", g_volcanoes.m_name[k]);
		m_comboVolcano.AddString(str);
	}
	m_comboVolcano.AddString("Other...");
}


void CLocationConfigurationDlg::OnChangeElectronics()
{
	if (m_curScanner == NULL)
		return;

	int curSel = m_comboElectronics.GetCurSel();
	if (curSel < 0)
		return;

	m_curScanner->electronicsBox = (ELECTRONICS_BOX)curSel;
}
