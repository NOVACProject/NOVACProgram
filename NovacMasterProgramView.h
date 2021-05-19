// NovacMasterProgramView.h : interface of the CNovacMasterProgramView class
//

#include "afxwin.h"

#include "MasterController.h"
#include "Configuration/Configuration.h"
#include "Configuration/EvaluationConfigurationDlg.h"
#include "Dialogs/ConfigurationDlg.h"
#include "Dialogs/ColumnHistoryDlg.h"
#include "Dialogs/FluxHistoryDlg.h"
#include "EvaluatedDataStorage.h"
#include "CommunicationDataStorage.h"
#include "Meteorology/MeteorologicalData.h"
#include "View_OverView.h"
#include "View_WindMeasOverView.h"
#include "View_Instrument.h"
#include "View_Scanner.h"

#include "afxcmn.h"

#pragma once


class CNovacMasterProgramView : public CFormView
{
protected: // create from serialization only
	CNovacMasterProgramView();
	virtual ~CNovacMasterProgramView();

	DECLARE_DYNCREATE(CNovacMasterProgramView)

public:
	//{{AFX_DATA(CNovacMasterProgramView)
	enum{ IDD = IDD_NOVACMASTERPROGRAM_FORM };
	//}}AFX_DATA

// Attributes
	CNovacMasterProgramDoc* GetDocument() const;

// Overrides
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnInitialUpdate(); // called first time after construct


// Implementation
public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	afx_msg LRESULT OnShowStatus(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnShowMessage(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUpdateMessage(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEvalSucess(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEvalFailure(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnCorrelationSuccess(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnPlumeHeightSuccess(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnNewWindField(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnScannerRun(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnScannerSleep(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnScannerNotConnect(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDownloadFinished(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUploadFinished(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnWriteReport(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnRewriteConfigurationXml(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()

private:
	/** A handle to the master controller which is the actual program */
	CMasterController m_controller;

	/** A common object for doing common things */
	Common m_common;

	/** This class contains critical sections of code */
	CCriticalSection m_critSect;

public:
	// --------------- EVENT HANDLERS ------------------------------
	/** Event handlers */
	afx_msg void OnMenuSetLanguageEnglish();
	afx_msg void OnMenuSetLanguageSpanish();
	afx_msg void OnUpdateSetLanguageEnglish(CCmdUI *pCmdUI);
	afx_msg void OnUpdateSetLanguageSpanish(CCmdUI *pCmdUI);
	afx_msg void OnMenuStartMasterController();
	afx_msg void OnUpdateStart(CCmdUI *pCmdUI);
	afx_msg void OnUpdateFileTransfer(CCmdUI *pCmdUI);
	afx_msg void OnUpdateMakeWindMeasurement(CCmdUI *pCmdUI);
	afx_msg void OnUpdateMakeCompositionMeasurement(CCmdUI *pCmdUI);
	afx_msg void OnMenuMakeWindMeasurement();
	afx_msg void OnMenuMakeCompositionMeasurement();
	afx_msg void OnMenuShowConfigurationDialog();
	afx_msg void OnMenuViewInstrumentTab();
	afx_msg void OnDestroy();
	afx_msg void OnMenuAnalysisFlux();
	afx_msg void OnMenuAnalysisReevaluate();
	afx_msg void OnMenuAnalysisSetup();
	afx_msg void OnMenuAnalysisWind();
	afx_msg void OnMenuAnalysisBrowseData(); 
	afx_msg void OnMenuFileExport();
	afx_msg void OnMenuFileImport();
	afx_msg void OnMenuFileCheckPakFile();
	afx_msg void OnMenuFileMergeEvaluationLogs();
	afx_msg void OnMenuFileSplitMergePak();
	afx_msg void OnMenuConfigurationFileTransfer();
	afx_msg void OnChangeUnitOfFluxToKgS();
	afx_msg void OnChangeUnitOfFluxToTonDay();
	afx_msg void OnChangeUnitOfColumnToPPMM();
	afx_msg void OnChangeUnitOfColumnToMolecCm2();
	afx_msg void OnUpdateChangeUnitOfFluxToKgS(CCmdUI *pCmdUI);
	afx_msg void OnUpdateChangeUnitOfFluxToTonDay(CCmdUI *pCmdUI);
	afx_msg void OnUpdateChangeUnitOfColumnToPPMM(CCmdUI *pCmdUI);
	afx_msg void OnUpdateChangeUnitOfColumnToMolecCm2(CCmdUI *pCmdUI);
	afx_msg void OnUpdateMenuSummarizeFluxData(CCmdUI *pCmdUI);

	
	virtual BOOL PreTranslateMessage(MSG* pMsg); // for handling the tool tips

	// --------------- DIALOG COMPONENTS ------------------------------
	// the status messages
	CListBox m_statusListBox;
	CStatic	 m_statusFrame;

	// the tool tips
	CToolTipCtrl  m_toolTip;

	// The property pages
	CArray<CView_Scanner *, CView_Scanner *> m_scannerPages;

	// The column history pages
	CArray<ColumnHistoryDlg *, ColumnHistoryDlg *> m_colHistoryPages;

	// The flux history pages
	CArray<FluxHistoryDlg *, FluxHistoryDlg *> m_fluxHistoryPages;

	// The overview page
	CView_OverView					*m_overView;

	// The wind-measurements overview page
	CView_WindMeasOverView	*m_windOverView;
	bool										m_showWindOverView;

	// The instrument page
	CView_Instrument				*m_instrumentView;
	bool										m_instrumentViewVisible; // true if the Instrument-page is shown

	// The property sheet, holds the property pages
	CPropertySheet	m_sheet;

	/** */
	CStatic m_masterFrame;
private:
	// --------------- DATA STRUCTURES FOR SHOWING THE EVALUATION RESULT -------------

	/** This object holds all the evaluated data that we need for plotting */
	CEvaluatedDataStorage *m_evalDataStorage;

	/** This object holds the communication status */
	CCommunicationDataStorage *m_commDataStorage;

	// ---------------------- AUXILLIARY FUNCTIONS -----------------------

	/** Called when the configuration file has been read */
	int InitializeControls();

	/** Read flux log */
	void ReadFluxLog(int scannerIndex, CString dateStr, CString serialNumber);

	/** Read eval log */
	void ReadEvalLog(int scannerIndex, CString dateStr, CString serialNumber);

	/** Forwards messages to the different views */
	void ForwardMessage(int message, WPARAM wParam, LPARAM lParam);

	/** Uploads auxiliary data to the FTP-Server */
	void	UploadAuxData();

	/** Scan the last status-log file for interesting data */
	void	ScanStatusLogFile();


public:
	afx_msg void OnUpdateMenuViewInstrumenttab(CCmdUI *pCmdUI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnMenuFileCreatereferences();
    afx_msg void OnFileCalibratespectrometer();
    afx_msg void OnUpdateFileCalibratespectrometer(CCmdUI *pCmdUI);
};

#ifndef _DEBUG  // debug version in NovacMasterProgramView.cpp
inline CNovacMasterProgramDoc* CNovacMasterProgramView::GetDocument() const
   { return reinterpret_cast<CNovacMasterProgramDoc*>(m_pDocument); }
#endif

