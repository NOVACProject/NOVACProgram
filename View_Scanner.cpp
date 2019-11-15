// ScannerView.cpp : implementation file
//

#include "stdafx.h"
#include "NovacMasterProgram.h"
#include "View_Scanner.h"
#include "Meteorology/MeteorologicalData.h"
#include "Configuration/Configuration.h"
#include "UserSettings.h"

extern CMeteorologicalData g_metData;      // <-- The metereological data
extern CConfigurationSetting g_settings;   // <-- The scanner/network settings
extern CUserSettings g_userSettings;       // <-- The users preferences

// CView_Scanner dialog
using namespace DlgControls;
using namespace Graph;

void DDX_Text_Formatted(CDataExchange* pDX, int nIDC, double& value, LPCTSTR lpszOutFormat=_T("%f"), LPCTSTR lpszInFormat=_T("%lf"));

// General method for rescaling a CGraphCtrl which is bounded inside a CStatic (owned by a window...)
void ResizeGraphControl(CGraphCtrl& controlToResize, CStatic& boundingFrame, CWnd* owningWindow)
{
    CRect boundingFramClientRegion;
    boundingFrame.GetWindowRect(&boundingFramClientRegion);
    owningWindow->ScreenToClient(boundingFramClientRegion);

    CRect plotControlClientRegion;
    controlToResize.GetWindowRect(&plotControlClientRegion);
    boundingFrame.ScreenToClient(plotControlClientRegion);

    plotControlClientRegion.right = boundingFramClientRegion.Width() - 20;
    plotControlClientRegion.bottom = boundingFramClientRegion.Height() - 10;

    controlToResize.MoveWindow(plotControlClientRegion);
    controlToResize.InvalidateCtrl();
}

IMPLEMENT_DYNAMIC(CView_Scanner, CPropertyPage)
CView_Scanner::CView_Scanner()
	: CPropertyPage(CView_Scanner::IDD)
{
	m_evalDataStorage = NULL;
	m_commDataStorage = NULL;
	m_scannerIndex = 0;
	m_serial.Format("");
	m_siteName.Format("");

	windDirection = g_metData.defaultWindField.GetWindDirection();
	windSpeed     = g_metData.defaultWindField.GetWindSpeed();
	plumeHeight   = g_metData.defaultWindField.GetPlumeHeight();
}

CView_Scanner::~CView_Scanner()
{
}

void CView_Scanner::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_COLUMN, m_lastScanFrame);
	DDX_Control(pDX, IDC_STATIC_FLUX, m_todayScanFrame);

	// The status lights
	DDX_Control(pDX, IDC_STATUS_LIGHT,        m_statusLight);
	DDX_Control(pDX, IDC_COMMSTATUS,          m_commStatusLight);
	DDX_Control(pDX, IDC_LABEL_EVALSTATUS,    m_statusEval);
	DDX_Control(pDX, IDC_LABEL_SCANNERSTATUS, m_statusScanner);

	// the meteorology things
	DDX_Text_Formatted(pDX, IDC_EDIT_PLUMEHEIGHT,   plumeHeight, "%.1lf");
	DDX_Text_Formatted(pDX, IDC_EDIT_WINDDIRECTION, windDirection, "%.1lf");
	DDX_Text_Formatted(pDX, IDC_EDIT_WINDSPEED,     windSpeed, "%.1lf");

	DDX_Control(pDX, IDC_BUTTON_SET_WINDFIELD, m_setButton);
	DDX_Control(pDX, IDC_EDIT_WINDDIRECTION,   m_windDirection);
	DDX_Control(pDX, IDC_EDIT_WINDSPEED,       m_windSpeed);
	DDX_Control(pDX, IDC_EDIT_PLUMEHEIGHT,     m_plumeHeight);
	DDX_Control(pDX, IDC_LABEL_WINDDIRECTION,  m_labelWindDirection);
	DDX_Control(pDX, IDC_LABEL_WINDSPEED,      m_labelWindSpeed);
	DDX_Control(pDX, IDC_LABEL_PLUMEHEIGHT,    m_labelPlumeHeight);

	// The information about the scanner
	DDX_Control(pDX, IDC_CONFINFO_SERIAL,    m_infoSerial);
	DDX_Control(pDX, IDC_CONFINFO_COMPASS,   m_infoCompass);
	DDX_Control(pDX, IDC_CONFINFO_LATITUDE,  m_infoLatitude);
	DDX_Control(pDX, IDC_CONFINFO_LONGITUDE, m_infoLongitude);

	// The information about the last scan
	DDX_Control(pDX, IDC_SCANINFO_STARTTIME,  m_scanInfo_Starttime);
	DDX_Control(pDX, IDC_SCANINFO_STOPTIME,   m_scanInfo_StopTime);
	DDX_Control(pDX, IDC_SCANINFO_DATE,       m_scanInfo_Date);

	DDX_Control(pDX, IDC_SCANINFO_EXPTIME,           m_scanInfo_ExpTime);
	DDX_Control(pDX, IDC_SCANINFO_NSPEC,             m_scanInfo_NumSpec);
	DDX_Control(pDX, IDC_LABEL_COLUMN_COLOR,         m_legend_ColumnColor);
	DDX_Control(pDX, IDC_LABEL_PEAK_INTENSITY_COLOR, m_legend_PeakIntensity);
	DDX_Control(pDX, IDC_LABEL_FIT_INTENSITY_COLOR,  m_legend_FitIntensity);
}


