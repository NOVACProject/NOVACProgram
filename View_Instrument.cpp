// View_Instrument.cpp : implementation file
//

#include "stdafx.h"
#include "NovacMasterProgram.h"
#include "View_Instrument.h"

#include "Configuration/Configuration.h"

extern CConfigurationSetting g_settings;

using namespace Graph;

// CView_Instrument dialog

IMPLEMENT_DYNAMIC(CView_Instrument, CPropertyPage)
CView_Instrument::CView_Instrument()
	: CPropertyPage(CView_Instrument::IDD)
{
	m_evalDataStorage = NULL;
	m_commDataStorage = NULL;
}

CView_Instrument::~CView_Instrument()
{
	m_evalDataStorage = NULL;
	m_commDataStorage	= NULL;

	// Clear the labels
	POSITION pos = m_serialLegend.GetHeadPosition();
	while(pos != NULL){
		CLegend *leg = m_serialLegend.GetNext(pos);
		delete leg;
	}
}

void CView_Instrument::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CView_Instrument, CPropertyPage)
	// Messages from other parts of the program
	ON_MESSAGE(WM_EVAL_SUCCESS,  OnUpdateGraphs)
	ON_MESSAGE(WM_EVAL_FAILURE,  OnUpdateGraphs)
END_MESSAGE_MAP()


// CView_Instrument message handlers

