#include "stdafx.h"
#include "../NovacMasterProgram.h"
#include "GeometryDlg.h"

#include "../Dialogs/QueryStringDialog.h"

#include "../Geometry/GeometryCalculator.h"
#include "../Common/Common.h"
#include "../VolcanoInfo.h"

// the version of the program
#include "../Common/Version.h"

extern CVolcanoInfo g_volcanoes;

// CGeometryDlg dialog
using namespace Dialogs;
using namespace Geometry;

IMPLEMENT_DYNAMIC(CGeometryDlg, CDialog)
CGeometryDlg::CGeometryDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGeometryDlg::IDD, pParent)
{
	m_plumeHeight = 1000.0;
	for(int k = 0; k < MAX_N_SCANNERS; ++k){
		m_evalLogReader[k] = NULL;
		m_curScan[k] = 0;
		m_volcanoIndex[k] = 0;
		m_compass[k]	= 0;
	}
	m_showVolcano = 1;
	m_showColumnAsSize = 0;
	m_curScanner	= -1;
	m_calcPlumeHeight = -999.0;
	m_calcWindDirection = -999.0;
}

CGeometryDlg::~CGeometryDlg()
{
	for(int k = 0; k < MAX_N_SCANNERS; ++k){
		delete(m_evalLogReader[k]);
	}
}

void CGeometryDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_VOLCANO,					m_comboVolcano);
	DDX_Control(pDX, IDC_MAP_FRAME,							m_frameMap);
	DDX_Control(pDX, IDC_SCAN_SPIN,							m_scanSpinCtrl);

	DDX_Text(pDX, IDC_EDIT_PLUMEHEIGHT,					m_plumeHeight);
	DDX_Control(pDX, IDC_SCANNER_LIST,					m_scannerList);

	DDX_Check(pDX,	IDC_CHECK_SHOW_VOLCANO,			m_showVolcano);
	DDX_Check(pDX,	IDC_CHECK_SHOW_SUNDIRECTION,m_showSunRay); 
	DDX_Check(pDX,	IDC_CHECK_VARYING_SIZE,			m_showColumnAsSize);

	if(m_curScanner >= 0){
		DDX_Text(pDX, IDC_EDIT_COMPASS,						m_compass[m_curScanner]);
	}
}


BEGIN_MESSAGE_MAP(CGeometryDlg, CDialog)
	// Opening a new evaluation-log
	ON_BN_CLICKED(IDC_BTN_BROWSEEVALLOG,		OnBrowseEvalLog1)
	ON_BN_CLICKED(IDC_BTN_BROWSEEVALLOG2,		OnBrowseEvalLog2)
	ON_BN_CLICKED(IDC_BTN_BROWSEEVALLOG3,		OnBrowseEvalLog3)
	ON_BN_CLICKED(IDC_BTN_BROWSEEVALLOG4,		OnBrowseEvalLog4)

	// Changing the assumed plume-height
	ON_EN_CHANGE(IDC_EDIT_PLUMEHEIGHT,				OnChangePlumeHeight)

	//// Changing the assumed compass direction
	ON_EN_CHANGE(IDC_EDIT_COMPASS,						OnChangeCompass)

	// Showing the next, or the previous scan.
  ON_NOTIFY(UDN_DELTAPOS, IDC_SCAN_SPIN,		OnChangeSelectedScan)
  //ON_COMMAND(ID_VIEW_PREVIOUSSCAN,				OnShowPreviousScan)
  //ON_COMMAND(ID_VIEW_NEXTSCAN,						OnShowNextScan)

	// Changing the selected scanner
	ON_LBN_SELCHANGE(IDC_SCANNER_LIST,				OnChangeScanner)

	// Changing the source
	ON_CBN_SELCHANGE(IDC_COMBO_VOLCANO,				OnChangeSource)

	// Changing the options for how to show the plot
	ON_BN_CLICKED(IDC_CHECK_SHOW_VOLCANO,				DrawMap)
	ON_BN_CLICKED(IDC_CHECK_SHOW_SUNDIRECTION,	DrawMap)
	ON_BN_CLICKED(IDC_CHECK_VARYING_SIZE,				DrawMap)

	// The menu commands
	ON_COMMAND(ID_FILE_SAVEGRAPHASIMAGE,												OnMenu_SaveGraph)
	ON_COMMAND(ID_ANALYSIS_CALCULATEPLUMEHEIGHTSWINDDIRECTIONS,	OnMenu_CalculateGeometries)
END_MESSAGE_MAP()


// CGeometryDlg message handlers
void CGeometryDlg::OnBrowseEvalLog1(){
	if(0 == BrowseForEvaluationLog(0))
		return;

	SetDlgItemText(IDC_EDIT_EVALLOG1, m_evalLogReader[0]->m_evaluationLog);
}
void CGeometryDlg::OnBrowseEvalLog2(){
	if(0 == BrowseForEvaluationLog(1))
		return;

	SetDlgItemText(IDC_EDIT_EVALLOG2, m_evalLogReader[1]->m_evaluationLog);
}

void CGeometryDlg::OnBrowseEvalLog3(){
	if(0 == BrowseForEvaluationLog(2))
		return;

	SetDlgItemText(IDC_EDIT_EVALLOG3, m_evalLogReader[2]->m_evaluationLog);
}

void CGeometryDlg::OnBrowseEvalLog4(){
	if(0 == BrowseForEvaluationLog(3))
		return;

	SetDlgItemText(IDC_EDIT_EVALLOG4, m_evalLogReader[3]->m_evaluationLog);
}

