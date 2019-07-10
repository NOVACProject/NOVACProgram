// FileTransferDlg.cpp : implementation file
//

#include "stdafx.h"
#include "../NovacMasterProgram.h"
#include "FileTransferDlg.h"

#include "../Common/Common.h"

#include "../Configuration/configuration.h"

extern CConfigurationSetting g_settings;

using namespace Dialogs;

//ftp functions
UINT DownloadFileListWithFTP(LPVOID pParam);
UINT DownloadDiskWithFTP(LPVOID pParam);
UINT DownloadFolderWithFTP(LPVOID pParam);
UINT DownloadFileWithFTP(LPVOID pParam);
UINT ViewFileByFTP(LPVOID pParam);
UINT DeleteFileByFTP(LPVOID pParam);
UINT UpdateFileWithFTP(LPVOID pParam);
UINT UploadFileWithFTP(LPVOID pParam);
UINT ExpandFolderByFTP(LPVOID pParam);
//serial functions
UINT DownloadFileWithSerial(LPVOID pParam);
UINT DownloadFolderWithSerial(LPVOID pParam);
UINT DownloadDiskWithSerial(LPVOID pParam);
UINT DownloadFileListWithSerial(LPVOID pParam);
UINT ViewFileBySerial(LPVOID pParam);
UINT DeleteFileBySerial(LPVOID pParam);
UINT UpdateFileWithSerial(LPVOID pParam);
UINT UploadFileWithSerial(LPVOID pParam);
UINT ExpandFolderBySerial(LPVOID pParam);

CFileTransferDlg* pFileTransferDlg;
//-----------------------------
// download file list by serial connection
UINT DownloadFileListWithSerial(LPVOID pParam)
{
    time_t tStart, tStop;
    CString textA, textB, msg;
    CFileTransferDlg* dlg = (CFileTransferDlg*)pParam;

    dlg->m_busy = true;
    dlg->m_fileStatusEdit.SetWindowText("Downloading file list ...");
    time(&tStart);


    if (!dlg->m_SerialController->InitCommunication('B')) {
        dlg->m_busy = false;
        return 0;
    }

    // Check the type of electronics-box first...
    dlg->m_SerialController->PollElectronicsType();

    if (!dlg->m_SerialController->GetFileListText(dlg->m_textA, dlg->m_textB))
    {
        dlg->m_fileStatusEdit.SetWindowText("Can not download file list from disk B ");
        dlg->m_busy = false;
        return 0;
    }
    dlg->m_fileStatusEdit.SetWindowText("Finish downloading file list from disk A and disk B");

    //dlg->m_SerialController->StartTx();
    time(&tStop);
    msg.Format("Finsh downloading file list\r\nUsed %d seconds", static_cast<int>(tStop - tStart));
    dlg->m_fileStatusEdit.SetWindowText(msg);

    // Update the lists...
    pFileTransferDlg->ClearOut();
    pFileTransferDlg->ParseDir(pFileTransferDlg->m_textA, pFileTransferDlg->m_fileListA);
    pFileTransferDlg->ParseDir(pFileTransferDlg->m_textB, pFileTransferDlg->m_fileListB);

    pFileTransferDlg->PostMessage(WM_UPDATE_FILE_TREE);

    dlg->m_busy = false;
    return 0;
}

// download file list by ftp connection
UINT DownloadFileListWithFTP(LPVOID pParam)
{
    CFileTransferDlg* dlg = (CFileTransferDlg*)pParam;

    /** Get the list for 'B'-disk */
    if (dlg->m_ftpController->GetDiskFileList(1))
    {
        // Update the lists...
        pFileTransferDlg->ClearOut();

        POSITION pos = dlg->m_ftpController->m_fileInfoList.GetHeadPosition();
        while (pos != NULL) {
            CScannerFileInfo *fileInfo = new CScannerFileInfo(dlg->m_ftpController->m_fileInfoList.GetNext(pos));
            pFileTransferDlg->m_fileListB.AddTail(fileInfo);
        }
        pos = dlg->m_ftpController->m_rFolderList.GetHeadPosition();
        while (pos != NULL) {
            CScannerFolderInfo *folderInfo = new CScannerFolderInfo('B', dlg->m_ftpController->m_rFolderList.GetNext(pos), "", "");
            pFileTransferDlg->m_folderListB.AddTail(folderInfo);
        }
    }
    else
        MessageBox(NULL, "Can not get ftp file list", "Notice", MB_OK);

    // Pause a little bit, for the small FTP-server to have time to recover...
    Sleep(1000);

    /** Get the list for 'A'-disk */
    if (dlg->m_ftpController->GetDiskFileList(0))
    {
        POSITION pos = dlg->m_ftpController->m_fileInfoList.GetHeadPosition();
        while (pos != NULL) {
            CScannerFileInfo *fileInfo = new CScannerFileInfo(dlg->m_ftpController->m_fileInfoList.GetNext(pos));
            pFileTransferDlg->m_fileListA.AddTail(fileInfo);
        }

        pFileTransferDlg->PostMessage(WM_UPDATE_FILE_TREE);
    }
    return 0;
}
//download file by serial connection
UINT DownloadFileWithSerial(LPVOID pParam)
{
    CFileTransferDlg* dlg = (CFileTransferDlg*)pParam;
    CString oldFullName, fileName, directory;
    char disk = dlg->m_diskName[0];
    int  curScanner = dlg->m_curScanner;
    char command[128];
    long fileSize;

    dlg->m_busy = true;
    dlg->m_fileStatusEdit.SetWindowText("Connecting...");

    // Change disk...
    dlg->m_SerialController->SwitchToDisk(disk);

    fileName.Format(dlg->m_remoteFileName);
    int slashIndex = fileName.Find('\\');

    if (slashIndex != -1) {
        // If the file resides inside one directory...
        directory.Format("%s", (LPCSTR)fileName.Left(slashIndex));
        fileName.Format("%s", (LPCSTR)fileName.Right(strlen(dlg->m_remoteFileName) - slashIndex - 1));
        fileSize = dlg->GetListFileSize(fileName, disk);

        // Change the directory
        sprintf(command, "cd %s", (LPCSTR)directory);
        dlg->m_SerialController->SendCommand(command);
    }
    else {
        // If the file resided in the top directory
        fileName.Format("%s", dlg->m_remoteFileName);
        fileSize = dlg->GetListFileSize(fileName, disk);
    }
    dlg->m_SerialController->StartTx();
    if (dlg->m_SerialController->DownloadFile(fileName, dlg->m_storagePath, disk, fileSize) == SUCCESS)
    {
        oldFullName.Format("%s%s", (LPCSTR)dlg->m_storagePath, (LPCSTR)fileName);
        if (oldFullName != dlg->m_localFileFullName)
            CFile::Rename(oldFullName, dlg->m_localFileFullName);
        dlg->m_fileStatusEdit.SetWindowText("SUCCESSFULLY DOWNLOADED");

    }

    // Remember to go up from the specified directory again...
    if (slashIndex != -1) {
        dlg->m_SerialController->SendCommand("cd ..");
    }

    dlg->m_busy = false;
    return 0;
}

