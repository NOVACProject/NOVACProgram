#include "stdafx.h"
#include "../NovacMasterProgram.h"
#include "PostWindDlg.h"
#include "../UserSettings.h"
#include "../Common/Common.h"

// CPostWindDlg dialog

extern CUserSettings g_userSettings;       // <-- The users preferences

using namespace Dialogs;

IMPLEMENT_DYNAMIC(CPostWindDlg, CDialog)
CPostWindDlg::CPostWindDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPostWindDlg::IDD, pParent)
{
	for(int k = 0; k < MAX_N_SERIES; ++k){
		m_OriginalSeries[k] = NULL;
		m_PreparedSeries[k] = NULL;
		m_logFileHandler[k]	= NULL;
	}
	m_showOption	= 0;

	corr = delay = ws = NULL;
}

CPostWindDlg::~CPostWindDlg()
{
	for(int k = 0; k < MAX_N_SERIES; ++k){
		delete m_OriginalSeries[k];
		delete m_PreparedSeries[k];
		delete m_logFileHandler[k];
	}

	delete[] corr;
	delete[] delay;
	delete[] ws;
}

void CPostWindDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COLUMN_FRAME,		m_columnFrame);
	DDX_Control(pDX, IDC_RESULT_FRAME,		m_resultFrame);
	DDX_Text(pDX, IDC_EDIT_LP_ITERATIONS, m_settings.lowPassFilterAverage);
	DDX_Text(pDX, IDC_EDIT_SHIFT_MAX,			m_settings.shiftMax);
	DDX_Text(pDX, IDC_EDIT_TESTLENGTH,		m_settings.testLength);
	DDX_Text(pDX,	IDC_EDIT_PLUMEHEIGHT,		m_settings.plumeHeight);
	DDX_Radio(pDX, IDC_RADIO_SHOW_CORR,		m_showOption);

	// The legends
	DDX_Control(pDX, IDC_LEGEND_SERIES1,	m_legendSeries1);
	DDX_Control(pDX, IDC_LEGEND_SERIES2,	m_legendSeries2);

	// The buttons.
	DDX_Control(pDX, IDC_BROWSE_SERIES1,	m_browseButton1);
	DDX_Control(pDX, IDC_BROWSE_SERIES2,	m_browseButton2);

	// The edit-boxes.
	DDX_Control(pDX, IDC_EDIT_EVALLOG1,		m_editEvalLog1);
	DDX_Control(pDX, IDC_EDIT_EVALLOG2,		m_editEvalLog2);
}


BEGIN_MESSAGE_MAP(CPostWindDlg, CDialog)
	ON_EN_CHANGE(IDC_EDIT_EVALLOG1,					OnChangeEvallog1)
	ON_EN_CHANGE(IDC_EDIT_EVALLOG2,					OnChangeEvallog2)
	ON_BN_CLICKED(IDC_BROWSE_SERIES1,				OnBrowseSeries1)
	ON_BN_CLICKED(IDC_BROWSE_SERIES2,				OnBrowseSeries2)
	ON_EN_CHANGE(IDC_EDIT_LP_ITERATIONS,			OnChangeLPIterations)
	ON_BN_CLICKED(IDC_BTN_CALCULATE_WINDSPEED,		OnCalculateWindspeed)
	ON_EN_CHANGE(IDC_EDIT_PLUMEHEIGHT,				OnChangePlumeHeight)
	ON_BN_CLICKED(IDC_RADIO_SHOW_CORR,				DrawResult)
	ON_BN_CLICKED(IDC_RADIO2,						DrawResult)
	ON_BN_CLICKED(IDC_RADIO5,						DrawResult)
END_MESSAGE_MAP()


// CPostWindDlg message handlers

void CPostWindDlg::OnChangeEvallog1()
{
}

void CPostWindDlg::OnChangeEvallog2()
{
}

