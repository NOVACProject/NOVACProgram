// PostFluxDlg.cpp : implementation file
//

#include "stdafx.h"
#include "../NovacMasterProgram.h"
#include "PostFluxDlg.h"

#include "../VolcanoInfo.h"
#include "../UserSettings.h"
#include "../Geometry/GeometryCalculator.h"
#include "../Dialogs/SelectionDialog.h"
#include "../Dialogs/GeometryDlg.h"


extern CVolcanoInfo g_volcanoes;           // The global list of volcanoes in the NOVAC network
extern CUserSettings g_userSettings;       // <-- The users preferences

// CPostFluxDlg dialog

IMPLEMENT_DYNAMIC(CPostFluxDlg, CDialog)
CPostFluxDlg::CPostFluxDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPostFluxDlg::IDD, pParent)
{
	m_calculator = NULL;

	m_chi2Limit = -1;
	m_intensityAbove = -1;
	m_intensityBelow = -1;

	// setting the offset
	m_offsetOption = OFFSET_CALCULATE;
	m_userOffset = 0;

	// setting the colors
	m_color.column				= RGB(255, 0, 0);
	m_color.badColumn			= RGB(128, 0, 0);
	m_color.delta				= RGB(0, 0, 255);
	m_color.chiSquare			= RGB(255, 0, 255);
	m_color.peakIntensity		= RGB(255, 255, 255);
	m_color.fitIntensity		= RGB(255, 255, 0);
	m_color.offset				= RGB(255,128,0);

	// the current selections
	m_curScan = 0;
	m_curSpecie = 0;
	m_selectedNum = 0;

	//
	m_automaticChange = false;

	// the options for what to show
	m_show.columnError		= true;
	m_show.delta			= false;
	m_show.chiSquare		= false;
	m_show.peakIntensity	= true;
	m_show.fitIntensity		= true;

	// assume that the data is given with intensities, not saturation-ratios
	m_useSaturationRatio	= false;

	// by default we don't have any wind-file readers
	m_wfReader = NULL;
}

CPostFluxDlg::~CPostFluxDlg()
{
	if(m_calculator != NULL){
		delete m_calculator;
		m_calculator = NULL;
	}

	if(m_wfReader != NULL){
		delete m_wfReader;
		m_wfReader = NULL;
	}

}

void CPostFluxDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_FLUXUNIT,				m_fluxUnitCombo);
	DDX_Control(pDX, IDC_SCANGRAPH_FRAME,				m_scanGraphFrame);
	DDX_Control(pDX, IDC_SCAN_SPINBTN,					m_scanSpinCtrl);
	DDX_Control(pDX, IDC_INTENSITY_SLIDER,				m_intensitySlider);
	DDX_Control(pDX, IDC_GAS_COMBO,						m_gasCombo);
	DDX_Radio(pDX, IDC_RADIO_CALCULATE_OFFSET,			m_offsetOption);

	// The 'calculate flux' button
	DDX_Control(pDX,	IDC_BTN_CALC_FLUX,				m_calcFluxBtn);
	DDX_Control(pDX,	IDC_EDIT_FLUX,					m_editFlux);

	DDX_Control(pDX,	IDC_EDIT_SELECTEDPOINTS,		m_editSelectedPoints);

	// The input-boxes for wind-speed and -direction
	DDX_Control(pDX,	IDC_PF_WINDSPEED				,m_editWindSpeed);
	DDX_Control(pDX,	IDC_PF_WINDDIRECTION			,m_editWindDirection);
	DDX_Control(pDX,	IDC_PF_PLUMEHEIGHT				,m_editPlumeHeight);
	DDX_Control(pDX,	IDC_PF_COMPASS					,m_editCompass);

	// The input-box for the tilt
	DDX_Control(pDX,	IDC_PF_TILT,					m_editTilt);

	// The parameters for judging the offset-level and the quality of the spectra
	DDX_Text(pDX,	IDC_EDIT_OFFSET_CHI2,				m_chi2Limit);
	DDX_Text(pDX, IDC_EDIT_OFFSET_INTENSITY_ABOVE,		m_intensityAbove);
	DDX_Text(pDX, IDC_EDIT_OFFSET_INTENSITY_BELOW,		m_intensityBelow);

	// The explanation to what is shown in the plot
	DDX_Control(pDX, IDC_LEGEND_COLUMN,					m_legendColumn);
	DDX_Control(pDX, IDC_LEGEND_PEAKINTENSITY,			m_legendPeakIntensity);
	DDX_Control(pDX, IDC_LEGEND_FITINTENSITY,			m_legendFitIntensity);
	DDX_Control(pDX, IDC_LEGEND_DELTA,					m_legendDelta);
	DDX_Control(pDX, IDC_LEGEND_CHISQUARE,				m_legendChiSquare);
	DDX_Control(pDX, IDC_STATIC_COLUMN,					m_labelColumn);
	DDX_Control(pDX, IDC_STATIC_PEAKINTENSITY,			m_labelPeakIntensity);
	DDX_Control(pDX, IDC_STATIC_FITINTENSITY,			m_labelFitIntensity);
	DDX_Control(pDX, IDC_STATIC_DELTA,					m_labelDelta);
	DDX_Control(pDX, IDC_STATIC_CHISQUARE,				m_labelChiSquare);

	// The user's selection of the cone-angle
	DDX_Control(pDX, IDC_PF_CONEANGLE,					m_coneangleCombo);

	// The user's selection of the instrument-type
	//DDX_Control(pDX, IDC_PF_INSTRUMENTTYPE,				m_instrumentTypeCombo);
}


