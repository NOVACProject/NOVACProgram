#include "StdAfx.h"
#include "FTPController.h"

using namespace Communication;
extern CFormView *pView;
extern CConfigurationSetting g_settings;

CFTPController::CFTPController(void)
{   
  m_pakFileHandler = new FileHandler::CPakFileHandler();
}

CFTPController::~CFTPController(void)
{
	delete m_pakFileHandler;
}
	/**set ftp information*/
void CFTPController::SetFTPInfo(int mainIndex, CString& IP, CString& userName,CString pwd,long portNumber)
{
	CString listFileName;
	m_mainIndex					= mainIndex;
	m_ftpInfo.IPAddress = IP;
	m_ftpInfo.userName	= userName;
	m_ftpInfo.password	= pwd;
	m_ftpInfo.port			= portNumber;
	m_spectrometerSerialID.Format("%s", g_settings.scanner[mainIndex].spec[0].serialNumber);

	m_storageDirectory.Format("%sTemp\\%s\\",g_settings.outputDirectory,m_spectrometerSerialID);
	CreateDirectoryStructure(m_storageDirectory);
	m_listFileName.Format("%sfileList.txt", m_storageDirectory);
	//SetLogFileName(listFileName);
}

/**poll the scanner for upload*.pak files */
bool CFTPController::PollScanner()
{
	m_msg.Format("Start downloading from %s", m_ftpInfo.IPAddress);
	ShowMessage(m_msg);
	CString remoteFile = TEXT("UPLOAD.PAK");

	//if(!Login(m_ftpInfo.IPAddress,m_ftpInfo.userName, m_ftpInfo.password))
	//{		
	//	pView->PostMessage(WM_SCANNER_NOT_CONNECT,(WPARAM)&(m_spectrometerSerialID),0);
	//	return false;
	//}
	//pView->PostMessage(WM_SCANNER_RUN,(WPARAM)&(m_spectrometerSerialID),0);
		//download upload.pak	
	DownloadSpectra(remoteFile,m_storageDirectory);	
	//Disconnect();
	return true;

}
long CFTPController::GetFtpSpeed()
{
	CString remoteFile = TEXT("CFG.TXT");
	CString fileFullName = m_storageDirectory + remoteFile;
	long downloadDuration,fileSize,interval;
	time_t startTime,stopTime;
	
	time(&startTime);
	if(!DownloadRemoteFile(remoteFile, m_storageDirectory))
		return 0;
	time(&stopTime);
	
	downloadDuration = (long)(stopTime - startTime);
	fileSize = Common::RetrieveFileSize(fileFullName);
	interval = g_settings.scanner[m_mainIndex].comm.queryPeriod - downloadDuration;
	if(downloadDuration  <= 0)
		downloadDuration = 1;

	m_dataSpeed = fileSize/downloadDuration;
	
	return m_dataSpeed;
}
bool CFTPController::DownloadOldPak(long interval)
{
	int fileListSum = 0;	
	CString folder;
	folder.Format("");
	//get file and folder list	
	if( m_oldPakList.GetCount() + m_rFolderList.GetCount()<= 0)
	{
		fileListSum = GetPakFileList(); //download Uxxx.pak list
	}
	if(fileListSum == 0)
	{
		return false;
	}
	//download Uxxx.pak files in first directory
	if(m_oldPakList.GetCount() > 0)
		DownloadPakFiles(folder);
	//download Uxxx.pak files from Rxxx directories

/*	for(i=0;i< m_oldPakList.GetCount();i++)
	{
		fileSize = m_oldPakList.GetTail()->m_fileSize;
		//estimatedTime = fileSize/m_dataSpeed;		
		downloadResult = DownloadSpectra(m_oldPakList.GetTail()->m_fileName,m_storageDirectory);
		
		if( downloadResult )
			m_oldPakList.RemoveTail();
	}	*/
	return true;

}
//download pak files listed in m_oldpakList
bool CFTPController::DownloadPakFiles(CString& folder)
{
	CString fileName;
	bool downloadResult = false;
	if(folder.GetLength() == 4) // RXXX folder. Enter folder first
	{
		EnterFolder(folder);
	}

	while(m_oldPakList.GetCount() > 0)
	{
		fileName.Format("%s.pak", m_oldPakList.GetTail().m_fileName);	
		
		downloadResult = DownloadSpectra(fileName,m_storageDirectory);

		if( downloadResult)
			m_oldPakList.RemoveTail();
	}
	GoToUpperFolder();
	return true;
}
//get pak file list
bool CFTPController::GetPakFileList()
{
	char ipAddr[16];
	sprintf(ipAddr,"%s", m_ftpInfo.IPAddress);

	CString listfile,listText;
	listfile.Format("%s\\fileList.txt",m_storageDirectory);

	if(GetFileList())
	{
 		FillFileList();
	}
	if(m_oldPakList.GetCount() > 0)
		return true;
	else
		return false;

}