void CPostWindDlg::OnBrowseSeries1()
{
	if(0 == BrowseForEvalLog(0))
		return;

	SetDlgItemText(IDC_EDIT_EVALLOG1, m_logFileHandler[0]->m_evaluationLog);

	DrawColumn();
}

void CPostWindDlg::OnBrowseSeries2()
{
	if(0 == BrowseForEvalLog(1))
		return;

	SetDlgItemText(IDC_EDIT_EVALLOG2, m_logFileHandler[1]->m_evaluationLog);

	DrawColumn();
}

int CPostWindDlg::BrowseForEvalLog(int seriesNumber){
	CString evLog;
	evLog.Format("");
	TCHAR filter[512];
	int n = _stprintf(filter, "Evaluation Logs\0");
	n += _stprintf(filter + n + 1, "*.txt;\0");
	filter[n + 2] = 0;
	Common common;

	if(seriesNumber > MAX_N_SERIES - 1)
		return 0;

	// let the user browse for an evaluation log file and if one is selected, read it
	if(common.BrowseForFile(filter, evLog)){

		// completely reset any old data
		if(m_logFileHandler[seriesNumber] != NULL)
			delete m_logFileHandler[seriesNumber];
		m_logFileHandler[seriesNumber] = new FileHandler::CEvaluationLogFileHandler();

		m_logFileHandler[seriesNumber]->m_evaluationLog.Format(evLog);
		if(SUCCESS != m_logFileHandler[seriesNumber]->ReadEvaluationLog()){
			m_logFileHandler[seriesNumber]->m_evaluationLog.Format("");
			return 0;
		}
		
		// find the wind-speed measurement series in the log file
		int k;
		for(k = 0; k < m_logFileHandler[seriesNumber]->m_scanNum; ++k){
			if(m_logFileHandler[seriesNumber]->IsWindSpeedMeasurement(k))
				break;
		}
		if(k == m_logFileHandler[seriesNumber]->m_scanNum){
			// no wind speed measurment found
			MessageBox("That evaluation log does not contain a wind speed measurement");
			return 0;
		}

		Evaluation::CScanResult &scan = m_logFileHandler[seriesNumber]->m_scan[k];
		long length = scan.GetEvaluatedNum();
		if(length <= 0) 
			return 0; // <-- something's wrong here!!

		if(scan.IsWindMeasurement_Gothenburg()){
			m_OriginalSeries[seriesNumber] = new WindSpeedMeasurement::CWindSpeedCalculator::CMeasurementSeries(length);
			if(m_OriginalSeries[seriesNumber] == NULL)
				return 0; // <-- failed to allocate enough memory

			const CSpectrumTime *startTime = scan.GetStartTime(0);
			for(int k = 0; k < length; ++k){
				const CSpectrumTime *time = scan.GetStartTime(k);
				m_OriginalSeries[seriesNumber]->column[k] = scan.GetColumn(k, 0);

				// Save the time difference
				m_OriginalSeries[seriesNumber]->time[k]		= 
					3600.0 * (time->hr - startTime->hr) + 
					60.0	 * (time->m - startTime->m) + 
					(time->sec - startTime->sec);
			}

			// remember the settings for the instrument
			m_coneAngle = scan.GetConeAngle();
			m_pitch			= scan.GetPitch();
			m_scanAngle	= scan.GetScanAngle(scan.GetEvaluatedNum() / 2);
		}else if(scan.IsWindMeasurement_Heidelberg()){
			m_OriginalSeries[0] = new WindSpeedMeasurement::CWindSpeedCalculator::CMeasurementSeries(length / 2);
			m_OriginalSeries[1] = new WindSpeedMeasurement::CWindSpeedCalculator::CMeasurementSeries(length / 2);
			if(m_OriginalSeries[0] == NULL || m_OriginalSeries[1] == NULL)
				return 0; // <-- failed to allocate enough memory

			const CSpectrumTime *startTime = scan.GetStartTime(0);
			for(int k = 0; k < length; ++k){
				const CSpectrumTime *time = scan.GetStartTime(k);
				m_OriginalSeries[k % 2]->column[k / 2] = scan.GetColumn(k, 0);

				// Save the time difference
				m_OriginalSeries[k % 2]->time[k / 2]		= 
					3600 * (time->hr - startTime->hr) + 
					60	 * (time->m - startTime->m) + 
					(time->sec - startTime->sec);
			}

			// remember the settings for the instrument
			m_coneAngle = scan.GetConeAngle();
			m_pitch			= scan.GetPitch();
			m_scanAngle	= scan.GetScanAngle(scan.GetEvaluatedNum() / 2);

			// calculate the angle between the two...
			int midpoint = (int)(length / 2);
			double d1 = scan.GetScanAngle(midpoint) - scan.GetScanAngle(midpoint + 1);
			double d2 = scan.GetScanAngle2(midpoint) - scan.GetScanAngle2(midpoint + 1);
			m_settings.angleSeparation = sqrt(d1 * d1 + d2 * d2);

			// For heidelberg measurements, we only need one eval-log file
			if(seriesNumber == 0){
				m_browseButton1.EnableWindow(TRUE);
				m_editEvalLog1.EnableWindow(TRUE);
				m_browseButton2.EnableWindow(FALSE);
				m_editEvalLog2.EnableWindow(FALSE);
			}else if(seriesNumber == 1){
				m_browseButton1.EnableWindow(FALSE);
				m_editEvalLog1.EnableWindow(FALSE);
				m_browseButton2.EnableWindow(TRUE);
				m_editEvalLog2.EnableWindow(TRUE);
			}
		}
		return 1;
	}
	return 0;
}

