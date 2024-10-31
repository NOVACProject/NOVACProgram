#include "StdAfx.h"
#include "serialcontrollerwithtx.h"
#include "FTPCom.h"
#include "..\Common\ASCII.h"
#include "../Common/CfgTxtFileHandler.h"
#include "../Configuration/configuration.h"
#include "atlstr.h"


#define SIZEOFBIGBUFFER 65536
#define MAXCHUNK 4096
#define NOSTDIO -1
#define USER 1
#define SHELL 3
#define BOTH 2
#define SWITCHMODE 0x06
#define COMMANDLENGTH 256
#define NOTXZMFILE -2
#define NOSERIALPORT -3
#define NOSWITCHSHELL -4
#define NOSTARTEXE -5
#define FAILDOWNLOADSPEC -6

using namespace Communication;

extern CFormView* pView;
extern CConfigurationSetting g_settings;

CSerialControllerWithTx::CSerialControllerWithTx(void)
{
    CString errorMsg;

    pakFileHandler = new FileHandler::CPakFileHandler(m_log);
    m_storageDirectory.Format("%sTemp\\", (LPCSTR)g_settings.outputDirectory);
    if (CreateDirectoryStructure(m_storageDirectory)) { // Make sure that the storage directory exists
        GetSysTempFolder(m_storageDirectory);
        if (CreateDirectoryStructure(m_storageDirectory)) {
            errorMsg.Format("Could not create storage-directory for serial-data!! Please check settings and restart");
            MessageBox(NULL, errorMsg, "Serious error", MB_OK);
        }
    }
    m_commandMode = false;
    m_sleepFlag = false;
    m_radioID.Format("");
    m_electronicsBox = BOX_AXIS;
}


CSerialControllerWithTx::CSerialControllerWithTx(ELECTRONICS_BOX box) {

    CString errorMsg;

    pakFileHandler = new FileHandler::CPakFileHandler(m_log);
    m_storageDirectory.Format("%sTemp\\", (LPCSTR)g_settings.outputDirectory);
    if (CreateDirectoryStructure(m_storageDirectory)) { // Make sure that the storage directory exists
        errorMsg.Format("Could not create storage-directory for serial-data!! Please check settings and restart");
        MessageBox(NULL, errorMsg, "Serious error", MB_OK);
    }
    m_commandMode = false;
    m_sleepFlag = false;
    m_radioID.Format("");
    m_electronicsBox = box;
}

CSerialControllerWithTx::~CSerialControllerWithTx(void)
{
}

void CSerialControllerWithTx::SetSerialPort(int serialID, int COMPort, int baudrate, int parity, int length, int stopBit, int flowControl)
{
    CString errorMsg;

    m_mainIndex = serialID;
    m_Port.baudrate = baudrate;
    m_Port.COMPort = COMPort;
    m_Port.length = length;
    m_Port.parity = parity;
    m_Port.stopBit = stopBit;
    if (flowControl == 1)
    {
        m_Port.fRTS = RTS_CONTROL_ENABLE;
        m_Port.fCTS = TRUE;
    }
    else
    {
        m_Port.fRTS = RTS_CONTROL_DISABLE;
        m_Port.fCTS = FALSE;
    }
    m_connectionID.Format("Serial %d", m_mainIndex);
    m_spectrometerSerialNumber.Format("%s", (LPCSTR)g_settings.scanner[m_mainIndex].spec[0].serialNumber);
    m_timeout = g_settings.scanner[m_mainIndex].comm.timeout;
    m_interval = g_settings.scanner[m_mainIndex].comm.queryPeriod;

    // Make one directory for each instrument...
    m_storageDirectory.Format("%sTemp\\%s\\", (LPCSTR)g_settings.outputDirectory, (LPCSTR)m_spectrometerSerialNumber);
    if (CreateDirectoryStructure(m_storageDirectory)) { // Make sure that the storage directory exists
        GetSysTempFolder(m_storageDirectory);
        if (CreateDirectoryStructure(m_storageDirectory)) {
            errorMsg.Format("Could not create storage-directory for serial-data!! Please check settings and restart");
            MessageBox(NULL, errorMsg, "Serious error", MB_OK);
        }
    }

}

void CSerialControllerWithTx::SetDefaultPort()
{
    sourceBufferPointer = 0;
    //	m_mainIndex		= 0;
    m_Port.baudrate = 115200;
    m_Port.COMPort = 1;
    m_Port.length = 8;
    m_Port.parity = 0;
    m_Port.stopBit = ONESTOPBIT;
    m_Port.fRTS = RTS_CONTROL_ENABLE;
    m_Port.fCTS = TRUE;
}

int CSerialControllerWithTx::InitCommandLine()
{
    char command[2];
    command[0] = CR;
    command[1] = LF;
    if (!WriteSerial(command, 2))
        return FALSE;
    return TRUE;
}

void CSerialControllerWithTx::SendCommand(BYTE command)
{
    char commandText[1];
    commandText[0] = command;
    WriteSerial(commandText, 1);
}

void CSerialControllerWithTx::SendCommand(char* commandLine)
{
    CString command;
    command.Format("%s", commandLine);
    int commandLength = command.GetLength();
    WriteSerial(commandLine, commandLength);
    InitCommandLine();
}

//size of buffer to receive data
#define SIZEOFBUFFER 2048

bool CSerialControllerWithTx::StartTx()
{
    char recBuf[SIZEOFBUFFER];
    CString timeTxt;
    int i;
    //quit tx
    Bye();
    Sleep(5000);
    // Check reply from remote PC, judge whether tx is started
    for (i = 0; i < 5; i++)	//avoid tx loop 2007.4.30
    {
        InitCommandLine(); // to avoid "qtx"
        SendCommand("a:\\tx");
        GetSerialData(recBuf, SIZEOFBUFFER, 1000);
        if (strstr(recBuf, "started") != NULL)
        {
            m_ErrorMsg.Format("successfully started tx.exe");
            ShowMessage(m_ErrorMsg, m_connectionID);
            return true;
        }
    }
    m_ErrorMsg.Format("Start tx failed");
    ShowMessage(m_ErrorMsg, m_connectionID);
    return false;
}

BOOL CSerialControllerWithTx::IsTxStarted(long timeout)
{
    char txt[8];

    SendCommand(TX_HELLO);
    txt[0] = 0;
    if (CheckSerial(m_timeout))
        ReadSerial(txt, 1);

    if (txt[0] == TX_ACK)
    {
        m_ErrorMsg.Format(" Remote PC responsed");
        ShowMessage(m_ErrorMsg, m_connectionID);
        return TRUE;
    }
    else
    {
        m_ErrorMsg.Format("Can not get response from remote PC");
        ShowMessage(m_ErrorMsg, m_connectionID);
        return FALSE;
    }
}
void CSerialControllerWithTx::Bye()
{
    SendCommand(TX_QUIT);
    Sleep(1000);
    InitCommandLine();
}

// Resynchronize the tx communication
RETURN_CODE CSerialControllerWithTx::Resync()
{
    int cycle = 0;
    char txt[8];
    FlushSerialPort(1);
    for (cycle = 0; cycle < 2; cycle++)
    {
        FlushSerialPort(100);
        Bye();
        //start tx
        SendCommand("a:\\tx");
        SendCommand(TX_HELLO);
        txt[0] = 0;
        if (CheckSerial(m_timeout))
            ReadSerial(txt, 1);
        if (txt[0] == TX_ACK)
            return SUCCESS;
        else
        {
            if (RestartTx(m_timeout))
                return SUCCESS;
            else
                continue;
        }
    }
    if (txt[0] == TX_ACK)
        return SUCCESS;
    else
    {
        ShowMessage("Resync fail");
        return FAIL;
    }
}

