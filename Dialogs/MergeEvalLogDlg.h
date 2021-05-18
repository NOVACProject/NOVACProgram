#pragma once
#include "afxwin.h"

#include <afxtempl.h>

// CMergeEvalLogDlg dialog

namespace Dialogs
{
	class CMergeEvalLogDlg : public CDialog
	{
		DECLARE_DYNAMIC(CMergeEvalLogDlg)

	public:
		CMergeEvalLogDlg(CWnd* pParent = NULL);   // standard constructor
		virtual ~CMergeEvalLogDlg();

	// Dialog Data
		enum { IDD = IDD_DLG_EVALLOGS_MERGE };

	protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

		DECLARE_MESSAGE_MAP()

		/** The list of eval-log files to merge*/
		CList <CString, CString&> m_evalLogFile;
	public:
		CStatic m_inputListLabel;
		CEdit m_outputFileEdit;
		afx_msg void OnBrowseEvallogs();
		afx_msg void OnBrowseOutputFileEvallog();
		afx_msg void OnBnClickedMerge();
		afx_msg void OnBnClickedCancel();
	};
}