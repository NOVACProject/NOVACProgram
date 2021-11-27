#pragma once

#include "../Common/Common.h"
#include "SystemConfigurationPage.h"

// CDarkConfigurationDlg dialog

namespace ConfigurationDialog
{
    class CDarkConfigurationDlg : public CSystemConfigurationPage
    {
        DECLARE_DYNAMIC(CDarkConfigurationDlg)

    public:
        CDarkConfigurationDlg();
        virtual ~CDarkConfigurationDlg();

        // Dialog Data
        enum { IDD = IDD_CONFIGURE_DARK };

    protected:
        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

        DECLARE_MESSAGE_MAP()
    public:
        /** Initializing the dialog */
        virtual BOOL OnInitDialog();

        /** Setup the tool tips */
        void InitToolTips();

        /** Called when this page is no longer the active page */
        virtual BOOL OnKillActive();

        /** Handling the tool tips */
        virtual BOOL PreTranslateMessage(MSG* pMsg);

        /** Saving the data in the dialog */
        afx_msg void SaveData();

        /** Updating the data in the dialog */
        afx_msg void UpdateDlg();

        /** Updating the status of the controls */
        afx_msg void UpdateControls();

        /** Called when the user has changed the currently selected scanner */
        virtual void OnChangeScanner() override;

        /** Caled when the user has changed the way the dark-spectrum
                should be collected */
        void OnChangeDarkSpecOption();

        // The 'Browse' buttons
        afx_msg void OnBrowseOffset1();
        afx_msg void OnBrowseOffset2();
        afx_msg void OnBrowseDC1();
        afx_msg void OnBrowseDC2();

        /** Browsing for a file, the result will be saved in the edit box 'editbox' */
        void	BrowseFile(int editBox);

    private:
        // ----------------------------------------------------------
        // ------------------ PRIVATE DATA --------------------------
        // ----------------------------------------------------------

        /** True when all the things in the dialog have been initialized */
        bool	m_initialized;

        /** The option for how to get the dark spectrum */
        int		m_darkSpecOption;

        /** The option for how to use the offset spectrum */
        int		m_offsetSpecOption;

        /** The option for how to use the dark-current spectrum */
        int		m_dcSpecOption;

        /** The paths for the offset spectra */
        CString	m_offsetPath1, m_offsetPath2;

        /** The paths for the dark-current spectra */
        CString	m_dcPath1, m_dcPath2;

        // ---------------------------------------------------------------
        // ------------------ DIALOG COMPONENTS --------------------------
        // ---------------------------------------------------------------

        /** The labels */
        CStatic m_labelMaster1, m_labelMaster2;
        CStatic m_labelSlave1, m_labelSlave2;
        CStatic	m_labelDarkOffsetCorr;

        /** The edit-boxes */
        CStatic	m_editOffset1, m_editOffset2;
        CStatic m_editDC1, m_editDC2;

        /** The combo-boxes */
        CComboBox	m_comboOffset, m_comboDC;

        /** The buttons */
        CButton		m_btnMasterOff, m_btnSlaveOff;
        CButton		m_btnMasterDC, m_btnSlaveDC;
    };
}