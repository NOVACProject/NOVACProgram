#include "StdAfx.h"
#include "FTPHandler.h"
#include "../Common/CfgTxtFileHandler.h"

#ifdef _MSC_VER
#pragma warning (push, 4)
#endif

using namespace Communication;

extern CFormView *pView;                   // <-- the main window
extern CConfigurationSetting g_settings;   // <-- the settings

// ------------------- Handling the different versions of electronics -------------------
bool IsPakFileExtension(ELECTRONICS_BOX version, const CString& fileSuffix)
{
    if (version == BOX_AXIS)
    {
        return fileSuffix == _T("PAK");
    }
    else
    {
        return fileSuffix == _T("pak");
    }
}

// Appends the typical file extension for a .pak file on an electronics box of the given generation to the filename.
CString AppendPakFileExtension(const CString& fileNameWithoutExtension, ELECTRONICS_BOX version)
{
    CString fullFileName;
    if (version == BOX_AXIS)
    {
        fullFileName.Format("%s.PAK", (LPCSTR)fileNameWithoutExtension);
    }
    else
    {
        fullFileName.Format("%s.pak", (LPCSTR)fileNameWithoutExtension);
    }
    return fullFileName;
}

// ------------------- CFTPHandle class implementation -------------------
CFTPHandler::CFTPHandler()
    : m_electronicsBox(BOX_AXIS), m_dataSpeed(4.0)
{
}

CFTPHandler::CFTPHandler(ELECTRONICS_BOX box)
    : m_electronicsBox(box), m_dataSpeed(4.0)
{
}

CFTPHandler::~CFTPHandler(void)
{
    CString message;

    // Tell the world...
    message.Format("<node %d> terminated", m_mainIndex);
    ShowMessage(message);
}

void CFTPHandler::SetFTPInfo(int mainIndex, const CString& hostname, const CString& userName, const CString &pwd, int timeOut, long portNumber)
{
    this->m_mainIndex = mainIndex;
    this->m_ftpInfo.hostName = hostname;
    this->m_ftpInfo.userName = userName;
    this->m_ftpInfo.password = pwd;
    this->m_ftpInfo.port = portNumber;
    this->m_ftpInfo.timeout = timeOut;
    this->m_spectrometerSerialID = g_settings.scanner[mainIndex].spec[0].serialNumber;

    this->m_storageDirectory.Format("%sTemp\\%s\\", (LPCSTR)g_settings.outputDirectory, (LPCSTR)m_spectrometerSerialID);

    // Make sure that the storage directory exists
    if (CreateDirectoryStructure(m_storageDirectory))
    {
        GetSysTempFolder(m_storageDirectory);
        if (CreateDirectoryStructure(m_storageDirectory))
        {
            CString errorMsg;
            errorMsg.Format("FTPHandler: Could not create temporary-directory for spectrometer; %s", (LPCSTR)m_spectrometerSerialID);
            ShowMessage(errorMsg);
            MessageBox(NULL, errorMsg, "Serious error", MB_OK);
        }
    }
}

void CFTPHandler::SetFTPInfo(int mainIndex, const CString& IP, const CString& userName, const CString &pwd, const CString &admUserName, const CString &admPwd, int timeOut, long portNumber)
{
    if (m_electronicsBox == BOX_AXIOMTEK)
    {
        // The AxiomTek electronics box only uses one login for all uses.
        this->m_ftpInfo.adminUserName = userName;
        this->m_ftpInfo.adminPassword = pwd;
    }
    else
    {
        this->m_ftpInfo.adminUserName = admUserName;
        this->m_ftpInfo.adminPassword = admPwd;
    }

    this->SetFTPInfo(mainIndex, IP, userName, pwd, timeOut, portNumber);
}

