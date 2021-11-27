#pragma once

#include "../DlgControls/ReferenceFileControl.h"
#include "SystemConfigurationPage.h"

// CEvaluationConfiguration dialog

namespace ConfigurationDialog
{
    class CEvaluationConfigurationDlg : public CSystemConfigurationPage
    {
        DECLARE_DYNAMIC(CEvaluationConfigurationDlg)

    public:
        CEvaluationConfigurationDlg();
        virtual ~CEvaluationConfigurationDlg();

        // Dialog Data
        enum { IDD = IDD_CONFIGURE_EVALUATION };

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

        /** The spectrometer channel that this object is modifying */
        unsigned int m_channel;

        /** The 'add a reference file' button */
        CButton m_addReferenceBtn;

        /** The 'remove  a reference file' button */
        CButton m_removeReferenceButton;

        /** Adding a reference file */
        afx_msg void OnAddReferenceFile();

        /** Removing a reference file */
        afx_msg void OnRemoveReferenceFile();

        /** Showing the properties of a reference file */
        afx_msg void OnShowPropertiesWindow();

        /** Showing the fit graphs of reference files */
        afx_msg void OnShowReferenceGraph();

        /** Saving the data in the control */
        afx_msg void SaveData();

        /** Updating the data in the control */
        void UpdateDlg();

        /** Updates the control */
        virtual void OnChangeScanner() override;

        virtual BOOL OnSetActive();

    private:
        CStatic m_referenceStatic;

        CReferenceFileControl m_referenceFileCtrl;

        /** Initialize the reference file control */
        void InitReferenceFileControl();

        /** Fill up the reference file control with items */
        void PopulateReferenceFileControl();

        // The labels on the screen.
        CStatic m_labelFitFrom;
        CStatic m_labelFitTo;

        // The edit controls on the screen.
        CStatic m_editFitFrom;
        CStatic m_editFitTo;

        /** The fitlow and fit high, from the configuration */
        int m_fitLow, m_fitHigh;
    };
}