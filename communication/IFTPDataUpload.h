#pragma once

#include <memory>

namespace Communication
{
    /** The IFTPDataUpload is an abstract base class for the classes which are able
        to upload data to the Novac data server, the FTPCom (which uses regular FTP) 
        and SFTPCom (which uses SFTP). */
    class IFTPDataUpload
    {
    public:
        IFTPDataUpload() { }

        virtual ~IFTPDataUpload() { }

        /** Creates and returns an instance of this interface using the provided protocol (must be either FTP or SFTP) */
        static std::unique_ptr<IFTPDataUpload> Create(const CString& protocol);

        /** The last error message, this will be set whenever something goes wrong. */
        CString m_ErrorMsg = "";

        /** Connect to the data server
        *   @siteName - address of the server
        *   @userName - user name for login
        *   @password - password for login */
        virtual int Connect(LPCTSTR siteName, LPCTSTR userName, LPCTSTR password, int timeout, int portNumber, bool passiveMode = false) = 0;

        /** Closes the connection to the server. No more calls to this object can be done until the connection is opened again */
        virtual int Disconnect() = 0;

        /** Sends a local file to the FTP-server, if the remote file exists then it will be overwritten.
            @param localFile The local file to read the data from, this must be the full path.
            @param remoteFile The filename of the remote file. Thus must only be the file name, not including any path.
            The path must be set before by calling 'SetCurDirectory'.
            @return 0 on success.
            @return 1 if the upload failed for some reason. */
        virtual int UpdateRemoteFile(LPCTSTR localFile, LPCTSTR remoteFile) = 0;

        /** Downloads a remote file.
            @param remoteFile the name of the remote file to download, in the current remote directory.
            @param fillFileName the local file name where to save the dta, this must be a full path.
            @reutrn true on success. */
        virtual bool DownloadAFile(LPCTSTR remoteFile, LPCTSTR fileFullName) = 0;

        /** Set current directory. This does the same as EnterFolder.
            @param curDirName current directory name
            @return true on success */
        virtual bool SetCurDirectory(CString curDirName) = 0;

        virtual int CreateDirectory(LPCTSTR remoteDirectory) = 0;
    };
}