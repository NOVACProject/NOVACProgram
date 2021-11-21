// ReEval_MiscSettingsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "../NovacMasterProgram.h"
#include "ReEval_MiscSettingsDlg.h"
#include "../Dialogs/DarkSettingsDialog.h"
#include "reevalsettingsfilehandler.h"

using namespace ReEvaluation;

// CReEval_MiscSettingsDlg dialog

IMPLEMENT_DYNAMIC(CReEval_MiscSettingsDlg, CPropertyPage)
CReEval_MiscSettingsDlg::CReEval_MiscSettingsDlg()
    : CPropertyPage(CReEval_MiscSettingsDlg::IDD)
{
    m_reeval = NULL;
}

CReEval_MiscSettingsDlg::~CReEval_MiscSettingsDlg()
{
    m_reeval = NULL;
}

void CReEval_MiscSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    DDX_Radio(pDX, IDC_IGNORE_DARK, (int&)m_reeval->m_ignore_Lower.m_type);
    DDX_Text(pDX, IDC_EDIT_IGNORE_INTENSITY, (double&)m_reeval->m_ignore_Lower.m_intensity);
    DDX_Text(pDX, IDC_EDIT_IGNORE_CHANNEL, (int&)m_reeval->m_ignore_Lower.m_channel);

    DDX_Radio(pDX, IDC_IGNORE_SATURATED, (int&)m_reeval->m_ignore_Upper.m_type);
    DDX_Text(pDX, IDC_EDIT_UPPER_IGNORE_INTENSITY, (double&)m_reeval->m_ignore_Upper.m_intensity);
    DDX_Text(pDX, IDC_EDIT_UPPER_IGNORE_CHANNEL, (int&)m_reeval->m_ignore_Upper.m_channel);

    DDX_Radio(pDX, IDC_RADIO_SKYSPECTRUM_FIRST, (int&)m_reeval->m_skySettings.skyOption);
    DDX_Text(pDX, IDC_EDIT_SKYINDEX, (long&)m_reeval->m_skySettings.indexInScan);

    DDX_Check(pDX, IDC_CHECK_AVERAGEDSPECTRA, (int&)m_reeval->m_averagedSpectra);

    // The buttons
    DDX_Control(pDX, ID_FILE_LOADMISC, m_loadBtn);
    DDX_Control(pDX, ID_FILE_SAVEMISC, m_saveBtn);

    // The user supplied sky-spectrum
    // TODO: Fix this.
    //DDX_Text(pDX,	IDC_EDIT_USER_SKY,								m_reeval->m_skySettings.skySpectrumFile);
}


BEGIN_MESSAGE_MAP(CReEval_MiscSettingsDlg, CPropertyPage)
    ON_COMMAND(IDC_BTN_BROWSESKY, OnBrowseSkySpectrum)

    ON_COMMAND(ID_FILE_LOADMISC, OnLoadMiscSettings)
    ON_COMMAND(ID_FILE_SAVEMISC, OnSaveMiscSettings)

    ON_COMMAND(IDC_BUTTON_DARK_SETTINGS, OnShowDarkSettings)

    ON_EN_CHANGE(IDC_EDIT_USER_SKY, OnChangeSkySpectrum)
END_MESSAGE_MAP()


// CReEval_MiscSettingsDlg message handlers
void CReEval_MiscSettingsDlg::OnBrowseSkySpectrum() {
    CString skySpec;
    skySpec.Format("");
    TCHAR filter[512];
    int n = _stprintf(filter, "Spectrum Files\0");
    n += _stprintf(filter + n + 1, "*.pak;*.std;*.txt;*.xs\0");
    filter[n + 2] = 0;
    Common common;

    // let the user browse for a spectrum-file
    if (common.BrowseForFile(filter, skySpec)) {
        m_reeval->m_skySettings.skySpectrumFile = std::string((LPCTSTR)skySpec);
        SetDlgItemText(IDC_EDIT_USER_SKY, skySpec);

        m_reeval->m_skySettings.skyOption = Configuration::SKY_OPTION::USER_SUPPLIED;
    }

    // Update the screen
    UpdateData(FALSE);
}

/** Lets the user see the possible options for how to remove the dark */
void CReEval_MiscSettingsDlg::OnShowDarkSettings() {
    Dialogs::CDarkSettingsDialog darkDlg;
    darkDlg.m_darkSettings = &m_reeval->m_darkSettings;
    darkDlg.DoModal();
}

void CReEval_MiscSettingsDlg::OnChangeSkySpectrum() {
    UpdateData(TRUE); // <-- save the data in the dialog
}

void CReEval_MiscSettingsDlg::SaveData() {
    CString str;

    if (m_hWnd == NULL)
        return;

    // ------- Save the change ----------- 
    if (UpdateData(TRUE))
    {
    }
}

BOOL ReEvaluation::CReEval_MiscSettingsDlg::OnInitDialog()
{
    CPropertyPage::OnInitDialog();

    // Save the changes to the dialg
    UpdateData(FALSE);

    // Load the images for the buttons
    m_loadBtn.SetBitmap(::LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BMP_OPEN)));
    m_saveBtn.SetBitmap(::LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BMP_SAVE)));


    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}


/** Called when the user wants to save the current settings into a file */
void ReEvaluation::CReEval_MiscSettingsDlg::OnSaveMiscSettings() {
    CString fileName;
    fileName.Format("");
    TCHAR filter[512];
    int n = _stprintf(filter, "ReEvaluation Settings Files\0");
    n += _stprintf(filter + n + 1, "*.xml;\0");
    filter[n + 2] = 0;
    Common common;

    // Get the data in the dialog
    if (!UpdateData(TRUE))
        return;

    // let the user browse for an evaluation log file and if one is selected, read it
    if (common.BrowseForFile_SaveAs(filter, fileName)) {
        // if there's not a .xml-ending on the file, append it!
        if (!Equals(".xml", fileName.Right(4))) {
            fileName.AppendFormat(".xml");
        }

        FileHandler::CReEvalSettingsFileHandler fileHandler;
        fileHandler.WriteSettings(*m_reeval, fileName, true);
    }
}

/** Called when the user wants to load the settings from file */
void ReEvaluation::CReEval_MiscSettingsDlg::OnLoadMiscSettings() {
    CString fileName;
    fileName.Format("");
    TCHAR filter[512];
    int n = _stprintf(filter, "ReEvaluation Settings Files\0");
    n += _stprintf(filter + n + 1, "*.xml;\0");
    filter[n + 2] = 0;
    Common common;

    // let the user browse for an evaluation log file and if one is selected, read it
    if (common.BrowseForFile(filter, fileName)) {
        FileHandler::CReEvalSettingsFileHandler fileHandler;
        fileHandler.ReadSettings(*m_reeval, fileName);

        // Update the dialog
        UpdateData(FALSE);
    }
}