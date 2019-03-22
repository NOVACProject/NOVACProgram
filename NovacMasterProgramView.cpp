// NovacMasterProgramView.cpp : implementation of the CNovacMasterProgramView class
//

#include "stdafx.h"
#include "NovacMasterProgram.h"

#include "MainFrm.h"
#include "NovacMasterProgramDoc.h"
#include "NovacMasterProgramView.h"

#include "UserSettings.h"

#include "View_Scanner.h"

#include "Common/ReportWriter.h"
#include "Common/FluxLogFileHandler.h"
#include "Communication/LinkStatistics.h"

#include "Evaluation/ScanResult.h"
#include "Evaluation/EvaluationController.h"

#include "Geometry/GeometryResult.h"

#include "Configuration/ConfigurationFileHandler.h"

#include "PostFlux/PostFluxDlg.h"

#include "ReEvaluation/ReEvaluator.h"
#include "ReEvaluation/ReEvaluationDlg.h"
#include "ReEvaluation/ReEval_ScanDlg.h"
#include "ReEvaluation/ReEval_WindowDlg.h"
#include "ReEvaluation/ReEval_MiscSettingsDlg.h"
#include "ReEvaluation/ReEval_DoEvaluationDlg.h"

#include "Dialogs/ColumnHistoryDlg.h"
#include "Dialogs/ExportDlg.h"
#include "Dialogs/ExportSpectraDlg.h"
#include "Dialogs/ExportEvallogDlg.h"
#include "Dialogs/ManualWindDlg.h"
#include "Dialogs/ManualCompositionDlg.h"
#include "Dialogs/ImportSpectraDlg.h"
#include "Dialogs/FileTransferDlg.h"
#include "Dialogs/GeometryDlg.h"
#include "Dialogs/SplitPakFilesDlg.h"
#include "Dialogs/MergePakFilesDlg.h"
#include "Dialogs/DataBrowserDlg.h"
#include "Dialogs/PakFileInspector.h"
#include "Dialogs/SummarizeFluxDataDlg.h"

#include "WindMeasurement/PostWindDlg.h"
#include "WindMeasurement/WindSpeedResult.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern CMeteorologicalData		g_metData;
extern CConfigurationSetting	g_settings;
extern CUserSettings			g_userSettings;

CFormView* pView;

// CNovacMasterProgramView

IMPLEMENT_DYNCREATE(CNovacMasterProgramView, CFormView)

BEGIN_MESSAGE_MAP(CNovacMasterProgramView, CFormView)
	// Messages from other parts of the program 
	ON_MESSAGE(WM_STATUSMSG,					OnShowStatus)
	ON_MESSAGE(WM_UPDATE_MESSAGE,				OnUpdateMessage)
	ON_MESSAGE(WM_SHOW_MESSAGE,					OnShowMessage)
	ON_MESSAGE(WM_EVAL_SUCCESS,					OnEvalSucess)
	ON_MESSAGE(WM_EVAL_FAILURE,					OnEvalFailure)
	ON_MESSAGE(WM_CORR_SUCCESS,					OnCorrelationSuccess)
	ON_MESSAGE(WM_SCANNER_RUN,					OnScannerRun)
	ON_MESSAGE(WM_SCANNER_SLEEP,				OnScannerSleep)
	ON_MESSAGE(WM_SCANNER_NOT_CONNECT,			OnScannerNotConnect)
	ON_MESSAGE(WM_FINISH_DOWNLOAD,				OnDownloadFinished)
	ON_MESSAGE(WM_FINISH_UPLOAD,				OnUploadFinished)
	ON_MESSAGE(WM_WRITE_REPORT,					OnWriteReport)
	ON_MESSAGE(WM_PH_SUCCESS,					OnPlumeHeightSuccess)
	ON_MESSAGE(WM_NEW_WINDFIELD,				OnNewWindField)
	ON_MESSAGE(WM_REWRITE_CONFIGURATION,		OnRewriteConfigurationXml)

	// Commands from the user
	ON_COMMAND(ID_SET_LANGUAGE_ENGLISH,					OnMenuSetLanguageEnglish)
	ON_COMMAND(ID_SET_LANGUAGE_ESPA,					OnMenuSetLanguageSpanish)
	ON_COMMAND(ID_CONTROL_START,						OnMenuStartMasterController)
	ON_COMMAND(ID_CONTROL_MAKEWINDMEASUREMENT,			OnMenuMakeWindMeasurement)
	ON_COMMAND(ID_CONTROL_MAKECOMPOSITIONMEASUREMENT,	OnMenuMakeCompositionMeasurement)
	ON_COMMAND(ID_ANALYSIS_FLUX,						OnMenuAnalysisFlux)
	ON_COMMAND(ID_ANALYSIS_REEVALUATE,					OnMenuAnalysisReevaluate)
	ON_COMMAND(ID_ANALYSIS_SETUP,						OnMenuAnalysisSetup)
	ON_COMMAND(ID_ANALYSIS_BROWSEMEASUREDDATA,			OnMenuAnalysisBrowseData)
	ON_COMMAND(ID_FILE_EXPORT,							OnMenuFileExport)
	ON_COMMAND(ID_FILE_IMPORT,							OnMenuFileImport)
	ON_COMMAND(ID_FILE_SPLITMERGE,						OnMenuFileSplitMergePak)
	ON_COMMAND(ID_FILE_CHECKPAKFILE,					OnMenuFileCheckPakFile)
	ON_COMMAND(ID_CONFIGURATION_FILETRANSFER,			OnMenuConfigurationFileTransfer)
	ON_COMMAND(ID_CONFIGURATION_CONFIGURATION,			OnMenuShowConfigurationDialog)
	ON_COMMAND(ID_MENU_VIEW_INSTRUMENTTAB,				OnMenuViewInstrumentTab)
	ON_COMMAND(ID_ANALYSIS_WIND,						OnMenuAnalysisWind)

	// Changing the units
	ON_COMMAND(ID_UNITOFFLUX_KG,						OnChangeUnitOfFluxToKgS)
	ON_COMMAND(ID_UNITOFFLUX_TON,						OnChangeUnitOfFluxToTonDay)
	ON_COMMAND(ID_UNITOFCOLUMNS_PPMM,					OnChangeUnitOfColumnToPPMM)
	ON_COMMAND(ID_UNITOFCOLUMNS_MOLEC_CM2,				OnChangeUnitOfColumnToMolecCm2)

	// Updating the interface
	ON_UPDATE_COMMAND_UI(ID_CONTROL_START,				OnUpdateStart)
	ON_UPDATE_COMMAND_UI(ID_CONFIGURATION_FILETRANSFER,	OnUpdateFileTransfer)
	ON_UPDATE_COMMAND_UI(ID_SET_LANGUAGE_ENGLISH,		OnUpdateSetLanguageEnglish)
	ON_UPDATE_COMMAND_UI(ID_SET_LANGUAGE_ESPA,			OnUpdateSetLanguageSpanish)
	ON_UPDATE_COMMAND_UI(ID_MENU_VIEW_INSTRUMENTTAB,	OnUpdateMenuViewInstrumenttab)
	ON_UPDATE_COMMAND_UI(ID_UNITOFFLUX_KG,				OnUpdateChangeUnitOfFluxToKgS)
	ON_UPDATE_COMMAND_UI(ID_UNITOFFLUX_TON,				OnUpdateChangeUnitOfFluxToTonDay)
	ON_UPDATE_COMMAND_UI(ID_UNITOFCOLUMNS_PPMM,			OnUpdateChangeUnitOfColumnToPPMM)
	ON_UPDATE_COMMAND_UI(ID_UNITOFCOLUMNS_MOLEC_CM2,	OnUpdateChangeUnitOfColumnToMolecCm2)
	ON_UPDATE_COMMAND_UI(ID_ANALYSIS_SUMMARIZEFLUXDATA, OnUpdateMenuSummarizeFluxData)
	ON_UPDATE_COMMAND_UI(ID_CONTROL_MAKEWINDMEASUREMENT,		OnUpdateMakeWindMeasurement)
	ON_UPDATE_COMMAND_UI(ID_CONTROL_MAKECOMPOSITIONMEASUREMENT,	OnUpdateMakeCompositionMeasurement)

	// Windows messages
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

