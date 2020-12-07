#pragma once
#include "afxwin.h"

#include "../Communication/SerialControllerWithTx.h"
#include "../Communication/FTPHandler.h"
#include "../FileTreeCtrl.h"
#include "../ScannerFileInfo.h"
#include "../ScannerFolderInfo.h"
#include "afxcmn.h"
#include "afxtempl.h"

namespace Dialogs
{
	class CFileTransferDlg : public CDialog
	{
		DECLARE_DYNAMIC(CFileTransferDlg)

	public:
		CFileTransferDlg(CWnd* pParent = NULL);   // standard constructor
		virtual ~CFileTransferDlg();

	// Dialog Data
		enum { IDD = IDD_FILETRANSFER_DLG };

	protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
		afx_msg LRESULT OnUpdateFileTree(WPARAM wParam, LPARAM lParam);
		DECLARE_MESSAGE_MAP()
	public:
		//--------------------------------------------//
		//------------public variables----------------//
		//--------------------------------------------//
		// edit box to show text file content
		CEdit m_fileContentEdit;

		// tree to show files in remote disks
		DlgControls::CFileTreeCtrl m_fileTree;
		CImageList m_ImageList;
		char  m_diskName[2];
		char m_remoteFileName[56];
		CString m_storagePath;
		CString m_localFileFullName;

		// If we're busy downloading something...
		bool m_busy;

		// show the message for transfering file in file transfer dialog
		CEdit m_fileStatusEdit;

		/** The list of configured scanning systems */
		CListBox m_scannerList;

		/** array to store file information */
		CList <CScannerFileInfo*, CScannerFileInfo*> m_fileListA;
		CList <CScannerFileInfo*, CScannerFileInfo*> m_fileListB;
		
		/** list of Rxxx folders */
		CList<CScannerFolderInfo*, CScannerFolderInfo*> m_folderListB;

        /** The serial controller, which makes it possible to upload/download through serial. */
        Communication::CSerialControllerWithTx *m_SerialController = nullptr;

        /** The ftp-controller, which makes it possible to upload/download through FTP */
        Communication::CFTPHandler* m_ftpController = nullptr;

        /** The type of communication, must be either SERIAL_CONNECTION or FTP_CONNECTION */
        int m_communicationType = SERIAL_CONNECTION;

		/**selected item text*/
		CString m_selItemText;

		//--------------------------------------------//
		//------------public functions----------------//
		//--------------------------------------------//
		/** Initializes the controls in the dialog */
		virtual BOOL OnInitDialog();

		/** Called to upload to the currently selected file in the file tree */
		afx_msg void OnUploadFile();

		/** Called to download the currently selected file in the File Tree */	
		afx_msg void  OnDownloadFile();

		/** Called to download the currently selected folder in the File Tree */	
		afx_msg void  OnDownloadFolder();

		/** Called to download the currently selected disk in the File Tree */	
		afx_msg void  OnDownloadDisk();

		/**Called to delete file in the remote PC*/
		afx_msg void  OnDeleteFile();

		/**Called to download file from remote PC and show file in edit box*/
		afx_msg void  OnViewFile();

		/**Called to expand the contents of a folder on the remote PC */
		afx_msg void OnExpandFolder();
		
		afx_msg void OnTvnSelchangedFileTree(NMHDR *pNMHDR, LRESULT *pResult);

		/**fill file information */
		void FillList(CString &token, CList <CScannerFileInfo*, CScannerFileInfo*> &list);

		/** Parse the contents of a directory */
		int ParseDir(CString line, CList <CScannerFileInfo*, CScannerFileInfo*> &list);

		/**add one item into the file tree*/
		HTREEITEM AddOneItem( HTREEITEM hParent, LPSTR szText,	
				HTREEITEM hInsAfter,int iImage)	;
		afx_msg void OnLbnSelchangeScannerList();
		void ClearOut();


		long GetListFileSize(CString& inFileName, char disk);
		void SetFileItemImage(HTREEITEM hItem, CString subfix);
		/**set file subfix*/
		void SetFilter(TCHAR* filterText);
		afx_msg void OnBnClickedBrowseButton();
		/**clean file arrays - m_fileArrayA, m_fileArrayB*/
		void EmptyFileArray();
		// selected file to be uploaded
		CEdit m_uploadFileEdit;
		/**file to be uploaded, full file name with path */
		CString m_fileToUpload;
		/**previous selected scanner*/
		int m_preScanner;
		/**current selected scanner*/
		int m_curScanner;
		/**count how many times the tree is clicked*/
		int m_click;
		/**file list string for A disk*/
		CString m_textA;
		/**file list string for B disk*/
		CString m_textB;
		
		// record the disk name
		CComboBox m_diskCombo;
		afx_msg void OnBnClickedOk();
		afx_msg void OnBnClickedUpdatefilebtn();
		/**show the running information in the editbox of the dialog*/
		void ShowDlgInfo(CString& msg);
		bool DisconnectPrevScanner(int scanner);
		void ShowFileContent(CString& fileName);

		/** Returns true if the file with the supplied filename contains
				ASCII - text. */
		static bool IsTextFile(const CString &fileName);

		/** Returns true if the file with the supplied filename is a directory */
		static bool IsDir(const CString &fileName);
	};
}