/** Does the actual browsing for evaluation logs */
int CGeometryDlg::BrowseForEvaluationLog(int seriesNumber)
{
	CString evLog;
	evLog.Format("");
	TCHAR filter[512];
	int n = _stprintf(filter, "Evaluation Logs\0");
	n += _stprintf(filter + n + 1, "*.txt;\0");
	filter[n + 2] = 0;
	Common common;
	  
	// let the user browse for an evaluation log file and if one is selected, read it
	if(common.BrowseForFile(filter, evLog)){
		// Completely reset the data.
		if(m_evalLogReader[seriesNumber] != NULL)
		delete m_evalLogReader[seriesNumber];
		m_evalLogReader[seriesNumber] = new FileHandler::CEvaluationLogFileHandler();


		// Read the evaluation log
		m_evalLogReader[seriesNumber]->m_evaluationLog.Format(evLog);
		if(SUCCESS != m_evalLogReader[seriesNumber]->ReadEvaluationLog()){
			MessageBox("Failed to read evaluation log");
			return 0;
		}

		m_curScan[seriesNumber] = 0;

		// Get the position of the scanning instrument
		double lat, lon;
		long	alt;
		GetGPSData(seriesNumber, lat, lon, alt);
		if(fabs(lat) < 1e-5 && fabs(lon) < 1e-5){// <-- no gps-data
			MessageBox("Evaluation log does not contain gps-data");
			return 0; 
		}else{
			// remember the data
			m_gps[seriesNumber].m_latitude	= lat;
			m_gps[seriesNumber].m_longitude = lon;
			m_gps[seriesNumber].m_altitude	= alt;
		}

		// Get the nearest volcano
		m_volcanoIndex[seriesNumber] = CGeometryCalculator::GetNearestVolcano(lat, lon);
		if(m_volcanoIndex[seriesNumber] == -1){
			MessageBox("No volcano could be found close to this instrument", "Error");
			return 0; // <-- no volcano could be found
		}

		// Get the serial-number of the spectrometer
		m_serialNumber[seriesNumber].Format(m_evalLogReader[seriesNumber]->m_specInfo.m_device);
			
		// Initialize the controls (mostly the spin-button)
		InitializeControls();

		// Select which scan(s) to show
		SelectScan();

		// draw the map
		DrawMap();
	}else{
		return 0;
	}

	m_comboVolcano.EnableWindow(TRUE);

	return 1;
}

/** Called when the user has change the plume-height assumption */
void CGeometryDlg::OnChangePlumeHeight(){
	UpdateData(TRUE); // <-- save the data in the dialog

	DrawMap(); // <-- redraw the map
}

/** Called when the user has change the assumed compass-direction */
void CGeometryDlg::OnChangeCompass(){
	CString str;
	UpdateData(TRUE); // <-- save the data in the dialog

	// Save the compass-direction
	int curScanner = m_scannerList.GetCurSel();
	if(curScanner < 0 || m_evalLogReader[curScanner] == NULL)
		return;
	GetDlgItemText(IDC_EDIT_COMPASS, str);
	sscanf(str, "%f", &m_evalLogReader[curScanner]->m_specInfo.m_compass);

	DrawMap(); // <-- redraw the map
}

