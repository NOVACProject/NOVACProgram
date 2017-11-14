#include "StdAfx.h"
#include "mastercontroller.h"

// Include synchronization classes
#include <afxmt.h>

// include the settings
#include "Configuration/Configuration.h"

// and all the threads that are needed
#include "WindMeasurement/WindEvaluator.h"
#include "Geometry/GeometryEvaluator.h"
#include "Evaluation/EvaluationController.h"
#include "Communication/CommunicationController.h"
#include "Communication/FTPServerContacter.h"
#include "WindFileController.h"
#include "Common/ReportWriter.h"

using namespace Evaluation;
using namespace Communication;
using namespace WindSpeedMeasurement;
using namespace Geometry;
using namespace FileHandler;

extern CConfigurationSetting g_settings;	// <-- The settings

/** The communication controller */
CWinThread *g_comm;

/** The evaluation thread */
CWinThread *g_eval;

/** The FTP-upload thread */
CWinThread *g_ftp;

/** The thread that evaluates the wind-measurements */
CWinThread *g_windMeas;

/** The thread that combines the scans into plume-height calculations */
CWinThread *g_geometry;

/** The thread that re-reads (and downloads) the wind-field file */
CWinThread *g_windFieldImport;

/** The Report-writing thread */
CWinThread *g_report;

// -------------- We also need to synchronize the threads somewhat -----------

/** This critical section tries to make sure that only one
      thread at a time tries to read from one of the small 
      evaluation-log - files */
CCriticalSection g_evalLogCritSect;

/** This function looks through the output directories and sees if there's any  
    old spectra there that should be evaluted. This function is only called at
    startup to make sure that there's no left-overs from previous runs of the
    program (if the program is exited while evaluating a pak-file, the pak file
    will still remain in the output directory) */
UINT CheckForOldSpectra(LPVOID pParam);
void CheckForSpectraInDir(const CString &path, CList <CString, CString&> &fileList);
void CheckForSpectraInHexDir(const CString &path, CList <CString, CString&> &fileList);
void SetThreadName(DWORD dwThreadID, LPCTSTR szThreadName) ;

#define MS_VC_EXCEPTION 0x406d1388 
typedef struct tagTHREADNAME_INFO { 
	DWORD dwType; // must be 0x1000 
	LPCSTR szName; // pointer to name (in same addr space) 
	DWORD dwThreadID; // thread ID (-1 caller thread) 
	DWORD dwFlags; // reserved for future use, most be zero 
} THREADNAME_INFO;

CMasterController::CMasterController(void)
{
	m_fRunning = false;
	g_comm = NULL;
	g_eval = NULL;
	g_windMeas = NULL;
	g_geometry = NULL;
	g_windFieldImport	= NULL;
	g_report = NULL;
}

CMasterController::~CMasterController(void)
{
}