BOOL CView_Instrument::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	CString serialString[50];
	CRect thisRect, batteryGraphRect, tempGraphRect, expTimeGraphRect, linkSpeedGraphRect;
	CRect labelRect, textRect;
	int margin = 4; // the margin between the graphs
	int labelWidth = 80; // the width of each label
	int	labelHeight = 15; // the height of each label
	int nSpectrometers = g_settings.scannerNum;
	COLORREF colors[] = {RGB(255, 0, 0), RGB(0, 255, 0), RGB(255, 255, 0), RGB(255, 0, 255), RGB(0, 255, 255), RGB(0, 0, 255)};
	int nColors = 6;

	// The size of the window.
	this->GetWindowRect(thisRect);

	// The legends are arranged in a matrix with up to 'nLegends' in each row
	//	and as many rows as are needed
	const int nLegendsPerRow = 8;
	int nRows = 1 + (int)(nSpectrometers / nLegendsPerRow);

	// The number of graps;
	int nGraphs = 4;

	// The height of each graph
	int graphHeight = (thisRect.Height() - nRows * labelHeight - margin) / nGraphs - margin;

	// Calculate the size of the graphs
	batteryGraphRect.left  = margin;
	batteryGraphRect.right = thisRect.Width() - 2 * margin;
	batteryGraphRect.top   = margin;
	batteryGraphRect.bottom = batteryGraphRect.top + graphHeight;

	tempGraphRect.left = margin;
	tempGraphRect.right = thisRect.Width() - 2 * margin;
	tempGraphRect.top = batteryGraphRect.bottom + margin;
	tempGraphRect.bottom = tempGraphRect.top + graphHeight;

	expTimeGraphRect.left     = margin;
	expTimeGraphRect.right    = thisRect.Width() - 2 * margin;
	expTimeGraphRect.top      = tempGraphRect.bottom + margin;
	expTimeGraphRect.bottom   = expTimeGraphRect.top + graphHeight;

	linkSpeedGraphRect.left   = margin;
	linkSpeedGraphRect.right  = thisRect.Width() - 2 * margin;
	linkSpeedGraphRect.top    = expTimeGraphRect.bottom + margin;
	linkSpeedGraphRect.bottom = linkSpeedGraphRect.top + graphHeight;

	// The font we want to use
	CFont *font = new CFont();
	font->CreateFont((int)(16 - 0.2*nSpectrometers), 0, 0, 0, FW_BOLD ,
                       FALSE, FALSE, 0, ANSI_CHARSET,
                       OUT_DEFAULT_PRECIS, 
                       CLIP_DEFAULT_PRECIS,
                       DEFAULT_QUALITY, 
                       DEFAULT_PITCH|FF_SWISS, "Arial") ;

	// Create the graphs
	// 1. The battery voltage graph
	m_batteryGraph.SetXUnits(m_common.GetString(AXIS_UTCTIME));
	m_batteryGraph.HideXScale();
	m_batteryGraph.Create(WS_VISIBLE | WS_CHILD, batteryGraphRect, this);
	m_batteryGraph.SetFontHeight((int)(14 - 0.3*nSpectrometers));
	m_batteryGraph.SetXAxisNumberFormat(FORMAT_DATETIME);
	m_batteryGraph.EnableGridLinesX(true);
	m_batteryGraph.SetYUnits("Battery [V]");
	m_batteryGraph.SetLineWidth(2);               // Increases the line width to 2 pixels
	m_batteryGraph.SetMinimumRangeY(1.0);
	m_batteryGraph.SetGridColor(RGB(255, 255, 255));
	m_batteryGraph.SetBackgroundColor(RGB(0, 0, 0));
	//m_batteryGraph.SetRange(0, 24*3600-1, 0, 0, 20, 0);
	m_batteryGraph.HideXScale();
	SetRange(m_batteryGraph, 20);

	// 2. The temperature graph
	m_temperatureGraph.SetXUnits(m_common.GetString(AXIS_UTCTIME));
	m_temperatureGraph.HideXScale();
	m_temperatureGraph.Create(WS_VISIBLE | WS_CHILD, tempGraphRect, this);
	m_temperatureGraph.SetFontHeight((int)(14 - 0.3*nSpectrometers));
	m_temperatureGraph.SetXAxisNumberFormat(FORMAT_DATETIME);
	m_temperatureGraph.EnableGridLinesX(true);
	m_temperatureGraph.SetYUnits("Temp [°C]");
	m_temperatureGraph.SetLineWidth(2);               // Increases the line width to 2 pixels
	m_temperatureGraph.SetMinimumRangeY(1.0);
	m_temperatureGraph.SetGridColor(RGB(255, 255, 255));
	m_temperatureGraph.SetBackgroundColor(RGB(0, 0, 0));
	//m_temperatureGraph.SetRange(0, 24*3600-1, 0, 0, 60, 0);
	SetRange(m_temperatureGraph, 60);

	// 3. The exposure time graph
	m_expTimeGraph.SetXUnits(m_common.GetString(AXIS_UTCTIME));
	m_expTimeGraph.HideXScale();
	m_expTimeGraph.Create(WS_VISIBLE | WS_CHILD, expTimeGraphRect, this);
	m_expTimeGraph.SetFontHeight((int)(14 - 0.3*nSpectrometers));
	m_expTimeGraph.SetXAxisNumberFormat(FORMAT_DATETIME);
	m_expTimeGraph.EnableGridLinesX(true);
	m_expTimeGraph.SetYUnits("Exp. time [ms]");
	m_expTimeGraph.SetLineWidth(2);               // Increases the line width to 2 pixels
	m_expTimeGraph.SetMinimumRangeY(1.0);
	m_expTimeGraph.SetGridColor(RGB(255, 255, 255));
	m_expTimeGraph.SetBackgroundColor(RGB(0, 0, 0));
	//m_expTimeGraph.SetRange(0, 24*3600-1, 0, 0, 1000, 0);
	SetRange(m_expTimeGraph, 1000);

	// 4. The data-link speed graph
	m_linkSpeedGraph.SetXUnits(m_common.GetString(AXIS_UTCTIME));
	m_linkSpeedGraph.Create(WS_VISIBLE | WS_CHILD, linkSpeedGraphRect, this);
	m_linkSpeedGraph.SetFontHeight((int)(14 - 0.3*nSpectrometers));
	m_linkSpeedGraph.SetXAxisNumberFormat(FORMAT_DATETIME);
	m_linkSpeedGraph.EnableGridLinesX(true);
	m_linkSpeedGraph.SetYUnits("Link [kb/s]");
	m_linkSpeedGraph.SetLineWidth(2);               // Increases the line width to 2 pixels
	m_linkSpeedGraph.SetMinimumRangeY(1.0);
	m_linkSpeedGraph.SetGridColor(RGB(255, 255, 255));
	m_linkSpeedGraph.SetBackgroundColor(RGB(0, 0, 0));
	//m_linkSpeedGraph.SetRange(0, 24*3600-1, 0, 0, 1000, 0);
	SetRange(m_linkSpeedGraph, 1000);

	// if there are no spectrometers then we don't have anything more to do
	if(nSpectrometers == 0)
		return TRUE;

	// Create the legends
	for(unsigned int index = 0; index < g_settings.scannerNum; ++index){
		// the size and position of the colored square
		labelRect.top    = linkSpeedGraphRect.bottom + margin / 2;
		labelRect.bottom = labelRect.top + labelHeight;
		labelRect.left   = linkSpeedGraphRect.left + (index % nLegendsPerRow) * (labelWidth + labelHeight);
		labelRect.right  = labelRect.left + labelHeight;

		// The size and position of the text label containing the serial-number
		textRect.top    = labelRect.top;
		textRect.bottom = labelRect.bottom;
		textRect.left   = labelRect.right + 2;
		textRect.right  = textRect.left + labelWidth;

		// Create it all...
		CLegend *legend = new CLegend();

		legend->m_color = colors[index % nColors];

		legend->m_serial.Format(g_settings.scanner[index].spec[0].serialNumber);
		legend->m_label.Create("", WS_VISIBLE | WS_CHILD | WS_BORDER, labelRect, this);
		legend->m_label.SetBackgroundColor(legend->m_color);
		legend->m_text.Create(legend->m_serial, WS_VISIBLE | WS_CHILD, textRect, this);
		legend->m_text.SetFont(font);

		m_serialLegend.AddTail(legend);
	}

	// Enable the tool tips
	if(!m_toolTip.Create(this)){
		TRACE0("Failed to create tooltip control\n"); 
	}
	m_toolTip.AddTool(&m_batteryGraph, "Variation in battery voltage of the connected instruments today");
	m_toolTip.AddTool(&m_temperatureGraph, "Variation in temperature in the connected instruments today");
	m_toolTip.AddTool(&m_expTimeGraph, "Variation in exposure-times of the connected instruments today");
	m_toolTip.AddTool(&m_linkSpeedGraph, "The speed of the downloads done today");
	m_toolTip.SetMaxTipWidth(INT_MAX);
	m_toolTip.Activate(TRUE);

	return TRUE;  // return TRUE unless you set the focus to a control
}

