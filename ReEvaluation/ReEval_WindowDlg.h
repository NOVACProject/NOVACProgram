#pragma once

#include "../DlgControls/ReferenceFileControl.h"
#include "FitWindowListBox.h"
#include "afxwin.h"

// CReEval_WindowDlg dialog

namespace ReEvaluation
{

	class CReEval_WindowDlg : public CPropertyPage 
	{
		DECLARE_DYNAMIC(CReEval_WindowDlg)

	public:
		CReEval_WindowDlg();
		virtual ~CReEval_WindowDlg();

		// Dialog Data
		enum { IDD = IDD_REEVAL_WINDOW };

    protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

		DECLARE_MESSAGE_MAP()

	public:

		// --------------------------- PUBLIC DATA -------------------------- 

		/** A handle to the reevaluator */
		CReEvaluator *m_reeval;

		/** The reference grid, enables the user to select reference files */
		DlgControls::CReferenceFileControl m_referenceGrid;

		/** The fit window list */
		CFitWindowListBox m_windowList;

		/** The electronic combo-box */
		CComboBox m_electronicCombo;

		/** The instrument-type combo-box */
		//CComboBox m_instrumentTypeCombo;

		/** The frame for defining the size of the reference file control */ 
		CStatic m_referenceFrame;

		/** The properties button */
		CButton m_btnRefProp;

		/** The insert new reference button */
		CButton m_btnInsertRef;

		/** The remove reference button */
		CButton m_btnRemoveRef;

		/** The 'shift-sky' checkbox */
		CButton m_checkShiftSky;

		/** The 'find optimal shift' checkbox */
		CButton m_checkFindOptimalShift;

		// --------------------------- PUBLIC METHODS --------------------------

		/** Initialize the dialog and its controls */
		virtual BOOL OnInitDialog();

		/** Initialize the reference file control */
		void InitReferenceFileControl();

		/** Populate the reference file control */
		void PopulateReferenceFileControl();

		/** Updates the controls */
		void UpdateControls();

		/** Called when the user has changed the selected fit window */
		afx_msg void OnChangeFitWindow();

		/** Called when the user has changed the instrument type */
		afx_msg void OnChangeInstrumentType();

		/** Called when the user wants to insert a new reference file */
		afx_msg void OnInsertReference();

		/** Called when the user wants to remove a reference file */
		afx_msg void OnRemoveReference();

		/** Called when the user wants to inspect the properties of a reference file */
		afx_msg void OnShowPropertiesWindow();

		/** Called when the user wants to view reference graph */
		afx_msg void OnShowReferenceGraph();

		/** Saves the data the user typed in to the user interface */
		afx_msg void SaveData();

		/** Lets the user browse for a solar-spectrum */
		afx_msg void OnBrowseSolarSpectrum();

    private:
        CString m_fraunhoferReferenceName;
	};
}