#pragma once

#include <afxinet.h>
#include "IFTPDataUpload.h"

namespace Communication
{
    class CFTPCom : public IFTPDataUpload
    {
    public:
        CFTPCom();
        ~CFTPCom();

        CString m_FTPSite;

        // ------------------------ Implementing IFTPDatapload ------------------------

        virtual int Connect(LPCTSTR siteName, LPCTSTR userName, LPCTSTR password, int timeout, bool passiveMode = false) override;

        virtual int Disconnect() override;

        virtual int UpdateRemoteFile(LPCTSTR localFile, LPCTSTR remoteFile) override;

        virtual bool SetCurDirectory(CString curDirName) override;

        virtual bool DownloadAFile(LPCTSTR remoteFile, LPCTSTR fileFullName) override;

        // ------------------------ Implementation of FTP data upload and download ------------------------

        /** Sends a local file to the FTP-server, this will skip the upload if the remove file exists.
            @return 0 on success
            @return 1 if the file already exists. */
        int UploadFile(LPCTSTR localFile, LPCTSTR remoteFile);

        int CreateDirectory(LPCTSTR remoteDirectory);

        /**find file in the current ftp folder
        *return TRUE if exists
        */
        int FindFile(CString& fileName);

        /*Remove a folder*/
        BOOL DeleteFolder(const CString& folder);

        /*Enter a folder*/
        BOOL EnterFolder(const CString& folder);

        /*Go to top directory "/"*/
        BOOL GotoTopDirectory();

        /*read response from the ftp server*/
        void ReadResponse(CInternetFile* file);

    protected:
        CFtpConnection* m_FtpConnection = nullptr;

    private:
        CInternetSession* m_InternetSession = nullptr;

    };
}
