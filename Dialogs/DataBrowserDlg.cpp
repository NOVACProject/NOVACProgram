#include "stdafx.h"
#include "../NovacMasterProgram.h"
#include "../VolcanoInfo.h"
#include "DataBrowserDlg.h"
#include "../UserSettings.h"

extern CVolcanoInfo g_volcanoes;           // <-- The global database of volcanoes
extern CUserSettings g_userSettings;       // <-- The users preferences

using namespace DlgControls;
using namespace Graph;
using namespace Dialogs;

// CDataBrowserDlg dialog

IMPLEMENT_DYNAMIC(CDataBrowserDlg, CDialog)
CDataBrowserDlg::CDataBrowserDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDataBrowserDlg::IDD, pParent)
{
	m_calculator = NULL;
	m_volcanoIndex = -1;
	
	// setting the colors
	m_color.column					= RGB(255, 0, 0);
	m_color.badColumn				= RGB(128, 0, 0);
	m_color.delta						= RGB(0, 0, 255);
	m_color.chiSquare				= RGB(255, 0, 255);
	m_color.peakIntensity		= RGB(255, 255, 255);
	m_color.fitIntensity		= RGB(255, 255, 0);
	m_color.offset					= RGB(255,128,0);

	// the options for what to show
	m_show.columnError		= true;
	m_show.delta					= false;
	m_show.chiSquare			= false;
	m_show.peakIntensity	= true;
	m_show.fitIntensity		= true;

}

CDataBrowserDlg::~CDataBrowserDlg()
{
	if(m_calculator != NULL){
		delete m_calculator;
		m_calculator = NULL;
	}
}

void CDataBrowserDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	// The Frames
	DDX_Control(pDX, IDC_SCANGRAPH_FRAME, m_scanGraphFrame);
	DDX_Control(pDX, IDC_FLUXGRAPH_FRAME, m_fluxGraphFrame);
	DDX_Control(pDX, IDC_SCANINFO_FRAME,	m_scanInfoFrame);

	DDX_Control(pDX, IDC_SCAN_SPINBTN,		m_scanSpinCtrl);

	// The explanation to what is shown in the plot
	DDX_Control(pDX, IDC_LEGEND_COLUMN,							m_legendColumn);
	DDX_Control(pDX, IDC_LEGEND_PEAKINTENSITY,			m_legendPeakIntensity);
	DDX_Control(pDX, IDC_LEGEND_FITINTENSITY,				m_legendFitIntensity);
	DDX_Control(pDX, IDC_LEGEND_DELTA,							m_legendDelta);
	DDX_Control(pDX, IDC_LEGEND_CHISQUARE,					m_legendChiSquare);
	DDX_Control(pDX, IDC_STATIC_COLUMN,							m_labelColumn);
	DDX_Control(pDX, IDC_STATIC_PEAKINTENSITY,			m_labelPeakIntensity);
	DDX_Control(pDX, IDC_STATIC_FITINTENSITY,				m_labelFitIntensity);
	DDX_Control(pDX, IDC_STATIC_DELTA,							m_labelDelta);
	DDX_Control(pDX, IDC_STATIC_CHISQUARE,					m_labelChiSquare);
}


BEGIN_MESSAGE_MAP(CDataBrowserDlg, CDialog)
	// Open a novel evaluation-log
	ON_COMMAND(ID_FILE_OPENEVALUATIONLOG,		OnBrowseEvallog)

	// Changing the scan to show
	ON_NOTIFY(UDN_DELTAPOS, IDC_SCAN_SPINBTN, OnChangeSelectedScan)
	ON_COMMAND(ID_VIEW_PREVIOUSSCAN,					OnShowPreviousScan)
	ON_COMMAND(ID_VIEW_NEXTSCAN,							OnShowNextScan)

	// Change the options for what to show in the column-plot
	ON_COMMAND(ID_VIEW_PEAKINTENSITY_BD,		OnViewPeakIntensity)
	ON_COMMAND(ID_VIEW_FITINTENSITY_BD,		OnViewFitIntensity)
	ON_COMMAND(ID_VIEW_COLUMNERROR_BD,			OnViewColumnError)
	ON_COMMAND(ID_VIEW_DELTA_BD,						OnViewDelta)
	ON_COMMAND(ID_VIEW_CHISQUARE_BD,				OnViewChiSquare)

	// Updating the interface
	ON_UPDATE_COMMAND_UI(ID_VIEW_PEAKINTENSITY_BD, OnUpdateViewPeakintensity)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FITINTENSITY_BD, OnUpdateViewFitintensity)
	ON_UPDATE_COMMAND_UI(ID_VIEW_COLUMNERROR_BD, OnUpdateViewColumnError)
	ON_UPDATE_COMMAND_UI(ID_VIEW_DELTA_BD, OnUpdateViewDelta)
	ON_UPDATE_COMMAND_UI(ID_VIEW_CHISQUARE_BD, OnUpdateViewChiSquare)

	// Updating the interface, notice that this has to be here due to a bug in Microsoft MFC
	//	http://support.microsoft.com/kb/242577
	ON_WM_INITMENUPOPUP()

