#include "StdAfx.h"
#include "SFTPCom.h"
#include "../Common/common.h"
#include <curl/curl.h>
#include <sstream>
#include <assert.h>

// Use warning level 4 here
#pragma warning(push, 4)

namespace Communication
{
    int libcurl_transfer_progress_callback(void* /*clientp*/, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
    {
        std::stringstream msg;
        msg << "Data uploaded: " << ulnow << " / " << ultotal << ", data downloaded: " << dlnow << " / " << dltotal << std::endl;
        OutputDebugString(msg.str().c_str());
        return 0;
    }

    // convenient handle to a file to be downloade
    struct FtpFile {
        const char *filename = nullptr;
        FILE *stream = nullptr;
    };

    /* read data to upload */
    static size_t readfunc(void *ptr, size_t size, size_t nmemb, void *stream)
    {
        FILE *f = (FILE *)stream;

        if (ferror(f))
        {
            return CURL_READFUNC_ABORT;
        }

        size_t n = fread(ptr, size, nmemb, f) * size;

        return n;
    }

    /* write data being downloaded to disk */
    static size_t writefunc(void *buffer, size_t size, size_t nmemb, void *stream)
    {
        struct FtpFile *out = (struct FtpFile *)stream;

        if (!out->stream) {
            /* open file for writing */
            out->stream = fopen(out->filename, "wb");
            if (!out->stream) {
                return (size_t )-1; /* failure, can't open file to write */
            }
        }
        return fwrite(buffer, size, nmemb, out->stream);
    }


    struct CSFTPCom::SftpConnection
    {
        SftpConnection()
        {
            this->curlHandle = curl_easy_init();
        }

        ~SftpConnection()
        {
            curl_easy_cleanup(curlHandle);
            curlHandle = nullptr;
        }

        // This object manages a connection and is thus not trivially copyable
        SftpConnection(const SftpConnection&) = delete;
        SftpConnection& operator=(const SftpConnection&) = delete;

        CURL *curlHandle = nullptr;
    };

    CSFTPCom::CSFTPCom()
        : m_ErrorMsg(""), m_site("")
    {
    }

    CSFTPCom::~CSFTPCom()
    {
        if (nullptr != m_FtpConnection)
        {
            delete m_FtpConnection;
        }
    }

    int CSFTPCom::Connect(LPCTSTR siteName, LPCTSTR userName, LPCTSTR password, int timeout, bool /*mode*/)
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
        if (returnCode != CURLE_OK) {
            m_ErrorMsg.Format("Failed to set directory flag: %s", curl_easy_strerror(returnCode));
            return 0;
        }

        returnCode = curl_easy_setopt(m_FtpConnection->curlHandle, CURLOPT_FTP_RESPONSE_TIMEOUT, (long)timeout);
        if (returnCode != CURLE_OK) {
            m_ErrorMsg.Format("Failed to set timeout: %s", curl_easy_strerror(returnCode));
            return 0;
        }

        returnCode = curl_easy_setopt(m_FtpConnection->curlHandle, CURLOPT_PORT, 22);
        if (returnCode != CURLE_OK) {
            m_ErrorMsg.Format("Failed to set port: %s", curl_easy_strerror(returnCode));
            return 0;
        }

        returnCode = curl_easy_setopt(m_FtpConnection->curlHandle, CURLOPT_PROTOCOLS, CURLPROTO_SFTP);
        if (returnCode != CURLE_OK) {
            m_ErrorMsg.Format("Failed to set SFTP-protocol: %s", curl_easy_strerror(returnCode));
            return 0;
        }

        returnCode = curl_easy_setopt(m_FtpConnection->curlHandle, CURLOPT_SSH_AUTH_TYPES, CURLSSH_AUTH_PASSWORD);
        if (returnCode != CURLE_OK) {
            m_ErrorMsg.Format("Failed to enable password authentication: %s", curl_easy_strerror(returnCode));
            return 0;
        }

        /* Switch on full protocol/debug output */
#ifdef DEBUG
        // enable progress report function
        returnCode = curl_easy_setopt(m_FtpConnection->curlHandle, CURLOPT_NOPROGRESS, FALSE);
        assert(returnCode == CURLE_OK);

        returnCode = curl_easy_setopt(m_FtpConnection->curlHandle, CURLOPT_XFERINFOFUNCTION, libcurl_transfer_progress_callback);        assert(returnCode == CURLE_OK);

        returnCode = curl_easy_setopt(m_FtpConnection->curlHandle, CURLOPT_VERBOSE, 1L);
        assert(returnCode == CURLE_OK);
#endif

        {
            std::stringstream site;
            char *urlEscapedPwd = curl_easy_escape(m_FtpConnection->curlHandle, password, strlen(password));
            // site << "sftp://" << userName << ":" << urlEscapedPwd << "@" << siteName;
            site << "sftp://" << siteName;
            m_site = site.str();

            curl_free(urlEscapedPwd);
        }

        return 1;
    }

