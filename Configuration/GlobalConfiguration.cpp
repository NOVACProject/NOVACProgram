// GlobalConfiguration.cpp : implementation file
//

#include "stdafx.h"
#include "../NovacMasterProgram.h"
#include "GlobalConfiguration.h"
#include "AdvancedFTPUploadSettings.h"

using namespace ConfigurationDialog;

// CGlobalConfiguration dialog

IMPLEMENT_DYNAMIC(CGlobalConfiguration, CPropertyPage)
CGlobalConfiguration::CGlobalConfiguration()
    : CPropertyPage(CGlobalConfiguration::IDD)
{
    m_useLocalDirectory = 0;
}

CGlobalConfiguration::~CGlobalConfiguration()
{
}

void CGlobalConfiguration::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);

    // The output-directory
    DDX_Control(pDX, IDC_EDIT_OUTPUT_DIRECTORY, m_outputDir);
    DDX_Text(pDX, IDC_EDIT_OUTPUT_DIRECTORY, m_configuration->outputDirectory);

    // The wind-field source
    DDX_Text(pDX, IDC_EDIT_WINDFIELDFILE, m_configuration->windSourceSettings.windFieldFile);
    DDX_Text(pDX, IDC_EDIT_RELOADWINDFIELDFILE, m_configuration->windSourceSettings.windFileReloadInterval);
    DDX_Check(pDX, IDC_CHECK_WINDFIELDFILE_ENABLED, m_configuration->windSourceSettings.enabled);

    // Automatic/manual startup
    DDX_Radio(pDX, IDC_STARTUP_MANUAL, m_configuration->startup);

    // Data FTP-server
    DDX_Control(pDX, IDC_FTPIP, m_ftpURL);
    DDX_Text(pDX, IDC_FTPIP, m_configuration->ftpSetting.ftpAddress);
    DDX_Control(pDX, IDC_FTPUSER, m_ftpUserName);
    DDX_Text(pDX, IDC_FTPUSER, m_configuration->ftpSetting.userName);
    DDX_Control(pDX, IDC_FTPPASS, m_ftpPassword);
    DDX_Text(pDX, IDC_FTPPASS, m_configuration->ftpSetting.password);
    DDX_CBString(pDX, IDC_DATA_UPLOAD_SERVER_PROTOCOL, m_configuration->ftpSetting.protocol);

    // Publishing the results
    DDX_Check(pDX, IDC_CHECK_PUBLISH, m_configuration->webSettings.publish);
    //DDX_Control(pDX, IDC_RADIO_LOCALDIRECTORY, m_localDirRadio);
    //DDX_Control(pDX, IDC_RADIO_FTPSERVER, m_ftpDirRadio);
    //DDX_Radio(pDX, IDC_RADIO_LOCALDIRECTORY, m_useLocalDirectory);
    DDX_Control(pDX, IDC_COMBO_IMAGEFORMAT, m_imageFormatCombo);
    DDX_Control(pDX, IDC_EDIT_LOCALDIRECTORY, m_localDir);
    DDX_Text(pDX, IDC_EDIT_LOCALDIRECTORY, m_configuration->webSettings.localDirectory);

    // Excecuting external excecutables/scripts
    DDX_Text(pDX, IDC_EDIT_SHELL_EXCECUTE, m_configuration->externalSetting.fullScanScript);
    DDX_Text(pDX, IDC_EDIT_SHELL_EXCECUTE_IMAGE, m_configuration->externalSetting.imageScript);

    // The labels (only used for the tooltips)
    DDX_Control(pDX, IDC_LABEL_OUTPUT_DIRECTORY, m_outputDirectory);
    DDX_Control(pDX, IDC_LABEL_STARTUP, m_startup);
    DDX_Control(pDX, IDC_LABEL_FTP_UPLOAD_SERVER, m_ftpUploadServer);
    DDX_Control(pDX, IDC_LABEL_FTP_USERNAME, m_ftpUserNameLabel);
    DDX_Control(pDX, IDC_LABEL_FTP_PASSWORD, m_ftpPasswordLabel);
}