BEGIN_MESSAGE_MAP(CPostFluxDlg, CDialog)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SCAN_SPINBTN, OnChangeSelectedScan)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_INTENSITY_SLIDER, OnReleaseIntensitySlider)

	// Changing the text-boxes
	ON_EN_CHANGE(IDC_EDIT_SELECTEDPOINTS,		OnChangeSelectionList)
	ON_EN_KILLFOCUS(IDC_EDIT_OFFSET,			OnKillFocus_Offset)
	ON_EN_CHANGE(IDC_EDIT_OFFSET,				OnChangeEdit_Offset)

	// Changing the gas to calculate the flux for
	ON_CBN_SELCHANGE(IDC_GAS_COMBO,				OnChangeGas)

	// Changing the cone-angle
	ON_CBN_SELCHANGE(IDC_PF_CONEANGLE,			OnChangeConeAngle)

	// Changing the instrument-type
	//ON_CBN_SELCHANGE(IDC_PF_INSTRUMENTTYPE,		OnChangeInstrumentType)

	// Clicking the buttons
	ON_BN_CLICKED(IDC_BTN_BROWSE_EVALLOG,		OnBrowseEvallog)
	ON_BN_CLICKED(IDC_BTN_CALC_FLUX,			OnCalcFlux)
	ON_BN_CLICKED(IDC_BTN_DELETEPOINTS,			OnDeleteSelectedPoints)
	ON_BN_CLICKED(IDC_BTN_RESETDELETION,		OnResetDeletion)

	// Saving the evaluation-log
	ON_BN_CLICKED(ID_FILE_SAVEEVALUATIONLOG,	OnSaveEvalLog)

	// Saving the scan graph as a picture or as an ascii-file
	ON_COMMAND(ID__SAVEASASCII,					OnSaveScanGraph_AsASCII)
	ON_COMMAND(ID__SAVEASBITMAP,				OnSaveScanGraph_AsBitmap)

	// Changing the view
	ON_COMMAND(ID_VIEW_PEAKINTENSITY,					OnViewPeakIntensity)
	ON_COMMAND(ID_VIEW_FITINTENSITY,					OnViewFitIntensity)
	ON_COMMAND(ID_VIEW_COLUMNERROR,						OnViewColumnError)
	ON_COMMAND(ID_VIEW_DELTA,							OnViewDelta)
	ON_COMMAND(ID_VIEW_CHISQUARE,						OnViewChiSquare)
	ON_COMMAND(ID_VIEW_PREVIOUSSCAN,					OnShowPreviousScan)
	ON_COMMAND(ID_VIEW_NEXTSCAN,						OnShowNextScan)
	ON_COMMAND(ID_CALCULATE_FLUXFORALLSCANSINTHEPLUME,	OnCalculateFlux_AllScansInPlume)
	ON_WM_KEYUP()

	// Changing the way the offset is calculated
	ON_BN_CLICKED(IDC_RADIO_CALCULATE_OFFSET,			OnChangeOffsetCalculation)
	ON_BN_CLICKED(IDC_RADIO_CALCULATEOFFSET_2,			OnChangeOffsetCalculation)
	ON_BN_CLICKED(IDC_RADIO_USETHIS_OFFSET,				OnChangeOffsetCalculation)

	// Changing the parameters for the offset-calculation
	ON_EN_CHANGE(IDC_EDIT_OFFSET_CHI2,					OnChangeOffsetCalculation)
	ON_EN_CHANGE(IDC_EDIT_OFFSET_INTENSITY_ABOVE,		OnChangeOffsetCalculation)
	ON_EN_CHANGE(IDC_EDIT_OFFSET_INTENSITY_BELOW,		OnChangeOffsetCalculation)
	ON_COMMAND(ID_CALCULATE_WINDDIRECTION,				OnCalculateWinddirection)

	// Importing wind
	ON_COMMAND(ID_FILE_IMPORTWINDFIELD,					OnImportWindField)

	// Updating the interface
	ON_UPDATE_COMMAND_UI(ID_VIEW_PEAKINTENSITY,			OnUpdateViewPeakintensity)
	ON_UPDATE_COMMAND_UI(ID_VIEW_FITINTENSITY,			OnUpdateViewFitintensity)
	ON_UPDATE_COMMAND_UI(ID_VIEW_CHISQUARE,				OnUpdateViewChiSquare)
	ON_UPDATE_COMMAND_UI(ID_VIEW_DELTA,					OnUpdateViewDelta)
	ON_UPDATE_COMMAND_UI(ID_VIEW_COLUMNERROR,			OnUpdateViewColumnError)

	// Updating the interface, notice that this has to be here due to a bug in Microsoft MFC
	//	http://support.microsoft.com/kb/242577
	ON_WM_INITMENUPOPUP()

	// The user has clicked the label 'compass direction'
	ON_STN_CLICKED(IDC_LABEL_COMPASSDIRECTION,			OnClickedLabelCompassdirection)
	ON_STN_CLICKED(IDC_LABEL_PLUMEHEIGHT,				OnClickedLabelCompassdirection)
	ON_STN_CLICKED(IDC_LABEL_CONEANGLE,					OnClickedLabelCompassdirection)
	ON_STN_CLICKED(IDC_LABEL_TILT,						OnClickedLabelCompassdirection)
END_MESSAGE_MAP()


// CPostFluxDlg message handlers

BOOL CPostFluxDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CRect rect, windowRect;
	int height, width;
	Common common;
	CString columnAxisLabel;

	// Move the window to the right side of the screen
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	GetWindowRect(windowRect);
	int windowWidth = windowRect.Width();
	windowRect.right = screenWidth; windowRect.left = windowRect.right - windowWidth;
	MoveWindow(windowRect);

	// load the available units into the unit selection box
	m_fluxUnitCombo.AddString("kg/s");
	m_fluxUnitCombo.AddString("ton/day");
	m_fluxUnitCombo.AddString("kg/h");
	if(g_userSettings.m_fluxUnit == UNIT_TONDAY)
		m_fluxUnitCombo.SetCurSel(1);
	else
		m_fluxUnitCombo.SetCurSel(0);

	// Initialize the list of available gases
	m_gasCombo.ResetContent();
	m_gasCombo.AddString("SO2");
	m_gasCombo.AddString("NO2");
	m_gasCombo.AddString("O3");
	m_gasCombo.SetCurSel(0);

	// Initialize the list of available instrument-types
	//m_instrumentTypeCombo.ResetContent();
	//m_instrumentTypeCombo.AddString("Gothenburg");
	//m_instrumentTypeCombo.AddString("Heidelberg");
	//m_instrumentTypeCombo.SetCurSel(0);

	// Initialize the unit to use
	if(g_userSettings.m_columnUnit == UNIT_MOLEC_CM2)
		columnAxisLabel.Format("%s [molec/cm²]", (LPCTSTR)common.GetString(AXIS_COLUMN));
	else
		columnAxisLabel.Format("%s [ppmm]", (LPCTSTR)common.GetString(AXIS_COLUMN));

	// Initialize the scan graph
	this->m_scanGraphFrame.GetWindowRect(rect);
	height = rect.bottom - rect.top;
	width	= rect.right - rect.left;
	rect.top = 20; rect.bottom = height - 10;
	rect.left = 10; rect.right = width - 10;	
	m_scanGraph.Create(WS_VISIBLE | WS_CHILD, rect, &m_scanGraphFrame);
	m_scanGraph.parent = this;
	m_scanGraph.SetSecondRangeY(0, 4096, 0, false);
	m_scanGraph.SetSecondYUnit(common.GetString(AXIS_INTENSITY));
	m_scanGraph.SetXUnits(common.GetString(AXIS_ANGLE));
	m_scanGraph.SetYUnits(columnAxisLabel);
	m_scanGraph.EnableGridLinesX(true);
	m_scanGraph.SetBackgroundColor(RGB(0, 0, 0));
	m_scanGraph.SetPlotColor(RGB(255, 0, 0));
	m_scanGraph.SetGridColor(RGB(255, 255, 255));
	m_scanGraph.SetRange(-90, 90, 0, 0, 100, 0);


	// Initialize the intensity slider
	m_intensitySlider.SetRange(0, 4095);
	m_intensitySlider.SetPos(4095 - 400);/* The intensity slider is upside down */
	m_intensitySlider.SetTicFreq(512);

	// The list of available cone-angles
	m_coneAngleNum = 2;
	m_coneAngles[0] = 90.0; m_coneAngles[1] = 60.0;
	m_coneangleCombo.ResetContent();
	for(int k = 0; k < m_coneAngleNum; ++k){
		CString coneAngleStr;
		coneAngleStr.Format("%.0f", m_coneAngles[k]);
		m_coneangleCombo.InsertString(k, coneAngleStr);
	}
	m_coneangleCombo.SetCurSel(0);
	m_editTilt.EnableWindow(FALSE);

	// Initialize the legend
	InitLegend();

	// Open the flux-error dialog
	m_fluxErrorDlg.m_geomError				= 30.0;
	m_fluxErrorDlg.m_specError				= 15.0;
	m_fluxErrorDlg.m_scatteringError	= 30.0;
	m_fluxErrorDlg.m_windError				= 30.0;
	m_fluxErrorDlg.Create(IDD_FLUXERROR_DIALOG);
	m_fluxErrorDlg.ShowWindow(SW_SHOW);

	// Disable the controls that we cannot use until we have opened a evaluation log
	// enable all the buttons and controls...
	m_editTilt.EnableWindow(FALSE);
	m_coneangleCombo.EnableWindow(FALSE);
	//m_instrumentTypeCombo.EnableWindow(FALSE);
	m_calcFluxBtn.EnableWindow(FALSE);
	m_editWindSpeed.EnableWindow(FALSE);
	m_editWindDirection.EnableWindow(FALSE);
	m_editPlumeHeight.EnableWindow(FALSE);
	m_gasCombo.EnableWindow(FALSE);
	m_editCompass.EnableWindow(FALSE);
	m_editFlux.EnableWindow(FALSE);
	m_editSelectedPoints.EnableWindow(FALSE);

	return TRUE;	// return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CPostFluxDlg::OnBrowseEvallog()
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
		if(m_calculator != NULL)
			delete m_calculator;
		m_calculator = new CPostFluxCalculator();

		// set the path
		m_calculator->m_evaluationLog.Format("%s", (LPCTSTR)evLog);

		// Read the evaluation log
		m_calculator->ReadEvaluationLog();

		// If there are no scans in the log-file, then return at once
		if(m_calculator->m_scanNum <= 0){
			MessageBox("There are no scans in the given log-file");
			delete m_calculator;
			m_calculator = NULL;
			return;
		}

		// Update the screen
		this->SetDlgItemText(IDC_EVALUATION_LOG_EDIT, m_calculator->m_evaluationLog);
		InitializeControls();
		DrawScan();
	}else{
		
	}
}

