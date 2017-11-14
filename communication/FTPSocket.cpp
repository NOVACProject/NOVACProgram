#include "StdAfx.h"

#include "FTPSocket.h"
#include "..\Common\Common.h"
using namespace Communication;

extern CConfigurationSetting g_settings;

CFTPSocket::CFTPSocket(void)
{
	m_timeout = 15;
}

CFTPSocket::~CFTPSocket(void)
{
}

//login to one ftp server
bool CFTPSocket::Login(const CString ftpServerIP, CString userName, CString pwd,int ftpPort)
{
	sprintf(m_serverParam.m_serverIP, "%s", ftpServerIP);
	m_serverParam.m_serverPort = ftpPort;
	m_serverParam.userName = userName;
	m_serverParam.password = pwd;

	m_msg.Format("%s is not accessible, check the connection", ftpServerIP);	
	if(!Connect(m_controlSocket,m_serverParam.m_serverIP,ftpPort))
	{
		ShowMessage(m_msg);
		return false;
	}
	SendCommand("USER",userName);
	SendCommand("PASS",pwd);
	if(ReadResponse() == 1)
	{
		m_msg.Format("%s is connected", ftpServerIP);
		ShowMessage(m_msg);
		return true;
	}
	else
	{
		ShowMessage(m_msg);
		return false;
	}
}
void CFTPSocket::GetFileName(CString& filePath)
{
	int position  = filePath.ReverseFind('\\'); 
	int length    = filePath.GetLength();
	filePath      = filePath.Right(length - position - 1);
}
void CFTPSocket::GetSysType(CString& type)
{
	SendCommand("SYST","");
	if(ReadResponse() == 1)
		type.Format("%s",m_serverMsg);
	
}
bool CFTPSocket::EnterFolder(CString& folder)
{
	SendCommand("CWD",folder);
	if(ReadResponse()==1)
	{
		return true;
	}
	else
		return false;
}

bool CFTPSocket::GoToUpperFolder()
{
	SendCommand("CDUP","");
	return true;
}

//enter passive mode, use common m_timeout
bool CFTPSocket::EnterPassiveMode()
{
	int round = 0;
	time_t startTime = -1;
	time_t timeNow = -1;
	time(&startTime);
	do
	{
		// Check for time-outs...
		if(timeNow - startTime > m_timeout)
			return false;
		time(&timeNow);

		// Send the command to enter passive mode
		SendCommand("PASV","");
		round++;
		Sleep(600);

		// Read the response from the server
		if(-1 == ReadResponse())
			continue; // No response...
		if(!IsFTPCommandDone())
			return false; // Server responded with an error
		if(-1 == m_serverMsg.Find("Passive")){
			// If we didn't get the correct answer from the server, then try again!
			continue;
		}

		// Get the port-number we should connect to
		m_serverParam.m_serverDataPort = GetPortNumber();
		if(m_serverParam.m_serverDataPort <= 0 || m_serverParam.m_serverDataPort > 65536){
			continue; // we could not get an ok port-number
		}

		// Connect to the server
		if(!Connect(m_dataSocket,m_serverParam.m_serverIP,m_serverParam.m_serverDataPort))
		{
			m_msg.Format("can not connect to server %s:%d,server msg:%s", m_serverParam.m_serverIP,m_serverParam.m_serverDataPort,m_serverMsg);
		
			ShowMessage(m_msg);
			m_serverParam.m_serverDataPort  = 0;
		}
	}while(m_serverParam.m_serverDataPort <= 0 || m_serverParam.m_serverDataPort > 65535);
	
	return true;
}
bool CFTPSocket::GetFileList()
{
	m_serverMsg = "";

	//enter passive mode because the client thus can work behind firewall
	if(!EnterPassiveMode())
		return false;

	m_msg.Format("connect to server %s:%d", m_serverParam.m_serverIP,m_serverParam.m_serverDataPort);
	ShowMessage(m_msg);

	return List();
}
//get file name list from the FTP server
bool CFTPSocket::GetFileNameList()
{
	//enter passive mode because the client thus can work behind firewall
	if(!EnterPassiveMode())
		return false;
	m_msg.Format("connect to server %s:%d", m_serverParam.m_serverIP,m_serverParam.m_serverDataPort);
	ShowMessage(m_msg);	//for test
	return NameList();  // test name list.	
}
bool CFTPSocket::NameList()
{
//-----------------
	SendCommand("NLST","");
	if(ReadData())
	{
		m_msg.Format("Successfully read list data");
		ShowMessage(m_msg);
		return true;
	}
	else
	{
		m_msg.Format("Can not read  list data");  //for test
		ShowMessage(m_msg);
		return false;
	}

}
//FTP command to get file information- file name, size, date
bool CFTPSocket::List()
{
	// Send command for file-list
	SendCommand("LIST","");

	//make sure that the server has replied the command 2006.11.21
	ReadResponse();
	if(!IsFTPCommandDone())
		return false;

	//read file dir list
	if(ReadData())
	{
		m_msg.Format("Successfully read list data");
		ShowMessage(m_msg);
		return true;
	}	
	else
	{
		m_msg.Format("Can not read  list data");  //for test
		ShowMessage(m_msg);
		return false;
	}
}

