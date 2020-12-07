#include "StdAfx.h"
#include "FTPServerContacter.h"
#include "IFTPDataUpload.h"
#include "../Common/Common.h"
#include "../Configuration/configuration.h"
#include "../VolcanoInfo.h"

// Include synchronization classes
#include <afxmt.h>

// Use warning level 4 here
#pragma warning(push, 4)


using namespace Communication;

// Global variables;
extern CConfigurationSetting g_settings;   // <-- the settings
extern CVolcanoInfo g_volcanoes;           // <-- the list of volcanoes
extern CFormView *pView;                   // <-- the main window
extern CCriticalSection g_evalLogCritSect; // synchronization access to evaluation-log files

IMPLEMENT_DYNCREATE(CFTPServerContacter, CWinThread)

BEGIN_MESSAGE_MAP(CFTPServerContacter, CWinThread)
    ON_THREAD_MESSAGE(WM_START_FTP, OnStartFTP)
    ON_THREAD_MESSAGE(WM_UPLOAD_NEW_FILE, OnArrivedFile)
    ON_THREAD_MESSAGE(WM_QUIT, OnQuit)
    ON_THREAD_MESSAGE(WM_TIMER, OnTimer)
END_MESSAGE_MAP()

CFTPServerContacter::CFTPServerContacter()
{
    m_listLogFile.Format("%s\\Temp\\UploadFileList.txt", (LPCSTR)g_settings.outputDirectory);
    m_listLogFile_Temp.Format("%s\\Temp\\UploadFileList_Temp.txt", (LPCSTR)g_settings.outputDirectory);

    m_nTimerID = 0;
    m_hasReadInFileList = false;
    time(&m_lastExportTime);
}

CFTPServerContacter::~CFTPServerContacter()
{
    m_ftp.reset();

    ExportList();
}

/** Quits the thread */
void CFTPServerContacter::OnQuit(WPARAM /*wp*/, LPARAM /*lp*/)
{
    //Write log file of ftp quit. What need to be uploaded.
    //Close ftp connection
    m_ftp.reset();

    ExportList();
}

/**When the thread is started, copy the UploadFileList.txt's content into m_fileList*/
void CFTPServerContacter::OnStartFTP(WPARAM /*wp*/, LPARAM /*lp*/)
{
    if (!IsExistingFile(m_listLogFile)) {
        // If we have a temporary-file, then parse that instead
        if (IsExistingFile(m_listLogFile_Temp)) {
            ParseAFile(m_listLogFile_Temp);
        }
        m_hasReadInFileList = true;
        return;
    }
    ParseAFile(m_listLogFile);

    m_hasReadInFileList = true;
}

void CFTPServerContacter::OnTimer(UINT /*nIDEvent*/, LPARAM /*lp*/) {
    // Call the uploading thing...
    OnIdle(0);
}

/** This function takes care of newly arrived files, put into uploading list
and upload the earliest. */
void CFTPServerContacter::OnArrivedFile(WPARAM wp, LPARAM lp)
{
    // The filename of the newly arrived file is the first parameter 
    CString *fileName = (CString *)wp;

    // This index tells us which volcano that the file belongs to 
    FTPUploadOptions *options = (FTPUploadOptions *)lp;
    int volcanoIndex = options->volcanoIndex;

    // Check the inputs
    if (fileName == nullptr || fileName->GetLength() <= 4) {
        ShowMessage("ERROR: Received command to upload file which does not exist");
        return; // quit, error in input-data
    }
    if (volcanoIndex < 0 || volcanoIndex > g_volcanoes.m_volcanoNum) {
        ShowMessage("ERROR: Received command to upload file to non-existing volcano");
        return; // quit, error in input-data
    }

    // Check if the file already exists in the list, if so then don't add it
    POSITION listPos = m_fileList.GetHeadPosition();
    while (listPos != nullptr) {
        UploadFile &f = m_fileList.GetNext(listPos);

        if (Equals(f.fileName, *fileName) && (f.volcanoIndex == volcanoIndex)) {
            delete fileName;
            delete options;
            return; // don't add the file to the list
        }
    }

    // A new UploadFile - struct
    UploadFile file;
    file.fileName.Format(*fileName);
    file.volcanoIndex = volcanoIndex;
    file.deleteFile = options->deleteFile;

    //add the file name to the file list
    m_fileList.AddTail(file);

    // Save the list to be sure we don't loose anything
    ExportList();

    // Reset the string and the options structure to avoid memory leaks
    delete fileName;
    delete options;
}

