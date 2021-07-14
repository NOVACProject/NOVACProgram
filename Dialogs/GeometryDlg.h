#pragma once
#include "afxwin.h"

#include "../Graphs/GraphCtrl.h"
#include "../Common/EvaluationLogFileHandler.h"

// CGeometryDlg dialog
namespace Dialogs{
	class CGeometryDlg : public CDialog
	{
		DECLARE_DYNAMIC(CGeometryDlg)

	public:
		CGeometryDlg(CWnd* pParent = NULL);   // standard constructor
		virtual ~CGeometryDlg();

	// Dialog Data
		enum { IDD = IDD_GEOMETRY_DIALOG };

	protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

		DECLARE_MESSAGE_MAP()
	public:

		// The maximum number of scanners that we can look at at one time
		static const int MAX_N_SCANNERS = 4;

	  // ------------------ DIALOG COMPONENTS -------------------------
	
		/** The volcano selection combo-box */
		CComboBox m_comboVolcano;

		/** The spin control, which lets the user select which scan to show. */
		CSpinButtonCtrl m_scanSpinCtrl;

		/** The frame around the map */
		CStatic m_frameMap;

		/** The actual map*/
		Graph::CGraphCtrl	m_map;

		/** The list of scanners */
		CListBox		m_scannerList;

	  // ------------------- DATA -------------------------

		/** The evaluation log file handlerS */
		FileHandler::CEvaluationLogFileHandler *m_evalLogReader[MAX_N_SCANNERS];

		/** The currently selected scanner */
		int						m_curScanner;

		/** The serial-numbers of the scanning instruments */
		CString				m_serialNumber[MAX_N_SCANNERS];

		/** The assumed compass directions */
		double				m_compass[MAX_N_SCANNERS];

		/** The assumed plume height */
		double				m_plumeHeight;

		/** The currently selected scan */
		int           m_curScan[MAX_N_SCANNERS];
	
		/** The position of the scanning instruments */
		novac::CGPSData			m_gps[MAX_N_SCANNERS];

		/** The index of the nearest volcano into the global g_volcanoes-list */
		int						m_volcanoIndex[MAX_N_SCANNERS];

		/** 1 if we should show the volcano on the map */
		int						m_showVolcano;

		/** 1 if we should show the columns as different-sized circles instead
					of varying colors */
		int						m_showColumnAsSize;

		/** 1 if we should show the direction to the sun on the map */ 
		int						m_showSunRay;

		/** The calculated plume height, from the last call to 'CalculatePlumeHeight' */
		double				m_calcPlumeHeight;

		/** The calculated wind direction, from the last call to 'CalculateWindDirection' */
		double				m_calcWindDirection;

		// ------------------ METHODS ----------------------- 
		/** Called when the user browses for a new evaluation log */
		afx_msg void OnBrowseEvalLog1();
		afx_msg void OnBrowseEvalLog2();
		afx_msg void OnBrowseEvalLog3();
		afx_msg void OnBrowseEvalLog4();

		/** Does the actual browsing for evaluation logs */
		int BrowseForEvaluationLog(int seriesNumber);

		/** Called when the user has change the plume-height assumption */
		afx_msg void OnChangePlumeHeight();

		/** Called when the user has change the assumed compass-direction */
		afx_msg void OnChangeCompass();

		/** Called when the user presses one of the buttons of the m_scanSpinCtrl,
				leads to the selection of another scan with redrawing of the controls. */
		afx_msg void OnChangeSelectedScan(NMHDR *pNMHDR, LRESULT *pResult);

		/** Called when the user has changed which is to be the source for the current
					scanner. */
		afx_msg void OnChangeSource();

		/** Called to initialize the controls in the dialog*/
		virtual BOOL OnInitDialog();

		/** Draws the map */
		void DrawMap();

		/** Shows the sun's position on the map and in text */
		void ShowSunPosition(int scanner);

		/** Initializes the controls of the dialog according to the information in
			the most recently read evaluation log. */
		void InitializeControls();

		/** Intitializing the plot-legend */
		void	InitLegend();

		/** Called when the user changes the selected scanner in the list of scanners */
		afx_msg void OnChangeScanner();

		/** The user wants to save the current graph as an image */
		afx_msg void OnMenu_SaveGraph();

		/** The user wants to calculate the plume heights and wind-directions
				for all possible combinations of scans */
		afx_msg void OnMenu_CalculateGeometries();

	protected:
	  // --------------- PROTECTED METHODS ----------------------- 

		/** This function extracts the gps - data from the evaluation log.
					If no data can be extracted, lat and lon will be set to 0.*/
		void	GetGPSData(int seriesNumber, double &lat, double &lon, long &alt);

		/** This function calculates the solar azimuth (saz) and solar zenith (sza)
				angles for the time of the given scan. */
		void	GetSunPosition(int seriesNumber, int scanNumber, double &sza, double &saz);

		/** Calculates the latitude and longitude for
				the measurements, at plume height. */
		int	CalculateIntersectionPoints(int seriesNumber, int scanNumber, double *lat, double *lon, double *column, int dataLength);

		/** Calculates and shows the wind direction for scan number 'scanNumber'. */
		void	CalculateWindDirection(int seriesNumber, int scanNumber);

		/** Calculates and shows the plume height by combining the two scans
				'scanNumber1' and 'scanNumber2' from the two measurement series
				'seriesNumber1' and 'seriesNumber2'. 
				@return 0 on success */
		int	CalculatePlumeHeight(int seriesNumber1, int scanNumber1, int seriesNumber2, int scanNumber2);

		/** Calculates and sets the range of the plot */
		int	SetPlotRange();

		/** If there are two scanners opened, then this selects which scans should
				be shown at the same time. */
		void SelectScan();

		/** Opens a post-geometry log-file, returns the handle to the opened file.
				On successful return, the name and path of the opened file is given by the parameter 'fileName' */
		FILE *OpenPostGeometryLogFile(int seriesNumber1, int seriesNumber2, CString &fileName);

		// --------------- PROTECTED DATA ----------------------- 

		typedef struct{
			double minLat;
			double maxLat;
			double minLon;
			double maxLon;
			double margin;
		}PlotRange;

		PlotRange	m_plotRange;
	};
}