// CNovacMasterProgramView construction/destruction
using namespace FileHandler;
CNovacMasterProgramView::CNovacMasterProgramView()
	: CFormView(CNovacMasterProgramView::IDD)
{
	pView = this;
	m_evalDataStorage = new CEvaluatedDataStorage();
	m_commDataStorage = new CCommunicationDataStorage();

	m_overView			= NULL;
	m_windOverView	= NULL;
	m_instrumentView= NULL;
}

CNovacMasterProgramView::~CNovacMasterProgramView()
{
	delete m_evalDataStorage;
	delete m_commDataStorage;
	delete m_overView;
	delete m_windOverView;
	delete m_instrumentView;

	for(int i = 0; i < m_scannerPages.GetCount(); ++i){
		CPropertyPage *page = m_scannerPages[i];
		delete page;
	}
	m_scannerPages.RemoveAll();

	for (int i = 0; i < m_colHistoryPages.GetCount(); ++i) {
		CPropertyPage *page = m_colHistoryPages[i];
		delete page;
	}
	m_colHistoryPages.RemoveAll();
}

void CNovacMasterProgramView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);

	//{{AFX_DATA_MAP(CDbSpecView)
	DDX_Control(pDX, IDC_STATUS_MESSAGE_LIST, m_statusListBox);
	DDX_Control(pDX, IDC_MASTERFRAME, m_masterFrame);
	DDX_Control(pDX, IDC_STATUS_STATIC, m_statusFrame);
	//}}AFX_DATA_MAP
}

BOOL CNovacMasterProgramView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//	the CREATESTRUCT cs

	return CFormView::PreCreateWindow(cs);
}

void CNovacMasterProgramView::OnInitialUpdate()
{
	CString message;
	CRect rect, rect2, tabRect;
	CString fileName, windFieldFile, userSettingsFile;
	CString serialNumber, dateStr, dateStr2;

	CFormView::OnInitialUpdate();
	GetParentFrame()->RecalcLayout();
	ResizeParentToFit();

	m_common.GetExePath();

	// Initialize the master-frame
	m_sheet.Construct("", this);

	// Read the configuration file
	fileName.Format("%sconfiguration.xml", (LPCTSTR)m_common.m_exePath);
	FileHandler::CConfigurationFileHandler reader;
	reader.ReadConfigurationFile(g_settings, &fileName);
	fileName.Format("%sftplogin.xml", (LPCTSTR)m_common.m_exePath);
	reader.ReadFtpLoginConfigurationFile(g_settings, &fileName);

	// Read the user settings
	userSettingsFile.Format("%s\\user.ini", (LPCTSTR)m_common.m_exePath);
	g_userSettings.ReadSettings(&userSettingsFile);

	// If there's no output-directory specified
	if(strlen(g_settings.outputDirectory) <= 2){
		g_settings.outputDirectory.Format(m_common.m_exePath);
	}

	// Check if there's any flux-logs with data from last 24 hours
	time_t rawtime;
	struct tm * utc;
	time(&rawtime);
	utc = gmtime(&rawtime);
	int year = utc->tm_year + 1900;
	int month = utc->tm_mon + 1;
	int mday = utc->tm_mday;
	dateStr.Format("%04d.%02d.%02d", year, month, mday); // today
	rawtime -= 86400;
	utc = gmtime(&rawtime);
	year = utc->tm_year + 1900;
	month = utc->tm_mon + 1;
	mday = utc->tm_mday;
	dateStr2.Format("%04d.%02d.%02d", year, month, mday); // yesterday
	for(unsigned int it = 0; it < g_settings.scannerNum; ++it){
		serialNumber.Format(g_settings.scanner[it].spec[0].serialNumber);
		// show both last 24 hour column plot on main screen and column history tab
		if (g_settings.scanner[it].plotColumn && g_settings.scanner[it].plotColumnHistory) {
			ReadEvalLog(it, dateStr, serialNumber);
			ReadEvalLog(it, dateStr2, serialNumber);
		}
		// show column history tab but flux on main screen
		else if (!g_settings.scanner[it].plotColumn && g_settings.scanner[it].plotColumnHistory) {
			ReadEvalLog(it, dateStr, serialNumber);
			ReadEvalLog(it, dateStr2, serialNumber);
			ReadFluxLog(it, dateStr, serialNumber);
			ReadFluxLog(it, dateStr2, serialNumber);
		}
		// no column history tab and flux on main screen
		else {
			ReadFluxLog(it, dateStr, serialNumber);
			ReadFluxLog(it, dateStr2, serialNumber);
		}
	}

	// Try to find and read in a wind-field file, if any can be found...
	if(g_settings.windSourceSettings.enabled == 1 && g_settings.windSourceSettings.windFieldFile.GetLength() > 0 && IsExistingFile(g_settings.windSourceSettings.windFieldFile)){
		if(0 == g_metData.ReadWindFieldFromFile(g_settings.windSourceSettings.windFieldFile)){
			ShowMessage("Successfully read in wind-field from file");
			this->PostMessage(WM_NEW_WINDFIELD, NULL, NULL);
		}
	}

	// Check if there is any old status-log file from which we can learn anything...
	ScanStatusLogFile();

	// Initialize the controls of the screen
	InitializeControls();

	// Enable the tool tips
	if(!m_toolTip.Create(this)){
		TRACE0("Failed to create tooltip control\n"); 
	}
	CTabCtrl *tabPtr = m_sheet.GetTabControl();

	for (int i = 0; i < g_settings.scannerNum; ++i) {
		tabPtr->GetItemRect(i, &tabRect);
		m_toolTip.AddTool(tabPtr, IDD_VIEW_SCANNERSTATUS, &tabRect, IDD_VIEW_SCANNERSTATUS);
	}
	tabPtr->GetItemRect(g_settings.scannerNum, &tabRect);
	m_toolTip.AddTool(tabPtr, IDD_VIEW_OVERVIEW, &tabRect, IDD_VIEW_OVERVIEW);
	tabPtr->SetToolTips(&m_toolTip);
	tabPtr->EnableToolTips(TRUE);

	m_toolTip.SetMaxTipWidth(INT_MAX);
	m_toolTip.Activate(TRUE);

	// If the configuration says automatic startup then start up automatically
	if(g_settings.startup == CConfigurationSetting::STARTUP_AUTOMATIC){
		// start the master controller
		message.Format("Program automatically started");
		ShowMessage(message);

		this->OnMenuStartMasterController();
	}else{
		message.Format("%s", (LPCTSTR)m_common.GetString(MSG_PLEASE_PRESS_START));
		ShowMessage(message);
	}

	// initialize the default wind field
	g_metData.defaultWindField.SetPlumeHeight(1000, MET_DEFAULT);
	g_metData.defaultWindField.SetWindDirection(0, MET_DEFAULT);
	g_metData.defaultWindField.SetWindSpeed(10, MET_DEFAULT);

	// update the window
	UpdateData(FALSE);
}

