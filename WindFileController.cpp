#include "StdAfx.h"
#include "WindFileController.h"

// we also need the meterological data
#include "Meteorology/MeteorologicalData.h"

// ... and the settings
#include "Configuration/Configuration.h"

// ... and the FTP-communication handle
#include "Communication/IFTPDataUpload.h"

// Use warning level 4 here
#pragma warning(push, 4)


extern CConfigurationSetting g_settings;    // <-- the settings
extern CMeteorologicalData g_metData;       // <-- The meteorological data
extern CFormView *pView;                    // <-- the main window

IMPLEMENT_DYNCREATE(CWindFileController, CWinThread)

BEGIN_MESSAGE_MAP(CWindFileController, CWinThread)
    ON_THREAD_MESSAGE(WM_QUIT, OnQuit)
    ON_THREAD_MESSAGE(WM_TIMER, OnTimer)
END_MESSAGE_MAP()

CWindFileController::CWindFileController(void)
 : m_nTimerID(0)
{
}

/** Quits the thread */
void CWindFileController::OnQuit(WPARAM /*wp*/, LPARAM /*lp*/)
{
}

void CWindFileController::OnTimer(UINT /*nIDEvent*/, LPARAM /*lp*/)
{
    ReadWindFieldFile();
}

void CWindFileController::ReadWindFieldFile()
{
    // Try to find and read in a wind-field file, if any can be found...
    if (g_settings.windSourceSettings.enabled == 1 && 
        g_settings.windSourceSettings.windFieldFile.GetLength() > 0)
    {
        // If the file is a local file...
        if (IsExistingFile(g_settings.windSourceSettings.windFieldFile))
        {
            if (0 == g_metData.ReadWindFieldFromFile(g_settings.windSourceSettings.windFieldFile))
            {
                ShowMessage("Successfully re-read wind-field from file");
                pView->PostMessage(WM_NEW_WINDFIELD, NULL, NULL);
            }
            return; // SUCCESS!!
        }

        // If the file exists on an FTP-server
        if (Equals(g_settings.windSourceSettings.windFieldFile.Left(6), "ftp://")
            || Equals(g_settings.windSourceSettings.windFieldFile.Left(7), "sftp://"))
        {
            DownloadFileByFTP();
        }
    }
}

BOOL CWindFileController::OnIdle(LONG /*lCount*/)
{
    return FALSE; // no more idle-time is needed
}

BOOL CWindFileController::InitInstance()
{
    CWinThread::InitInstance();

    // Check the global settings if we are to be re-loading the wind-field file
    //	every now and then then set the timer
    if (g_settings.windSourceSettings.enabled == 1 && 
        g_settings.windSourceSettings.windFileReloadInterval > 0 && 
        g_settings.windSourceSettings.windFieldFile.GetLength() > 3) {

        // Read the log-file now
        ReadWindFieldFile();

        // Set a timer to wake up with the given interval
        int nmSeconds = g_settings.windSourceSettings.windFileReloadInterval * 60 * 1000;
        m_nTimerID = ::SetTimer(NULL, 0, nmSeconds, NULL);
    }
    else
    {
        // If we should not re-read the wind-field file then this thread has
        //	nothing to do. We can just as well quit it
        AfxEndThread(0);
    }

    return 1;
}

int CWindFileController::ExitInstance()
{
    if (0 != m_nTimerID)
    {
        ::KillTimer(NULL, m_nTimerID);
    }

    return CWinThread::ExitInstance();
}

CString RemoveProtocol(const CString& fileName)
{
    const int initialLength = fileName.GetLength();

    if (Equals(fileName.Left(7), "sftp://"))
    {
        return fileName.Right(initialLength - 7);
    }
    else
    {
        return fileName.Right(initialLength - 6);
    }
}

void CWindFileController::DownloadFileByFTP()
{
    // remove the protocol-prefix ('ftp://' or 'sftp://')
    CString fileName = RemoveProtocol(g_settings.windSourceSettings.windFieldFile);
    
    int firstSlash = fileName.FindOneOf("/\\");
    int remainingFileNameLength = fileName.GetLength();

    // 0. Checks!!
    if (remainingFileNameLength <= 0)
    {
        return;
    }

    // 1. Parse the IP-number
    const CString ipNumber = fileName.Left(firstSlash);

    // 2. If the IP-number is the same as to the FTP-server then we already know the login and password
    if (!Equals(ipNumber, g_settings.ftpSetting.ftpAddress))
    {
        return; // unknown login + pwd...
    }
    const CString userName = g_settings.ftpSetting.userName;
    const CString password = g_settings.ftpSetting.password;

    // 3. Setup the FTP-communication handler
    std::unique_ptr<Communication::IFTPDataUpload> ftp = Communication::IFTPDataUpload::Create(g_settings.ftpSetting.protocol);

    // 4. Connect!
    if (ftp->Connect(ipNumber, userName, password, TRUE) != 1)
    {
        return; // fail
    }

    // 5. If necessary then enter a directory
    fileName.Format(g_settings.windSourceSettings.windFieldFile.Right(remainingFileNameLength - firstSlash - 1));
    remainingFileNameLength = fileName.GetLength();
    firstSlash = fileName.FindOneOf("/\\");
    while (-1 != firstSlash)
    {
        const CString directoryName = fileName.Left(firstSlash);

        // Enter the directory
        if (0 == ftp->SetCurDirectory(directoryName))
        {
            ftp->Disconnect();
            return;
        }

        Sleep(500);

        // Remove the directory-name from the file-name
        fileName.Format(fileName.Right(remainingFileNameLength - firstSlash - 1));
        remainingFileNameLength = fileName.GetLength();
        firstSlash = fileName.FindOneOf("/\\");
    }

    // 6. Download the file!
    CString localFileName;
    localFileName.Format("%sTemp\\WindField.txt", (LPCTSTR)g_settings.outputDirectory);
    if (IsExistingFile(localFileName))
    {
        if (0 == DeleteFile(localFileName))
        {
            ftp->Disconnect();
            return;
        }
    }

    if (0 == ftp->DownloadAFile(fileName, localFileName))
    {
        ftp->Disconnect();
        return;
    }

    CString msg;
    msg.Format("Downloaded %s", (LPCTSTR)g_settings.windSourceSettings.windFieldFile);
    ShowMessage(msg);

    // 7. Disconnect!
    ftp->Disconnect();

    // 8. Parse the file!
    if (0 == g_metData.ReadWindFieldFromFile(localFileName))
    {
        ShowMessage("Successfully re-read wind-field from file");
        pView->PostMessage(WM_NEW_WINDFIELD, NULL, NULL);
    }
}