void CPostFluxDlg::DrawScan(){
	CString message, scalingStr, columnAxisLabel;
	Common common;
	static double angle[MAX_SPEC_PER_SCAN];
	static double times[MAX_SPEC_PER_SCAN];
	static double column[MAX_SPEC_PER_SCAN];
	static double badColumn[MAX_SPEC_PER_SCAN];
	static double badColumnErr[MAX_SPEC_PER_SCAN];
	static bool   badEvaluation[MAX_SPEC_PER_SCAN];
	static double columnError[MAX_SPEC_PER_SCAN];
	static double peakIntensity[MAX_SPEC_PER_SCAN];
	static double fitIntensity[MAX_SPEC_PER_SCAN];
	static double delta[MAX_SPEC_PER_SCAN];
	static double chiSquare[MAX_SPEC_PER_SCAN];
	static double selectedAngles[MAX_SPEC_PER_SCAN];
	static double selectedColumns[MAX_SPEC_PER_SCAN];
	double *xAxisValues = angle;

	// remove the old plot
	m_scanGraph.CleanPlot();

	// Set the unit of the plot
	if(g_userSettings.m_columnUnit == UNIT_MOLEC_CM2)
		columnAxisLabel.Format("%s [molec/cm²]", (LPCTSTR)common.GetString(AXIS_COLUMN));
	else
		columnAxisLabel.Format("%s [ppmm]", (LPCTSTR)common.GetString(AXIS_COLUMN));
	m_scanGraph.SetYUnits(columnAxisLabel);

	// If no ev.log has been opened yet.
	if(m_calculator == NULL)
		return;

	// A handle to the current scan
	Evaluation::CScanResult &scan = m_calculator->m_scan[m_curScan];

	// The number of spectra in this scan
	int numSpec = min(scan.GetEvaluatedNum(), MAX_SPEC_PER_SCAN);

	// Check the type of measurement
	bool isTimeSeries = (scan.IsWindMeasurement() || scan.IsStratosphereMeasurement());

	// The unit conversion
	double columnUnitConversionFactor = 1.0;
	if(g_userSettings.m_columnUnit == UNIT_MOLEC_CM2)
		columnUnitConversionFactor = 2.5e15;

	// The offset, calculate or use the users value
	switch(m_offsetOption){
		case OFFSET_CALCULATE:
			m_userOffset = m_calculator->CalculateOffset(m_curScan, m_calculator->m_specie[m_curSpecie]);
			break;
		case OFFSET_CALCULATE_PARAM:
			m_userOffset = m_calculator->CalculateOffset(m_curScan, m_calculator->m_specie[m_curSpecie], m_chi2Limit, m_intensityAbove, m_intensityBelow);
			break;
		case OFFSET_USER:
			SaveUserOffset();
			m_calculator->m_scan[m_curScan].SetOffset(m_userOffset);
			break;
		default:
			ASSERT(0);
	}
	
	// Copy the data to a local buffer
	for(int k = 0; k < numSpec; ++k){
		angle[k]			= scan.GetScanAngle(k);
		peakIntensity[k]	= scan.GetPeakIntensity(k);
		fitIntensity[k]		= scan.GetFitIntensity(k);
		delta[k]			= scan.GetDelta(k);
		chiSquare[k]		= scan.GetChiSquare(k);
		if(scan.IsOk(k) && !scan.IsDeleted(k)){
			column[k]		= columnUnitConversionFactor * (scan.GetColumn(k, m_curSpecie) - m_userOffset);
			columnError[k]	= columnUnitConversionFactor * scan.GetColumnError(k, m_curSpecie);
			badColumn[k]	= 0.0;
			badColumnErr[k]	= 0.0;
			badEvaluation[k]= false;
		}else{
			column[k]		= 0.0;
			columnError[k]	= 0.0;
			badColumn[k]	= columnUnitConversionFactor * (scan.GetColumn(k, m_curSpecie) - m_userOffset);
			badColumnErr[k]	= columnUnitConversionFactor * scan.GetColumnError(k, m_curSpecie);
			badEvaluation[k]= true;
		}
	}
	// If this is a time-series then we need the start-times for each spectrum
	if(isTimeSeries){
		for(int k = 0; k < numSpec; ++k){
			const CDateTime *tid = scan.GetStartTime(k);
			times[k] = tid->hour * 3600 + tid->minute * 60 + tid->second;
		}
	}

	// Check if this scan is in the plume
	bool inplume = false;
	double plumeCentre1, plumeCentre2, plumeCompleteness, plumeEdge_low, plumeEdge_high;
	if(!scan.IsWindMeasurement() && !scan.IsStratosphereMeasurement()){
		inplume = scan.CalculatePlumeCentre(scan.GetSpecieName(0, m_curSpecie), plumeCentre1, plumeCentre2, plumeCompleteness, plumeEdge_low, plumeEdge_high);
	}

	// update the plume-info label
	if(inplume){
		message.Format("In Plume, centre at %.0lf degrees, %.0lf %% complete", plumeCentre1, plumeCompleteness * 100.0);
	}else{
		if(scan.IsFluxMeasurement()){
			message.Format("No Plume");
		}else if(scan.IsWindMeasurement()){
			if(scan.IsWindMeasurement_Heidelberg())
				message.Format("Wind measurement");
			if(scan.GetSkySpectrumInfo().m_channel == 0)
				message.Format("Wind measurement - Master channel");
			else
				message.Format("Wind measurement - Slave channel");
		}else if(scan.IsCompositionMeasurement()){
			message.Format("Composition measurement");
		}else if(scan.IsStratosphereMeasurement()){
			message.Format("Stratosphere measurement");
		}
	}
	SetDlgItemText(IDC_LABEL_PLUME_INFO, message);

	// update the status message
	message.Format("Scan number: %d out of %d scans in file", 1 + m_curScan, m_calculator->m_scanNum);
	SetDlgItemText(IDC_STATUSBAR, message);

	// Update the start-time label on the screen
	const CDateTime *startTime = scan.GetStartTime(0);
	if(startTime != NULL){
		unsigned short date[3];
		if(SUCCESS == scan.GetDate(0, date)){
			message.Format("Scan started on: %04d.%02d.%02d at %02d:%02d:%02d", 
			date[0], date[1], date[2], startTime->hour, startTime->minute, startTime->second);
		}else{
			message.Format("Scan started at: %02d:%02d:%02d", startTime->hour, startTime->minute, startTime->second);
		}
		SetDlgItemText(IDC_LEGEND_STARTTIME, message);
	}

	// Update the type of the instrument
	//INSTRUMENT_TYPE type = scan.GetInstrumentType();
	//switch(type){
	//	case INSTR_GOTHENBURG:
	//		m_instrumentTypeCombo.SetCurSel(0);
	//		m_coneangleCombo.EnableWindow(TRUE); break;
	//	case INSTR_HEIDELBERG:
	//		m_instrumentTypeCombo.SetCurSel(1);
	//		m_coneangleCombo.EnableWindow(FALSE); break;
	//	default: 
	//		m_instrumentTypeCombo.SetCurSel(0);
	//		m_coneangleCombo.EnableWindow(TRUE); break;
	//}

	// Get the ranges for the intensites, normalize if necessary
	double maxPeakIntensity = Max(peakIntensity, numSpec);
	double divisor = 1.0;
	if(maxPeakIntensity > 4096){
		if(scan.GetSpecNum(0) == 0){
			divisor = ceil(maxPeakIntensity / 4095);
		}else{
			divisor = scan.GetSpecNum(0);
		}
	}
	if(divisor > 1){
		for(int k = 0; k < numSpec; ++k){
			peakIntensity[k] /= (double)divisor;
			fitIntensity[k]	/= (double)divisor;
		}
	}

	// If the intensities are given as saturation ratio, change the right-axis and legend
	if(maxPeakIntensity < 1.01){
		if(!m_useSaturationRatio){
			// Initialize the intensity slider
			m_intensitySlider.SetRange(0, 100);
			m_intensitySlider.SetPos(97);/* The intensity slider is upside down */
			m_intensitySlider.SetTicFreq(10);
		}
		m_useSaturationRatio = true;

		m_scanGraph.SetSecondRangeY(0, 100, 0, false);
		m_scanGraph.SetSecondYUnit("Saturation [%]");
		SetDlgItemText(IDC_STATIC_PEAKINTENSITY, "Peak Saturation Ratio");
		SetDlgItemText(IDC_STATIC_FITINTENSITY, "Fit Saturation Ratio");
		for(int k = 0; k < numSpec; ++k){
			peakIntensity[k] *= 100.0;
			fitIntensity[k]	*= 100.0;
		}
	}else{
		if(m_useSaturationRatio){
			// Initialize the intensity slider
			m_intensitySlider.SetRange(0, 4095);
			m_intensitySlider.SetPos(4095 - 400);/* The intensity slider is upside down */
			m_intensitySlider.SetTicFreq(512);
		}
		m_useSaturationRatio = false;

		m_scanGraph.SetSecondRangeY(0, 4096, 0, false);
		m_scanGraph.SetSecondYUnit(common.GetString(AXIS_INTENSITY));
		SetDlgItemText(IDC_STATIC_PEAKINTENSITY, "Peak Intensity");
		SetDlgItemText(IDC_STATIC_FITINTENSITY, "Fit Intensity");
	}

	// Get the ranges for the data
	double minColumn	= Min(column, numSpec);
	double maxColumn	= Max(column, numSpec);
	if(minColumn == maxColumn)
		maxColumn += 1;

	if(!isTimeSeries){
		double minAngle		= Min(angle, numSpec);
		double maxAngle		= Max(angle, numSpec);
		if(minAngle == maxAngle)
			maxAngle += 1;

		// Set the properties for the plot
		if(maxAngle > 90.0 || minAngle < -90.0){
			m_scanGraph.SetRange(minAngle, maxAngle, 0, minColumn, maxColumn, 1);
		}else{
			m_scanGraph.SetRange(-90.0, 90.0, 0, minColumn, maxColumn, 1);
		}
		m_scanGraph.SetXAxisNumberFormat(Graph::FORMAT_GENERAL);
		m_scanGraph.SetXUnits(common.GetString(AXIS_ANGLE));

		// draw the columns
		this->m_scanGraph.SetPlotColor(m_color.column);
		if(m_show.columnError){
			m_scanGraph.BarChart(angle, column, columnError, numSpec, Graph::CGraphCtrl::PLOT_FIXED_AXIS);
		}else{
			m_scanGraph.BarChart(angle, column, numSpec, Graph::CGraphCtrl::PLOT_FIXED_AXIS);
		}

		// draw the bad columns
		m_scanGraph.SetPlotColor(m_color.badColumn);
		if(m_show.columnError){
			m_scanGraph.BarChart(angle, badColumn, badColumnErr, numSpec, Graph::CGraphCtrl::PLOT_FIXED_AXIS);
		}else{
			m_scanGraph.BarChart(angle, badColumn, numSpec, Graph::CGraphCtrl::PLOT_FIXED_AXIS);
		}

		// draw the selected columns
		if(m_selectedNum > 0){
			memset(selectedColumns, 0, MAX_SPEC_PER_SCAN*sizeof(double));
			for(int i = 0; i < m_selectedNum; ++i){
				selectedColumns[m_selected[i]] = column[m_selected[i]];
			}
			m_scanGraph.SetPlotColor(RGB(0, 0, 255), false);
			m_scanGraph.BarChart(angle, selectedColumns, numSpec, Graph::CGraphCtrl::PLOT_FIXED_AXIS);
		}
	}else{
		// this is a time-series
		double minTime		= Min(times, numSpec);
		double maxTime		= Max(times, numSpec);
		if(minTime == maxTime)
			maxTime += 1;

		// Set the properties for the plot
		m_scanGraph.SetRange(minTime, maxTime, 0, minColumn, maxColumn, 1);
		m_scanGraph.SetXUnits(common.GetString(AXIS_TIMEOFDAY));
		m_scanGraph.SetXAxisNumberFormat(Graph::FORMAT_TIME);

		// draw the columns
		this->m_scanGraph.SetPlotColor(m_color.column);
		if(m_show.columnError){
			m_scanGraph.XYPlot(times, column, NULL, NULL, columnError, numSpec, Graph::CGraphCtrl::PLOT_FIXED_AXIS | Graph::CGraphCtrl::PLOT_CONNECTED);
		}else{
			m_scanGraph.XYPlot(times, column, numSpec, Graph::CGraphCtrl::PLOT_FIXED_AXIS | Graph::CGraphCtrl::PLOT_CONNECTED);
		}

		// to make the plot show against time
		xAxisValues = times;
	}

	// draw the intensity
	if(m_show.peakIntensity){
		m_scanGraph.SetCircleColor(m_color.peakIntensity);
		m_scanGraph.DrawCircles(xAxisValues, peakIntensity, numSpec, Graph::CGraphCtrl::PLOT_SECOND_AXIS);
	}
	
	// draw the fit-intensity
	if(m_show.fitIntensity){
		m_scanGraph.SetCircleColor(m_color.fitIntensity);
		m_scanGraph.DrawCircles(xAxisValues, fitIntensity, numSpec, Graph::CGraphCtrl::PLOT_SECOND_AXIS);
	}

	// draw the delta of the residual
	if(m_show.delta){
		m_scanGraph.SetCircleColor(m_color.delta);
		double maxDelta = Max(delta, numSpec);
		float peakValue = (m_useSaturationRatio) ? 100.0f : 4096.0f;
		for(int i = 0; i < numSpec; ++i)
			delta[i] *= (peakValue / (float)maxDelta);
		m_scanGraph.DrawCircles(xAxisValues, delta, numSpec, Graph::CGraphCtrl::PLOT_SECOND_AXIS);

		// Show the scaling constant used...
		scalingStr.Format("Delta(residual) × %.2g", (peakValue / (float)maxDelta));
		SetDlgItemText(IDC_STATIC_DELTA, scalingStr);
	}

	// draw the chi-square of the fit
	if(m_show.chiSquare){
		m_scanGraph.SetCircleColor(m_color.chiSquare);
		double maxChi2	= Max(chiSquare, numSpec);
		float peakValue = (m_useSaturationRatio) ? 100.0f : 4096.0f;
		for(int i = 0; i < numSpec; ++i)
			chiSquare[i] *= (peakValue / (float)maxChi2);
		m_scanGraph.DrawCircles(xAxisValues, chiSquare, numSpec, Graph::CGraphCtrl::PLOT_SECOND_AXIS);

		// Show the scaling constant used...
		scalingStr.Format("Chi² of Fit × %.2g", (peakValue / (float)maxChi2));
		SetDlgItemText(IDC_STATIC_CHISQUARE, scalingStr);
	}

	// Draw the plume-centre position (if known);
	if(inplume){
		m_scanGraph.DrawLine(Graph::VERTICAL, plumeCentre1, RGB(255, 255, 0), Graph::STYLE_DASHED);
		
		//m_scanGraph.DrawLine(Graph::VERTICAL, plumeEdge_low,  RGB(150, 150, 0), Graph::STYLE_DASHED);
		//m_scanGraph.DrawLine(Graph::VERTICAL, plumeEdge_high, RGB(150, 150, 0), Graph::STYLE_DASHED);
	}
	
	// If this is not a normal scan then we cannot calculate any flux for it...
	if(scan.IsFluxMeasurement()){
		m_calcFluxBtn.EnableWindow(TRUE);
	}else{
		m_calcFluxBtn.EnableWindow(FALSE);
	}
}