BEGIN_MESSAGE_MAP(CView_Scanner, CPropertyPage)
	// Messages from other parts of the program
	ON_MESSAGE(WM_EVAL_SUCCESS,        OnEvaluatedScan)
	ON_MESSAGE(WM_EVAL_FAILURE,        OnEvaluatedScan)
	ON_MESSAGE(WM_SCANNER_RUN,         OnUpdateEvalStatus)
	ON_MESSAGE(WM_SCANNER_SLEEP,       OnUpdateEvalStatus)
	ON_MESSAGE(WM_SCANNER_NOT_CONNECT, OnUpdateEvalStatus)
	ON_MESSAGE(WM_NEW_WINDFIELD,       OnUpdateWindParam)

	// Commands from the user
	ON_COMMAND(IDC_BUTTON_SET_WINDFIELD, OnBnClickedButtonSetWindfield)
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CView_Scanner message handlers

void CView_Scanner::SetLayout(){
    const int rightMargin = 25;

	CRect thisClientRegion;
	GetWindowRect(thisClientRegion);
    this->ScreenToClient(thisClientRegion);

	// Adjust the width of the column frame
	if(this->m_lastScanFrame.m_hWnd != NULL){
        CRect lastScanFrameClientRegion;
		m_lastScanFrame.GetWindowRect(&lastScanFrameClientRegion);
        this->ScreenToClient(lastScanFrameClientRegion);

        lastScanFrameClientRegion.right  = thisClientRegion.right - rightMargin;
		m_lastScanFrame.MoveWindow(lastScanFrameClientRegion);

        ResizeGraphControl(m_lastScanPlot, m_lastScanFrame, this);
	}

	// Adjust the width of the flux frame
	if(this->m_todayScanFrame.m_hWnd != NULL){
        CRect fluxFrameClientRegion;
		m_todayScanFrame.GetWindowRect(&fluxFrameClientRegion);
        this->ScreenToClient(fluxFrameClientRegion);

        fluxFrameClientRegion.right  = thisClientRegion.right - rightMargin;
		m_todayScanFrame.MoveWindow(fluxFrameClientRegion);

        ResizeGraphControl(m_todayPlot, m_todayScanFrame, this);
	}
}

