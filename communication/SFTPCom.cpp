#include "StdAfx.h"
#include "SFTPCom.h"
#include "../Common/common.h"
#include <curl/curl.h>
#include <assert.h>

namespace Communication
{
    struct CSFTPCom::SftpConnection
    {
        SftpConnection()
        {
            this->curlHandle = curl_easy_init();
        }

        ~SftpConnection()
        {
            DeInit();
        }

        int Open(const char* siteName, const char* userName, const char* password)
        {
            // curl_easy_setopt(curlHandle, CURLOPT_URL, "sftp://iceland@ec2-13-53-136-215.eu-north-1.compute.amazonaws.com");
        }

        CURL *curlHandle = nullptr;

        void DeInit()
        {

        }
    };

    CSFTPCom::CSFTPCom()
     : m_FTPSite("")
    {
    }

    CSFTPCom::~CSFTPCom()
    {
        if (nullptr != m_FtpConnection)
        {
            delete m_FtpConnection;
        }
    }

    //Connect to FTP server
    //return 0 - fail
    //return 1 - success
    //return 2 - ftp address parsing problem
    //return 3 - can not connect to internet
    //return 4 - ftp exception
    int CSFTPCom::Connect(LPCTSTR siteName, LPCTSTR userName, LPCTSTR password, int timeout, BOOL mode)
    {
        if (nullptr != m_FtpConnection)
        {
            delete m_FtpConnection;
        }
        m_FtpConnection = new SftpConnection();

        CURLcode returnCode;

        returnCode = curl_easy_setopt(m_FtpConnection->curlHandle, CURLOPT_USERNAME, (const char*)userName);
        assert(returnCode == CURLE_OK);

        returnCode = curl_easy_setopt(m_FtpConnection->curlHandle, CURLOPT_PASSWORD, (const char*)password);
        assert(returnCode == CURLE_OK);

        returnCode = curl_easy_setopt(m_FtpConnection->curlHandle, CURLOPT_FTP_CREATE_MISSING_DIRS, CURLFTP_CREATE_DIR_RETRY);
        assert(returnCode == CURLE_OK);

        /* Switch on full protocol/debug output */
        returnCode = curl_easy_setopt(m_FtpConnection->curlHandle, CURLOPT_VERBOSE, 1L);
        assert(returnCode == CURLE_OK);

        m_FTPSite = CString("sftp://") + CString(siteName);

        return 1;
    }

    int CSFTPCom::Disconnect()
    {
        return 1;
        /*
        if (m_FtpConnection != nullptr)
            m_FtpConnection->Close();
        delete m_FtpConnection;
        m_FtpConnection = nullptr;
        if (m_InternetSession != nullptr)
            m_InternetSession->Close();
        delete m_InternetSession;
        m_InternetSession = nullptr;
        return 1;
        */
    }

    /* read data to upload */
    static size_t readfunc(void *ptr, size_t size, size_t nmemb, void *stream)
    {
        FILE *f = (FILE *)stream;

        if (ferror(f))
            return CURL_READFUNC_ABORT;

        size_t n = fread(ptr, size, nmemb, f) * size;

        return n;
    }