void CPostFluxDlg::InitializeControls(){
	CString str;

	// If no ev.log has been opened yet
	if(m_calculator == NULL)
		return;

	// enable all the buttons and controls...
	m_editTilt.EnableWindow(TRUE);
	m_coneangleCombo.EnableWindow(TRUE);
	//m_instrumentTypeCombo.EnableWindow(TRUE);
	m_calcFluxBtn.EnableWindow(TRUE);
	m_editWindSpeed.EnableWindow(TRUE);
	m_editWindDirection.EnableWindow(TRUE);
	m_editPlumeHeight.EnableWindow(TRUE);
	m_gasCombo.EnableWindow(TRUE);
	m_editCompass.EnableWindow(TRUE);
	m_editFlux.EnableWindow(TRUE);
	m_editSelectedPoints.EnableWindow(TRUE);

	// Initialize the spin control
	m_curScan = 0;
	m_scanSpinCtrl.SetRange(0, (short)(m_calculator->m_scanNum));
	m_scanSpinCtrl.SetPos((short)m_curScan);

	// The wind data
	str.Format("%.1lf", m_windField.GetWindSpeed());
	SetDlgItemText(IDC_PF_WINDSPEED, str);

	str.Format("%.1lf", m_windField.GetWindDirection());
	SetDlgItemText(IDC_PF_WINDDIRECTION, str);

	str.Format("%.0lf", m_windField.GetPlumeHeight());
	SetDlgItemText(IDC_PF_PLUMEHEIGHT, str);

	// The scanner setup
	const CSpectrumInfo &info = m_calculator->m_scan[m_curScan].GetSpectrumInfo(0);
	m_calculator->m_compass = info.m_compass;
	str.Format("%.1f", m_calculator->m_compass);
	SetDlgItemText(IDC_PF_COMPASS, str);

	// The tilt
	str.Format("%.1lf", info.m_pitch);
	SetDlgItemText(IDC_PF_TILT, str);

	// The cone-angle
	int k;
	for(k = 0; k < m_coneAngleNum; ++k){
		if(fabs(info.m_coneAngle - m_coneAngles[k]) < 1){
			m_coneangleCombo.SetCurSel(k);
			if(k == 0){
				m_editTilt.EnableWindow(FALSE); // <-- we cannot have any tilt for a flat scanner
			}else{
				m_editTilt.EnableWindow(TRUE);
			}
			break;
		}
	}

	if(k == m_coneAngleNum){
		str.Format("%.1f", m_calculator->m_coneAngle);
		m_coneangleCombo.InsertString(m_coneAngleNum, str);
		m_coneAngles[m_coneAngleNum] = m_calculator->m_coneAngle;
		++m_coneAngleNum;
	}

	// The species evaluated for
	m_gasCombo.ResetContent();
	for(int k = 0; k < m_calculator->m_specieNum; ++k)
		m_gasCombo.InsertString(k, m_calculator->m_specie[k]);
	m_gasCombo.SetCurSel(0);

	// Initialize the legend
	InitLegend();
}

