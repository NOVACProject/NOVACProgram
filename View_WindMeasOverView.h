#pragma once

#include "Common/Common.h"

#include "EvaluatedDataStorage.h"
#include "Graphs/GraphCtrl.h"
#include "DlgControls/Label.h"

/** The <b>CView_WindMeasOverView</b>-class is a class that takes care of 
	showing the result from the wind-measurements made with the connected instruments. */

class CView_WindMeasOverView : public CPropertyPage
{
	DECLARE_DYNAMIC(CView_WindMeasOverView)

public:
	CView_WindMeasOverView();
	virtual ~CView_WindMeasOverView();

// Dialog Data
	enum { IDD = IDD_VIEW_WIND_OVERVIEW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	afx_msg LRESULT OnUpdateGraphs(WPARAM wParam, LPARAM lParam);

	class CLegend{
	public:
		DlgControls::CLabel	m_label;
		CStatic							m_text;
		COLORREF						m_color;
		CString							m_serial;
	};

public:
	/** A common object for doing common things */
	Common m_common;

	/** A pointer to a shared instance of 'CEvaluatedDataStorage' */
	CEvaluatedDataStorage *m_evalDataStorage;

	// --------------- EVENT HANDLERS ------------------------------

	/** Draws the wind-measurement-graphs */
	void DrawWindGraphs();

	/** Called when the overview page is created */
	virtual BOOL OnInitDialog();

	/** Called when the overview page is selected */
	virtual BOOL OnSetActive();

	/** For the tool tips */
	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
	// ----------- DIALOG CONTROLS ---------------

	/** The graph, shows the result of the wind-measurements */
	Graph::CGraphCtrl m_graph;

	/** The legend that tells which serial-number is connected with which color */
	CList<CLegend *, CLegend *> m_serialLegend;

	/** the tool tips */
	CToolTipCtrl  m_toolTip;

	// ----------- PROTECTED DATA ---------------

};