BOOL CPostWindDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	Common common;
	CString columnAxisLabel;

	// Initialize the unit to use
	if(g_userSettings.m_columnUnit == UNIT_MOLEC_CM2)
		columnAxisLabel.Format("%s [ppmm]", (LPCTSTR)common.GetString(AXIS_COLUMN));
	else
		columnAxisLabel.Format("%s [molec/cm²]", (LPCTSTR)common.GetString(AXIS_COLUMN));

	// Initialize the column graph
	CRect rect;
	m_columnFrame.GetWindowRect(rect);
	int height = rect.bottom - rect.top;
	int width  = rect.right - rect.left;
	rect.top = 20; rect.bottom = height - 10;
	rect.left = 10; rect.right = width - 10;
	m_columnGraph.Create(WS_VISIBLE | WS_CHILD, rect, &m_columnFrame);
	m_columnGraph.SetXUnits("Time [s]");
	m_columnGraph.SetYUnits(columnAxisLabel);
	m_columnGraph.SetBackgroundColor(RGB(0, 0, 0));
	m_columnGraph.SetPlotColor(RGB(255, 0, 0));
	m_columnGraph.SetGridColor(RGB(255, 255, 255));
	m_columnGraph.SetRange(0, 600, 0, 0, 100, 1);

	// Initialize the results graph
	m_resultFrame.GetWindowRect(rect);
	height = rect.bottom - rect.top;
	width  = rect.right - rect.left;
	rect.top = 20; rect.bottom = height - 10;
	rect.left = 10; rect.right = width - 10;
	m_resultGraph.Create(WS_VISIBLE | WS_CHILD, rect, &m_resultFrame);
	m_resultGraph.SetXUnits("Time [s]");
	m_resultGraph.SetYUnits("Wind Speed [m/s]");
	m_resultGraph.SetBackgroundColor(RGB(0, 0, 0));
	m_resultGraph.SetPlotColor(RGB(255, 0, 0));
	m_resultGraph.SetGridColor(RGB(255, 255, 255));
	m_resultGraph.SetRange(0, 600, 0, 0, 100, 1);

	// Setup the colors to use for the different time-series
	m_colorSeries[0] = RGB(255, 0, 0);
	m_colorSeries[1] = RGB(0, 0, 255);
	m_colorSeries[2] = RGB(0, 255, 0);

	// Setup the legends
	InitLegends();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