void CPostFluxDlg::InitLegend(){
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
}
void CPostFluxDlg::OnChangeSelectedScan(NMHDR *pNMHDR, LRESULT *pResult)
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

void CPostFluxDlg::OnReleaseIntensitySlider(NMHDR *pNMHDR, LRESULT *pResult)
{
	UpdateSelectionList();

	*pResult = 0;
}

void CPostFluxDlg::UpdateSelectionList(){
	CString str;

	// If no ev.log has been opened yet
	if(m_calculator == NULL)
		return;

	// The currently selected scan
	Evaluation::CScanResult &scan = m_calculator->m_scan[m_curScan];

	// The selected intensity level
	double selectedIntensity, divisor;
	if(m_useSaturationRatio){
		selectedIntensity = (100 - m_intensitySlider.GetPos()) / 100.0;
		divisor						= 1.0;
	}else{
		selectedIntensity = 4095 - m_intensitySlider.GetPos();
		if(scan.GetSpectrumInfo(0).m_numSpec != 0){
			divisor					= scan.GetSpectrumInfo(0).m_numSpec;
		}else{
			double maxIntensity = 0.0;
			for(unsigned long i = 0; i < scan.GetEvaluatedNum(); ++i){
				maxIntensity = max(maxIntensity, scan.GetPeakIntensity(i));
			}
			divisor					= floor(maxIntensity / 4095);
		}
	}

	// The offset
	m_userOffset = scan.GetOffset();
	str.Format("%.3lf", m_userOffset);
	SetDlgItemText(IDC_EDIT_OFFSET, str);

	// Reset
	m_selectedNum = 0;
	str.Format("");

	// Update the list with currently selected spectra
	for(unsigned long i = 0; i < scan.GetEvaluatedNum(); ++i){
		if(scan.GetPeakIntensity(i)/divisor < selectedIntensity){
			if(m_selectedNum == 0)
				str.AppendFormat("%d", i);
			else
				str.AppendFormat(",%d", i);

			m_selected[m_selectedNum] = i;
			++m_selectedNum;
		}
	}

	// Finally, do the update. 
	m_automaticChange = true;
	SetDlgItemText(IDC_EDIT_SELECTEDPOINTS, str);

	DrawScan();
}

void CPostFluxDlg::OnChangeSelectionList()
{
	CString str;
	char buffer[16384];
	unsigned long i;

	// If no ev.log has been opened yet
	if(m_calculator == NULL)
		return;

	// If the update was made by the program, not by the user.
	if(m_automaticChange){
		m_automaticChange = false;
		return;
	}

	// Get the list of selected items
	this->GetDlgItemText(IDC_EDIT_SELECTEDPOINTS, str);
	sprintf(buffer, "%s", (LPCTSTR)str);

	// reset the selection list
	m_selectedNum = 0;

	char* szToken = (char*)(LPCSTR)buffer;
	while(szToken = strtok(szToken, ",; ")){
		if(strlen(szToken) == 0)
			continue;

		if(!sscanf(szToken, "%d", &i))
			break;

		if(i >= 0 && i <= m_calculator->m_scan[m_curScan].GetEvaluatedNum()){
			m_selected[m_selectedNum] = i;
			++m_selectedNum;
		}


		szToken = NULL;
	}
	// Redraw the screen
	DrawScan();
}

/** Retrieves the wind-speed and -direction to use and fills the values
		into the correct place for calculating the flux */
