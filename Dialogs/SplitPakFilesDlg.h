#pragma once

#include <afxtempl.h>

// CSplitPakFilesDlg dialog

namespace Dialogs {
    class CSplitPakFilesDlg : public CPropertyPage
    {
        DECLARE_DYNAMIC(CSplitPakFilesDlg)

    public:
        CSplitPakFilesDlg();
        virtual ~CSplitPakFilesDlg();

        // Dialog Data
        enum { IDD = IDD_PAKFILES_SPLIT };

        /** Initializes the dialog and its controls */
        virtual BOOL OnInitDialog();

        /** The output directory */
        CString	m_outputDir;

        /** The list of .pak files */
        CList <CString, CString&> m_pakFile;


    protected:
        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

        DECLARE_MESSAGE_MAP()

        /** Called to let the user browse for .pak files to split */
        afx_msg void OnBrowsePakFiles();

        /** Called to let the user browse for an output directory */
        afx_msg void OnBrowseOutputDir();

        /** Called when the user pressed the 'split files' button.
                Does the actual work in this dialog. */
        afx_msg void OnSplitFiles();

        /** Called when the user has edited the output-directory edit-box */
        afx_msg void OnChangeOutputDir();

        /** Called when the thread that's splitting the files made some progress */
        LRESULT OnProgress(WPARAM wp, LPARAM lp);

        /** Called when the thread that's splitting the files failed */
        LRESULT OnFailure(WPARAM wp, LPARAM lp);

        /** The number of pak-files in the list */
        long    m_pakFileNum;

        /** The 'split files' button */
        CButton m_btnSplitFiles;

        /** The progressbar */
        CProgressCtrl m_progressBar;

        /** The statusbar */
        CStatic	m_statusBar;
    };
}