END_MESSAGE_MAP()


// CDataBrowserDlg message handlers

BOOL CDataBrowserDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CRect rect;
	int height, width;
	Common common;
	CString columnAxisLabel, fluxAxisLabel;

	// Initialize the unit to use
	if(g_userSettings.m_columnUnit == UNIT_MOLEC_CM2){
		columnAxisLabel.Format("%s [molec/cm²]", (LPCSTR)common.GetString(AXIS_COLUMN));
	}else{
		columnAxisLabel.Format("%s [ppmm]", (LPCSTR)common.GetString(AXIS_COLUMN));
	}
	
	if(g_userSettings.m_fluxUnit == UNIT_KGS){
		fluxAxisLabel.Format("%s [kg/s]", (LPCSTR)common.GetString(AXIS_FLUX));
	}else{
		fluxAxisLabel.Format("%s [ton/day]", (LPCSTR)common.GetString(AXIS_FLUX));
	}

	// Initialize the scan graph
	this->m_scanGraphFrame.GetWindowRect(rect);
	height = rect.bottom - rect.top;
	width  = rect.right - rect.left;
	rect.top = 20; rect.bottom = height - 10;
	rect.left = 10; rect.right = width - 10;  
	m_scanGraph.Create(WS_VISIBLE | WS_CHILD, rect, &m_scanGraphFrame);
	m_scanGraph.SetSecondRangeY(0, 4096, 0, false);
	m_scanGraph.SetSecondYUnit(common.GetString(AXIS_INTENSITY));
	m_scanGraph.SetXUnits(common.GetString(AXIS_ANGLE));
	m_scanGraph.SetYUnits(columnAxisLabel);
	m_scanGraph.SetBackgroundColor(RGB(0, 0, 0));
	m_scanGraph.SetPlotColor(RGB(255, 0, 0));
	m_scanGraph.SetGridColor(RGB(255, 255, 255));
	m_scanGraph.SetRange(-90, 90, 0, 0, 100, 0);

	  // Initialize the Flux graph
	this->m_fluxGraphFrame.GetWindowRect(rect);
	height = rect.bottom - rect.top;
	width  = rect.right - rect.left;
	rect.top = 20; rect.bottom = height - 10;
	rect.left = 10; rect.right = width - 10;  
	m_fluxGraph.Create(WS_VISIBLE | WS_CHILD, rect, &m_fluxGraphFrame);
	m_fluxGraph.SetXUnits(common.GetString(AXIS_LOCALTIME));
	m_fluxGraph.SetXAxisNumberFormat(Graph::FORMAT_TIME);
	m_fluxGraph.SetYUnits(fluxAxisLabel);
	m_fluxGraph.SetBackgroundColor(RGB(0, 0, 0));
	m_fluxGraph.SetPlotColor(RGB(255, 255, 255));
	m_fluxGraph.SetGridColor(RGB(255, 255, 255));
	m_fluxGraph.SetRange(0, 24*3600-1, 0, 0, 100, 0);
	m_fluxGraph.SetLineWidth(2);               // Increases the line width to 2 pixels
	m_fluxGraph.SetMinimumRangeY(1.0);

	// Initialize the legend
	InitLegend();

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CDataBrowserDlg::InitLegend(){
	int show;

	// The legends for the plot
	m_legendColumn.ShowWindow(SW_SHOW);
	m_legendColumn.SetBackgroundColor(m_color.column);
	m_legendColumn.SetWindowText("");
	m_labelColumn.ShowWindow(SW_SHOW);

	show = (m_show.delta) ? SW_SHOW: SW_HIDE;
	m_legendDelta.ShowWindow(show);
	m_legendDelta.SetBackgroundColor(m_color.delta);
	m_legendDelta.SetWindowText("");
	m_labelDelta.ShowWindow(show);

	show = (m_show.chiSquare) ? SW_SHOW: SW_HIDE;
	m_legendChiSquare.ShowWindow(show);
	m_legendChiSquare.SetBackgroundColor(m_color.chiSquare);
	m_legendChiSquare.SetWindowText("");
	m_labelChiSquare.ShowWindow(show);

	show = (m_show.peakIntensity) ? SW_SHOW: SW_HIDE;
	m_legendPeakIntensity.ShowWindow(show);
	m_legendPeakIntensity.SetBackgroundColor(m_color.peakIntensity);
	m_legendPeakIntensity.SetWindowText("");
	m_labelPeakIntensity.ShowWindow(show);

	show = (m_show.fitIntensity) ? SW_SHOW: SW_HIDE;
	m_legendFitIntensity.ShowWindow(show);
	m_legendFitIntensity.SetBackgroundColor(m_color.fitIntensity);
	m_legendFitIntensity.SetWindowText("");
	m_labelFitIntensity.ShowWindow(show);

	// ------ Initialize the scan-information list ----------
	CRect rect;
	this->m_scanInfoFrame.GetWindowRect(rect);
	int height = rect.bottom - rect.top;
	int width  = rect.right - rect.left;
	rect.top = 20; rect.bottom = height - 10;
	rect.left = 10; rect.right = width - 10;  
	int columnWidth = (int)(0.5*(rect.right - rect.left));
	m_scanInfoList.Create(WS_VISIBLE|WS_BORDER|LVS_REPORT, rect, &m_scanInfoFrame, 65536);
	m_scanInfoList.InsertColumn(0, "", LVCFMT_LEFT, columnWidth);
	m_scanInfoList.InsertColumn(1, "", LVCFMT_LEFT, columnWidth);

	Common common;
	int index = 0;
	m_scanInfoList.InsertItem(index++, "Scan number:");
	m_scanInfoList.InsertItem(index++, "Scans in File:");
	m_scanInfoList.InsertItem(index++, "Date (UTC):");
	m_scanInfoList.InsertItem(index++, common.GetString(STR_STARTTIME) + " (UTC):");
	m_scanInfoList.InsertItem(index++, common.GetString(STR_STOPTIME) + " (UTC):");
	m_scanInfoList.InsertItem(index++, "Hours to GMT:");
	m_scanInfoList.InsertItem(index++, "Volcano:");
	m_scanInfoList.InsertItem(index++, common.GetString(STR_SITE) + ":");
	m_scanInfoList.InsertItem(index++, common.GetString(AXIS_LATITUDE) + ":");
	m_scanInfoList.InsertItem(index++, common.GetString(AXIS_LONGITUDE) + ":");
	m_scanInfoList.InsertItem(index++, "Altitude (masl):");
	m_scanInfoList.InsertItem(index++, "Compass [deg]:");
	m_scanInfoList.InsertItem(index++, "Spec. Serial number:");
	m_scanInfoList.InsertItem(index++, "Scanner:");
	m_scanInfoList.InsertItem(index++, common.GetString(STR_EXPTIME) + " [ms]:");
	m_scanInfoList.InsertItem(index++, common.GetString(STR_NUMSPECTRA) + ":");
	m_scanInfoList.InsertItem(index++, "Flux [kg/s]:");
	m_scanInfoList.InsertItem(index++, "Temp. [°C]:");
	m_scanInfoList.InsertItem(index++, "Voltage [V]:");
}