BEGIN_MESSAGE_MAP(CGlobalConfiguration, CPropertyPage)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_OUTPUTPATH, OnBrowseOutputPath)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_WINDFIELDFILE, OnBrowseWindFieldFile)
    ON_BN_CLICKED(IDC_BUTTON_ADVANCED_FTP, OnAdvancedFTPSettings)
    ON_BN_CLICKED(IDC_STARTUP_MANUAL, SaveData)
    ON_BN_CLICKED(IDC_STARTUP_AUTOMATIC, SaveData)
    ON_LBN_SELCHANGE(IDC_COMBO_IMAGEFORMAT, SaveData)
    ON_LBN_SELCHANGE(IDC_DATA_UPLOAD_SERVER_PROTOCOL, SaveData)
    ON_EN_CHANGE(IDC_FTPIP, SaveData)
    ON_EN_CHANGE(IDC_FTPUSER, SaveData)
    ON_EN_CHANGE(IDC_FTPPASS, SaveData)
    ON_EN_CHANGE(IDC_EDIT_OUTPUT_DIRECTORY, SaveData)
    ON_EN_CHANGE(IDC_EDIT_LOCALDIRECTORY, SaveData)
    ON_EN_CHANGE(IDC_EDIT_SHELL_EXCECUTE, SaveData)
    ON_EN_CHANGE(IDC_EDIT_SHELL_EXCECUTE_IMAGE, SaveData)
    ON_EN_CHANGE(IDC_EDIT_RELOADWINDFIELDFILE, SaveData)
    ON_EN_CHANGE(IDC_EDIT_WINDFIELDFILE, SaveData)
    ON_LBN_SELCHANGE(IDC_COMBO_IMAGEFORMAT, SaveData)

    ON_BN_CLICKED(IDC_CHECK_PUBLISH, OnChangePublish)
    ON_BN_CLICKED(IDC_CHECK_WINDFIELDFILE_ENABLED, OnChangeWindFileUpdate)
END_MESSAGE_MAP()


// CGlobalConfiguration message handlers

void CGlobalConfiguration::OnBrowseOutputPath() {
    Common common;
    CString folderName;

    if (common.BrowseForDirectory(folderName))
        this->m_configuration->outputDirectory.Format("%s", (LPCSTR)folderName);

    UpdateData(FALSE);
}

void CGlobalConfiguration::OnBrowseWindFieldFile() {
    TCHAR filter[512];
    CString fileName;
    Common common;

    // Make a filter for files to look for
    int n = _stprintf(filter, "Wind Field Files\0");
    n += _stprintf(filter + n + 1, "*.txt;*.nc;\0");
    filter[n + 2] = 0;

    // let the user browse for an wind-field file
    if (!common.BrowseForFile(filter, fileName)) {
        return; // cancelled
    }

    m_configuration->windSourceSettings.windFieldFile.Format(fileName);

    UpdateData(FALSE);
}

void CGlobalConfiguration::SaveData() {
    UpdateData(TRUE);

    // Save the image-format for the web - publishing
    CString imgFormat;
    m_imageFormatCombo.GetLBText(m_imageFormatCombo.GetCurSel(), imgFormat);
    m_configuration->webSettings.imageFormat.Format(imgFormat);
}

