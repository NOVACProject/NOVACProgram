// LocationConfigurationDlg.cpp : implementation file
//

#include "stdafx.h"
#include "../NovacMasterProgram.h"
#include "LocationConfigurationDlg.h"
#include "ScannerConfiguration.h"
#include "../VolcanoInfo.h"
#include "../ObservatoryInfo.h"
#include "../CustomSpectrometerModelDlg.h"
#include <SpectralEvaluation/Spectra/SpectrometerModel.h>

#include "../Dialogs/QueryStringDialog.h"

using namespace ConfigurationDialog;

extern CVolcanoInfo			g_volcanoes;

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
{
    CSystemConfigurationPage::DoDataExchange(pDX);

    // The labels (only used for showing tooltips);
    DDX_Control(pDX, IDC_LABEL_VOLCANO, m_labelVolcano);
    DDX_Control(pDX, IDC_LABEL_SITE, m_labelSite);
    DDX_Control(pDX, IDC_LABEL_OBSERVATORY, m_labelObservatory);

    // The edits (only used for showing tooltips);
    DDX_Control(pDX, IDC_EDIT_SITE, m_editSite);
    DDX_Control(pDX, IDC_EDIT_OBSERVATORY, m_editObservatory);
    DDX_Control(pDX, IDC_EDIT_SERIALNUMBER, m_editSerial);

    // The combo boxes
    DDX_Control(pDX, IDC_COMBO_VOLCANO, m_comboVolcano);
    DDX_Control(pDX, IDC_COMBO_ELECTRONICS, m_comboElectronics);
    DDX_Control(pDX, IDC_COMBO_SPECTROMETERMODEL, m_comboSpectrometerModel);
    DDX_Control(pDX, IDC_COMBO_CHANNELS, m_comboSpectrometerChannels);

    // plot options

    DDX_Check(pDX, IDC_CHECK_PLOT_COLUMN, m_plotColumn);
    DDX_Check(pDX, IDC_CHECK_PLOT_COLUMN_HISTORY, m_plotColumnHistory);
    DDX_Control(pDX, IDC_EDIT_MINCOL, m_minColumn);
    DDX_Control(pDX, IDC_EDIT_MAXCOL, m_maxColumn);
	DDX_Check(pDX, IDC_CHECK_PLOT_FLUX_HISTORY, m_plotFluxHistory);
	DDX_Control(pDX, IDC_EDIT_MINFLUX, m_minFlux);
	DDX_Control(pDX, IDC_EDIT_MAXFLUX, m_maxFlux);

    if (m_curScanner != NULL) {
        DDX_Text(pDX, IDC_EDIT_SITE, m_curScanner->site);
        DDX_Text(pDX, IDC_EDIT_OBSERVATORY, m_curScanner->observatory);
        DDX_Text(pDX, IDC_EDIT_SERIALNUMBER, m_curScanner->spec[0].serialNumber);

        DDX_Check(pDX, IDC_CHECK_PLOT_COLUMN, m_curScanner->plotColumn);
        DDX_Check(pDX, IDC_CHECK_PLOT_COLUMN_HISTORY, m_curScanner->plotColumnHistory);
        DDX_Text(pDX, IDC_EDIT_MINCOL, m_curScanner->minColumn);
        DDX_Text(pDX, IDC_EDIT_MAXCOL, m_curScanner->maxColumn);
		DDX_Check(pDX, IDC_CHECK_PLOT_FLUX_HISTORY, m_curScanner->plotFluxHistory);
		DDX_Text(pDX, IDC_EDIT_MINFLUX, m_curScanner->minFlux);
		DDX_Text(pDX, IDC_EDIT_MAXFLUX, m_curScanner->maxFlux);

    }
    else {
        CString site, observatory, serialNumber;
        DDX_Text(pDX, IDC_EDIT_SITE, site);
        DDX_Text(pDX, IDC_EDIT_OBSERVATORY, observatory);
        DDX_Text(pDX, IDC_EDIT_SERIALNUMBER, serialNumber);
    }
}