bool CFTPSocket::UploadFile(CString fileLocalPath)
{
	bool result;
	SendCommand("TYPE","I");
	if(!EnterPassiveMode())
		return false;

		//Send the STOR command
	CString fileName = fileLocalPath;
	//check file existence first - to be done
	GetFileName(fileName);		
	
	SendCommand("STOR ", fileName);
	result = SendFileToServer(fileLocalPath);
	if(result)
	{
		m_msg.Format("%s has been uploaded to %s", fileName,m_serverParam.m_serverIP);
		ShowMessage(m_msg);
	}
	else
	{
		m_msg.Format("%s couldn't be uploaded to %s", fileName,m_serverParam.m_serverIP);
		ShowMessage(m_msg);
	}
	return result;
}
bool CFTPSocket::DeleteFTPFile(CString fileName)
{
	SendCommand("DELE",fileName);
	if(ReadResponse()!=1)
		return false;
	if(IsFTPCommandDone())
		return true;
	else
		return false;
}
int CFTPSocket::SendCommand(CString command,CString commandText)
{
	char buf[100];
	Sleep(100); // Added 2008.06.30 to work with the Axis computer
	if(commandText.GetLength() == 0)
		wsprintf(buf,"%s\r\n", command);
	else
		wsprintf(buf,"%s %s\r\n", command, commandText);
	int result = send(m_controlSocket,buf,strlen(buf),0);
	if(result == SOCKET_ERROR)
	{
		result = WSAGetLastError();
		WSACleanup();
	}
	//	ShowMessage(command); //for test
	result = 0;
	return result;
}

