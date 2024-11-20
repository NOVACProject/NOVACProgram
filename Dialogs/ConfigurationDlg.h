#pragma once
#include "afxcmn.h"
#include "afxwin.h"

#include "../Configuration/GlobalConfiguration.h"
#include "../Configuration/ScannerConfiguration.h"
#include "../Configuration/Configuration.h"

// CConfigurationDlg dialog

namespace ConfigurationDialog
{

class CConfigurationDlg : public CDialog
{
    DECLARE_DYNAMIC(CConfigurationDlg)

public:
    CConfigurationDlg(CWnd* pParent = NULL);   // standard constructor
    virtual ~CConfigurationDlg();

    // Dialog Data
    enum { IDD = IDD_CONFIGURATION_DIALOG };

    virtual BOOL OnInitDialog();

    CPropertySheet m_sheet;

    CStatic m_frame;

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()

    /** When the user presses the OK - button.
        Writes the updated configuration file to disk. */
    virtual void OnOK();

private:
    /** the global configuration */
    CGlobalConfiguration m_pageGlobal;

    /** the scanner-specific configuration */
    CScannerConfiguration m_pageScanner;

    /** the actual configuration */
    CConfigurationSetting m_configuration;

    /** This function goes through the settings and checks their reasonability */
    RETURN_CODE CheckSettings();

};

}