void	CPostFluxDlg::RetrieveWindField(){
	CString ws, wd, ph;
	double tmpDouble;

	if(m_wfReader == NULL){
		GetDlgItemText(IDC_PF_WINDSPEED, ws);
		GetDlgItemText(IDC_PF_WINDDIRECTION, wd);
		GetDlgItemText(IDC_PF_PLUMEHEIGHT, ph);
		if (sscanf(ws, "%lf", &tmpDouble) == 1) {
			m_calculator->m_wind.SetWindSpeed(tmpDouble, MET_USER);
		}
		if (sscanf(wd, "%lf", &tmpDouble) == 1) {
			m_calculator->m_wind.SetWindDirection(tmpDouble, MET_USER);
		}
		if (sscanf(ph, "%lf", &tmpDouble) == 1) {
			m_calculator->m_wind.SetPlumeHeight(tmpDouble, MET_USER);
		}
	}else{
		// Find the time 
		CDateTime dt;
		m_calculator->m_scan[this->m_curScan].GetStartTime(0, dt);
		if(SUCCESS != m_wfReader->InterpolateWindField(dt, m_calculator->m_wind)){
			MessageBox("Failed to interpolate the wind for the date and time for the current scan from the given wind file. Please supply another wind file and try again", "Error");
			return;
		}

		// Tell the user what wind field we've used
		if(m_wfReader->m_containsWindSpeed){
			ws.Format("%.2lf", m_calculator->m_wind.GetWindSpeed());
			SetDlgItemText(IDC_PF_WINDSPEED,			ws);
		}else{
			GetDlgItemText(IDC_PF_WINDSPEED, ws);
			int ret = sscanf(ws, "%lf", &tmpDouble);	m_calculator->m_wind.SetWindSpeed(tmpDouble, MET_USER);
		}

		if(m_wfReader->m_containsWindDirection){
			wd.Format("%.2lf", m_calculator->m_wind.GetWindDirection());
			SetDlgItemText(IDC_PF_WINDDIRECTION,	wd);
		}else{
			GetDlgItemText(IDC_PF_WINDDIRECTION, wd);
			int ret = sscanf(wd, "%lf", &tmpDouble);	m_calculator->m_wind.SetWindDirection(tmpDouble, MET_USER);
		}

		if(m_wfReader->m_containsPlumeHeight){
			ph.Format("%.1lf", m_calculator->m_wind.GetPlumeHeight());
			SetDlgItemText(IDC_PF_PLUMEHEIGHT,	ph);
		}else{
			GetDlgItemText(IDC_PF_PLUMEHEIGHT, ph);
			int ret = sscanf(ph, "%lf", &tmpDouble);	m_calculator->m_wind.SetPlumeHeight(tmpDouble, MET_USER);
		}
	}
}

void CPostFluxDlg::OnCalcFlux()
{
	CString compass, ph, coneAngle, str, tilt;
//	double tmpDouble;
	int ret; // return value from sscanf

#ifdef _DEBUG
	// this is for searching for memory leaks
	CMemoryState newMem, oldMem, diffMem;
	oldMem.Checkpoint();
#endif

	// If no ev.log has been opened yet
	if(m_calculator == NULL)
		return;

	// if there's no data, there's no use to try to calculate the flux
	if(m_calculator->m_scanNum <= 0)
		return;

	// Save all the data
	UpdateData(TRUE);

	// get the wind-speed and direction
	RetrieveWindField();

	// Get the compass-direction
	GetDlgItemText(IDC_PF_COMPASS, compass);
	ret = sscanf(compass, "%f", &m_calculator->m_compass);

	// The cone-angle
	if(m_coneangleCombo.GetCurSel() == -1)
		m_calculator->m_coneAngle = 90.0;
	else
		m_calculator->m_coneAngle = m_coneAngles[m_coneangleCombo.GetCurSel()];

	// The tilt
	GetDlgItemText(IDC_PF_TILT, tilt);
	ret = sscanf(tilt, "%f", &m_calculator->m_tilt);

	// The instrument-type
	//if(m_instrumentTypeCombo.GetCurSel() == 0)
	//	m_calculator->m_scan[m_curScan].SetInstrumentType(INSTR_GOTHENBURG);
	//else if(m_instrumentTypeCombo.GetCurSel() == 1)
	//	m_calculator->m_scan[m_curScan].SetInstrumentType(INSTR_HEIDELBERG);
	//else
	//	m_calculator->m_scan[m_curScan].SetInstrumentType(INSTR_GOTHENBURG);

	// The offset, calculate or use the users value
	switch(m_offsetOption){
		case OFFSET_CALCULATE:
			m_userOffset = m_calculator->CalculateOffset(m_curScan, m_calculator->m_specie[m_curSpecie]);
			break;
		case OFFSET_CALCULATE_PARAM:
			m_userOffset = m_calculator->CalculateOffset(m_curScan, m_calculator->m_specie[m_curSpecie], m_chi2Limit, m_intensityBelow, m_intensityAbove);
			break;
		case OFFSET_USER:
			SaveUserOffset();
			m_calculator->m_scan[m_curScan].SetOffset(m_userOffset);
			break;
		default:
			ASSERT(0);
	}
	str.Format("%.3lf", m_userOffset);
	SetDlgItemText(IDC_EDIT_OFFSET, str);

	// Remember the error-estimates the user has supplied us with
	m_fluxErrorDlg.UpdateData(TRUE);
	m_calculator->m_scan[m_curScan].SetGeometricalError(m_fluxErrorDlg.m_geomError);
	m_calculator->m_scan[m_curScan].SetScatteringError(m_fluxErrorDlg.m_scatteringError);
	m_calculator->m_scan[m_curScan].SetSpectroscopicalError(m_fluxErrorDlg.m_specError);
	m_calculator->m_wind.SetWindError(m_fluxErrorDlg.m_windError);

	// Calculate the flux
	m_calculator->CalculateFlux(m_curScan);

	double flux = Convert(m_calculator->m_flux);

	// show the result in the window
	str.Format("%.2lf", flux);
	SetDlgItemText(IDC_EDIT_FLUX, str);

	// redraw the screen
	DrawScan();

#ifdef _DEBUG
	// this is for searching for memory leaks
	newMem.Checkpoint();
	if(diffMem.Difference(oldMem, newMem)){
		diffMem.DumpStatistics(); 
//		diffMem.DumpAllObjectsSince();
	}
#endif
}

void CPostFluxDlg::OnDeleteSelectedPoints()
{
	int i;

	// If no ev.log has been opened yet
	if(m_calculator == NULL)
		return;

	// A handle to the current scan.
	Evaluation::CScanResult &scan = m_calculator->m_scan[m_curScan];

	// Mark the selected data points as deleted
	for(i = 0; i < m_selectedNum; ++i)
    {
		scan.MarkAs(m_selected[i], MARK_DELETED);
	}

	// Redraw the graph
	DrawScan();
}

void CPostFluxDlg::OnResetDeletion()
{
	// If no ev.log has been opened yet
	if(m_calculator == NULL)
		return;

	// A handle to the current scan.
	Evaluation::CScanResult &scan = m_calculator->m_scan[m_curScan];

	// Mark the selected data points as deleted
	for(unsigned long i = 0; i < scan.GetEvaluatedNum(); ++i){
		scan.RemoveMark(i, MARK_DELETED);
	}

	// Redraw the graph
	DrawScan();
}

void CPostFluxDlg::OnChangeGas()
{
	CString gas;

	// If no ev.log has been opened yet
	if(m_calculator == NULL)
		return;

	// Get the specie
	m_curSpecie = m_gasCombo.GetCurSel();
	m_gasCombo.GetLBText(m_curSpecie, gas);

	// Get the gas factor (conversion from ppmm to mg/m^2)
	m_calculator->m_gasFactor = Common::GetGasFactor(gas);

	// Redraw the screen
	DrawScan();

}

/** Called when the user changes the cone-angle */
void CPostFluxDlg::OnChangeConeAngle(){
	if(m_coneangleCombo.GetCurSel() == 0){
		m_editTilt.EnableWindow(FALSE); // <-- we cannot have any tilt for a flat scanner
	}else{
		m_editTilt.EnableWindow(TRUE);
	}
}

//void CPostFluxDlg::OnChangeInstrumentType()
//{
//	// If no ev.log has been opened yet
//	if(m_calculator == NULL)
//		return;
//
//	// The instrument-type
//	if(m_instrumentTypeCombo.GetCurSel() == 0)
//		m_calculator->m_instrumentType = INSTR_GOTHENBURG;
//	else if(m_instrumentTypeCombo.GetCurSel() == 1)
//		m_calculator->m_instrumentType = INSTR_HEIDELBERG;
//	else
//		m_calculator->m_instrumentType = INSTR_GOTHENBURG;
//
//	// enabling or disabling the cone-angle
//	switch(m_calculator->m_instrumentType){
//		case INSTR_HEIDELBERG: 
//			m_coneangleCombo.EnableWindow(FALSE); break;
//		default:
//			m_coneangleCombo.EnableWindow(TRUE); break;
//	}
//
//}


