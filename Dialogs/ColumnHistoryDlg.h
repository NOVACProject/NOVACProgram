#pragma once


// ColumnHistoryDlg dialog

class ColumnHistoryDlg : public CDialog
{
	DECLARE_DYNAMIC(ColumnHistoryDlg)

public:
	ColumnHistoryDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~ColumnHistoryDlg();

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_COLUMN_HISTORY_DLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};