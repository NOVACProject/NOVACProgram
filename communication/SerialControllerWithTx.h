#pragma once
#include "serialcom.h"
#include <afxtempl.h>
#include "../Common/Spectra/PakFileHandler.h"
#include "../StatusFileReader.h"
#include "../Common/Common.h"
#include "../NovacProgramLog.h"
#include "FileInfo.h"
#include "LinkStatistics.h"

#define TX_NAME 'n'
#define TX_PUT 'p'
#define TX_GET 'g'
#define TX_VER 'v'
#define TX_ACK 'a'
#define TX_HELLO 'h'
#define TX_QUIT 'q'
#define TX_SIZE 's'
#define TX_DELETE 'd'
#define TX_ERR 'e'


namespace Communication
{
class CSerialControllerWithTx :
    public CSerialCOM
{
public:
    CSerialControllerWithTx(void);
    CSerialControllerWithTx(ELECTRONICS_BOX box);
    ~CSerialControllerWithTx(void);
    //------------------------------------------------------------//
    //----------------------public functions----------------------//
    //------------------------------------------------------------//

    /**Organize file list returned by "dir" command
    */
    int OrganizeFileList(CString folderName, char diskName = 'B');

    /**Download remote file
    *@param fileName the file to be downloaded
    *@param filePath the path to store the downloaded file
    *@param diskName the disk where the remote file is stored in the remote PC, 'A' or 'B'
    *@param delFlag if delFlag is true, delete the remote file after downloading
    */
    bool DownloadFile(char* fileName, CString filePath, char diskName, bool delFlag = false);
    bool DownloadFile(const CString& fileName, CString filePath, char diskName, bool delFlag = false);

    /**Set port parameters
    *@param serialID serial ID of this serial connection
    *@param COMPort com port number
    *@param baudrate baudrate of the COM port
    *@param parity parity, 0 or 1
    *@param length length of one byte to be sent, 5~8 bits
    *@param stopBit stop bit, 0 or 1
    *@param fRTS RTS setting
    */
    void SetSerialPort(int serialID, int COMPort, int baudrate, int parity, int length, int stopBit, int flowControl);

    /**Set default serial port parameters*/
    void SetDefaultPort();

    /**Send one character command*/
    void SendCommand(BYTE command);

    /**Send command line*/
    void SendCommand(char* commandLine);

    /**Send CR*/
    int InitCommandLine();

    /**Judge whether the remote PC is connected
    *@param timeout the longest waiting time to test connection
    *@return TRUE if it is connected, FALSE if it is not connected
    */
    BOOL IsTxStarted(long timeout);

    /**quit Tx.exe in remote PC*/
    void Bye();

    /**Download file from remote PC
    *@param fileName the file to be downloaded
    *@param filePath
    */
    RETURN_CODE GetFile(char* fileName, CString filePath, char diskName);


    /**Reconnect the remote PC
    *@param fileName the file to be downloaded
    *@param timeout the longest waiting time for connection
    */
    RETURN_CODE Resync();

    /**Start tx.exe in remote PC
    *@param fileName the file to be downloaded
    *@param timeout the longest waiting time for connection
    */
    bool StartTx();

    /**Restart tx, return true if successful*/
    bool RestartTx(long timeout);

    /**Write upload.pak file
    *@param mem the data buffer
    *@param fileSize the size of the file to be writened
    *@param filePath the full path of the file
    */
    RETURN_CODE WriteSpectraFile(BYTE* mem, long fileSize, CString filePath);

    /** Go to the top-most directory of the data-disk */
    void GoTo_TopDataDir();

    /**Send command to switch remote PC mode once*/
    void SwitchMode();

    /**Switch to A:\\ or B:\\ in remote PC
    *@param diskName the disk to be switched to
    */
    bool SwitchToDisk(char diskName);

    /**Switch remote PC to stdio: Shell mode so that command can be received there */
    RETURN_CODE SwitchToShell();

    /**Parse the received string
    * to check in which mode the remote PC is working
    *@param string the string showing stdio mode
    */
    int ParseMode(CString string);

    /** Gets the type of electronics-box that the instrument uses
            this will set the type of electronics-box that this connection uses
            @return 0 on success else 1 */
    int PollElectronicsType();

    /**Get file size from remote pc
    *@param fileName name of the file
    */
    long GetFileSize(char* fileName);

    /**get file  size  from list*/
    long GetListFileSize(char* fileName);

    /**get file size from the "dir" result
    *@param fileName file name
    *@param string the string to be parsed
    */
    long ParseFileSize(CString& fileName, char* string);

    /**reboot the remote pc*/
    bool Reboot();

    RETURN_CODE Test();

    /**Start */
    bool Start();

    /**Go to sleep*/
    virtual bool GoToSleep();

    /**make the command file*/
    bool MakeCommandFile(char* content);

    /**wake up the kongo
    @param - mode - starting the program or during program
    *0 - when start the program
    *>0 after program is started
    */
    int WakeUp(int mode = 1);

    /**Check whether kongo exited
        @return 0 not exit
        @return 1 exit
    */
    int CheckKongoExit();

    /**set the file to be handled in the remote PC*/
    bool SetName(char* name);

    /**Calculate checksum
    *@param bufferLength buffer length
    *@param buffer input buffer
    *return checksum*/
    unsigned short CalcChecksum(long bufferLength, unsigned char* buffer);

    /**upload file
    *@param name file name
    *@param fileFullPath the foldername + file Name
    */
    bool PutFile(char* name, CString fileFullPath, char diskName);

    /**execute remote command
    *@param cmd command in remote PC
    */
    void ExecuteCommand(char* cmd);

    /**delete file in remote PC
    *@param fileName the name of the file in the remote PC, such as cfg.txt
    */
    bool DelFile(char* fileName, char diskName);

    /**initiate serial communication
    *@param diskName the remote disk where the file is stored, 'A' or 'B'
    */
    bool InitCommunication(char diskName);

    /**remove local file
    *@param fileName remove the file in the local PC if exists.
    */
    bool CleanLocalFile(char* fileName);

    /** find the pak file name and pak folder in one line of "dir" result
    *@param one line of text from "dir" result
    */
    bool ArrangeFileList(char* string, CString folder);

    /**download upload*.pak files
    *@param interval the interval (in seconds) to download files
    */
    bool DownloadOldPak(long interval);

    /**Add u*.pak and old work.pak into file list*/
    void AddPakFileInfo(CString string);

    /**Add Rxxx folder into folder list*/
    void AddFolderInfo(CString string);

    /**Get the list of upload*.pak files
    *@param folderName - the folder which contains pak files.
    *return the total file number in the list
    */
    int GetOldPakList(CString folderName);

    /**check whether kongo is running*/
    bool IsKongoRunning();

    bool UploadFile(CString localPath, char* fileName, char diskName);

    bool DownloadSpectra(char* pakFileName);

    bool GetFileListText(CString& textA, CString& textB);
    bool GetFileListText_Folder(CString& folderName, CString& text);

    /**backup method when gps receiver stops working*/
    void SetRemotePCTime();

    /**get file list*/
    bool CreatePakFileList();

    /**download Uxxx.pak file
    **@param folderName - the folder where files are
    **@param interval - for how long time files can be downloaded
    *return true - if finish all downloading
    */
    bool DownloadPakFile(CString folderName, long interval);

    /**delete a folder*/
    bool DeleteFolder(CString folder);

    bool CallModem();

    void SetModem(CString radioID);

    /** enter a folder on the remote pc */
    void EnterFolder(const CString& folderName);
    void EnterFolder(const char* folderName);

    /**use dos command to delete a file */
    bool Delete(char* fileName, char diskName);

    //------------------------------------------------------------//
    //----------------------public variables----------------------//
    //------------------------------------------------------------//
    /**radio ID or callbook ID for radio modem*/
    CString m_radioID;

    /**index in the configuration.xml*/
    int m_mainIndex;

    /** The name of the connection, used for outputting messages. */
    CString m_connectionID;

    /**spectrometer's serial number, can be found by connecting spectrometer with USB cable*/
    CString m_spectrometerSerialNumber;

    /**directory to store downloaded files*/
    CString m_storageDirectory;

    /**downloading interval in configuration.xml*/
    long m_interval;

    /** flag of the command mode in the remote PC */
    bool m_commandMode;

    /** The timeout of the connection. Configured through configuration.xml */
    int m_timeout;

    /** average download-speed for the last downloaded file [kbits/second]*/
    double	m_avgDownloadSpeed;

    /**status messages*/
    CString m_ErrorMsg;
    Common m_common;
    bool m_sleepFlag;
    FileHandler::CPakFileHandler* pakFileHandler;
    FileHandler::CStatusFileReader m_statusFileReader;

    /** list of Uxxx.pak files */
    CList<CFileInfo, CFileInfo&> m_oldPakList;

    /** number of the pak files in current folder*/
    int m_fileSum;

    /** list for Rxxx folders */
    CList<CString, CString&> m_rFolderList;

    /** number of folders in current folder*/
    int m_folderSum;

    /** The kind of electronics box that we're communicating with, good to know... */
    ELECTRONICS_BOX m_electronicsBox;

    /** The statistics for this link */
    CLinkStatistics	m_linkStatistics;

private:

    NovacProgramLog m_log;

    /** This function tries to download cfg.txt from this instrument.
            The cfg.txt file can be used to assess e.g. the motorstepscomp
            used in the instrument
        @return 0 on success otherwise 1 */
    int DownloadCfgTxt();
};


}