#pragma once

#include "../Configuration/Configuration.h"

namespace Dialogs{

	// CDarkSettingsDialog dialog

	class CDarkSettingsDialog : public CDialog
	{
		DECLARE_DYNAMIC(CDarkSettingsDialog)

	public:
		CDarkSettingsDialog(CWnd* pParent = NULL);   // standard constructor
		virtual ~CDarkSettingsDialog();

	// Dialog Data
		enum { IDD = IDD_DARK_SETTINGS_DLG };

	protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

		DECLARE_MESSAGE_MAP()
		
		/** The use wants to close the dialog but not save the changes */
		virtual void OnCancel();

		/** The use wants to close the dialog and save the changes */
		virtual void OnOK();

	public:
		/** Initializing the dialog */
		virtual BOOL OnInitDialog();

		/** Setup the tool tips */
		void InitToolTips();

		/** Handling the tool tips */
		virtual BOOL PreTranslateMessage(MSG* pMsg); 

		/** Saving the data in the dialog */
		afx_msg void SaveData();

		/** Updating the data in the dialog */
		afx_msg void UpdateDlg();

		/** Updating the status of the controls */
		afx_msg void UpdateControls();

		/** Caled when the user has changed the way the dark-spectrum
				should be collected */
		void OnChangeDarkSpecOption();

		// The 'Browse' buttons
		afx_msg void OnBrowseOffset1();
		afx_msg void OnBrowseOffset2();
		afx_msg void OnBrowseDC1();
		afx_msg void OnBrowseDC2();

		/** Browsing for a file, the result will be saved in the edit box 'editbox' */
		int	BrowseFile(int editBox);

		// ----------------------------------------------------------
		// ------------------- PUBLIC DATA --------------------------
		// ----------------------------------------------------------

		/** the actual configuration. Will be modified when the user presses the 'ok' button */ 
        Configuration::CDarkSettings *m_darkSettings;

		private:
			// ----------------------------------------------------------
			// ------------------ PRIVATE DATA --------------------------
			// ----------------------------------------------------------

			/** True when all the things in the dialog have been initialized */
			bool	m_initialized;

			/** The option for how to get the dark spectrum */
			int		m_darkSpecOption;

			/** The option for how to use the offset spectrum */
			int		m_offsetSpecOption;

			/** The option for how to use the dark-current spectrum */
			int		m_dcSpecOption;

			/** The paths for the offset spectra */
			CString	m_offsetPath1, m_offsetPath2;

			/** The paths for the dark-current spectra */
			CString	m_dcPath1, m_dcPath2;

			/** The path for the dark-spectrum, if given by the user... */
			CString	m_darkSpectrum_UserSupplied;

			// ---------------------------------------------------------------
			// ------------------ DIALOG COMPONENTS --------------------------
			// ---------------------------------------------------------------

			/** The labels */
			CStatic m_labelMaster1, m_labelMaster2;
			CStatic m_labelSlave1,	m_labelSlave2;
			CStatic	m_labelDarkOffsetCorr;

			/** The edit-boxes */
			CStatic	m_editOffset1,	m_editOffset2;
			CStatic m_editDC1,			m_editDC2;

			/** The combo-boxes */
			CComboBox	m_comboOffset, m_comboDC;

			/** The buttons */
			CButton		m_btnMasterOff, m_btnSlaveOff;
			CButton		m_btnMasterDC,	m_btnSlaveDC;

			/** The tooltip control */
			CToolTipCtrl m_toolTip;

	};
}