//download file from remote PC
RETURN_CODE CSerialControllerWithTx::GetFile(char* fileName, CString filePath, char diskName)
{
    //	#define maxchunk 8192
    unsigned long maxchunk;
    if (m_electronicsBox == BOX_MOXA) {
        maxchunk = 2048; // This is necessary since it is not possible to download too large chunks using the Axis and the radio modems!!!
    }
    else {
        maxchunk = 8192;
    }
    long tmp;
    unsigned char* mem, ok;
    unsigned long size, retries, sendlen, start, rstart;
    unsigned short rlen, chksum1, chksum2;
    time_t startTime, stopTime;
    CString timeTxt;
    double downloadedSize;
    char fullfileName[64];
    //set the name of the file which will be downloaded
    if (diskName == 'A' && m_electronicsBox != BOX_MOXA)
        sprintf(fullfileName, "%c:\\%s", diskName, fileName);
    else
        sprintf(fullfileName, "%s", fileName);
    m_ErrorMsg.Format("Will download %s", fullfileName);
    ShowMessage(m_ErrorMsg, m_connectionID);

    // The number of downloaded chunks, used to calculate the average speed
    int nChunks = 0;
    m_avgDownloadSpeed = 0;
    double curSpeed = 0.0;

    time(&startTime);


    //get file size for status.dat or other files which are not pak files
    if (strstr(fullfileName, ".pak") == NULL || strstr(fileName, "UPLOAD.PAK") != NULL)
    {
        size = GetFileSize(fullfileName);
    }
    else
    {
        //get file size for pak files 
        //setname is in getfilesize()
        if (!SetName(fileName))
        {
            if (Resync() == FAIL) {
                m_linkStatistics.AppendFailedDownload();
                return FAIL;
            }
            if (!SetName(fileName)) {
                m_linkStatistics.AppendFailedDownload();
                return FAIL;
            }
        }

        size = GetListFileSize(fileName);
    }
    // size can not be too big or too small
    if (size <= 0 || size >= 1048576) {
        m_linkStatistics.AppendFailedDownload();
        return FAIL;
    }
    // allocate enough memory for the file
    if (!(mem = (unsigned char*)malloc(size)))
    {
        ShowMessage("Could not allocate memory to save file");
        return FAIL;
    }
    retries = 0;

    //---- loop to download data -----//
    for (start = 0; start < size;)
    {
        //send start point and data block size to remote PC
        sendlen = size - start;
        if (sendlen > maxchunk)
            sendlen = maxchunk;
        rlen = sendlen;

        FlushSerialPort(100);
        SendCommand(TX_GET);
        if (!WriteSerial(&start, 4))
            ShowMessage("Serial communication may be broken, please check");
        if (!WriteSerial(&rlen, 2))
            ShowMessage("Serial communication may be broken, please check");

        rstart = rlen = 0;
        ok = 1;
        //FlushSerialPort(1);
        // check remote PC's reply
        if (GetSerialData(&rstart, 4, m_timeout) != 4)
        {
            ShowMessage("No rstart");
            ok = 0;
        }
        else if (GetSerialData(&rlen, 2, m_timeout) != 2)
        {
            ShowMessage("No rlen");
            ok = 0;
        }
        else
        {
            tmp = GetSerialData(&mem[start], sendlen, m_timeout);//100);
            if (tmp != sendlen)
            {
                ok = 0;
                m_ErrorMsg.Format("get data size %d", tmp);
                ShowMessage(m_ErrorMsg);
            }
            else if (GetSerialData(&chksum1, 2, m_timeout) != 2)
            {
                ShowMessage("No checksum");
                ok = 0;
            }
        }
        if (start != rstart)
        {
            m_ErrorMsg.Format("Start does not match requested %d!= %d", start, rstart);
            ShowMessage(m_ErrorMsg);
            ok = 0;
        }
        if (rlen != sendlen)
        {
            m_ErrorMsg.Format("Length does not match requested %d!= %d", rlen, sendlen);
            ShowMessage(m_ErrorMsg);
            ok = 0;
        }
        // Resyncronize communication when error happens, max 5 times
        if (!ok)
        {
            retries++;
            if (retries == 5)
            {
                /*if(Resync()== SUCCESS)
                {
                    SetName(fullfileName);
                    retries=0;
                }
                else*///get out of loop 2007.5.14

                m_linkStatistics.AppendFailedDownload();
                return FAIL;	//get out of loop 2007.4.30
            }
        }
        else
        {
            m_ErrorMsg.Format("start=%d len=%d ", rstart, rlen);
            UpdateMessage(m_ErrorMsg);
            // compare checksum to check data validity
            chksum2 = CalcChecksum(sendlen, &mem[start]);
            if (chksum1 != chksum2)
            {
                m_ErrorMsg.Format("Checksum not correct 0x%0x!=0x%0x", chksum1, chksum2);
                ShowMessage(m_ErrorMsg);
                ok = 0;
            }
            // No error in data transfer
            if (ok)
            {
                start += sendlen;
                time(&stopTime);

                m_common.GetDateTimeText(timeTxt);
                downloadedSize = start / 1024.0;
                if (stopTime - startTime > 0.01) {
                    curSpeed = downloadedSize / (stopTime - startTime);
                }
                m_ErrorMsg.Format("<%s>:%s Have downloaded %.1lfKBytes data at %.1f KBytes/second. Duration is %d seconds. %.1lf percent is finished",
                    (LPCSTR)m_connectionID, (LPCSTR)timeTxt, downloadedSize, curSpeed, static_cast<int>(stopTime - startTime), 100.0 * start / size);
                if (nChunks == 0)
                    ShowMessage(m_ErrorMsg);
                else
                    UpdateMessage(m_ErrorMsg);

                m_avgDownloadSpeed += curSpeed;
                ++nChunks;
            }
        }
    }
    //---- end of the loop to download data -----// 
    WriteSpectraFile(mem, size, filePath);
    free(mem);

    // Calculate the average download speed
    m_avgDownloadSpeed /= nChunks;

    // Remember the link-speed
    m_linkStatistics.AppendDownloadSpeed(m_avgDownloadSpeed);

    return SUCCESS;
}
// Write the received data into a file

RETURN_CODE CSerialControllerWithTx::WriteSpectraFile(BYTE* mem, long fileSize, CString filePath)
{
    FILE* localFile = fopen(filePath, "r+b");
    if (localFile == NULL)
    {
        localFile = fopen(filePath, "w+b");
    }
    if (localFile == NULL)
    {
        m_ErrorMsg.Format("Fail to write %s", (LPCSTR)filePath);
        ShowMessage(m_ErrorMsg, m_connectionID);
        return FAIL;
    }
    fseek(localFile, 0, SEEK_END);
    fwrite(mem, fileSize, 1, localFile);
    fclose(localFile);
    m_ErrorMsg.Format("Data is written to %s, file size is %.1lf KBytes", (LPCSTR)filePath, fileSize / 1024.0);
    ShowMessage(m_ErrorMsg, m_connectionID);
    return SUCCESS;

}
void CSerialControllerWithTx::SwitchMode()
{
    char txt[3];

    if (m_electronicsBox == BOX_MOXA)
        return; // never necessary to switch disk for the Axis box

    txt[0] = SWITCHMODE;
    txt[1] = CR;
    txt[2] = LF;
    SendCommand(txt);
    m_ErrorMsg = TEXT("Switch remote PC's mode");
    ShowMessage(m_ErrorMsg, m_connectionID);
}
bool CSerialControllerWithTx::SwitchToDisk(char diskName)
{
    char message[128];

    if (m_electronicsBox == BOX_MOXA) {
        int nChar = 0;
        if (diskName == 'A')
            nChar = sprintf(message, "cd /mnt/flash\r\n");
        else
            nChar = sprintf(message, "cd /mnt/flash/novac\r\n");
        WriteSerial(message, nChar + 2);
        FlushSerialPort(1);
        m_commandMode = true;
    }
    else {
        message[0] = diskName;
        message[1] = ':';
        message[2] = CR;
        message[3] = LF;
        WriteSerial(message, 4);
        FlushSerialPort(1);
        m_commandMode = true;
    }

    return true;
}