BOOL CFTPServerContacter::OnIdle(LONG /*lCount*/) {
    CString localFile, remoteFile, volcano, message;
    CString dateText, timeText;
    double linkSpeed;
    int ret = 0;

    // High resolution counter
    LARGE_INTEGER lpFrequency, timingStart, timingStop;
    BOOL useHighResolutionCounter = QueryPerformanceFrequency(&lpFrequency);

    // Low resolution counter, used as a backup
    clock_t timing_Stop, timing_Start; // <-- timing of the upload speed

    if (m_fileList.GetCount() == 0)
    {
        if (m_hasReadInFileList) {
            DeleteFile(m_listLogFile);
            return FALSE; // don't need more idle time
        }
        else {
            OnStartFTP(NULL, NULL);
        }
    }

    // Check the settings for when we are to upload data
    // There are two options, either stopTime > startTime:
    //      ------------------------------------------------
    //      |           xxxxxxxxxxxxxxxxxxxxxxxxxx         |
    //      ------------^-------------------------^---------
    //               startTime                 stopTime
    // or startTime > stopTime:
    //      ------------------------------------------------
    //      |xxxxxxxxxxxx                         xxxxxxxxx|
    //      ------------^-------------------------^---------
    //               stopTime                 startTime

    CTime currentTime = CTime::GetCurrentTime();
    int now = currentTime.GetHour() * 3600 + currentTime.GetMinute() * 60 + currentTime.GetSecond();
    int startTime = g_settings.ftpSetting.ftpStartTime;
    int stopTime = g_settings.ftpSetting.ftpStopTime;
    if (stopTime > startTime) {
        if (now < startTime || now > stopTime) {
            return FALSE;	// no more idle-time needed
        }
    }
    else {
        if (now > stopTime && now < startTime) {
            return FALSE;	// no more idle-time needed
        }
    }

    // upload file since the latest
    if(!m_ftp)
    {
        m_ftp = IFTPDataUpload::Create(g_settings.ftpSetting.protocol);
    }

    if (m_ftp->Connect(g_settings.ftpSetting.ftpAddress, g_settings.ftpSetting.userName, g_settings.ftpSetting.password, 60, TRUE) == 1)
    {
        while (m_fileList.GetCount() > 0)
        {
            // The file to upload
            UploadFile &upload = m_fileList.GetTail();

            // The name of the file to upload
            localFile.Format("%s", (LPCSTR)upload.fileName);

            // Make sure that the file does exist...
            if (!IsExistingFile(localFile)) {
                m_fileList.RemoveTail();
                continue;
            }

            // The name of the volcano
            if (upload.volcanoIndex >= 0 && upload.volcanoIndex < g_volcanoes.m_volcanoNum) {
                volcano.Format("%s", (LPCSTR)g_volcanoes.m_simpleName[upload.volcanoIndex]);
            }
            else {
                volcano.Format("unknown");
            }

            // Change the current directory on the FTP-Server
            SetRemoteDirectory(volcano);

            // The name of the file on the remote ftp-server
            remoteFile.Format("%s", (LPCSTR)upload.fileName);
            Common::GetFileName(remoteFile);

            // Get the size of the file, to be able to calculate the size of the link
            const double fileSize_kB = Common::RetrieveFileSize(localFile) / 1024.0;

            // Make sure that no-one else is trying to access this file...
            CSingleLock singleLock(&g_evalLogCritSect);
            singleLock.Lock();

            if (singleLock.IsLocked()) {
                // Upload the file!
                timing_Start = clock(); // <-- timing...
                useHighResolutionCounter = QueryPerformanceCounter(&timingStart);
                ret = m_ftp->UpdateRemoteFile(localFile, remoteFile);
            }

            singleLock.Unlock();

            if (ret != 0)
            {
                useHighResolutionCounter = QueryPerformanceCounter(&timingStop);
                timing_Stop = clock();

                // Remember the speed of the upload
                double clocksPerSec = CLOCKS_PER_SEC;
                double elapsedTime = max(1.0 / clocksPerSec, (double)(timing_Stop - timing_Start) / clocksPerSec);
                double elapsedTime2 = ((double)timingStop.LowPart - (double)timingStart.LowPart) / (double)lpFrequency.LowPart;

                if (useHighResolutionCounter)
                    linkSpeed = fileSize_kB / elapsedTime;
                else
                    linkSpeed = fileSize_kB / elapsedTime2;

                m_linkStatistics.AppendDownloadSpeed(linkSpeed);

                // The file is uploaded!!
                if (upload.deleteFile) {
                    ::DeleteFile(localFile);
                }
                // remove the file from the list
                m_fileList.RemoveTail();

                // Tell the world!
                message.Format("Finished uploading file %s to %s @ %.1lf kB/s", (LPCSTR)remoteFile, g_settings.ftpSetting.ftpAddress, linkSpeed);
                ShowMessage(message);

                pView->PostMessage(WM_FINISH_UPLOAD, (WPARAM)linkSpeed);
            }
            else {
                // Failed to upload the file...
                ShowMessage(m_ftp->m_ErrorMsg);
                m_linkStatistics.AppendFailedUpload();
                break;
            }

            // Go to the directory two steps up
            m_ftp->SetCurDirectory("..");
            m_ftp->SetCurDirectory("..");
        }
        m_ftp->Disconnect();

        ExportList(); //export file list to log file UploadFileList.txt
    }

    return 0; // no more time is needed
}