int  CFTPController::FillFileList()
{	
	CString resToken,str;
	int curPos = 0;
	int round = 0;
	int tokenLength = 0;
	CStdioFile file;
	CFileException fileException;
	if(!file.Open(m_listFileName, CFile::modeRead | CFile::typeText, &fileException))
	{
		m_msg.Format("Can not open %s", m_listFileName);
		ShowMessage(m_msg);
		return false;
	}
	
	while(file.ReadString(str))
	{		
		if(str.GetLength()==0)
			break;	
					
		resToken= str;//.Tokenize("\n",curPos);
		//resToken.Trim();
		tokenLength = resToken.GetLength();
	
		if(resToken.Find("Í")!=-1)
			break;
		
		if(tokenLength > 0)
		{
			ParseFileInfo(resToken);
			round++;
		}	
	}	// token length should be bigger than 56 bytes//!= 0);
	file.Close();
	return round;
}
void CFTPController::ParseFileInfo(CString line)
{
	CString resToken, resultStr,fileName;
	long fileSize;
	int curPos = 0;
	for(int i= 0;i < 9;i++)
	{
		resToken = line.Tokenize(" ",curPos);
		if(i== 4)
		{
			fileSize  = atoi(resToken);
		}
		else if(i== 8)
		{
			fileName  = resToken;
		}
	}
	m_fileList.AddTail( CFileInfo(fileName,fileSize));
	if(fileName.Find("U") != -1)
		m_oldPakList.AddTail(CFileInfo(fileName,fileSize)); 
}
bool CFTPController::DownloadSpectra(CString remoteFile,CString savetoPath)
{
	CString msg;
	m_localFileFullPath.Format("%s%s", savetoPath, remoteFile);

	//connect to the ftp server
	if(!DownloadRemoteFile(remoteFile,savetoPath))
	{
		m_statusMsg.Format("Can not download file from remote scanner (%s) by FTP", m_ftpInfo.IPAddress);
		ShowMessage(m_statusMsg);
		return false;
	}
	//calculate data transfer speed
 //call pakhandler
  if(1 == m_pakFileHandler->ReadDownloadedFile(m_localFileFullPath))
	{
		if(!DownloadRemoteFile(remoteFile,m_storageDirectory))
			return false;
		if(1 == m_pakFileHandler->ReadDownloadedFile(m_localFileFullPath))
		{
			ShowMessage("The pak file is corrupted");
			
			DeleteFile(m_localFileFullPath);// delete the local copy of Upload.pak
			//DELETE remote file
			DeleteRemoteFile(remoteFile); //add later
			return false;
		}			
  }
//DELETE remote file
	msg.Format("%s has been downloaded from %s", remoteFile, m_ftpInfo.IPAddress);
	ShowMessage(msg);		
	DeleteRemoteFile(remoteFile);  //add later
	return true;
}
//delete remote file in ftp server ( scanner)
BOOL CFTPController::DeleteRemoteFile(CString& remoteFile)
{
	BOOL result = FALSE;
	/*if(Connect(m_ftpInfo.IPAddress, m_ftpInfo.userName, m_ftpInfo.password)==1)
	{*/
		if(!FindFile(remoteFile))
		{
	//		Disconnect();
			return FALSE;
		}
		result = DeleteFTPFile(remoteFile);
	//	Disconnect();
	//}
	return result;
}


