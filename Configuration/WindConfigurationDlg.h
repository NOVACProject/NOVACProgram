#pragma once

#include "SystemConfigurationPage.h"

// CWindConfigurationDlg dialog
namespace ConfigurationDialog
{

	class CWindConfigurationDlg : public CSystemConfigurationPage
	{
		DECLARE_DYNAMIC(CWindConfigurationDlg)

	public:
		CWindConfigurationDlg();
		virtual ~CWindConfigurationDlg();

	// Dialog Data
		enum { IDD = IDD_CONFIGURE_WIND };

	protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

		DECLARE_MESSAGE_MAP()
	public:
		/** Setup the tool tips */
		void InitToolTips();

		/** Initializing the dialog */
		virtual BOOL OnInitDialog();

		/** Handling the tool tips */
		virtual BOOL PreTranslateMessage(MSG* pMsg);

		/** Called when the user has changed the currently selected scanner */
		void OnChangeScanner();

		/** Called when the automatic measurements have been turned on/off */
		afx_msg void OnOnOff();

		/** Saves the data in the dialog */
		afx_msg void SaveData();

		/** Called when the user has changed between using angles or distances */
		afx_msg void OnChangeAngleDistance();

	protected:

		/** the local copies of the settings */
		int automatic; // 0 or 1
		int duration; //minutes
		int	interval; //minutes
		double maxAngle; // degrees
		int stablePeriod; // scans
		double minPeakColumn; // ppmm
		int motorStepsComp; // steps
		double desiredAngle, desiredDistance, desiredSwitchRange;
		int useCalcWind;
		int useAngle;


		/** The edit-boxes on the screen */
		CEdit m_editDuration, m_editInterval, m_editMaxAngle, m_editStablePeriod, m_editMinPeakColumn, m_editMotorStepsComp;

		// The special controls for the Heidelberg-instrument
		CEdit m_editAngle, m_editDistance, m_editSwitchRange;
		CButton m_radioAngle, m_radioDistance;
		CButton m_useCalcWind;
		CStatic m_labelDeg, m_labelMeters, m_frameDistance, m_labelSwitchRange;


	};
}