void CNovacMasterProgramView::ReadFluxLog(int scannerIndex, CString dateStr, CString serialNumber) {
	FileHandler::CFluxLogFileHandler fluxLogReader;
	CString path;
	path.Format("%sOutput\\%s\\%s\\FluxLog_%s_%s.txt",
		(LPCTSTR)g_settings.outputDirectory,
		(LPCTSTR)dateStr,
		(LPCTSTR)serialNumber,
		(LPCTSTR)serialNumber,
		(LPCTSTR)dateStr);

	m_evalDataStorage->AddData(serialNumber, NULL);

	if (IsExistingFile(path)) {
		// Try to read the flux-log
		fluxLogReader.m_fluxLog.Format(path);
		if (FAIL == fluxLogReader.ReadFluxLog())
			return;

		if (fluxLogReader.m_fluxesNum > 0) {
			// Copy the read-in data to the m_evalDataStorage
			int fluxesNum = fluxLogReader.m_fluxesNum;

			for (int it2 = 0; it2 < fluxesNum; ++it2) {
				Evaluation::CFluxResult &fl = fluxLogReader.m_fluxes[it2];
				CSpectrumInfo &info = fluxLogReader.m_scanInfo[it2];
				m_evalDataStorage->AppendFluxResult(scannerIndex, fl.m_startTime, fl.m_flux, fl.m_fluxOk, info.m_batteryVoltage, info.m_temperature, info.m_exposureTime);
			}

			// Insert the last used wind-field for the current spectrometer
			CWindField windField;
			if (fluxLogReader.m_fluxes[fluxesNum - 1].m_plumeHeight > 0 && fluxLogReader.m_fluxes[fluxesNum - 1].m_plumeHeight < 5000) {
				windField.SetPlumeHeight(fluxLogReader.m_fluxes[fluxesNum - 1].m_plumeHeight, fluxLogReader.m_fluxes[fluxesNum - 1].m_plumeHeightSource);
			}
			else {
				windField.SetPlumeHeight(1000, MET_DEFAULT);
			}
			if (fluxLogReader.m_fluxes[fluxesNum - 1].m_windDirection > -180 && fluxLogReader.m_fluxes[fluxesNum - 1].m_windDirection <= 360) {
				windField.SetWindDirection(fluxLogReader.m_fluxes[fluxesNum - 1].m_windDirection, fluxLogReader.m_fluxes[fluxesNum - 1].m_windDirectionSource);
			}
			else {
				windField.SetWindDirection(0, MET_DEFAULT);
			}
			if (fluxLogReader.m_fluxes[fluxesNum - 1].m_windSpeed > -1 && fluxLogReader.m_fluxes[fluxesNum - 1].m_windSpeed <= 30) {
				windField.SetWindSpeed(fluxLogReader.m_fluxes[fluxesNum - 1].m_windSpeed, fluxLogReader.m_fluxes[fluxesNum - 1].m_windSpeedSource);
			}
			else {
				windField.SetWindSpeed(10, MET_DEFAULT);
			}

			g_metData.SetWindField(serialNumber, windField);
		}
	}
}

void CNovacMasterProgramView::ReadEvalLog(int scannerIndex, CString dateStr, CString serialNumber) {
	Common common;
	int now = common.Epoch();
	FileHandler::CEvaluationLogFileHandler evalLogReader;
	CString path;
	path.Format("%sOutput\\%s\\%s\\EvaluationLog_%s_%s.txt",
		(LPCTSTR)g_settings.outputDirectory,
		(LPCTSTR)dateStr,
		(LPCTSTR)serialNumber,
		(LPCTSTR)serialNumber,
		(LPCTSTR)dateStr);

	m_evalDataStorage->AddData(serialNumber, NULL);

	if (IsExistingFile(path)) {
		// Try to read the eval-log
		evalLogReader.m_evaluationLog.Format(path);
		if (FAIL == evalLogReader.ReadEvaluationLog())
			return;

		if (evalLogReader.m_scanNum > 0) {
			// Copy the read-in data to the m_evalDataStorage
			int scanNum = evalLogReader.m_scanNum;

			for (int i = 0; i < scanNum; ++i) {
				Evaluation::CScanResult &sr = evalLogReader.m_scan[i];
				for (unsigned long j = 0; j < sr.GetEvaluatedNum(); ++j) {
					CDateTime st;
					sr.GetStopTime(j, st);
					int time = common.Epoch(st);
					double column = sr.GetColumn(j, 0);
					double columnError = sr.GetColumnError(j, 0);
					double peakIntensity = sr.GetPeakIntensity(j);
					double fitIntensity = sr.GetFitIntensity(j);
					double angle = sr.GetScanAngle(j);
					bool isBadFit = sr.IsBad(j);
					if ((now - time) <= 86400) {
						m_evalDataStorage->AppendSpecDataHistory(scannerIndex, time, column, columnError,
							peakIntensity, fitIntensity, angle, isBadFit);
					}

				}
			}
		}
	}
}

// CNovacMasterProgramView diagnostics
#ifdef _DEBUG
void CNovacMasterProgramView::AssertValid() const
{
	CFormView::AssertValid();
}

void CNovacMasterProgramView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

CNovacMasterProgramDoc* CNovacMasterProgramView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CNovacMasterProgramDoc)));
	return (CNovacMasterProgramDoc*)m_pDocument;
}
#endif //_DEBUG

// CNovacMasterProgramView message handlers

// updates the status bar
LRESULT CNovacMasterProgramView::OnShowStatus(WPARAM wParam, LPARAM lParam){
 // CString str = "status";
	CString *msg;				 
	//msg	= &str;
	msg = (CString *)wParam;
 
	CMainFrame* pFrame = (CMainFrame*)AfxGetApp()->m_pMainWnd;
	
	pFrame->SetStatusBarText(*msg);
	
	return 0;
}

//update the first message of list box
LRESULT CNovacMasterProgramView::OnUpdateMessage(WPARAM wParam,LPARAM lParam)
{
	CString *msg = (CString *)wParam;
	int topIndex = 0;
	topIndex = m_statusListBox.GetTopIndex();
	m_statusListBox.DeleteString(topIndex);
	m_statusListBox.InsertString(0,*msg);
	delete msg;
	return 0;
}

