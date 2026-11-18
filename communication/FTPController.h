#pragma once
#include <afxtempl.h>
#include "../communication/ftpcom.h"
#include "../communication/ftpsocket.h"
#include "../Common/Spectra/PakFileHandler.h"
#include "../Common/Common.h"
#include "../Configuration/configuration.h"
#include "../FileInfo.h"

namespace Communication
{
	struct FTPInformation
		{
			CString IPAddress;
			CString userName;
			CString password;
			long		port;
		};
	/** <b>CFTPController</b> class handles one link of downloading and uploading file 
	*to the remote PC
	*/
	class CFTPController :
		public CFTPSocket
	{
	public:
		CFTPController(void);
		~CFTPController(void);
		//-------------------------------------------//
		//--------------public functions ------------//
		//-------------------------------------------//
		/**set ftp information*/
		void SetFTPInfo(int mainIndex, CString& IP, CString& userName,CString pwd,long portNumber= 5551);
		/**poll one instrument*/
		bool PollScanner();
		bool MakeCommandFile(char* cmdString);
		bool SendCommand(char* cmd);
		void GotoSleep();
		void WakeUp();
		/**reboot the remote scanner*/
		void Reboot();
		bool DownloadRemoteFile(CString remoteFileName, CString savetoPath);
		/**download upload.pak, Uxxx.pak files and evaluate*/
		bool DownloadSpectra( CString remoteFile,CString savetoPath);
		/**download old pak files and evaluate*/
		bool DownloadOldPak(long interval);
		/**get file list*/
		bool GetPakFileList();
		void ParseFileInfo(CString line);
		int  FillFileList();
		/**delet remote file
		@param remote file name ( without path)
		return TRUE if deleted successfully
		*/
		BOOL DeleteRemoteFile(CString& remoteFile);
		/**get the file transfer speed, return bytes per second*/
		long GetFtpSpeed();
		/*arrange file structure list*/
		bool ArrangeFileList(char* string);
		/*add pak file name into m_oldpakfile list*/
		void AddPakFileInfo(CString string);
		/*add Rxxx folder name into m_rFolderList*/
		void AddFolderInfo(CString string);
		/*download Uxxx.pak files in m_oldpakList*/
		bool DownloadPakFiles(CString& folder);
		//-------------------------------------------//
		//--------------public variables ------------//
		//-------------------------------------------//
		
		struct FTPInformation m_ftpInfo;
		/**index in the configuration.xml*/
		int m_mainIndex;		
		/**spectrometer's serial number, can be found by connecting spectrometer with USB cable*/
		CString m_spectrometerSerialID;
		FileHandler::CPakFileHandler* m_pakFileHandler;
		CString m_localFileFullPath;
		CString m_storageDirectory;
		Common m_common;
		CString m_statusMsg;
		/**speed to download file, byte/second*/
		long m_dataSpeed;
		//list for first layer Uxxx.pak files
		CList<CFileInfo, CFileInfo&> m_oldPakList;
		CList<CFileInfo, CFileInfo&> m_fileList;		
		//list for Rxxx folders
		CList<CString, CString &> m_rFolderList;
	};
}