BOOL CView_Instrument::OnSetActive()
{
	// Redraw the screen
	DrawGraphs();

	return CPropertyPage::OnSetActive();
}

BOOL CView_Instrument::PreTranslateMessage(MSG* pMsg){
	m_toolTip.RelayEvent(pMsg);

	return CPropertyPage::PreTranslateMessage(pMsg);
}

LRESULT CView_Instrument::OnUpdateGraphs(WPARAM wParam, LPARAM lParam){
	// Re-draw the screen
	DrawGraphs();

	return 0;
}

void CView_Instrument::DrawGraphs(){
	DrawTempGraph();
	DrawBatteryGraph();
	DrawExpTimeGraph();
	DrawLinkSpeedGraph();
}

/** Draws the temperature graph */
void CView_Instrument::DrawTempGraph(){
	const int BUFFER_SIZE = 200;
	double timeBuffer[BUFFER_SIZE];
	double tempBuffer[BUFFER_SIZE];
	CString serialString;

	// Clear the plot
	m_temperatureGraph.CleanPlot();

	SetRange(m_temperatureGraph, 60);
	// loop through all double-spectrometers
	if(m_serialLegend.GetCount() == 0)
		return;

	POSITION pos = m_serialLegend.GetHeadPosition();
	while(pos != NULL){
		CLegend *legend = m_serialLegend.GetNext(pos);

		// get the serial of the spectrometer
		CString &serial = legend->m_serial;

		// Get the data
		int dataLength = m_evalDataStorage->GetTemperatureData(serial, timeBuffer, tempBuffer, BUFFER_SIZE);

		// ---------- Plot the data ------------

		// Draw the wind-measurements as function of time
		if(0 != dataLength){
			m_temperatureGraph.SetCircleColor(legend->m_color);
			m_temperatureGraph.XYPlot(timeBuffer, tempBuffer, dataLength, CGraphCtrl::PLOT_CIRCLES | CGraphCtrl::PLOT_FIXED_AXIS);
		}
	}
}

/** Draws the battery-voltage graph */
void CView_Instrument::DrawBatteryGraph(){
	const int BUFFER_SIZE = 200;
	double timeBuffer[BUFFER_SIZE];
	double batteryBuffer[BUFFER_SIZE];
	CString serialString;

	// Clear the plot
	m_batteryGraph.CleanPlot();
	SetRange(m_batteryGraph, 20);

	// loop through all double-spectrometers
	POSITION pos = m_serialLegend.GetHeadPosition();
	while(pos != NULL){
		CLegend *legend = m_serialLegend.GetNext(pos);

		// get the serial of the spectrometer
		CString &serial = legend->m_serial;

		// Get the data
		int dataLength = m_evalDataStorage->GetBatteryVoltageData(serial, timeBuffer, batteryBuffer, BUFFER_SIZE);

		// ---------- Plot the data ------------

		// Draw the wind-measurements as function of time
		if(0 != dataLength){
			m_batteryGraph.SetCircleColor(legend->m_color);
			m_batteryGraph.XYPlot(timeBuffer, batteryBuffer, dataLength, CGraphCtrl::PLOT_FIXED_AXIS | CGraphCtrl::PLOT_CIRCLES);
		}
	}

}