LRESULT CNovacMasterProgramView::OnShowMessage(WPARAM wParam, LPARAM lParam){
	CString *msg = (CString *)wParam;
	CString logFile,dateStr,logPath;

	if(msg == NULL)
		return 0;

	// add the message to the log file
	m_common.GetDateText(dateStr);
	logPath.Format("%sOutput\\%s", (LPCTSTR)g_settings.outputDirectory, (LPCTSTR)dateStr);

	CreateDirectory(logPath,NULL);
	logFile.Format("%s\\StatusLog.txt", (LPCTSTR)logPath);
	FILE *f = fopen(logFile, "a+");
	if(f != NULL){
		fprintf(f, "%s\n", (LPCTSTR)*msg);
		fclose(f);
	}

	// update the status message listbox
	int nItems = m_statusListBox.GetCount();
	if(nItems > 100){
		m_statusListBox.DeleteString(100);
	}
	m_statusListBox.InsertString(0, *msg);

	if(strlen(*msg) > 15){
		// Find the longest string in the list box.
		CString str;
		CSize sz;
		int dx = 0;
		TEXTMETRIC tm;
		CDC* pDC = m_statusListBox.GetDC();
		CFont* pFont = m_statusListBox.GetFont();

		// Select the listbox font, save the old font
		CFont* pOldFont = pDC->SelectObject(pFont);
		// Get the text metrics for avg char width
		pDC->GetTextMetrics(&tm); 

		for (int i = 0; i < m_statusListBox.GetCount(); i++)
		{
			m_statusListBox.GetText(i, str);
			sz = pDC->GetTextExtent(str);

			// Add the avg width to prevent clipping
			sz.cx += tm.tmAveCharWidth;

			if (sz.cx > dx)
				dx = sz.cx;
		}
		// Select the old font back into the DC
		pDC->SelectObject(pOldFont);
		m_statusListBox.ReleaseDC(pDC);

		// Set the horizontal extent so every character of all strings can be scrolled to.
		m_statusListBox.SetHorizontalExtent(dx);
	}

	delete msg;

	return 0;
}

void CNovacMasterProgramView::ForwardMessage(int message, WPARAM wParam, LPARAM lParam){
	unsigned int i;

	// 1. forward the message to the flux-overview, if it is selected
	if(m_overView->m_hWnd != NULL)
		m_overView->PostMessage(message, wParam, lParam);

	// 2. forward the message to the instrument-overview, if it is selected
	if(m_instrumentView->m_hWnd != NULL)
		m_instrumentView->PostMessage(message, wParam, lParam);

	// 3. Find the scanner view to forward to...
	if(wParam != NULL){
		CString *serial = (CString *)wParam;

		// 3a. look for the correct spectrometer
		for(i = 0; i < g_settings.scannerNum; ++i){
			if(Equals(*serial, g_settings.scanner[i].spec[0].serialNumber))
				break;
		}
		if(i == g_settings.scannerNum)
			return; // <-- nothing found.

		// 3b. forward the message to the correct scanner view, if it is selected
		if(m_scannerPages[i]->m_hWnd != NULL)
			m_scannerPages[i]->PostMessage(message, wParam, lParam);
	}else{
		// if not to any specific scanner view then just forward to the one which is shown right now
		for(i = 0; i < g_settings.scannerNum; ++i){
			if(m_scannerPages[i]->m_hWnd != NULL){
				m_scannerPages[i]->PostMessage(message, wParam, lParam);
				break;
			}
		}
	}
}

LRESULT CNovacMasterProgramView::OnScannerRun(WPARAM wParam, LPARAM lParam)
{
	CString *serialID = (CString *)wParam;
	m_commDataStorage->SetStatus(*serialID, COMM_STATUS_GREEN);

	// forward the message to the correct scanner view
	ForwardMessage(WM_SCANNER_RUN, wParam, lParam);

	return 0;
}

LRESULT CNovacMasterProgramView::OnScannerSleep(WPARAM wParam, LPARAM lParam)
{
	CString *serialID = (CString *)wParam;
	m_commDataStorage->SetStatus(*serialID, COMM_STATUS_YELLOW);
	
	// forward the message to the correct scanner view
	ForwardMessage(WM_SCANNER_SLEEP, wParam, lParam);

	return 0;
}
LRESULT CNovacMasterProgramView::OnScannerNotConnect(WPARAM wParam, LPARAM lParam)
{
	CString *serialID = (CString *)wParam;
	m_commDataStorage->SetStatus(*serialID, COMM_STATUS_RED);
	
	// forward the message to the correct scanner view
	ForwardMessage(WM_SCANNER_NOT_CONNECT, wParam, lParam);

	return 0;
}

LRESULT CNovacMasterProgramView::OnNewWindField(WPARAM wParam, LPARAM lParam)
{
	this->ForwardMessage(WM_NEW_WINDFIELD, wParam, lParam);

	return 0;
}

LRESULT CNovacMasterProgramView::OnDownloadFinished(WPARAM wParam, LPARAM lParam)
{
	CString *serialID			= (CString *)wParam;
	double	*dataSpeed		= (double	*)lParam;

	m_commDataStorage->AddDownloadData(*serialID, *dataSpeed);
	
	return 0;
}

LRESULT CNovacMasterProgramView::OnUploadFinished(WPARAM wParam, LPARAM lParam)
{
	double linkSpeed = (double)wParam;

	m_commDataStorage->AddDownloadData("FTP", linkSpeed);
	
	return 0;
}

LRESULT CNovacMasterProgramView::OnEvalSucess(WPARAM wParam, LPARAM lParam){
	// the serial number of the spectrometer that has sucessfully evaluated one scan
	CString *serial = (CString *)wParam;
	Evaluation::CScanResult *result = (Evaluation::CScanResult *)lParam;

	if(result->GetCorruptedNum() == 0)
		m_evalDataStorage->SetStatus(*serial, STATUS_GREEN);
	else if(result->GetCorruptedNum() < 5)
		m_evalDataStorage->SetStatus(*serial, STATUS_YELLOW);
	else
		m_evalDataStorage->SetStatus(*serial, STATUS_RED);

	m_evalDataStorage->AddData(*serial, result);

	// forward the message to the correct scanner view
	if(m_overView->m_hWnd != NULL){
		m_overView->PostMessage(WM_EVAL_SUCCESS, wParam, NULL);
	}
	if(m_instrumentView->m_hWnd != NULL){
		m_instrumentView->PostMessage(WM_EVAL_SUCCESS, wParam, NULL);
	}
	for(unsigned long i = 0; i < g_settings.scannerNum; ++i){
		if(Equals(*serial, g_settings.scanner[i].spec[0].serialNumber)){
			if(m_scannerPages[i]->m_hWnd != NULL){
				Evaluation::CScanResult *copiedResult = new Evaluation::CScanResult();
				*copiedResult = *result;
				m_scannerPages[i]->PostMessage(WM_EVAL_SUCCESS, wParam, (LPARAM)copiedResult);
			}
		}
	}
	
	// See if we need to upload any auxilliary data to the FTP-server
	UploadAuxData();

	// clean up the results...
	delete result;

	return 0;
}

LRESULT CNovacMasterProgramView::OnEvalFailure(WPARAM wParam, LPARAM lParam){
	// the serial number of the spectrometer that has failed to evaluate one scan
	CString *serial = (CString *)wParam;

	if(lParam != NULL){
		Evaluation::CScanResult *result = (Evaluation::CScanResult *)lParam;
		m_evalDataStorage->AddData(*serial, result);
	}
	m_evalDataStorage->SetStatus(*serial, STATUS_RED);

	// forward the message to the correct scanner view
	ForwardMessage(WM_EVAL_FAILURE, wParam, lParam);

	return 0;
}

