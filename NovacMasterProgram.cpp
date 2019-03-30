// NovacMasterProgram.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "NovacMasterProgram.h"
#include "MainFrm.h"

#include "NovacMasterProgramDoc.h"
#include "NovacMasterProgramView.h"

#include "Evaluation/EvaluationController.h"
#include "UserSettings.h"

#include <curl/curl.h>
#include <assert.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern CUserSettings g_userSettings;       // <-- The users preferences
// CNovacMasterProgramApp

BEGIN_MESSAGE_MAP(CNovacMasterProgramApp, CWinApp)
    ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
    // Standard file based document commands
    ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
    ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()


// CNovacMasterProgramApp construction

CNovacMasterProgramApp::CNovacMasterProgramApp()
{
    // Place all significant initialization in InitInstance
}


// The one and only CNovacMasterProgramApp object

CNovacMasterProgramApp theApp;

// CNovacMasterProgramApp initialization

BOOL CNovacMasterProgramApp::InitInstance()
{
    // InitCommonControls() is required on Windows XP if an application
    // manifest specifies use of ComCtl32.dll version 6 or later to enable
    // visual styles.  Otherwise, any window creation will fail.
    InitCommonControls();

    CWinApp::InitInstance();

    /** ---------------- SET LANGUAGE ----------------------- */
    Common common;
    common.GetExePath();
    CString userSettingsFile;
    CString oldFileName = TEXT(common.m_exePath + "\\language.txt");

    // Read the user settings
    userSettingsFile.Format("%s\\user.ini", (LPCTSTR)common.m_exePath);
    g_userSettings.ReadSettings(&userSettingsFile);

    // this portion of code reading 'language.txt' is for
    //	backward compatibility only. Should eventually be removed...
    if (IsExistingFile(oldFileName)) {
        FILE *f = fopen(oldFileName, "r");
        if (f != 0) {
            char buffer[512];
            fgets(buffer, 512, f);
            if (0 == _tcsncicmp(buffer, "spanish", 7)) {
                g_userSettings.m_language = LANGUAGE_SPANISH;
            }
            else {
                g_userSettings.m_language = LANGUAGE_ENGLISH;
            }
            fclose(f);
            DeleteFile(oldFileName);
        }
    }

    switch (g_userSettings.m_language) {
    case LANGUAGE_ENGLISH:
        ::SetThreadLocale(MAKELCID(MAKELANGID(0x0409, SUBLANG_DEFAULT), SORT_DEFAULT));
        primaryLanguage = 0x0409;
        subLanguage = SUBLANG_DEFAULT;
        break;
    case LANGUAGE_SPANISH:
        ::SetThreadLocale(MAKELCID(MAKELANGID(0x0c0a, SUBLANG_DEFAULT), SORT_DEFAULT));
        primaryLanguage = 0x0c0a;
        subLanguage = SUBLANG_DEFAULT;
        break;
    default:
        ::SetThreadLocale(MAKELCID(MAKELANGID(0x0409, SUBLANG_DEFAULT), SORT_DEFAULT));
        primaryLanguage = 0x0409;
        subLanguage = SUBLANG_DEFAULT;
        break;
    }

    // Initialize OLE libraries
    if (!AfxOleInit())
    {
        AfxMessageBox(IDP_OLE_INIT_FAILED);
        return FALSE;
    }
    AfxEnableControlContainer();
    // Standard initialization
    // If you are not using these features and wish to reduce the size
    // of your final executable, you should remove from the following
    // the specific initialization routines you do not need
    // Change the registry key under which our settings are stored
    SetRegistryKey(_T("Chalmers University of Technology, Sweden."));
    LoadStdProfileSettings(4);  // Load standard INI file options (including MRU)
    // Register the application's document templates.  Document templates
    //  serve as the connection between documents, frame windows and views
    CSingleDocTemplate* pDocTemplate;

    pDocTemplate = new CSingleDocTemplate(
        IDR_MAINFRAME,
        RUNTIME_CLASS(CNovacMasterProgramDoc),
        RUNTIME_CLASS(CMainFrame),       // main SDI frame window
        RUNTIME_CLASS(CNovacMasterProgramView));
    AddDocTemplate(pDocTemplate);
    // Parse command line for standard shell commands, DDE, file open
    CCommandLineInfo cmdInfo;
    ParseCommandLine(cmdInfo);
    // Dispatch commands specified on the command line.  Will return FALSE if
    // app was launched with /RegServer, /Register, /Unregserver or /Unregister.
    if (!ProcessShellCommand(cmdInfo))
        return FALSE;
    // The one and only window has been initialized, so show and update it
    m_pMainWnd->SetWindowText("NovacProgram");

    // Get the resolution of the screen
    int cx = GetSystemMetrics(SM_CXSCREEN);
    int cy = GetSystemMetrics(SM_CYSCREEN);

    //m_pMainWnd->ShowWindow(SW_SHOWMAXIMIZED);
    // if(cx <= 1024){
    m_pMainWnd->ShowWindow(SW_SHOWMAXIMIZED);
    // }else{
    // 	int width = 1050;
    // 	int height = 750;
    // 	m_pMainWnd->ShowWindow(SW_SHOWNORMAL);
    // 	m_pMainWnd->MoveWindow((cx-width)/2, (cy-height)/2, width, height);
    // }

    // Initialize resources used by libCurl
    CURLcode returnCode = curl_global_init(CURL_GLOBAL_ALL);
    assert(returnCode == CURLE_OK);

    // call DragAcceptFiles only if there's a suffix
    //  In an SDI app, this should occur after ProcessShellCommand
    return TRUE;
}


BOOL CNovacMasterProgramApp::ExitInstance()
{
    // Release resources used by libCurl
    curl_global_cleanup();

    CWinApp::ExitInstance();

    return TRUE;
}


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
    CAboutDlg();

    // Dialog Data
    enum { IDD = IDD_ABOUTBOX };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CString copyright, version, credits;
    CDialog::DoDataExchange(pDX);

    version.Format("Novac Program Version %d.%d Built: %s", CVersion::majorNumber, CVersion::minorNumber, __DATE__);
    copyright.Format("Copyright (c) 2005 - %4d Optical Remote Sensing Group\n\nChalmers University of Technology, Sweden", Common::GetYear());
    credits.Format("Programming:\n  Yan Zhang\n  Mattias Johansson\n  Diana Norgaard");
    DDX_Text(pDX, IDC_STATIC_VERSIONNUMBER, version);
    DDX_Text(pDX, IDC_STATIC_COPYRIGHT, copyright);
    DDX_Text(pDX, IDC_STATIC_CREDITS, credits);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// App command to run the dialog
void CNovacMasterProgramApp::OnAppAbout()
{
    CAboutDlg aboutDlg;
    aboutDlg.DoModal();
}


// CNovacMasterProgramApp message handlers


BOOL CNovacMasterProgramApp::OnIdle(LONG lCount)
{
    return CWinApp::OnIdle(lCount);
}