void CFTPServerContacter::SetRemoteDirectory(const CString &volcanoName)
{
    CString dateText;
    Common::GetDateText(dateText);

    // The directory on the remote server
    CString rootDirectory;
    rootDirectory.Format("%s", (LPCSTR)volcanoName);

    // Create the volcano-directory and set it as the current directory
    m_ftp->CreateDirectory(rootDirectory);
    m_ftp->SetCurDirectory(rootDirectory);

    // Create the date sub-directory and set it as the current directory
    m_ftp->CreateDirectory(dateText);
    m_ftp->SetCurDirectory(dateText);
}

BOOL CFTPServerContacter::InitInstance() {
    CWinThread::InitInstance();

    // Set a timer to wake up every 10 minutes
    m_nTimerID = ::SetTimer(nullptr, 0, 10 * 60 * 1000, nullptr);

    return 1;
}

int Communication::CFTPServerContacter::ExitInstance()
{
    if (0 != m_nTimerID) {
        KillTimer(nullptr, m_nTimerID);
    }

    return CWinThread::ExitInstance();
}


bool CFTPServerContacter::ParseAFile(const CString& fileName)
{
    CStdioFile fileRef;
    CFileException exceFile;
    CString line, token;
    int lastVolcanoIndex = -1;
    bool duplicates = false; // true if there are duplicates in the list...


    if (!fileRef.Open(fileName, CFile::modeRead | CFile::typeText, &exceFile))
    {
        return false;
    }

    // Temporarily lower the priority of this thread. This since the
    //	reading in of very long file-lists can take quite some time...
    SetThreadPriority(THREAD_PRIORITY_LOWEST);

    while (fileRef.ReadString(line))
    {
        char* szToken = (char*)(LPCSTR)line;

        while (szToken = strtok(szToken, "\n"))
        {
            CString fileName, volcanoName;
            int			volcanoIndex, deleteFile;

            // find the tab which separates the file-name and the volcano index
            char *pt = strstr(szToken, "\t");
            if (pt != 0) {
                volcanoIndex = -1; // reset

                pt[0] = 0; // remove the tab
                fileName.Format("%s", szToken);

                // The rest of the string is; 
                // 1) the volcano-name, either as an index (old) or as a string (new)...
                // 2) the 'deleteFlag'
                volcanoName.Format("%s", pt + 1);
                int tabIndex = volcanoName.Find("\t");
                if (-1 != tabIndex) {
                    volcanoName = volcanoName.Left(tabIndex);

                    // First compare with the last volcano-index, to save some time...
                    if (lastVolcanoIndex >= 0 && Equals(g_volcanoes.m_simpleName[lastVolcanoIndex], volcanoName)) {
                        volcanoIndex = lastVolcanoIndex;
                    }
                    else {
                        for (unsigned int k = 0; k < g_volcanoes.m_volcanoNum; ++k) {
                            if (Equals(volcanoName, g_volcanoes.m_simpleName[k])) {
                                volcanoIndex = k;
                                lastVolcanoIndex = k;
                                break;
                            }
                        }
                    }

                    if (0 == sscanf(pt + 1 + tabIndex, "%d", &deleteFile)) {
                        deleteFile = 0;
                    }
                }

                if (volcanoIndex == -1) {
                    if (0 == sscanf(pt + 1, "%d\t", &volcanoIndex)) {
                        volcanoIndex = -1;
                        deleteFile = 0;
                    }
                }
            }
            else {
                fileName.Format(szToken);
                volcanoIndex = -1;
                deleteFile = 0;
            }

            // Check if the file already exists in the list, if so then don't add it
            bool isAlreadyInList = false;
            POSITION listPos = m_fileList.GetHeadPosition();
            while (listPos != nullptr) {
                UploadFile &f = m_fileList.GetNext(listPos);
                if (Equals(f.fileName, fileName)) {
                    // don't add this to the list
                    isAlreadyInList = true;
                    duplicates = true;
                    break;
                }
            }

            // Add the file to the list of files, if not already there...
            if (!isAlreadyInList) {
                UploadFile upload;
                upload.fileName.Format(fileName);
                upload.volcanoIndex = volcanoIndex;
                upload.deleteFile = (deleteFile == 1) ? true : false;
                m_fileList.AddTail(upload);
            }

            // init to get next token
            szToken = nullptr;
        }
    }
    fileRef.Close();

    // If there were duplicates in the file, then write it again now without duplicated
    if (duplicates) {
        ExportList();
    }

    // Restore the priority of this thread. 
    SetThreadPriority(THREAD_PRIORITY_ABOVE_NORMAL);

    return true;
}