bool CFTPSocket::SendFileToServer(CString& fileLocalPath)
{
	long fileSize;
	HANDLE hFile; 
	int errorNum = 0;
	hFile = CreateFile(fileLocalPath,        // open file in local disk
                GENERIC_READ,              // open for reading 
                FILE_SHARE_READ,           // share for reading 
                NULL,                      // no security 
                OPEN_EXISTING,             // existing file only 
                FILE_ATTRIBUTE_NORMAL,     // normal file 
                NULL);                     // no attr. template 
	 
	if (hFile == INVALID_HANDLE_VALUE) 
	{ 
		ShowMessage("Could not open file.");   // process error 
		return false;
	} 
	fileSize = GetFileSize(hFile, NULL);
	if( TransmitFile(m_dataSocket,hFile,fileSize,0,NULL,NULL,TF_DISCONNECT)==FALSE) 
	{
		errorNum = WSAGetLastError();
		WSACleanup();
		ShowMessage("Cannot upload file");
		return false;
	}
	return true;

}
bool CFTPSocket::ReadData()
{
#define LOCAL_BUF_SIZE 1024
	
	int bytesRecv = 0;
	long errorNum = 0;
	long byteSum = 0;
	time_t startTime,stopTime;
	int count = 0;

	TByteVector vLocalBuf(LOCAL_BUF_SIZE);
	Sleep(50);
	time(&startTime);
	do
	{
		if(IsDataReady(m_dataSocket, 10))
			bytesRecv = recv(m_dataSocket, &(*(vLocalBuf.begin())),LOCAL_BUF_SIZE, 0);
		else
		{
			ShowMessage("Read data timeout");
			return false;
		}
		StoreReceivedBytes(vLocalBuf, bytesRecv);
		byteSum += bytesRecv;
		count++;
		time(&stopTime);
		if((stopTime - startTime >= m_timeout)&&(byteSum <= 0))
		{
			m_msg.Format("ReadData error num: %d", errorNum);
			ShowMessage(m_msg);
			return false;
		}
	}while((bytesRecv !=0));
		
	m_msg.Format("data is %d bytes",byteSum);
	ShowMessage(m_msg);
	m_msg.Format("read data cycle: %d in %d seconds", count,stopTime-startTime);
	ShowMessage(m_msg);
	if(errorNum == 0)
	{
		WriteVectorFile(m_listFileName,m_vDataBuffer, m_vDataBuffer.size());
		return true;
	}
	else
	{
		m_msg.Format("ReadData error num: %d", errorNum);
		ShowMessage(m_msg);
		return false;
	}
}
//set the name for m_listFileName
void CFTPSocket::SetLogFileName(const CString fileName)
{
	m_listFileName.Format("%s",fileName);
}
//find a file in ftp server
bool CFTPSocket::FindFile(CString fileName)
{
	CString  listFileName, list;
	time_t startTime,stopTime;
	time(&startTime);
	bool result = false;
	if(!GetFileNameList())
		return false;

	CStdioFile listFile;
	CFileException fileException;
	listFileName.Format("%s", m_listFileName);
	if(!listFile.Open(listFileName, CFile::modeRead | CFile::typeText, &fileException))
	{
		m_msg.Format("Can not open %s", listFileName);
		ShowMessage(m_msg);
		return false;
	}
	while(listFile.ReadString(list))
	{
		if(list.Find(fileName)!=  -1)
		{
			listFile.Close();
			time(&stopTime);
			m_msg.Format("Find a file using %d", stopTime -startTime);
			ShowMessage(m_msg);
			return true;
		}
	}
	time(&stopTime);
	m_msg.Format("Try to find a file using %d", stopTime -startTime);
	ShowMessage(m_msg);

	listFile.Close();
	return result;
}
void CFTPSocket::WriteVectorFile(CString fileName, const TByteVector& vBuffer, long receivedBytes)
{
	FILE *localFile;
	
	localFile = fopen(fileName,"w");
	if(localFile < (FILE*)1)
	{
		ShowMessage("fileList.txt can not be written");
		return;
	}
	fwrite(&(*vBuffer.begin()), receivedBytes,1,localFile);
	fclose(localFile);
	m_vDataBuffer.clear();
}
void CFTPSocket::StoreReceivedBytes(const TByteVector& vBuffer, long receivedBytes)
{
	if(receivedBytes <= 0)
		return;
	std::copy(vBuffer.begin(), vBuffer.begin()+receivedBytes, std::back_inserter(m_vDataBuffer));
}
int CFTPSocket::ReadResponse()
{
	int bytesRecv = SOCKET_ERROR;
	long errorNum = 0;
	int count = 0;
	time_t startTime, stopTime;
	memset(m_receiveBuf,0,RESPONSE_LEN);
	time(&startTime);
	do
	{
		Sleep(100);
		if(IsDataReady(m_controlSocket, 10))
			bytesRecv = recv( m_controlSocket, m_receiveBuf, RESPONSE_LEN, 0 );
		else
		{
			ShowMessage("Read data timeout");
			return -1;
		}
		
		if (bytesRecv < 0)
		{
			errorNum = WSAGetLastError();
			if(errorNum == WSAETIMEDOUT||errorNum == WSAENOTCONN||errorNum==WSAENETRESET||errorNum == WSAESHUTDOWN)
			{
				m_msg.Format("error num in ReadResponse %d", errorNum);
				ShowMessage(m_msg);
				return errorNum;
			}
		}
		else
			break;

		count++;
		time(&stopTime);
		if(stopTime - startTime >= m_timeout)
		{
			ShowMessage("Read data timeout");
			return -1;
		}
	}while(bytesRecv!= 0);

	m_serverMsg.Format("%s",m_receiveBuf);
	//msg.Format("%s",m_receiveBuf);
	if(errorNum == 0)
		return 1;
	else
	{
		m_msg.Format("Read response Timeout.");
		ShowMessage(m_msg);
		return -1;
	}
}