RETURN_CODE CSerialControllerWithTx::SwitchToShell()
{
#define SMALLBUFFER 2048
    char receivingBuf[SMALLBUFFER];
    CString string, msg;
    int stdioMode;
    int byteNum;

    if (m_electronicsBox == BOX_MOXA)
        return SUCCESS; // never necessary to switch disk for the Axis box

    msg.Format("Check remote PC´s mode");
    ShowMessage(msg, m_connectionID);
    FlushSerialPort(10);
    Bye();
    InitCommandLine();
    byteNum = ReceiveFile(m_timeout / 2, receivingBuf, SMALLBUFFER);

    string.Format("Remote PC reply: %s", receivingBuf);
    string.Trim();
    memset(receivingBuf, 0, SMALLBUFFER);
    ShowMessage(string, m_connectionID);
    if (string.Find(":\\>") != -1)
    {
        stdioMode = SHELL;//SHELL OR BOTH
        return SUCCESS;
    }
    else
    {
        Bye();
        FlushSerialPort(10);
        SwitchMode();
        ReceiveFile(m_timeout / 2, receivingBuf, SMALLBUFFER);
        string.Format("Remote PC reply: %s", receivingBuf);
        ShowMessage(string, m_connectionID);
        stdioMode = ParseMode(string);
        if (stdioMode == NOSTDIO)
        {
            m_ErrorMsg.Format("Can not switch %s to shell mode, check it by hyperterminal.", (LPCSTR)m_connectionID);
            ShowMessage(m_ErrorMsg);
            return FAIL;
        }
        else if (stdioMode == USER)
        {
            SwitchMode();
            return SUCCESS;
        }
        else
            return SUCCESS;
    }
}

int CSerialControllerWithTx::ParseMode(CString string)
{
    int result[3] = { NOSTDIO,NOSTDIO,NOSTDIO };
    int modeIndex = NOSTDIO;
    CString mode[3] = { "Stdio: User","Stdio: Both","Stdio: Shell" };
    for (int i = 0; i < 3; i++)
    {
        result[i] = string.Find(mode[i]);
        //printf("i=%d,result[i]=%d\n",i,result[i]);
        if (result[i] >= 0)
        {
            modeIndex = i + 1;
            break;
        }
    }
    //	printf("mode= %d\n,1-user,2-both,3-shell\n",modeIndex);
    return modeIndex;

}
RETURN_CODE CSerialControllerWithTx::Test()
{
    //download cfg.txt
    SetDefaultPort();
    if (InitialSerialPort() == 0)
    {
        printf("can not initiate serial port\n");
        return FAIL;
    }

    return SUCCESS;
}
bool CSerialControllerWithTx::Start()
{
    bool downloadResult;
    time_t startTime, stopTime;
    long interval; // left time for this polling
    time(&startTime);
    ShowMessage("start", m_connectionID);
    if (!InitCommunication('B'))
        return false;
    Bye();
    InitCommandLine();
    Sleep(1000);
    GoTo_TopDataDir();
    StartTx();
    downloadResult = DownloadSpectra("UPLOAD.PAK");

    time(&stopTime);

    Sleep(1000);
    // download old upload*.pak files if time permitted	
    interval = g_settings.scanner[m_mainIndex].comm.queryPeriod - static_cast<long>(stopTime + startTime);

    DownloadOldPak(interval);

    return downloadResult;
}

bool CSerialControllerWithTx::UploadFile(CString localPath, char* fileName, char diskName)
{
    bool result;
    CString filePath;
    filePath.Format("%s%s", (LPCSTR)localPath, (LPCSTR)fileName);

    if (!IsTxStarted(m_timeout))
    {
        if (Resync() == FAIL)
        {
            return false;
        }
    }
    if (PutFile(fileName, filePath, diskName))
    {
        m_ErrorMsg.Format("%s is uploaded successfully", fileName);
        result = true;
    }
    else
    {
        m_ErrorMsg.Format("%s was not uploaded", fileName);
        result = false;
    }

    ShowMessage(m_ErrorMsg, m_connectionID);
    return result;
}

bool CSerialControllerWithTx::DownloadFile(const CString& fileName, CString filePath, char diskName, bool delFlag) {
    long length = strlen(fileName);
    if (length <= 0 || length > 2048)
        return false;

    char* fileBuffer = (char*)calloc(length + 2, sizeof(char));
    sprintf(fileBuffer, "%s", (LPCSTR)fileName);

    bool retCode = DownloadFile(fileBuffer, filePath, diskName, delFlag);

    free(fileBuffer);

    return retCode;
}
bool CSerialControllerWithTx::DownloadFile(char* fileName, CString filePath, char diskName, bool delFlag)
{
    CString fileFullName;
    fileFullName.Format("%s%s", (LPCSTR)filePath, (LPCSTR)fileName);

    pView->PostMessage(WM_SCANNER_RUN, (WPARAM) & (m_spectrometerSerialNumber), 0);
    long remoteFileSize = 0;
    if (!CleanLocalFile(fileName))
    {
        return false;
    }
    if (!IsTxStarted(m_timeout))//test if tx is started
    {
        if (!StartTx())
        {
            return false;
        }
    }
    ShowMessage("Start downloading files");
    if (GetFile(fileName, fileFullName, diskName) == FAIL)
    {
        m_ErrorMsg.Format("%s is not available now", fileName);
        ShowMessage(m_ErrorMsg, m_connectionID);
        return false;
    }
    else
    {
        m_ErrorMsg.Format("Finished downloading file %s from %s @ %.1lf kb/s", (LPCSTR)fileName, (LPCSTR)m_spectrometerSerialNumber, m_avgDownloadSpeed);
        ShowMessage(m_ErrorMsg, m_connectionID);
        if (IsExistingFile(fileFullName) == 0)
        {
            m_ErrorMsg.Format("%s is not found in local disk", (LPCSTR)fileFullName);
            ShowMessage(m_ErrorMsg, m_connectionID);
            return false;
        }
    }

    // Tell the world that we've done with one download
    pView->PostMessage(WM_FINISH_DOWNLOAD, (WPARAM)&m_spectrometerSerialNumber, (LPARAM)&m_avgDownloadSpeed);

    return true;
}
int CSerialControllerWithTx::OrganizeFileList(CString folderName, char diskName)
{
    long fileSize = -1;
    int folderNameLen;
    long bufIndex = 0;
    char recBuf[MAXBUFFER];
    char smallBuf[1024];
    memset(recBuf, 0, MAXBUFFER);
    memset(smallBuf, 0, 1024);
    char command[128];
    if (this->m_electronicsBox != BOX_MOXA) {
        sprintf(command, "dir %c:\\%s\\", diskName, (LPCSTR)folderName);
    }
    else {
        sprintf(command, "dir %s", (LPCSTR)folderName);
    }
    int len = 0;
    int loop = 0;
    CString testFile, testFileInfo;
    FlushSerialPort(10);
    InitCommandLine();
    folderNameLen = folderName.GetLength();

    //judge whether is folder
    if (folderNameLen != 4 && folderNameLen != 0)
    {
        m_ErrorMsg.Format("Folder name %s is not 4-character long, invalid folder", (LPCSTR)folderName);
        ShowMessage(m_ErrorMsg, m_connectionID);
        return 0;
    }
    if (folderNameLen == 0)
        m_ErrorMsg.Format("Download file list from root directory");
    else
        m_ErrorMsg.Format("Download file list from %s", (LPCSTR)folderName);
    ShowMessage(m_ErrorMsg, m_connectionID);

    SendCommand(command);

    do
    {
        len = GetSerialData(smallBuf, 1024, m_timeout / 5);
        if (len > 0)
            m_common.FillBuffer(smallBuf, recBuf, bufIndex, len);
        bufIndex = bufIndex + len;
        memset(smallBuf, 0, 1024);
        loop++;
        if (loop == 65)	//get out of loop if this doesnt work or buffer is full. 2007.4.30
            break;

    } while (strstr(recBuf, "bytes free") == NULL);

    // if the folder is not valid, return -10 2007.05.16
    if (strstr(recBuf, "Invalid path") != NULL)
    {
        m_ErrorMsg.Format("The folder %s is not valid", (LPCSTR)folderName);
        ShowMessage(m_ErrorMsg, m_connectionID);
        return -10;
    }
    //if file list is replied
    if (len > 0)
    {
        ArrangeFileList(recBuf, folderName);
    }
    return len;
}