BEGIN_MESSAGE_MAP(CLocationConfigurationDlg, CPropertyPage)
    // immediately save all changes made in the dialog
    ON_EN_CHANGE(IDC_EDIT_SITE, SaveData)
    ON_EN_CHANGE(IDC_EDIT_OBSERVATORY, SaveData)
    ON_EN_CHANGE(IDC_EDIT_SERIALNUMBER, SaveData)

    ON_CBN_SELCHANGE(IDC_COMBO_VOLCANO, OnChangeVolcano)
    ON_CBN_SELCHANGE(IDC_COMBO_SPECTROMETERMODEL, OnChangeModel)
    ON_CBN_SELCHANGE(IDC_COMBO_CHANNELS, OnChangeChannelNum)
    ON_CBN_SELCHANGE(IDC_COMBO_ELECTRONICS, &CLocationConfigurationDlg::OnChangeElectronics)

    ON_BN_CLICKED(IDC_CHECK_PLOT_COLUMN, SaveData)
    ON_BN_CLICKED(IDC_CHECK_PLOT_COLUMN_HISTORY, SaveData)
    ON_EN_CHANGE(IDC_EDIT_MINCOL, SaveData)
    ON_EN_CHANGE(IDC_EDIT_MAXCOL, SaveData)
	ON_BN_CLICKED(IDC_CHECK_PLOT_FLUX_HISTORY, SaveData)
	ON_EN_CHANGE(IDC_EDIT_MINFLUX, SaveData)
	ON_EN_CHANGE(IDC_EDIT_MAXFLUX, SaveData)
END_MESSAGE_MAP()

BOOL CLocationConfigurationDlg::OnInitDialog()
{
    Common common;
    CString str;

    CDialog::OnInitDialog();

    // The volcanoes - combo box 
    UpdateVolcanoList();

    // The spectrometer models combo box
    RecreateSpectrometerModelCombo();

    // The channels combo box
    m_comboSpectrometerChannels.ResetContent();
    for (int k = 0; k < MAX_CHANNEL_NUM; ++k)
    {
        str.Format("%d", k + 1);
        m_comboSpectrometerChannels.AddString(str);
    }

	// The electronics combo-box
	m_comboElectronics.ResetContent();
	m_comboElectronics.AddString("Axis"); // Axis
	m_comboElectronics.AddString("Moxa"); // Moxa
    m_comboElectronics.AddString("Axiomtek"); // Axiomtek

    //m_plotColumn = 0;

    UpdateData(FALSE);

    // setup the tool tips
    InitToolTips();

    // Setup the label.
    //str.Format("* The serial number is used to identify spectra\n\n* Data from connected scanners will only be evaluated if the serial-number is configured\n");
    //SetDlgItemText(IDC_LABEL_MESSAGE, str);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CLocationConfigurationDlg::InitToolTips() {
    // Don't initialize the tool tips twice
    if (m_toolTip.m_hWnd != NULL)
        return;

    // Enable the tool tips
    if (!m_toolTip.Create(this)) {
        TRACE0("Failed to create tooltip control\n");
    }
    m_toolTip.AddTool(&m_labelVolcano, IDC_EDIT_VOLCANO);
    m_toolTip.AddTool(&m_comboVolcano, IDC_EDIT_VOLCANO);
    m_toolTip.AddTool(&m_labelSite, IDC_EDIT_SITE);
    m_toolTip.AddTool(&m_editSite, IDC_EDIT_SITE);
    m_toolTip.AddTool(&m_labelObservatory, IDC_EDIT_OBSERVATORY);
    m_toolTip.AddTool(&m_editObservatory, IDC_EDIT_OBSERVATORY);
    m_toolTip.AddTool(&m_comboSpectrometerModel, IDC_COMBO_SPECTROMETERMODEL);
    m_toolTip.AddTool(&m_comboSpectrometerChannels, IDC_COMBO_CHANNELS);
    m_toolTip.AddTool(&m_editSerial, IDC_EDIT_SERIALNUMBER);

    m_toolTip.SetMaxTipWidth(SHRT_MAX);
    m_toolTip.Activate(TRUE);
}