void CMasterController::Start(){
	CString message;

	// Start by checking the settings in the program
	if(CheckSettings()){
		ShowMessage("Fail to start program. Please check settings and restart");
		return;
	}

	/** Start the FTP-uploading thread */
	g_ftp = AfxBeginThread(RUNTIME_CLASS(CFTPServerContacter), 
	         THREAD_PRIORITY_ABOVE_NORMAL, 0, 0, NULL);
	g_ftp->PostThreadMessage(WM_START_FTP,NULL,NULL);
	SetThreadName(g_ftp->m_nThreadID, "FTPUpload");

	/** Start the wind-field importin file */
	g_windFieldImport = AfxBeginThread(RUNTIME_CLASS(CWindFileController), 
	                     THREAD_PRIORITY_BELOW_NORMAL, 0, 0, NULL);
	SetThreadName(g_windFieldImport->m_nThreadID, "WindImp");

	/** Start the wind-measurement thread */
	g_windMeas = AfxBeginThread(RUNTIME_CLASS(CWindEvaluator), 
	             THREAD_PRIORITY_BELOW_NORMAL, 0, 0, NULL);
	SetThreadName(g_windMeas->m_nThreadID, "WindMeas");

	/** Start the geometry thread */
	g_geometry = AfxBeginThread(RUNTIME_CLASS(CGeometryEvaluator), 
	              THREAD_PRIORITY_BELOW_NORMAL, 0, 0, NULL);
	SetThreadName(g_geometry->m_nThreadID, "Geometry");

	/* start the communication thread */
	g_comm = AfxBeginThread(RUNTIME_CLASS(CCommunicationController), 
	          THREAD_PRIORITY_NORMAL,0,0,NULL);
	SetThreadName(g_comm->m_nThreadID, "Comm");
	Sleep(1000);

	/* start the evaluation thread */
	g_eval = AfxBeginThread(RUNTIME_CLASS(CEvaluationController), 
	          THREAD_PRIORITY_NORMAL, 0,0,NULL);
	SetThreadName(g_eval->m_nThreadID, "Eval");

	/* start the report-writing thread */
	g_report = AfxBeginThread(RUNTIME_CLASS(CReportWriter), 
	            THREAD_PRIORITY_NORMAL, 0,0,NULL);
	SetThreadName(g_report->m_nThreadID, "Report");

	m_fRunning = true;

	// Wait a little while, to give time for the threads to start
	Sleep(500);

	// Check if there's any old pak-files lying around that should be taken care of
	AfxBeginThread(CheckForOldSpectra, NULL, THREAD_PRIORITY_NORMAL, 0, 0, NULL);

	message.Format("%s. Compile date: %s", m_common.GetString(MSG_PROGRAM_STARTED_SUCESSFULLY), __DATE__);
	ShowMessage(message);
}

void CMasterController::Stop(){
	/** Stop the evaluation and wind-speed correlation thread */
	if(m_fRunning){
		g_eval->PostThreadMessage(WM_QUIT, NULL, NULL);
		::WaitForSingleObject(g_eval, INFINITE);

		g_windMeas->PostThreadMessage(WM_QUIT, NULL, NULL);
		::WaitForSingleObject(g_windMeas, INFINITE);

		if(g_geometry != NULL){
			g_geometry->PostThreadMessage(WM_QUIT, NULL, NULL);
			::WaitForSingleObject(g_geometry, INFINITE);
		}

		if(g_ftp != NULL){
			g_ftp->PostThreadMessage(WM_QUIT, NULL, NULL);
			::WaitForSingleObject(g_ftp, INFINITE);
		}

		if(g_report != NULL){
			g_report->PostThreadMessage(WM_QUIT, NULL, NULL);
			::WaitForSingleObject(g_report, INFINITE);
		}
	}

	m_fRunning = false;
}

UINT CheckForOldSpectra(LPVOID pParam){
	CString path, serial;
	Common common;
	common.GetExePath();
	CList	<CString, CString &> fileNames;

	// 1. check for spectra in the output\\temp - directory
	if(strlen(g_settings.outputDirectory) == 0){
		path.Format("%sTemp", common.m_exePath);
	}else{
		path.Format("%sTemp", g_settings.outputDirectory);
	}
	CheckForSpectraInDir(path, fileNames);

	// 2. Check for spectra in the output\\temp\\SERIAL - directory(-ies)
	for(unsigned int i = 0; i < g_settings.scannerNum; ++i){
		serial.Format(g_settings.scanner[i].spec[0].serialNumber);

		if(strlen(g_settings.outputDirectory) == 0){
			path.Format("%sTemp\\%s", common.m_exePath, serial);
		}else{
			path.Format("%sTemp\\%s", g_settings.outputDirectory, serial);
		}
		CheckForSpectraInDir(path, fileNames);
	}

	// 3. Check for spectra in the output\\temp\\RXYZ - directory(-ies)
	if(strlen(g_settings.outputDirectory) == 0){
		path.Format("%sTemp\\", common.m_exePath);
	}else{
		path.Format("%sTemp\\", g_settings.outputDirectory);
	}
	CheckForSpectraInHexDir(path, fileNames);

	// 4. Go through all the spectrum files found and evaluate them
	if(!fileNames.IsEmpty()){
		CPakFileHandler *pakFileHandler = new CPakFileHandler();
		POSITION pos = fileNames.GetHeadPosition();
		while(pos != NULL){
			CString &fn = fileNames.GetNext(pos);
			if(IsExistingFile(fn)){
				pakFileHandler->ReadDownloadedFile(fn);
			}
			
			Sleep(1000);
		}
	}

	return 0;
}

