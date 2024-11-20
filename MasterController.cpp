#include "StdAfx.h"
#include "MasterController.h"
#include "Common/Common.h"
#include "NovacProgramLog.h"

#ifdef _MSC_VER
#pragma warning (push, 4)
#endif

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
CWinThread* g_comm;

/** The evaluation thread */
CWinThread* g_eval;

/** The FTP-upload thread */
CWinThread* g_ftp;

/** The thread that evaluates the wind-measurements */
CWinThread* g_windMeas;

/** The thread that combines the scans into plume-height calculations */
CWinThread* g_geometry;

/** The thread that re-reads (and downloads) the wind-field file */
CWinThread* g_windFieldImport;

/** The Report-writing thread */
CWinThread* g_report;

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

void SetThreadName(DWORD dwThreadID, LPCTSTR szThreadName);

#define MS_VC_EXCEPTION 0x406d1388 
typedef struct tagTHREADNAME_INFO
{
    DWORD dwType; // must be 0x1000 
    LPCSTR szName; // pointer to name (in same addr space) 
    DWORD dwThreadID; // thread ID (-1 caller thread) 
    DWORD dwFlags; // reserved for future use, most be zero 
} THREADNAME_INFO;

CMasterController::CMasterController()
{
    g_comm = nullptr;
    g_eval = nullptr;
    g_windMeas = nullptr;
    g_geometry = nullptr;
    g_windFieldImport = nullptr;
    g_report = nullptr;
}

void CMasterController::Start()
{
    CString message;

    // Start by checking the settings in the program
    if (CheckSettings())
    {
        ShowMessage("Fail to start program. Please check settings and restart");
        return;
    }

    /** Start the FTP-uploading thread */
    g_ftp = AfxBeginThread(RUNTIME_CLASS(CFTPServerContacter),
        THREAD_PRIORITY_ABOVE_NORMAL, 0, 0, nullptr);
    g_ftp->PostThreadMessage(WM_START_FTP, NULL, NULL);
    SetThreadName(g_ftp->m_nThreadID, "FTPUpload");

    /** Start the wind-field importin file */
    g_windFieldImport = AfxBeginThread(RUNTIME_CLASS(CWindFileController),
        THREAD_PRIORITY_BELOW_NORMAL, 0, 0, nullptr);
    SetThreadName(g_windFieldImport->m_nThreadID, "WindImp");

    /** Start the wind-measurement thread */
    g_windMeas = AfxBeginThread(RUNTIME_CLASS(CWindEvaluator),
        THREAD_PRIORITY_BELOW_NORMAL, 0, 0, nullptr);
    SetThreadName(g_windMeas->m_nThreadID, "WindMeas");

    /** Start the geometry thread */
    g_geometry = AfxBeginThread(RUNTIME_CLASS(CGeometryEvaluator),
        THREAD_PRIORITY_BELOW_NORMAL, 0, 0, nullptr);
    SetThreadName(g_geometry->m_nThreadID, "Geometry");

    /* start the communication thread */
    g_comm = AfxBeginThread(RUNTIME_CLASS(CCommunicationController),
        THREAD_PRIORITY_NORMAL, 0, 0, nullptr);
    SetThreadName(g_comm->m_nThreadID, "Comm");
    Sleep(1000);

    /* start the evaluation thread */
    g_eval = AfxBeginThread(RUNTIME_CLASS(CEvaluationController),
        THREAD_PRIORITY_NORMAL, 0, 0, nullptr);
    SetThreadName(g_eval->m_nThreadID, "Eval");

    /* start the report-writing thread */
    g_report = AfxBeginThread(RUNTIME_CLASS(CReportWriter),
        THREAD_PRIORITY_NORMAL, 0, 0, nullptr);
    SetThreadName(g_report->m_nThreadID, "Report");

    m_fRunning = true;

    // Wait a little while, to give time for the threads to start
    Sleep(500);

    // Check if there's any old pak-files lying around that should be taken care of
    AfxBeginThread(CheckForOldSpectra, nullptr, THREAD_PRIORITY_NORMAL, 0, 0, nullptr);

    Common common;
    message.Format("%s. Compile date: %s", (LPCTSTR)common.GetString(MSG_PROGRAM_STARTED_SUCESSFULLY), __DATE__);
    ShowMessage(message);
}