/** Called when the user wants to browse for, and open an evaluation log */
void CDataBrowserDlg::OnBrowseEvallog(){
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
		if(m_calculator != NULL)
		delete m_calculator;
		m_calculator = new PostFlux::CPostFluxCalculator();
		m_curScan = 0;
		m_curSpecie = 0;
		m_volcanoIndex = -1;

		// set the path
		m_calculator->m_evaluationLog.Format("%s", (LPCSTR)evLog);

		// Read the evaluation log
		m_calculator->ReadEvaluationLog();

		// If there are no scans in the log-file, then return at once
		if(m_calculator->m_scanNum <= 0){
			MessageBox("There are no scans in the given log-file");
			delete m_calculator;
			m_calculator = NULL;
			return;
		}

		// Find the volcano that we're observing
		CString volcanoName = CString(m_calculator->m_scan[0].GetSpectrumInfo(0).m_volcano);
		for(unsigned int k = 0; k < g_volcanoes.m_volcanoNum; ++k){
			if(Equals(volcanoName, g_volcanoes.m_name[k])){
				m_volcanoIndex = k;
				break;
			}
		}

		// Update the screen
		this->SetDlgItemText(IDC_EVALUATION_LOG_EDIT, m_calculator->m_evaluationLog);
		InitializeControls();
		DrawScan();
		DrawFlux();
		UpdateScanInfo();
	}else{

	}
}