long CSerialControllerWithTx::GetFileSize(char* fileName)
{
    long size;
    char txt[8];
    // query file 
    if (!SetName(fileName))
    {
        if (Resync() == FAIL)
            return FAIL;
        if (!SetName(fileName))
            return FAIL;
    }
    // ask for file size , remote PC should reply file size
    FlushSerialPort(100);
    SendCommand(TX_SIZE);
    if ((GetSerialData(txt, 4, m_timeout) != 4) || strncmp(txt, "size", 4))
    {
        ShowMessage("No file size returned");
        return FAIL;
    }
    if (GetSerialData(&size, 4, m_timeout) != 4)
    {
        ShowMessage("No file size returned");
        return FAIL;
    }

    m_ErrorMsg.Format("File %s size is %d", fileName, size);
    ShowMessage(m_ErrorMsg);

    return size;
}

long CSerialControllerWithTx::ParseFileSize(CString& fileName, char* string)
{
    static char seps[] = "\t\n";
    char* token;
    CString tokenStr;
    token = strtok(string, seps);
    int find = -1;
    long fileSize = -1;
    while (token != NULL)
    {
        /* While there are tokens in "string" */
        tokenStr.Format("%s", token);

        if (tokenStr.Find(fileName) != -1)
        {
            find = 1;
            break;
        }
        /* Get next token: */
        token = strtok(NULL, seps);
    }
    if (find == 1)
    {
        int i = 0;
        CAtlString str = tokenStr;
        CAtlString resToken;
        int curPos = 0;

        resToken = str.Tokenize(" \t", curPos);
        while (resToken != "")
        {
            resToken = str.Tokenize(" \t", curPos);
            if (i == 1)
            {
                fileSize = _tstol(resToken);
                break;
            }
            i++;
        }
    }
    return fileSize;
}

void CSerialControllerWithTx::EnterFolder(const char* folderName)
{
    CString str;
    str.Format("%s", folderName);
    EnterFolder(str);
}

void CSerialControllerWithTx::EnterFolder(const CString& folderName)
{
    char command[32];
    CString folder;
    folder.Format("%s", (LPCSTR)folderName);
    //make sure folder name is not too long
    if (m_electronicsBox == BOX_AXIS && folder.GetLength() > 16)
        return;
    sprintf(command, "cd %s", (LPCSTR)folderName);
    SendCommand(command);
}

bool CSerialControllerWithTx::GoToSleep()
{
    CString timeTxt, msg;
    int nRound = 0;
    CString bufferStr;
    if (m_sleepFlag)
        return SUCCESS;
    struct timeStruct wakeupT = g_settings.scanner[m_mainIndex].comm.wakeupTime;
    // go to root directory
    GoTo_TopDataDir();

    //write command.txt
    MakeCommandFile("pause\npoweroff");
    if (!InitCommunication('B'))
        return true;//to get all sleep later. 2007.5.23
    do
    { //keep it, change later.
        if (UploadFile(m_storageDirectory, "command.txt", 'B'))
        {
            msg.Format("Command.txt is sent. Begin to stop kongo.");
            ShowMessage(msg, m_connectionID);
            Sleep(5000);	//let status.dat have time to react on the commands
        }
        else
            return true;//to get all sleep later. 2007.5.23
        nRound++;
        if (nRound == 5)
        {
            msg = TEXT("Can not stop kongo.");
            ShowMessage(msg, m_connectionID);
            break; // Changed 2007.11.13. To make sure that the program always downloads the old spectra
            //CloseSerialPort();
            //return true;	//to get all sleep later. 2007.5.23
        }
    } while (CheckKongoExit() != 1);
    //CloseSerialPort();
    msg.Format("Have stopped Kongo. Will wake up at %02d:%02d:%02d", wakeupT.hour, wakeupT.minute, wakeupT.second);
    ShowMessage(msg, m_connectionID);
    pView->PostMessage(WM_SCANNER_SLEEP, (WPARAM) & (m_spectrometerSerialNumber), 0);

    DownloadOldPak(14400); //2007.06.12 download old pak files during sleeping time
    m_sleepFlag = true;
    return true;
}
int CSerialControllerWithTx::CheckKongoExit()
{
    //check it by checking the screen print?

    CString filePath;
    filePath.Format("%sSTATUS.DAT", (LPCSTR)m_storageDirectory);
    if (DownloadFile("STATUS.DAT", m_storageDirectory, 'B') == FAIL)
        return FAIL;
    m_statusFileReader.SetWorkingPath(m_storageDirectory);
    if (m_statusFileReader.IsPoweroff() == 1)
    {
        ShowMessage("Kongo has paused");
        return SUCCESS;
    }
    else
    {
        ShowMessage("Kongo has not paused");
        return FAIL;
    }
}

int CSerialControllerWithTx::WakeUp(int mode)
{
    bool result = false;
    bool isConnect = false;
    int i;
    m_ErrorMsg = TEXT("Begin to connect to remote PC");
    ShowMessage(m_ErrorMsg, m_connectionID);

    for (i = 0; i < 3; i++)
    {
        isConnect = InitCommunication('B');
        if (isConnect)
            break;
    }
    if (!isConnect)
    {
        CloseSerialPort();
        m_ErrorMsg.Format("Can not start serial communication, check connection.");
        ShowMessage(m_ErrorMsg, m_connectionID);
        return FAIL;
    }
    if (mode == 0)
        SwitchToShell();

    // Check the type of electronics-box...
    PollElectronicsType();

    //go to root directory
    GoTo_TopDataDir();

    MakeCommandFile("poweron\nresume");	//make command file for resuming kongo
    StartTx();	//start tx for serial file transfering
    UploadFile(m_storageDirectory, "command.txt", 'B');	// upload file to root folder of B: disk

    CloseSerialPort();	//turn off serial

    //delete old file list
    m_oldPakList.RemoveAll();
    m_rFolderList.RemoveAll();

    //set scanner communication status green
    pView->PostMessage(WM_SCANNER_RUN, (WPARAM) & (m_spectrometerSerialNumber), 0);
    m_sleepFlag = false;

    // This is currently being tested.
    // Also download the cfg.txt - file, just for checking some parameters
    //	like motorstepscomp and stepsperround...
    DownloadCfgTxt();

    return SUCCESS;
}

bool CSerialControllerWithTx::Reboot()
{
#define REBOOT_MSG_SIZE 8192

    if (!InitCommunication('B'))
        return false;
    Sleep(5000);//wait for quiting tx
    Bye();
    InitCommandLine();
    SendCommand("del command.txt");
    m_ErrorMsg.Format("Reboot remote PC");
    ShowMessage(m_ErrorMsg, m_connectionID);

    SendCommand("reboot");
    CloseSerialPort();
    Sleep(9000); //wait for rebooting
    SetRemotePCTime();

    return true;
}

// send file name to remote PC
bool CSerialControllerWithTx::SetName(char* name)
{
#define FILE_NAME_LENGTH 17
    char b[FILE_NAME_LENGTH];
    unsigned short chksum;

    // send "n+fileName" to remote PC
    memset(b, 0, FILE_NAME_LENGTH);
    b[0] = TX_NAME;

    for (int i = 1; i < FILE_NAME_LENGTH; i++)
    {
        b[i] = name[i - 1];
    }
    FlushSerialPort(100);
    WriteSerial(b, FILE_NAME_LENGTH);
    // send checksum of the previous string
    chksum = CalcChecksum(FILE_NAME_LENGTH - 1, (unsigned char*)(b + 1));
    WriteSerial(&chksum, 2);
    // Receive reply, 'a' is valid acknowledgement
    if ((GetSerialData(b, 1, m_timeout) != 1) || (b[0] != 'a'))
    {
        ShowMessage("TX can not set file name");
        return false;	//No ACK 
    }
    return true;
}

