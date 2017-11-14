#pragma once

#include "Configuration.h"

namespace ConfigurationDialog{


	// CAdvancedFTPUploadSettings dialog

	class CAdvancedFTPUploadSettings : public CDialog
	{
		DECLARE_DYNAMIC(CAdvancedFTPUploadSettings)

	public:
		CAdvancedFTPUploadSettings(CWnd* pParent = NULL);   // standard constructor
		virtual ~CAdvancedFTPUploadSettings();

	// Dialog Data
		enum { IDD = IDD_CONFIGURATION_ADVANCED_FTP_UPLOAD };

	protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

		DECLARE_MESSAGE_MAP()
	public:

		// ---------------------- CONSTANTS ---------------------------

		// ------------------ DIALOG COMPONENTS -------------------------

		// ------------------- DATA -------------------------

		/** the actual configuration */ 
		CConfigurationSetting *m_configuration;

		/** The time when to start uploading */
		int	m_startHr, m_startMin, m_startSec;

		/** The time when to stop uploading */
		int m_stopHr,  m_stopMin,	 m_stopSec;
			
		// ------------------ METHODS ----------------------- 

		/** Initialzing the dialog */
		virtual BOOL OnInitDialog();

		/** Called when the user presses the 'OK'-button.
				Saves the filled in data in the dialog */
		virtual void OnOK();

	protected:

		// ---------------------- CONSTANTS ---------------------------

		// ------------------ DIALOG COMPONENTS -------------------------

		// ------------------- DATA -------------------------
			
		// ------------------ METHODS ----------------------- 

		/** Returns the width of the supplied component to be able to fit in
				the supplied string */
		void	GetNecessaryComponentSize(CWnd *wnd, CString &str, int &width, int &height);

	};
}