    int CSFTPCom::UpdateRemoteFile(LPCTSTR localFile, LPCTSTR remoteFile)
    {
        if (nullptr == m_FtpConnection || nullptr == m_FtpConnection->curlHandle)
        {
            ShowMessage("ERROR: Attempted to update file using FTP while not connected!");
            return 0;
        }

        CString remoteUrl;
        remoteUrl.Format("%s%s%s", m_FTPSite, GetCurrentPath(), remoteFile);

        CURLcode returnCode;
        returnCode = curl_easy_setopt(m_FtpConnection->curlHandle, CURLOPT_URL, (const char*)remoteUrl);
        assert(returnCode == CURLE_OK);

        returnCode = curl_easy_setopt(m_FtpConnection->curlHandle, CURLOPT_UPLOAD, 1L);
        assert(returnCode == CURLE_OK);

        FILE* f = fopen(localFile, "rb");
        if (nullptr == f) {
            return 0;
        }

        // read the data from the file
        returnCode = curl_easy_setopt(m_FtpConnection->curlHandle, CURLOPT_READDATA, f);
        assert(returnCode == CURLE_OK);

        // reading-function
        returnCode = curl_easy_setopt(m_FtpConnection->curlHandle, CURLOPT_READFUNCTION, readfunc);
        assert(returnCode == CURLE_OK);

        // to the upload
        returnCode = curl_easy_perform(m_FtpConnection->curlHandle);

        fclose(f);

        if (returnCode == CURLE_OK) {
            return 1;
        }
        else {
            fprintf(stderr, "%s\n", curl_easy_strerror(returnCode));
            return 0;
        }
        /*
        int result = 0;
        if (m_FtpConnection == nullptr) {
            ShowMessage("ERROR: Attempted to update file using FTP while not connected!");
            return 0;
        }

        // If the file exists, remove it first...
        if (FindFile((CString&)remoteFile) == TRUE)
            m_FtpConnection->Remove(remoteFile);

        // Upload the file
        result = m_FtpConnection->PutFile(localFile, remoteFile);
        return result;
        */
    }

    BOOL CSFTPCom::DownloadAFile(LPCTSTR remoteFile, LPCTSTR fileFullName)
    {
        return 1;
        /*
        BOOL result = FALSE;
        CString msg;

        // Check that we're connected...
        if (m_FtpConnection == nullptr) {
            ShowMessage("ERROR: Attempted to upload file using FTP while not connected!");
            return FALSE;
        }

        msg.Format("Trying to download %s", fileFullName);
        ShowMessage(msg);

        try
        {
            // Try to download the file
            result = m_FtpConnection->GetFile(remoteFile, fileFullName, FALSE);

            if (0 == result) { // this means something went wrong
                int ftpError = GetLastError();
                if (ftpError != 0)
                {
                    msg.Format("FTP error happened when downloading %s from %s: %d", (LPCSTR)fileFullName, (LPCSTR)m_FTPSite, ftpError);
                    ShowMessage(msg);
                    DWORD code;
                    DWORD size_needed = 0;
                    InternetGetLastResponseInfo(&code, nullptr, &size_needed);
                    char *message = (char*)malloc(size_needed + 1);
                    InternetGetLastResponseInfo(&code, message, &size_needed);
                    msg.Format("Error message :%s", message);
                    ShowMessage(msg);
                }
            }
            else {
                // SUCCESS!!
                msg.Format("Finish downloading %s", fileFullName);
                ShowMessage(msg);
                return result;
            }
        }
        catch (CInternetException* pEx)
        {
            // catch errors from WinINet
            TCHAR szErr[255];
            if (pEx->GetErrorMessage(szErr, 255))
            {
                m_ErrorMsg.Format("FTP error happened when downloading %s from %s: %s", (LPCSTR)fileFullName, (LPCSTR)m_FTPSite, (LPCSTR)szErr);
                ShowMessage(m_ErrorMsg);
            }
            else
            {
                m_ErrorMsg.Format("FTP exception");
                ShowMessage(m_ErrorMsg);
            }
            pEx->Delete();

            m_FtpConnection = nullptr;
        }
        return result;
        */
    }

    int CSFTPCom::UploadFile(LPCTSTR localFile, LPCTSTR remoteFile)
    {
        return 1;

        /*
        int result;

        // Check that we are connected
        if (m_FtpConnection == nullptr) {
            ShowMessage("ERROR: Attempted to upload file using FTP while not connected!");
            return 0;
        }

        // See if we can find the file on the remote computer, if so
        //	then we can't upload it...
        if (FindFile((CString&)remoteFile) == TRUE)
            return 1;

        try {
            // Try to upload the file
            result = m_FtpConnection->PutFile(localFile, remoteFile);

        }
        catch (CInternetException *pEx) {
            // catch errors from WinINet
            TCHAR szErr[255];
            if (pEx->GetErrorMessage(szErr, 255))
            {
                m_ErrorMsg.Format("FTP error happened when uploading %s to %s: %s", (LPCSTR)localFile, (LPCSTR)m_FTPSite, (LPCSTR)szErr);
                ShowMessage(m_ErrorMsg);
            }
            else
            {
                m_ErrorMsg.Format("FTP exception");
                ShowMessage(m_ErrorMsg);
            }
            pEx->Delete();
        }
        return result;
        */
    }