//download  file  by ftp connection
UINT DownloadFileWithFTP(LPVOID pParam)
{
    CFileTransferDlg* dlg = (CFileTransferDlg*)pParam;
    CString msg, fileName, directory, fileFullName;
    time_t startTime, stopTime;

    time(&startTime);
    char disk = dlg->m_diskName[0];
    if (disk == 'B') {
        if (dlg->m_ftpController->Connect(
            dlg->m_ftpController->m_ftpInfo.hostName,
            dlg->m_ftpController->m_ftpInfo.userName,
            dlg->m_ftpController->m_ftpInfo.password,
            dlg->m_ftpController->m_ftpInfo.timeout) != 1)
        {
            msg.Format("%s can not be connected", (LPCSTR)dlg->m_ftpController->m_ftpInfo.hostName);
            dlg->m_fileStatusEdit.SetWindowText(msg);
            dlg->m_ftpController->Disconnect();
            return 0;
        }
    }
    else {
        if (dlg->m_ftpController->Connect(dlg->m_ftpController->m_ftpInfo.hostName,
            dlg->m_ftpController->m_ftpInfo.adminUserName,
            dlg->m_ftpController->m_ftpInfo.adminPassword,
            dlg->m_ftpController->m_ftpInfo.timeout) != 1)
        {
            msg.Format("%s can not be connected", (LPCSTR)dlg->m_ftpController->m_ftpInfo.hostName);
            dlg->m_fileStatusEdit.SetWindowText(msg);
            dlg->m_ftpController->Disconnect();
            return 0;
        }
    }

    fileName.Format(dlg->m_remoteFileName);
    int slashIndex = fileName.Find('\\');
    if (slashIndex != -1) {
        // If the file resides inside one directory...
        directory.Format("%s", (LPCSTR)fileName.Left(slashIndex));
        fileName.Format("%s", (LPCSTR)fileName.Right(strlen(dlg->m_remoteFileName) - slashIndex - 1));

        // Enter the folder of the file
        dlg->m_ftpController->EnterFolder(directory);
    }
    else {
        // If the file resided in the top directory
        fileName.Format("%s", dlg->m_remoteFileName);
    }
    fileFullName.Format("%s%s", (LPCSTR)dlg->m_storagePath, (LPCSTR)fileName);
    msg.Format("Begin to download %s from %s", (LPCSTR)fileName, (LPCSTR)dlg->m_ftpController->m_ftpInfo.hostName);
    ShowMessage(msg);

    if (!dlg->m_ftpController->DownloadAFile(fileName, fileFullName))
    {
        dlg->m_ftpController->Disconnect();
        return false;
    }
    if (slashIndex != -1)
        dlg->m_ftpController->GotoTopDirectory();

    time(&stopTime);
    dlg->m_ftpController->Disconnect();
    msg.Format("%s was downloaded. The downloading spent %d seconds", (LPCSTR)fileFullName, static_cast<int>(stopTime - startTime));
    dlg->m_fileStatusEdit.SetWindowText(msg);
    return true;
}

UINT DownloadFolderWithSerial(LPVOID pParam)
{
    CFileTransferDlg* dlg = (CFileTransferDlg*)pParam;
    CString fileName, folderName;
    CString oldFullName, msg;
    char disk = dlg->m_diskName[0];
    int  curScanner = dlg->m_curScanner;
    char command[128];
    long fileSize;

    dlg->m_busy = true;

    // First get the list of the files to download
    if (ExpandFolderBySerial(pParam)) {
        msg.Format("Failed to get file list");
        return 1;
    }

    // The name of the folder to download...
    folderName.Format(dlg->m_remoteFileName);

    // Get a handle to the folder-list
    CList <CScannerFileInfo*, CScannerFileInfo*> *list = NULL;
    POSITION pos = dlg->m_folderListB.GetHeadPosition();
    while (pos != NULL) {
        CScannerFolderInfo *folderInfo = dlg->m_folderListB.GetNext(pos);
        if (Equals(folderInfo->folderName, folderName)) {
            list = &folderInfo->m_fileList;
            break;
        }
    }
    if (list == NULL) {
        dlg->m_busy = false;
        return 1; // ERROR...
    }

    // Change disk...
    dlg->m_SerialController->SwitchToDisk(disk);

    // Enter the directory
    sprintf(command, "cd %s", (LPCSTR)folderName);
    dlg->m_SerialController->SendCommand(command);

    // Start Tx...
    dlg->m_SerialController->StartTx();

    // Download all files in the directory...
    pos = list->GetHeadPosition();
    while (pos != NULL) {
        CScannerFileInfo *folderItem = list->GetNext(pos);

        // the name and size of the remote file to download
        fileName.Format("%s.%s", (LPCSTR)folderItem->fileName, (LPCSTR)folderItem->fileSubfix);
        fileSize = folderItem->fileSize;

        // Download the file
        if (dlg->m_SerialController->DownloadFile(fileName, dlg->m_storagePath, disk, fileSize) == SUCCESS)
        {
            // Get the file-name on the local computer
            if (!Equals(dlg->m_storagePath.Right(1), "\\"))
                oldFullName.Format("%s\\%s", (LPCSTR)dlg->m_storagePath, (LPCSTR)fileName);
            else
                oldFullName.Format("%s%s", (LPCSTR)dlg->m_storagePath, (LPCSTR)fileName);

            // Tell the user about our progress
            msg.Format("%s downloaded", (LPCSTR)fileName);
            dlg->m_fileStatusEdit.SetWindowText(msg);

        }
    }

    // Remember to go up from the specified directory again...
    dlg->m_SerialController->SendCommand("cd ..");
    dlg->m_busy = false;
    return 0;
}

//download  folder  by ftp connection
UINT DownloadFolderWithFTP(LPVOID pParam)
{
    CFileTransferDlg* dlg = (CFileTransferDlg*)pParam;
    CString msg, folderName, fileName, fileFullName;
    time_t startTime, stopTime;

    time(&startTime);

    // First get the list of the files to download
    if (ExpandFolderByFTP(pParam)) {
        msg.Format("Failed to get file list");
        return 1;
    }

    // The name of the folder to download...
    folderName.Format(dlg->m_remoteFileName);

    // Get a handle to the folder-list
    CList <CScannerFileInfo*, CScannerFileInfo*> *list = NULL;
    POSITION pos = dlg->m_folderListB.GetHeadPosition();
    while (pos != NULL) {
        CScannerFolderInfo *folderInfo = dlg->m_folderListB.GetNext(pos);
        if (Equals(folderInfo->folderName, folderName)) {
            list = &folderInfo->m_fileList;
            break;
        }
    }
    if (list == NULL)
        return 1; // ERROR...

    if (dlg->m_ftpController->Connect(dlg->m_ftpController->m_ftpInfo.hostName,
        dlg->m_ftpController->m_ftpInfo.userName,
        dlg->m_ftpController->m_ftpInfo.password,
        dlg->m_ftpController->m_ftpInfo.timeout) != 1)
    {
        msg.Format("%s can not be connected", (LPCSTR)dlg->m_ftpController->m_ftpInfo.hostName);
        dlg->m_fileStatusEdit.SetWindowText(msg);
        dlg->m_ftpController->Disconnect();
        return false;
    }

    // Enter the directory
    dlg->m_ftpController->EnterFolder(folderName);

    // Download all files in the directory...
    pos = list->GetHeadPosition();
    while (pos != NULL) {
        CScannerFileInfo *folderItem = list->GetNext(pos);

        // the name and size of the remote file to download
        fileName.Format("%s.%s", (LPCSTR)folderItem->fileName, (LPCSTR)folderItem->fileSubfix);
        fileFullName.Format("%s%s", (LPCSTR)dlg->m_storagePath, (LPCSTR)fileName);

        msg.Format("Begin to download %s from %s", (LPCSTR)fileName, (LPCSTR)dlg->m_ftpController->m_ftpInfo.hostName);
        ShowMessage(msg);

        // Download the file
        if (!dlg->m_ftpController->DownloadAFile(fileName, fileFullName))
        {
            dlg->m_ftpController->Disconnect();
            return false;
        }

        msg.Format("%s downloaded.", (LPCSTR)fileName);
        dlg->m_fileStatusEdit.SetWindowText(msg);
    }

    time(&stopTime);
    dlg->m_ftpController->Disconnect();

    msg.Format("All files downloaded in %d seconds", static_cast<int>(stopTime - startTime));
    dlg->m_fileStatusEdit.SetWindowText(msg);
    ShowMessage(msg);

    return 0;
}