BOOL CLocationConfigurationDlg::OnKillActive() {
    return CPropertyPage::OnKillActive();
}

void CLocationConfigurationDlg::SaveData() {
    UpdateData(TRUE);
}

void CLocationConfigurationDlg::UpdateDlg() {
    UpdateData(FALSE);
}

BOOL CLocationConfigurationDlg::PreTranslateMessage(MSG* pMsg) {
    m_toolTip.RelayEvent(pMsg);

    return CPropertyPage::PreTranslateMessage(pMsg);
}

void CLocationConfigurationDlg::OnChangeScanner() {
    if (UpdateData(TRUE)) { // <-- first save the data in the dialog

        // Then change the settings so that we're using the newly selected scanner
        CSystemConfigurationPage::OnChangeScanner();

        if (m_curScanner == NULL) {
            return;
        }

        // Then Update the volcano combo-box
        for (unsigned int i = 0; i < g_volcanoes.m_volcanoNum; ++i) {
            if (Equals(g_volcanoes.m_name[i], m_curScanner->volcano)) {
                m_comboVolcano.SetCurSel(i);
                break;
            }
        }

        // Then update the spectrometer model
        const int spectrometerTypeIdx = CSpectrometerDatabase::GetInstance().GetModelIndex(m_curScanner->spec[0].modelName);
        m_comboSpectrometerModel.SetCurSel(spectrometerTypeIdx);

        // Then update the channel numbers
        m_comboSpectrometerChannels.SetCurSel(m_curScanner->spec[0].channelNum - 1);

        // Update the electronics
        m_comboElectronics.SetCurSel((int)m_curScanner->electronicsBox);

        m_comboSpectrometerChannels.EnableWindow(TRUE);

        // Finally, update the screen to reflect the changes
        UpdateData(FALSE);
    }
}

BOOL ConfigurationDialog::CLocationConfigurationDlg::OnSetActive()
{
    UpdateData(FALSE);
    return CSystemConfigurationPage::OnSetActive();
}

void CLocationConfigurationDlg::OnChangeVolcano() {

    if (m_curScanner == NULL)
        return;

    int curSel = m_comboVolcano.GetCurSel();
    if (curSel < 0)
        return;

    if (curSel == g_volcanoes.m_volcanoNum) {
        // The user has selected the 'Other' - volcano, add one more volcano to the list
        AddAVolcano();
    }

    curSel = m_comboVolcano.GetCurSel();
    m_curScanner->volcano.Format("%s", (LPCSTR)g_volcanoes.m_name[curSel]);

    m_scannerTree->UpdateTree();

    // Also update the name of the file that the program should read from the server...
    m_configuration->windSourceSettings.windFieldFile.Format("ftp://129.16.35.206/wind/wind_%s.txt", (LPCSTR)g_volcanoes.m_simpleName[curSel]);
}

void CLocationConfigurationDlg::RecreateSpectrometerModelCombo()
{
    m_comboSpectrometerModel.ResetContent();
    std::vector<std::string> modelNames = CSpectrometerDatabase::GetInstance().ListModels();
    for (size_t j = 0; j < modelNames.size(); ++j)
    {
        CString modelStr(modelNames[j].c_str());
        m_comboSpectrometerModel.AddString(modelStr);
    }
    m_comboSpectrometerModel.AddString("Custom...");
}