/** Draws the currently selected scan */
void CDataBrowserDlg::DrawScan(){
	CString message, columnAxisLabel;
	Common common;
	static double angle[MAX_SPEC_PER_SCAN];
	static double column[MAX_SPEC_PER_SCAN];
	static double badColumn[MAX_SPEC_PER_SCAN];
	static double badColumnErr[MAX_SPEC_PER_SCAN];
	static bool		badEvaluation[MAX_SPEC_PER_SCAN];
	static double columnError[MAX_SPEC_PER_SCAN];
	static double peakIntensity[MAX_SPEC_PER_SCAN];
	static double fitIntensity[MAX_SPEC_PER_SCAN];
	static double delta[MAX_SPEC_PER_SCAN];
	static double chiSquare[MAX_SPEC_PER_SCAN];
	static double selectedAngles[MAX_SPEC_PER_SCAN];
	static double selectedColumns[MAX_SPEC_PER_SCAN];

	// If no ev.log has been opened yet.
	if(m_calculator == NULL)
		return;

	// remove the old plot
	m_scanGraph.CleanPlot();

	// Set the unit of the plot
	double columnUnitConversionFactor = 1.0;
	if(g_userSettings.m_columnUnit == UNIT_MOLEC_CM2){
		columnAxisLabel.Format("%s [molec/cm²]", (LPCSTR)common.GetString(AXIS_COLUMN));
		columnUnitConversionFactor = 2.5e15;
	}else{
		columnAxisLabel.Format("%s [ppmm]", (LPCSTR)common.GetString(AXIS_COLUMN));
	}
	m_scanGraph.SetYUnits(columnAxisLabel);

	// A handle to the current scan
	Evaluation::CScanResult &scan = m_calculator->m_scan[m_curScan];

	// The number of spectra in this scan
	int numSpec = min(scan.GetEvaluatedNum(), MAX_SPEC_PER_SCAN);

	// Calculate the offset
	double m_userOffset = m_calculator->CalculateOffset(m_curScan, m_calculator->m_specie[m_curSpecie]);
	
	// Copy the data to a local buffer
	for(int k = 0; k < numSpec; ++k){
		angle[k]        = scan.GetScanAngle(k);

		peakIntensity[k]	= scan.GetPeakIntensity(k);
		fitIntensity[k]		= scan.GetFitIntensity(k);
		delta[k]			= scan.GetDelta(k);
		chiSquare[k]		= scan.GetChiSquare(k);
		if(scan.IsOk(k) && !scan.IsDeleted(k)){
		column[k]			= columnUnitConversionFactor * scan.GetColumn(k, m_curSpecie) - m_userOffset;
			columnError[k]	= columnUnitConversionFactor * scan.GetColumnError(k, m_curSpecie);
			badColumn[k]	= 0.0;
			badColumnErr[k]	= 0.0;
			badEvaluation[k]= false;
		}else{
		column[k]			= 0.0;
			columnError[k]	= 0.0;
			badColumn[k]	= columnUnitConversionFactor * scan.GetColumn(k, m_curSpecie) - m_userOffset;
			badColumnErr[k]	= columnUnitConversionFactor * scan.GetColumnError(k, m_curSpecie);
			badEvaluation[k]= true;
		}
	}

	// Get the ranges for the intensites, normalize if necessary
	double maxPeakIntensity = Max(peakIntensity, numSpec);
	int divisor = 1;
	while(maxPeakIntensity > 4096){
		maxPeakIntensity /= 2.0;
		divisor = divisor << 1;
	}
	if(divisor > 1){
		for(int k = 0; k < numSpec; ++k){
			peakIntensity[k] /= (double)divisor;
		}
	}

	// If the intensities are given as saturation ratio, change the right-axis and legend
	if(maxPeakIntensity < 1.01){
		m_scanGraph.SetSecondRangeY(0, 100, 0, false);
		m_scanGraph.SetSecondYUnit("Saturation [%]");
		SetDlgItemText(IDC_STATIC_PEAKINTENSITY, "Peak Saturation Ratio");
		SetDlgItemText(IDC_STATIC_FITINTENSITY, "Fit Saturation Ratio");
		for(int k = 0; k < numSpec; ++k){
			peakIntensity[k] *= 100.0;
			fitIntensity[k]  *= 100.0;
		}
	}else{
		m_scanGraph.SetSecondRangeY(0, 4096, 0, false);
		m_scanGraph.SetSecondYUnit(common.GetString(AXIS_INTENSITY));
		SetDlgItemText(IDC_STATIC_PEAKINTENSITY, "Peak Intensity");
		SetDlgItemText(IDC_STATIC_FITINTENSITY, "Fit Intensity");
	}

	// Get the ranges for the data
	double minColumn = Min(column, numSpec);
	double maxColumn = Max(column, numSpec);
	double minAngle  = Min(angle,  numSpec);
	double maxAngle  = Max(angle,  numSpec);
	if(minColumn == maxColumn)
		maxColumn += 1;
	if(minAngle == maxAngle)
		maxAngle += 1;

	// Set the properties for the plot
	m_scanGraph.SetRange(minAngle, maxAngle, 0, minColumn, maxColumn, 1);

	// draw the columns
	this->m_scanGraph.SetPlotColor(m_color.column);
	if(m_show.columnError){
		m_scanGraph.BarChart(angle, column, numSpec, Graph::CGraphCtrl::PLOT_FIXED_AXIS);
	}else{
		m_scanGraph.BarChart(angle, column, columnError, numSpec, Graph::CGraphCtrl::PLOT_FIXED_AXIS);
	}

	// draw the bad columns
	m_scanGraph.SetPlotColor(m_color.badColumn);
	if(m_show.columnError){
		m_scanGraph.BarChart(angle, badColumn, badColumnErr, numSpec, Graph::CGraphCtrl::PLOT_FIXED_AXIS);
	}else{
		m_scanGraph.BarChart(angle, badColumn, numSpec, Graph::CGraphCtrl::PLOT_FIXED_AXIS);
	}

	// draw the intensity
	if(m_show.peakIntensity){
		m_scanGraph.SetCircleColor(m_color.peakIntensity);
		m_scanGraph.DrawCircles(angle, peakIntensity, numSpec, Graph::CGraphCtrl::PLOT_SECOND_AXIS);
	}

	// draw the fit-intensity
	if(m_show.fitIntensity){
		m_scanGraph.SetCircleColor(m_color.fitIntensity);
		m_scanGraph.DrawCircles(angle, fitIntensity, numSpec, Graph::CGraphCtrl::PLOT_SECOND_AXIS);
	}

	// draw the delta of the residual
	if(m_show.delta){
		m_scanGraph.SetCircleColor(m_color.delta);
		double maxDelta = Max(delta, numSpec);
		for(int i = 0; i < numSpec; ++i)
			delta[i] *= (4096.0f / (float)maxDelta);
		m_scanGraph.DrawCircles(angle, delta, numSpec, Graph::CGraphCtrl::PLOT_SECOND_AXIS);
	}

	// draw the chi-square of the fit
	if(m_show.chiSquare){
		m_scanGraph.SetCircleColor(m_color.chiSquare);
		double maxChi2 = Max(chiSquare, numSpec);
		for(int i = 0; i < numSpec; ++i)
			chiSquare[i] *= (4096.0f / (float)maxChi2);
		m_scanGraph.DrawCircles(angle, chiSquare, numSpec, Graph::CGraphCtrl::PLOT_SECOND_AXIS);
	}
}