double CPostFluxDlg::Convert(double flux){
	if(m_fluxUnitCombo.GetCurSel() == 0)
		return flux;

	if(m_fluxUnitCombo.GetCurSel() == 1)
		return flux * 3.6 * 24.0;	// ton/day

	if(m_fluxUnitCombo.GetCurSel() == 2)
		return flux * 3600.0;		// kg/h

	// shouldn't happen.
	return flux;
}
void CPostFluxDlg::OnViewPeakIntensity()
{
	m_show.peakIntensity = !m_show.peakIntensity;
	int show = (m_show.peakIntensity) ? SW_SHOW: SW_HIDE;
	m_legendPeakIntensity.ShowWindow(show);
	m_legendPeakIntensity.SetBackgroundColor(m_color.peakIntensity);
	m_labelPeakIntensity.ShowWindow(show);
	DrawScan();
}

void CPostFluxDlg::OnViewFitIntensity()
{
	m_show.fitIntensity = !m_show.fitIntensity;
	int show = (m_show.fitIntensity) ? SW_SHOW: SW_HIDE;
	m_legendFitIntensity.ShowWindow(show);
	m_legendFitIntensity.SetBackgroundColor(m_color.fitIntensity);
	m_labelFitIntensity.ShowWindow(show);
	DrawScan();
}

void CPostFluxDlg::OnViewColumnError()
{
	m_show.columnError = !m_show.columnError;
	DrawScan();
}

void CPostFluxDlg::OnViewDelta()
{
	m_show.delta = !m_show.delta;
	int show = (m_show.delta) ? SW_SHOW: SW_HIDE;
	m_legendDelta.ShowWindow(show);
	m_legendDelta.SetBackgroundColor(m_color.delta);
	m_labelDelta.ShowWindow(show);
	DrawScan();
}

void CPostFluxDlg::OnViewChiSquare()
{
	m_show.chiSquare = !m_show.chiSquare;
	
	int show = (m_show.chiSquare) ? SW_SHOW: SW_HIDE;
	m_legendChiSquare.ShowWindow(show);
	m_legendChiSquare.SetBackgroundColor(m_color.chiSquare);
	m_labelChiSquare.ShowWindow(show);

	DrawScan();
}

void CPostFluxDlg::OnKillFocus_Offset()
{

	// save the offset.
	SaveUserOffset();

	// change the radio, if necessary
	if(m_offsetOption == OFFSET_CALCULATE){
		m_offsetOption = OFFSET_USER;
		UpdateData(FALSE);
	}

	DrawScan();
}

void CPostFluxDlg::SaveUserOffset(bool correct){
	double fValue;
	CString str;

	// get the text in the offset dialog
	GetDlgItemText(IDC_EDIT_OFFSET, str);

	if(1 != sscanf(str, "%lf", &fValue)){
		if(correct){
			// cannot parse the data as a string
			str.Format("%.3lf", m_userOffset);
			SetDlgItemText(IDC_EDIT_OFFSET, str);
		}
		return;
	}

	// read the data 
	m_userOffset = fValue;
}
void CPostFluxDlg::OnChangeEdit_Offset()
{
	// save the offset.
	SaveUserOffset(false);

	DrawScan();
}

void CPostFluxDlg::OnShowPreviousScan()
{
	m_curScan -= 1;

	// set the limits for the selected scan
	m_curScan = max(m_curScan, 0);
	m_curScan = min(m_curScan, m_calculator->m_scanNum - 1);

	// remove the old flux calculation
	SetDlgItemText(IDC_EDIT_FLUX, "");

	// updates the selection list
	UpdateSelectionList();

}

void CPostFluxDlg::OnShowNextScan()
{
	m_curScan += 1;


	// set the limits for the selected scan
	m_curScan = max(m_curScan, 0);
	m_curScan = min(m_curScan, m_calculator->m_scanNum - 1);

	// remove the old flux calculation
	SetDlgItemText(IDC_EDIT_FLUX, "");

	// updates the selection list
	UpdateSelectionList();

}

void CPostFluxDlg::OnCalculateFlux_AllScansInPlume()
{
	CString compass, ph, coneAngle, str;
	double tmpDouble;
	int ret; // return value from sscanf

	// If no ev.log has been opened yet
	if(m_calculator == NULL)
		return;

	// If there's no data, there's no use to try to calculate the flux
	if(m_calculator->m_scanNum <= 0)
		return;

	// Tell the user about what is about to happen
	str.Format("This will calculate the flux for all scans which are judged as being in the plume. Only automatically removed data points will be deleted, and the offset will be calculated automatically. Do you want to continue?");
	int answer = MessageBox(str, "Automatic Calculation", MB_YESNO | MB_ICONWARNING);
	if(answer == IDNO)
		return;

	// Save all the data
	UpdateData(TRUE);

	// Get the plume height
	GetDlgItemText(IDC_PF_PLUMEHEIGHT, ph);
	ret = sscanf(ph, "%lf", &tmpDouble);	m_calculator->m_wind.SetPlumeHeight(tmpDouble, MET_USER);

	// Get the compass-direction
	GetDlgItemText(IDC_PF_COMPASS, compass);
	ret = sscanf(compass, "%f", &m_calculator->m_compass);

	// The cone-angle
	if(m_coneangleCombo.GetCurSel() == -1)
		m_calculator->m_coneAngle = 90.0;
	else
		m_calculator->m_coneAngle = m_coneAngles[m_coneangleCombo.GetCurSel()];

	// The instrument-type
	//if(m_instrumentTypeCombo.GetCurSel() == 0)
	//	m_calculator->m_scan[m_curScan].SetInstrumentType(INSTR_GOTHENBURG);
	//else if(m_instrumentTypeCombo.GetCurSel() == 1)
	//	m_calculator->m_scan[m_curScan].SetInstrumentType(INSTR_HEIDELBERG);
	//else
	//	m_calculator->m_scan[m_curScan].SetInstrumentType(INSTR_GOTHENBURG);

	for(m_curScan = 0; m_curScan < m_calculator->m_scanNum; ++m_curScan){

		// A handle to the current scan
		Evaluation::CScanResult &scan = m_calculator->m_scan[m_curScan];

		// Check if this scan is in the plume
		double plumeCentre1, plumeCentre2, plumeCompleteness, plumeEdge_low, plumeEdge_high;
		bool inplume = scan.CalculatePlumeCentre(m_calculator->m_specie[m_curSpecie], plumeCentre1, plumeCentre2, plumeCompleteness, plumeEdge_low, plumeEdge_high);

		// If this measurement is not in the plume, then continue with the next
		//	scan in the file.
		if(!inplume)
			continue;

		// get the wind-speed and direction
		RetrieveWindField();

		// The offset, calculate or use the users value
		m_userOffset = m_calculator->CalculateOffset(m_curScan, m_calculator->m_specie[m_curSpecie]);

		str.Format("%.3lf", m_userOffset);
		SetDlgItemText(IDC_EDIT_OFFSET, str);

		// Calculate the flux
		m_calculator->CalculateFlux(m_curScan);

		double flux = Convert(m_calculator->m_flux);

		// show the result in the window
		str.Format("%.2lf", flux);
		SetDlgItemText(IDC_EDIT_FLUX, str);

		// redraw the screen
		DrawScan();
	}
}

void CPostFluxDlg::OnChangeOffsetCalculation()
{
	UpdateData(TRUE);

	DrawScan();
}