bool CFTPSocket::Connect(SOCKET& usedSocket,char* serverIP, int serverPort)
{
		// Initialize Winsock.
		WSADATA wsaData;
		int iResult = WSAStartup( MAKEWORD(2,2), &wsaData );
		if ( iResult != NO_ERROR )
			ShowMessage(_T("Error at WSAStartup()"));

		// Create a socket.
		usedSocket = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP );

		if ( usedSocket == INVALID_SOCKET ) 
		{
			m_msg.Format( "Error at socket(): %ld", WSAGetLastError());
			ShowMessage(m_msg );
			WSACleanup();
			return false;
		}
	// Bind the socket.
	sockaddr_in service;

	service.sin_family = AF_INET;
	service.sin_addr.s_addr = inet_addr(serverIP);
	service.sin_port = htons( serverPort );

		 // Connect to server.
	if ( connect( usedSocket,(SOCKADDR*)&service,sizeof(service)) == SOCKET_ERROR) 
	{
		ShowMessage( _T("Failed to connect." ));
		iResult = WSAGetLastError();
		WSACleanup();

		return false;
	}
	return true;
}
int CFTPSocket::CloseASocket(SOCKET sock)
{
	/*
	 1. Call WSAAsyncSelect to register for FD_CLOSE notification.
   2. Call shutdown with how=SD_SEND.
   3. When FD_CLOSE received, call recv until zero returned, or SOCKET_ERROR.
   4. Call closesocket.

	*/
	int ret;
	int reply = -1;
	char buf[1024];
	int bytesRecv = 0;
	try
	{

		ret = WSAAsyncSelect(sock,NULL,NULL,FD_CLOSE);
		
			if(ret == 0)
			{
				reply = shutdown(sock,1);
			
				do
				{
					bytesRecv = recv( sock, buf, 1024, 0 );			
					break;
				}
				while(bytesRecv!=0);
				closesocket(sock);
			}
			else
			{
				shutdown(sock,1);
				reply = closesocket(sock);
			}
	}
	catch (std::exception pEx)
	{
		ShowMessage("Disconnect exception");
	}
	return reply;

}
int CFTPSocket::Disconnect()
{
	int reply;
	reply = CloseASocket(m_controlSocket);
	reply += CloseASocket(m_dataSocket);
	return reply;
}
int CFTPSocket::GetPortNumber()
{
	int portNumber = 0;
	int serverIPPort[32];
	CString resToken,str;
	int curPos = 0;
	int index = 0;
	int tokenLength = 0;
	if(m_serverMsg.GetLength()==0)
		return 0;
	str = m_serverMsg;
	CString seperator1 = TEXT("(");
	CString seperator2 = TEXT(")");
	GetCitedString(str, seperator1, seperator2);
	do
	{
		resToken= str.Tokenize(",",curPos);
		tokenLength = resToken.GetLength();
		serverIPPort[index] = atoi(resToken);
		index++;
	}	while (tokenLength != 0);
	portNumber = serverIPPort[4]*256 + serverIPPort[5];

	if(portNumber < 0 || portNumber > 65536){
		str.Format("Recieved illegal port-number; %s", m_serverMsg);
		ShowMessage(str);
	}

	return portNumber;
}
bool CFTPSocket::GetServerFeature()
{
	SendCommand("FEAT","");
	if(ReadResponse()==1)
		return true;
	else
		return false;
}
//NOT FOR THE NOVAC SCANNER FTP
long CFTPSocket::Size(CString& fileName)
{
	long fileSize = -1;
	CString list,sizeStr,listFile;
	listFile.Format("%sTemp\\fileList.txt", g_settings.outputDirectory);
	GetFileList();
	SendCommand("SIZE",fileName);
	ReadData();
	sizeStr = m_serverMsg;
	MessageBox(NULL,sizeStr,"notice",MB_OK);
	return fileSize;
}
//NOT FOR NOVAC SCANNER FTP SERVER
bool CFTPSocket::CheckFileExistence(CString& remoteFileName, CString& remoteDirectory)
{
	CString serverReply;
	SendCommand("SIZE", remoteFileName);
	if(ReadResponse()!=1)
		return false;
	serverReply = m_serverMsg;
	SendCommand("CWD", remoteDirectory + remoteFileName);
	if(ReadResponse()!=1)
		return false;
	if(m_serverMsg.Find("550") != -1)
		return true;
	else 
		return false;	
}
int CFTPSocket::GetMsgCode(CString& ftpMsg)
{
	if(ftpMsg.GetLength() == 0)
		return -1;
	int curWordPos = 0;
	int curLinePos =  0;
	
	int round = 0;
	CString resToken,line;
	//Empty the list first
	if(m_ftpCode.GetCount() > 0)
		m_ftpCode.RemoveAll();
	//int stopPos = ftpMsg.ReverseFind('\n') -1;
	do
	{
		curWordPos = 0;
		line = ftpMsg.Tokenize("\n",curLinePos);
		if(line.GetLength() == 0)
			return round;
		resToken = line.Tokenize(" ",curWordPos);
		m_ftpCode.AddTail(atoi(resToken));
		round++;
	}while(line.GetLength() != 0);
	round = round -1;
	return round;

}
int CFTPSocket::JudgeFTPCode(int code)
{
	//ftp mistakes
	if(code < 400 && code > 0)
	{
		return TRUE;
	}
	else if(code >= 400)
	{
		m_msg.Format("error happened when communicating with ftp, code %d.", code);
		ShowMessage(m_msg);
		return FALSE;
	}
	return FALSE;
}