/** The user has changed the model of the spectrometer */
void CLocationConfigurationDlg::OnChangeModel()
{
    if (m_curScanner == nullptr)
        return;

    const int curSel = m_comboSpectrometerModel.GetCurSel();
    if (curSel < 0)
        return;

    SpectrometerModel spectrometerModel = CSpectrometerDatabase::GetInstance().GetModel(curSel);

    if (spectrometerModel.IsUnknown())
    {
        CustomSpectrometerModelDlg dlg;
        INT_PTR ret = dlg.DoModal();
        if (ret == IDCANCEL)
        {
            return;
        }
        else
        {
            CSpectrometerDatabase::GetInstance().AddModel(dlg.m_configuredSpectrometer);
            RecreateSpectrometerModelCombo();

            m_curScanner->spec[0].modelName = dlg.m_configuredSpectrometer.modelName;

            const int spectrometerTypeIdx = CSpectrometerDatabase::GetInstance().GetModelIndex(m_curScanner->spec[0].modelName);
            m_comboSpectrometerModel.SetCurSel(spectrometerTypeIdx);
        }
    }
    else
    {
        m_curScanner->spec[0].modelName = spectrometerModel.modelName;
    }
}

/** The user has changed the number of channels in the spectrometer */
void CLocationConfigurationDlg::OnChangeChannelNum() {
    if (m_curScanner == NULL)
        return;

    int curSel = m_comboSpectrometerChannels.GetCurSel();
    if (curSel < 0)
        return;

    m_curScanner->spec[0].channelNum = curSel + 1;

    ((CScannerConfiguration *)m_parent)->OnChangeScanner();
}


/** Adds a volcano to the list of volcanoes */
void	CLocationConfigurationDlg::AddAVolcano() {
    CString name, tempStr;
    Common common;
    double latitude, longitude;
    long	altitude;

    // 1. Ask the user about the name of the volcano
    Dialogs::CQueryStringDialog nameDlg;
    nameDlg.m_windowText.Format("What is the name of the source?");
    nameDlg.m_inputString = &name;
    INT_PTR ret = nameDlg.DoModal();

    if (IDCANCEL == ret)
        return;

    // 2. Ask the user about the latitude, longitude and altitude of the source
    nameDlg.m_windowText.Format("The latitude of the source?");
    nameDlg.m_inputString = &tempStr;
    tempStr.Format("0.0");
    ret = nameDlg.DoModal();

    if (IDCANCEL == ret || 1 != sscanf(tempStr, "%lf", &latitude))
        return;

    nameDlg.m_windowText.Format("The longitude of the source?");
    tempStr.Format("0.0");
    ret = nameDlg.DoModal();

    if (IDCANCEL == ret || 1 != sscanf(tempStr, "%lf", &longitude))
        return;

    nameDlg.m_windowText.Format("The altitude of the source?");
    tempStr.Format("0");
    ret = nameDlg.DoModal();

    if (IDCANCEL == ret || 1 != sscanf(tempStr, "%ld", &altitude))
        return;

    // 3. Add the user-given source to the list of volcanoes
    unsigned int index = g_volcanoes.m_volcanoNum;
    g_volcanoes.m_name[index].Format(name);
    g_volcanoes.m_simpleName[index].Format(common.SimplifyString(name));
    g_volcanoes.m_peakLatitude[index] = latitude;
    g_volcanoes.m_peakLongitude[index] = longitude;
    g_volcanoes.m_peakHeight[index] = altitude;
    g_volcanoes.m_hoursToGMT[index] = 0;
    g_volcanoes.m_observatory[index] = 1;
    ++g_volcanoes.m_volcanoNum;

    // Update the list of volcanoes
    UpdateVolcanoList();

    // Set the new volcano to be the currently selected
    m_comboVolcano.SetCurSel(g_volcanoes.m_volcanoNum - 1);
}

/** (Re-)initializes the list of volcanoes */
void CLocationConfigurationDlg::UpdateVolcanoList() {
    CString str;

    m_comboVolcano.ResetContent();
    for (unsigned int k = 0; k < g_volcanoes.m_volcanoNum; ++k) {
        str.Format("%s", (LPCSTR)g_volcanoes.m_name[k]);
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

