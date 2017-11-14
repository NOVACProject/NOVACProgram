#pragma once

#include "../Graphs/GraphCtrl.h"
#include "../DlgControls/Label.h"
#include "../PostFlux/PostFluxCalculator.h"
#include "afxwin.h"

// CDataBrowserDlg dialog

namespace Dialogs{
	class CDataBrowserDlg : public CDialog
	{
		DECLARE_DYNAMIC(CDataBrowserDlg)

		typedef struct Show{
			bool  peakIntensity;
			bool	fitIntensity;
			bool  columnError;
			bool  delta;
			bool	chiSquare;
		}Show;

		typedef struct Colors{
			COLORREF  peakIntensity;
			COLORREF	fitIntensity;
			COLORREF  column;
			COLORREF	badColumn;
			COLORREF  delta;
			COLORREF  chiSquare;
			COLORREF  offset;
		}Colors;

	public:
		CDataBrowserDlg(CWnd* pParent = NULL);   // standard constructor
		virtual ~CDataBrowserDlg();

	// Dialog Data
		enum { IDD = IDD_DATA_BROWSE_DLG };

	protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

		DECLARE_MESSAGE_MAP()

	public:

		// ---------------------- CONSTANTS ---------------------------

		// ------------------ DIALOG COMPONENTS -------------------------

		/** The graph showing the measured columns */
		Graph::CGraphCtrl  m_scanGraph;

		/** The bounding frame for the column - graph */
		CStatic m_scanGraphFrame;

		/** The graph showing the information of the fluxes */
		Graph::CGraphCtrl  m_fluxGraph;

		/** The bounding frame for the flux - graph */
		CStatic m_fluxGraphFrame;

		/** The bounging frame for the scanInformation list */
		CStatic m_scanInfoFrame;

		/** The spin control, which lets the user select which scan to show. */
		CSpinButtonCtrl m_scanSpinCtrl;

		// ------------------- DATA -------------------------

		/** The post flux calculator */
		PostFlux::CPostFluxCalculator *m_calculator;

		/** The currently selected scan */
		int           m_curScan;

		/** The currently selected specie (if there are several in the evaluationlog) */
		int           m_curSpecie;

		/** The volcano-index of volcano in the evaluation-log file (-1 if unknown) */
		int						m_volcanoIndex;

		// ------------------ METHODS ----------------------- 

		/** Initialzing the dialog */
		virtual BOOL OnInitDialog();

		/** Intitializing the plot-legend */
		void	InitLegend();

		/** Called when the user wants to browse for, and open a new evaluation log */
		afx_msg void OnBrowseEvallog();

		/** Draws the currently selected scan */
		void  DrawScan();

		/** Draws the fluxes up to the currently selected scan */
		void  DrawFlux();

		/** Updates the information about the currently selected scan */
		void UpdateScanInfo();

		/** Initializes the controls of the dialog according to the information in
			the most recently read evaluation log. */
		void InitializeControls();

		afx_msg void OnViewPeakIntensity();
		afx_msg void OnViewFitIntensity();
		afx_msg void OnViewColumnError();
		afx_msg void OnViewDelta();
		afx_msg void OnViewChiSquare();

		/** Called when the user presses one of the buttons of the m_scanSpinCtrl,
				leads to the selection of another scan with redrawing of the controls. */
		afx_msg void OnChangeSelectedScan(NMHDR *pNMHDR, LRESULT *pResult);

		/** Called when the user wants to see the next scan */
		afx_msg void OnShowPreviousScan();

		/** Called when the user wants to go back one scan */
		afx_msg void OnShowNextScan();

	protected:
		// ------------------ PROTECTED DIALOG COMPONENTS -------------------------
		DlgControls::CLabel m_legendColumn;
		DlgControls::CLabel m_legendPeakIntensity;
		DlgControls::CLabel m_legendFitIntensity;
		DlgControls::CLabel m_legendDelta;
		DlgControls::CLabel m_legendChiSquare;
		CStatic m_labelColumn;
		CStatic m_labelPeakIntensity;
		CStatic m_labelFitIntensity;
		CStatic m_labelDelta;
		CStatic m_labelChiSquare;

		/** The list of the properties of the shown scan */
		CListCtrl m_scanInfoList;

		// ------------------ PROTECTED DATA -------------------------

		/** The options for what to show in the graph */
		Show    m_show;

		/** The options for the colors of the graph */
		Colors  m_color;

	};
}