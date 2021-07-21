#pragma once
#include "afxwin.h"

#include "ReEvaluator.h"
#include "afxcmn.h"

//#include "../Graphs/OScopeCtrl.h"
#include "../Graphs/SpectrumGraph.h"
#include "PakFileListBox.h"

#include <SpectralEvaluation/Spectra/Spectrum.h>

// CReEval_ScanDlg dialog

namespace ReEvaluation
{

	class CReEval_ScanDlg : public CPropertyPage
	{
		DECLARE_DYNAMIC(CReEval_ScanDlg)

	public:
		CReEval_ScanDlg();
		virtual ~CReEval_ScanDlg();

	// Dialog Data
		enum { IDD = IDD_REEVAL_SCANFILES };

	protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

		DECLARE_MESSAGE_MAP()
	public:

		// --------------------------- PUBLIC DATA -------------------------- 

		/** The scan file list */
		CPakFileListBox m_scanfileList;

		/** A handle to the reevaluator */
		CReEvaluator *m_reeval;

		/** The frame for the spectrum graph */
		CStatic m_graphFrame;

		/** The spectrum graph */
		Graph::CSpectrumGraph m_specGraph;

		/** The spin button, controlls which spectrum to show */
		CSpinButtonCtrl m_specSpin;

		/** The currently selected spectrum-file */
		long    m_curSpecFile;

		/** The currently selected spectrum inside the selected spectrum-file */
		long    m_curSpec;

		/** The number of spectra inside the selected spectrum file */
		long    m_specNum;

		/** The currently selected spectrum */
		novac::CSpectrum m_spectrum;

		// --------------------------- PUBLIC METHODS --------------------------

		/** Initializes the dialog and its controls */
		virtual BOOL OnInitDialog();

		/** When the user presses the 'browse' button. Opens a dialog and lets the user
			select one or more spectrum files that will be loaded into the program. */
		afx_msg void OnBnClickedBtnBrowsescanfile();

		/** Called when the user selects a new spectrum file to look at. */
		afx_msg void OnLbnSelchangeScanfileList();

		/** Called when the user wants to remove the currently selected scan */
		afx_msg void OnRemoveSelected();

		/** Called when the user has pressed the spin control and wants 
			to see the next or the previous spectrum */
		afx_msg void OnChangeSpectrum(NMHDR *pNMHDR, LRESULT *pResult);

		/** Draws the currently selected spectrum from the currently selected spectrum-file
			into the spectrum graph. */
		afx_msg void  DrawSpectrum();

		/** Tries to read the desired spectrum from the spectrum file */
		int TryReadSpectrum();

		/** Scans through the selected scan-file to check how many spectra there are */
		int CheckScanFile();

		/** Update the information labels */
		void  UpdateInfo();

	protected:
		// --------------------------- PROTECTED METHODS --------------------------

		/** Gets the range of the plot */
		void GetPlotRange(Graph::CSpectrumGraph::plotRange& range);

		/** Zooming in the graph */
		LRESULT OnZoomGraph(WPARAM wParam, LPARAM lParam);
	};
}