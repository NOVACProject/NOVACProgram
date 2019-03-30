#pragma once

#include <list>
#include <string>

namespace Communication
{
    class CSFTPCom
    {
    public:
        CSFTPCom();
        ~CSFTPCom();

        /** The last error message, this will be set whenever something goes wrong. */
        CString m_ErrorMsg;

        /** Connect to one FTP server
        *   @siteName - address of the FTP server
        *   @userName - user name for login
        *   @password - password for login */
        int Connect(LPCTSTR siteName, LPCTSTR userName, LPCTSTR password, int timeout, bool mode = false);

        /** Closes the connection to the server. No more calls to this object can be done until the connection is opened again */
        int Disconnect();

        /** Sends a local file to the FTP-server, this will skip the upload if the remote file exists.
            @return 0 on success.
            @return 1 if the upload failed for some reason. */
        int UploadFile(LPCTSTR localFile, LPCTSTR remoteFile);

        /** Sends a local file to the FTP-server, if the remote file exists then it will be overwritten. 
            @param localFile The local file to read the data from, this must be the full path.
            @param remoteFile The filename of the remote file. Thus must only be the file name, not including any path. 
                The path must be set before by calling 'SetCurDirectory'.
            @return 0 on success.
            @return 1 if the upload failed for some reason. */
        int UpdateRemoteFile(LPCTSTR localFile, LPCTSTR remoteFile);

        /** Downloads a remote file.
            @param remoteFile the name of the remote file to download, in the current remote directory.
            @param fillFileName the local file name where to save the dta, this must be a full path. 
            @reutrn 0 on success. */
        bool DownloadAFile(LPCTSTR remoteFile, LPCTSTR fileFullName);

        int CreateDirectory(LPCTSTR remoteDirectory);

        /** Searches for a file in the current folder on the SFTP server.
            @return true if the file exists. */
        bool FindFile(const CString& remoteFile);

        /** @return the file size of the provided remote file on the SFTP server.
            @return -1 if the file doesn't exist. */
        std::int64_t GetFileSize(const CString& remoteFile);

        /** Remove a folder*/
        bool DeleteFolder(const CString& folder);

        /** Set current directory. This does the same as EnterFolder.
            @param curDirName current directory name
            @return true on success */
        bool SetCurDirectory(CString curDirName) { return EnterFolder(curDirName); }

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
        int UploadFile(LPCTSTR localFile, LPCTSTR remoteFile, bool overwrite);
    };
}
