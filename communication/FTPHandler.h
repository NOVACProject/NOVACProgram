#pragma once
#include <afxtempl.h>
#include <vector>
#include "../communication/ftpcom.h"
#include "../communication/ftpsocket.h"
#include "../Common/Spectra/PakFileHandler.h"
#include "../Common/Common.h"
#include "../Configuration/configuration.h"
#include "../scannerfileinfo.h"

namespace Communication
{
    struct FTPInformation
    {
        CString hostName; // ip-address or host-name
        CString userName;
        CString password;
        CString adminUserName;
        CString adminPassword;
        long    port;
        int     timeout;
    };

    /** <b>CFTPHandler</b> class handles one link of downloading and uploading file
    *to the remote PC */
    class CFTPHandler :
        public CFTPCom
    {
    public:
        CFTPHandler(void);
        CFTPHandler(ELECTRONICS_BOX box);
        ~CFTPHandler(void);

        //-------------------------------------------//
        //--------------public functions ------------//
        //-------------------------------------------//

        /**set ftp information*/
        void SetFTPInfo(int mainIndex, const CString& hostName, const CString& userName, const CString &pwd, int timeOut, long portNumber = 5551);
        void SetFTPInfo(int mainIndex, const CString& hostName, const CString& userName, const CString &pwd, const CString &admUserName, const CString &admPwd, int timeOut, long portNumber = 5551);

        /**poll one instrument*/
        bool PollScanner();

        // ---------------- MANAGING THE INSTRUMENT ------------------

        /** Send a command to kongo.exe in the instrument
             by creating a command.txt file and upload it to the instrument.
             @return true if the file creation and upload was successful. */
        bool SendCommand(const char* cmd);

        /** Creates a command.txt file to upload to the instrument
            containing the provided command to send.
            @return true if the file could be created successfully. */
        static bool MakeCommandFile(const CString& fileName, const char* command);

        /** Tells the instrument to go to sleep by uploading
             a command.txt file to it */
        void GotoSleep();

        /** Tells the instrument to wake up by uploading
             a command.txt file to it */
        void WakeUp();

        /** reboot the remote scanner */
        void Reboot();

        /** Download cfg.txt from the instrument.
            @return 1 if successful, otherwise 0*/
        int DownloadCfgTxt();

        // --------------- DOWNLOADING OF THE SPECTRA ---------------------

        /** Download a file in the remote computer */
        bool DownloadFile(const CString &remoteFileName, const CString &savetoPath);

        /**download upload.pak, Uxxx.pak files and evaluate*/
        bool DownloadSpectra(const CString &remoteFile, const CString &savetoPath);

        /* Downloads all Uxxx.pak files found in the m_fileInfoList.*/
        bool DownloadPakFiles(const CString& folder);

        /*download all old pak files*/
        bool DownloadAllOldPak();

        /**download old pak files and evaluate*/
        bool DownloadOldPak(long interval);

        /** Delete one file in the instrument.
            @param remote file name (without path)
            @return TRUE if deleted successfully */
        BOOL DeleteRemoteFile(const CString& remoteFile);

        // ----------------- HANDLING THE FILE-LISTS -------------------

        /** Retrieves the list of files from the given directory,
                calls 'FillFileList' which rebuilds the lists
                'm_fileInfoList' and 'm_rFolderList'.
                @param disk - the disk to retrieve the file-list from,
                    '0' corresponds to program/configuration-disk
                    '1' corresponds to data-disk
                --------- This function is only called from the CFileTransferDlg ---------
                */
        bool GetDiskFileList(int disk = 1);

        /** Retrieves the list of files from the given directory on the instrument.
            This will call 'FillFileList' which rebuilds the two lists
                'm_fileInfoList' and 'm_rFolderList'
            @return the number of pak-files in the given folder. */
        long GetPakFileList(const CString& folder);

        /** Use the result from the file-listing command to build 
            the lists of files inside the current directory of the instrument.
            This rebuilds the lists 'm_fileInfoList' and 'm_rFolderList'.
            @param fileName The local filename where the result of the file-listing
                command has been saved. 
            @param disk The disk on the remote device where the file listing was done.
            @return The number of found .pak files or directories which may contain .pak files. */
        int FillFileList(const CString& fileName, char disk = 'B');

        /** Parse one line in the result of the file-listing command 
            and inserts it into the appropriate list (either m_fileInfoList or m_rFolderList).
            @return true if the line represents a .pak-file or folder which may contain .pak-files */
        bool ParseFileInfo(CString line, char disk = 'B');

        /** Extracts and removes the suffix (file extension) of a file-name and puts it into the provided string.
            @param fileName The file to get the suffix from.
                This will be modified to not contain the suffix.
            @param fileSuffix Will on successful return be filled with the suffix.
                If no suffix is found, then this will be empty.*/
        static void ExtractSuffix(CString& fileName, CString& fileSuffix);

        /** Removes all stored file-information from
                    m_fileInfoList and m_rFolderList */
        void EmptyFileInfo();

        /** Parse one line in the result of the file-listing command and if the 
            result is a folder which may contain .pak-files then the name
            of the folder is added to the list m_rfolderList.
            Only folders of the format RXXX will be inserted.
            @param line One line from the result of the file-listing command.
            @return true if the folder name may contain .pak-files. */
        bool AddFolderInfo(const CString& line);

        //-------------------------------------------//
        //--------------public variables ------------//
        //-------------------------------------------//

        /** Information on this connection */
        struct FTPInformation m_ftpInfo;

        long m_remoteFileSize;

        /** The index of this device in the configuration.xml.
            Used to retrieve settings for the device. */
        int m_mainIndex;

        /** The spectrometer's serial number,
            used to identify the instrument when communicating with other parts of the program. */
        CString m_spectrometerSerialID;

        /** The temporary storage directory on the local computer where we 
            temporarily store downloaded files */
        CString m_storageDirectory;

        CString m_statusMsg;

        /** The list of files in the current directory */
        std::vector<CScannerFileInfo> m_fileInfoList;

        /** The list of RXX folders in the current directory */
        CList<CString, CString &> m_rFolderList;

        /** The kind of electronics box that we're communicating with.
            This is necessary since different models of boxes behave differently. */
        ELECTRONICS_BOX m_electronicsBox;

        /** Statistics on the speed to download file, in kilo-bytes/second*/
        double m_dataSpeed;

    private:

        /** The name of the file used to send commands to kongo in the instrument. */
        const CString m_commandFileName = "command.txt";

    };
}