    int CSFTPCom::Disconnect()
    {
        if (nullptr != m_FtpConnection)
        {
            delete m_FtpConnection;
        }

        m_FtpConnection = nullptr;

        return 1;
    }

    int CSFTPCom::UpdateRemoteFile(LPCTSTR localFile, LPCTSTR remoteFile)
    {
        if (nullptr == m_FtpConnection || nullptr == m_FtpConnection->curlHandle)
        {
            ShowMessage("ERROR: Attempted to update file using SFTP while not connected!");
            return 0;
        }

        // build the full url, from the filename
        CString remoteUrl;
        remoteUrl.Format("%s%s%s", m_site.c_str(), GetCurrentPath().c_str(), remoteFile);

        // set the mode.
        CURLcode returnCode;
        returnCode = curl_easy_setopt(m_FtpConnection->curlHandle, CURLOPT_UPLOAD, TRUE);
        if (returnCode != CURLE_OK) {
            m_ErrorMsg.Format("Error while uploading, failed to enable file upload mode: %s", curl_easy_strerror(returnCode));
            return 0;
        }

        // set the remote url
        returnCode = curl_easy_setopt(m_FtpConnection->curlHandle, CURLOPT_URL, (const char*)remoteUrl);
        if (returnCode != CURLE_OK) {
            m_ErrorMsg.Format("Error while uploading, failed to set URL: %s", curl_easy_strerror(returnCode));
            return 0;
        }

        // make sure we get the contents of the file (this may be disabled by e.g. getting the file size)
        returnCode = curl_easy_setopt(m_FtpConnection->curlHandle, CURLOPT_NOBODY, FALSE);
        if (returnCode != CURLE_OK) {
            m_ErrorMsg.Format("Error while uploading, failed to enable sending body: %s", curl_easy_strerror(returnCode));
            return 0;
        }

        // set the function reading the data from the file handle
        returnCode = curl_easy_setopt(m_FtpConnection->curlHandle, CURLOPT_READFUNCTION, readfunc);
        if (returnCode != CURLE_OK) {
            m_ErrorMsg.Format("Error while uploading, failed to set read-function: %s", curl_easy_strerror(returnCode));
            return 0;
        }

        FILE* f = fopen(localFile, "rb");
        if (nullptr == f) {
            m_ErrorMsg.Format("Failed to upload file '%s'. Cannot open file for reading", localFile);
            return 0;
        }

        // set the file handle
        returnCode = curl_easy_setopt(m_FtpConnection->curlHandle, CURLOPT_READDATA, f);
        if (returnCode != CURLE_OK) {
            m_ErrorMsg.Format("Error while uploading, failed to set read-data-function: %s", curl_easy_strerror(returnCode));
            fclose(f);
            return 0;
        }

        // make sure we overwrite the file if it already exists
        returnCode = curl_easy_setopt(m_FtpConnection->curlHandle, CURLOPT_APPEND, 0L);
        if (returnCode != CURLE_OK) {
            m_ErrorMsg.Format("Error while uploading, failed to set append-mode: %s", curl_easy_strerror(returnCode));
            fclose(f);
            return 0;
        }

        // do the upload
        returnCode = curl_easy_perform(m_FtpConnection->curlHandle);

        fclose(f);

        if (returnCode == CURLE_OK) {
            return 1;
        }
        else {
            m_ErrorMsg.Format("Error while uploading, failed to upload file: %s", curl_easy_strerror(returnCode));
            return 0;
        }
    }