bool CFTPHandler::DownloadPakFiles(const CString& folder, std::vector<CScannerFileInfo>& fileInfoList)
{
    const CString workPak = "WORK.PAK";  // case here doesn't matter since the comparison below is case insensitive
    const CString uploadPak = "upload.pak";  // case here doesn't matter since the comparison below is case insensitive

    if (Connect(m_ftpInfo.hostName, m_ftpInfo.userName, m_ftpInfo.password, m_ftpInfo.timeout, m_ftpInfo.port) != 1)
    {
        pView->PostMessage(WM_SCANNER_NOT_CONNECT, (WPARAM)&(m_spectrometerSerialID), 0);
        return false;
    }

    // if we should download data from a folder, then enter the folder first
    if (folder.GetLength() > 0)
    {
        EnterFolder(folder);
    }

    time_t start;
    time(&start);
    while (fileInfoList.size() > 0)
    {
        const CScannerFileInfo& fileInfo = fileInfoList.back();
        const CString fileName = AppendPakFileExtension(fileInfo.fileName, m_electronicsBox);

        if ((Equals(fileName, workPak) || Equals(fileName, uploadPak)) && folder.GetLength() == 0)
        {
            fileInfoList.pop_back(); // do not download work.pak not upload.pak in the root folder
            continue;
        }

        m_statusMsg.Format("Begin to download %s/%s", (LPCSTR)folder, (LPCSTR)fileName);
        ShowMessage(m_statusMsg);

        bool downloadSuccessful = DownloadSpectrumFile(fileName, m_storageDirectory, fileInfo.fileSize);

        if (downloadSuccessful)
        {
            fileInfoList.pop_back();
        }
        else
        {
            break; //get out of loop, 2007.4.30
        }

        time_t current;
        time(&current);
        const time_t secondsElapsed = current - start;
        if (secondsElapsed > g_settings.scanner[m_mainIndex].comm.queryPeriod)
        {
            break; // spent long enough on one scanner; move to next
        }
    }

    fileInfoList.clear();
    Disconnect();
    return true;
}

bool CFTPHandler::PollScanner()
{
    CString msg;
    msg.Format("<node %d> Checking for files to download", m_mainIndex);
    ShowMessage(msg);

    // get file and folder list
    long numberOfFilesAndFolders = m_fileInfoList.size() + m_rFolderList.size();
    if (numberOfFilesAndFolders <= 0)
    {
        numberOfFilesAndFolders = GetPakFileList(""); //download Uxxx.pak list
        numberOfFilesAndFolders = max(0, numberOfFilesAndFolders); // These must be on separate lines otherwise there's risk that the scanner will be polled twice(!)
    }

    if (numberOfFilesAndFolders + m_rFolderList.size() == 0)
    {
        msg.Format("<node %d> No more files to download", m_mainIndex);
        ShowMessage(msg);
        return false;
    }

    // download Uxxx.pak files in top directory
    Sleep(5000);
    if (m_fileInfoList.size() > 0)
    {
        DownloadPakFiles("", m_fileInfoList);
    }

    // Enter each RXXX folder to download the files there
    if (m_rFolderList.size() > 0)
    {
        // Make a backup-copy of the folder-list
        std::vector<CString> localFolderList(begin(m_rFolderList), end(m_rFolderList));

        time_t start;
        time(&start);
        time_t current;
        while (localFolderList.size() > 0)
        {
            m_fileInfoList.clear();

            // Get the folder name
            CString folder = localFolderList.back();
            localFolderList.pop_back();

            // Rebuild m_fileInfoList
            if (GetPakFileList(folder) < 0)
            {
                return true;
            }

            Sleep(5000);
            if (DownloadPakFiles(folder, m_fileInfoList))
            {
                // we managed to download the files, now remove the folder
                if (Connect(m_ftpInfo.hostName, m_ftpInfo.userName, m_ftpInfo.password, m_ftpInfo.timeout, m_ftpInfo.port) != 1)
                {
                    pView->PostMessage(WM_SCANNER_NOT_CONNECT, (WPARAM)&(m_spectrometerSerialID), 0);
                    return false;
                }
                DeleteFolder(folder);
                Disconnect();
            }
            else
            {
                // we failed to download the files...
                Disconnect(); //get out of loop 2007.4.30
            }

            time(&current);
            const time_t seconds = current - start;
            long queryPeriod = g_settings.scanner[m_mainIndex].comm.queryPeriod;
            if (seconds > queryPeriod)
            {
                break; // spent long enough on one scanner; move to next
            }
        }// end while
    }

    return true;
}

bool CFTPHandler::DownloadOldPak(long interval)
{
    time_t startTime, stopTime;

    if (m_fileInfoList.size() <= 0)	//changed 2006-12-12. Omit get pak file list one more time
    {
        return false;
    }

    // Assume that the data-speed is at least 4kb/s
    if (m_dataSpeed <= 0)
    {
        m_dataSpeed = 4.0;
    }

    const CScannerFileInfo& pakFileInfo = m_fileInfoList.back();

    time(&startTime);
    CString fileFullName = AppendPakFileExtension(pakFileInfo.fileName, m_electronicsBox);
    bool downloadSuccessful = DownloadSpectrumFile(fileFullName, m_storageDirectory, pakFileInfo.fileSize);
    time(&stopTime);

    if (downloadSuccessful)
    {
        m_fileInfoList.pop_back();
    }

    return true;
}