//Restart tx program in remote PC
bool CSerialControllerWithTx::RestartTx(long timeout)
{
    char txt[256];
    Bye();
    FlushSerialPort(1);
    InitCommandLine();
    SendCommand("a:\\tx");
    txt[0] = 0;
    // judge successful or not by checking the length of the replied data and word
    if (GetSerialData(txt, 256, timeout) < 8 || !strstr(txt, "started"))
    {
        m_ErrorMsg.Format("Can not restart TX in remote PC");
        CloseSerialPort();
        if (!InitCommunication('B'))
            return false;
        Bye();
        InitCommandLine();
        SendCommand("a:\\tx");
        txt[0] = 0;
        // judge successful by checking the length of the replied data and word
        if (GetSerialData(txt, 256, timeout) < 8 || !strstr(txt, "started"))
        {
            CloseSerialPort();
            ShowMessage("Can't restart TX in remote PC", m_connectionID);
            return false;
        }
        else
        {
            ShowMessage("Successfully restarted TX in remote PC", m_connectionID);
            return true;
        }
    }
    ShowMessage("Successfully restarted TX in remote PC", m_connectionID);
    return true;
}

// calculate checksum 
unsigned short CSerialControllerWithTx::CalcChecksum(long bufferLength, unsigned char* buffer)
{
    long j;
    unsigned short chksum;
    for (chksum = 0, j = 0; j < bufferLength; j++)
        chksum += buffer[j];
    return chksum;
}

//upload a file to the remote PC
bool CSerialControllerWithTx::PutFile(char* name, CString fileFullPath, char diskName)
{
#define SEND_SIZE 3584 //512*7
    char nametoset[128];
    char rack;
    FILE* localFile;
    long retries;
    unsigned long rstart, start;
    unsigned short rlen, sendlen, chksum2;
    unsigned char ok;
    time_t startTime, stopTime;

    // High resolution counter
    LARGE_INTEGER lpFrequency, timingStart, timingStop;
    BOOL useHighResolutionCounter = QueryPerformanceFrequency(&lpFrequency);

    // The number of downloaded chunks, used to calculate the average speed
    int nChunks = 0;
    double avgUploadSpeed = 0.0;
    double curSpeed = 0.0;

    m_ErrorMsg.Format("Begin to upload %s", name);
    ShowMessage(m_ErrorMsg, m_connectionID);

    time(&startTime);
    useHighResolutionCounter = QueryPerformanceCounter(&timingStart);

    if (m_electronicsBox == BOX_MOXA) {
        sprintf(nametoset, "/mnt/flash/XYZ");
    }
    else {
        sprintf(nametoset, "%c:\\%s", diskName, name);
    }

    if (!SetName(nametoset))
    {
        if (Resync() == FAIL) {
            m_linkStatistics.AppendFailedUpload();
            return false;
        }
        if (!SetName(nametoset)) {
            m_linkStatistics.AppendFailedUpload();
            return false;
        }
    }
    localFile = fopen(fileFullPath, "rb");
    if (localFile == NULL)
    {
        m_ErrorMsg.Format("Can not open %s", (LPCSTR)fileFullPath);
        ShowMessage(m_ErrorMsg);
        m_linkStatistics.AppendFailedUpload();
        return false;
    }

    unsigned char* mem = (unsigned char*)malloc(SEND_SIZE);
    if (mem == NULL)
    {
        ShowMessage("Could not get enough memory to upload file");
        fclose(localFile);
        return false;
    }
    start = 0;
    sendlen = fread(mem, 1, SEND_SIZE, localFile);
    retries = 0;

    while (sendlen)
    {
        m_ErrorMsg.Format("Will send %d bytes", sendlen);
        ShowMessage(m_ErrorMsg, m_connectionID);
        // calculate the checksum
        chksum2 = CalcChecksum(sendlen, mem);

        //send command to upload file
        FlushSerialPort(100);
        SendCommand(TX_PUT);
        WriteSerial(&start, 4);
        WriteSerial(&sendlen, 2);
        rstart = rlen = 0;
        ok = 1;
        //check replies
        if (GetSerialData(&rstart, 4, m_timeout) != 4) { ShowMessage("No rstart"); ok = 0; }
        else if (GetSerialData(&rlen, 2, m_timeout) != 2) { ShowMessage("No rlen"); ok = 0; }
        else if (GetSerialData(&rack, 1, m_timeout) != 1) { ShowMessage("No ACK"); ok = 0; }
        else if (rack != TX_ACK)
        {
            m_ErrorMsg.Format("Incorrect ACK==(%d %c)", rack, rack);
            ShowMessage(m_ErrorMsg, m_connectionID);
            ok = 0;
        }

        if (start != rstart)
        {
            m_ErrorMsg.Format("Start does not match requested %d!=%d", start, rstart);
            ShowMessage(m_ErrorMsg, m_connectionID);
            ok = 0;
        }
        if (rlen != sendlen)
        {
            m_ErrorMsg.Format("Length does not match requested %d!=%d", sendlen, rlen);
            ShowMessage(m_ErrorMsg, m_connectionID);
            ok = 0;
        }

        if (!ok)
        {
            retries++;
            if (retries > 10)
            {
                if (Resync() == SUCCESS)
                {
                    SetName(name);
                    retries = 0;
                }
            }
        }
        else
        {
            m_ErrorMsg.Format("start=%d length=%d ", start, rlen);
            UpdateMessage(m_ErrorMsg);

            WriteSerial(&chksum2, 2);
            WriteSerial(mem, sendlen);
            rack = 0;
            if (GetSerialData(&rack, 1, m_timeout) != 1)
            {
                ShowMessage("No ACK");
                ok = 0;
            }
            else
            {
                if (rack != 'a')
                {
                    if (rack == TX_ERR) {
                        m_ErrorMsg.Format("Tx returned Error Acknowledgement");
                    }
                    else {
                        m_ErrorMsg.Format("Incorrect ACK==(%d %c)", rack, rack);
                    }
                    ok = 0;
                    ShowMessage(m_ErrorMsg, m_connectionID);
                }
            }
            if (ok)
            {
                start += sendlen;
                sendlen = fread(mem, 1, SEND_SIZE, localFile);
                time(&stopTime);
                useHighResolutionCounter = QueryPerformanceCounter(&timingStop);

                if (useHighResolutionCounter)
                    curSpeed = (start / 1024.0) / ((double)timingStop.LowPart - (double)timingStart.LowPart) / (double)lpFrequency.LowPart;
                else
                    curSpeed = (start / 1024.0) / (stopTime - startTime);

                m_ErrorMsg.Format("%.1lf KBytes/second", curSpeed);
                UpdateMessage(m_ErrorMsg);

                // Calculate the average upload speed
                avgUploadSpeed += curSpeed;
                ++nChunks;
            }
        }
    } //end of while loop
    fclose(localFile);
    free(mem);

    // Remember the speed of the upload
    m_linkStatistics.AppendUploadSpeed(avgUploadSpeed / (double)nChunks);

    // If this is an axis-box then change the file-name to the correct one
    if (m_electronicsBox == BOX_MOXA) {
        // Close tx
        Bye();
        // Rename the file to it's correct filename
        if (diskName == 'A') {
            sprintf(nametoset, "mv /mnt/flash/XYZ /mnt/flash/%s", name);
            SendCommand(nametoset);
        }
        else {
            sprintf(nametoset, "mv /mnt/flash/XYZ /mnt/flash/novac/%s", name);
            SendCommand(nametoset);
        }
    }

    return true;
}