void CPostFluxDlg::OnCalculateWinddirection()
{
	CString phStr, compStr;
	double assumedPlumeHeight, coneAngle, compass, plumeCentre, plumeCompleteness, tilt, tmp;
	double plumeEdge_low, plumeEdge_high;
	CGPSData source, scannerPos;
	int ret; // return values from sscanf

	// If no ev.log has been opened yet.
	if(m_calculator == NULL)
		return;

	// A handle to the current scan
	Evaluation::CScanResult &scan = m_calculator->m_scan[m_curScan];

	// 1. Get the assumed plume-height
	GetDlgItemText(IDC_PF_PLUMEHEIGHT, phStr);
	ret = sscanf(phStr, "%lf", &assumedPlumeHeight);

	// 2. Get the assumed compass-direction
	GetDlgItemText(IDC_PF_COMPASS, compStr);
	ret = sscanf(compStr, "%lf", &compass);

	// 3. Get the nearest volcano
	double sLat = scan.GetLatitude();
	double sLon = scan.GetLongitude();
	int volcanoIndex;
	if(fabs(sLat) < 1e-2 && fabs(sLon) < 1e-2){
		MessageBox("Could not calculate wind-direction: There is no GPS-information for this scan.");
		return;
	}else{
		volcanoIndex = Geometry::CGeometryCalculator::GetNearestVolcano(sLat, sLon);
		if(volcanoIndex == -1)
			return; //<-- no volcano found
	}
	source.m_latitude		= g_volcanoes.m_peakLatitude[volcanoIndex];
	source.m_longitude	= g_volcanoes.m_peakLongitude[volcanoIndex];
	source.m_altitude		= g_volcanoes.m_peakHeight[volcanoIndex];

	// 4. Get the system GPS
	scannerPos.m_altitude	= scan.GetAltitude();
	scannerPos.m_latitude	= scan.GetLatitude();
	scannerPos.m_longitude= scan.GetLongitude();

	// 5. Check if the plume-centre position has been calculated yet
	plumeCentre = scan.GetCalculatedPlumeCentre();
	if(plumeCentre < -900){
		scan.CalculatePlumeCentre(scan.GetSpecieName(0, m_curSpecie), plumeCentre, tmp, plumeCompleteness, plumeEdge_low, plumeEdge_high);
	}
	if(plumeCentre < -900)
		return; // <-- no plume to see.

	// 6. Get the cone-angle of the system
	coneAngle	= scan.GetConeAngle();

	// 7. Get the tilt of the system
	tilt = scan.GetPitch();

	// 8. Calculate the wind-direction
	double windDirection = Geometry::CGeometryCalculator::GetWindDirection(source, assumedPlumeHeight, scannerPos, compass, plumeCentre, coneAngle, tilt);
	if(windDirection < -900)
		return;

	m_windField.SetWindDirection(windDirection, MET_GEOMETRY_CALCULATION);

	// Update the label on the screen
	CString str;
	str.Format("%.1lf", m_windField.GetWindDirection());
	SetDlgItemText(IDC_PF_WINDDIRECTION, str);

}


/** Called when the user wants to save a copy of the current evaluation log */
void CPostFluxDlg::OnSaveEvalLog(){
	CString fileName, directory;
	CString dateStr, timeStr;
	CString message;

	// Check if the user has opened one eval-log first
	if(m_calculator == NULL || strlen(m_calculator->m_evaluationLog) < 3){
		MessageBox("Cannot save evaluation log file, please open one log file first");
		return;
	}

	// Get the directory of the currently open evaluation-log file
	directory.Format("%s", (LPCTSTR)m_calculator->m_evaluationLog);
	Common::GetDirectory(directory);

	// The file-name of the new evaluation-log file
	Common::GetDateText(dateStr);
	Common::GetTimeText(timeStr, "_");
	fileName.Format("%sEvaluationLog_%s_%s.txt", (LPCTSTR)directory, (LPCTSTR)dateStr, (LPCTSTR)timeStr);

	// Save the log-file
	if(SUCCESS == m_calculator->WriteEvaluationLog(fileName)){
		message.Format("Evaluation log file copied to %s", (LPCTSTR)fileName);
		MessageBox(message, "Success", MB_OK);
	}else{
		message.Format("Failed to write %s", (LPCTSTR)fileName);
		MessageBox(message, "Fail", MB_OK);
	}
}

void CPostFluxDlg::OnImportWindField(){
	CString windFile, message;
	windFile.Format("");
	TCHAR filter[512];
	int n = _stprintf(filter, "Text Files\0");
	n += _stprintf(filter + n + 1, "*.txt;\0");
	filter[n + 2] = 0;
	Common common;

	// let the user browse for an evaluation log file and if one is selected, read it
	if(!common.BrowseForFile(filter, windFile))
		return;

	// Completely reset the data.
	if(m_wfReader != NULL)
		delete m_wfReader;
	m_wfReader = new FileHandler::CWindFileReader();

	// Set the path to the file
	m_wfReader->m_windFile.Format(windFile);

	// Read the wind-file
	if(SUCCESS != m_wfReader->ReadWindFile()){
		MessageBox("Failed to parse given wind-file");
		m_wfReader = NULL;
		return;
	}else{
		message.Format("Successfully read %ld data points from file", m_wfReader->GetRecordNum());
		MessageBox(message);
	}

	// Disable the user input of the wind-speed and direction
	if(m_wfReader->m_containsWindSpeed)
		m_editWindSpeed.EnableWindow(FALSE);
	else
		m_editWindSpeed.EnableWindow(TRUE);
	if(m_wfReader->m_containsWindDirection)
		m_editWindDirection.EnableWindow(FALSE);
	else
		m_editWindDirection.EnableWindow(TRUE);
	if(m_wfReader->m_containsPlumeHeight)
		m_editPlumeHeight.EnableWindow(FALSE);
	else
		m_editPlumeHeight.EnableWindow(TRUE);
}

void CPostFluxDlg::OnInitMenuPopup(CMenu *pPopupMenu, UINT nIndex,BOOL bSysMenu)
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

void CPostFluxDlg::OnUpdateViewPeakintensity(CCmdUI *pCmdUI)
{
	if(m_show.peakIntensity)
		pCmdUI->SetCheck(BST_CHECKED);
	else
		pCmdUI->SetCheck(BST_UNCHECKED);
}
void CPostFluxDlg::OnUpdateViewFitintensity(CCmdUI *pCmdUI)
{
	if(m_show.fitIntensity)
		pCmdUI->SetCheck(1);
	else
		pCmdUI->SetCheck(0);
}
void CPostFluxDlg::OnUpdateViewChiSquare(CCmdUI *pCmdUI)
{
	if(m_show.chiSquare)
		pCmdUI->SetCheck(1);
	else
		pCmdUI->SetCheck(0);
}
void CPostFluxDlg::OnUpdateViewDelta(CCmdUI *pCmdUI)
{
	if(m_show.delta)
		pCmdUI->SetCheck(1);
	else
		pCmdUI->SetCheck(0);
}
void CPostFluxDlg::OnUpdateViewColumnError(CCmdUI *pCmdUI)
{
	if(m_show.columnError)
		pCmdUI->SetCheck(1);
	else
		pCmdUI->SetCheck(0);
}

void CPostFluxDlg::OnClickedLabelCompassdirection()
{
	if(m_calculator == NULL)
		return; // no evaluation log to open....

	Dialogs::CGeometryDlg geomDlg;

	// let the geometry dialog read in the evaluation-log
	geomDlg.m_evalLogReader[0] = new FileHandler::CEvaluationLogFileHandler();
	geomDlg.m_evalLogReader[0]->m_evaluationLog.Format(m_calculator->m_evaluationLog);
	geomDlg.m_evalLogReader[0]->ReadEvaluationLog();

	geomDlg.DoModal();

}

void CPostFluxDlg::OnSaveScanGraph_AsASCII(){
	CString fileName;
	TCHAR filter[512];
	int n = _stprintf(filter, "Text File\0");
	n += _stprintf(filter + n + 1, "*.txt\0");
	filter[n + 2] = 0;
	Common common;

	if(common.BrowseForFile_SaveAs(filter, fileName)){
		FILE *f = fopen(fileName, "w");
		if(f == NULL){
			MessageBox("Could not open file for writing. Failed to save graph.");
			return;
		}
		
		
		
		fclose(f);
	}
}

void CPostFluxDlg::OnSaveScanGraph_AsBitmap(){
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
		m_scanGraph.SaveGraph(fileName);
	}
}