    int CSFTPCom::UploadFile(LPCTSTR localFile, LPCTSTR remoteFile)
    {
        // Check the size of the file on the remote server. This returns -1 if the file doesn't exist.
        const std::int64_t fileSize = GetFileSize(remoteFile);

        if (fileSize < 0)
        {
            // File doesn't exist, upload
            return UpdateRemoteFile(localFile, remoteFile);
        }
        else
        {
            // File does exist, quit
            return 0;
        }
    }

    bool CSFTPCom::DownloadAFile(LPCTSTR remoteFile, LPCTSTR fileFullName)
    {
        if (nullptr == m_FtpConnection || nullptr == m_FtpConnection->curlHandle)
        {
            ShowMessage("ERROR: Attempted to update file using SFTP while not connected!");
            return 0;
        }

        // build the full url, from the filename
        CString remoteUrl;
        remoteUrl.Format("%s%s%s", m_site.c_str(), GetCurrentPath().c_str(), remoteFile);

        // set the mode.
        CURLcode returnCode;
        returnCode = curl_easy_setopt(m_FtpConnection->curlHandle, CURLOPT_UPLOAD, FALSE);
        if (returnCode != CURLE_OK) {
            m_ErrorMsg.Format("Error while downloading, failed to enable file download mode: %s", curl_easy_strerror(returnCode));
            return 0;
        }

        // set the remote url
        returnCode = curl_easy_setopt(m_FtpConnection->curlHandle, CURLOPT_URL, (const char*)remoteUrl);
        if (returnCode != CURLE_OK) {
            m_ErrorMsg.Format("Error while downloading, failed to set URL: %s", curl_easy_strerror(returnCode));
            return 0;
        }

        // make sure we get the contents of the file (this may be disabled by e.g. getting the file size)
        returnCode = curl_easy_setopt(m_FtpConnection->curlHandle, CURLOPT_NOBODY, FALSE);
        if (returnCode != CURLE_OK) {
            m_ErrorMsg.Format("Error while downloading, failed to enable sending body: %s", curl_easy_strerror(returnCode));
            return 0;
        }

        // set the function reading the data to the file handle
        returnCode = curl_easy_setopt(m_FtpConnection->curlHandle, CURLOPT_WRITEFUNCTION, writefunc);
        if (returnCode != CURLE_OK) {
            m_ErrorMsg.Format("Error while downloading, failed to set read-function: %s", curl_easy_strerror(returnCode));
            return 0;
        }

        // create a structure to upload data.
        FtpFile ftpfile;
        ftpfile.filename = (const char*)fileFullName;
        ftpfile.stream = nullptr; // this will be opened by 'writefunc'

        // set a pointer to our struct to pass to the callback
        returnCode = curl_easy_setopt(m_FtpConnection->curlHandle, CURLOPT_WRITEDATA, &ftpfile);
        if (returnCode != CURLE_OK) {
            m_ErrorMsg.Format("Error while downloading, failed to set data structure: %s", curl_easy_strerror(returnCode));
            return 0;
        }

        // do the download
        returnCode = curl_easy_perform(m_FtpConnection->curlHandle);

        // remember to close the local file stream
        if (ftpfile.stream) {
            fclose(ftpfile.stream);
            ftpfile.stream = nullptr;
        }

        if (returnCode == CURLE_OK) {
            return 1;
        }
        else {
            m_ErrorMsg.Format("Error while downloading, failed to download file: %s", curl_easy_strerror(returnCode));
            return 0;
        }
    }