void CSerialControllerWithTx::ExecuteCommand(char* cmd)
{
    m_ErrorMsg.Format("Executing command <%s>", cmd);
    ShowMessage(m_ErrorMsg, m_connectionID);
    FlushSerialPort(1);
    SendCommand(TX_QUIT); // make sure tx.exe is not running
    FlushSerialPort(500);
    SendCommand(cmd);
}
bool CSerialControllerWithTx::Delete(char* fileName, char diskName)
{
    CString msg;
    char buf[512];
    char command[128];
    sprintf(command, "del %c:\\%s", diskName, fileName);

    SendCommand(command);
    GetSerialData(buf, 512, m_timeout / 2);
    if (strstr(buf, "1 files deleted") != NULL)
    {
        // reply "1 files deleted", success
        return true;
    }
    else
        return false;
}

bool CSerialControllerWithTx::DelFile(char* fileName, char diskName)
{
    char txt[2] = { 0 };
    CString msg;
    char fileFullName[64];
    if (diskName == 'A' && this->m_electronicsBox != BOX_MOXA)
        sprintf(fileFullName, "%c:\\%s", diskName, fileName);
    //for B:\R001\U222.pak. long path
    else
        sprintf(fileFullName, "%s", fileName);
    if (!SetName(fileFullName))
    {
        if (Resync() == FAIL)
            return false;
        if (!SetName(fileFullName))
            return false;
    }
    SendCommand("del");
    if (CheckSerial(m_timeout))
        ReadSerial(txt, 1);
    if (txt[0] == 'a')
    {
        msg.Format("Successfully deleted %s", fileFullName);
        ShowMessage(msg);
        return true;
    }
    else
    {
        msg.Format("Can not delete file %s", fileFullName);
        ShowMessage(msg);
        return false;
    }
}

bool CSerialControllerWithTx::CallModem()
{
#define BUFFER_SIZE 1024
    char buf[BUFFER_SIZE];
    char command[24];
    time_t startTime, stopTime;
    int loopCount = 0;
    memset((void*)buf, 0, BUFFER_SIZE * sizeof(char));
    memset((void*)command, 0, 24);

    //------init port----------------------------
    if (InitialSerialPort() == 0)
    {
        ShowMessage("Can not initialize radio link");
        return false;
    }

    if (m_radioID.GetLength() == 1)
        sprintf(command, "ATDT%s", (LPCSTR)m_radioID);
    else
        sprintf(command, "ATD%s", (LPCSTR)m_radioID);

    time(&startTime);
    WriteSerial((void*)command, strlen(command));

#ifndef _DEBUG
    m_ErrorMsg.Format("Have sent command - %s", command);
    ShowMessage(m_ErrorMsg, m_connectionID);
#endif

    do {
        GetSerialData(buf, BUFFER_SIZE, 3000);
        loopCount++;
        time(&stopTime);
        if (stopTime - startTime >= 60) //10 sec timeout
        {
            CloseSerialPort();
            m_ErrorMsg.Format("can not connect to radio link, use time %d seconds", static_cast<int>(stopTime - startTime));
            ShowMessage(m_ErrorMsg, m_connectionID);
            return false;
        }
    } while (strstr(buf, "CONNECT") == NULL);
    m_ErrorMsg.Format("The radio modem %s is connected after %d loops, %d seconds",
        (LPCSTR)m_connectionID, loopCount, static_cast<int>(stopTime - startTime));
    ShowMessage(m_ErrorMsg);
    pView->PostMessage(WM_SCANNER_RUN, (WPARAM) & (m_spectrometerSerialNumber), 0);
    Sleep(10000);
    return true;
}

void CSerialControllerWithTx::SetModem(CString radioID)
{
    SetDTRControl(true);
    m_radioID.Format("%s", (LPCSTR)radioID);
}

bool CSerialControllerWithTx::InitCommunication(char diskName)
{
    ShowMessage("Initialize serial port", m_connectionID);
    if (m_radioID.GetLength() == 0)
    {
        if (InitialSerialPort() == 0)
        {		//post error message,serial port is occupied or wrong setting
            CloseSerialPort();
            ShowMessage("Can not initialize serial port", m_connectionID);
            pView->PostMessage(WM_SCANNER_NOT_CONNECT, (WPARAM) & (m_spectrometerSerialNumber), 0);
            return false;
        }
    }
    else
    {
        if (!CallModem())
            return false;
    }
    sourceBufferPointer = 0;

    Bye();		// quit tx 2007.5.16
    if (!InitCommandLine())
    {
        ShowMessage("Command can not be sent. Check serial connection or power supply for the scanner");
        return false;
    }

    SwitchToDisk(diskName);
    return true;
}

bool CSerialControllerWithTx::CleanLocalFile(char* fileName)
{
    CString localFile;
    localFile.Format("%s%s", (LPCSTR)m_storageDirectory, (LPCSTR)fileName);
    if (IsExistingFile(localFile))
    {
        if (!DeleteFile(localFile))
        {
            m_ErrorMsg.Format("Can not delete old file in local folder - %s", (LPCSTR)localFile);
            ShowMessage(m_ErrorMsg);
            return false;
        }
    }
    return true;
}

bool CSerialControllerWithTx::ArrangeFileList(char* string, CString folder)
{
    int curPos = 0;
    static char seps[] = "\t\n";
    char* token;
    CString tokenStr;

    CString strFileName, strFolder;
    strFileName.Format("U");
    strFolder.Format("<DIR>");
    token = strtok(string, seps);
    //initialize the file sum and folder sum
    m_fileSum = 0;
    m_folderSum = 0;
    if (strstr(string, "0 files uses 0 bytes") != NULL)
    {
        ShowMessage("Empty folder");
        return false;
    }
    while (token != NULL)
    {
        /* While there are tokens in "string" */
        tokenStr.Format("%s", token);
        //find the file name in the list
        if (tokenStr.Find("PAK") != -1)
        {
            if (tokenStr.Find("WORK") == -1)
            {
                AddPakFileInfo(tokenStr);
            }
            else
            {
                if (folder.GetLength() > 0)
                    AddPakFileInfo(tokenStr);
            }
        }
        else if (tokenStr.Find(strFolder) != -1)
        {
            AddFolderInfo(tokenStr);
            m_folderSum++;
        }
        /* Get next token: */
        token = strtok(NULL, seps);
    }
    m_ErrorMsg.Format("Current path has %d pak files, %d folders", m_fileSum, m_folderSum);
    ShowMessage(m_ErrorMsg, m_connectionID);
    return false;
}

// download old pak files
bool CSerialControllerWithTx::DownloadOldPak(long interval)
{
    bool downloadResult = true;
    int fileListSum = 0;
    time_t startTime, stopTime;
    CString fileFullName, folder;
    folder.Format("");
    //start communication
    time(&startTime); //start timer
    Bye();
    InitCommandLine();
    Sleep(1000);
    GoTo_TopDataDir();

    if (m_oldPakList.GetCount() + m_rFolderList.GetCount() <= 0)
    {
        fileListSum = GetOldPakList(folder); //download Uxxx.pak list
        if (fileListSum == 0)
        {
            m_ErrorMsg.Format("There is no file to download");
            ShowMessage(m_ErrorMsg, m_connectionID);
            CloseSerialPort();
            return false;
        }
    }

    time(&stopTime);
    //continue downloading Uxxx.pak files
    interval = interval + static_cast<long>(startTime - stopTime);
    //if there is no time to download more files in current folder,return
    if (interval <= 0)
    {
        CloseSerialPort();
        return false;
    }
    time(&startTime);
    if (m_oldPakList.GetCount() > 0)
        downloadResult = DownloadPakFile(folder, interval);

    time(&stopTime);
    interval = interval + static_cast<long>(startTime - stopTime);
    //if there is no time to download more folders,return
    if (interval <= 0)
    {
        CloseSerialPort();
        return downloadResult;
    }
    // download pak files from folders
    while (m_rFolderList.GetCount() > 0)
    {
        time(&startTime);
        m_oldPakList.RemoveAll();
        folder.Format("%s", (LPCSTR)m_rFolderList.GetTail());
        fileListSum = GetOldPakList(folder);
        if (fileListSum == 0)
        {
            break;
        }
        else if (fileListSum == -10)
        {
            //when the folder is invalid, remove it
            m_rFolderList.RemoveTail();
            continue;
        }
        time(&stopTime);
        interval = interval + static_cast<long>(startTime - stopTime);
        if (interval <= 0)
            break;
        if (DownloadPakFile(folder, interval))
        {
            m_rFolderList.RemoveTail();
            Bye();
            GoTo_TopDataDir();
            DeleteFolder(folder);
        }
        else
            break;
    }
    //stop communication
    CloseSerialPort();
    return downloadResult;
}