BOOL ConfigurationDialog::CGlobalConfiguration::OnInitDialog()
{
    CPropertyPage::OnInitDialog();

    // Setup the combo-box with image-file formats
    m_imageFormatCombo.AddString(".png");
    m_imageFormatCombo.AddString(".bmp");
    m_imageFormatCombo.AddString(".gif");
    m_imageFormatCombo.AddString(".jpg");
    if (Equals(m_configuration->webSettings.imageFormat, ".png"))
        m_imageFormatCombo.SetCurSel(0);
    else if (Equals(m_configuration->webSettings.imageFormat, ".bmp"))
        m_imageFormatCombo.SetCurSel(1);
    else if (Equals(m_configuration->webSettings.imageFormat, ".gif"))
        m_imageFormatCombo.SetCurSel(2);
    else if (Equals(m_configuration->webSettings.imageFormat, ".jpg"))
        m_imageFormatCombo.SetCurSel(3);
    else
        m_imageFormatCombo.SetCurSel(0);

    // Select the 'local' - directory
    m_useLocalDirectory = 0;

    // Setup the tool tips
    InitToolTips();

    // Set which controls should be enabled and which should be disabled
    OnChangePublish();
    OnChangeWindFileUpdate();
    return TRUE;	// return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CGlobalConfiguration::InitToolTips() {
    if (!m_toolTip.Create(this)) {
        TRACE0("Failed to create tooltip control\n");
    }
    // add tools here...
    m_toolTip.AddTool(&m_outputDirectory, IDC_LABEL_OUTPUT_DIRECTORY);		// TODO: THIS DOES NOT WORK FOR THE GROUP BOXES!!!
    m_toolTip.AddTool(&m_outputDir, IDC_LABEL_OUTPUT_DIRECTORY);
    m_toolTip.AddTool(&m_startup, IDC_LABEL_STARTUP);		// TODO: THIS DOES NOT WORK FOR THE GROUP BOXES!!!
    m_toolTip.AddTool(&m_ftpUploadServer, IDC_LABEL_FTP_UPLOAD_SERVER);		// TODO: THIS DOES NOT WORK FOR THE GROUP BOXES!!!
    m_toolTip.AddTool(&m_ftpURL, IDC_LABEL_FTP_SERVER_URL);
    m_toolTip.AddTool(&m_ftpUserName, IDC_LABEL_FTP_USERNAME);
    m_toolTip.AddTool(&m_ftpUserNameLabel, IDC_LABEL_FTP_USERNAME);
    m_toolTip.AddTool(&m_ftpPassword, IDC_LABEL_FTP_PASSWORD);
    m_toolTip.AddTool(&m_ftpPasswordLabel, IDC_LABEL_FTP_PASSWORD);
    //	m_toolTip.AddTool(this, IDC_LABEL_OUTPUT_DIRECTORY);
    m_toolTip.SetMaxTipWidth(SHRT_MAX);
    m_toolTip.Activate(TRUE);
}

BOOL ConfigurationDialog::CGlobalConfiguration::PreTranslateMessage(MSG* pMsg) {
    m_toolTip.RelayEvent(pMsg);

    return CPropertyPage::PreTranslateMessage(pMsg);
}

/** Called when the user changes whether we should publish the data or not */
void ConfigurationDialog::CGlobalConfiguration::OnChangePublish() {
    SaveData();
    if (m_configuration->webSettings.publish) {
        m_imageFormatCombo.EnableWindow(true);
        m_localDir.EnableWindow(true);
        //m_localDirRadio.EnableWindow(true);
        //m_ftpDirRadio.EnableWindow(false);
    }
    else {
        m_imageFormatCombo.EnableWindow(false);
        m_localDir.EnableWindow(false);
        //m_localDirRadio.EnableWindow(false);
        //m_ftpDirRadio.EnableWindow(false);
    }
}

void ConfigurationDialog::CGlobalConfiguration::OnAdvancedFTPSettings() {
    CAdvancedFTPUploadSettings advDialog;
    advDialog.m_configuration = m_configuration;
    advDialog.DoModal();
}

void ConfigurationDialog::CGlobalConfiguration::OnChangeWindFileUpdate()
{
    CButton *m_ctlCheck = (CButton*)GetDlgItem(IDC_CHECK_WINDFIELDFILE_ENABLED);
    m_configuration->windSourceSettings.enabled = m_ctlCheck->GetCheck();
    if (m_configuration->windSourceSettings.enabled == 0) {
        GetDlgItem(IDC_BUTTON_BROWSE_WINDFIELDFILE)->EnableWindow(FALSE);
        GetDlgItem(IDC_EDIT_WINDFIELDFILE)->EnableWindow(FALSE);
        GetDlgItem(IDC_EDIT_RELOADWINDFIELDFILE)->EnableWindow(FALSE);
    }
    else {
        GetDlgItem(IDC_BUTTON_BROWSE_WINDFIELDFILE)->EnableWindow(TRUE);
        GetDlgItem(IDC_EDIT_WINDFIELDFILE)->EnableWindow(TRUE);
        GetDlgItem(IDC_EDIT_RELOADWINDFIELDFILE)->EnableWindow(TRUE);
    }
}
