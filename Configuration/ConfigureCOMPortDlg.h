#pragma once

#include "SystemConfigurationPage.h"
#include "afxwin.h"

namespace ConfigurationDialog
{

    // CConfigureCOMPortDlg dialog

    class CConfigureCOMPortDlg : public CSystemConfigurationPage
    {
        DECLARE_DYNAMIC(CConfigureCOMPortDlg)

    public:
        CConfigureCOMPortDlg();
        virtual ~CConfigureCOMPortDlg();

        // Dialog Data
        enum { IDD = IDD_CONFIGURE_COM_PORT };

        /** Called when the current selection changes */
        virtual void OnChangeScanner() override;

        /** Saving the data in the control */
        afx_msg void SaveData();

        /** Changing the communication protocol */
        afx_msg void OnChangeMethod();

        /** Updating the data in the control */
        afx_msg void UpdateDlg();

        /** Handling the tool tips */
        virtual BOOL PreTranslateMessage(MSG* pMsg);

        /** Initializing the dialog */
        virtual BOOL OnInitDialog();

        /** Initializing the tooltips */
        void InitToolTips();

        /** Called when the current page becomes the active one */
        virtual BOOL OnSetActive();
    protected:
        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

        DECLARE_MESSAGE_MAP()

        /** The Connection Interval */
        int m_intervalHr;
        int m_intervalMin;
        int m_intervalSec;

        /** Sleep time*/
        int m_sleepHr;
        int m_sleepMin;
        int m_sleepSec;

        /** Wake up time*/
        int m_wakeHr;
        int m_wakeMin;
        int m_wakeSec;

        /** FTP - Hostname/IP-number */
        CString m_ftpHostName;

        /** FTP - Password */
        CString	m_ftpPassword;

        /** FTP - UserName */
        CString m_ftpUserName;

        CStatic m_connectionIntervalLabel;

        /** Polling directory */
        CString m_directory;

    private:

        // ------- The serial settings controls --------
        // The combo-boxes for the serial-settings
        CComboBox	m_comPort, m_baudrate, m_handshake;

        /** A list of the available baudrates */
        int m_availableBaudrates[5];

        /** The control for the timeout- edit */
        CEdit		m_editTimeOut;

        /** The time out */
        int m_timeout;

        /** The edit for callbooknumber/RadioID */
        CEdit m_editRadioID;

        /** The radio ID */
        CString m_radioID;

        // ------- The FTP settings controls --------

        /** The login information for the FTP-settings */
        CEdit m_editUserName;
        CEdit m_editPassword;
        CEdit m_editFtpHostname;

        // ------- Directory polling controls --------

        /** The directory to look for pak files in */
        CEdit m_editDirectory;
        CButton m_dirBrowse;

        // ------- The sleeping controls --------
        // The edits for the sleeping
        CEdit		m_editSleepHr, m_editSleepMin, m_editSleepSec;
        CEdit		m_editWakeHr, m_editWakeMin, m_editWakeSec;

        // ------- The labels controls --------
        // The labels, only used for tooltips
        CStatic	m_labelSleepFrom, m_labelSleepTo;
        CStatic m_label1, m_label2, m_label3, m_label4, m_label5, m_labelRadioID;

        /** The current selection of settings
                0 == Serial Cable
                1 == Freewave Serial Radio
                2 == FTP - Communication
                3 == Directory Polling
        */
        int m_curSetting = 2;

        /** Showing the settings for connecting through a serial cable */
        void ShowSerialCable();

        /** Showing the settings for connecting using a Freewave serial modem */
        void ShowFreewaveSerial();

        /** Showing the settings for connecting using FTP-Protocol */
        void ShowFTP();

        /** Showing the settings when polling directory */
        void ShowDirectoryPolling();
    public:
        afx_msg void OnBnClickedButtonPolDirBrowse();
    };
}