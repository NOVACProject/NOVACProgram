#pragma once
#include "../Graphs/GraphCtrl.h"


// ColumnHistoryDlg dialog
class ColumnHistoryDlg : public CDialog
{
	DECLARE_DYNAMIC(ColumnHistoryDlg)

private:
	RECT minSize;

	// scanner drop down;
	CComboBox m_scanners;

	// 10 day plot
	CStatic m_frame10;
	Graph::CGraphCtrl m_plot10;

	// 30 day plot
	CStatic m_frame30;
	Graph::CGraphCtrl m_plot30;

	virtual BOOL ColumnHistoryDlg::OnInitDialog();

	void Init10DayPlot();
	void Init30DayPlot();
	void PopulateScannerList();
	void ReadEvalLogs();
	void DrawColumns();

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
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClose();;
	afx_msg void UpdatePlots();
};