/** Draws the result */
void	CPostWindDlg::DrawResult(){
	static const int BUFFER_SIZE = 1024;
	if(corr == NULL){
		corr	= new double[BUFFER_SIZE];
		delay	= new double[BUFFER_SIZE];
		ws		= new double[BUFFER_SIZE];
	}

	if(m_calc.m_length == 0 || m_calc.corr == 0)
		return;

	UpdateData(TRUE); // <-- get the options for how to plot the data

	/** Calculate the distance */
	double distance;
	if(fabs(m_coneAngle - 90.0) < 1.0){
		// Flat scanner
		distance		= m_settings.plumeHeight * tan(DEGREETORAD * m_settings.angleSeparation);
	}else{
		// Cone scanner
		double angle	= DEGREETORAD * (90.0 - (m_coneAngle - fabs(m_pitch)));
		distance			= m_settings.plumeHeight * fabs(tan(angle) - tan(angle - DEGREETORAD * m_settings.angleSeparation));
	}
	// If the scanners are not looking straight up then the distance between
	//		the two directions gets decreased with the scanAngle
	distance			*= cos(m_scanAngle * DEGREETORAD);

	/** Copy the values to the local buffers */
	int length = 0;
	for(int k = 0; k < m_calc.m_length; ++k){
		if(length > BUFFER_SIZE)
			break;
		if(m_calc.used[k]){
			corr[length]	= m_calc.corr[k];
			delay[length] = m_calc.delays[k];
			if(fabs(delay[length]) > 0.01)
				ws[length]		= distance / delay[length];
			else
				ws[length]		= 0.0;
			++length;
		}
	}

	if(m_showOption == 0){
		/** Draw the correlation */
		m_resultGraph.SetYUnits("Correlation [-]");
		m_resultGraph.XYPlot(NULL, corr, length);
	}else if(m_showOption == 1){
		/** Draw the delay */
		m_resultGraph.SetYUnits("Temporal delay [s]");
		m_resultGraph.XYPlot(NULL, delay, length);
	}else if(m_showOption == 2){
		/** Draw the resulting wind speed */
		m_resultGraph.SetYUnits("Wind speed [m/s]");
		m_resultGraph.XYPlot(NULL, ws, length);
	}
}

/** Draws the column plot */
void	CPostWindDlg::DrawColumn(){

	double minT = 1e16, maxT = -1e16, minC = 1e16, maxC = -1e16;
	int nSeries = 0;
		
	// get the range for the plot
	for(int k = 0; k < MAX_N_SERIES; ++k){
		if(m_OriginalSeries[k] != NULL){
			minT = min(minT, m_OriginalSeries[k]->time[0]);
			maxT = max(maxT, m_OriginalSeries[k]->time[m_OriginalSeries[k]->length-1]);

			minC = min(minC, Min(m_OriginalSeries[k]->column, m_OriginalSeries[k]->length));
			maxC = max(maxC, Max(m_OriginalSeries[k]->column, m_OriginalSeries[k]->length));

			++nSeries;
		}
	}

	if(nSeries == 0)
		return; // <-- nothing to plot

	// Set the range for the plot
	m_columnGraph.SetRange(minT, maxT, 0,		minC, maxC, 1);

	// Draw the time series
	for(int k = 0; k < MAX_N_SERIES; ++k){
		if(m_OriginalSeries[k] != NULL){

			// ---------- Draw the original time series -----------
			// set the color
			m_columnGraph.SetPlotColor(m_colorSeries[k % 3]);

			// draw the series
			m_columnGraph.XYPlot(
				m_OriginalSeries[k]->time, 
				m_OriginalSeries[k]->column, 
				m_OriginalSeries[k]->length,
				Graph::CGraphCtrl::PLOT_FIXED_AXIS | Graph::CGraphCtrl::PLOT_CONNECTED);

			// ---------- Draw the filtered time series -----------
			if(m_settings.lowPassFilterAverage > 0){
				m_columnGraph.SetLineWidth(2);
				m_columnGraph.SetPlotColor(RGB(255,255,255));
				// perform the low pass filtering
				LowPassFilter(k);

				// draw the series
				m_columnGraph.XYPlot(
					m_PreparedSeries[k]->time, 
					m_PreparedSeries[k]->column, 
					m_PreparedSeries[k]->length,
					Graph::CGraphCtrl::PLOT_FIXED_AXIS | Graph::CGraphCtrl::PLOT_CONNECTED);

//				m_columnGraph.SetLineWidth(1);
			}

		}

	}
}