/** Draws the fluxes up to the currently selected scan */
void  CDataBrowserDlg::DrawFlux(){
	static double flux[200];
	static double tid[200];
	double maxFlux = -10;
	double minFlux = 10;
	CString columnAxisLabel;
	CDateTime dateTime;
	Common common;

	// If no ev.log has been opened yet.
	if(m_calculator == NULL)
		return;


	// The unit conversion
	// Set the unit of the plot
	double fluxUnitConversionFactor = 1.0;
	if(g_userSettings.m_fluxUnit == UNIT_TONDAY){
		columnAxisLabel.Format("%s [ton/day]", (LPCSTR)common.GetString(AXIS_FLUX));
		fluxUnitConversionFactor = 24*3.6;;
	}else{
		columnAxisLabel.Format("%s [kg/s]", (LPCSTR)common.GetString(AXIS_FLUX));
	}
	m_scanGraph.SetYUnits(columnAxisLabel);

	// Get how many hours we have to GMT
	double hoursToGMT = 0;
	if(m_volcanoIndex >= 0)
		hoursToGMT = g_volcanoes.m_hoursToGMT[m_volcanoIndex];


	// Copy the flux-data to the local buffer
	for(int k = 0; k <= m_curScan; ++k){
		m_calculator->m_scan[k].GetStartTime(0,dateTime);
		tid[k]	= (dateTime.hour + hoursToGMT) * 3600.0 + dateTime.minute * 60 + dateTime.second;
		flux[k] = fluxUnitConversionFactor * m_calculator->m_scan[k].GetFlux();
		maxFlux = max(maxFlux, flux[k]);
		minFlux = min(minFlux, flux[k]);
	}

	// Set the scale of the flux-graph
	m_fluxGraph.SetRangeY(minFlux, maxFlux, 1);

	// Draw the fluxes
	m_fluxGraph.DrawCircles(tid, flux, m_curScan + 1, CGraphCtrl::PLOT_FIXED_AXIS | CGraphCtrl::PLOT_CIRCLES);
}

