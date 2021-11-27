#pragma once

#include "ConfigureCOMPortDlg.h"
#include "EvaluationConfigurationDlg.h"
#include "DarkConfigurationDlg.h"
#include "LocationConfigurationDlg.h"
#include "WindConfigurationDlg.h"
#include "VIIAdvancedConfigurationDlg.h"
#include "ConfigurationTreeCtrl.h"
#include "CCalibrationConfigurationDlg.h"
#include "afxwin.h"

namespace ConfigurationDialog
{
    // CScannerConfiguration dialog

    class CScannerConfiguration : public CPropertyPage
    {
        DECLARE_DYNAMIC(CScannerConfiguration)

    public:
        CScannerConfiguration();
        virtual ~CScannerConfiguration();

        // Dialog Data
        enum { IDD = IDD_CONFIGURATION_SCANNER };

    protected:
        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

        DECLARE_MESSAGE_MAP()
    public:
        CStatic m_frame;
        CPropertySheet m_sheet;

        /** Pointer to the configuration which we are modifying here. */
        CConfigurationSetting* m_configuration;

    private:
        /** The location configuration */
        CLocationConfigurationDlg m_pageLocation;

        /** the evaluation configuration */
        CEvaluationConfigurationDlg m_pageEvaluation[MAX_CHANNEL_NUM];

        /** The dark configuration */
        CDarkConfigurationDlg  m_pageDark;

        /** the automatic wind speed measurements page */
        CWindConfigurationDlg m_pageWind;

        /** Setting up automatic instrument calibration */
        CCalibrationConfigurationDlg m_pageCalibration;

        /** the advanced settings for the Version-II instrument */
        CVIIAdvancedConfigurationDlg m_pageVII;

        /** The communication configuration */
        CConfigureCOMPortDlg m_pageCommunication;

        /** The add scanner button */
        CButton m_addScannerBtn;

        /** The remove scanner button */
        CButton m_removeScannerBtn;

        /** Called when the selection of scanner has changed */
        void OnScannerSelectionChange(NMHDR* pNMHDR, LRESULT* pResult);

        /** Called when the user has clicked the button 'Add Scanner' */
        afx_msg void OnAddScanner();

        /** Called when the user has clicked the button 'Remove Scanner' */
        afx_msg void OnRemoveScanner();

    public:
        /** Used to update the window after the selection of scanner has changed */
        void OnChangeScanner();

        /** Called to fill in all the items in the scanner list */
        afx_msg void PopulateScannerList();

        /** Initializing the dialog */
        virtual BOOL OnInitDialog();

        /** Handling the tool tips */
        virtual BOOL PreTranslateMessage(MSG* pMsg);

        /** The scanner tree-list */
        CConfigurationTreeCtrl m_scannerTree;

        /** The tooltip control */
        CToolTipCtrl m_toolTip;

        /** Returns the index of the currently selected scanner and spectrometer. */
        void GetScannerAndSpec(int& curScanner, int& curSpec);

        /** m_showEvalPage[i] is true if m_pageEvaluation[i] is visible */
        bool m_showEvalPage[MAX_CHANNEL_NUM];

        /** m_showWindPage is true if the wind configration page is visible */
        bool m_showWindPage;

        /** m_showVIIPage is true if the Version-II configuration page is visible */
        //bool m_showVIIPage;
    };
}