long CFTPHandler::GetPakFileList(const CString& folder)
{
    CString fileList, listFilePath, msg;
    CFTPSocket ftpSocket(m_ftpInfo.timeout);

    // Start with clearing out the list of files...
    m_fileInfoList.clear();

    // We save the data to a temporary file on disk....
    listFilePath.Format("%sfileList.txt", (LPCSTR)m_storageDirectory);
    ftpSocket.SetLogFileName(listFilePath);

    // Log in to the instrument's FTP-server
    if (!ftpSocket.Login(m_ftpInfo.hostName, m_ftpInfo.userName, m_ftpInfo.password, m_ftpInfo.port))
    {
        return -1;
    }

    Sleep(100);

    // Enter Rxxx folder
    if (folder.GetLength() == 4)
    {
        msg.Format("<node %d> Getting file-list from folder: %s", m_mainIndex, (LPCSTR)folder);
        ShowMessage(msg);

        if (!ftpSocket.EnterFolder(folder))
        {
            ftpSocket.Disconnect();

            msg.Format("<node %d> Failed to enter folder: %s", m_mainIndex, (LPCSTR)folder);
            ShowMessage(msg);

            return -2;
        }

        // Download the list of files...
        if (ftpSocket.GetFileList())
        {
            FillFileList(listFilePath);
        }
    }
    else
    {
        msg.Format("<node %d> Getting file-list", m_mainIndex);
        ShowMessage(msg);

        // Download the list of files...
        if (ftpSocket.GetFileList())
        {
            FillFileList(listFilePath);
        }
    }

    // Close the connection
    ftpSocket.Disconnect();

    // Count the number of files and folders in the instrument
    const long pakFileSum = m_fileInfoList.size();
    const long pakFolderSum = m_rFolderList.size();

    msg.Format("<node %d> %d files and %d folders found on disk", m_mainIndex, pakFileSum, pakFolderSum);
    ShowMessage(msg);

    return pakFileSum;
}

bool CFTPHandler::GetDiskFileList(char disk)
{
    CString listFilePath;
    listFilePath.Format("%sfileList.txt", (LPCSTR)m_storageDirectory);

    CFTPSocket ftpSocket;
    ftpSocket.SetLogFileName(listFilePath);

    if (disk == 'B')
    {
        if (!ftpSocket.Login(m_ftpInfo.hostName, m_ftpInfo.userName, m_ftpInfo.password, m_ftpInfo.port))
        {
            return false;
        }
    }
    else
    {
        if (!ftpSocket.Login(m_ftpInfo.hostName, m_ftpInfo.adminUserName, m_ftpInfo.adminPassword, m_ftpInfo.port))
        {
            return false;
        }
    }

    if (ftpSocket.GetFileList())
    {
        FillFileList(ftpSocket.m_listFileName, disk);

        ftpSocket.Disconnect();
        ShowMessage("File list was downloaded.");
        
        return (m_fileInfoList.size() > 0);
    }
    else
    {
        ftpSocket.Disconnect();
        ShowMessage("File list was not downloaded. It may be caused by slow or broken Ethernet connection.");
        EmptyFileInfo();
        return false;
    }
}

void CFTPHandler::EmptyFileInfo()
{
    m_fileInfoList.clear();
    m_rFolderList.clear();
}

int CFTPHandler::FillFileList(const CString& fileName, char disk)
{
    CStdioFile file;
    CFileException fileException;
    if (!file.Open(fileName, CFile::modeRead | CFile::typeText, &fileException))
    {
        CString msg;
        msg.Format("Can not open file %s", (LPCSTR)fileName);
        ShowMessage(msg);
        return false;
    }

    EmptyFileInfo(); //empty m_fileInfoList to fill in new info

    CString str;
    int nofFilesFound = 0;
    while (file.ReadString(str))
    {
        if (str.GetLength() <= 0)
            break;

        if (str.Find("Í") != -1)
            break;

        if (ParseFileInfo(str, disk))
        {
            nofFilesFound++;
        }
    } // token length should be bigger than 56 bytes//!= 0);

    file.Close();

    return nofFilesFound;
}

