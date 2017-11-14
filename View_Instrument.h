#pragma once

#include "Common/Common.h"

#include "EvaluatedDataStorage.h"
#include "CommunicationDataStorage.h"
#include "Graphs/GraphCtrl.h"
#include "DlgControls/Label.h"

/** The <b>CView_Instrument</b>-class is a class that takes care of 
	showing the hardware status of the connected instruments */

class CView_Instrument : public CPropertyPage
{
	DECLARE_DYNAMIC(CView_Instrument)

public:
	CView_Instrument();
	virtual ~CView_Instrument();

// Dialog Data
	enum { IDD = IDD_VIEW_INSTRUMENT_OVERVIEW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	afx_msg LRESULT OnUpdateGraphs(WPARAM wParam, LPARAM lParam);

	class CLegend{
	public:
		DlgControls::CLabel	m_label;
		CStatic		m_text;
		COLORREF	m_color;
		CString		m_serial;
	};

public:
	/** A common object for doing common things */
	Common m_common;

	/** A pointer to a shared instance of 'CEvaluatedDataStorage' */
	CEvaluatedDataStorage *m_evalDataStorage;

	/** A pointer to a shared instance of 'CCommunicationDataStorage' */
	CCommunicationDataStorage *m_commDataStorage;

	// --------------- EVENT HANDLERS ------------------------------

	/** Draws the graphs */
	void DrawGraphs();

	/** Called when the overview page is created */
	virtual BOOL OnInitDialog();

	/** Called when the overview page is selected */
	virtual BOOL OnSetActive();

	virtual BOOL PreTranslateMessage(MSG* pMsg); // for handling the tool tips

protected:
	// ----------- DIALOG CONTROLS ---------------

	/** The graphs, one for the temperature, one for the battery voltage 
			and one for the exposure times */
	Graph::CGraphCtrl			m_batteryGraph;
	Graph::CGraphCtrl			m_temperatureGraph;
	Graph::CGraphCtrl			m_expTimeGraph;
	Graph::CGraphCtrl			m_linkSpeedGraph;

	/** The legend that tells which serial-number is connected with which color */
	CList<CLegend *, CLegend *> m_serialLegend;

	// the tool tips
	CToolTipCtrl  m_toolTip;

	// --------------- EVENT HANDLERS ------------------------------

	/** Draws the temperature graph */
	void DrawTempGraph();

	/** Draws the battery-voltage graph */
	void DrawBatteryGraph();

	/** Draws the exposure-time graph */
	void DrawExpTimeGraph();

	/** Draws the link-speed graph */
	void DrawLinkSpeedGraph();
};