void CFTPServerContacter::ExportList()
{
    POSITION	listPos;
    CString line;
    int nLines = 0;
    time_t now;

    time(&now);
    double timeSinceLastExport = difftime(now, m_lastExportTime);
    m_lastExportTime = now;
    if (timeSinceLastExport < 10) {
        return; // <-- no need to update the file within less than 10 seconds
    }

    if (m_fileList.GetCount() == 0)
    {
        if (!m_hasReadInFileList) {
            ParseAFile(m_listLogFile_Temp);
        }
        else {
            DeleteFile(m_listLogFile);
        }
        return;
    }

    //write log file into output directory \\temp\\fileList_temp.txt
    FILE *f = fopen(m_listLogFile_Temp, "w");
    if (f != nullptr) {
        listPos = m_fileList.GetHeadPosition();
        while (listPos != nullptr)
        {
            UploadFile &upload = m_fileList.GetNext(listPos);

            // Write to file only every 50 lines, to create fewer writing events
            if (nLines == 0) {
                line.Format("%s\t%s\t%d\n", (LPCSTR)upload.fileName, (LPCSTR)g_volcanoes.m_simpleName[upload.volcanoIndex], upload.deleteFile);
                ++nLines;
            }
            else {
                line.AppendFormat("%s\t%s\t%d\n", (LPCSTR)upload.fileName, (LPCSTR)g_volcanoes.m_simpleName[upload.volcanoIndex], upload.deleteFile);
                ++nLines;
                if (nLines == 50) {
                    fprintf(f, "%s", (LPCSTR)line);
                    nLines = 0;
                }
            }
        }
        if (nLines) {
            fprintf(f, "%s", (LPCSTR)line); // <-- write the last portion of the file
        }
        fclose(f);
    }

    // Delete the old file-list and move the new one to it's place
    //	(move the file fileList_temp.txt to fileList.txt)
    DeleteFile(m_listLogFile);
    MoveFile(m_listLogFile_Temp, m_listLogFile);
}

