#pragma once
#include <afxtempl.h>
#include "../communication/ftpcom.h"
#include "../communication/ftpsocket.h"
#include "../Common/Spectra/PakFileHandler.h"
#include "../Common/Common.h"
#include "../Configuration/configuration.h"
#include "../FileInfo.h"
#include "../scannerfileinfo.h"
namespace Communication
{
	struct FTPInformation
		{
			CString IPAddress;
			CString userName;
			CString password;
			CString adminUserName;
			CString adminPassword;
			long    port;
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

		/**set ftp information*/
		void SetFTPInfo(int mainIndex, CString& IP, CString& userName, CString &pwd, long portNumber= 5551);
		void SetFTPInfo(int mainIndex, CString& IP, CString& userName, CString &pwd, CString &admUserName, CString &admPwd, long portNumber= 5551);

		/**poll one instrument*/
		bool PollScanner();

		// ---------------- MANAGING THE INSTRUMENT ------------------

		/** Send a command to kongo.exe in the instrument
		     by creating a command.txt file and upload it... */
		bool SendCommand(char* cmd);

		/** Create a command.txt file to upload to the instrument */
		bool MakeCommandFile(char* cmdString);

		/** Tells the instrument to go to sleep by uploading
		     a command.txt file to it */
		void GotoSleep();

		/** Tells the instrument to wake up by uploading
		     a command.txt file to it */
		void WakeUp();

		/** reboot the remote scanner */
		void Reboot();
		
		/** Download cfg.txt. 
			@return 1 if successful, otherwise 0*/
		int DownloadCfgTxt();

		// --------------- DOWNLOADING OF THE SPECTRA ---------------------

		/** Download a file in the remote computer */
		bool DownloadFile(const CString &remoteFileName, const CString &savetoPath);

		/**download upload.pak, Uxxx.pak files and evaluate*/
		bool DownloadSpectra(const CString &remoteFile, const CString &savetoPath);

		/*download Uxxx.pak files on m_fileInfoList*/
		bool DownloadPakFiles(const CString& folder);

		/*download all old pak files*/
		bool DownloadAllOldPak();

		/**download old pak files and evaluate*/
		bool DownloadOldPak(long interval);

		/**delete remote file
			@param remote file name ( without path)
			@return TRUE if deleted successfully */
		BOOL DeleteRemoteFile(const CString& remoteFile);

		// ----------------- HANDLING THE FILE-LISTS -------------------

		/** Retrieves the list of files from the given directory, 
				calls 'FillFileList' which rebuilds the lists
				'm_fileInfoList' and 'm_rFolderList'.
				@param disk - the disk to retrieve the file-list from, 
					'0' corresponds to program/configuration-disk
					'1' corresponds to data-disk
				--------- This function is only called from the CFileTransferDlg ---------
				*/
		bool GetDiskFileList(int disk = 1);

		/** Retrieves the list of files from the given directory, 
				calls 'FillFileList' which rebuilds the lists
				'm_fileInfoList' and 'm_rFolderList' */
		long GetPakFileList(CString& folder);

		/** Use the result fo the file-listing command to
					build the lists of files. 
				This rebuilds the lists 'm_fileInfoList' and 'm_rFolderList' */
		int  FillFileList(CString& fileName, char disk = 'B');

		/** Parse a line in the file-list and insert it into
					the appropriate list. */
		void ParseFileInfo(CString line, char disk = 'B');

		/** Extracts the suffix of a file-name */
		void GetSuffix(CString& fileName, CString& fileSubfix);

		/** Removes all stored file-information from
					m_fileInfoList and m_rFolderList */
		void EmptyFileInfo();

		/* Add a folder name into m_rfolderList. 
				Only folders of the format RXXX will be inserted */
		void AddFolderInfo(CString& line);

		//-------------------------------------------//
		//--------------public variables ------------//
		//-------------------------------------------//
		
		struct FTPInformation m_ftpInfo;

		long m_remoteFileSize;

		/**index in the configuration.xml*/
		int m_mainIndex;

		/** spectrometer's serial number */
		CString m_spectrometerSerialID;

		/** The .pak-file-handler, used to check the downloaded .pak-files */
		FileHandler::CPakFileHandler* m_pakFileHandler;

		CString m_localFileFullPath;
		CString m_storageDirectory;
		Common m_common;
		CString m_statusMsg;

		/** The list of files in the current directory */
		CList<CScannerFileInfo, CScannerFileInfo&> m_fileInfoList;

		/** The list of RXX folders in the current directory */
		CList<CString, CString &> m_rFolderList;

		/** The kind of electronics box that we're communicating with, good to know... */
		ELECTRONICS_BOX m_electronicsBox;

		/** speed to download file, in kilo-bytes/second*/
		double m_dataSpeed;
	};
}
