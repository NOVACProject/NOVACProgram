#pragma once

#include "../Common/EvaluationLogFileHandler.h"
#include "../Graphs/GraphCtrl.h"
#include "../DlgControls/Label.h"
#include "WindSpeedCalculator.h"
#include "WindSpeedMeasSettings.h"
#include "afxwin.h"

// CPostWindDlg dialog

#define MAX_N_SERIES 3
namespace Dialogs{
	class CPostWindDlg : public CDialog
	{
		DECLARE_DYNAMIC(CPostWindDlg)

	public:
		CPostWindDlg(CWnd* pParent = NULL);   // standard constructor
		virtual ~CPostWindDlg();

	// Dialog Data
		enum { IDD = IDD_POST_WINDSPEED_DLG };

	protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

		DECLARE_MESSAGE_MAP()
	public:
		/** Called when the user changes the name of evallog 1 in the edit-box */
		afx_msg void OnChangeEvallog1();

		/** Called when the user changes the name of evallog 2 in the edit-box */
		afx_msg void OnChangeEvallog2();

		/** Called when the user wants to browse for evallog 1 */
		afx_msg void OnBrowseSeries1();

		/** Called when the user wants to browse for evallog 2 */
		afx_msg void OnBrowseSeries2();

		/** The user has changed the number of iterations in the low pass filtering */
		afx_msg void OnChangeLPIterations();

		/** Opens a 'Open' dialog and lets the user browse for an evaluation
				log to open. If the file is readable, the CEvaluationLogFileHandler number
				'seriesNumber' will be used to open and read the log-file.
				Returns the number of evaluation logs opened and read (0 or 1). */
		int BrowseForEvalLog(int seriesNumber);

		/** Initializing the dialog and its controls */
		virtual BOOL OnInitDialog();

		/** Draws the column plot */
		afx_msg void	DrawColumn();

		/** Draws the result plot */
		afx_msg void	DrawResult();

		/** Performes a low pass filtering procedure on series number 'seriesNo'.
				The number of iterations is taken from 'm_lowPassIterations'
				The treated series is m_OriginalSeries[seriesNo]
				The result is saved as m_PreparedSeries[seriesNo]
				@return - the number of successfully filtered series (0 or 1) */
		int		LowPassFilter(int seriesNo);

		/** Called when the user presses the 'Calculate wind speed' - button. 
				Here lies the actual work of the dialog. */
		afx_msg void OnCalculateWindspeed();

		/** Called when the user changes the plume height used in the calculations */
		afx_msg void OnChangePlumeHeight();

	protected:
		/** The log-file handler for the measured series */
		FileHandler::CEvaluationLogFileHandler *m_logFileHandler[MAX_N_SERIES];

		/** The frame that surrounds the column - graph */
		CStatic m_columnFrame;

		/** The frame that surrounds the results - graph */
		CStatic m_resultFrame;

		/** The column graph */
		Graph::CGraphCtrl m_columnGraph;

		/** The results graph */
		Graph::CGraphCtrl m_resultGraph;

		/** The browse buttons */
		CButton m_browseButton1, m_browseButton2;

		/** The edit-boxes */
		CEdit m_editEvalLog1, m_editEvalLog2;

		/** The legends, shows which series is shown with which color */
		DlgControls::CLabel m_legendSeries1;
		DlgControls::CLabel m_legendSeries2;

		/** Original measurement series, as they are in the file */
		WindSpeedMeasurement::CWindSpeedCalculator::CMeasurementSeries		*m_OriginalSeries[MAX_N_SERIES];

		/** Treated measurement series, low pass filtered etc. */
		WindSpeedMeasurement::CWindSpeedCalculator::CMeasurementSeries		*m_PreparedSeries[MAX_N_SERIES];

		/** The settings for how the windspeed calculations should be done */
		WindSpeedMeasurement::CWindSpeedMeasSettings	m_settings;

		/** The wind speed measurement-calculator. */
		WindSpeedMeasurement::CWindSpeedCalculator		m_calc;

		/** Initializes the legends */
		void	InitLegends();

	private:
		/** The arrays for the data shown in the result - graph */
		double		*corr, *delay, *ws;

		/** The settings for the instrument used */
		double		m_coneAngle;	// <-- the cone-angle of the instrument that was used
		double		m_pitch;			// <-- the pitch (tilt) of the instrument
		double		m_scanAngle;	// <-- the scan-angle at which the series was collected

		/** The colors for the time-series */
		COLORREF	m_colorSeries[MAX_N_SERIES];

	public:
		int m_showOption;
	};
}