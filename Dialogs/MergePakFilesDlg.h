#pragma once

#include <afxtempl.h>

// CMergePakFilesDlg dialog

namespace Dialogs{
	class CMergePakFilesDlg : public CPropertyPage
	{
		DECLARE_DYNAMIC(CMergePakFilesDlg)

	public:
		CMergePakFilesDlg();
		virtual ~CMergePakFilesDlg();

	// Dialog Data
		enum { IDD = IDD_PAKFILES_MERGE };

    /** Initializes the dialog and its controls */
    virtual BOOL OnInitDialog();

		/** The output file */
		CString	m_outputFile;

		/** The list of .pak files to merge*/
		CList <CString, CString&> m_pakFile;

		/** How to merge the scans, put them after each other or combine
					several scans into one? */
		int			m_mergeOption;

		/** If the scans are to be combined, how many scans should there
						be in one saved scan? */
		int			m_mergeNum;

		/** The maximum number of scans that can be combined together */
		static const int	MAX_SCANS_COMBINATIONS = 16;
	protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

		DECLARE_MESSAGE_MAP()

		/** Called to let the user browse for .pak files to merge */
		afx_msg void OnBrowsePakFiles();

		/** Called to let the user browse for an output file */
		afx_msg void OnBrowseOutputFile();

		/** Called when the user pressed the 'merge files' button. 
				Does the actual work in this dialog. */
		afx_msg void OnMergeFiles();

		/** Called when the user has edited the output-file edit-box */
		afx_msg void OnChangeOutputFile();

		/** Called when the thread that's merging the files made some progress */
		LRESULT OnProgress(WPARAM wp, LPARAM lp);

		/** Called when the thread that's merging the files failed */
		LRESULT OnFailure(WPARAM wp, LPARAM lp);

		/** The number of pak-files in the list */
    long    m_pakFileNum;

		/** The order in which to sort the files */
		int			m_sortOrder;

		/** The 'merge files' button */
		CButton m_btnMergeFiles;

		/** The progressbar */
		CProgressCtrl m_progressBar;

		/** The statusbar */
		CStatic	m_statusBar;
	};
}