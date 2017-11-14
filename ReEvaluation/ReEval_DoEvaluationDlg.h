#pragma once

#include "ReEvaluator.h"
#include "afxwin.h"

#include "../Graphs/OScopeCtrl.h"
#include "../Graphs/DoasFitGraph.h"
#include "afxcmn.h"

// CReEval_DoEvaluationDlg dialog

UINT DoEvaluation(LPVOID pParam);

namespace ReEvaluation
{

	class CReEval_DoEvaluationDlg : public CPropertyPage
	{
		DECLARE_DYNAMIC(CReEval_DoEvaluationDlg)

	public:
		CReEval_DoEvaluationDlg();
		virtual ~CReEval_DoEvaluationDlg();

		// Dialog Data
		enum { IDD = IDD_REEVAL_FINAL };

	protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

		DECLARE_MESSAGE_MAP()

	public:
		// --------------------------- PUBLIC DATA -------------------------- 

		/** A handle to the reevaluator */
		CReEvaluator *m_reeval;

		/** Controlls wheather the total fit window should show the 
			total fit or the residual */
		int m_showFit;

		/** The listbox with the species which are used in the fit. Lets the user
			choose which reference to show */
		CListBox m_specieList;

		/** The currently selected specie in the specie list */
		long    m_curSpecie;

		/** The frame for the total fit graph */
		CStatic m_frameTotalfit;

		/** The frame for the reference fit graph */
		CStatic m_frameRefFit;

		/** The total fit graph */
		Graph::COScopeCtrl m_GraphTotal;

		/** The reference fit graph */
//		Graph::COScopeCtrl m_GraphRef;
		Graph::CDOASFitGraph m_GraphRef;


		// --------------------------- PUBLIC METHODS --------------------------

		/** Initializes the graphs */
		void  InitializeGraphs();

		/** Populates the reference list */
		void  PopulateRefList();

		/** Called to initialize the dialog */
		virtual BOOL OnInitDialog();

		/** Called when the dialog has been hidden and is shown again */
		virtual BOOL OnSetActive();

		/** Called when the user changes the selected specie in the species list */
		afx_msg void OnChangeSpecie();

		/** Called to redraw the 'total' graph */
		afx_msg void RedrawTotalFitGraph();

		/** The user has pressed the 'do Evaluation' button */
		afx_msg void OnDoEvaluation();

		/** The user has pressed the Cancel button */
		afx_msg void OnCancelEvaluation();

		/** The 'do Evaluation' button */
		CButton m_btnDoEval;

		/** The 'cancel' button */
		CButton m_btnCancel;

		/** The progress bar indicating which scan-file we're at */
		CProgressCtrl m_progressBar;

		/** The progress bar indicating which spectrum inside the .pak-file we're at */
		CProgressCtrl m_progressBar2;

		/** Draws the fitted reference */
		void  DrawReference();

		/** Draws the fitted spectrum */
		void  DrawFit();

		/** Draws the residual */
		void DrawResidual();

		/** Called when a spectrum has been evaluated */
		LRESULT OnEvaluatedSpectrum(WPARAM, LPARAM);

		/** Called when the reevaluator has something to say */
		LRESULT OnStatusUpdate(WPARAM, LPARAM);

		/** Called when the reevaluator is done with evaluating all the spectra */
		LRESULT OnDone(WPARAM wp, LPARAM lp);

		/** Called when the reevaluator has gone to sleep and is waiting for the user
			to continue. */
		LRESULT OnEvaluationSleep(WPARAM wp, LPARAM lp);

		/** Called when the reevaluator has made some progress in the evaluation
				of the scan-files */
		LRESULT OnProgress(WPARAM wp, LPARAM lp);

		/** Called when the reevaluator has made some progress in the evaluation 
				of the current scan-file */
		LRESULT OnProgress2(WPARAM wp, LPARAM lp);
	private:

		/** The reevaluation is run as a separate thread */
		CWinThread *pReEvalThread;  

		/** A local copy of the residual of the last fit */
		double residual[MAX_SPECTRUM_LENGTH];

		/** A local copy of the last fit result */
		double fitResult[MAX_N_REFERENCES][MAX_SPECTRUM_LENGTH];

		/** A local copy of the last read spectrum */
		double spectrum[MAX_SPECTRUM_LENGTH];

		/** A pointer to the result-set */
		Evaluation::CScanResult *m_result;

	public:
		afx_msg void OnBnClickedReevalCheckPause();
	};
}