void CMasterController::Stop()
{
    /** Stop the evaluation and wind-speed correlation thread */
    if (m_fRunning)
    {
        g_eval->PostThreadMessage(WM_QUIT, NULL, NULL);
        ::WaitForSingleObject(g_eval, INFINITE);

        g_windMeas->PostThreadMessage(WM_QUIT, NULL, NULL);
        ::WaitForSingleObject(g_windMeas, INFINITE);

        if (g_geometry != nullptr)
        {
            g_geometry->PostThreadMessage(WM_QUIT, NULL, NULL);
            ::WaitForSingleObject(g_geometry, INFINITE);
        }

        if (g_ftp != nullptr)
        {
            g_ftp->PostThreadMessage(WM_QUIT, NULL, NULL);
            ::WaitForSingleObject(g_ftp, INFINITE);
        }

        if (g_report != nullptr)
        {
            g_report->PostThreadMessage(WM_QUIT, NULL, NULL);
            ::WaitForSingleObject(g_report, INFINITE);
        }
    }

    m_fRunning = false;
}

UINT CheckForOldSpectra(LPVOID /*pParam*/)
{
    CList <CString, CString&> pakFilesToEvaluate;
    Common common;

    // 1. check for spectra in the output\\temp - directory
    CString baseDirectory;
    if (strlen(g_settings.outputDirectory) == 0)
    {
        baseDirectory.Format("%sTemp\\", (LPCTSTR)common.m_exePath);
    }
    else
    {
        baseDirectory.Format("%sTemp\\", (LPCTSTR)g_settings.outputDirectory);
    }
    Common::CheckForSpectraInDir(baseDirectory, pakFilesToEvaluate);

    // 2. Check for spectra in the output\\temp\\SERIAL - directory(-ies)
    for (unsigned int i = 0; i < g_settings.scannerNum; ++i)
    {
        CString path;
        path.Format("%s%s", (LPCTSTR)baseDirectory, (LPCTSTR)g_settings.scanner[i].spec[0].serialNumber);

        Common::CheckForSpectraInDir(path, pakFilesToEvaluate);
    }

    // 3. Check for spectra in the output\\temp\\RXYZ - directory(-ies)
    {
        // TODO: Figure out why this is identical to the path we have already checked above!
        CString path;
        path.Format("%s", (LPCTSTR)baseDirectory);
        Common::CheckForSpectraInDir(path, pakFilesToEvaluate);
    }

    NovacProgramLog log;

    // 4. Go through all the spectrum files found and evaluate them
    if (!pakFilesToEvaluate.IsEmpty())
    {
        CPakFileHandler pakFileHandler(log);
        POSITION pos = pakFilesToEvaluate.GetHeadPosition();
        while (pos != nullptr)
        {
            CString& fn = pakFilesToEvaluate.GetNext(pos);
            if (IsExistingFile(fn))
            {
                pakFileHandler.ReadDownloadedFile(fn);
            }

            Sleep(1000);
        }
    }

    return 0;
}

bool CMasterController::CheckSettings()
{
    if (g_settings.scannerNum <= 0)
    {
        MessageBox(NULL, "No scanners are configured, please use the configuration dialog to configure the instruments and restart the program", "Error in configuration", MB_OK);
        return true;
    }

    // Test the reference files
    for (unsigned long i = 0; i < g_settings.scannerNum; ++i)
    {
        for (unsigned long j = 0; j < g_settings.scanner[i].specNum; ++j)
        {
            for (int k = 0; k < g_settings.scanner[i].spec[j].channelNum; ++k)
            {
                const novac::CFitWindow* window = &g_settings.scanner[i].spec[j].channel[k].fitWindow;
                for(const auto& reference : window->reference)
                {
                    FILE* f = fopen(reference.m_path.c_str(), "r");
                    if (nullptr == f)
                    {
                        CString message;
                        message.Format("Cannot read reference file: %s", reference.m_path.c_str());
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

void SetThreadName(DWORD dwThreadID, LPCTSTR szThreadName)
{
    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = szThreadName;
    info.dwThreadID = dwThreadID;
    info.dwFlags = 0;
    __try
    {
        RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(DWORD), (ULONG_PTR*)&info);
    }
    __except (EXCEPTION_CONTINUE_EXECUTION)
    {
    }
}

#ifdef _MSC_VER
#pragma warning (pop)
#endif
