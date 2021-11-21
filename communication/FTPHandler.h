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

        /** Sets the necessary connection information for us to be able
            to create an FTP connection */
        void SetFTPInfo(int mainIndex, const CString& hostName, const CString& userName, const CString& pwd, int timeOut, long portNumber = 5551);
        void SetFTPInfo(int mainIndex, const CString& hostName, const CString& userName, const CString& pwd, const CString& admUserName, const CString& admPwd, int timeOut, long portNumber = 5551);

        /** Polls the connected instrument for new data.
            This will attempt to download all old UXXX.pak spectrum files and
            enter the RXXX directories in search of data. */
        bool PollScanner();

        // ---------------- MANAGING THE INSTRUMENT ------------------

        /** Tells the instrument to go to sleep by uploading
             a command.txt file to it */
        void GotoSleep();

        /** Tells the instrument to wake up by uploading
             a command.txt file to it */
        void WakeUp();

        // --------------- DOWNLOADING DATA ---------------------

        /** Download a file (pak file or other type) from the instrument computer.
            @param remoteFileName The name of the file to download from the instrument.
                This shall be a file in the current directory and include the file extension.
            @param localDirectory The local directory to which the file should be downloaded.
                This must exist prior to calling this method.
            @param remoteFileSize The size of the file on the instrument, in bytes. */
        bool DownloadFile(const CString& remoteFileName, const CString& localDirectory, long remoteFileSize);

        /** Delete one file in the instrument.
            @param remote file name (without path)
            @return true if deleted successfully */
        bool DeleteRemoteFile(const CString& remoteFile);

        // ----------------- HANDLING THE FILE-LISTS -------------------

        /** Retrieves the list of files from the given directory,
            calls 'FillFileList' which rebuilds the lists
            'm_fileInfoList' and 'm_rFolderList'.
        @param disk - the disk to retrieve the file-list from,
            'A' corresponds to program/configuration-disk
            'B' corresponds to data-disk
        --------- This function is only called from the CFileTransferDlg ---------
                */
        bool GetDiskFileList(char disk = 'B');

        /** Retrieves the list of pak files from the given directory on the instrument.
            This will call 'FillFileList' which rebuilds the two lists
                'm_fileInfoList' and 'm_rFolderList'
            @return the number of pak-files in the given folder. */
        long GetPakFileList(const CString& folder);

        //-------------------------------------------//
        //--------------public variables ------------//
        //-------------------------------------------//

        /** Information on this connection */
        struct FTPInformation m_ftpInfo;

        /** The index of this device in the configuration.xml.
            Used to retrieve settings for the device. */
        int m_mainIndex;

        CString m_statusMsg;

        /** The list of files in the current directory */
        std::vector<CScannerFileInfo> m_fileInfoList;

        /** The list of RXX folders in the current directory */
        std::vector<CString> m_rFolderList;

        /** The kind of electronics box that we're communicating with.
            This is necessary since different models of boxes behave differently. */
        ELECTRONICS_BOX m_electronicsBox;

    private:

        //-------------------------------------------//
        //------------ private functions ------------//
        //-------------------------------------------//

        /** Send a command to kongo.exe in the instrument
             by creating a command.txt file and upload it to the instrument.
             @return true if the file creation and upload was successful. */
        bool SendCommand(const char* cmd);

        /** Creates a command.txt file to upload to the instrument
            containing the provided command to send.
            @return true if the file could be created successfully. */
        static bool MakeCommandFile(const CString& fileName, const char* command);

        /** Reboots the remote scanner */
        void Reboot();

        /** Download cfg.txt from the instrument.
            @return 1 if successful, otherwise 0*/
        int DownloadCfgTxt();

        /** Downloads a single .pak file from the instrument and deletes it from the instrument.
            This will verify that the file size is correct and that the file can be parsed properly.
            @param remoteFile The name of the file to download in the instrument,
                including the .pak file extension.
            @param localDirectory The local directory where the file should be saved.
                This must exist prior to calling this method.
            @param remoteFileSize The size of the file in the remote system, in bytes.
            @return true if the file was successfully downloaded and
                the size of the downloaded file equals the remote file size and
                the file can be parsed properly. */
        bool DownloadSpectrumFile(const CString& remoteFile, const CString& localDirectory, long remoteFileSize);

        /* Downloads all Uxxx.pak files found in the provided list of files.*/
        bool DownloadPakFiles(const CString& folder, std::vector<CScannerFileInfo>& filesToDownload);

        /** download old pak files and evaluate*/
        bool DownloadOldPak(long interval);

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

        /** Removes all stored file-information from m_fileInfoList and m_rFolderList */
        void EmptyFileInfo();

        /** Parse one line in the result of the file-listing command and if the
            result is a folder which may contain .pak-files then the name
            of the folder is added to the list m_rfolderList.
            Only folders of the format RXXX will be inserted.
            @param line One line from the result of the file-listing command.
            @return true if the folder name may contain .pak-files. */
        bool AddFolderInfo(const CString& line);

        //-------------------------------------------//
        //---------------- private data -------------//
        //-------------------------------------------//

        /** The name of the file used to send commands to kongo in the instrument. */
        const CString m_commandFileName = "command.txt";

        /** The temporary storage directory on the local computer where we
            temporarily store downloaded files */
        CString m_storageDirectory;

        /** The spectrometer's serial number,
            used to identify the instrument when communicating with other parts of the program. */
        CString m_spectrometerSerialID;

        /** Statistics on the speed to download file, in kilo-bytes/second*/
        double m_dataSpeed;

    };
}
