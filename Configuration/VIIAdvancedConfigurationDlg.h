#pragma once

#include "SystemConfigurationPage.h"

// CVIIAdvancedConfigurationDlg dialog
namespace ConfigurationDialog
{

    class CVIIAdvancedConfigurationDlg : public CSystemConfigurationPage
    {
        DECLARE_DYNAMIC(CVIIAdvancedConfigurationDlg)

    public:
        CVIIAdvancedConfigurationDlg();
        virtual ~CVIIAdvancedConfigurationDlg();

        // Dialog Data
        enum { IDD = IDD_CONFIGURE_VII_ADVANCED };

    protected:
        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

        DECLARE_MESSAGE_MAP()

    public:
        /** Setup the tool tips */
        void InitToolTips();

        /** Initializing the dialog */
        virtual BOOL OnInitDialog();

        /** Handling the tool tips */
        virtual BOOL PreTranslateMessage(MSG* pMsg);

        /** Called when the user has changed the currently selected scanner */
        virtual void OnChangeScanner() override;

        /** Called when the automatic measurements have been turned on/off */
        afx_msg void OnOnOff();

        /** Saves the data in the dialog */
        afx_msg void SaveData();

    protected:

        // ----------------------- DIALOG DATA ------------------------

        /** the local copies of the settings */
        int    m_automaticSetupChange;
        int    m_useCalculatedPlumeParameters;
        double m_windDirectionTolerance;
        int    m_mode;

        // ----------------------- DIALOG CONTROLS ------------------------
        CEdit   m_editTolerance;
        CButton m_radioMode1, m_radioMode2;
        CButton m_checkUseAutoValues;
    };
}