LRESULT CNovacMasterProgramView::OnCorrelationSuccess(WPARAM wParam, LPARAM lParam){
	// the serial number of the spectrometer from which one wind-speed measurement has been done
	WindSpeedMeasurement::CWindSpeedResult *result = (WindSpeedMeasurement::CWindSpeedResult *)wParam;

	// Remember the result
	m_evalDataStorage->AddWindData(result->m_serial, result);

	// Tell the wind-measurement overview to update, if it is available and selected
	if(m_showWindOverView == true && m_windOverView->m_hWnd != NULL)
		m_windOverView->PostMessage(WM_CORR_SUCCESS, NULL, NULL);

	// Finally, release the memory
	delete result;

	return 0;
}

LRESULT CNovacMasterProgramView::OnPlumeHeightSuccess(WPARAM wParam, LPARAM lParam){
	// The result of the measurement...
	Geometry::CGeometryResult *result = (Geometry::CGeometryResult *)wParam;

	// Release the memory
	delete result;

	return 0;
}

LRESULT CNovacMasterProgramView::OnWriteReport(WPARAM wParam, LPARAM lParam){
	FileHandler::CReportWriter::WriteReport(m_evalDataStorage, m_commDataStorage);

	return 0;
}


void CNovacMasterProgramView::OnMenuSetLanguageEnglish()
{
	g_userSettings.m_language = LANGUAGE_ENGLISH;
	g_userSettings.WriteToFile();
	
	::SetThreadLocale(MAKELCID(MAKELANGID(0x0409, SUBLANG_DEFAULT),SORT_DEFAULT));
	primaryLanguage = 0x0409;

	Common common;
	MessageBox(common.GetString(MSG_YOU_HAVE_TO_RESTART), "Change of language", MB_OK);
}

void CNovacMasterProgramView::OnMenuSetLanguageSpanish()
{
	g_userSettings.m_language = LANGUAGE_SPANISH;
	g_userSettings.WriteToFile();

	::SetThreadLocale(MAKELCID(MAKELANGID(0x0c0a, SUBLANG_DEFAULT),SORT_DEFAULT));
	primaryLanguage = 0x0c0a;

	Common common;
	MessageBox(common.GetString(MSG_YOU_HAVE_TO_RESTART), "Change of language", MB_OK);
}

void CNovacMasterProgramView::OnUpdateSetLanguageEnglish(CCmdUI *pCmdUI)
{
	if(primaryLanguage == 0x0c0a) // 0x0c0a == spanish...
		pCmdUI->SetCheck(0);
	else
		pCmdUI->SetCheck(1);
}

void CNovacMasterProgramView::OnUpdateSetLanguageSpanish(CCmdUI *pCmdUI)
{
	if(primaryLanguage == 0x0c0a)
		pCmdUI->SetCheck(1);
	else
		pCmdUI->SetCheck(0);
}

void CNovacMasterProgramView::OnUpdateStart(CCmdUI *pCmdUI)
{
	if(m_controller.m_fRunning){
		pCmdUI->Enable(FALSE);
	}else{
		pCmdUI->Enable(TRUE);
	}
}

void CNovacMasterProgramView::OnUpdateFileTransfer(CCmdUI *pCmdUI)
{
	if(m_controller.m_fRunning){
		pCmdUI->Enable(FALSE);
	}else{
		pCmdUI->Enable(TRUE);
	}
}

void CNovacMasterProgramView::OnUpdateMakeWindMeasurement(CCmdUI *pCmdUI)
{
	if(m_controller.m_fRunning){
		pCmdUI->Enable(TRUE);
	}else{
		pCmdUI->Enable(FALSE);
	}
}

void CNovacMasterProgramView::OnUpdateMakeCompositionMeasurement(CCmdUI *pCmdUI)
{
	if(m_controller.m_fRunning){
		pCmdUI->Enable(TRUE);
	}else{
		pCmdUI->Enable(FALSE);
	}
}

void CNovacMasterProgramView::OnUpdateMenuSummarizeFluxData(CCmdUI *pCmdUI)
{
#ifndef _DEBUG
	pCmdUI->Enable(FALSE);
#endif
}

void CNovacMasterProgramView::OnMenuShowConfigurationDialog()
{
	ConfigurationDialog::CConfigurationDlg dlg;
	INT_PTR ret = dlg.DoModal();
}

void CNovacMasterProgramView::OnMenuViewInstrumentTab(){
	if(m_instrumentViewVisible){
		m_sheet.RemovePage(m_instrumentView);
		m_instrumentViewVisible = false;
	}else{
		m_sheet.AddPage(m_instrumentView);
		m_instrumentViewVisible = true;
	}
}

void CNovacMasterProgramView::OnDestroy()
{
	this->m_controller.Stop();
	CFormView::OnDestroy();
}

/** Starting of the master controller */
void CNovacMasterProgramView::OnMenuStartMasterController(){
	CString fileName, programName;
	int pids[1024];

	// Check if there's already a NovacProgram running!!
	m_common.GetExePath();
	programName.Format("%s", (LPCTSTR)m_common.m_exeFileName);
	if(1 != Common::GetAllProcessIDs(programName, pids)){
		// There's more than just this instance of the program running - expect problems!
		MessageBox("NovacProgram is already running!! \n Please close the other instances of the NovacProgram and restart.", "Error");
		return;
	}

	// check if the controller is already running, if not then start it.
	if(!m_controller.m_fRunning){
		m_controller.Start();
	}else{
		ShowMessage(m_common.GetString(MSG_PROGRAM_ALREADY_RUNNING));
	}
}

void CNovacMasterProgramView::OnMenuMakeWindMeasurement(){
	Dialogs::CManualWindDlg windDlg;
	windDlg.DoModal();
}

void CNovacMasterProgramView::OnMenuMakeCompositionMeasurement(){
	Dialogs::CManualCompositionDlg compDlg;
	compDlg.DoModal();
}

BOOL CNovacMasterProgramView::PreTranslateMessage(MSG* pMsg){
	m_toolTip.RelayEvent(pMsg);

	return CFormView::PreTranslateMessage(pMsg);
}