bool CFTPHandler::AddFolderInfo(const CString& line)
{
    const int folderNameLength = 4; // <-- all RXXX folders contains 4 characters

    CString folderName = line.Right(folderNameLength);

    if (folderName.Left(1) != _T("R") && folderName.Left(1) != _T("r"))
    {
        return false; // The folder is not a RXXX - folder, do not insert it into the list!
    }

    m_rFolderList.push_back(folderName);

    return true;
}

bool CFTPHandler::ParseFileInfo(CString line, char disk)
{
    CString fileName, fileSubfix, month, date, time, mmdd;
    long fileSize;
    int curPos = 0;

    line.Remove('\n');
    line.Remove('\r');
    if (line.Find("drw") != -1) // Changed 2008.06.30
    {
        return AddFolderInfo(line);
    }

    for (int columnIdx = 0; columnIdx < 9; columnIdx++)
    {
        CString resToken = line.Tokenize(" ", curPos);
        if (curPos < 0)
        {
            return false;
        }

        if (columnIdx == 4)
        {
            fileSize = atoi(resToken);
        }
        else if (columnIdx == 5)
        {
            month.Format("%s", (LPCSTR)resToken);
        }
        else if (columnIdx == 6)
        {
            date.Format("%s", (LPCSTR)resToken);
        }
        else if (columnIdx == 7)
        {
            time.Format("%s", (LPCSTR)resToken);
        }
        else if (columnIdx == 8)
        {
            fileName.Format("%s", (LPCSTR)resToken);
            fileName.Remove('\n');
            fileName.Remove('\r');
            ExtractSuffix(fileName, fileSubfix);
        }
    }
    mmdd.Format("%s %s", (LPCSTR)month, (LPCSTR)date);

    if (Equals(fileName, "..") || Equals(fileName, "."))
    {
        return false; // no use with these
    }

    // If the file found is indeed a .pak-file then add it to the list of files
    if (IsPakFileExtension(m_electronicsBox, fileSubfix))
    {
        CScannerFileInfo scannerFileInfo(disk, fileName, fileSubfix, fileSize, mmdd, time);
        m_fileInfoList.push_back(scannerFileInfo);
        return true;
    }

    return false;
}

void CFTPHandler::ExtractSuffix(CString& fileName, CString& fileSuffix)
{
    const int position = fileName.ReverseFind('.');
    const int length = CString::StringLength(fileName);
    if (length < 5 || position < 0)
    {
        fileSuffix = "";
        return;
    }
    fileSuffix = fileName.Right(length - position - 1);
    fileName = fileName.Left(position);
}

bool CFTPHandler::DownloadSpectrumFile(const CString& remoteFile, const CString& localDirectory, long remoteFileSize)
{
    CString localFileFullPath;
    localFileFullPath.Format("%s%s", (LPCSTR)localDirectory, (LPCSTR)remoteFile);

    //connect to the ftp server
    if (!DownloadFile(remoteFile, localDirectory, remoteFileSize))
    {
        m_statusMsg.Format("Can not download file from remote scanner (%s) by FTP", (LPCSTR)m_ftpInfo.hostName);
        ShowMessage(m_statusMsg);
        return false;
    }

    // Check that the size on disk is same as the size in the remote computer
    if (Common::RetrieveFileSize(localFileFullPath) != remoteFileSize)
    {
        m_statusMsg.Format("Error while downloading file (%s) by FTP, local file size does not agree with remote", (LPCSTR)m_ftpInfo.hostName);
        ShowMessage(m_statusMsg);
        return false;
    }

    /** The .pak-file-handler, used to check the downloaded .pak-files */
    FileHandler::CPakFileHandler pakFileHandler;

    // Check the contents of the file and make sure it's an ok file
    if (1 == pakFileHandler.ReadDownloadedFile(localFileFullPath))
    {
        m_statusMsg.Format("CPakFileHandler found an error with the file %s. Will try to download again", (LPCSTR)localFileFullPath);
        ShowMessage(m_statusMsg);

        // Download the file again
        if (!DownloadFile(remoteFile, localDirectory, remoteFileSize))
        {
            return false;
        }

        if (1 == pakFileHandler.ReadDownloadedFile(localFileFullPath))
        {
            ShowMessage("The pak file is corrupted");
            //DELETE remote file
            if (!DeleteRemoteFile(remoteFile))
            {
                m_statusMsg.Format("<node %d> Remote File %s could not be removed", m_mainIndex, (LPCSTR)remoteFile);
                ShowMessage(m_statusMsg);
            }
            return false;
        }
    }

    //DELETE remote file
    m_statusMsg.Format("%s has been downloaded", (LPCSTR)remoteFile);
    ShowMessage(m_statusMsg);
    if (!DeleteRemoteFile(remoteFile))
    {
        m_statusMsg.Format("<node %d> Remote File %s could not be removed", m_mainIndex, (LPCSTR)remoteFile);
        ShowMessage(m_statusMsg);
    }

    // Tell the world that we've done with one download
    pView->PostMessage(WM_FINISH_DOWNLOAD, (WPARAM)&m_spectrometerSerialID, (LPARAM)&m_dataSpeed);

    return true;
}

