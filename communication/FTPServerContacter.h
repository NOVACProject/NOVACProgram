#pragma once
#include <afxtempl.h>
#include "../Common/Common.h"
#include "ftpcom.h"
#include "LinkStatistics.h"

namespace Communication
{
	struct FTPUploadOptions{
		int   volcanoIndex; // the index of the volcano to upload to...
		bool  deleteFile;   // true if the file shall be deleted once it is uploaded
	};


	/** The class CFTPServerContacter is responsible for the uploading of 
			spectra and results to the data-server. */

	class CFTPServerContacter :
		public CWinThread
	{
	public:
		CFTPServerContacter(void);
		~CFTPServerContacter(void);
		DECLARE_DYNCREATE(CFTPServerContacter);
		DECLARE_MESSAGE_MAP()
		
		// ----------------------------------------------------------------------
		// --------------------- PUBLIC METHODS ---------------------------------
		// ----------------------------------------------------------------------
	
		/** Handling uploading file and file queueing. 
			@param wp is a pointer to a CString object telling the filename of the file to be uploaded. 
			@param lp - unused. */
		afx_msg void OnArrivedFile(WPARAM wp, LPARAM lp);		

		/** Called when the thread is to be stopped */
		afx_msg void OnQuit(WPARAM wp, LPARAM lp);

		/**Called when the thread is started*/
		afx_msg void OnStartFTP(WPARAM wp, LPARAM lp);

		/** */
		afx_msg void OnTimer(UINT nIDEvent, LPARAM lp);

		/** Called when the thread is starting */
		virtual BOOL InitInstance();

		/** Called when the thread is stopping */
		virtual int ExitInstance();

		/** Called when there's nothing else to do. */
		virtual BOOL OnIdle(LONG lCount);

		/**set the current directory to volcanoName\yyyy.mm.dd*/
		void SetRemoteDirectory(const CString &volcanoName);

		/**parse a file by \n, fill in m_fileList*/
		bool ParseAFile(const CString& fileName);

		/**export the  file list to UploadFileList.txt*/
		void ExportList();

		// ----------------------------------------------------------------------
		// --------------------- PUBLIC VARIABLES --------------------------------
		// ----------------------------------------------------------------------

		typedef struct UploadFile{
			CString    fileName;      // <-- the name of the file to upload
			int        volcanoIndex;  // <-- the volcano to which the file belongs
			bool       deleteFile;    // <-- true if the file should be deleted when successfully uploaded
		}UploadFile;

		/** List to stored file names that should be uploaded*/
		CList<UploadFile, UploadFile&> m_fileList;

		/** Error message */
		CString m_ErrorMsg;

		/** log file (with path) to record file list */
		CString m_listLogFile;
		CString m_listLogFile_Temp;

		/** The ftp-communciation handler */
		CFTPCom* m_ftp;

		/** Timer */
		UINT_PTR m_nTimerID;

		/** True if this thread has initialized properly and read in the 
				old list of files to upload. */
		bool m_hasReadInFileList;

		/** The time of the last exporting of the file-list. This is 
				to prevent the program from writing too often to disk. */
		time_t m_lastExportTime;

		/** The statistics of the upload-link */
		CLinkStatistics	m_linkStatistics;
	};
}