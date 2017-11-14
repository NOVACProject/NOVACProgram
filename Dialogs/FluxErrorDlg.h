#pragma once


namespace Dialogs{
	// CFluxErrorDlg dialog

	class CFluxErrorDlg : public CDialog
	{
		DECLARE_DYNAMIC(CFluxErrorDlg)

	public:
		CFluxErrorDlg(CWnd* pParent = NULL);   // standard constructor
		virtual ~CFluxErrorDlg();

	// Dialog Data
		enum { IDD = IDD_FLUXERROR_DIALOG };

	protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

		DECLARE_MESSAGE_MAP()

	public:

		// ------------------------------------------------------
		// ------------------ PUBLIC DATA ----------------------- 
		// ------------------------------------------------------

		/** The geometrical error */
		double	m_geomError;

		/** The spectroscopical error */
		double	m_specError;

		/** The scattering error */
		double	m_scatteringError;

		/** The wind error */
		double	m_windError;

		// ------------------------------------------------------
		// ------------------ PUBLIC METHODS --------------------
		// ------------------------------------------------------

		/** Initializing the dialog and everything in it */
		virtual BOOL OnInitDialog();

		/** Saving the data in the dialog */
		afx_msg void SaveData();

		/** When the window is about to loose the input focus */
		afx_msg void OnKillFocus(CWnd* pNewWnd);

	private:
		// ------------------------------------------------------
		// ------------------ PRIVATE DATA ----------------------
		// ------------------------------------------------------


		// ------------------------------------------------------
		// ------------------ PRIVATE METHODS -------------------
		// ------------------------------------------------------

	};
}