/** Updates the information about the currently selected scan */
void CDataBrowserDlg::UpdateScanInfo(){
	CString str;
	CDateTime dateTime;

	// If no ev.log has been opened yet.
	if(m_calculator == NULL)
		return;

	// A handle to the current scan
	Evaluation::CScanResult &scan = m_calculator->m_scan[m_curScan];

	// The number of spectra in one scan
	int numSpecPerScan	= scan.GetEvaluatedNum();

	int index = 0;

	// The scan-index
	str.Format("%d", m_curScan + 1);
	m_scanInfoList.SetItemText(index++, 1, str);

	// The number of scans in this eval-log
	str.Format("%d", m_calculator->m_scanNum);
	m_scanInfoList.SetItemText(index++, 1, str);

	// The date
	scan.GetStartTime(0, dateTime);
	str.Format("%04d.%02d.%02d",	dateTime.year, dateTime.month,	dateTime.day);
	m_scanInfoList.SetItemText(index++, 1, str);

	// The startTime
	str.Format("%02d.%02d.%02d",	dateTime.hour, dateTime.minute,	dateTime.second);
	m_scanInfoList.SetItemText(index++, 1, str);

	// The stopTime
	scan.GetStopTime(numSpecPerScan-1, dateTime);
	str.Format("%02d.%02d.%02d",	dateTime.hour, dateTime.minute,	dateTime.second);
	m_scanInfoList.SetItemText(index++, 1, str);

	// The number of hours to GMT
	if(m_volcanoIndex == -1){
		str.Format("");
		m_scanInfoList.SetItemText(index++, 1, str);
	}else{
		str.Format("%.0lf", g_volcanoes.m_hoursToGMT[m_volcanoIndex]);
		m_scanInfoList.SetItemText(index++, 1, str);
	}

	// The volcano
	const CSpectrumInfo &info = scan.GetSpectrumInfo(0);
	str.Format("%s", (LPCSTR)info.m_volcano);
	m_scanInfoList.SetItemText(index++, 1, str);

	// The site
	str.Format("%s", (LPCSTR)info.m_site);
	m_scanInfoList.SetItemText(index++, 1, str);

	// The latitude
	str.Format("%.5lf", scan.GetLatitude());
	m_scanInfoList.SetItemText(index++, 1, str);

	// The longitude
	str.Format("%.5lf", scan.GetLongitude());
	m_scanInfoList.SetItemText(index++, 1, str);

	// The altitude
	str.Format("%.0lf", scan.GetAltitude());
	m_scanInfoList.SetItemText(index++, 1, str);

	// The compass-direction
	str.Format("%.1lf", scan.GetCompass());
	m_scanInfoList.SetItemText(index++, 1, str);

	// The serial-number of the spectrometer used
	str.Format("%s", (LPCSTR)scan.GetSerial());
	m_scanInfoList.SetItemText(index++, 1, str);

	// The type of scanner used
	if(fabs(scan.GetConeAngle() - 90.0) < 1)
		str.Format("Flat");
	else
		str.Format("Cone %.1lf°", scan.GetConeAngle());
	m_scanInfoList.SetItemText(index++, 1, str);

	// The exposure-time
	str.Format("%d", scan.GetExposureTime(0));
	m_scanInfoList.SetItemText(index++, 1, str);

	// The number of exposures
	str.Format("%d", scan.GetSpecNum(0));
	m_scanInfoList.SetItemText(index++, 1, str);

	// The calculated flux
	str.Format("%.1lf", scan.GetFlux());
	m_scanInfoList.SetItemText(index++, 1, str);

	// The temperature of the sensor
	str.Format("%.1lf", scan.GetTemperature());
	m_scanInfoList.SetItemText(index++, 1, str);

	// The battery voltage
	str.Format("%.1f", scan.GetBatteryVoltage());
	m_scanInfoList.SetItemText(index++, 1, str);
}