int CFTPSocket::DownloadFile(CString remoteFileName,CString localFileName)
{
	#define DOWNLOAD_BUF_SIZE 4096
	time_t startTime, stopTime;
	long fileSize = 0;
	float fileKBytes;
	char buf[DOWNLOAD_BUF_SIZE];
	DWORD bytesWritten;
	int bytesRecv;
	long errorNum;
	DWORD lastBytesWritten;
	// check file existence in remote ftp server  - to be done
	if(!OpenFileHandle(localFileName))
		return -1;

	//receive file parts
	time(&startTime);
	SendCommand("TYPE","I");
	
	if(!EnterPassiveMode())
		return 0;
	
	SendCommand("RETR",remoteFileName);
	//check file existence
	ReadResponse();
	if(!IsFTPCommandDone())
		return 0;
	lastBytesWritten = 0;
	do
	{
		bytesRecv = recv( m_dataSocket, buf, DOWNLOAD_BUF_SIZE, 0 );
	
		if (bytesRecv < 0)
		{
			errorNum = WSAGetLastError();
		}
		else
		{
			fileSize+= bytesRecv;
			WriteFile(m_hDownloadedFile,buf + lastBytesWritten,bytesRecv,&bytesWritten,NULL);
			lastBytesWritten = bytesWritten;
		}
	}while( bytesRecv!= 0 ) ;

	CloseHandle(m_hDownloadedFile);  //close the downloaded file handle so that it can be used.
	if ( bytesRecv == SOCKET_ERROR )
	{
		m_msg.Format("recv failed: %d ", WSAGetLastError());
		ShowMessage(m_msg);  // for test
		CloseHandle(m_hDownloadedFile);
		return -1;
	}
	//show time duration
	time(&stopTime);
	fileKBytes = (float)(fileSize / 1024.0);
	if(stopTime - startTime != 0)
	{	
		m_msg.Format("%f kBytes File is downloaded in %d seconds, speed is %f KBytes/sec", fileKBytes,
			stopTime - startTime, fileKBytes/(stopTime-startTime)	);
		ShowMessage(m_msg);
	}
	return 1;
}
bool CFTPSocket::OpenFileHandle(CString& fileName)
{
	int errorNum = 0;
	m_hDownloadedFile = CreateFile(fileName,        // open file in local disk
                GENERIC_WRITE,              // open for writing 
                FILE_SHARE_WRITE,           // share for writing 
                NULL,                      // no security 
                CREATE_ALWAYS,             // Opens the file, if it exists. If the file does not exist, the function creates the file
                FILE_ATTRIBUTE_NORMAL,     // normal file 
                NULL);                     // no attr. template 
	 
	if (m_hDownloadedFile == INVALID_HANDLE_VALUE) 
	{
		m_msg.Format("Can not open file %s", fileName);
		ShowMessage(m_msg);   // process error 
		return false;
	}
	return true;
}
bool CFTPSocket::SetCurrentFTPDirectory(CString& directory)
{
	SendCommand("CWD", directory);
	if(ReadResponse()!=1)
		return false;
	bool returnFlag = IsFTPCommandDone();
	CString curDirectory;
	GetCurrentFTPDirectory(curDirectory);
	return returnFlag;
}