bool CFTPHandler::DeleteRemoteFile(const CString& remoteFile)
{
    if (m_FtpConnection == nullptr)
    {
        if (Connect(m_ftpInfo.hostName, m_ftpInfo.userName, m_ftpInfo.password, m_ftpInfo.timeout, m_ftpInfo.port) != 1)
        {
            pView->PostMessage(WM_SCANNER_NOT_CONNECT, (WPARAM)&(m_spectrometerSerialID), 0);
            return false;
        }
    }

    if (m_electronicsBox == BOX_AXIS)
    {
        CString localCopyOfRemoteFileName;
        localCopyOfRemoteFileName.Format(remoteFile);
        if (!FindFile(localCopyOfRemoteFileName)) // This does not work with the axis-system, for some reason...
        {
            return false;
        }
    }

    BOOL result = m_FtpConnection->Remove((LPCTSTR)remoteFile);

    return (result == TRUE);
}

bool CFTPHandler::DownloadFile(const CString& remoteFileName, const CString& localDirectory, long remoteFileSize)
{
    // High resolution counter
    LARGE_INTEGER lpFrequency, timingStart, timingStop;
    BOOL useHighResolutionCounter = QueryPerformanceFrequency(&lpFrequency);

    // Low resolution counter, used as a backup
    clock_t timing_Stop, timing_Start; // <-- timing of the upload speed

    // The filename
    CString fileFullName;
    fileFullName.Format("%s%s", (LPCSTR)localDirectory, (LPCSTR)remoteFileName);

    //check local file,if a file with same name exists, delete it
    if (IsExistingFile(fileFullName))
    {
        DeleteFile(fileFullName);
    }

    //show running lamp on interface
    pView->PostMessage(WM_SCANNER_RUN, (WPARAM)&(m_spectrometerSerialID), 0);

    timing_Start = clock(); // <-- timing...
    useHighResolutionCounter = QueryPerformanceCounter(&timingStart);

    if (!DownloadAFile(remoteFileName, fileFullName))
    {
        return false;
    }

    // Timing...
    useHighResolutionCounter = QueryPerformanceCounter(&timingStop);
    timing_Stop = clock();

    // Remember the speed of the upload
    if (useHighResolutionCounter)
    {
        double clocksPerSec = CLOCKS_PER_SEC;
        double elapsedTime = max(1.0 / clocksPerSec, (double)(timing_Stop - timing_Start) / clocksPerSec);
        m_dataSpeed = remoteFileSize / (elapsedTime * 1024.0);
    }
    else
    {
        double elapsedTime2 = ((double)timingStop.LowPart - (double)timingStart.LowPart) / (double)lpFrequency.LowPart;
        m_dataSpeed = remoteFileSize / (elapsedTime2 * 1024.0);
    }

    m_statusMsg.Format("Finished downloading file %s from %s @ %.1lf kb/s", (LPCSTR)fileFullName, (LPCSTR)m_spectrometerSerialID, m_dataSpeed);
    ShowMessage(m_statusMsg);

    return true;
}

bool CFTPHandler::MakeCommandFile(const CString& fileName, const char* command)
{
    FILE *f = fopen(fileName, "w");
    if (f == nullptr)
    {
        return false; // could not open file for writing
    }

    fprintf(f, command);
    fclose(f);
    return true;
}

