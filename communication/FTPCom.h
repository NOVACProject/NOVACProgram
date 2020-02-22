#include <afxinet.h>
#pragma once
namespace Communication
{
	class CFTPCom
	{
	public:
		CFTPCom();
		~CFTPCom();

        // This class is not copyable due to the managed resources m_InternetSession and m_FtpConnection
        CFTPCom(CFTPCom&) = delete;
        CFTPCom& operator=(CFTPCom&) = delete;

		/**Connect to one FTP server
		*@siteName - address of the FTP server
		*@userName - user name for login
		*@password - password for login
		*@mode		 - Specifies passive(TRUE) or active mode(FASLE) for this FTP session. 
		*						 If set to TRUE, it sets the Win32 API dwFlag to INTERNET_FLAG_PASSIVE. 
		*						 Passive mode is for client behind a firewall; it is safer comparing 
		*						 with active mode.
		*/
		int Connect(LPCTSTR siteName, LPCTSTR userName, LPCTSTR password, int timeout, BOOL mode= FALSE);

		void Disconnect();

		int UploadFile(LPCTSTR localFile, LPCTSTR remoteFile);

		BOOL DownloadAFile(LPCTSTR remoteFile, LPCTSTR fileFullName);

		int UpdateFile(LPCTSTR localFile, LPCTSTR remoteFile);

		int CreateDirectory(LPCTSTR remoteDirectory);

		/**find file in the current ftp folder
		*return TRUE if exists
		*/
		int FindFile(CString& fileName);

		/**Set current directory
		*@param curDirName current directory name
		*return TRUE  - success
		*/
		BOOL SetCurDirectory(LPCTSTR curDirName);

		/* Attempts to remove a folder, returns TRUE on success. */
		BOOL DeleteFolder(const CString& folder);

		/* Enter a folder, returns TRUE on success. */
		BOOL EnterFolder(const CString& folder);

		/*Go to top directory "/"*/
		BOOL GotoTopDirectory();

		/* read response from the ftp server*/
		void ReadResponse(CInternetFile* file);

    protected:
        CInternetSession* m_InternetSession;
        CFtpConnection* m_FtpConnection;
        CString m_FTPSite; //< the name of the site which we are currently connected to
        CString m_ErrorMsg; //< Last error message returned
    };
}