UINT ViewFileByFTP(LPVOID pParam)
{
    CString fileName, fullFileName, directory, msg;
    time_t startTime, stopTime;
    CFileTransferDlg* dlg = (CFileTransferDlg*)pParam;

    fileName.Format(dlg->m_remoteFileName);
    if (!CFileTransferDlg::IsTextFile(fileName))
    {
        msg.Format("%s is not a text file.\r\nSo it can't be opened", (LPCSTR)fileName);
        dlg->ShowDlgInfo(msg);

        return 0;
    }

    // download file
    char disk = dlg->m_diskName[0];
    time(&startTime);
    if (disk == 'B') {
        if (dlg->m_ftpController->Connect(dlg->m_ftpController->m_ftpInfo.hostName,
            dlg->m_ftpController->m_ftpInfo.userName,
            dlg->m_ftpController->m_ftpInfo.password,
            dlg->m_ftpController->m_ftpInfo.timeout) != 1)
        {
            msg.Format("%s can not be connected", (LPCSTR)dlg->m_ftpController->m_ftpInfo.hostName);
            dlg->m_fileStatusEdit.SetWindowText(msg);
            dlg->m_ftpController->Disconnect();
            return 0;
        }
    }
    else {
        if (dlg->m_ftpController->Connect(dlg->m_ftpController->m_ftpInfo.hostName,
            dlg->m_ftpController->m_ftpInfo.adminUserName,
            dlg->m_ftpController->m_ftpInfo.adminPassword,
            dlg->m_ftpController->m_ftpInfo.timeout) != 1)
        {
            msg.Format("%s can not be connected", (LPCSTR)dlg->m_ftpController->m_ftpInfo.hostName);
            dlg->m_fileStatusEdit.SetWindowText(msg);
            dlg->m_ftpController->Disconnect();
            return 0;
        }
    }

    int slashIndex = fileName.Find('\\');
    if (slashIndex != -1) {
        // If the file resides inside one directory...
        directory.Format("%s", (LPCSTR)fileName.Left(slashIndex));
        fileName.Format("%s", (LPCSTR)fileName.Right(strlen(dlg->m_remoteFileName) - slashIndex - 1));

        // Enter the folder of the file
        dlg->m_ftpController->EnterFolder(directory);
    }
    else {
        // If the file resided in the top directory
        fileName.Format("%s", dlg->m_remoteFileName);
    }
    fullFileName.Format("%s%s", (LPCSTR)dlg->m_storagePath, (LPCSTR)fileName);
    msg.Format("Begin to download %s from %s", (LPCSTR)fileName, (LPCSTR)dlg->m_ftpController->m_ftpInfo.hostName);
    ShowMessage(msg);

    if (!dlg->m_ftpController->DownloadAFile(fileName, fullFileName))
    {
        dlg->m_ftpController->Disconnect();
        return 0;
    }
    time(&stopTime);
    dlg->m_ftpController->Disconnect();
    //	dlg->m_ftpController->m_dataSpeed = (dlg->m_ftpController->m_remoteFileSize/1024.0)/(stopTime - startTime);
    msg.Format("%s was downloaded. The downloading spent %d seconds", (LPCSTR)fileName, static_cast<int>(stopTime - startTime));
    dlg->m_fileStatusEdit.SetWindowText(msg);
    //show content
    dlg->ShowFileContent(fullFileName);
    return 0;

}
UINT ViewFileBySerial(LPVOID pParam)
{
    CString string, fileName, msg;
    long fileSize;
    char* buffer = 0;
    char  disk;
    CFileTransferDlg* dlg = (CFileTransferDlg*)pParam;
    //judge whether the file is  downloaded
    fileName.Format("%s%s", (LPCSTR)dlg->m_storagePath, (LPCSTR)dlg->m_remoteFileName);

    dlg->m_busy = true;
    if (!dlg->IsTextFile(fileName))
    {
        msg.Format("%s is not a text file.\r\nSo it can't be opened", (LPCSTR)fileName);
        dlg->ShowDlgInfo(msg);
        dlg->m_busy = false;
        return 0;
    }
    disk = dlg->m_diskName[0];
    dlg->m_SerialController->SwitchToDisk(disk);
    fileSize = dlg->GetListFileSize(fileName, disk);
    dlg->m_SerialController->StartTx();
    if (dlg->m_SerialController->DownloadFile(dlg->m_remoteFileName, dlg->m_storagePath, disk, fileSize) != SUCCESS)
    {
        msg.Format("%s is not downloaded, check serial connection", dlg->m_remoteFileName);
        dlg->ShowDlgInfo(msg);
        dlg->m_busy = false;
        return 0;
    }
    dlg->ShowFileContent(fileName);
    dlg->m_busy = false;
    return 0;
}

UINT ExpandFolderBySerial(LPVOID pParam)
{
    CString string, folderName, text, msg;
    CFileTransferDlg* dlg = (CFileTransferDlg*)pParam;
    time_t tStart, tStop;

    // Judge if this actually is a folder
    folderName.Format(dlg->m_remoteFileName);

    if (!dlg->IsDir(folderName)) {
        msg.Format("%s is not a directory\r\n", (LPCSTR)folderName);
        dlg->ShowDlgInfo(msg);
        return 1;
    }

    dlg->m_busy = true;

    // Get a handle to the folder-list
    CList <CScannerFileInfo*, CScannerFileInfo*> *list = NULL;
    POSITION pos = dlg->m_folderListB.GetHeadPosition();
    while (pos != NULL) {
        CScannerFolderInfo *folderInfo = dlg->m_folderListB.GetNext(pos);
        if (Equals(folderInfo->folderName, folderName)) {
            list = &folderInfo->m_fileList;
            break;
        }
    }
    if (list == NULL) {
        dlg->m_busy = false;
        return 1; // ERROR...
    }

    time(&tStart);
    dlg->m_SerialController->Bye();

    if (!dlg->m_SerialController->GetFileListText_Folder(folderName, text)) {
        dlg->m_fileStatusEdit.SetWindowText("Can not download file list");
        dlg->m_busy = false;
        return 1;
    }
    dlg->m_fileStatusEdit.SetWindowText("Finish downloading file list");

    time(&tStop);
    msg.Format("Finsh downloading file list\r\nUsed %d seconds", static_cast<int>(tStop - tStart));
    dlg->m_fileStatusEdit.SetWindowText(msg);


    pFileTransferDlg->ParseDir(text, *list);
    pFileTransferDlg->PostMessage(WM_UPDATE_FILE_TREE);
    dlg->m_busy = false;
    return 0;
}

UINT ExpandFolderByFTP(LPVOID pParam)
{
    CString string, folderName, text, msg;
    CFileTransferDlg* dlg = (CFileTransferDlg*)pParam;
    time_t tStart, tStop;

    // Judge if this actually is a folder
    folderName.Format(dlg->m_remoteFileName);

    if (!dlg->IsDir(folderName)) {
        msg.Format("%s is not a directory\r\n", (LPCSTR)folderName);
        dlg->ShowDlgInfo(msg);
        return 0;
    }

    // Get a handle to the folder-list
    CList <CScannerFileInfo*, CScannerFileInfo*> *list = NULL;
    POSITION pos = dlg->m_folderListB.GetHeadPosition();
    while (pos != NULL) {
        CScannerFolderInfo *folderInfo = dlg->m_folderListB.GetNext(pos);
        if (Equals(folderInfo->folderName, folderName)) {
            list = &folderInfo->m_fileList;
            break;
        }
    }
    if (list == NULL)
        return 0; // ERROR...

    time(&tStart);
    if (!dlg->m_ftpController->GetPakFileList(folderName)) {
        dlg->m_fileStatusEdit.SetWindowText("Can not download file list");
        return 0;
    }

    dlg->m_fileStatusEdit.SetWindowText("Finish downloading file list");

    time(&tStop);
    msg.Format("Finsh downloading file list\r\nUsed %d seconds", static_cast<int>(tStop - tStart));
    dlg->m_fileStatusEdit.SetWindowText(msg);

    pos = dlg->m_ftpController->m_fileInfoList.GetHeadPosition();
    while (pos != NULL) {
        CScannerFileInfo *fileInfo = new CScannerFileInfo(dlg->m_ftpController->m_fileInfoList.GetNext(pos));
        list->AddTail(fileInfo);
    }

    dlg->m_ftpController->Disconnect();

    pFileTransferDlg->PostMessage(WM_UPDATE_FILE_TREE);
    return 0;
}