bool CFTPSocket::IsFTPCommandDone()
{
	int code = GetMsgCode(m_serverMsg);
	int i;
	int suc = TRUE;
	
	POSITION listPos = m_ftpCode.GetHeadPosition();
	if(listPos == NULL){
		return false; // no ftp-code received at all...
	}
	for(i = 0; i < m_ftpCode.GetCount(); i++)
	{
		suc = suc * JudgeFTPCode(m_ftpCode.GetNext(listPos));
	}
	
	if(suc == TRUE)
		return true;
	else
		return false;
}
bool CFTPSocket::GetCurrentFTPDirectory(CString& curDirectory)
{
	int msgLength, directoryLength;
	CString seperator1 = _T("\"");
	CString seperator2 = _T("\"");
	SendCommand("PWD", "");
	if(ReadResponse()!=1)
		return false;
	
	curDirectory = m_serverMsg;
	msgLength = m_serverMsg.GetLength();
	bool returnFlag = IsFTPCommandDone();
	if(returnFlag)
	{
		GetCitedString(curDirectory, seperator1,seperator2);	
		directoryLength = curDirectory.GetLength();
		if(directoryLength == msgLength)
			returnFlag = false;
		m_msg.Format("Current directory is %s", curDirectory);
		ShowMessage(m_msg);
	}
	return returnFlag;

}
void CFTPSocket::GetCitedString(CString& line, CString& leftSeperator,CString& rightSeperator)
{
	int leftPosition, rightPosition, msgLength;

	leftPosition = line.Find(leftSeperator);
	msgLength = line.GetLength();
	line = line.Right(msgLength - leftPosition -1);
	rightPosition = line.Find(rightSeperator);	

	if(leftPosition == -1 || rightPosition == -1 )
		return ;

	line = line.Left(rightPosition);

}
bool CFTPSocket::CreateFTPDirectory(CString& parentDirectory, CString& newDirectory)
{
	CString curDirectory;
	if(!GetCurrentFTPDirectory(curDirectory))
		return false;
	SetCurrentFTPDirectory(parentDirectory);
	SendCommand("MKD", newDirectory);
	if(ReadResponse()!=1)
		return false;
	if(IsFTPCommandDone())
		return true;
	else
		return false;

}
bool CFTPSocket::IsDataReady(const SOCKET& socket, long timeout)
{
	timeval expireTime;
	expireTime.tv_sec = timeout;
	expireTime.tv_usec = 0;
	fd_set socketSet;
	socketSet.fd_count  = 1;
	socketSet.fd_array[0] = socket;
	int result = select(0,&socketSet,NULL,&socketSet,&expireTime);
	if(result == 1)
		return true;
	else
		return false;
}