bool CFTPHandler::SendCommand(const char* cmd)
{
    CString localFileFullPath;
    localFileFullPath.Format("%scommand.txt", (LPCSTR)m_storageDirectory);
    if (false == MakeCommandFile(localFileFullPath, cmd))
    {
        m_statusMsg.Format("Failed to create the command file for (%s)", (LPCSTR)m_ftpInfo.hostName);
        ShowMessage(m_statusMsg);
        return false;
    }

    // Connect to the ftp server
    if (!Connect(m_ftpInfo.hostName, m_ftpInfo.userName, m_ftpInfo.password, m_ftpInfo.timeout, m_ftpInfo.port) == 1)
    {
        return false;
    }

    // Upload file to the server
    if (UploadFile(localFileFullPath, m_commandFileName) == 0)
    {
        Disconnect();
        m_statusMsg.Format("Can not upload command file to the remote scanner (%s) by FTP", (LPCSTR)m_ftpInfo.hostName);
        ShowMessage(m_statusMsg);
        return false;
    }

    Disconnect();
    return true;
}

void CFTPHandler::GotoSleep()
{
    if (!DeleteRemoteFile(m_commandFileName))
    {
        ShowMessage("Remote File command.txt could not be removed");
    }
    SendCommand("pause\npoweroff");
    pView->PostMessage(WM_SCANNER_SLEEP, (WPARAM)&(m_spectrometerSerialID), 0);
    //download old pak files during sleeping time
    DownloadOldPak(14400);
    //disconnect when finish downloading 2007-09-23
    Disconnect();
}

void CFTPHandler::WakeUp()
{
    if (!DeleteRemoteFile(m_commandFileName))
    {
        //	ShowMessage("Remote File command.txt could not be removed");
    }
    bool success = SendCommand("poweron\nresume");

    // If we failed to upload the command-file then try again, at most 5 times
    int iterations = 0;
    while (success == false && iterations < 5)
    {
        Sleep(1000);
        success = SendCommand("poweron\nresume");
        ++iterations;
    }

    Disconnect();

    if (success == false)
    {
        ShowMessage("Failed to wake instrument up");
    }
    else
    {
        // Try to download the cfg.txt - file from the instrument. This is done 
        //	both to test the connection and to get information from the
        //	file (such as motor-steps-comp)
        this->DownloadCfgTxt();
    }
}

void CFTPHandler::Reboot()
{
    if (!DeleteRemoteFile(m_commandFileName))
    {
        //		ShowMessage("Remote File command.txt could not be removed");
    }
    SendCommand("reboot");
}

int CFTPHandler::DownloadCfgTxt()
{
    CString localFileName;

    // Connect to the administrators account
    if (Connect(m_ftpInfo.hostName, m_ftpInfo.adminUserName, m_ftpInfo.adminPassword, m_ftpInfo.timeout, m_ftpInfo.port) != 1)
    {
        return 0; // failed to connect
    }

    // The names of the local and remote files
    CString remoteFileName = "cfg.txt";
    localFileName.Format("%scfg.txt", (LPCSTR)m_storageDirectory);

    // Download the file.
    if (!DownloadAFile(remoteFileName, localFileName))
    {
        Disconnect();
        return 0;
    }

    // Tell the user about what we've done!
    m_statusMsg.Format("Downloaded cfg.txt from %s", (LPCSTR)m_spectrometerSerialID);
    ShowMessage(m_statusMsg);

    // Remember to Disconnect from the server
    Disconnect();

    // Try to parse the cfg.txt - file in order to get the paramters...
    FileHandler::CCfgTxtFileHandler cfgTxtReader;
    if (0 == cfgTxtReader.ReadCfgTxt(localFileName))
    {
        return 0;
    }

    // if todays date is dividable by 7 then try to upload the file
    int todaysDate = Common::GetDay();
    if (todaysDate % 7 == 0)
    {
        // Copy the file to a new name.
        CString copyFileName;
        copyFileName.Format("%scfg_%s.txt", (LPCSTR)m_storageDirectory, (LPCSTR)m_spectrometerSerialID);
        if (0 != CopyFile(localFileName, copyFileName, FALSE))
        {
            // Get the index of this volcano
            int volcanoIndex = Common::GetMonitoredVolcano(m_spectrometerSerialID);

            if (-1 != volcanoIndex)
            {
                UploadToNOVACServer(copyFileName, volcanoIndex, true);
            }
        }
    }

    return 1;
}


#ifdef _MSC_VER
#pragma warning (pop)
#endif