BOOL CView_Scanner::OnInitDialog()
{
	int height, width;
	CPropertyPage::OnInitDialog();
	Common common;
	CString columnAxisLabel, fluxAxisLabel;

	// Fix the layout first
	//	SetLayout();

	// Load the bitmaps
	m_greenLight.LoadBitmap(MAKEINTRESOURCE(IDB_GREEN_LIGHT));
	m_yellowLight.LoadBitmap(MAKEINTRESOURCE(IDB_YELLOW_LIGHT));
	m_redLight.LoadBitmap(MAKEINTRESOURCE(IDB_RED_LIGHT));

	// Initialize the unit to use
	if(g_userSettings.m_columnUnit == UNIT_MOLEC_CM2){
		columnAxisLabel.Format("%s [ppmm]", (LPCTSTR)common.GetString(AXIS_COLUMN));
	}else{
		columnAxisLabel.Format("%s [molec/cm²]", (LPCTSTR)common.GetString(AXIS_COLUMN));
	}


	// initialize the last scan plot (columns)
	CRect rect;
	m_lastScanFrame.GetWindowRect(rect);
	height = rect.bottom - rect.top;
	width = rect.right - rect.left;
	rect.top = 20; rect.bottom = height - 10;
	rect.left = 10; rect.right = width - 10;
	m_lastScanPlot.Create(WS_VISIBLE | WS_CHILD, rect, &m_lastScanFrame);
	m_lastScanPlot.SetSecondYUnit(common.GetString(AXIS_INTENSITY));
	m_lastScanPlot.SetSecondRangeY(0, 4096, 0);
	m_lastScanPlot.SetXUnits(common.GetString(AXIS_ANGLE));
	m_lastScanPlot.SetYUnits(columnAxisLabel);
	m_lastScanPlot.SetBackgroundColor(RGB(0, 0, 0));
	m_lastScanPlot.SetPlotColor(RGB(255, 0, 0));
	m_lastScanPlot.SetGridColor(RGB(255, 255, 255));
	m_lastScanPlot.SetRange(-90, 90, 0, 0, 100, 0);

	// The legend
	m_legend_ColumnColor.SetBackgroundColor(RGB(255, 0, 0));
	m_legend_PeakIntensity.SetBackgroundColor(RGB(255, 255, 255));
	m_legend_FitIntensity.SetBackgroundColor(RGB(255, 255, 0));
	m_legend_ColumnColor.SetWindowText("");
	m_legend_PeakIntensity.SetWindowText("");
	m_legend_FitIntensity.SetWindowText("");

	// initialize today's plot (may be flux or columns)
	m_todayScanFrame.GetWindowRect(rect);
	height = rect.bottom - rect.top;
	width = rect.right - rect.left;
	rect.top = 20; rect.bottom = height - 10;
	rect.left = 10; rect.right = width - 10;
	m_todayPlot.Create(WS_VISIBLE | WS_CHILD, rect, &m_todayScanFrame);
	m_todayPlot.SetXUnits(common.GetString(AXIS_UTCTIME));
	m_todayPlot.SetXAxisNumberFormat(FORMAT_DATETIME);
	m_todayPlot.SetYUnits("Column");
	m_todayPlot.EnableGridLinesX(true);
	m_todayPlot.SetPlotColor(RGB(255, 255, 255));
	m_todayPlot.SetGridColor(RGB(255, 255, 255));
	m_todayPlot.SetBackgroundColor(RGB(0, 0, 0));
	m_todayPlot.SetMinimumRangeY(1.0);
	m_todayPlot.SetRangeY(0, 100, 0);
	SetTodayRange();

	// Enable the tool tips
	if(!m_toolTip.Create(this)){
		TRACE0("Failed to create tooltip control\n"); 
	}
	m_toolTip.AddTool(&m_windDirection,     IDC_EDIT_WINDDIRECTION);
	m_toolTip.AddTool(&m_labelWindDirection,IDC_EDIT_WINDDIRECTION);
	m_toolTip.AddTool(&m_windSpeed,         IDC_EDIT_WINDSPEED);
	m_toolTip.AddTool(&m_labelWindSpeed,		IDC_EDIT_WINDSPEED);
	m_toolTip.AddTool(&m_plumeHeight,       IDC_EDIT_PLUMEHEIGHT);
	m_toolTip.AddTool(&m_labelPlumeHeight,	IDC_EDIT_PLUMEHEIGHT);
	m_toolTip.AddTool(&m_setButton,         IDC_BUTTON_SET_WINDFIELD);
	m_toolTip.AddTool(&m_todayPlot,          IDC_FLUX_PLOT);
	m_toolTip.AddTool(&m_infoSerial,        IDC_CONFINFO_SERIAL);
	m_toolTip.AddTool(&m_infoCompass,       IDC_CONFINFO_COMPASS);
	m_toolTip.AddTool(&m_infoLatitude,      IDC_CONFINFO_LATITUDE);
	m_toolTip.AddTool(&m_infoLongitude,     IDC_CONFINFO_LONGITUDE);
	m_toolTip.AddTool(&m_scanInfo_Starttime,IDC_SCANINFO_STARTTIME);
	m_toolTip.AddTool(&m_scanInfo_StopTime, IDC_SCANINFO_STOPTIME);
	m_toolTip.AddTool(&m_scanInfo_Date,     IDC_SCANINFO_DATE);
	m_toolTip.AddTool(&m_scanInfo_ExpTime,  IDC_SCANINFO_EXPTIME);
	m_toolTip.AddTool(&m_scanInfo_NumSpec,  IDC_SCANINFO_NSPEC);
	m_toolTip.AddTool(&m_statusLight,       IDC_STATUS_LIGHT);
	m_toolTip.AddTool(&m_commStatusLight,   IDC_COMMSTATUS);
	m_toolTip.AddTool(&m_statusEval,        IDC_STATUS_LIGHT);
	m_toolTip.AddTool(&m_statusScanner,     IDC_COMMSTATUS);

	m_toolTip.SetMaxTipWidth(INT_MAX);
	m_toolTip.Activate(TRUE);

	// Update the plot...
	if (g_settings.scanner[m_scannerIndex].plotColumn) {
		m_todayPlot.SetCircleRadius(1);
		DrawColumnDay();
	}
	else {
		m_todayPlot.SetLineWidth(2);
		DrawFlux();
	};

	// If the wind-field has been set automatically, then show it to the user
	CWindField windField;
	CDateTime	dt;
	if(0 == g_metData.GetWindField(m_serial, dt, windField)){
		this->windSpeed     = windField.GetWindSpeed();
		this->windDirection = windField.GetWindDirection();
		this->plumeHeight   = windField.GetPlumeHeight();
		UpdateData(FALSE);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CView_Scanner::SetTodayRange() {
	// set time range for last day plot
	struct tm *tm;
	time_t endtime;
	time(&endtime);
	tm = gmtime(&endtime);
	time_t starttime = endtime - 60 * 60 * 24;
	m_todayPlot.SetRangeX(starttime, endtime, 0);
}

void CView_Scanner::DrawColumn(){
	Common common;
	// Strings used for the saving of images
	CString sDate, sTime, imageFileName, lastImageFileName, directory, latestInfoFileName;

	const int BUFFER_SIZE = 1000;
	double time[BUFFER_SIZE], column[BUFFER_SIZE], columnError[BUFFER_SIZE], badColumn[BUFFER_SIZE], angle[BUFFER_SIZE], peakSat[BUFFER_SIZE], fitSat[BUFFER_SIZE];
	double maxAngle, minAngle, maxColumn, minColumn;

	// remove the old plot
	m_lastScanPlot.CleanPlot();

	// Set the unit of the plot
	if(g_userSettings.m_columnUnit == UNIT_PPMM)
		m_lastScanPlot.SetYUnits("Column [ppmm]");
	else if(g_userSettings.m_columnUnit == UNIT_MOLEC_CM2)
		m_lastScanPlot.SetYUnits("Column [molec/cm²]");

	// Get the data
	int dataLength = m_evalDataStorage->GetColumnData(m_serial, column, columnError, BUFFER_SIZE);
	dataLength     = min(dataLength, m_evalDataStorage->GetAngleData(m_serial, angle, BUFFER_SIZE));
	dataLength     = min(dataLength, m_evalDataStorage->GetIntensityData(m_serial, peakSat, fitSat, BUFFER_SIZE));

	// If there's no data then don't draw anything
	if(dataLength <= 0)
		return;

	// Get the ranges for the data
	m_evalDataStorage->GetAngleRange(m_serial, minAngle, maxAngle); 
	m_evalDataStorage->GetColumnRange(m_serial, minColumn,	maxColumn);

	// Copy the bad-columns to the 'badColumn' - array
	m_evalDataStorage->GetBadColumnData(m_serial, badColumn, BUFFER_SIZE);

	// Check if this is a fixed-angle or wind-measurement of if its a normal scan
	m_isTimeSeries  = fabs(maxAngle - minAngle) < 1e-2;
	int nRepetitions   = 0;
	for (int i = 2; i < dataLength; ++i) {
		if (angle[i] == angle[i - 2]) {
			++nRepetitions;
		}
	}
	if (nRepetitions > 5) {
		m_isTimeSeries = true;
	}

	// Check data ranges
	minColumn = min(minColumn, 0);
	if(fabs(maxColumn - minColumn) < 1E-2){
		minColumn -= 1;
		maxColumn += 1;
	}
	// set limits on the data ranges
	maxColumn = min(maxColumn, 1e22);
	minColumn = max(minColumn, -1e22);
	
	// get the offset
	double offset = m_evalDataStorage->GetOffset(m_serial);

	// and the plume centre
	double	plumeCentre = m_evalDataStorage->GetPlumeCentre(m_serial);

	// remove the offset
	for(int i = 0; i < dataLength; ++i){
		column[i] = column[i] - offset;
		if (fabs(badColumn[i]) > 1.0) {
			badColumn[i] -= offset;
		}
	}

	minColumn -= offset;
	maxColumn -= offset;
	minColumn = (minColumn < 0) ? (minColumn * 1.25) : (minColumn / 1.5);
	maxColumn = (maxColumn > 0) ? (maxColumn * 1.25) : (maxColumn / 1.5);

	// Convert the saturation ratios to percent
	for(int i = 0; i < dataLength; ++i){
		fitSat[i]  *= 100.0;
		peakSat[i] *= 100.0;
	}

	// Set the range for the graph
	if(m_isTimeSeries){
		// wind speed measurement?
		// Get the timeOfDay for each spectrum
		m_evalDataStorage->GetTimeData(m_serial, time, BUFFER_SIZE);

		m_lastScanPlot.SetRange(Min(time, dataLength), Max(time, dataLength), 1, minColumn, maxColumn, 0);
		m_lastScanPlot.SetXUnits(common.GetString(AXIS_TIMEOFDAY));
		m_lastScanPlot.SetXAxisNumberFormat(Graph::FORMAT_TIME);
	}else{
		// normal scan
		m_lastScanPlot.SetRange(minAngle, maxAngle, 1, minColumn, maxColumn, 0);
		m_lastScanPlot.SetXUnits(common.GetString(AXIS_ANGLE));
		m_lastScanPlot.SetXAxisNumberFormat(Graph::FORMAT_GENERAL);
	}

	// If we know the dynamic range for the spectrometer, 
	//	set the plot to show saturation-levels instead of intensities
	double maxIntensity = m_evalDataStorage->GetDynamicRange(m_serial);
	if(maxIntensity > 0){
		m_lastScanPlot.SetSecondRangeY(0, 100, 0, false);
		m_lastScanPlot.SetSecondYUnit(common.GetString(AXIS_SATURATION));
	}else{
		m_lastScanPlot.SetSecondRangeY(0, 4096, 0, false);
		m_lastScanPlot.SetSecondYUnit(common.GetString(AXIS_INTENSITY));
	}

	// draw the columns
	if(m_isTimeSeries){
		m_lastScanPlot.XYPlot(time, column, dataLength, CGraphCtrl::PLOT_FIXED_AXIS | CGraphCtrl::PLOT_CONNECTED);

		// draw the peak saturation
		m_lastScanPlot.SetCircleColor(RGB(255, 255, 255));
		m_lastScanPlot.DrawCircles(time, peakSat, dataLength, CGraphCtrl::PLOT_SECOND_AXIS);

		// draw the fit-saturation
		m_lastScanPlot.SetCircleColor(RGB(255, 255, 0));
		m_lastScanPlot.DrawCircles(time, fitSat, dataLength, CGraphCtrl::PLOT_SECOND_AXIS);
	}else{
		// All the column-values
		m_lastScanPlot.SetPlotColor(RGB(255, 0, 0));
		m_lastScanPlot.BarChart(angle, column, columnError, dataLength, CGraphCtrl::PLOT_FIXED_AXIS | CGraphCtrl::PLOT_BAR_SHOW_X);

		// The bad column-values
		m_lastScanPlot.SetPlotColor(RGB(128, 0, 0));
		m_lastScanPlot.BarChart(angle, badColumn, dataLength, CGraphCtrl::PLOT_FIXED_AXIS | CGraphCtrl::PLOT_BAR_SHOW_X);		

		// draw the peak saturation
		m_lastScanPlot.SetCircleColor(RGB(255, 255, 255));
		m_lastScanPlot.DrawCircles(angle, peakSat, dataLength, CGraphCtrl::PLOT_SECOND_AXIS);

		// draw the fit-saturation
		m_lastScanPlot.SetCircleColor(RGB(255, 255, 0));
		m_lastScanPlot.DrawCircles(angle, fitSat, dataLength, CGraphCtrl::PLOT_SECOND_AXIS);

		// Draw the plume-centre position (if known);
		if(plumeCentre > -200){
			m_lastScanPlot.DrawLine(Graph::VERTICAL, plumeCentre, RGB(255, 255, 0), Graph::STYLE_DASHED);
		}
	}

	// If the user wants to save the graph, then do so
	if(g_settings.webSettings.publish){
		sDate.Format("%02d%02d%02d", m_lastScanStart.year % 1000, m_lastScanStart.month,		m_lastScanStart.day);
		sTime.Format("%02d%02d", m_lastScanStart.hour, m_lastScanStart.minute);

		if(strlen(g_settings.webSettings.localDirectory) > 0){
			// The directory where to store the images
			directory.Format("%s\\%s", (LPCTSTR)g_settings.webSettings.localDirectory, (LPCTSTR)sDate);
			if(CreateDirectoryStructure(directory)){
				ShowMessage("Could not create storage-directory for images. No image stored.");
				return; // failed to create directory, no idea to try to continue...
			}

			// The name of the image-file
			imageFileName.Format("%s\\%s_%s_%s_%1d%s", 
				(LPCTSTR)directory, 
				(LPCTSTR)m_serial, 
				(LPCTSTR)sDate, 
				(LPCTSTR)sTime,
				m_lastScanChannel,
				(LPCTSTR)g_settings.webSettings.imageFormat);

			// Save the column-plot
			m_lastScanPlot.SaveGraph(imageFileName);

			// Also save a copy of the column-plot to an additional directory, if this is
			//		the latest file received
			if(m_lastScanStart == m_mostRecentScanStart){
				lastImageFileName.Format("%s\\%s\\", (LPCTSTR)g_settings.webSettings.localDirectory, (LPCTSTR)m_serial);
				if(CreateDirectoryStructure(lastImageFileName)){
					ShowMessage("Could not create storage-directory for images. No image stored.");
					return; // failed to create directory, no idea to try to continue...
				}
				lastImageFileName.AppendFormat("Image.jpg");
				m_lastScanPlot.SaveGraph(lastImageFileName);
			}

			// Make a small file which contains the name of the last file generated
			latestInfoFileName.Format("%s\\LastColumnGraph_%s.txt", (LPCTSTR)g_settings.webSettings.localDirectory, (LPCTSTR)m_serial);
			FILE *f = fopen(latestInfoFileName, "w");
			if(f != NULL){
				fprintf(f, "%s\n", (LPCTSTR)imageFileName);
				fclose(f);
			}

			// if the user wants to call an external script, then do so
			if(strlen(g_settings.externalSetting.imageScript) > 2){
				ExecuteScript_Image(imageFileName);
			}
		}
	}
}

/** For single direction measurements, do not plot flux.  Instead, plot columns for the day. */
void CView_Scanner::DrawColumnDay() {	
	Common common;

	// variables
	const int BUFFER_SIZE = 10000;
	double time[BUFFER_SIZE], column[BUFFER_SIZE], columnError[BUFFER_SIZE], badColumn[BUFFER_SIZE], peakSat[BUFFER_SIZE], fitSat[BUFFER_SIZE];
	double maxColumn, minColumn;

	// Get the data
	int dataLength = m_evalDataStorage->GetColumnData(m_serial, column, columnError, BUFFER_SIZE, true);
	dataLength = min(dataLength, m_evalDataStorage->GetIntensityData(m_serial, peakSat, fitSat, BUFFER_SIZE, true));

	// If there's no data then don't draw anything
	if (dataLength <= 0)
		return;

	// remove the old plot
	m_todayPlot.CleanPlot();

	// Set the unit of the plot
	if (g_userSettings.m_columnUnit == UNIT_PPMM)
		m_todayPlot.SetYUnits("Column [ppmm]");
	else if (g_userSettings.m_columnUnit == UNIT_MOLEC_CM2)
		m_todayPlot.SetYUnits("Column [molec/cm²]");

	// Get the ranges for the data
	m_evalDataStorage->GetColumnRange(m_serial, minColumn, maxColumn, true);

	// Check data ranges
	minColumn = min(minColumn, 0);
	if (fabs(maxColumn - minColumn) < 1E-2) {
		minColumn -= 1;
		maxColumn += 1;
	}
	// set limits on the data ranges
	maxColumn = min(maxColumn, 1e22);
	minColumn = max(minColumn, -1e22);

	// Copy the bad-columns to the 'badColumn' - array
	m_evalDataStorage->GetBadColumnData(m_serial, badColumn, BUFFER_SIZE, true);

	// get the offset
	double offset = m_evalDataStorage->GetOffset(m_serial);
	
	// remove the offset
	for (int i = 0; i < dataLength; ++i) {
		column[i] = column[i] - offset;
		if (fabs(badColumn[i]) > 1.0) {
			badColumn[i] -= offset;
		}
	}

	minColumn -= offset;
	maxColumn -= offset;
	minColumn = (minColumn < 0) ? (minColumn * 1.25) : (minColumn / 1.5);
	maxColumn = (maxColumn > 0) ? (maxColumn * 1.25) : (maxColumn / 1.5);

	// Convert the saturation ratios to percent
	for (int i = 0; i < dataLength; ++i) {
		fitSat[i] *= 100.0;
		peakSat[i] *= 100.0;
	}
	
	m_todayPlot.SetRangeY(minColumn, maxColumn, 0);
	SetTodayRange();

	// If we know the dynamic range for the spectrometer, 
	//	set the plot to show saturation-levels instead of intensities
	double maxIntensity = m_evalDataStorage->GetDynamicRange(m_serial);
	if (maxIntensity > 0) {
		m_todayPlot.SetSecondRangeY(0, 100, 0, false);
		m_todayPlot.SetSecondYUnit(common.GetString(AXIS_SATURATION));
	}
	else {
		m_todayPlot.SetSecondRangeY(0, 4096, 0, false);
		m_todayPlot.SetSecondYUnit(common.GetString(AXIS_INTENSITY));
	}

	m_evalDataStorage->GetTimeData(m_serial, time, BUFFER_SIZE, true);

	// draw the columns
	m_todayPlot.SetCircleColor(RGB(255, 0, 0));
	m_todayPlot.XYPlot(time, column, dataLength, CGraphCtrl::PLOT_FIXED_AXIS | CGraphCtrl::PLOT_CIRCLES);

	// draw the peak saturation
	m_todayPlot.SetCircleColor(RGB(255, 255, 255));
	m_todayPlot.DrawCircles(time, peakSat, dataLength, CGraphCtrl::PLOT_SECOND_AXIS);

	// draw the fit-saturation
	m_todayPlot.SetCircleColor(RGB(255, 255, 0));
	m_todayPlot.DrawCircles(time, fitSat, dataLength, CGraphCtrl::PLOT_SECOND_AXIS);

	// If the user wants to save the graph, then do so
	if (g_settings.webSettings.publish) {

		// Strings used for the saving of images
		CString sDate, sTime, imageFileName, lastImageFileName, directory, latestInfoFileName;

		sDate.Format("%02d%02d%02d", m_lastScanStart.year % 1000, m_lastScanStart.month, m_lastScanStart.day);

		if (strlen(g_settings.webSettings.localDirectory) > 0) {
			// The directory where to store the images
			directory.Format("%s\\%s", (LPCTSTR)g_settings.webSettings.localDirectory, (LPCTSTR)sDate);
			if (CreateDirectoryStructure(directory)) {
				ShowMessage("Could not create storage-directory for images. No image stored.");
				return; // failed to create directory, no idea to try to continue...
			}

			// The name of the image-file
			imageFileName.Format("%s\\%s_%s_%1d%s",
				(LPCTSTR)directory,
				(LPCTSTR)m_serial,
				(LPCTSTR)sDate,
				m_lastScanChannel,
				(LPCTSTR)g_settings.webSettings.imageFormat);

			// Save the column-plot
			m_todayPlot.SaveGraph(imageFileName);

			// Also save a copy of the column-plot to an additional directory, if this is
			//		the latest file received
			if (m_lastScanStart == m_mostRecentScanStart) {
				lastImageFileName.Format("%s\\%s\\", (LPCTSTR)g_settings.webSettings.localDirectory, (LPCTSTR)m_serial);
				if (CreateDirectoryStructure(lastImageFileName)) {
					ShowMessage("Could not create storage-directory for images. No image stored.");
					return; // failed to create directory, no idea to try to continue...
				}
				lastImageFileName.AppendFormat("Image.jpg");
				m_todayPlot.SaveGraph(lastImageFileName);
			}

			// Make a small file which contains the name of the last file generated
			latestInfoFileName.Format("%s\\DayColumnGraph_%s.txt", (LPCTSTR)g_settings.webSettings.localDirectory, (LPCTSTR)m_serial);
			FILE *f = fopen(latestInfoFileName, "w");
			if (f != NULL) {
				fprintf(f, "%s\n", (LPCTSTR)imageFileName);
				fclose(f);
			}

			// if the user wants to call an external script, then do so
			if (strlen(g_settings.externalSetting.imageScript) > 2) {
				ExecuteScript_Image(imageFileName);
			}
		}
	}
}

void CView_Scanner::DrawFlux(){
	double maxFlux;
	const int BUFFER_SIZE = 200;
	double allFluxes[BUFFER_SIZE];
	double tid[BUFFER_SIZE];
	int fluxOk[BUFFER_SIZE];
	double goodFluxes[BUFFER_SIZE];
	double badFluxes[BUFFER_SIZE];
	double goodTime[BUFFER_SIZE];
	double badTime[BUFFER_SIZE];
	int nGoodFluxes = 0;
	int nBadFluxes = 0;
	CString fluxAxisLabel;
	Common common;

	// Get the data
	int dataLength = m_evalDataStorage->GetFluxData(m_serial, tid, allFluxes, fluxOk, BUFFER_SIZE);

	// sort the data into good values and bad values
	for(int k = 0; k < dataLength; ++k){
		if(fluxOk[k]){
			goodFluxes[nGoodFluxes] = allFluxes[k];
			goodTime[nGoodFluxes]   = tid[k];
			++nGoodFluxes;
		}else{
			badFluxes[nBadFluxes]  = allFluxes[k];
			badTime[nBadFluxes]    = tid[k];
			++nBadFluxes;
		}
	}

	// remove the old plot
	m_todayPlot.CleanPlot();

	// Set the unit of the plot
	if(g_userSettings.m_fluxUnit == UNIT_TONDAY){
		fluxAxisLabel.Format("%s [ton/day]", (LPCTSTR)common.GetString(AXIS_FLUX));
	}else{
		fluxAxisLabel.Format("%s [kg/s]", (LPCTSTR)common.GetString(AXIS_FLUX));
	}
	m_todayPlot.SetYUnits(fluxAxisLabel);

	// If there's no data, don't draw anything
	if(dataLength == 0)
		return;

	// Get the ranges for the flux data
	maxFlux = Max(allFluxes, dataLength);

	// set the range for the plot
	m_todayPlot.SetRangeY(0, maxFlux, 1);
	SetTodayRange();

	// First draw the bad values, then the good ones
	m_todayPlot.SetCircleColor(RGB(150, 150, 150));
	m_todayPlot.XYPlot(badTime, badFluxes, nBadFluxes, CGraphCtrl::PLOT_FIXED_AXIS | CGraphCtrl::PLOT_CIRCLES);

	m_todayPlot.SetCircleColor(RGB(255, 255, 255));
	m_todayPlot.XYPlot(goodTime, goodFluxes, nGoodFluxes, CGraphCtrl::PLOT_FIXED_AXIS | CGraphCtrl::PLOT_CIRCLES);


	// If the user wants to save the graph, then do so
	if(g_settings.webSettings.publish){
		CString sDate;
		Common::GetDateText(sDate);
		if(strlen(g_settings.webSettings.localDirectory) > 0){
			CString fileName;
			fileName.Format("%s\\flux_%s_%s%s", 
				(LPCTSTR)g_settings.webSettings.localDirectory,
				(LPCTSTR)m_serial, 
				(LPCTSTR)sDate,
				(LPCTSTR)g_settings.webSettings.imageFormat);
			m_todayPlot.SaveGraph(fileName);
		}
	}
}

LRESULT CView_Scanner::OnEvaluatedScan(WPARAM wParam, LPARAM lParam){
	// Update the screen
	OnUpdateEvalStatus(wParam, lParam);
	OnUpdateWindParam(wParam, lParam);

	if(lParam != NULL){
		Evaluation::CScanResult *result = (Evaluation::CScanResult *)lParam;
		delete result;
	}	

	return 0;
}

LRESULT CView_Scanner::OnUpdateEvalStatus(WPARAM wParam, LPARAM lParam){
	CString txt;

	// The status for the evaluation
	SPECTROMETER_STATUS status;

	// The status for the communication
	COMMUNICATION_STATUS comm_status;

	// Get the status
	m_evalDataStorage->GetStatus(m_serial, status);
	comm_status = (COMMUNICATION_STATUS)(m_commDataStorage->GetStatus(m_serial));

	// update the status light
	switch(status){
		case STATUS_RED:          m_statusLight.SetBitmap((HBITMAP)m_redLight); break;
		case STATUS_GREEN:        m_statusLight.SetBitmap((HBITMAP)m_greenLight); break;
		case STATUS_YELLOW:       m_statusLight.SetBitmap((HBITMAP)m_yellowLight); break;
		default:                  m_statusLight.SetBitmap((HBITMAP)m_yellowLight); break;
	}
	switch(comm_status){
		case COMM_STATUS_RED:     m_commStatusLight.SetBitmap((HBITMAP)m_redLight); break;
		case COMM_STATUS_GREEN:   m_commStatusLight.SetBitmap((HBITMAP)m_greenLight); break;
		case COMM_STATUS_YELLOW:  m_commStatusLight.SetBitmap((HBITMAP)m_yellowLight); break;
		default:                  m_commStatusLight.SetBitmap((HBITMAP)m_yellowLight); break;
	}

	Common common;
	if(g_settings.scanner[m_scannerIndex].gps.m_latitude == 0){
		printf("ojsan");
	}
	// update the configuration info
	txt.Format("%s", (LPCTSTR)g_settings.scanner[m_scannerIndex].spec[0].serialNumber);
	this->SetDlgItemText(IDC_CONFINFO_SERIAL, txt);
	txt.Format("%.1lf [deg]", fmod(g_settings.scanner[m_scannerIndex].compass, 360.0f));
	this->SetDlgItemText(IDC_CONFINFO_COMPASS, txt);
	txt.Format("%.5lf", g_settings.scanner[m_scannerIndex].gps.m_latitude);
	this->SetDlgItemText(IDC_CONFINFO_LATITUDE, txt);
	txt.Format("%.5lf", g_settings.scanner[m_scannerIndex].gps.m_longitude);
	this->SetDlgItemText(IDC_CONFINFO_LONGITUDE, txt);
	if(fabs(g_settings.scanner[m_scannerIndex].coneAngle - 90.0) < 1e-2){
		// flat scanner
		txt.Format("%s", (LPCTSTR)common.GetString(STR_FLAT));
	}else{
		// Cone-scanner
		txt.Format("%s,%.0lf [deg]", (LPCTSTR)common.GetString(STR_CONE), g_settings.scanner[m_scannerIndex].coneAngle);
	}
	this->SetDlgItemText(IDC_CONFINFO_CONEORFLAT, txt);

	// Update the info about the last scan
	if(lParam != NULL){
		Evaluation::CScanResult *result = (Evaluation::CScanResult *)lParam;
		CDateTime startTime;
		result->GetSkyStartTime(startTime);
        CDateTime stopTime	= result->GetSpectrumInfo(result->GetEvaluatedNum()-1).m_stopTime;

		txt.Format("%02d:%02d:%02d", startTime.hour, startTime.minute, startTime.second);
		SetDlgItemText(IDC_SCANINFO_STARTTIME, txt);

		txt.Format("%02d:%02d:%02d", stopTime.hour, stopTime.minute, stopTime.second);
		SetDlgItemText(IDC_SCANINFO_STOPTIME, txt);

		txt.Format("%04d.%02d.%02d", startTime.year, startTime.month, startTime.day);
		SetDlgItemText(IDC_SCANINFO_DATE, txt);

		txt.Format("%d [ms]", result->GetSpectrumInfo(0).m_exposureTime);
		SetDlgItemText(IDC_SCANINFO_EXPTIME, txt);

		txt.Format("%d", result->GetSpectrumInfo(0).m_numSpec);
		SetDlgItemText(IDC_SCANINFO_NSPEC, txt);

		// Remember the time when the last shown scan started
		m_lastScanStart.year = startTime.year;
		m_lastScanStart.month = startTime.month;
		m_lastScanStart.day = startTime.day;
		m_lastScanStart.hour = startTime.hour;
		m_lastScanStart.minute= startTime.minute;
		m_lastScanStart.second= startTime.second;
		if(m_mostRecentScanStart < m_lastScanStart){
			m_mostRecentScanStart = m_lastScanStart;
		}

		// Also remember the channel that the last collected scan used
		m_lastScanChannel = result->GetSpectrumInfo(0).m_channel;
	}

	// update the last scan plot
	DrawColumn();

	// update today's plot
	if (g_settings.scanner[m_scannerIndex].plotColumn) {
		DrawColumnDay();
	}
	else {
		DrawFlux();
	}

	return 0;
}

LRESULT CView_Scanner::OnUpdateWindParam(WPARAM wParam, LPARAM lParam){
	CWindField wind;
	CDateTime dt;

	// Get the current time
	dt.SetToNow();
	dt.Increment(_timezone); // convert from local-time to GMT

	// Get the wind-field for this scanner...
	if(g_metData.GetWindField(m_serial, dt, wind)){
		// if the wind field could not be found, use the standard values
		wind = g_metData.defaultWindField;
	}

	// Update the screen
	plumeHeight   = wind.GetPlumeHeight();
	windDirection = wind.GetWindDirection();
	windSpeed     = wind.GetWindSpeed();
	UpdateData(FALSE);

	return 0;
}

void CView_Scanner::OnBnClickedButtonSetWindfield()
{
	CString message, curVolcano;
	unsigned int it;
	static time_t now = 0, lastTime = 0;

	// Get the user defined wind field
	UpdateData(TRUE);

	CWindField wf;
	wf.SetPlumeHeight(plumeHeight, MET_USER);
	wf.SetWindDirection(windDirection, MET_USER);
	wf.SetWindSpeed(windSpeed, MET_USER);

	// Set the wind field
	g_metData.SetWindField(m_serial, wf);

	// get the time now
	lastTime = now;
	time(&now);
	if(difftime(now, lastTime) < 5*60){
		// If the user has pressed this button two (or more) times
		//  within five minutes, then interpret this as he/she wanted
		//  to set the wind-field for all instruments on the first
		//  click and to set the wind-field for this instrument on
		//  the following clicks...
			
		// tell the user which wind field has been changed
		message.Format("Set windfield @ %s to: ", (LPCTSTR)m_siteName);

		// tell the user what the wind field is 
		message.AppendFormat("%.1lf m/s, %.1lf degrees, plume @ %.0lf m above the scanner", 
			wf.GetWindSpeed(), wf.GetWindDirection(), wf.GetPlumeHeight());

	}else{
		// It's quite some time since the user pressed this button
		//  last time. Set the wind-field for all instruments and not just for 
		//	the currently shown...
	
		// The wind-field should be the same for all instruments at this volcano, 
		//			find out which other instruments look at the same volcano and set the
		//			wind-field for them too.
		curVolcano.Format(g_settings.scanner[m_scannerIndex].volcano);
		for(it = 0; it < g_settings.scannerNum; ++it){
			if(it == m_scannerIndex)
				continue;
			if(Equals(curVolcano, g_settings.scanner[it].volcano))
				g_metData.SetWindField(g_settings.scanner[it].spec[0].serialNumber, wf);
		}

		// tell the user which wind field has been changed
		message.Format("Set windfield @ %s to: ", (LPCTSTR)curVolcano);

		// tell the user what the wind field is 
		message.AppendFormat("%.1lf m/s, %.1lf degrees, plume @ %.0lf m above the scanner", 
			wf.GetWindSpeed(), wf.GetWindDirection(), wf.GetPlumeHeight());
	}

	ShowMessage(message);
}


BOOL CView_Scanner::PreTranslateMessage(MSG* pMsg)
{
	m_toolTip.RelayEvent(pMsg);

	return CPropertyPage::PreTranslateMessage(pMsg);
}

BOOL CView_Scanner::OnSetActive()
{
	// Redraw the screen
	OnUpdateEvalStatus(NULL, NULL);
	OnUpdateWindParam(NULL, NULL);
	return CPropertyPage::OnSetActive();
}

void CView_Scanner::ExecuteScript_Image(const CString &imageFile){
	CString command, filePath, directory;

	// The file to execute
	filePath.Format("%s", (LPCTSTR)g_settings.externalSetting.imageScript);

	// The directory of the executable file
	directory.Format(filePath);
	Common::GetDirectory(directory);

	// The parameters to the script
	command.Format("%s", (LPCTSTR)imageFile);

	// Call the script
	int result = (int)ShellExecute(NULL, "open", filePath, command, directory, SW_SHOW);

	// Check the return-code if there was any error...
	if(result > 32)
		return;
	switch(result){
		case 0:	break;// out of memory
		case ERROR_FILE_NOT_FOUND:	MessageBox("Specified script-file not found");	break;	// file not found
		case ERROR_PATH_NOT_FOUND:	MessageBox("Specified script-file not found");	break;	// path not found
		case ERROR_BAD_FORMAT:		break;	// 
		case SE_ERR_ACCESSDENIED:break;
		case SE_ERR_ASSOCINCOMPLETE:break;
		case SE_ERR_DDEBUSY:break;
		case SE_ERR_DDEFAIL:break;
		case SE_ERR_DDETIMEOUT:break;
		case SE_ERR_DLLNOTFOUND:break;
		case SE_ERR_NOASSOC:break;
		case SE_ERR_OOM:break;
		case SE_ERR_SHARE:break;
	}
	// don't try again to access the file
	g_settings.externalSetting.imageScript.Format("");

}
void CView_Scanner::OnSize(UINT nType, int cx, int cy)
{
	CPropertyPage::OnSize(nType, cx, cy);

	this->SetLayout();
}

void DDX_Text_Formatted(CDataExchange* pDX, int nIDC, double& value, LPCTSTR lpszOutFormat, LPCTSTR lpszInFormat)
{
	CString temp;

	if (! pDX->m_bSaveAndValidate)
		temp.Format(lpszOutFormat,value);

	DDX_Text(pDX,nIDC,temp);

	if (pDX->m_bSaveAndValidate) {
		int ret = sscanf(temp, lpszInFormat, &value);
	}
}
