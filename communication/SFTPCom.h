#pragma once

#include <list>
#include <string>
#include "IFTPDataUpload.h"

namespace Communication
{
    class CSFTPCom : public IFTPDataUpload
    {
    public:
        CSFTPCom();
        ~CSFTPCom();

        // ------------------------ Implementing IFTPDatapload ------------------------
        virtual int Connect(LPCTSTR siteName, LPCTSTR userName, LPCTSTR password, int timeout, bool passiveMode = false, int portNumber = 22) override;

        virtual int Disconnect() override;

        virtual int UpdateRemoteFile(LPCTSTR localFile, LPCTSTR remoteFile) override;

        virtual int CreateDirectory(LPCTSTR remoteDirectory) override;

        virtual bool SetCurDirectory(CString curDirName) override { return EnterFolder(curDirName); }

        virtual bool DownloadAFile(LPCTSTR remoteFile, LPCTSTR fileFullName) override;


        // ------------------------ Implementation of SFTP data upload ------------------------

        /** Sends a local file to the FTP-server, this will skip the upload if the remote file exists.
            @return 0 on success.
            @return 1 if the upload failed for some reason. */
        int UploadFile(LPCTSTR localFile, LPCTSTR remoteFile);

        /** Searches for a file in the current folder on the SFTP server.
            @return true if the file exists. */
        bool FindFile(const CString& remoteFile);

        /** @return the file size of the provided remote file on the SFTP server.
            @return -1 if the file doesn't exist. */
        std::int64_t GetFileSize(const CString& remoteFile);

        /** Enter a folder
            @param folder the folder to enter, relative to the current folder. */
        bool EnterFolder(CString folder);

        /** Enters the top directory "/" */
        bool GotoTopDirectory();

    private:
        struct SftpConnection;
        SftpConnection* m_FtpConnection = nullptr;

        std::list<std::string> m_currentPath;
        std::string GetCurrentPath() const;

        /** The path to the SFTP-site, this will also hold the username and password... */
        std::string m_site;

        /** Uploads a file.
            @param localFile the full filename and path of the local file.
            @param remoteFile the filename (excluding path) of the remote file.
            @param overwrite if set to true then any existing files will be overwritten. */
        //int UploadFile(LPCTSTR localFile, LPCTSTR remoteFile, bool overwrite);
    };
}