int CNovacMasterProgramView::InitializeControls(){
	// Initialize the master frame
	CRect rect, rect2;
	CView_Scanner *page;
	ColumnHistoryDlg *histPage;
	CString site;
	TCITEM tcItem;
	m_showWindOverView = false;

	// Add the scanner view pages, one for every spectrometer configured
	if(g_settings.scannerNum == 0){
			page = new CView_Scanner();
			page->Construct(IDD_VIEW_SCANNERSTATUS);

			page->m_evalDataStorage = this->m_evalDataStorage;
			page->m_commDataStorage = this->m_commDataStorage;
			page->m_scannerIndex				= 0;
			page->m_serial.Format("unknown");

			m_sheet.AddPage(page);
			m_scannerPages.Add(page);
	}else{
		for(int i = 0; i < g_settings.scannerNum; ++i){
			page = new CView_Scanner();
			page->Construct(IDD_VIEW_SCANNERSTATUS);

			page->m_evalDataStorage = this->m_evalDataStorage;
			page->m_commDataStorage = this->m_commDataStorage;
			page->m_scannerIndex	= i;
			page->m_serial.Format("%s", (LPCTSTR)g_settings.scanner[i].spec[0].serialNumber);
			page->m_siteName.Format("%s", (LPCTSTR)g_settings.scanner[i].site);

			m_sheet.AddPage(page);
			m_scannerPages.Add(page);

			// if this system can make wind-measurements then show the wind-overView page
			if(g_settings.scanner[i].spec[0].channelNum == 2 ){
				m_showWindOverView = true;
			}
		}
	}

	// ...add the overview page...
	m_overView = new CView_OverView();
	m_overView->Construct(IDD_VIEW_OVERVIEW);
	m_overView->m_evalDataStorage = this->m_evalDataStorage;
	m_overView->m_commDataStorage = this->m_commDataStorage;
	m_sheet.AddPage(m_overView);

	// ...add the instrument overview page...
	m_instrumentView = new CView_Instrument();
	m_instrumentView->Construct(IDD_VIEW_INSTRUMENT_OVERVIEW);
	m_instrumentView->m_evalDataStorage = this->m_evalDataStorage;
	m_instrumentView->m_commDataStorage	= this->m_commDataStorage;
	m_sheet.AddPage(m_instrumentView);
	m_instrumentViewVisible	= true;

	// Add the column history pages, one for every spectrometer if configured to show
	for (int i = 0; i < g_settings.scannerNum; ++i) {
		if (g_settings.scanner[i].plotColumnHistory) {
			histPage = new ColumnHistoryDlg();
			histPage->Construct(IDD_COLUMN_HISTORY_DLG);

			histPage->m_evalDataStorage = this->m_evalDataStorage;
			histPage->m_scannerIndex = i;
			histPage->m_serialNumber.Format("%s", (LPCTSTR)g_settings.scanner[i].spec[0].serialNumber);
			histPage->m_siteName.Format("%s", (LPCTSTR)g_settings.scanner[i].site);

			m_sheet.AddPage(histPage);
			m_colHistoryPages.Add(histPage);
		}
	}

	// At last, add the wind-measurements overview page, 
	//	if there is at least one instrument which is capable of making wind-measurements
	if(m_showWindOverView){
		m_windOverView = new CView_WindMeasOverView();
		m_windOverView->Construct(IDD_VIEW_WIND_OVERVIEW);
		m_windOverView->m_evalDataStorage = this->m_evalDataStorage;
		m_sheet.AddPage(m_windOverView);
	}

	// Create the sheet and move it to it's position on the screen
	m_sheet.Create(this, WS_CHILD | WS_VISIBLE);
	m_sheet.ModifyStyleEx(0, WS_EX_CONTROLPARENT);

	// Move everything into place...
	m_masterFrame.GetWindowRect(rect);
	GetWindowRect(rect2);
	m_sheet.MoveWindow(rect.left, rect.top - rect2.top, rect2.Width(), rect.Height());
	m_masterFrame.GetWindowRect(rect);

	// Get the tab control and set the title of each tab to the serial number of the spectrometer
	CTabCtrl *tabPtr = m_sheet.GetTabControl();
	if(g_settings.scannerNum <= 0){
		
		// Get the 'item' of the tab.
		tcItem.mask = TCIF_TEXT;

		// Set the text in the 'item'
		site.Format("Unknown Scanner");
		tcItem.pszText = site.GetBuffer(256);

		// Update the tab with the updated 'item'
		tabPtr->SetItem(0, &tcItem);
	}else{
		for(int i = 0; i < g_settings.scannerNum; ++i){
			tcItem.mask = TCIF_TEXT;

			// Set the text in the 'item'. !!!! The serial-string is necessary
			//	otherwise this function changes the global object !!!!!!!!!!!
			site.Format("%s", (LPCTSTR)g_settings.scanner[i].site);
			tcItem.pszText = site.GetBuffer(256);
			
			// Update the tab with the updated 'item'
			tabPtr->SetItem(i, &tcItem);
		}
		int histCount = 0;
		for (int i =0; i < g_settings.scannerNum; ++i) {
			if (g_settings.scanner[i].plotColumnHistory) {
				tcItem.mask = TCIF_TEXT;

				// Set the text in the 'item'. !!!! The serial-string is necessary
				//	otherwise this function changes the global object !!!!!!!!!!!
				site.Format("%s History", (LPCTSTR)g_settings.scanner[i].site);
				tcItem.pszText = site.GetBuffer(256);

				// Update the tab with the updated 'item'
				int tabIndex = g_settings.scannerNum + 2 + histCount;
				tabPtr->SetItem(tabIndex, &tcItem);
				histCount++;
			}
		}
	}
	return 0;
}

void CNovacMasterProgramView::OnMenuAnalysisFlux()
{
	CPostFluxDlg fluxDlg;
	fluxDlg.DoModal();
}

void CNovacMasterProgramView::OnMenuAnalysisBrowseData(){
	Dialogs::CDataBrowserDlg dlg;
	dlg.DoModal();
}

void CNovacMasterProgramView::OnMenuAnalysisReevaluate()
{
	ReEvaluation::CReEvaluator *reeval = new ReEvaluation::CReEvaluator();
	
	ReEvaluation::CReEvaluationDlg dlg;
	dlg.Construct("ReEvaluation", this, 0);

	// the scan file page
	ReEvaluation::CReEval_ScanDlg page1;
	page1.Construct(IDD_REEVAL_SCANFILES);
	page1.m_reeval = reeval;

	// the fit windows page
	ReEvaluation::CReEval_WindowDlg page2;
	page2.Construct(IDD_REEVAL_WINDOW);
	page2.m_reeval = reeval;

	// the 'misc settings' page
	ReEvaluation::CReEval_MiscSettingsDlg page3;
	page3.Construct(IDD_REEVAL_MISC);
	page3.m_reeval = reeval;

	// the 'do evaluation' page
	ReEvaluation::CReEval_DoEvaluationDlg page4;
	page4.Construct(IDD_REEVAL_FINAL);
	page4.m_reeval = reeval;

	// add the pages
	dlg.AddPage(&page1);
	dlg.AddPage(&page2);
	dlg.AddPage(&page3);
	dlg.AddPage(&page4);

	// show the window
	dlg.DoModal();

	delete reeval;
}

void CNovacMasterProgramView::OnMenuFileExport()
{
	Dialogs::CExportDlg dlg;
	dlg.Construct("Export", this, 0);

	// the spectrum page
	Dialogs::CExportSpectraDlg page1;
	page1.Construct(IDD_EXPORT_SPECTRA);

	// the evaluation log page
	Dialogs::CExportEvallogDlg page2;
	page2.Construct(IDD_EXPORT_EVALLOG);

	// add the pages
	dlg.AddPage(&page1);
	dlg.AddPage(&page2);

	// show the window
	dlg.DoModal();
}

void CNovacMasterProgramView::OnMenuFileImport()
{
	Dialogs::CExportDlg dlg;
	dlg.Construct("Import", this, 0);

	// the spectrum page
	Dialogs::CImportSpectraDlg page1;
	page1.Construct(IDD_IMPORT_SPECTRA);

	// add the pages
	dlg.AddPage(&page1);

	// show the window
	dlg.DoModal();
}