//download file from ftp server
bool CFTPController::DownloadRemoteFile(CString remoteFileName, CString savetoPath)
{	
	CString msg, fileFullName;
	fileFullName.Format("%s%s", savetoPath, remoteFileName);
	//check local file,if a file with same name exists, delete it
	if(IsExistingFile(fileFullName))
		DeleteFile(fileFullName);
	//connect to ftp server
	msg.Format("Begin to download file from %s", m_ftpInfo.IPAddress);
	ShowMessage(msg);

	//search file in the ftp server
	if(!FindFile(remoteFileName))
	{
		//Disconnect();
		msg.Format("%s doens't exist on remote PC %s now", remoteFileName,m_ftpInfo.IPAddress);
		ShowMessage(msg);
		return false;
	}
	//download the file
	msg.Format("ftpController: download file %s", fileFullName);
	ShowMessage(msg);
	if(!DownloadFile(remoteFileName, fileFullName))
	{		
		//Disconnect();
		return false;
	}
	//Disconnect();
	
	return true;
}
bool CFTPController::MakeCommandFile(char* cmdString)
{
	CString fileName;
	fileName.Format("%scommand.txt",m_storageDirectory);
	FILE *f = fopen(fileName,"w");
	if(f == NULL)
		return false;
	fprintf(f,cmdString); 
	fclose(f);
	return true;
}
bool CFTPController::SendCommand(char* cmd)
{
	CString localFileFullPath,remoteFile;
	remoteFile = _T("command.txt");
	localFileFullPath.Format("%sCOMMAND.TXT", m_storageDirectory);
	MakeCommandFile(cmd);
	//upload file to the server
	if(UploadFile(localFileFullPath )== 0)
	{
	//	Disconnect();
		return false;
	}
//	Disconnect();
	return true;

}
void CFTPController::GotoSleep()
{
	CString cmdFile = TEXT("COMMAND.TXT");
	DeleteRemoteFile(cmdFile);
	SendCommand("pause\npoweroff");
	pView->PostMessage(WM_SCANNER_SLEEP,(WPARAM)&(m_spectrometerSerialID),0);
	//download old pak files during sleeping time
	DownloadOldPak(14400);
}

void CFTPController::WakeUp()
{
	CString cmdFile = TEXT("COMMAND.TXT");
	DeleteRemoteFile(cmdFile);
	SendCommand("poweron\nresume");
}
void CFTPController::Reboot()
{
	CString cmdFile = TEXT("COMMAND.TXT");
	DeleteRemoteFile(cmdFile);
	SendCommand("reboot");
}
bool CFTPController::ArrangeFileList(char* string)
{
	int curPos = 0;	
	static char seps[]   = "\t\n";
	char *token;
	CString tokenStr;
//	CString str, resToken;
	CString strFileName,strFolder;
	strFileName.Format("U");
	strFolder.Format("drw-------");
	token = strtok( string, seps );
	while( token != NULL )
	{
	/* While there are tokens in "string" */
		tokenStr.Format("%s",token);
		//find the file name in the list
		if(tokenStr.Left(1)== strFileName)
		{
			if(tokenStr.Find("UPLOAD") == -1)
				AddPakFileInfo(tokenStr);			
		}			
		if(tokenStr.Find(strFolder) != -1)
		{
			AddFolderInfo(tokenStr);
		}
		
		/* Get next token: */
		token = strtok( NULL, seps );
	}
	return false;
}
void CFTPController::AddFolderInfo(CString string)
{
	int folderNameLength = string.Find(" ");
	if(folderNameLength !=4)
		return;
	CString folderName = string.Left(folderNameLength);
	if(folderName.Left(1) != _T("R"))
		return;
	m_rFolderList.AddTail(folderName);
	
}
// add the info of uxxx*.pak into list
void CFTPController::AddPakFileInfo(CString string)
{
	CString strSize, fileName;
	int sizeLength, fileSize; // length of the size string 
	int fileNameLength = string.Find(" ");
	if(fileNameLength < 0)
		return;
	
	fileName = string.Left(fileNameLength);
	string = string.Right(string.GetLength() - fileNameLength);
	string.Trim();
	string.TrimLeft("PAK");
	string.TrimLeft();
	sizeLength = string.Find(" ");
	if(sizeLength < 0)
		return;
	fileSize = atoi(string.Left(sizeLength));
	if(fileSize > 0)
	{
		m_oldPakList.AddTail(CFileInfo(fileName, fileSize));
		
	}

}