/** Performes a low pass filtering procedure on series number 'seriesNo'.
		The number of iterations is taken from 'm_settings.lowPassFilterAverage'
		The treated series is m_OriginalSeries[seriesNo]
		The result is saved as m_PreparedSeries[seriesNo]	*/
int	CPostWindDlg::LowPassFilter(int seriesNo){
	if(m_settings.lowPassFilterAverage <= 0)
		return 0;

	if(seriesNo < 0 || seriesNo > MAX_N_SERIES)
		return 0;

	if(m_OriginalSeries[seriesNo] == NULL)
		return 0;

	int length = m_OriginalSeries[seriesNo]->length;
	if(length <= 0)
		return 0;

	if(m_PreparedSeries[seriesNo] == NULL)
		m_PreparedSeries[seriesNo] = new WindSpeedMeasurement::CWindSpeedCalculator::CMeasurementSeries();

	if(SUCCESS != WindSpeedMeasurement::CWindSpeedCalculator::LowPassFilter(m_OriginalSeries[seriesNo], m_PreparedSeries[seriesNo], m_settings.lowPassFilterAverage))
		return 0;

	return 1;
}

void CPostWindDlg::OnChangeLPIterations()
{
	UpdateData(TRUE); // <-- save the data in the dialog
	if(m_settings.lowPassFilterAverage >= 0)
		DrawColumn();
}

void CPostWindDlg::OnCalculateWindspeed()
{
	UpdateData(TRUE); // <-- start by saving the data in the dialog

	double delay; // <-- the calculated delay

	// 1. Perform the correlation - calculations...
	m_calc.CalculateDelay(delay, m_OriginalSeries[0], m_OriginalSeries[1], m_settings);

	// 2. Calculate the average value of the correlation
	double	 avgCorr1 = Average(m_calc.corr, m_calc.m_length);

	// 3. Perform the correlation - calculations with the second series as upwind
	m_calc.CalculateDelay(delay, m_OriginalSeries[1], m_OriginalSeries[0], m_settings);

	// 4. Calculate the average correlation for the second case
	double	avgCorr2 = Average(m_calc.corr, m_calc.m_length);

	// 5. Use the results which gave the highest average correlation
	if(avgCorr1 > avgCorr2){
		m_calc.CalculateDelay(delay, m_OriginalSeries[0], m_OriginalSeries[1], m_settings);
	}

	// 6. Display the results on the screen
	DrawResult();
}

void CPostWindDlg::OnChangePlumeHeight()
{
	// 1. save the data in the dialog
	UpdateData(TRUE);

	// 2. Update the result-graph, if necessary
	if(m_showOption == 2)
		DrawResult();

	
}

void CPostWindDlg::InitLegends(){
	// The legend for series 1
	m_legendSeries1.ShowWindow(SW_SHOW);
	m_legendSeries1.SetBackgroundColor(m_colorSeries[0]);
	m_legendSeries1.SetWindowText("");

	// The legend for series 2
	m_legendSeries2.ShowWindow(SW_SHOW);
	m_legendSeries2.SetBackgroundColor(m_colorSeries[1]);
	m_legendSeries2.SetWindowText("");
}