bool CSerialControllerWithTx::DeleteFolder(CString folder)
{//out of tx
    char cmd[32];
    if (m_electronicsBox != BOX_MOXA) {
        sprintf(cmd, "rd %s", (LPCSTR)folder);
    }
    else {
        sprintf(cmd, "rmdir %s", (LPCSTR)folder);
    }
    Sleep(1000);
    SendCommand(cmd);
    m_ErrorMsg.Format("delete folder %s", (LPCSTR)folder);
    ShowMessage(m_ErrorMsg, m_connectionID);
    return true;
}

// download file from subfolders
bool CSerialControllerWithTx::DownloadPakFile(CString folderName, long interval)
{
    CString fileName;
    char fileFullName[14];
    bool downloadResult = false;
    time_t startTime, stopTime;

    EnterFolder(folderName);
    while (m_oldPakList.GetCount() > 0)
    {
        //judge if there is still time for the last file at 2k/sec speed
        if (interval > m_oldPakList.GetTail().m_fileSize / 2048)
        {
            time(&startTime);
            fileName.Format("%s.pak", (LPCSTR)m_oldPakList.GetTail().m_fileName);
            sprintf(fileFullName, "%s", (LPCSTR)fileName);

            downloadResult = DownloadSpectra(fileFullName);
            time(&stopTime);
            interval = interval + static_cast<long>(startTime - stopTime);
            if (downloadResult)
                m_oldPakList.RemoveTail();
            else
                return false;
        }
        else
            return false;
    }
    return true;
}

bool CSerialControllerWithTx::CreatePakFileList()
{
    CString listTextB, listTextA, folderName;
    char text[16184];
    folderName.Format("");
    if (!GetFileListText(listTextA, listTextB))
    {
        ShowMessage("Can not get pak file list", m_connectionID);
        return false;
    }
    sprintf(text, "%s", (LPCSTR)listTextB);
    ArrangeFileList(text, folderName);
    return true;
}

void CSerialControllerWithTx::AddFolderInfo(CString string)
{
    int folderNameLength = string.Find(" ");
    if (folderNameLength != 4)
        return;
    CString folderName = string.Left(folderNameLength);
    if (folderName.Left(1) != _T("R"))
        return;
    m_rFolderList.AddTail(folderName);
}

// add the info of uxxx*.pak into list
void CSerialControllerWithTx::AddPakFileInfo(CString string)
{
    CString strSize, fileName;
    int sizeLength, fileSize, stringLength, fileNameLength; // length of the size string 
    stringLength = string.GetLength();
    if (string.Find("PAK") == -1 || stringLength > 42)
        return;

    fileNameLength = string.Find(" ");
    if (fileNameLength > 6)
        return;
    fileName = string.Left(fileNameLength);
    string = string.Right(string.GetLength() - fileNameLength);
    string.TrimLeft(); // Changed 2007.10.22
    if (string.Find("PAK") == -1)
        return;
    string.TrimLeft("PAK");
    string.TrimLeft();
    if (string.Find("A--") != -1)
    {
        string.TrimLeft("A--");
        string.TrimLeft();
    }
    sizeLength = string.Find(" ");
    if (sizeLength < 0)
        return;
    fileSize = atoi(string.Left(sizeLength));
    if (fileSize > 0)
    {
        m_oldPakList.AddTail(CFileInfo(fileName, fileSize));
        m_fileSum++;
    }
}

// fill in the upload*.pak list - m_oldPakList
int CSerialControllerWithTx::GetOldPakList(CString folderName)
{
    int result, fileSum;
    char command[128];
    result = 0;
    //quit tx
    Bye();
    if (m_oldPakList.GetCount() > 0) //?
    {
        m_oldPakList.RemoveAll();
    }
    fileSum = OrganizeFileList(folderName);
    if (fileSum == -10)
        return fileSum;

    result = m_oldPakList.GetCount() + m_rFolderList.GetCount();
    if (result > 0)
    {
        //enter folder 
        if (folderName.GetLength() > 0)
        {
            sprintf(command, "cd %s", (LPCSTR)folderName);
            SendCommand(command);
        }
        if (!StartTx())
        {
            ShowMessage("Can not start TX. Check whether scanner is powered.");
            result = 0; //avoid tx loop
        }
    }
    return result;
}

bool CSerialControllerWithTx::IsKongoRunning()
{
    int counter = 0;
    CString statusFile;
    statusFile.Format("%sSTATUS.DAT", (LPCSTR)m_storageDirectory);
    while (DownloadFile("STATUS.DAT", m_storageDirectory, 'B') != SUCCESS)
    {
        counter++;
        if (counter == 3)
            return false;
    }
    m_statusFileReader.SetWorkingPath(m_storageDirectory);
    if (m_statusFileReader.IsStart() == 1)
    {
        ShowMessage("Kongo is running");
        return true;
    }
    else
    {
        ShowMessage("Kongo is not running");
        return false;
    }
}

bool CSerialControllerWithTx::DownloadSpectra(char* pakFileName)
{
    CString specFile;
    specFile.Format("%s%s", (LPCSTR)m_storageDirectory, (LPCSTR)pakFileName);

    bool downloadResult = DownloadFile(pakFileName, m_storageDirectory, 'B', true);
    if (!downloadResult)
        return false;
    //call pakhandler
    if (1 == pakFileHandler->ReadDownloadedFile(specFile))
    {
        ShowMessage("downloaded file is corrupted", m_connectionID);
        DeleteFile(specFile); // delete the local copy of Upload.pak		
        return false;
    }

    DelFile(pakFileName, 'B');

    return true;
}

bool CSerialControllerWithTx::GetFileListText(CString& textA, CString& textB)
{
    long bufIndex = 0;
    char recBuf[MAXBUFFER];
    char smallBuf[1024];
    memset(recBuf, 0, MAXBUFFER);

    int len = 0;
    int loop = 0;

    //char buffer[SMALLBUFFER];
    //long cycleNumber =0 ;
    //long validIndex = 0;
    // time_t startTime,stopTime;
    //if(!InitCommunication(diskName))
    //	return false;
    //
    Bye();

    SwitchToDisk('B');
    FlushSerialPort(100);

    GoTo_TopDataDir();
    SendCommand("dir");
    // time(&startTime);

    do
    {
        len = GetSerialData(smallBuf, 1024, m_timeout / 5);
        if (len > 0)
            m_common.FillBuffer(smallBuf, recBuf, bufIndex, len);
        bufIndex = bufIndex + len;
        memset(smallBuf, 0, 1024);
        loop++;
        if (loop == 65)	//get out of loop if this doesnt work or buffer is full. 2007.4.30
            break;

    } while (strstr(recBuf, "bytes free") == NULL);
    textB.Format("%s", recBuf);

    //do
    //{
    //	memset(buffer,0,SMALLBUFFER);
    //	validIndex = GetSerialData(buffer,SMALLBUFFER,1000);
    //	buffer[validIndex] = 0;
    //	textB.AppendFormat("%s",buffer);
    //	cycleNumber++;
    //	time(&stopTime);
    //	if(stopTime - startTime > 7)
    //	{
    //		return false;
    //	}
    //	
    //}while(textB.Find("files use") == -1);

    if (m_electronicsBox == BOX_MOXA)
        return true; // On the Axis system, there's no A-disk to check...

    //check A disk
    bufIndex = 0;
    len = 0;
    loop = 0;
    SwitchToDisk('A');
    FlushSerialPort(100);
    InitCommandLine();
    SendCommand("dir");
    // time(&startTime);
    do
    {
        len = GetSerialData(smallBuf, 1024, m_timeout / 5);
        if (len > 0)
            m_common.FillBuffer(smallBuf, recBuf, bufIndex, len);
        bufIndex = bufIndex + len;
        memset(smallBuf, 0, 1024);
        loop++;
        if (loop == 65)	//get out of loop if this doesnt work or buffer is full. 2007.4.30
            break;

    } while (strstr(recBuf, "bytes free") == NULL);
    textA.Format("%s", recBuf);

    //do
    //{
    //	memset(buffer,0,SMALLBUFFER);
    //	validIndex = GetSerialData(buffer,SMALLBUFFER,1000);
    //	buffer[validIndex] = 0;
    //	textA.AppendFormat("%s",buffer);
    //	time(&stopTime);
    //	if(stopTime - startTime > 7)
    //	{
    //		return false;
    //	}
    //	
    //}while(textA.Find("files use") == -1);
    return true;
}

