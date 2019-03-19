#pragma once
#include "afxwin.h"
#include "../Graphs/GraphCtrl.h"
#include "../EvaluatedDataStorage.h"


// ColumnHistoryDlg dialog
class ColumnHistoryDlg : public CPropertyPage
{
	DECLARE_DYNAMIC(ColumnHistoryDlg)


public:
	ColumnHistoryDlg();   // standard constructor
	virtual ~ColumnHistoryDlg();

	virtual BOOL OnInitDialog();

	// Dialog Data
	enum { IDD = IDD_COLUMN_HISTORY_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnSetActive();


	/** A pointer to a shared instance of 'CEvaluatedDataStorage' */
	CEvaluatedDataStorage	*m_evalDataStorage;

	// scanner info
	int m_scannerIndex;
	CString m_serialNumber;
	CString m_siteName;

private:
	RECT minSize;


	// min and max column (Y-axis range)
	int m_minColumn;
	int m_maxColumn;

	// 10 day plot
	CStatic m_frame10;
	Graph::CGraphCtrl m_plot10;

	// 30 day plot
	CStatic m_frame30;
	Graph::CGraphCtrl m_plot30;

	void Init10DayPlot();
	void Init30DayPlot();
	void ReadEvalLogs();
};