void CheckForSpectraInDir(const CString &path, CList <CString, CString&> &fileList){
	WIN32_FIND_DATA FindFileData;
	char fileToFind[MAX_PATH];

	// Find all .pak-files in the specified directory

	sprintf(fileToFind, "%s\\?????????.pak", path);

	// Search for the file
	HANDLE hFile = FindFirstFile(fileToFind, &FindFileData);

	if(hFile == INVALID_HANDLE_VALUE)
		return; // no files found

	do{
		CString fileName;
		fileName.Format("%s\\%s", path, FindFileData.cFileName);

		if(!Equals(FindFileData.cFileName, "Upload.pak")){
			// Tell the user that we've found one scan which hasn't been evaluated
			ShowMessage(_T("Found spectra which are not evaluated. Will evaluate them now."));

			// Append the found file to the list of files to split and evaluate...
			fileList.AddTail(fileName);
		}

	}while(0 != FindNextFile(hFile, &FindFileData));

	FindClose(hFile);

	return;
}

/** Checks the settings */
bool CMasterController::CheckSettings(){
	unsigned int i, j;
	int k, n;
	CString message;
	FILE *f  = NULL;

	// Test that there's something configured
	if(g_settings.scannerNum <= 0){
		MessageBox(NULL, "No scanners are configured, please use the configuration dialog to configure the instruments and restart the program", "Error in configuration", MB_OK);
		return true;
	}

	// Test the reference files
	for(i = 0; i < g_settings.scannerNum; ++i){
		for(j = 0; j < g_settings.scanner[i].specNum; ++j){
			for(k  = 0; k < g_settings.scanner[i].spec[j].channelNum; ++k){
				Evaluation::CFitWindow *window = &g_settings.scanner[i].spec[j].channel[k].fitWindow;
				for(n  = 0; n < window->nRef; ++n){
					f  =  fopen(window->ref[n].m_path, "r");
					if(NULL == f){
						message.Format("Cannot read reference file: %s", window->ref[n].m_path);
						MessageBox(NULL, message, "Error in settings", MB_OK);
						return true;
					}
					fclose(f);
				}
			}
		}
	}

	return false;
}

void CheckForSpectraInHexDir(const CString &path, CList <CString, CString&> &fileList){
	WIN32_FIND_DATA FindFileData;
	char fileToFind[MAX_PATH];
	CList <CString, CString &> pathList;
	CString pathName;

	// Find all RXYZ - directories...
	sprintf(fileToFind, "%s\\R???", path);

	// Search for the directories
	HANDLE hFile = FindFirstFile(fileToFind, &FindFileData);

	if(hFile == INVALID_HANDLE_VALUE)
		return; // no directories found
	do{
		pathName.Format("%s\\%s", path, FindFileData.cFileName);

		if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
			// This is a directory, add it to the list of directories to check...

			pathList.AddTail(pathName);
		}
	}while(0 != FindNextFile(hFile, &FindFileData));

	FindClose(hFile);

	// Check each of the directories found...
	POSITION pos = pathList.GetHeadPosition();
	while(pos != NULL){
		pathName.Format(pathList.GetNext(pos));

		// Check the directory...
		CheckForSpectraInDir(pathName, fileList);
	}

	return;
}


void SetThreadName(DWORD dwThreadID, LPCTSTR szThreadName) { 
	THREADNAME_INFO info; 
	info.dwType = 0x1000; 
	info.szName = szThreadName; 
	info.dwThreadID = dwThreadID; 
	info.dwFlags = 0; 
	__try { 
		RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(DWORD), (DWORD *)&info); 
	} 
	__except (EXCEPTION_CONTINUE_EXECUTION){ 
	} 
} 