bool CSerialControllerWithTx::GetFileListText_Folder(CString& folderName, CString& text)
{
    char buffer[SMALLBUFFER];
    long cycleNumber = 0;
    long validIndex = 0;
    time_t startTime, stopTime;

    // The command to send...
    char command[128];
    if (this->m_electronicsBox != BOX_MOXA) {
        sprintf(command, "dir B:\\%s\\", (LPCSTR)folderName);
    }
    else {
        sprintf(command, "dir %s", (LPCSTR)folderName);
    }

    Bye();

    SwitchToDisk('B');
    FlushSerialPort(100);

    GoTo_TopDataDir();
    SendCommand(command);
    time(&startTime);
    do
    {
        memset(buffer, 0, SMALLBUFFER);
        validIndex = GetSerialData(buffer, SMALLBUFFER, 1000);
        buffer[validIndex] = 0;
        text.AppendFormat("%s", buffer);
        cycleNumber++;
        time(&stopTime);
        if (stopTime - startTime > 7)
        {
            return false;
        }

    } while (text.Find("files use") == -1);

    return true;
}

void CSerialControllerWithTx::SetRemotePCTime()
{
    char command[128];
    char receiveBuf[SIZEOFBUFFER];
    CString str;
    int year, month, day, hour, minute, second;
    int timeout = g_settings.scanner[m_mainIndex].comm.timeout;
    year = m_common.GetYear();
    month = m_common.GetMonth();
    day = m_common.GetDay();
    hour = m_common.GetHour();
    minute = m_common.GetMinute();
    second = m_common.GetSecond();
    memset(command, 0, 128 * sizeof(char));
    sprintf(command, "SETSTIME %d %d %d %d %d %d",
        year, month, day, hour, minute, second);

    WriteSerial(command, strlen(command));
    InitCommandLine();

    GetSerialData(receiveBuf, SIZEOFBUFFER, timeout);
    str.Format("%s", receiveBuf);
    ShowMessage("Set remote PC time");

    if (str.Find("Not enough memory") != -1)
    {
        m_ErrorMsg.Format("Not enough memory, reboot remote PC");
        ShowMessage(m_ErrorMsg, m_connectionID);
        SendCommand("reboot");
        InitCommandLine();
        Sleep(3000);
    }
    else
    {
        m_ErrorMsg.Format("set remote PC to local PC time %d-%d-%d %02d:%02d:%02d",
        year, month, day, hour, minute, second);
        ShowMessage(m_ErrorMsg, m_connectionID);
    }
    //CloseSerialPort();
}

bool CSerialControllerWithTx::MakeCommandFile(char* content)
{
    CString fileName;
    fileName.Format("%scommand.txt", (LPCSTR)m_storageDirectory);
    FILE* f = fopen(fileName, "w");
    if (f == NULL)
        return false;
    fprintf(f, content);
    fclose(f);
    return true;
}

long CSerialControllerWithTx::GetListFileSize(char* fileName)
{
    POSITION listPos;
    CFileInfo* fileInfo;
    fileInfo = NULL;
    long size = 0;
    CString strFile;
    strFile.Format("%s", fileName);
    listPos = m_oldPakList.GetHeadPosition();
    while (listPos != NULL)
    {
        fileInfo = &m_oldPakList.GetNext(listPos);

        if (strFile.Find(fileInfo->m_fileName) != -1)
        {
            size = fileInfo->m_fileSize;

            break;
        }
    }
    return size;

}

/** Go to the top-most directory of the data-disk */
void CSerialControllerWithTx::GoTo_TopDataDir() {
    if (m_electronicsBox != BOX_MOXA) {
        SendCommand("cd ..");
    }
    else {
        SendCommand("cd /mnt/flash/novac/");
    }
}

/** Gets the type of electronics-box that the instrument uses
        this will set the type of electronics-box that this connection uses
        @return 0 on success else 1 */
int CSerialControllerWithTx::PollElectronicsType() {
    const long bufferSize = 512;
    char buffer[bufferSize];

    // Poll the version of the electronics - box
    SendCommand("ver");

    if (CheckSerial(m_timeout)) {
        ReadSerial(buffer, bufferSize);
        if (strstr(buffer, "BECK")) {
            this->m_electronicsBox = BOX_AXIS;
        }
        else if (strstr(buffer, "-sh: ver")) {
            this->m_electronicsBox = BOX_MOXA;
        }
    }
    else {
        // don't know type of box...
        return 1;
    }

    return 0;
}

int CSerialControllerWithTx::DownloadCfgTxt() {
    CString localFileName, copyFileName;

    // Connect to the A-disk...
    if (!InitCommunication('A'))
        return 0;
    Bye();
    InitCommandLine();
    StartTx();
    bool downloadResult = DownloadFile("cfg.txt", m_storageDirectory, 'A', false);
    CloseSerialPort();

    // Check if we got the file...
    if (!downloadResult)
        return 0;

    // If we managed to download the file, then check it!
    // Try to parse the cfg.txt - file in order to get the paramters...
    FileHandler::CCfgTxtFileHandler cfgTxtReader;
    localFileName.Format("%scfg.txt", (LPCSTR)m_storageDirectory);
    if (0 == cfgTxtReader.ReadCfgTxt(localFileName)) {
        return 0;
    }
    if (cfgTxtReader.m_motorStepsComp[0] != 0) { // If we've found a motor steps comp...
        g_settings.scanner[m_mainIndex].motor[0].motorStepsComp = cfgTxtReader.m_motorStepsComp[0];
        g_settings.scanner[m_mainIndex].motor[0].stepsPerRound = cfgTxtReader.m_motorStepsPerRound[0];
        if (cfgTxtReader.m_nMotors == 2) {
            g_settings.scanner[m_mainIndex].motor[1].motorStepsComp = cfgTxtReader.m_motorStepsComp[1];
            g_settings.scanner[m_mainIndex].motor[1].stepsPerRound = cfgTxtReader.m_motorStepsPerRound[1];
        }
    }

    // if todays date is dividable by 7 then try to upload the file
    int todaysDate = m_common.GetDay();
    if (todaysDate % 7 == 0) {
        // Copy the file to a new name.
        copyFileName.Format("%scfg_%s.txt", (LPCSTR)m_storageDirectory, (LPCSTR)m_spectrometerSerialNumber);
        if (0 != CopyFile(localFileName, copyFileName, FALSE)) {
            // Get the index of this volcano
            int volcanoIndex = Common::GetMonitoredVolcano(m_spectrometerSerialNumber);

            UploadToNOVACServer(copyFileName, volcanoIndex, true);
        }
    }

    return 1;
}
//------------------End of CSerialControllerWithTx ---------------//


