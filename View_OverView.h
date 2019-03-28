#pragma once

#include "Common/Common.h"

#include "EvaluatedDataStorage.h"
#include "CommunicationDataStorage.h"

#include "Graphs/GraphCtrl.h"

/** The <b>CView_OverView</b>-class is a class that takes care of showing the result
	from and the status of all the scanners in the main window of the program.
	This gives a more overview look than the CView_Scanner-pages which goes more into detail*/

class CView_OverView : public CPropertyPage
{
	DECLARE_DYNAMIC(CView_OverView)

public:
	CView_OverView();
	virtual ~CView_OverView();

// Dialog Data
	enum { IDD = IDD_VIEW_OVERVIEW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	afx_msg LRESULT OnUpdateFluxes(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()

public:
	/** A common object for doing common things */
	Common m_common;

	/** A pointer to a shared instance of 'CEvaluatedDataStorage' */
	CEvaluatedDataStorage *m_evalDataStorage;

	/** A pointer to a shared instance of 'CCommunicationDataStorage' */
	CCommunicationDataStorage *m_commDataStorage;

	// --------------- EVENT HANDLERS ------------------------------

	/** Draws the flux-graphs */
	void DrawFlux();

	/** Called when the overview page is created */
	virtual BOOL OnInitDialog();

	/** Called when the overview page is selected */
	virtual BOOL OnSetActive();

protected:
	// ----------- DIALOG CONTROLS ---------------

	/** The graphs */
	CArray<Graph::CGraphCtrl *, Graph::CGraphCtrl *> m_graphs;

	/** The label that tells the name and location of each system */
	CArray<CStatic *, CStatic *> m_specLabel;

	/** The label that tells the status of each system */
	CArray<CStatic *, CStatic *> m_statusLabel;

	/** The label that tells the average flux of each system */
	CArray<CStatic *, CStatic *> m_fluxLabel;

private:
	/** Set the graph's time range */
	void SetTodayRange(Graph::CGraphCtrl *graph);
};