void CNovacMasterProgramView::OnMenuFileSplitMergePak(){
	Dialogs::CExportDlg dlg;
	dlg.Construct("Split/Merge", this, 0);

	// the split pak-files into scans - page
	Dialogs::CSplitPakFilesDlg page1;
	page1.Construct(IDD_PAKFILES_SPLIT);

	// the merge pak-files - page
	Dialogs::CMergePakFilesDlg page2;
	page2.Construct(IDD_PAKFILES_MERGE);

	// add the pages
	dlg.AddPage(&page1);
	dlg.AddPage(&page2);

	// show the window
	dlg.DoModal();
}

void CNovacMasterProgramView::OnMenuFileCheckPakFile(){
	Dialogs::CPakFileInspector dialog;
	dialog.DoModal();
}

void CNovacMasterProgramView::OnMenuConfigurationFileTransfer()
{
	Dialogs::CFileTransferDlg dialog;
	dialog.DoModal();
}

void CNovacMasterProgramView::OnMenuAnalysisSetup()
{
	Dialogs::CGeometryDlg geomDlg;
	geomDlg.DoModal();
}

void CNovacMasterProgramView::OnMenuAnalysisWind()
{
	Dialogs::CPostWindDlg windDlg;
	windDlg.DoModal();
}

void CNovacMasterProgramView::UploadAuxData(){
	static int lastUploadDate = 0;
	CString fileName[52];
	int fileNameIndex = 0;
	CString	cfgFileName, observatoryStr, dateStr, timeStr;

	int todaysDate	= Common::GetDay();

	// if todays date is dividable by 7 and if we haven't uploaded anything
	//	today, then try to upload the files. (i.e. uploads will only occur
	//	arround once a week, maximum)
	if((todaysDate != lastUploadDate) && (todaysDate % 7 == 0)){
		int nVolcanoes = 0;
		int monitoredVolcanoes[64]; // a list of the volcanoes that we're monitoring

		dateStr.Format("%02d%02d%02d", m_common.GetYear(), m_common.GetMonth(), m_common.GetDay()); // current day
		timeStr.Format("%02d%02d", m_common.GetHour(), m_common.GetMinute());	// current time
		unsigned int it;
		for(it = 0; it < g_settings.scannerNum; ++it){
			// The volcano that this instrument is monitoring
			int thisVolcano = Common::GetMonitoredVolcano(g_settings.scanner[it].spec[0].serialNumber);

			// If not in the list of volcanoes, then add this one to the list
			bool found = false;
			for(int k = 0; k < nVolcanoes; ++k){
				if(monitoredVolcanoes[k] == thisVolcano){
					found = true;
				}
			}
			if(!found){
				monitoredVolcanoes[nVolcanoes++] = thisVolcano;
			}

			// Upload the reference-files for this spectrometer
			for(int j = 0; j < g_settings.scanner[it].spec[0].channelNum; ++j){
				for(int k = 0; k < g_settings.scanner[it].spec[0].channel[j].fitWindow.nRef; ++k){

					// make a copy of the reference-file
					fileName[fileNameIndex].Format("%sTemp\\%s_%1d_%s_%s_%s.txt", 
						(LPCTSTR)g_settings.outputDirectory, 
						(LPCTSTR)g_settings.scanner[it].spec[0].serialNumber,
						j, 
						g_settings.scanner[it].spec[0].channel[j].fitWindow.ref[k].m_specieName.c_str(),
						(LPCTSTR)dateStr,
						(LPCTSTR)timeStr);
					CopyFile(CString(g_settings.scanner[it].spec[0].channel[j].fitWindow.ref[k].m_path.c_str()), fileName[fileNameIndex], false);

					// upload the copy of the reference-file
					UploadToNOVACServer(fileName[fileNameIndex++], thisVolcano, true);
				}
			}
		}

		// Also send the configuration.xml - file !
		for(int k = 0; k < nVolcanoes; ++k){
			int thisVolcano = monitoredVolcanoes[k];
			observatoryStr.Format("%s", (LPCTSTR)m_common.SimplifyString(g_settings.scanner[0].observatory));
			cfgFileName.Format("%sconfiguration.xml", (LPCTSTR)m_common.m_exePath);
			fileName[fileNameIndex].Format("%sTemp\\%s\\%s_%s_%s_conf.xml", 
				(LPCTSTR)g_settings.outputDirectory, 
				(LPCTSTR)g_settings.scanner[it].spec[k].serialNumber, 
				(LPCTSTR)observatoryStr, 
				(LPCTSTR)dateStr, 
				(LPCTSTR)timeStr);
			CopyFile(cfgFileName, fileName[fileNameIndex], false);
			UploadToNOVACServer(fileName[fileNameIndex++], thisVolcano, true);
		}

		lastUploadDate = todaysDate;
	}// endif
}
void CNovacMasterProgramView::OnUpdateMenuViewInstrumenttab(CCmdUI *pCmdUI)
{
	if(m_instrumentViewVisible)
		pCmdUI->SetCheck(1);
	else
		pCmdUI->SetCheck(0);
}

void CNovacMasterProgramView::OnChangeUnitOfFluxToKgS(){
	g_userSettings.m_fluxUnit = UNIT_KGS;
	g_userSettings.WriteToFile();

	// Re-draw the graphs
	if(m_overView->m_hWnd != NULL){
		m_overView->DrawFlux();
		return;
	}
	for(int i = 0; i < m_scannerPages.GetCount(); ++i){
		CView_Scanner *page = (CView_Scanner *)m_scannerPages[i];
		if(page->m_hWnd != NULL){
			page->DrawFlux();
			return;
		}
	}
}
void CNovacMasterProgramView::OnChangeUnitOfFluxToTonDay(){
	g_userSettings.m_fluxUnit = UNIT_TONDAY;
	g_userSettings.WriteToFile();

	// Re-draw the graphs
	if(m_overView->m_hWnd != NULL){
		m_overView->DrawFlux();
		return;
	}
	for(int i = 0; i < m_scannerPages.GetCount(); ++i){
		CView_Scanner *page = (CView_Scanner *)m_scannerPages[i];
		if(page->m_hWnd != NULL){
			page->DrawFlux();
			return;
		}
	}
}

void CNovacMasterProgramView::OnUpdateChangeUnitOfFluxToKgS(CCmdUI *pCmdUI)
{
	if(g_userSettings.m_fluxUnit == UNIT_KGS)
		pCmdUI->SetCheck(1);
	else
		pCmdUI->SetCheck(0);
}

void CNovacMasterProgramView::OnUpdateChangeUnitOfFluxToTonDay(CCmdUI *pCmdUI)
{
	if(g_userSettings.m_fluxUnit == UNIT_TONDAY)
		pCmdUI->SetCheck(1);
	else
		pCmdUI->SetCheck(0);
}

void CNovacMasterProgramView::OnChangeUnitOfColumnToPPMM(){
	g_userSettings.m_columnUnit = UNIT_PPMM;
	g_userSettings.WriteToFile();

	for(int i = 0; i < m_scannerPages.GetCount(); ++i){
		CView_Scanner *page = (CView_Scanner *)m_scannerPages[i];
		if(page->m_hWnd != NULL){
			page->DrawColumn();
		}
	}
	for (int i = 0; i < m_colHistoryPages.GetCount(); ++i) {
		ColumnHistoryDlg *page = (ColumnHistoryDlg *)m_colHistoryPages[i];
		if (page->m_hWnd != NULL) {
			page->RedrawAll();
		}
	}
}

