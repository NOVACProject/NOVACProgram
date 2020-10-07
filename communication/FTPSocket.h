#pragma once

#include "afxsock.h"
#include <vector>
#include <afxtempl.h>
#define RESPONSE_LEN 12288

#include "../Configuration/Configuration.h"

namespace Communication
{
	class CFTPSocket
	{
		typedef std::vector<char> TByteVector;

	public:
		CFTPSocket(int timeout=30);
		
		~CFTPSocket(void);
		// ----------------------------------------------------------------------
		// ---------------------- PUBLIC METHODS --------------------------------
		// ----------------------------------------------------------------------

		/**Send command to FTP server
		  @command - the standard commands that are listed in RFC 959
		  @commandText - the parameters to be set in this command. 
		   When there is no parameter to be used, this value is "".
		  @return the error code if this thread's last Windows Sockets operation failed.
		  @return 0 if successful.
		*/
		int SendCommand(CString command, CString commandText);

		/**Connect FTP server
		   @usedSocket - the socket to be used to make connection
		   @serverIP - server ip address
		   @serverPort - the port on the server for connection
		   @return true if connected
		*/
		bool Connect(SOCKET& usedSocket, char* serverIP, int serverPort);

		/**Read the control response from the server, write message to m_serverMsg
		    @return true if there is response
		*/
		int ReadResponse();

		/**Read data from the FTP server*/
		bool ReadData();

		/** Set the ftp log file name*/
		void SetLogFileName(const CString fileName);

		/**Upload a file from the local path
		   @fileLocalPath file full path including file name
		*/
		bool UploadFile(CString fileLocalPath);

		/**delete a file in remote ftp server*/
		bool DeleteFTPFile(CString fileName);

		/**Get file name from full file path
		    @filePath - file's full path
		*/
		void GetFileName(CString& filePath);

		/**Get port number from FTP server's response
		    @return port number if successful
		    @return 0 if it fails
		*/
		int GetPortNumber();

		bool SendFileToServer(CString& fileLocalPath);

        /**Login to one FTP server
            @param ftpServerIPOrHostName - ip address or hostname of the ftp server.
                Maximum length of this string is 255 characters.
            @param userName - Login name
            @param pwd - Login password
            @param ftpPort - ftp server's port, for control channel, standard is 21.
            @return true if successful */
        bool Login(const CString& ftpServerIPOrHostName, const CString& userName, const CString& pwd, int ftpPort = 21);

		/**Get system type*/
		void GetSysType(CString& type);

		/*Enter one folder*/
		bool EnterFolder(CString& folder);

		/*Go to upper folder*/
		bool GoToUpperFolder();

		/**Enter passive mode*/
		bool EnterPassiveMode();

		/**Send list command to the FTP server*/
		bool List();

		/**Send nlst command to the FTP server*/
		bool NameList();

		/**List the file names from the FTP server*/
		bool GetFileNameList();

		/**List the files and other file information*/
		bool GetFileList();

		/**Get the FTP server's features*/
		bool GetServerFeature();

		/**Get the remote file size in FTP server,NOT FOR NOVAC SCANNER FTP SERVER
		  @fileName - the remote file's name
		  @return file size if successful,
		  @return -1 if fails
		*/
		long Size(CString& fileName);

		/**Check whehter the remote file exists
		*@remoteFileName - the remote file name 
		*@remoteDirectory - remote directory
		*return true if exists
		*return false if the file does not exist
		*/
		bool CheckFileExistence(CString& remoteFileName, CString& remoteDirectory);
		
		/**Get FTP error code which is at the beginning of the reply
		*@ftpMsg- the mesage that the FTP server responses.
		*return 0 if fail to find a number at the beginning of the reply
		*return a positive integer if successful
		*/
		int GetMsgCode(CString& ftpMsg);
		
		/**judge the ftp error code meaning
		*@code - the ftp error code
		*return TRUE if successful
		*return FALSE if there is error in last communication
		*/
		int JudgeFTPCode(int code);
		
		/**Judge successful or not from the ftp reply message 
		*return true if successful
		*/
		bool IsFTPCommandDone();
		
		int DownloadFile(CString remoteFileName,CString localFileName);
		
		bool OpenFileHandle(CString& fileName);
		
		/**Makes the given directory to be the current directory in the FTP server
		*@directory the directory to be change to in the FTP server
		*/
		bool SetCurrentFTPDirectory(CString& directory);
		
		/**Get the current directory*/
		bool GetCurrentFTPDirectory(CString& curDirectory);
		
		/**Get a string which is in one pair of seperator
		*@param line a string which includes the wanted part
		*@seperator one pair of seperators which keep the wanted part in the middle
		*for example, 220 xx "\" is the current. "\" is what we want
		*/
		void GetCitedString(CString& line, CString& leftSeperator, CString& rightSeperator);
		
		/**Create a directory in remote FTP server
		*@param parenetDirectory parent directory name
		*@param newDirectory new directory name
		*/
		bool CreateFTPDirectory(CString& parentDirectory, CString& newDirectory);
		
		/**close socket*/
		int Disconnect();
		
		int CloseASocket(SOCKET sock);
		
		/**find a file in ftp server
		*return true if it finds the file
		*/
		bool FindFile(CString fileName);
		
		/**store receive data from data socket in m_vDataBuffer*/
		virtual void StoreReceivedBytes(const TByteVector& vBuffer, long receivedBytes);
		
		/**write the data from the vector into a file*/
		void WriteVectorFile(CString fileName, const TByteVector& vBuffer, long receivedBytes);
		
		/**check whether there  is data to be read*/	
		bool IsDataReady(const SOCKET& socket, long timeout);
		
		// ----------------------------------------------------------------------
		// ---------------------- PUBLIC DATA -----------------------------------
		// ----------------------------------------------------------------------
	public:
		/**special socket for control connection */
		SOCKET m_controlSocket;

		/**the server's reply */
		CString m_serverMsg;

		CString m_msg;

		/**data structure to store ftp server parameters*/
		struct ftpParam
		{
			char m_serverIP[256];
			int m_serverPort;
			int m_serverDataPort;
			CString userName;
			CString password;
		};
		typedef struct ftpParam ftpParameter;
		ftpParameter m_serverParam;

		/**file handle to the file to be downloaded*/
		HANDLE m_hDownloadedFile;

		CString m_listFileName; // file to store the file list of the FTP server

		// ----------------------------------------------------------------------
		// ---------------------- PRIVATE DATA -----------------------------------
		// ----------------------------------------------------------------------
	private:
		/**socket for data connection - to send or download data*/
		SOCKET m_dataSocket;

		/**longest duration for one connection*/
		long m_timeout;

		/**buffer to store received control info from the FTP server*/
		char m_receiveBuf[RESPONSE_LEN];

		/**vector to store received data from the FTP server*/
		TByteVector m_vDataBuffer;

		/**list to store the ftp codes which indicate the status of last communication */
		CList<int,int> m_ftpCode;
	};
}