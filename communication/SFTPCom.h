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

        CString m_FTPSite;
        CString m_ErrorMsg;

        /**Connect to one FTP server
        *@siteName - address of the FTP server
        *@userName - user name for login
        *@password - password for login
        *@mode		 - Specifies passive(TRUE) or active mode(FASLE) for this FTP session.
        *						 If set to TRUE, it sets the Win32 API dwFlag to INTERNET_FLAG_PASSIVE.
        *						 Passive mode is for client behind a firewall; it is safer comparing
        *						 with active mode.
        */
        int Connect(LPCTSTR siteName, LPCTSTR userName, LPCTSTR password, int timeout, BOOL mode = FALSE);

        int Disconnect();

        /** Sends a local file to the FTP-server, this will skip the upload if the remove file exists.
            @return 0 on success
            @return 1 if the file already exists. */
        int UploadFile(LPCTSTR localFile, LPCTSTR remoteFile);

        BOOL DownloadAFile(LPCTSTR remoteFile, LPCTSTR fileFullName);

        /** Sends a local file to the FTP-server, if the remote file exists then it will be overwritten. */
        int UpdateRemoteFile(LPCTSTR localFile, LPCTSTR remoteFile);

        int CreateDirectory(LPCTSTR remoteDirectory);

        /**find file in the current ftp folder
            return TRUE if exists
        */
        int FindFile(CString& fileName);

        /**Set current directory
            *@param curDirName current directory name
            *return TRUE  - success
        */
        BOOL SetCurDirectory(CString curDirName);

        /*Remove a folder*/
        BOOL DeleteFolder(const CString& folder);

        /*Enter a folder*/
        BOOL EnterFolder(const CString& folder);

        /*Go to top directory "/"*/
        BOOL GotoTopDirectory();


    private:
        // CInternetSession* m_InternetSession;
        // CFtpConnection* m_FtpConnection;
        struct SftpConnection;
        SftpConnection* m_FtpConnection = nullptr;

        std::list<std::string> m_currentPath;

        CString GetCurrentPath() const;
    };
}