    int CSFTPCom::CreateDirectory(LPCTSTR remoteDirectory)
    {
        // This doesn't work here. The paths are created as files are updated.
        return 1;
    }

    std::int64_t CSFTPCom::GetFileSize(const CString& remoteFile)
    {
        if (nullptr == m_FtpConnection || nullptr == m_FtpConnection->curlHandle)
        {
            ShowMessage("ERROR: Attempted to find a file using SFTP while not connected!");
            return -1;
        }

        curl_off_t remoteFileSizeByte = -1;

        // build the full url, from the filename
        CString remoteUrl;
        remoteUrl.Format("%s%s%s", m_site.c_str(), GetCurrentPath().c_str(), (const char*)remoteFile);

        CURLcode returnCode;
        returnCode = curl_easy_setopt(m_FtpConnection->curlHandle, CURLOPT_URL, (const char*)remoteUrl);
        if (returnCode != CURLE_OK) {
            m_ErrorMsg.Format("Failed to set URL: %s", curl_easy_strerror(returnCode));
            return -1;
        }

        returnCode = curl_easy_setopt(m_FtpConnection->curlHandle, CURLOPT_NOBODY, TRUE);
        if (returnCode != CURLE_OK) {
            m_ErrorMsg.Format("Failed to disable body: %s", curl_easy_strerror(returnCode));
            return -1;
        }

        returnCode = curl_easy_setopt(m_FtpConnection->curlHandle, CURLOPT_HEADER, TRUE);
        if (returnCode != CURLE_OK) {
            m_ErrorMsg.Format("Failed to enable header: %s", curl_easy_strerror(returnCode));
            return -1;
        }

        returnCode = curl_easy_setopt(m_FtpConnection->curlHandle, CURLOPT_FILETIME, 1);
        if (returnCode != CURLE_OK) {
            m_ErrorMsg.Format("Failed to set URL: %s", curl_easy_strerror(returnCode));
            return -1;
        }

        CURLcode result = curl_easy_perform(m_FtpConnection->curlHandle);

        if (CURLE_REMOTE_FILE_NOT_FOUND == result) {
            return -1;
        }

        if (CURLE_OK == result) {
            result = curl_easy_getinfo(m_FtpConnection->curlHandle, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &remoteFileSizeByte);
            if (result) {
                return -1;
            }
            printf("filesize: %" CURL_FORMAT_CURL_OFF_T "\n", remoteFileSizeByte);
            return remoteFileSizeByte;
        }

        return -1;
    }

    bool CSFTPCom::FindFile(const CString& remoteFile)
    {
        if (nullptr == m_FtpConnection || nullptr == m_FtpConnection->curlHandle)
        {
            ShowMessage("ERROR: Attempted to find a file using SFTP while not connected!");
            return 0;
        }

        return (GetFileSize(remoteFile) > 0);
    }

    // @return 0 if fail...
    bool CSFTPCom::DeleteFolder(const CString& folder)
    {
        return 1;
    }

    bool CSFTPCom::EnterFolder(CString folder)
    {
        if (0 == strcmp(folder, ".."))
        {
            m_currentPath.pop_back();
        }
        else
        {
            // remove any forward or backward slashes in the path
            folder.Trim("/");
            m_currentPath.push_back(std::string(folder));
        }
        return true;
    }

    bool CSFTPCom::GotoTopDirectory()
    {
        m_currentPath.clear();
        return true;
    }

    std::string CSFTPCom::GetCurrentPath() const
    {
        std::stringstream currentPath;
        currentPath << "/";
        for (const std::string& dir : m_currentPath)
        {
            currentPath << dir << "/";
        }
        return currentPath.str();
    }


}

