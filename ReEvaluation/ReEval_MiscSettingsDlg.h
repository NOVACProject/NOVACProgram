#pragma once

#include "ReEvaluator.h"

namespace ReEvaluation
{

	// CReEval_MiscSettingsDlg dialog

	class CReEval_MiscSettingsDlg : public CPropertyPage
	{
		DECLARE_DYNAMIC(CReEval_MiscSettingsDlg)

	public:
		CReEval_MiscSettingsDlg();
		virtual ~CReEval_MiscSettingsDlg();

	// Dialog Data
		enum { IDD = IDD_REEVAL_MISC };

	protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

		DECLARE_MESSAGE_MAP()
	public:
		// --------------------------- PUBLIC DATA -------------------------- 

		/** A handle to the reevaluator */
		CReEvaluator *m_reeval;


		// --------------------------- PUBLIC METHODS --------------------------

		/** Saving the data in the dialog */
		afx_msg void SaveData();

	protected:
		// --------------------------- DIALOG COMPONENTS -------------------------- 
		CButton m_loadBtn, m_saveBtn;

		// --------------------------- PROTECTED DATA -------------------------- 

		// --------------------------- PROTECTED METHODS --------------------------

		/** Lets the user browse for a spectrum to use as sky. */
		afx_msg void OnBrowseSkySpectrum();

		/** Lets the user see the possible options for how to remove the dark */
		afx_msg void OnShowDarkSettings();

		/** Called when someone has changed the string in the sky-spectrum edit box */
		afx_msg void OnChangeSkySpectrum();

		/** Called when the user wants to save the current settings into a file */
		afx_msg void OnSaveMiscSettings();

		/** Called when the user wants to load the settings from file */
		afx_msg void OnLoadMiscSettings();

	public:
		virtual BOOL OnInitDialog();
	};
}