BOOL CGeometryDlg::OnInitDialog()
{
	CDialog::OnInitDialog();


	CRect rect;
	Common common;
	m_frameMap.GetWindowRect(rect);
	int height = rect.bottom - rect.top;
	int width  = rect.right - rect.left;
	rect.top = 20; rect.bottom = height - 10;
	rect.left = 10; rect.right = width - 10;
	m_map.Create(WS_VISIBLE | WS_CHILD, rect, &m_frameMap);
	m_map.SetXUnits(common.GetString(AXIS_LONGITUDE));
	m_map.SetYUnits(common.GetString(AXIS_LATITUDE));
	m_map.EnableGridLinesX(true);
	m_map.SetBackgroundColor(RGB(230, 230, 230));
	m_map.SetPlotColor(RGB(255, 0, 0));
	m_map.SetCircleColor(RGB(255, 0, 0));
	m_map.SetGridColor(RGB(0, 0, 0));
	m_map.SetRange(-1, 1, 3, -1, 1, 3);

	// disable the volcano picker until we've opened an evaluation log
	m_comboVolcano.EnableWindow(FALSE);

	// Initialize the volcano list
	for(unsigned int i = 0; i < g_volcanoes.m_volcanoNum; ++i){
		m_comboVolcano.AddString(g_volcanoes.m_name[i]);
	}

	// Check if the evaluation-log has been set from outside of this dialog
	if(m_evalLogReader[0] != NULL){
		m_curScan[0] = 0;

		// Get the position of the scanning instrument
		double lat, lon;
		long	alt;
		GetGPSData(0, lat, lon, alt);
		if(fabs(lat) < 1e-5 && fabs(lon) < 1e-5){// <-- no gps-data
			MessageBox("Evaluation log does not contain gps-data");
			return 0; 
		}else{
			// remember the data
			m_gps[0].m_latitude	= lat;
			m_gps[0].m_longitude = lon;
			m_gps[0].m_altitude	= alt;
		}

		// Get the nearest volcano
		m_volcanoIndex[0] = CGeometryCalculator::GetNearestVolcano(lat, lon);
		if(m_volcanoIndex[0] == -1){
			MessageBox("No volcano could be found close to this instrument", "Error");
			return 0; // <-- no volcano could be found
		}

		// Get the serial-number of the spectrometer
		m_serialNumber[0].Format(m_evalLogReader[0]->m_specInfo.m_device);
			
		// Initialize the controls (mostly the spin-button)
		InitializeControls();

		// Select which scan(s) to show
		SelectScan();

		// draw the map
		DrawMap();

		// Show the name of the evaluation log
		SetDlgItemText(IDC_EDIT_EVALLOG1, m_evalLogReader[0]->m_evaluationLog);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CGeometryDlg::DrawMap(){
	static const int BUFFERSIZE = 128;
	static double iLat[BUFFERSIZE];
	static double iLon[BUFFERSIZE];
	static double iCol[BUFFERSIZE];
	int		nScanners = 0; // <-- the number of scanners to draw
	int scanner;				// <-- iterator over the scanners
	double maxColumn = -1e16;	// <-- the highest column we will draw
	double minColumn = 1e16;	// <-- the lowest column we will draw
	int k; // <-- iterator over spectra

	UpdateData(TRUE); // <-- save the data in the dialog

	// First set the range of the plot
	nScanners = SetPlotRange();

	// If there are no scanners defined, return now
	if(nScanners == 0)
		return;

	// Make sure that the plot is square, i.e. the range in latitude is 
	//	same as the range in longitude
	if(m_plotRange.maxLon - m_plotRange.minLon > m_plotRange.maxLat - m_plotRange.minLat){
		double midLat = (m_plotRange.maxLat + m_plotRange.minLat) * 0.5;
		m_plotRange.maxLat = midLat + 0.5*(m_plotRange.maxLon - m_plotRange.minLon);
		m_plotRange.minLat = midLat - 0.5*(m_plotRange.maxLon - m_plotRange.minLon);
	}else{
		double midLon = (m_plotRange.maxLon + m_plotRange.minLon) * 0.5;
		m_plotRange.maxLon	= midLon + 0.5*(m_plotRange.maxLat - m_plotRange.minLat);
		m_plotRange.minLon	= midLon - 0.5*(m_plotRange.maxLat - m_plotRange.minLat);
	}

	// Now set the range for the plot!
	m_map.SetRange( m_plotRange.minLon - m_plotRange.margin, m_plotRange.maxLon + m_plotRange.margin, 3,
									m_plotRange.minLat - m_plotRange.margin, m_plotRange.maxLat + m_plotRange.margin, 3);

	// To fix the scale for the column-values, we must normalize it. Find the 
	//	highest and the lowest column value there is
	for(scanner = 0; scanner < MAX_N_SCANNERS; ++scanner){
		// If this evaluation-log has not yet been opened, then check the next one
		if(m_evalLogReader[scanner] == NULL || strlen(m_evalLogReader[scanner]->m_evaluationLog) <= 0)
			continue;
		if((m_evalLogReader[scanner]->m_scanNum <= 0) || (m_evalLogReader[scanner]->m_specieNum <= 0))
			continue;

		// Calculate the offset for this scanner...
		m_evalLogReader[scanner]->m_scan[m_curScan[scanner]].CalculateOffset(m_evalLogReader[scanner]->m_scan[m_curScan[scanner]].GetSpecieName(0, 0));

		// Get the columns
		for(k = 0; k < m_evalLogReader[scanner]->m_scan[m_curScan[scanner]].GetEvaluatedNum(); ++k){
			if(m_evalLogReader[scanner]->m_scan[m_curScan[scanner]].IsOk(k)){
				double curColumn = m_evalLogReader[scanner]->m_scan[m_curScan[scanner]].GetColumn(k, 0);
				maxColumn = max(maxColumn, curColumn);
				minColumn = min(minColumn, curColumn);
			}
		}
	}


	// Go through the scanners again and draw the data
	for(scanner = 0; scanner < MAX_N_SCANNERS; ++scanner){
		// If this evaluation-log has not yet been opened, then check the next one
		if(m_evalLogReader[scanner] == NULL || strlen(m_evalLogReader[scanner]->m_evaluationLog) <= 0)
			continue;
		if((m_evalLogReader[scanner]->m_scanNum <= 0) || (m_evalLogReader[scanner]->m_specieNum <= 0))
			continue;

		// Get the intersection points
		int nPoints = CalculateIntersectionPoints(scanner, m_curScan[scanner], iLat, iLon, iCol, BUFFERSIZE);

		// Normalize the columns to the range [0->1]
		for(k = 0; k < nPoints; ++k){
			iCol[k] = (iCol[k] - minColumn) / (maxColumn - minColumn);
		}

		// get the volcanoes latitude & longitude
		double vLat = g_volcanoes.m_peakLatitude[m_volcanoIndex[scanner]];
		double vLon = g_volcanoes.m_peakLongitude[m_volcanoIndex[scanner]];

		// Draw the volcano
		if(m_showVolcano){
			m_map.SetCircleRadius(5);
			m_map.SetCircleColor(RGB(255, 0, 0)); // <-- make the volcano red
			m_map.DrawCircles(&vLon, &vLat, 1, Graph::CGraphCtrl::PLOT_FIXED_AXIS);
		}

		// Draw the scanning instrument
		m_map.SetCircleColor(RGB(0, 0, 0)); // <-- make the instrument black
		m_map.DrawCircles(&m_gps[scanner].m_longitude, &m_gps[scanner].m_latitude, 1, Graph::CGraphCtrl::PLOT_FIXED_AXIS);

		// Fix the options for how to draw the scan
		int plotOption = Graph::CGraphCtrl::PLOT_FIXED_AXIS | Graph::CGraphCtrl::PLOT_CIRCLES | Graph::CGraphCtrl::PLOT_NORMALIZED_COLORS;
		if(m_showColumnAsSize)
			plotOption |= Graph::CGraphCtrl::PLOT_SCALE_CIRCLE;

		// Draw the scan
		m_map.DrawCircles(iLon, iLat, iCol, nPoints, plotOption);

		// Update the selection box
		m_comboVolcano.SetCurSel(m_volcanoIndex[scanner]);

		// Calculate the solar zenith angle and the solar azimuth angle
		ShowSunPosition(scanner);
	}

	// Update the text in the window
	InitLegend();

	// Calculate and show the winddirection
	int curScanner = m_scannerList.GetCurSel();
	CalculateWindDirection(curScanner, m_curScan[curScanner]);

	// If there are more than two evaluation-log files then we can combine them
	//	to calculate plume-height
	if(nScanners > 1)
		CalculatePlumeHeight(0, m_curScan[0], 1, m_curScan[1]);
}

int CGeometryDlg::SetPlotRange(){
	static const int BUFFERSIZE = 128;
	static double iLat[BUFFERSIZE];
	static double iLon[BUFFERSIZE];
	static double iCol[BUFFERSIZE];
	int nScanners = 0;

	m_plotRange.margin = 0.02;

	// First go through the scanners to get the ranges of the data...
	for(int scanner = 0; scanner < MAX_N_SCANNERS; ++scanner){
		// If this evaluation-log has not yet been opened, then check the next one
		if(m_evalLogReader[scanner] == NULL || strlen(m_evalLogReader[scanner]->m_evaluationLog) <= 0)
			continue;
		if((m_evalLogReader[scanner]->m_scanNum <= 0) || (m_evalLogReader[scanner]->m_specieNum <= 0))
			continue;

		// Get the intersection points
		int nPoints = CalculateIntersectionPoints(scanner, m_curScan[scanner], iLat, iLon, iCol, BUFFERSIZE);

		// get the volcanoes latitude & longitude
		double vLat = g_volcanoes.m_peakLatitude[m_volcanoIndex[scanner]];
		double vLon = g_volcanoes.m_peakLongitude[m_volcanoIndex[scanner]];

		// Get the ranges for the plot
		if(nScanners == 0){
			m_plotRange.minLat = m_plotRange.maxLat = m_gps[scanner].Latitude();
			m_plotRange.minLon = m_plotRange.maxLon	= m_gps[scanner].Longitude();
		}else{
			m_plotRange.minLat = min(m_plotRange.minLat, m_gps[scanner].Latitude());
			m_plotRange.maxLat = max(m_plotRange.maxLat, m_gps[scanner].Latitude());
			m_plotRange.minLon = min(m_plotRange.minLon, m_gps[scanner].Longitude());
			m_plotRange.maxLon = max(m_plotRange.maxLon, m_gps[scanner].Longitude());
		}
		if(m_showVolcano){
			m_plotRange.minLat = min(m_plotRange.minLat, vLat);
			m_plotRange.maxLat = max(m_plotRange.maxLat, vLat);
			m_plotRange.minLon = min(m_plotRange.minLon, vLon);
			m_plotRange.maxLon = max(m_plotRange.maxLon, vLon);
		}
		m_plotRange.minLat = min(m_plotRange.minLat, Min(iLat, nPoints));
		m_plotRange.maxLat = max(m_plotRange.maxLat, Max(iLat, nPoints));
		m_plotRange.minLon = min(m_plotRange.minLon, Min(iLon, nPoints));
		m_plotRange.maxLon = max(m_plotRange.maxLon, Max(iLon, nPoints));

		++nScanners;
	}

	return nScanners;
}

void CGeometryDlg::ShowSunPosition(int scanner){
	double saz, sza; // solar azimuth and zenith angle 
	GetSunPosition(scanner, m_curScan[scanner], sza, saz);
	CString msgAngle;
	msgAngle.Format("SZA: %.1lf [deg]", sza);
	SetDlgItemText(IDC_LABEL_SZA, msgAngle);
	msgAngle.Format("SAZ: %.1lf [deg]", saz);
	SetDlgItemText(IDC_LABEL_SAZ, msgAngle);

	// If the user wants to see the direction towards the sun in the map, then
	//	calculate and show it also
	if(m_showSunRay){
		double sunLat[2], sunLon[2];
		// The length of the line showing the direction to the sun
		double L = sin(sza*DEGREETORAD) * (2*m_plotRange.margin + min(m_plotRange.maxLat - m_plotRange.minLat, m_plotRange.maxLon - m_plotRange.minLon));

		sunLat[0] = m_gps[scanner].Latitude();
		sunLon[0]	= m_gps[scanner].Longitude();
		sunLat[1] = m_gps[scanner].Latitude()  + L * cos(saz * DEGREETORAD);
		sunLon[1] = m_gps[scanner].Longitude() + L * sin(saz * DEGREETORAD);
		m_map.SetPlotColor(RGB(255, 255, 0));
		m_map.XYPlot(sunLon, sunLat, 2, Graph::CGraphCtrl::PLOT_FIXED_AXIS | Graph::CGraphCtrl::PLOT_CONNECTED);
	}
}

/** This function extracts the gps - data from the evaluation log */
void	CGeometryDlg::GetGPSData(int seriesNumber, double &lat, double &lon, long &alt){
	lat = 0.0;
	lon = 0.0;
	alt = 0;
	int nAveraged = 0;

	// no data to read
	if(m_evalLogReader[seriesNumber] == NULL || m_evalLogReader[seriesNumber]->m_scanNum <= 0 )
		return;

	// check all data
	for(int k = 0; k < m_evalLogReader[seriesNumber]->m_scanNum; ++k){
		double tmpLat = m_evalLogReader[seriesNumber]->m_scan[k].GetSpectrumInfo(0).m_gps.Latitude();
		double tmpLon = m_evalLogReader[seriesNumber]->m_scan[k].GetSpectrumInfo(0).m_gps.Longitude();
		double tmpAlt	= m_evalLogReader[seriesNumber]->m_scan[k].GetSpectrumInfo(0).m_gps.Altitude();
		
		// Check if this position has GPS-data or not
		if(fabs(tmpLat) < 1e-5 || fabs(tmpLon) < 1e-5)
			continue;

		// Check so that the GPS-data is reasonable
		if(fabs(tmpLat) > 180.0 || fabs(tmpLon) > 180.0)
			continue;

		lat += tmpLat;
		lon += tmpLon;
		alt += (long)tmpAlt;
		++nAveraged;
	}

	if(nAveraged == 0)
		return;

	lat /= nAveraged;
	lon /= nAveraged;
	alt /= nAveraged;

	return; // <-- success!!
}	

/** Calculates the latitude and longitude for
		the measurements, at plume height. */
int	CGeometryDlg::CalculateIntersectionPoints(int seriesNumber, int scanNumber, double *lat, double *lon, double *column, int dataLength){
	// Check the parameters
	if(m_evalLogReader[seriesNumber] == NULL || scanNumber > m_evalLogReader[seriesNumber]->m_scanNum)
		return 0;
	if(scanNumber < 0)
		scanNumber = 0;
	if(lat == 0 || lon == 0)
		return 0;

	double sLat = m_gps[seriesNumber].Latitude();
	double sLon = m_gps[seriesNumber].Longitude();

	Common common;

	// the number of points
	int nPoints = min(dataLength, m_evalLogReader[seriesNumber]->m_scan[scanNumber].GetEvaluatedNum());

	// Get the cone-angle
	double coneAngle	=	m_evalLogReader[seriesNumber]->m_specInfo.m_coneAngle;

	// Get the tilt
	double tilt				= m_evalLogReader[seriesNumber]->m_specInfo.m_pitch;

	//calculate the points
	int index = 0;
	if(fabs(coneAngle - 90) < 5){
		// ----------- FLAT SCANNERS ---------------
		for(int k = 0; k < min(dataLength, m_evalLogReader[seriesNumber]->m_scan[scanNumber].GetEvaluatedNum()); ++k){
			double scanAngle = m_evalLogReader[seriesNumber]->m_scan[scanNumber].GetScanAngle(k);

			if(fabs(scanAngle) > 85){
				--nPoints;
				continue;
			}

			// the distance from the system to the intersection-point
			double intersectionDistance = m_plumeHeight * tan(DEGREETORAD * scanAngle);

			// the direction from the system to the intersection-point
			double angle = (m_compass[seriesNumber] - 90);

			// the intersection-points
			common.CalculateDestination(sLat, sLon, intersectionDistance, angle, lat[index], lon[index]);

			// the columns (for specie 0) at the intersection-point
			if(m_evalLogReader[seriesNumber]->m_scan[scanNumber].IsOk(k)){
				column[index] = m_evalLogReader[seriesNumber]->m_scan[scanNumber].GetColumn(k, 0);
				column[index] -= m_evalLogReader[seriesNumber]->m_scan[scanNumber].GetOffset();
			}else
				column[index] = 0.0;

			++index;
		}
	}else{
		// ----------- CONE SCANNERS ---------------
		for(int k = 0; k < min(dataLength, m_evalLogReader[seriesNumber]->m_scan[scanNumber].GetEvaluatedNum()); ++k){
			double scanAngle = m_evalLogReader[seriesNumber]->m_scan[scanNumber].GetScanAngle(k);

			if(fabs(scanAngle) > 85){
				--nPoints;
				continue;
			}

			double cos_tilt				=	cos(tilt * DEGREETORAD);
			double sin_tilt				= sin(tilt * DEGREETORAD);
			double cos_alpha			= cos(scanAngle * DEGREETORAD);
			double sin_alpha			=	sin(scanAngle * DEGREETORAD);
			double tan_coneangle	=	tan(coneAngle * DEGREETORAD);

			// Calculate the distance from the system to the intersection point
			double commonDenominator	= cos_alpha * cos_tilt + sin_tilt / tan_coneangle;

			double x = (cos_tilt/tan_coneangle - cos_alpha*sin_tilt) /	commonDenominator;
			double y = sin_alpha	/ commonDenominator;

			double intersectionDistance =	m_plumeHeight * sqrt(x*x + y*y);

			// the direction from the system to the intersection-point
			double angle = m_compass[seriesNumber] + RADTODEGREE * atan2(y, x);

			// the intersection-points
			common.CalculateDestination(sLat, sLon, intersectionDistance, angle, lat[index], lon[index]);

			// the columns (for specie 0) at the intersection-point
			if(m_evalLogReader[seriesNumber]->m_scan[scanNumber].IsOk(k))
				column[index] = m_evalLogReader[seriesNumber]->m_scan[scanNumber].GetColumn(k, 0);
			else
				column[index] = 0.0;

			++index;
		}
	}

	return nPoints;
}

/** Calculates and shows the wind direction for scan number 'scanNumber'. */
void	CGeometryDlg::CalculateWindDirection(int seriesNumber, int scanNumber){
	// Check the parameters
	if(m_evalLogReader[seriesNumber] == NULL || scanNumber > m_evalLogReader[seriesNumber]->m_scanNum)
		return;
	if(scanNumber < 0)
		scanNumber = 0;

	double maxColumn = -1e16;
	int		 indexOfMax = -1;

	// Get the maximum column
	int nDataPoints = m_evalLogReader[seriesNumber]->m_scan[scanNumber].GetEvaluatedNum();
	for(int k = 0; k < nDataPoints; ++k){
		double curColumn = m_evalLogReader[seriesNumber]->m_scan[scanNumber].GetColumn(k, 0);
		if(curColumn > maxColumn){
			if(m_evalLogReader[seriesNumber]->m_scan[scanNumber].IsOk(k)){
				maxColumn		= curColumn;
				indexOfMax	= k;
			}
		}
	}

	// check so not all points are bad
	if(indexOfMax == -1)
		return;

	// the location of the system
	double sLat = m_gps[seriesNumber].Latitude();
	double sLon	= m_gps[seriesNumber].Longitude();

	// calculate the intersection point...
	
	// the scan-angle
	double scanAngle = m_evalLogReader[seriesNumber]->m_scan[scanNumber].GetScanAngle(indexOfMax);

	// the distance from the system to the intersection-point
	double intersectionDistance = m_plumeHeight * tan(DEGREETORAD * scanAngle);

	// the direction from the system to the intersection-point
	double angle = (m_compass[seriesNumber] - 90);

	// the intersection-point
	double lat2, lon2;
	Common common;
	common.CalculateDestination(sLat, sLon, intersectionDistance, angle, lat2, lon2);

	// the bearing from the volcano to the intersection-point

	// Get the nearest volcano
	int volcanoIndex = CGeometryCalculator::GetNearestVolcano(sLat, sLon);
	if(volcanoIndex == -1)
		return; // <-- no volcano could be found

	// get the volcanoes latitude & longitude
	double vLat = g_volcanoes.m_peakLatitude[volcanoIndex];
	double vLon = g_volcanoes.m_peakLongitude[volcanoIndex];

	// the wind-direction
	m_calcWindDirection = common.GPSBearing(vLat, vLon, lat2, lon2);

	CString label;
	label.Format("%.2lf [deg]", m_calcWindDirection);
	SetDlgItemText(IDC_LABEL_WINDDIRECTION, label);
}
void CGeometryDlg::OnChangeSelectedScan(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);

	m_curScanner = m_scannerList.GetCurSel();
	if(m_curScanner < 0 || m_evalLogReader[m_curScanner] == NULL)
		return;

	// If there's no scans read, we cannot change the current scannumber
	if(m_evalLogReader[m_curScanner]->m_scanNum <= 0){
		m_curScan[m_curScanner] = 0;
		return;
	}

	// Set the currently selected scan
	if(pNMUpDown->iDelta < 0)
		--m_curScan[m_curScanner];
	else
		++m_curScan[m_curScanner];

	// set the limits for the selected scan
	m_curScan[m_curScanner] = max(m_curScan[m_curScanner], 0);
	m_curScan[m_curScanner] = min(m_curScan[m_curScanner], m_evalLogReader[m_curScanner]->m_scanNum - 1);

	*pResult = 0;

	// Select which scans we should show
	SelectScan();

	// Re-draw the map
	DrawMap();
}

void CGeometryDlg::InitializeControls(){
	CString str;

	// Clear the list of read-in scanners.
	m_scannerList.ResetContent();

	for(int k = 0; k < MAX_N_SCANNERS; ++k){
		if(m_evalLogReader[k] == NULL || strlen(m_evalLogReader[k]->m_evaluationLog) <= 0)
			continue;

		str.Format("%s (%d scans)", m_serialNumber[k], m_evalLogReader[k]->m_scanNum);
		m_scannerList.AddString(str);
	}

	// How many scanners are there?
	int nScanners = m_scannerList.GetCount();
	if(nScanners <= 0)
		return;

	// Select the last scanner in the list
	m_scannerList.SetCurSel(nScanners - 1);
	
	// We've changed the scanner
	OnChangeScanner();
}

/** Called when the user changes the selected scanner in the list of scanners */
void CGeometryDlg::OnChangeScanner(){
	int curScanner = m_scannerList.GetCurSel();
	if(curScanner < 0)
		return;

	// Initialize the spin control
	m_scanSpinCtrl.SetRange(0, (short)(m_evalLogReader[curScanner]->m_scanNum));
	m_scanSpinCtrl.SetPos((short)m_curScan[curScanner]);

	// Update the legend on the screen
	InitLegend();

	// Re-draw the map
	DrawMap();
}

/** Intitializing the plot-legend */
void CGeometryDlg::InitLegend(){
	Common common;
	CString distanceLabel, compassLabel, scannumber, message;

	// Get the currently selected scanner
	m_curScanner = m_scannerList.GetCurSel();
	if(m_curScanner < 0 || m_evalLogReader[m_curScanner] == NULL)
		return;

	// get the volcanoes latitude & longitude
	double vLat = g_volcanoes.m_peakLatitude[m_volcanoIndex[m_curScanner]];
	double vLon = g_volcanoes.m_peakLongitude[m_volcanoIndex[m_curScanner]];

	// Calculate the distance to the volcano
	double dist = common.GPSDistance(vLat, vLon, m_gps[m_curScanner].Latitude(), m_gps[m_curScanner].Longitude());
	distanceLabel.Format("Distance: %.1lf [km]", dist * 0.001);
	SetDlgItemText(IDC_LABEL_VOLCANODISTANCE, distanceLabel);

	// Show the compass direction
	m_compass[m_curScanner] = m_evalLogReader[m_curScanner]->m_specInfo.m_compass;
	//compassLabel.Format("%.1f", m_evalLogReader[m_curScanner]->m_specInfo.m_compass);
	//SetDlgItemText(IDC_EDIT_COMPASS, compassLabel);

	// Update the start-time label on the screen
	const CSpectrumTime *startTime = m_evalLogReader[m_curScanner]->m_scan[m_curScan[m_curScanner]].GetStartTime(0);
	if(startTime != NULL){
		unsigned short date[3];
		if(SUCCESS == m_evalLogReader[m_curScanner]->m_scan[m_curScan[m_curScanner]].GetDate(0, date)){
			message.Format("Scan started on: %04d.%02d.%02d at %02d:%02d:%02d", 
			date[0], date[1], date[2], startTime->hr, startTime->m, startTime->sec);
		}else{
			message.Format("Scan started at: %02d:%02d:%02d", startTime->hr, startTime->m, startTime->sec);
		}
	}

	// Update the values
	UpdateData(FALSE);
}

/** Calculates and shows the plume height by combining the two scans
		'scanNumber1' and 'scanNumber2' from the two measurement series
		'seriesNumber1' and 'seriesNumber2'. */
int CGeometryDlg::CalculatePlumeHeight(int seriesNumber1, int scanNumber1, int seriesNumber2, int scanNumber2){
	CString label;
	double compass[2], plumeCentre[2], coneAngle[2], tilt[2];
	double plumeCompleteness, tmp, plumeEdge_low, plumeEdge_high;
	CGPSData gps[2], source;

	// 1. Get the gps-positions
	gps[0] = m_gps[seriesNumber1];
	gps[1] = m_gps[seriesNumber2];

	// 2. Get the plume centre positions
	if(false == m_evalLogReader[seriesNumber1]->m_scan[scanNumber1].CalculatePlumeCentre("SO2", plumeCentre[0], tmp, plumeCompleteness, plumeEdge_low, plumeEdge_high))
		return 1;	// <-- cannot see the plume
	if(false == m_evalLogReader[seriesNumber2]->m_scan[scanNumber2].CalculatePlumeCentre("SO2", plumeCentre[1], tmp, plumeCompleteness, plumeEdge_low, plumeEdge_high))
		return 1;	// <-- cannot see the plume

	// 3. Get the cone-angles
	coneAngle[0] = m_evalLogReader[seriesNumber1]->m_scan[scanNumber1].GetConeAngle();
	coneAngle[1] = m_evalLogReader[seriesNumber2]->m_scan[scanNumber2].GetConeAngle();

	// 4. Get the tilt of the systems
	tilt[0] = m_evalLogReader[seriesNumber1]->m_specInfo.m_pitch;
	tilt[1] = m_evalLogReader[seriesNumber2]->m_specInfo.m_pitch;

	// 5. Get the compass-directions of the systems
	compass[0]	=	m_evalLogReader[seriesNumber1]->m_specInfo.m_compass;
	compass[1]	=	m_evalLogReader[seriesNumber2]->m_specInfo.m_compass;

	// 6. Get the position of the source
	source.m_latitude = g_volcanoes.m_peakLatitude[m_volcanoIndex[1]];
	source.m_longitude= g_volcanoes.m_peakLongitude[m_volcanoIndex[1]];

	if(false == CGeometryCalculator::GetPlumeHeight_Exact(gps, compass, plumeCentre, coneAngle, tilt, m_calcPlumeHeight)){
			if(false == CGeometryCalculator::GetPlumeHeight_Fuzzy(source, gps, compass, plumeCentre, coneAngle, tilt, m_calcPlumeHeight))
				return 1; // could not calculate anything
	}

	// successfully calculated a plume-height, tell the user what we've done.
	label.Format("%.2lf [m]", m_calcPlumeHeight);
	SetDlgItemText(IDC_LABEL_PLUMEHEIGHT, label);

	return 0;
}

/** This function calculates the solar azimuth (saz) and solar zenith (sza)
		angles for the time of the given scan. */
void	CGeometryDlg::GetSunPosition(int seriesNumber, int scanNumber, double &sza, double &saz){
	CDateTime gmtTime;

	m_evalLogReader[seriesNumber]->m_scan[scanNumber].GetStartTime(0, gmtTime);
	Common::GetSunPosition(gmtTime, m_gps[seriesNumber].Latitude(), m_gps[seriesNumber].Longitude(), sza, saz);
}

/** Called when the user has changed which is to be the source for the current
			scanner. */
void	CGeometryDlg::OnChangeSource(){
	// The currently selected scanner 
	int curScanner = m_scannerList.GetCurSel();
	if(curScanner == -1)
		return;

	m_volcanoIndex[curScanner] = m_comboVolcano.GetCurSel();

	// Update the graph
	DrawMap();
}

/** The user wants to save the current graph as an image */
void CGeometryDlg::OnMenu_SaveGraph(){
	CString fileName;
	TCHAR filter[512];
	int n = _stprintf(filter, "Image File\0");
	n += _stprintf(filter + n + 1, "*.png;*.bmp;*.gif;*.jpg\0");
	filter[n + 2] = 0;
	Common common;

	if(common.BrowseForFile_SaveAs(filter, fileName)){
		if(!Equals(fileName.Right(4), ".bmp") && !Equals(fileName.Right(4), ".jpg") && !Equals(fileName.Right(4), ".gif") && !Equals(fileName.Right(4), ".png")){
			// If the user has not supplied a file-ending to the file, then append ".png"
			fileName.AppendFormat(".png");
		}
		m_map.SaveGraph(fileName);
	}
}

void Dialogs::CGeometryDlg::SelectScan(){
	CString dateAndTimeStr;

	// When we change the time for the currently selected scanner, 
	//	we should also change the time for the other scanner(s) to the
	//	scan closest in time to the current one
	CDateTime currentlySelectedTime, time2;
	int otherScanners[2];
	double diffTime, smallestDiffTime;
	const Evaluation::CScanResult &curScan = m_evalLogReader[m_curScanner]->m_scan[m_curScan[m_curScanner]];
	curScan.GetStartTime(0, currentlySelectedTime);
	if(m_curScanner == 0){
		otherScanners[0] = 1; otherScanners[1] = 2;
	}else if(m_curScanner == 1){
		otherScanners[0] = 0; otherScanners[1] = 2;
	}else{
		otherScanners[0] = 0; otherScanners[1] = 1;
	}
	if(m_evalLogReader[otherScanners[0]] != NULL){
		// Select the scan which has the smallest difference to the currently selected scan
		smallestDiffTime = 1e9;
		for(int i = 0; i < m_evalLogReader[otherScanners[0]]->m_scanNum; ++i){
			m_evalLogReader[otherScanners[0]]->m_scan[i].GetStartTime(0, time2);
			diffTime = fabs(CDateTime::Difference(currentlySelectedTime, time2));
			if(diffTime < smallestDiffTime){
				smallestDiffTime = diffTime;
				m_curScan[otherScanners[0]] = i;
			}
		}
	}
	if(m_evalLogReader[otherScanners[1]] != NULL){
		// Select the scan which has the smallest difference to the currently selected scan
		smallestDiffTime = 1e9;
		for(int i = 0; i < m_evalLogReader[otherScanners[1]]->m_scanNum; ++i){
			m_evalLogReader[otherScanners[1]]->m_scan[i].GetStartTime(0, time2);
			diffTime = fabs(CDateTime::Difference(currentlySelectedTime, time2));
			if(diffTime < smallestDiffTime){
				smallestDiffTime = diffTime;
				m_curScan[otherScanners[1]] = i;
			}
		}
	}

	// THE DATE AND TIME
	if(m_evalLogReader[0] != NULL){
		m_evalLogReader[0]->m_scan[m_curScan[0]].GetStartTime(0, time2);
		dateAndTimeStr.Format("%04d-%02d-%02d %02d:%02d:%02d", time2.year, time2.month, time2.day, time2.hour, time2.minute, time2.second);
		SetDlgItemText(IDC_LABEL_DATEANDTIME, dateAndTimeStr);
	}
	if(m_evalLogReader[1] != NULL){
		m_evalLogReader[1]->m_scan[m_curScan[1]].GetStartTime(0, time2);
		dateAndTimeStr.Format("%04d-%02d-%02d %02d:%02d:%02d", time2.year, time2.month, time2.day, time2.hour, time2.minute, time2.second);
		SetDlgItemText(IDC_LABEL_DATEANDTIME2, dateAndTimeStr);
	}
	if(m_evalLogReader[2] != NULL){
		m_evalLogReader[2]->m_scan[m_curScan[2]].GetStartTime(0, time2);
		dateAndTimeStr.Format("%04d-%02d-%02d %02d:%02d:%02d", time2.year, time2.month, time2.day, time2.hour, time2.minute, time2.second);
		SetDlgItemText(IDC_LABEL_DATEANDTIME3, dateAndTimeStr);
	}
}


/** The user wants to calculate the plume heights and wind-directions
		for all possible combinations of scans */
void Dialogs::CGeometryDlg::OnMenu_CalculateGeometries(){
	CString userInputStr;
	CString fileName, volcanoName, serial1, serial2;
	CString message;
	double maxStartTimeDifference;
	int it, it2; // <- iterators for evaluation-logs
	int scanNumber1, scanNumber2; // <-- iterators for scans
	int nOpenedEvalLogs = 0;
	int nCalculatedResults;
	CDateTime time1, time2;

	// Check the number of opened evaluation-logs. Need at least 2 to calculate anything
	for(it = 0; it < MAX_N_SCANNERS; ++it){
		if(m_evalLogReader[it] != NULL)
			++nOpenedEvalLogs;
	}
	if(nOpenedEvalLogs < 2){
		MessageBox("You need to open at least two evaluation-logs to be able to calculate wind-directions and plume heights");
		return;
	}

	// Ask the user for the maximum time-difference between scans to use
	Dialogs::CQueryStringDialog timeDialog;
	timeDialog.m_windowText.Format("Combine scans with start-time-difference less than (minutes):");
	timeDialog.m_inputString = &userInputStr;
	INT_PTR ret = timeDialog.DoModal();

	if(IDCANCEL == ret)
		return;

	// Check what the user typed
	if(0 == sscanf(userInputStr, "%lf", &maxStartTimeDifference)){
		MessageBox("That is not a number, analysis aborted");
		return;
	}
	maxStartTimeDifference *= 60; // <-- convert to seconds from minutes

	nCalculatedResults = 0;

	// Now combine all scans with a not too large time-difference
	for(it = 0; it < MAX_N_SCANNERS - 1; ++it){			// <-- these two for-loops loop through all combinations of the evaluation-logs
		if(m_evalLogReader[it] == NULL)
			continue;

		// Serial-number of spectrometer #1
		serial1.Format("%s", m_evalLogReader[it]->m_scan[0].GetSerial());

		for(it2 = it+1; it2 < MAX_N_SCANNERS; ++it2){
			if(m_evalLogReader[it2] == NULL)
				continue;

			// Serial-number of spectrometer #2
			serial2.Format("%s", m_evalLogReader[it2]->m_scan[0].GetSerial());

			// Open a post-geometry log
			FILE *f = OpenPostGeometryLogFile(it, it2, fileName);
			// The name of the volcano we're working on...
			if(m_volcanoIndex[0] != -1)
				volcanoName.Format("%s", g_volcanoes.m_simpleName[m_volcanoIndex[0]]);
			else
				volcanoName.Format("Unknown");

			for(scanNumber1 = 0; scanNumber1 < m_evalLogReader[it]->m_scanNum; ++scanNumber1){ // <-- loop through all scans in eval-log #1
				// Start-time of scan #1
				m_evalLogReader[it]->m_scan[scanNumber1].GetStartTime(0, time1);

				for(scanNumber2 = 0; scanNumber2 < m_evalLogReader[it2]->m_scanNum; ++scanNumber2){ // <-- loop through all scans in eval-log #2
					// Start-time of scan #2
					m_evalLogReader[it2]->m_scan[scanNumber2].GetStartTime(0, time2);

					// Check the difference in start-time
					if(maxStartTimeDifference < fabs(CDateTime::Difference(time1, time2)))
						continue;

					// we're ok, combine the two scans...

					// 1. Calculate the plume height
					if(0 == CalculatePlumeHeight(it, scanNumber1, it2, scanNumber2)){
						m_plumeHeight = m_calcPlumeHeight;
						++nCalculatedResults;

						// 2. Calculate the Wind-direction
						CalculateWindDirection(it,  scanNumber1);
						//CalculateWindDirection(it2, scanNumber2);

						// 3. Output the results
						fprintf(f, "%s\t%s\t%s\t",			volcanoName, serial1, serial2);
						fprintf(f, "%04d.%02d.%02d\t",	time1.year, time1.month, time1.day);
						fprintf(f, "%02d:%02d:%02d\t",	time1.hour, time1.minute,time1.second);
						fprintf(f, "%02d:%02d:%02d\t",	time2.hour, time2.minute,time2.second);
						fprintf(f, "%.2lf\t%.2lf\n",		m_calcPlumeHeight, m_calcWindDirection);
					}
				}
			}
			// Close the post-geometry log-file
			fclose(f);
		} // end for (it2...
	}// end for(it...

	if(nCalculatedResults > 0){
		message.Format("%d results calculated and written to: %s", nCalculatedResults, fileName);
		MessageBox(message);
	}

	return;
}

/** Opens a post-geometry log-file, returns the handle to the opened file */
FILE *Dialogs::CGeometryDlg::OpenPostGeometryLogFile(int seriesNumber1, int seriesNumber2, CString &fileName){
	CString directory;

	directory.Format(m_evalLogReader[seriesNumber1]->m_evaluationLog);
	Common::GetDirectory(directory);		// get the directory of the evaluation-log files
	directory = directory.Left((int)strlen(directory) - 1);
	Common::GetDirectory(directory);		// get the parent-directory to the evaluation-log files
	fileName.Format("%sPostGeometryLog.txt", directory);
	int exists	= IsExistingFile(fileName);
	FILE *f			= fopen(fileName, "a+");
	if(f == NULL){
		// Ask the user for another directory for the output file
		Dialogs::CQueryStringDialog timeDialog;
		timeDialog.m_windowText.Format("Cannot create output-file. Please give a directory to put output-file...");
		timeDialog.m_inputString = &directory;
		INT_PTR ret = timeDialog.DoModal();

		if(IDCANCEL == ret)
			return NULL;

		// Try to open the file again
		fileName.Format("%sPostGeometryLog.txt", directory);
		exists	= IsExistingFile(fileName);
		f				= fopen(fileName, "a+");
		if(f == NULL){
			MessageBox("Could not create output-file. Analysis aborted");
			return NULL;
		}
	}

	if(!exists){
		fprintf(f, "# This is the PostGeometryLog of the NovacProgram version %d.%02d\n", CVersion::majorNumber, CVersion::minorNumber);
		fprintf(f, "# This file contains the result of combining two scans to calculate plume-height and/or wind-direction\n");
		fprintf(f, "Volcano\tSerialNumber1\tSerialNumber2\tDate\tStartTime1\tStartTime2\tCalculatedPlumeHeight\tWindDirection\n");
	}

	return f;
}
