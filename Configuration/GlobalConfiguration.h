#pragma once

#include "Configuration.h"

namespace ConfigurationDialog
{

// CGlobalConfiguration dialog

class CGlobalConfiguration : public CPropertyPage
{
    DECLARE_DYNAMIC(CGlobalConfiguration)

public:
    CGlobalConfiguration();
    virtual ~CGlobalConfiguration();

    // Dialog Data
    enum { IDD = IDD_CONFIGURATION_GLOBAL };

    /** the actual configuration */
    CConfigurationSetting* m_configuration;

    /** Browsing for a new output directory */
    afx_msg void OnBrowseOutputPath();

    /** Browsing for a wind-field file */
    afx_msg void OnBrowseWindFieldFile();

    /** Update whether to read wind field file or not */
    afx_msg void OnChangeWindFileUpdate();

    /** Opening the dialog for advanced FTP-options */
    afx_msg void OnAdvancedFTPSettings();

    /** Saves the data in the form */
    afx_msg void SaveData();

    /** Called when the user changes whether we should publish the data or not */
    afx_msg void OnChangePublish();

    /** Handling the tool tips */
    virtual BOOL PreTranslateMessage(MSG* pMsg);

    /** Initializing the dialog */
    virtual BOOL OnInitDialog();

    /** Setup the tool tips */
    void InitToolTips();
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()

    /** the tool tips */
    CToolTipCtrl  m_toolTip;

public:
    // The labels (only used for the tooltips)
    CStatic m_outputDirectory;
    CStatic m_startup;
    CStatic m_ftpUploadServer;
    CStatic m_ftpURLLabel;
    CStatic m_ftpUserNameLabel;
    CStatic m_ftpPasswordLabel;

    /** Controlling the behaviour of the output-edit */
    CEdit m_outputDir;

    /** Controlling the behaviour of the ftp-url - edit */
    CEdit m_ftpURL;

    /** Controlling the behaviour of the ftp-url - username */
    CEdit m_ftpUserName;

    /** Controlling the behaviour of the ftp-url - password */
    CEdit m_ftpPassword;

    /** Controlling the behaviour of the image-format combo-box */
    CComboBox m_imageFormatCombo;

    /** Controlling the behaviour of the local-directory edit */
    CEdit m_localDir;

    /** Controlling the behaviour of the local/ftp radio-button */
    int     m_useLocalDirectory;
};
}