UINT UpdateFileWithFTP(LPVOID pParam)
{
    CFileTransferDlg* dlg = (CFileTransferDlg*)pParam;
    CString localFilePath, msg, remoteFileName;
    remoteFileName.Format("%s", dlg->m_remoteFileName);
    localFilePath.Format("%s%s", (LPCSTR)dlg->m_storagePath, (LPCSTR)dlg->m_remoteFileName);
    //connect with ftp server
    char disk = dlg->m_diskName[0];
    if (disk == 'B') {
        if (dlg->m_ftpController->Connect(dlg->m_ftpController->m_ftpInfo.hostName,
            dlg->m_ftpController->m_ftpInfo.userName,
            dlg->m_ftpController->m_ftpInfo.password,
            dlg->m_ftpController->m_ftpInfo.timeout) != 1)
        {
            msg.Format("%s can not be connected", (LPCSTR)dlg->m_ftpController->m_ftpInfo.hostName);
            dlg->m_fileStatusEdit.SetWindowText(msg);
            dlg->m_ftpController->Disconnect();
            return 0;
        }
    }
    else {
        if (dlg->m_ftpController->Connect(dlg->m_ftpController->m_ftpInfo.hostName,
            dlg->m_ftpController->m_ftpInfo.adminUserName,
            dlg->m_ftpController->m_ftpInfo.adminPassword,
            dlg->m_ftpController->m_ftpInfo.timeout) != 1)
        {
            msg.Format("%s can not be connected", (LPCSTR)dlg->m_ftpController->m_ftpInfo.hostName);
            dlg->m_fileStatusEdit.SetWindowText(msg);
            dlg->m_ftpController->Disconnect();
            return 0;
        }
    }
    //delete old file
    if (!dlg->m_ftpController->DeleteRemoteFile(remoteFileName))
    {
        msg.Format("%s can not be deleted, try one more time", dlg->m_remoteFileName);
        dlg->ShowDlgInfo(msg);
    }
    //upload new file
        //upload this file
    AfxBeginThread(UploadFileWithFTP, dlg, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
    return 0;
}
UINT UploadFileWithFTP(LPVOID pParam)
{
    CFileTransferDlg* dlg = (CFileTransferDlg*)pParam;
    CString localFilePath, msg;
    localFilePath.Format("%s%s", (LPCSTR)dlg->m_storagePath, (LPCSTR)dlg->m_remoteFileName);
    //connect with ftp server
    char disk = dlg->m_diskName[0];
    if (disk == 'B') {
        if (dlg->m_ftpController->Connect(dlg->m_ftpController->m_ftpInfo.hostName,
            dlg->m_ftpController->m_ftpInfo.userName,
            dlg->m_ftpController->m_ftpInfo.password,
            dlg->m_ftpController->m_ftpInfo.timeout) != 1)
        {
            msg.Format("%s can not be connected", (LPCSTR)dlg->m_ftpController->m_ftpInfo.hostName);
            dlg->m_fileStatusEdit.SetWindowText(msg);
            dlg->m_ftpController->Disconnect();
            return 0;
        }
    }
    else {
        if (dlg->m_ftpController->Connect(dlg->m_ftpController->m_ftpInfo.hostName,
            dlg->m_ftpController->m_ftpInfo.adminUserName,
            dlg->m_ftpController->m_ftpInfo.adminPassword,
            dlg->m_ftpController->m_ftpInfo.timeout) != 1)
        {
            msg.Format("%s can not be connected", (LPCSTR)dlg->m_ftpController->m_ftpInfo.hostName);
            dlg->m_fileStatusEdit.SetWindowText(msg);
            dlg->m_ftpController->Disconnect();
            return 0;
        }
    }
    if (!dlg->m_ftpController->UploadFile(localFilePath, dlg->m_remoteFileName))
    {
        msg.Format("%s can not be uploaded, try one more time", dlg->m_remoteFileName);
        dlg->ShowDlgInfo(msg);
    }
    msg.Format("%s has been updated", dlg->m_remoteFileName);
    dlg->ShowDlgInfo(msg);
    AfxBeginThread(DownloadFileListWithFTP, dlg, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
    return 0;
}
UINT UpdateFileWithSerial(LPVOID pParam)
{
    //delete existing file
    CFileTransferDlg* dlg = (CFileTransferDlg*)pParam;
    CString msg, textA, textB;

    dlg->m_busy = true;

    // delete file
    if (dlg->m_SerialController->DelFile(dlg->m_remoteFileName, dlg->m_diskName[0]))
    {
        msg.Format("%s is deleted, will download new file information", dlg->m_remoteFileName);
        dlg->m_fileStatusEdit.SetWindowText((LPCTSTR)msg);
    }
    //upload this file
    AfxBeginThread(UploadFileWithSerial, dlg, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
    dlg->m_busy = false;
    return 0;
}

//delete file with ftp connection
UINT DeleteFileByFTP(LPVOID pParam)
{
    CString msg, remoteFileName;
    CFileTransferDlg* dlg = (CFileTransferDlg*)pParam;
    remoteFileName.Format("%s", dlg->m_remoteFileName);

    char disk = dlg->m_diskName[0];
    if (disk == 'B') {
        if (dlg->m_ftpController->Connect(dlg->m_ftpController->m_ftpInfo.hostName,
            dlg->m_ftpController->m_ftpInfo.userName,
            dlg->m_ftpController->m_ftpInfo.password,
            dlg->m_ftpController->m_ftpInfo.timeout) != 1)
        {
            msg.Format("%s can not be connected", (LPCSTR)dlg->m_ftpController->m_ftpInfo.hostName);
            dlg->m_fileStatusEdit.SetWindowText(msg);
            dlg->m_ftpController->Disconnect();
            return 0;
        }
    }
    else {
        if (dlg->m_ftpController->Connect(dlg->m_ftpController->m_ftpInfo.hostName,
            dlg->m_ftpController->m_ftpInfo.adminUserName,
            dlg->m_ftpController->m_ftpInfo.adminPassword,
            dlg->m_ftpController->m_ftpInfo.timeout) != 1)
        {
            msg.Format("%s can not be connected", (LPCSTR)dlg->m_ftpController->m_ftpInfo.hostName);
            dlg->m_fileStatusEdit.SetWindowText(msg);
            dlg->m_ftpController->Disconnect();
            return 0;
        }
    }
    if (!dlg->m_ftpController->DeleteRemoteFile(remoteFileName))
    {
        msg.Format("%s can not be deleted from %s", (LPCSTR)dlg->m_remoteFileName, (LPCSTR)dlg->m_ftpController->m_ftpInfo.hostName);
    }
    dlg->m_ftpController->Disconnect();
    AfxBeginThread(DownloadFileListWithFTP, dlg, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
    return 0;

}
//delete file with serial connection
UINT DeleteFileBySerial(LPVOID pParam)
{
    CFileTransferDlg* dlg = (CFileTransferDlg*)pParam;
    CString msg, textA, textB;

    dlg->m_busy = true;

    // delete file
    dlg->m_SerialController->Bye();
    dlg->m_SerialController->InitCommandLine();
    if (dlg->m_SerialController->Delete(dlg->m_remoteFileName, dlg->m_diskName[0]))
    {
        msg.Format("%s is deleted, will download new file information", dlg->m_remoteFileName);
        dlg->m_fileStatusEdit.SetWindowText((LPCTSTR)msg);
    }
    dlg->m_textA.Format("");
    dlg->m_textB.Format("");
    if (!dlg->m_SerialController->GetFileListText(dlg->m_textA, dlg->m_textB))
    {
        dlg->m_fileStatusEdit.SetWindowText("Can not download file list from disks ");
        dlg->m_busy = false;
        return 0;
    }
    // Update the lists...
    pFileTransferDlg->ClearOut();
    pFileTransferDlg->ParseDir(pFileTransferDlg->m_textA, pFileTransferDlg->m_fileListA);
    pFileTransferDlg->ParseDir(pFileTransferDlg->m_textB, pFileTransferDlg->m_fileListB);

    pFileTransferDlg->PostMessage(WM_UPDATE_FILE_TREE);
    dlg->m_fileStatusEdit.SetWindowText("Finish downloading file list from disk A and disk B");

    dlg->m_busy = false;
    return 0;
}
UINT UploadFileWithSerial(LPVOID pParam)
{
    CFileTransferDlg* dlg = (CFileTransferDlg*)pParam;
    CString msg, textA, textB;

    dlg->m_busy = true;

    // Set the settings for the serial controller
    int curScanner = dlg->m_curScanner;
    dlg->m_SerialController->StartTx();
    if (dlg->m_SerialController->UploadFile(dlg->m_storagePath, dlg->m_remoteFileName, dlg->m_diskName[0]))
    {
        msg.Format("%s was uploaded", dlg->m_remoteFileName);
        dlg->ShowDlgInfo(msg);
        if (!dlg->m_SerialController->GetFileListText(dlg->m_textA, dlg->m_textB))
        {
            dlg->m_fileStatusEdit.SetWindowText("Can not download file list from disks ");
            dlg->m_busy = false;
            return 0;
        }
        dlg->m_fileStatusEdit.SetWindowText("Finish downloading file list from disks ");

    }

    // Update the lists...
    pFileTransferDlg->ClearOut();
    pFileTransferDlg->ParseDir(pFileTransferDlg->m_textA, pFileTransferDlg->m_fileListA);
    pFileTransferDlg->ParseDir(pFileTransferDlg->m_textB, pFileTransferDlg->m_fileListB);
    pFileTransferDlg->PostMessage(WM_UPDATE_FILE_TREE);
    dlg->m_busy = false;
    return 0;
}



IMPLEMENT_DYNAMIC(CFileTransferDlg, CDialog)
CFileTransferDlg::CFileTransferDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CFileTransferDlg::IDD, pParent)
{
    pFileTransferDlg = this;
    m_diskName[0] = 'Z';

    m_busy = false;

    m_SerialController = NULL;

    m_storagePath.Format("%sTemp\\", (LPCSTR)g_settings.outputDirectory);
    m_preScanner = -100;
    m_click = 0;
    if (CreateDirectoryStructure(m_storagePath)) { // Make sure that the storage directory exists
        GetSysTempFolder(m_storagePath); // Try with the system - temp folder instead
        if (CreateDirectoryStructure(m_storagePath)) {
            MessageBox("Could not create temporary directory");
        }
    }

}

CFileTransferDlg::~CFileTransferDlg()
{
    ClearOut();
    if (m_SerialController != NULL) {
        delete(m_SerialController);
        m_SerialController = NULL;
    }

}

void CFileTransferDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_SCANNER_LIST, m_scannerList);

    DDX_Control(pDX, IDC_FILE_CONTENT, m_fileContentEdit);
    DDX_Control(pDX, IDC_FILE_TREE, m_fileTree);
    DDX_Control(pDX, IDC_STATUS_MSG, m_fileStatusEdit);
    DDX_Control(pDX, IDC_LOCALFILEEDIT, m_uploadFileEdit);
    DDX_Control(pDX, IDC_DISKCOMBO, m_diskCombo);
}


BEGIN_MESSAGE_MAP(CFileTransferDlg, CDialog)
    ON_MESSAGE(WM_UPDATE_FILE_TREE, OnUpdateFileTree)
    ON_BN_CLICKED(IDC_UPLOAD_BTN, OnUploadFile)
    ON_NOTIFY(TVN_SELCHANGED, IDC_FILE_TREE, OnTvnSelchangedFileTree)
    ON_COMMAND(ID_FILETREE_VIEWFILE, OnViewFile)
    ON_COMMAND(ID_FILETREE_DOWNLOAD, OnDownloadFile)
    ON_COMMAND(ID_FILETREE_DELETE, OnDeleteFile)
    ON_COMMAND(ID_FILETREE_ENTERFOLDER, OnExpandFolder)
    ON_LBN_SELCHANGE(IDC_SCANNER_LIST, OnLbnSelchangeScannerList)
    ON_BN_CLICKED(IDC_BROWSE_BUTTON, OnBnClickedBrowseButton)
    ON_BN_CLICKED(IDOK, OnBnClickedOk)
    ON_BN_CLICKED(IDC_UPDATEFILEBTN, OnBnClickedUpdatefilebtn)
END_MESSAGE_MAP()


BOOL CFileTransferDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    unsigned int i;
    m_preScanner = -100;
    // Update the scanner list.
    m_scannerList.ResetContent();
    if (g_settings.scannerNum > 0) {
        for (i = 0; i < g_settings.scannerNum; ++i) {
            m_scannerList.AddString(g_settings.scanner[i].spec[0].serialNumber);
        }
        m_scannerList.SetCurSel(0);
    }
    m_ImageList.Create(IDB_DISK, 24, 1, RGB(0, 0, 0));
    CBitmap bm, bm2, bm3, bm4, bm5;
    bm.LoadBitmap(IDB_TXT); // 1
    bm2.LoadBitmap(IDB_EXE); // 2
    bm3.LoadBitmap(IDB_PAK); //3
    bm4.LoadBitmap(IDB_DAT);//4
    bm5.LoadBitmap(IDB_FOLDER); //5
    m_ImageList.Add(&bm, RGB(0, 0, 0));
    m_ImageList.Add(&bm2, RGB(0, 0, 0));
    m_ImageList.Add(&bm3, RGB(0, 0, 0));
    m_ImageList.Add(&bm4, RGB(0, 0, 0));
    m_ImageList.Add(&bm5, RGB(0, 0, 0));
    int totalImg = m_ImageList.GetImageCount();
    m_fileTree.SetImageList(&m_ImageList, TVSIL_NORMAL);

    m_fileTree.parent = this;

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

LRESULT CFileTransferDlg::OnUpdateFileTree(WPARAM wParam, LPARAM lParam)
{
    CString fileFullName, folderName, subfix;
    HTREEITEM hTRootA, hTRootB, hTItem, hTItem2;
    m_fileTree.DeleteAllItems();

    POSITION listPos;
    CScannerFileInfo* fileInfo;
    fileInfo = NULL;
    int i = 0;
    CString folderFlag;
    folderFlag.Format("DIR");

    if (!(m_SerialController != NULL && m_SerialController->GetElectronicsBoxVersion() == BOX_VERSION_2))
    {
        hTRootA = AddOneItem((HTREEITEM)NULL, "Disk A", (HTREEITEM)
            TVI_ROOT, 0);
        m_fileTree.SetItemImage(hTRootA, 0, 0);
        listPos = m_fileListA.GetHeadPosition();
        while (listPos != NULL)
        {
            fileInfo = m_fileListA.GetNext(listPos);
            fileFullName.Format("%s.%s", (LPCSTR)fileInfo->fileName, (LPCSTR)fileInfo->fileSubfix);
            hTItem = AddOneItem(hTRootA, (LPTSTR)(LPCTSTR)fileFullName, (HTREEITEM)TVI_LAST, 2 * (i + 1));
            SetFileItemImage(hTItem, fileInfo->fileSubfix);
            i++;
        }
    }

    hTRootB = AddOneItem((HTREEITEM)NULL, "Disk B", (HTREEITEM)
        TVI_ROOT, 1);

    m_fileTree.SetItemImage(hTRootB, 0, 0);
    i = 0;
    listPos = m_fileListB.GetHeadPosition();
    while (listPos != NULL)
    {
        fileInfo = m_fileListB.GetNext(listPos);

        fileFullName.Format("%s.%s", (LPCSTR)fileInfo->fileName, (LPCSTR)fileInfo->fileSubfix);

        hTItem = AddOneItem(hTRootB, (LPTSTR)(LPCTSTR)fileFullName, (HTREEITEM)TVI_LAST, 2 * (i + 1));
        SetFileItemImage(hTItem, fileInfo->fileSubfix);
        i++;
    }
    listPos = m_folderListB.GetHeadPosition();
    while (listPos != NULL)
    {
        CScannerFolderInfo *folderInfo = m_folderListB.GetNext(listPos);
        folderName.Format("%s", (LPCSTR)folderInfo->folderName);
        hTItem = AddOneItem(hTRootB, (LPTSTR)(LPCTSTR)folderName, (HTREEITEM)TVI_LAST, 2 * (i + 1));
        SetFileItemImage(hTItem, folderFlag);
        i++;

        // Also fill in the file information in the folder, if any...
        POSITION folderListPos = folderInfo->m_fileList.GetHeadPosition();
        while (folderListPos != NULL) {
            CScannerFileInfo *fileInfo = folderInfo->m_fileList.GetNext(folderListPos);
            fileFullName.Format("%s.%s", (LPCSTR)fileInfo->fileName, (LPCSTR)fileInfo->fileSubfix);

            hTItem2 = AddOneItem(hTItem, (LPTSTR)(LPCTSTR)fileFullName, (HTREEITEM)TVI_LAST, 2 * (i + 1));
            SetFileItemImage(hTItem2, fileInfo->fileSubfix);
            i++;
        }
        if (folderInfo->m_fileList.GetSize() > 0)
            m_fileTree.Expand(hTItem, TVE_EXPAND);
    }

    m_fileTree.Expand(hTRootA, TVE_EXPAND);
    m_fileTree.Expand(hTRootB, TVE_EXPAND);

    m_busy = false;
    return 0;
}

void CFileTransferDlg::SetFilter(TCHAR *filterText)
{
    CString subfix = m_selItemText.Right(3);

    if (subfix = _T("PAK"))
        sprintf(filterText, "Compressed spectrum file\0*.pak\0\0");
    else if (subfix = _T("EXE"))
        sprintf(filterText, "Executable file\0*.exe\0\0");
    else
        sprintf(filterText, "%s file\0*.%s\0\0", (LPCSTR)subfix);
}

void CFileTransferDlg::OnDownloadFile()
{
    int ret;
    Common common;
    CString filePath, directory, fileSubfix;

    // check if we're busy doing something else...
    if (m_busy) {
        MessageBox("Please wait for current transfer to finish");
        return;
    }

    // Get the currently selected scanner
    int curScanner = m_scannerList.GetCurSel();
    if (curScanner < 0)
        return;

    // Check if this is a disk
    if (Equals(m_remoteFileName, "disk A") || Equals(m_remoteFileName, "disk B")) {
        // this is a disk
        return OnDownloadDisk();
    }

    // Check if this is a file or a folder...
    POSITION pos = m_folderListB.GetHeadPosition();
    while (pos != NULL) {
        CScannerFolderInfo *folderInfo = m_folderListB.GetNext(pos);
        if (Equals(m_remoteFileName, folderInfo->folderName)) {
            return OnDownloadFolder(); // this is a folder, call the function to downloads folders instead
        }
    }

    // Ask the user where to save the file
    TCHAR subfix[56];
    SetFilter(subfix);
    char *pt = strstr(m_remoteFileName, "\\");
    if (pt != NULL) {
        filePath.Format("%s", pt + 1);
    }
    else {
        filePath.Format("%s", m_remoteFileName);
    }
    common.BrowseForFile_SaveAs(subfix, filePath);
    m_localFileFullName = filePath;
    if (strlen(filePath) <= 0)
        return;

    // Get the directory that the user chose
    directory.Format(filePath);
    common.GetDirectory(directory);
    m_storagePath = directory;
    // Remove the file, if it already exists
    if (IsExistingFile(filePath))
    {
        ret = MessageBox("There is already a file with same name in the folder. Are you sure to overite the file?", NULL, MB_YESNO);
        if (ret == IDYES)
        {
            if (0 == DeleteFile(filePath))
            {
                MessageBox("Could not delete already existing file. Cannot replace the file. Please save as another file name.");
                return;
            }
        }
        else
            return;
    }
    if (m_diskName[0] == 'Z')
        return;

    m_curScanner = curScanner;
    if (m_communicationType == SERIAL_CONNECTION)
        AfxBeginThread(DownloadFileWithSerial, this, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
    else
        AfxBeginThread(DownloadFileWithFTP, this, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
}

void CFileTransferDlg::OnDownloadFolder()
{
    Common common;
    CString localFolderName;

    // Check if we're busy downloading something else...
    if (m_busy) {
        MessageBox("Please wait for current transfer to finish");
        return;
    }

    // Get the currently selected scanner
    int curScanner = m_scannerList.GetCurSel();
    if (curScanner < 0)
        return;

    // Ask the user where to store the folder
    if (!common.BrowseForDirectory(localFolderName)) {
        return;
    }

    // Add the name of the folder to the local folder name
    localFolderName.AppendFormat("\\%s\\", m_remoteFileName);

    // Try to create the directory that the user wants...
    if (CreateDirectoryStructure(localFolderName)) {
        MessageBox("Could not create local directory");
        return;
    }

    m_storagePath.Format(localFolderName);
    m_curScanner = curScanner;
    if (m_communicationType == SERIAL_CONNECTION)
        AfxBeginThread(DownloadFolderWithSerial, this, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
    else
        AfxBeginThread(DownloadFolderWithFTP, this, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
}

void CFileTransferDlg::OnDownloadDisk()
{
    Common common;
    CString localFolderName;

    MessageBox("Sorry, this function is not yet implemented");
    return;

    // Check if we're busy downloading something else...
    if (m_busy) {
        MessageBox("Please wait for current transfer to finish");
        return;
    }

    // Get the currently selected scanner
    int curScanner = m_scannerList.GetCurSel();
    if (curScanner < 0)
        return;

    // Ask the user where to store the data
    if (!common.BrowseForDirectory(localFolderName)) {
        return;
    }

    // Add the name of the folder to the local folder name
    localFolderName.AppendFormat("\\%s\\", m_remoteFileName);

    // Try to create the directory that the user wants...
    if (CreateDirectoryStructure(localFolderName)) {
        MessageBox("Could not create local directory");
        return;
    }

    m_storagePath.Format(localFolderName);
    m_curScanner = curScanner;
    if (m_communicationType == SERIAL_CONNECTION) {
        //	AfxBeginThread(DownloadDiskWithSerial,this,THREAD_PRIORITY_NORMAL,0,0,NULL);
    }
    else {
        //	AfxBeginThread(DownloadDiskWithFTP,this,THREAD_PRIORITY_NORMAL,0,0,NULL);
    }
}

void CFileTransferDlg::OnUploadFile()
{
    Common common;
    CString   name, diskName;

    // Check if we're busy downloading something else...
    if (m_busy) {
        MessageBox("Please wait for current transfer to finish");
        return;
    }

    // Get the currently selected scanner
    m_curScanner = m_scannerList.GetCurSel();
    if (m_curScanner < 0)
        return;

    m_uploadFileEdit.SetWindowText(m_fileToUpload);
    if (m_fileToUpload.GetLength() <= 3)
        return;

    // Get the directory that the user chose
    m_storagePath = m_fileToUpload;
    common.GetDirectory(m_storagePath);

    // Get the filename that the user chose
    name.Format(m_fileToUpload);
    common.GetFileName(name);
    sprintf(m_remoteFileName, name);
    //set the disk name
    m_diskCombo.GetWindowText(diskName);
    sprintf(m_diskName, diskName);

    m_fileStatusEdit.SetWindowText("Begin to upload file");
    if (m_communicationType == SERIAL_CONNECTION)
        AfxBeginThread(UploadFileWithSerial, this, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
    else if (m_communicationType == FTP_CONNECTION)
        AfxBeginThread(UploadFileWithFTP, this, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
}



void CFileTransferDlg::OnTvnSelchangedFileTree(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
    // get the selected file name and its disk name

    HTREEITEM hSelItem, hRootItem, hParentItem;
    CString itemText, parentText, rootText, msg;
    hSelItem = m_fileTree.GetSelectedItem();
    hParentItem = m_fileTree.GetParentItem(hSelItem);

    itemText = m_fileTree.GetItemText(hSelItem);
    parentText = m_fileTree.GetItemText(hParentItem);

    m_selItemText = itemText;

    if (-1 == parentText.Find("Disk") && -1 == itemText.Find("Disk")) {
        sprintf(m_remoteFileName, "%s\\%s", (LPCSTR)parentText, (LPCSTR)itemText);

        hRootItem = m_fileTree.GetParentItem(hParentItem);
    }
    else {
        sprintf(m_remoteFileName, itemText);

        hRootItem = hParentItem;
    }

    if (hRootItem != NULL)
    {
        rootText = m_fileTree.GetItemText(hRootItem);
        sprintf(m_diskName, "%s", (LPCSTR)rootText.Right(1));
    }
    *pResult = 0;
}
long CFileTransferDlg::GetListFileSize(CString& inFileName, char disk)
{
    long fileSize = 0;
    POSITION listPos;

    CScannerFileInfo* fileInfo;
    fileInfo = new CScannerFileInfo();
    if (disk == 'B')
        listPos = m_fileListB.GetHeadPosition();
    else
        listPos = m_fileListA.GetHeadPosition();
    while (listPos != NULL)
    {
        if (disk == 'B')
            fileInfo = m_fileListB.GetNext(listPos);
        else
            fileInfo = m_fileListA.GetNext(listPos);
        if (inFileName.Find(fileInfo->fileName) != -1)
        {
            fileSize = fileInfo->fileSize;
            return fileSize;
        }
    }
    return fileSize;
}
void CFileTransferDlg::FillList(CString &token, CList <CScannerFileInfo*, CScannerFileInfo*> &list)
{
    CString resToken;
    int folderNameLength;
    int curPos = 0;
    int count = 0;
    CScannerFileInfo* fileInfo = NULL;
    CScannerFolderInfo *folderInfo = NULL;
    CString folderName, emptyName1, emptyName2;
    emptyName1.Format(".");
    emptyName2.Format("..");

    // add in folder info
    if (token.Find("<DIR>") != -1)
    {
        folderNameLength = token.Find(" ");
        if (folderNameLength <= 0)
            return;
        folderName.Format("%s", (LPCSTR)token.Left(folderNameLength));
        if (Equals(folderName, emptyName1) || Equals(folderName, emptyName2))
            return;

        folderInfo = new CScannerFolderInfo();
        folderInfo->folderName = folderName;

        m_folderListB.AddTail(folderInfo);
        return;

    }

    //add in file info
    fileInfo = new CScannerFileInfo();
    do
    {
        resToken = token.Tokenize(" \t", curPos);
        if (resToken.GetLength() <= 0)
            break;
        switch (count)
        {
        case 0: fileInfo->fileName = resToken;
            break;
        case 1: fileInfo->fileSubfix = resToken;
            break;
        case 2: fileInfo->fileSize = atoi(resToken);
            break;
        case 3: fileInfo->date = resToken;
            break;
        case 4: fileInfo->time = resToken;
            break;
        }
        count++;
        if (count == 5)
            break;
    } while (resToken != "");

    if (count >= 3) {
        list.AddTail(fileInfo);
    }
    else {
        delete fileInfo;
    }
}

HTREEITEM CFileTransferDlg::AddOneItem(HTREEITEM hParent, LPSTR szText,
    HTREEITEM hInsAfter, int iImage)
{
    HTREEITEM hItem;
    TVITEM tvI;            // tree control structures	
    TVINSERTSTRUCT tvIns;
    //	
    //  set mask to text and images	
    //	
    tvI.mask = TVIF_TEXT | TVIF_IMAGE;
    tvI.iImage = iImage;  // associate the image	
    tvI.pszText = szText; // associate the text	
    tvI.cchTextMax = lstrlen(szText);
    //	
    tvIns.item = tvI;
    tvIns.hInsertAfter = hInsAfter;     // where to insert	
    tvIns.hParent = hParent;
    //	
    // insert this item into the tree	
    //	
    hItem = m_fileTree.InsertItem(&tvIns);
    //	
    //    make selected image index+1 	
    //	
    m_fileTree.SetItemImage(hItem, iImage, (iImage + 1));
    return (hItem);
}   // end AddOneItem()

// Release the memory allocated to arrays, preventing memory leak
void CFileTransferDlg::ClearOut()
{
    POSITION listPos;
    //delete list A
    listPos = m_fileListA.GetHeadPosition();
    while (listPos != NULL)
    {
        delete m_fileListA.GetNext(listPos);
    }

    //delete list B
    listPos = m_fileListB.GetHeadPosition();
    while (listPos != NULL)
    {
        delete m_fileListB.GetNext(listPos);
    }

    // delete folder lists
    listPos = m_folderListB.GetHeadPosition();
    while (listPos != NULL)
    {
        CScannerFolderInfo *info = m_folderListB.GetNext(listPos);
        POSITION listPos2 = info->m_fileList.GetHeadPosition();
        while (listPos2 != NULL) {
            delete info->m_fileList.GetNext(listPos2);
        }
        info->m_fileList.RemoveAll();
        delete info;
    }

    m_fileListA.RemoveAll();
    m_fileListB.RemoveAll();
    m_folderListB.RemoveAll();
}

bool CFileTransferDlg::DisconnectPrevScanner(int scanner)
{
    int preCommunication;

    if (scanner < 0)
        return false;
    CConfigurationSetting::CommunicationSetting &comm = g_settings.scanner[scanner].comm;
    preCommunication = comm.connectionType;
    switch (preCommunication)
    {
    case SERIAL_CONNECTION:
        m_SerialController->Bye();
        m_SerialController->CloseSerialPort();
        m_SerialController = NULL;
        ShowMessage("Disconnect previous serial connection");
        break;
    case FTP_CONNECTION:
        m_ftpController->Disconnect();
        m_ftpController = NULL;
        ShowMessage("Disconnect previous FTP connection");
        break;
    default:
        break;
    }

    return true;
}
//click the scanner list, connect to specified scanner
void CFileTransferDlg::OnLbnSelchangeScannerList()
{
    CString admUserName, admPwd;
    m_click = m_click + 1;
    // judge the connecting method first
    int curScanner = m_scannerList.GetCurSel();
    if (m_click > 1)
    {
        m_preScanner = m_curScanner;
    }
    m_curScanner = curScanner;

    admUserName.Format("administrator");
    admPwd.Format("1225");

    //disconnnect the previous scanner
    if (curScanner < 0)
    {
        return;
    }

    //disconnect the previous connection
    if (m_preScanner != curScanner)
    {
        DisconnectPrevScanner(m_preScanner);
    }

    //empty the file tree.
    m_fileTree.DeleteAllItems();
    CConfigurationSetting::CommunicationSetting &comm = g_settings.scanner[curScanner].comm;
    m_communicationType = comm.connectionType;
    switch (m_communicationType)
    {
    case SERIAL_CONNECTION:

        m_SerialController = new Communication::CSerialControllerWithTx(g_settings.scanner[curScanner].electronicsBox);
        m_SerialController->SetSerialPort(curScanner, comm.port, comm.baudrate,
            NOPARITY, 8, ONESTOPBIT, comm.flowControl);

        if (comm.medium == MEDIUM_FREEWAVE_SERIAL_MODEM)
        {
            m_SerialController->SetModem(comm.radioID);
        }
        AfxBeginThread(DownloadFileListWithSerial, this, THREAD_PRIORITY_NORMAL, 0, 0, NULL);

        break;
        //when it is ftp connection
    case FTP_CONNECTION:
        m_ftpController = new Communication::CFTPHandler();

        m_ftpController->SetFTPInfo(curScanner,
            g_settings.scanner[curScanner].comm.ftpHostName,
            g_settings.scanner[curScanner].comm.ftpUserName,
            g_settings.scanner[curScanner].comm.ftpPassword,
            admUserName,
            admPwd,
            g_settings.scanner[curScanner].comm.timeout / 1000);
        AfxBeginThread(DownloadFileListWithFTP, this, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
        break;
    default:
        break;
    }
}
int CFileTransferDlg::ParseDir(CString line, CList <CScannerFileInfo*, CScannerFileInfo*> &list)
{
    CString resToken, str, subToken;
    CString tmpStr;
    int curPos = 0;
    int beginFlag = 0;
    int round = 0;
    int tokenLength = 0;
    int index;
    int lineLength = line.GetLength();

    // Remove any unwanted junk in the end...
    index = line.Find("bytes free");
    if (index >= 0) {
        line = line.Left(index + 10);
        lineLength = line.GetLength();
    }

    // Remove any unwanted junk in the beginning...
    index = line.Find("mksh>");
    while (index >= 0) {
        line = line.Right(lineLength - index - 5);
        lineLength = line.GetLength();
        index = line.Find("mksh>");
    }
    index = line.Find("Directory of");
    while (index >= 0) {
        line = line.Right(lineLength - index - 16);
        lineLength = line.GetLength();
        index = line.Find("Directory of");
    }

    do
    {
        resToken = line.Tokenize("\n", curPos);
        tokenLength = resToken.GetLength();
        resToken.Trim();
        if (resToken.Find("files use") != -1)
            break;
        if (resToken.GetLength() > 0)
        {
            FillList(resToken, list);
            round++;
        }
    } while (tokenLength != 0);
    return round;
}

void CFileTransferDlg::SetFileItemImage(HTREEITEM hItem, CString subfix)
{
    if (subfix == _T("TXT") || subfix == _T("INI"))
        m_fileTree.SetItemImage(hItem, 1, 1);
    else if (subfix == _T("EXE"))
        m_fileTree.SetItemImage(hItem, 2, 2);
    else if (subfix == _T("PAK"))
        m_fileTree.SetItemImage(hItem, 3, 3);
    else if (subfix == _T("DIR"))
        m_fileTree.SetItemImage(hItem, 5, 5);
    else
        m_fileTree.SetItemImage(hItem, 4, 4);
}
void CFileTransferDlg::OnDeleteFile()
{
    int ret = MessageBox("Are you sure to delete the file?", NULL, MB_YESNO);
    if (ret == IDYES)
    {
        if (m_communicationType == SERIAL_CONNECTION)
            AfxBeginThread(DeleteFileBySerial, this, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
        else if (m_communicationType == FTP_CONNECTION)
            AfxBeginThread(DeleteFileByFTP, this, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
    }

}
void CFileTransferDlg::OnViewFile()
{
    if (m_communicationType == SERIAL_CONNECTION)
        AfxBeginThread(ViewFileBySerial, this, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
    else if (m_communicationType == FTP_CONNECTION)
        AfxBeginThread(ViewFileByFTP, this, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
}

/** Called to expand the contents of a folder on the remote PC */
void CFileTransferDlg::OnExpandFolder() {
    if (m_communicationType == SERIAL_CONNECTION)
        AfxBeginThread(ExpandFolderBySerial, this, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
    else if (m_communicationType == FTP_CONNECTION)
        AfxBeginThread(ExpandFolderByFTP, this, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
}

void CFileTransferDlg::OnBnClickedBrowseButton()
{
    // Browse folders to choose one file
    Common common;
    common.BrowseForFile("", m_fileToUpload);
    m_uploadFileEdit.SetWindowText(m_fileToUpload);
}

void CFileTransferDlg::EmptyFileArray()
{
    if (!m_fileListA.IsEmpty())
        m_fileListA.RemoveAll();
    if (!m_fileListB.IsEmpty())
        m_fileListB.RemoveAll();
}
void CFileTransferDlg::OnBnClickedOk()
{
    // TODO: Add your control notification handler code here
//	CloseSerialPort();
    DisconnectPrevScanner(m_curScanner);
    OnOK();
}

void CFileTransferDlg::OnBnClickedUpdatefilebtn()
{
    char* buffer = 0;
    CString fileName;

    int ret = MessageBox("Are you sure to update the remote file?", NULL, MB_YESNO);
    if (ret != IDYES)
        return;
    fileName.Format("%s%s", (LPCSTR)m_storagePath, (LPCSTR)m_remoteFileName);
    //get current selected scanner
    m_curScanner = m_scannerList.GetCurSel();
    if (m_curScanner < 0)
        return;
    //Save to local file
    FILE *f = fopen(fileName, "w");
    if (f == NULL)
    {
        m_fileStatusEdit.SetWindowText("Can not opoen local file");
        return;
    }
    int fileLength = m_fileContentEdit.GetWindowTextLength() + 1;
    if (fileLength < 2)
    {
        fclose(f);
        return;
    }
    buffer = (char*)malloc(fileLength);
    m_fileContentEdit.GetWindowText(buffer, fileLength);
    fprintf(f, "%s", buffer);
    fclose(f);
    free(buffer);
    //update file
    if (m_communicationType == SERIAL_CONNECTION)
        AfxBeginThread(UpdateFileWithSerial, this, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
    else if (m_communicationType == FTP_CONNECTION)
        AfxBeginThread(UpdateFileWithFTP, this, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
}

void CFileTransferDlg::ShowDlgInfo(CString& msg)
{
    m_fileStatusEdit.SetWindowText(msg);
}

void CFileTransferDlg::ShowFileContent(CString& fileName)
{
    CString string;
    char* buffer = 0;
    //get file size	
    long fileLength = Common::RetrieveFileSize(fileName);
    //allocate buffer
    buffer = (char*)malloc(fileLength + 1);
    // read file content
    FILE *f = fopen(fileName, "r");
    if (f == NULL)
    {
        if (buffer)
            free(buffer);
        return;
    }
    while (fgets(buffer, fileLength, f) != NULL)
    {
        string.AppendFormat("%s\r\n", buffer);
        memset(buffer, 0, fileLength * sizeof(char));
    }
    fclose(f);
    m_fileContentEdit.SetWindowText(string);	// show text in content
    //free memory
    if (buffer)
        free(buffer);
}
bool CFileTransferDlg::IsTextFile(const CString &fileName)
{
    if (strlen(fileName) < 4)
        return false;

    CString ending;
    ending.Format(fileName.Right(3));

    if (Equals(ending, "txt") || Equals(ending, "bat") || Equals(ending, "ini")) {
        return true;
    }
    return false;
}
bool CFileTransferDlg::IsDir(const CString &fileName)
{
    if (-1 != fileName.Find('.')) {
        return false;
    }

    if (Equals(fileName.Left(1), "R"))
        return true;

    return false;
}