/** Initializes the controls of the dialog according to the information in
  the most recently read evaluation log. */
void CDataBrowserDlg::InitializeControls(){

	m_curScan = 0;
	m_scanSpinCtrl.SetRange(0, (short)(m_calculator->m_scanNum));
	m_scanSpinCtrl.SetPos((short)m_curScan);

}

void CDataBrowserDlg::OnViewPeakIntensity(){
	m_show.peakIntensity = !m_show.peakIntensity;
	int show = (m_show.peakIntensity) ? SW_SHOW: SW_HIDE;
	m_legendPeakIntensity.ShowWindow(show);
	m_legendPeakIntensity.SetBackgroundColor(m_color.peakIntensity);
	m_labelPeakIntensity.ShowWindow(show);
	DrawScan();
}

void CDataBrowserDlg::OnViewFitIntensity(){
	m_show.fitIntensity = !m_show.fitIntensity;
	int show = (m_show.fitIntensity) ? SW_SHOW: SW_HIDE;
	m_legendFitIntensity.ShowWindow(show);
	m_legendFitIntensity.SetBackgroundColor(m_color.fitIntensity);
	m_labelFitIntensity.ShowWindow(show);
	DrawScan();
}
void CDataBrowserDlg::OnViewColumnError(){
	m_show.columnError = !m_show.columnError;
	int show = (m_show.columnError) ? SW_SHOW : SW_HIDE;
	m_legendColumn.ShowWindow(show);
	m_legendColumn.SetBackgroundColor(m_color.fitIntensity);
	m_labelColumn.ShowWindow(show);
	DrawScan();
}

void CDataBrowserDlg::OnViewDelta(){
	m_show.delta = !m_show.delta;
	int show = (m_show.delta) ? SW_SHOW: SW_HIDE;
	m_legendDelta.ShowWindow(show);
	m_legendDelta.SetBackgroundColor(m_color.delta);
	m_labelDelta.ShowWindow(show);
	DrawScan();
}

void CDataBrowserDlg::OnViewChiSquare(){
	m_show.chiSquare = !m_show.chiSquare;	
	int show = (m_show.chiSquare) ? SW_SHOW: SW_HIDE;
	m_legendChiSquare.ShowWindow(show);
	m_legendChiSquare.SetBackgroundColor(m_color.chiSquare);
	m_labelChiSquare.ShowWindow(show);
	DrawScan();
}

/** Called when the user wants to see the next scan */
void CDataBrowserDlg::OnShowPreviousScan(){
	m_curScan -= 1;

	// set the limits for the selected scan
	m_curScan = max(m_curScan, 0);
	m_curScan = min(m_curScan, m_calculator->m_scanNum - 1);

	// Redraw the screen
	DrawScan();
	DrawFlux();

	// Update the scan-information
	UpdateScanInfo();
}

/** Called when the user wants to go back one scan */
void CDataBrowserDlg::OnShowNextScan(){
	m_curScan += 1;

	// set the limits for the selected scan
	m_curScan = max(m_curScan, 0);
	m_curScan = min(m_curScan, m_calculator->m_scanNum - 1);

	// Redraw the screen
	DrawScan();
	DrawFlux();

	// Update the scan-information
	UpdateScanInfo();
}

void CDataBrowserDlg::OnChangeSelectedScan(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);

	// If no ev.log has been opened yet
	if(m_calculator == NULL)
		return;

	// If there's no scans read, we cannot change the current scannumber
	if(m_calculator->m_scanNum <= 0){
		m_curScan = 0;
		return;
	}

	// Set the currently selected scan
	this->m_curScan = m_scanSpinCtrl.GetPos32();
	if(pNMUpDown->iDelta > 0)
		OnShowNextScan();
	else
		OnShowPreviousScan();

	*pResult = 0;
}