    int CSFTPCom::CreateDirectory(LPCTSTR remoteDirectory)
    {
        // This doesn't work here. The paths are created as files are updated.
        return 1;
    }

    BOOL CSFTPCom::SetCurDirectory(CString curDirName)
    {
        if (0 == strcmp(curDirName, ".."))
        {
            m_currentPath.pop_back();
        }
        else
        {
            // remove any forward or backward slashes in the path
            curDirName.Trim("/");
            m_currentPath.push_back(std::string(curDirName));
        }
        return TRUE;
    }

    int CSFTPCom::FindFile(CString& fileName)
    {
        return 1;
        /* if (m_FtpConnection == nullptr) {
            ShowMessage("ERROR: Attempted to find file using FTP while not connected!");
            return 0; // cannot connect...
        }

        // use a file find object to enumerate files
        CFtpFileFind finder(m_FtpConnection);
        BOOL result = finder.FindFile(_T(fileName));
        // DLN 12/19/17: The above seems to return 1 if the file does not exists, instead of 0 that is expected.
        // A brief search of internet seemed to say that when running this on Unix/Linux system
        // the return codes are backwards.
        //if (0 != result) {
        //	result = finder.FindNextFile();
        //	CString name = finder.GetFileURL();	// Just here for debugging
        //}
        //TODO:  Need to check for instrumentation box.  This fix will work for Moxa but will break it for older DOS based boxes.

        if (0 < result) {
            DWORD retcode = GetLastError();
            //ShowMessage("Could not find remote file");
        }

        return !result; */
    }

    // @return 0 if fail...
    BOOL CSFTPCom::DeleteFolder(const CString& folder)
    {
        return 1;
        /*
        // Check
        if (m_FtpConnection == nullptr) {
            ShowMessage("ERROR: Attempted to delete folder using FTP while not connected");
            return 0;
        }

        // Remove the directory
        BOOL result = m_FtpConnection->RemoveDirectory(folder);
        return result;
        */
    }

    BOOL CSFTPCom::EnterFolder(const CString& folder)
    {
        return TRUE;
        /*
        CString strDir, strFolder, msg;

        // Check...
        if (m_FtpConnection == nullptr) {
            ShowMessage("ERROR: Attempted to enter folder using FTP while not connected");
            return FALSE;
        }

        // Set the current directory, return 0 if fail...
        if (0 == m_FtpConnection->SetCurrentDirectory(folder))
            return FALSE;

        // Get the current directory, return 0 if fail...
        if (0 == m_FtpConnection->GetCurrentDirectory(strDir))
            return FALSE;

        // The response we want to have...
        strFolder.Format("/%s/", (LPCSTR)folder);

        // Compare if the returned string is the same as what we want...
        // If a relative directory is passed into this function
        // the Equals below will compare it agains a full path and return false.
        // Misleading 'Can not get into folder' message displays even if the
        // change directory was successful. Best to handle messages externally.
        if (Equals(strDir, strFolder))
        {
            //msg.Format("Get into folder %s", (LPCSTR)folder);
            //ShowMessage(msg);
            return TRUE;
        }
        else
        {
            //msg.Format("Can not get into folder %s", (LPCSTR)folder);
            //ShowMessage(msg);
            return FALSE;
        }
        */
    }

    BOOL CSFTPCom::GotoTopDirectory()
    {
        m_currentPath.clear();
        return TRUE;
    }

    CString CSFTPCom::GetCurrentPath() const
    {
        CString currentPath = "/";
        for (const std::string& dir : m_currentPath)
        {
            currentPath.AppendFormat("%s/", dir.c_str());
        }
        return currentPath;
    }


}