/** Draws the exposure-time graph */
void CView_Instrument::DrawExpTimeGraph(){
	const int BUFFER_SIZE = 200;
	double timeBuffer[BUFFER_SIZE];
	double expTimeBuffer[BUFFER_SIZE];
	CString serialString;

	// Clear the plot
	m_expTimeGraph.CleanPlot();
	SetRange(m_expTimeGraph, 1000);

	// loop through all double-spectrometers
	POSITION pos = m_serialLegend.GetHeadPosition();
	while(pos != NULL){
		CLegend *legend = m_serialLegend.GetNext(pos);

		// get the serial of the spectrometer
		CString &serial = legend->m_serial;

		// Get the data
		int dataLength = m_evalDataStorage->GetExposureTimeData(serial, timeBuffer, expTimeBuffer, BUFFER_SIZE);

		// ---------- Plot the data ------------

		// Draw the wind-measurements as function of time
		if(0 != dataLength){
			m_expTimeGraph.SetCircleColor(legend->m_color);
			m_expTimeGraph.XYPlot(timeBuffer, expTimeBuffer, dataLength, CGraphCtrl::PLOT_FIXED_AXIS | CGraphCtrl::PLOT_CIRCLES);
		}
	}
}

/** Draws the link-speed graph */
void CView_Instrument::DrawLinkSpeedGraph(){
	const int BUFFER_SIZE = 1024;
	double *timeBuffer = new double[BUFFER_SIZE];
	double *linkSpeed =  new double[BUFFER_SIZE];
	double divisor = 1.0;
	CString serialString;
	int dataLength;

	// Clear the plot
	m_linkSpeedGraph.CleanPlot();

	// First find the range for the graph
	double maxDataRate = 1.0; //kb/s
	POSITION pos = m_serialLegend.GetHeadPosition();
	while(pos != NULL){
		CLegend *legend = m_serialLegend.GetNext(pos);

		// get the serial of the spectrometer
		CString &serial = legend->m_serial;

		// Get the data
		dataLength = this->m_commDataStorage->GetLinkSpeedData(serial, timeBuffer, linkSpeed, BUFFER_SIZE);

		maxDataRate = max(maxDataRate, Max(linkSpeed, dataLength));
	}

	maxDataRate = 2 * ceil(maxDataRate / 2); // to make the max-rate a factor of two...

	// decide on a unit...
	if(maxDataRate > 1024){
		// Mb/s
		divisor = 1024.0;
		m_linkSpeedGraph.SetYUnits("Link [MB/s]");
	}else{
		m_linkSpeedGraph.SetYUnits("Link [kB/s]");
	}
	maxDataRate /= divisor;
	//m_linkSpeedGraph.SetRangeY(0, maxDataRate, 0);
	SetRange(m_linkSpeedGraph, maxDataRate);

	// loop through all spectrometers
	pos = m_serialLegend.GetHeadPosition();
	while(pos != NULL){
		CLegend *legend = m_serialLegend.GetNext(pos);

		// get the serial of the spectrometer
		CString &serial = legend->m_serial;

		// Get the data
		int dataLength = this->m_commDataStorage->GetLinkSpeedData(serial, timeBuffer, linkSpeed, BUFFER_SIZE);

		// Shold we scale to MB/s?
		if(divisor > 1.5){
			for(int k = 0; k < dataLength; ++k){
				linkSpeed[k] /= divisor;
			}
		}
		// ---------- Plot the data ------------

		// Draw the wind-measurements as function of time
		if(0 != dataLength){
			m_linkSpeedGraph.SetCircleColor(legend->m_color);
			m_linkSpeedGraph.XYPlot(timeBuffer, linkSpeed, dataLength, CGraphCtrl::PLOT_FIXED_AXIS | CGraphCtrl::PLOT_CIRCLES);
		}
	}

	delete[] timeBuffer;
	delete[] linkSpeed;
}

void CView_Instrument::SetRange(Graph::CGraphCtrl &graph, int maxy) {
	struct tm *tm;
	time_t t;
	time(&t);
	tm = gmtime(&t);
	time_t endtime = t;
	time_t starttime = endtime - 86400;
	graph.SetRange(starttime, endtime, 0, 0, maxy, 0);
}