void CNovacMasterProgramView::OnChangeUnitOfColumnToMolecCm2(){
	g_userSettings.m_columnUnit = UNIT_MOLEC_CM2;
	g_userSettings.WriteToFile();

	for(int i = 0; i < m_scannerPages.GetCount(); ++i){
		CView_Scanner *page = (CView_Scanner *)m_scannerPages[i];
		if(page->m_hWnd != NULL){
			page->DrawColumn();
		}
	}

	for (int i = 0; i < m_colHistoryPages.GetCount(); ++i) {
		ColumnHistoryDlg *page = (ColumnHistoryDlg *)m_colHistoryPages[i];
		if (page->m_hWnd != NULL) {
			page->RedrawAll();
		}
	}
}

void CNovacMasterProgramView::OnUpdateChangeUnitOfColumnToPPMM(CCmdUI *pCmdUI)
{
	if(g_userSettings.m_columnUnit == UNIT_PPMM)
		pCmdUI->SetCheck(1);
	else
		pCmdUI->SetCheck(0);
}

void CNovacMasterProgramView::OnUpdateChangeUnitOfColumnToMolecCm2(CCmdUI *pCmdUI)
{
	if(g_userSettings.m_columnUnit == UNIT_MOLEC_CM2)
		pCmdUI->SetCheck(1);
	else
		pCmdUI->SetCheck(0);
}

void CNovacMasterProgramView::ScanStatusLogFile(){
	CString fileName;
	const int BUFFER_SIZE = 16384;
	char *szLine = new char[BUFFER_SIZE];
	CString serial;
	double linkSpeed;
	int dHour, dMinute, dSecond;
	CDateTime today, timeOfDownload;
	unsigned int nDownloadsFound = 0;

	// Get todays date...
	today.SetToNow();

	// The file-name of today's status-log file, if any...
	fileName.Format("%sOutput\\%04d.%02d.%02d\\StatusLog.txt", (LPCTSTR)g_settings.outputDirectory, today.year, today.month, today.day);

	// Try to open the status-log file
	FILE *f = fopen(fileName, "r");
	if(NULL == f){
		delete[] szLine;
		return; // could not open file, skip it...
	}

	// Read the file, one line at a time
	while(fgets(szLine, BUFFER_SIZE, f)){

		// look for lines which tells us download-speeds...
		if(strlen(szLine) > 30 && NULL != strstr(szLine, "Finished downloading file ")){
			// parse the download-speed
			char *pt1	= strstr(szLine, " from ");
			char *pt2	= strstr(szLine, "@");
			int nFound			= 0;
			size_t lineLength	= strlen(szLine);
			if(pt1 != NULL && pt2 != NULL){
				*(pt2 - 1) = '\0'; // make an end of string just before the '@'
				serial.Format(pt1 + 6);
				nFound += sscanf(pt2 + 2, "%lf",	&linkSpeed);
				nFound += sscanf(&szLine[lineLength - 9], "%02d:%02d:%02d", &dHour, &dMinute, &dSecond);
				if(nFound == 4){
					timeOfDownload.hour		= dHour;
					timeOfDownload.minute	= dMinute;
					timeOfDownload.second	= dSecond;
					m_commDataStorage->AddDownloadData(serial, linkSpeed, &timeOfDownload);

					++nDownloadsFound;
				}
			}
		}

		// also look for lines which tells us upload-speeds to the FTP-Server...
		if(strlen(szLine) > 30 && NULL != strstr(szLine, "Finished uploading file to FTP-Server")){
			// parse the download-speed
			char *pt					= strstr(szLine, "@");
			int nFound				= 0;
			size_t lineLength	= strlen(szLine);
			if(pt != NULL){
				nFound += sscanf(pt + 2, "%lf",	&linkSpeed);
				nFound += sscanf(&szLine[lineLength - 9], "%02d:%02d:%02d", &dHour, &dMinute, &dSecond);
				if(nFound == 4){
					timeOfDownload.hour		= dHour;
					timeOfDownload.minute	= dMinute;
					timeOfDownload.second	= dSecond;
					m_commDataStorage->AddDownloadData("FTP", linkSpeed, &timeOfDownload);
				}
			}
		}
	}

	// remember to close the file
	fclose(f);

	// clear up
	delete[] szLine;
}

LRESULT CNovacMasterProgramView::OnRewriteConfigurationXml(WPARAM wParam, LPARAM lParam){
	FileHandler::CConfigurationFileHandler writer;
	CString fileName, oldConfigurationFile, backupFile;

	// Lock this object to make sure that now one else tries to read
	//	data from this object while we are reading
	CSingleLock singleLock(&m_critSect);
	singleLock.Lock();

	if(singleLock.IsLocked()){

		// Write to a temporary file
		fileName.Format("%stemp_configuration.xml", (LPCTSTR)m_common.m_exePath);
		oldConfigurationFile.Format("%sconfiguration.xml", (LPCTSTR)m_common.m_exePath);
		backupFile.Format("%sconfiguration.bak", (LPCTSTR)m_common.m_exePath);

		if(0 == writer.WriteConfigurationFile(g_settings, &fileName)){
			// Make a backup of the old configuration-file
			if(CopyFile(oldConfigurationFile, backupFile, FALSE)){
				// Delete the old configuration-file
				if(DeleteFile(oldConfigurationFile)){
					// Rename the new configuration-file to 'configuration.xml'
					MoveFile(fileName, oldConfigurationFile);
				}
			}
		}
	
		// Remember to open up this object again
		singleLock.Unlock();
	}

	return 0;
}




void CNovacMasterProgramView::OnSize(UINT nType, int cx, int cy)
{
	// NOT WORKING 
	CFormView::OnSize(nType, cx, cy);
	if (IsWindow(this->m_hWnd)) {
		// move the main window
		//this->MoveWindow(0, 0, cx, cy);
		// Resize this window to the size of the main-window
		int screenHeight	= GetSystemMetrics(SM_CYSCREEN);
		int screenWidth		= GetSystemMetrics(SM_CXSCREEN);
		//this->MoveWindow(0,0,screenWidth, screenHeight);

		// move the tab sheet
		if (IsWindow(m_masterFrame.m_hWnd) && IsWindow(m_sheet.m_hWnd)) {
			CRect rect;
			m_masterFrame.GetWindowRect(rect);
			rect.left = rect.left + 10;
			rect.right = rect.right - 10;
			rect.top = rect.top + 15;
			rect.bottom = rect.bottom - 10;
			//m_sheet.MoveWindow(rect.left, rect.top, rect.Width(), rect.Height());

			//m_sheet.MoveWindow(rect.left, rect.top, cx, rect.Height());
			if (m_sheet.GetTabControl()) {
				//m_sheet.GetTabControl()->MoveWindow(rect.left, rect.top, cx, rect.Height());
				//m_sheet.GetActivePage()->MoveWindow(15, 35, width, height);
				/**
				int pageCount = m_sheet.GetPageCount();
				for (int i = 0; i < pageCount; i++) {
					CPropertyPage* page = m_sheet.GetPage(i);
					page->MoveWindow(15, 20, width, height);
				}
				*/
			}
		}
	}
}