void CDataBrowserDlg::OnUpdateViewPeakintensity(CCmdUI *pCmdUI)
{
	if (m_show.peakIntensity)
		pCmdUI->SetCheck(1);
	else
		pCmdUI->SetCheck(0);
}
void CDataBrowserDlg::OnUpdateViewFitintensity(CCmdUI *pCmdUI)
{
	if (m_show.fitIntensity)
		pCmdUI->SetCheck(1);
	else
		pCmdUI->SetCheck(0);
}
void CDataBrowserDlg::OnUpdateViewChiSquare(CCmdUI *pCmdUI)
{
	if (m_show.chiSquare)
		pCmdUI->SetCheck(1);
	else
		pCmdUI->SetCheck(0);
}
void CDataBrowserDlg::OnUpdateViewDelta(CCmdUI *pCmdUI)
{
	if (m_show.delta)
		pCmdUI->SetCheck(1);
	else
		pCmdUI->SetCheck(0);
}
void CDataBrowserDlg::OnUpdateViewColumnError(CCmdUI *pCmdUI)
{
	if (m_show.columnError)
		pCmdUI->SetCheck(1);
	else
		pCmdUI->SetCheck(0);
}

void CDataBrowserDlg::OnInitMenuPopup(CMenu *pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
	ASSERT(pPopupMenu != NULL);
	// Check the enabled state of various menu items.

	CCmdUI state;
	state.m_pMenu = pPopupMenu;
	ASSERT(state.m_pOther == NULL);
	ASSERT(state.m_pParentMenu == NULL);

	// Determine if menu is popup in top-level menu and set m_pOther to
	// it if so (m_pParentMenu == NULL indicates that it is secondary popup).
	HMENU hParentMenu;
	if (AfxGetThreadState()->m_hTrackingMenu == pPopupMenu->m_hMenu)
		state.m_pParentMenu = pPopupMenu;		// Parent == child for tracking popup.
	else if ((hParentMenu = ::GetMenu(m_hWnd)) != NULL)
	{
		CWnd* pParent = this;
		// Child windows don't have menus--need to go to the top!
		if (pParent != NULL &&
			(hParentMenu = ::GetMenu(pParent->m_hWnd)) != NULL)
		{
			int nIndexMax = ::GetMenuItemCount(hParentMenu);
			for (int nIndex = 0; nIndex < nIndexMax; nIndex++)
			{
				if (::GetSubMenu(hParentMenu, nIndex) == pPopupMenu->m_hMenu)
				{
					// When popup is found, m_pParentMenu is containing menu.
					state.m_pParentMenu = CMenu::FromHandle(hParentMenu);
					break;
				}
			}
		}
	}

	state.m_nIndexMax = pPopupMenu->GetMenuItemCount();
	for (state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax;
		state.m_nIndex++)
	{
		state.m_nID = pPopupMenu->GetMenuItemID(state.m_nIndex);
		if (state.m_nID == 0)
			continue; // Menu separator or invalid cmd - ignore it.

		ASSERT(state.m_pOther == NULL);
		ASSERT(state.m_pMenu != NULL);
		if (state.m_nID == (UINT)-1)
		{
			// Possibly a popup menu, route to first item of that popup.
			state.m_pSubMenu = pPopupMenu->GetSubMenu(state.m_nIndex);
			if (state.m_pSubMenu == NULL ||
				(state.m_nID = state.m_pSubMenu->GetMenuItemID(0)) == 0 ||
				state.m_nID == (UINT)-1)
			{
				continue;			 // First item of popup can't be routed to.
			}
			state.DoUpdate(this, TRUE);	 // Popups are never auto disabled.
		}
		else
		{
			// Normal menu item.
			// Auto enable/disable if frame window has m_bAutoMenuEnable
			// set and command is _not_ a system command.
			state.m_pSubMenu = NULL;
			state.DoUpdate(this, FALSE);
		}

		// Adjust for menu deletions and additions.
		UINT nCount = pPopupMenu->GetMenuItemCount();
		if (nCount < state.m_nIndexMax)
		{
			state.m_nIndex -= (state.m_nIndexMax - nCount);
			while (state.m_nIndex < nCount &&
				pPopupMenu->GetMenuItemID(state.m_nIndex) == state.m_nID)
			{
				state.m_nIndex++;
			}
		}
		state.m_nIndexMax = nCount;
	}
}