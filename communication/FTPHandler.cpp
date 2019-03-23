#include "StdAfx.h"
#include "ftphandler.h"
#include "../Common/CfgTxtFileHandler.h"

using namespace Communication;

extern CFormView *pView;                   // <-- the main window
extern CConfigurationSetting g_settings;   // <-- the settings
extern CWinThread *g_comm;                 // <-- The communication controller

CFTPHandler::CFTPHandler(void)
{
	m_pakFileHandler = new FileHandler::CPakFileHandler();
	m_dataSpeed = 4.0;
	m_electronicsBox = BOX_VERSION_1;
}

CFTPHandler::CFTPHandler(ELECTRONICS_BOX box)
{
	m_pakFileHandler = new FileHandler::CPakFileHandler();
	m_dataSpeed = 4.0;
	this->m_electronicsBox = box;
}

CFTPHandler::~CFTPHandler(void)
{
	CString message;

	// Tell the world...
	message.Format("<node %d> terminated", m_mainIndex);
	ShowMessage(message);
	
	delete m_pakFileHandler;
}

/**set ftp information*/
void CFTPHandler::SetFTPInfo(int mainIndex, CString& IP, CString& userName,CString &pwd,long portNumber)
{
	CString errorMsg;

	m_mainIndex         = mainIndex;
	m_ftpInfo.IPAddress = IP;
	m_ftpInfo.userName  = userName;
	m_ftpInfo.password  = pwd;
	m_ftpInfo.port      = portNumber;
	m_spectrometerSerialID.Format("%s", (LPCSTR)g_settings.scanner[mainIndex].spec[0].serialNumber);

	m_storageDirectory.Format("%sTemp\\%s\\", (LPCSTR)g_settings.outputDirectory, (LPCSTR)m_spectrometerSerialID);
	if(CreateDirectoryStructure(m_storageDirectory)){ // Make sure that the storage directory exists
		GetSysTempFolder(m_storageDirectory);
		if(CreateDirectoryStructure(m_storageDirectory)){
			errorMsg.Format("FTPHandler: Could not create temporary-directory for spectrometer; %s", (LPCSTR)m_spectrometerSerialID);
			ShowMessage(errorMsg);
			MessageBox(NULL, errorMsg, "Serious error", MB_OK);
		}
	}
}

void CFTPHandler::SetFTPInfo(int mainIndex, CString& IP, CString& userName, CString &pwd, CString &admUserName, CString &admPwd, long portNumber){

	this->m_ftpInfo.adminUserName.Format(admUserName);
	this->m_ftpInfo.adminPassword.Format(admPwd);

	this->SetFTPInfo(mainIndex, IP, userName, pwd, portNumber);
}

/**poll the scanner for upload*.pak files */
bool CFTPHandler::PollScanner()
{
	CString msg;
	msg.Format("<node %d> Checking for files to download", m_mainIndex);
	ShowMessage(msg);

	bool result = DownloadAllOldPak();
	return result;
}

//download pak files listed in m_fileInfoList
bool CFTPHandler::DownloadPakFiles(const CString& folder)
{
	CString fileName,workPak, uploadPak;
	workPak.Format("WORK.PAK");
	uploadPak.Format("upload.pak");
	bool downloadResult = false;
	CScannerFileInfo* fileInfo = new CScannerFileInfo();

	//connect to server
	if(Connect(m_ftpInfo.IPAddress, m_ftpInfo.userName, m_ftpInfo.password)!=1)
	{
		pView->PostMessage(WM_SCANNER_NOT_CONNECT,(WPARAM)&(m_spectrometerSerialID),0);
		return false;
	}

	// if we should download data from a folder, then enter the folder first
	if(folder.GetLength() > 0)
		EnterFolder(folder);

	time_t start;
	time(&start);
	time_t current;
	while(m_fileInfoList.GetCount() > 0)
	{
		fileInfo = &m_fileInfoList.GetTail();
		if(m_electronicsBox == BOX_VERSION_2){
			fileName.Format("%s.pak", (LPCSTR)fileInfo->fileName);
		}else{
			fileName.Format("%s.PAK", (LPCSTR)fileInfo->fileName);
		}
		m_remoteFileSize = fileInfo->fileSize;

		if((Equals(fileName, workPak) || Equals(fileName, uploadPak)) && folder.GetLength() == 0)
		{
			m_fileInfoList.RemoveTail();
			continue;
		}

		m_statusMsg.Format("Begin to download %s/%s", (LPCSTR)folder, (LPCSTR)fileName);
		downloadResult = DownloadSpectra(fileName, m_storageDirectory);

		if (downloadResult) {
			m_fileInfoList.RemoveTail();
		}
		else {
			break;	//get out of loop, 2007.4.30
		}

		time(&current);
		double seconds = current - start;
		long queryPeriod = g_settings.scanner[m_mainIndex].comm.queryPeriod;
		if (seconds > queryPeriod) {
			break; // spent long enough on one scanner; move to next
		}
	}

	m_fileInfoList.RemoveAll();
	Disconnect();
	return true;
}

bool CFTPHandler::DownloadAllOldPak()
{
	long fileListSum = 0;
	CString folder, msg;
	folder.Format("");

	//connect to scanner ftp server

	//get file and folder list
	fileListSum = m_fileInfoList.GetCount() + m_rFolderList.GetCount();
	if(fileListSum <= 0)
	{
		fileListSum = GetPakFileList(folder); //download Uxxx.pak list
		fileListSum = max(0, fileListSum); // These must be on separate lines otherwise there's risk that the scanner will be polled twice(!)
	}
	if(fileListSum + m_rFolderList.GetCount() == 0)
	{
		msg.Format("<node %d> No more files to download", m_mainIndex);
		ShowMessage(msg);
		return false;
	}
	
	// download Uxxx.pak files in top directory
	Sleep(5000);
	if(m_fileInfoList.GetCount() > 0)
		DownloadPakFiles(folder);

	if(m_rFolderList.GetCount() > 0)
	{
		// Make a backup-copy of the folder-list
		CList <CString, CString &> localFolderList;
		localFolderList.AddTail(&m_rFolderList);


		time_t start;
		time(&start);
		time_t current;
		while(localFolderList.GetCount() > 0){
			m_fileInfoList.RemoveAll();

			// Get the folder name
			folder.Format("%s", (LPCSTR)localFolderList.GetTail());
			localFolderList.RemoveTail();

			if(GetPakFileList(folder) < 0)
				return true;

			Sleep(5000);
			if(DownloadPakFiles(folder))
			{
				// we managed to download the files, now remove the folder
				if(Connect(m_ftpInfo.IPAddress, m_ftpInfo.userName, m_ftpInfo.password)!=1)
				{
					pView->PostMessage(WM_SCANNER_NOT_CONNECT,(WPARAM)&(m_spectrometerSerialID),0);
					return false;
				}
				DeleteFolder(folder);
				Disconnect();
			}
			else
			{
				// we failed to download the files...
				Disconnect(); //get out of loop 2007.4.30
			}
			time(&current);
			double seconds = current - start;
			long queryPeriod = g_settings.scanner[m_mainIndex].comm.queryPeriod;
			if (seconds > queryPeriod) {
				break; // spent long enough on one scanner; move to next
			}
		}// end while
	}
	return true;
}

bool CFTPHandler::DownloadOldPak(long interval)
{
	int  estimatedTime;
	bool downloadResult;
	time_t startTime, stopTime;
	CString fileFullName;
	CScannerFileInfo *pakFileInfo = NULL;

	if(m_fileInfoList.GetCount() <= 0)	//changed 2006-12-12. Omit get pak file list one more time
		return false;

	// Assume that the data-speed is at least 4kb/s
	if(m_dataSpeed <= 0)
		m_dataSpeed = 4.0;

	pakFileInfo		 = &m_fileInfoList.GetTail();
	m_remoteFileSize = pakFileInfo->fileSize;
	estimatedTime = (int)((m_remoteFileSize * 1024.0) / m_dataSpeed);
	time(&startTime);
	if(this->m_electronicsBox == BOX_VERSION_2){
		fileFullName.Format("%s.pak", (LPCSTR)pakFileInfo->fileName);
	}else{
		fileFullName.Format("%s.PAK", (LPCSTR)pakFileInfo->fileName);
	}
	downloadResult = DownloadSpectra(fileFullName,m_storageDirectory);
	time(&stopTime);

	if(downloadResult)
		m_fileInfoList.RemoveTail();
	
	return true;
}

//download file list from B disk
long CFTPHandler::GetPakFileList(CString& folder)
{
	long pakFileSum = 0;
	CString fileList,listFilePath, msg;
	CFTPSocket* ftpSocket = new CFTPSocket();
	char ipAddr[16];
	sprintf(ipAddr,"%s", (LPCSTR)m_ftpInfo.IPAddress);

	// Start with clearing out the list of files...
	m_fileInfoList.RemoveAll();

	// We save the data to a temporary file on disk....
	listFilePath.Format("%sfileList.txt", (LPCSTR)m_storageDirectory);
	ftpSocket->SetLogFileName(listFilePath);
	
	// Log in to the instrument's FTP-server
	if(!ftpSocket->Login(ipAddr, m_ftpInfo.userName, m_ftpInfo.password))
		return -1;

	// While we're at it, check the brand of the electronics box in the 
	//	login-response from the FTP-server
	if(ftpSocket->m_serverMsg.Find("AXIS") >= 0){
		m_electronicsBox = BOX_VERSION_2;
		g_settings.scanner[m_mainIndex].electronicsBox = BOX_VERSION_2;
	}

	Sleep(100);

	// Enter Rxxx folder
	if(folder.GetLength() == 4)
	{
		msg.Format("<node %d> Getting file-list from folder: %s", m_mainIndex, (LPCSTR)folder);
		ShowMessage(msg);

		if(!ftpSocket->EnterFolder(folder))
		{
			ftpSocket->Disconnect();
			delete ftpSocket;

			msg.Format("<node %d> Failed to enter folder: %s", m_mainIndex, (LPCSTR)folder);
			ShowMessage(msg);

			return -2;
		}

		// Download the list of files...
		if(ftpSocket->GetFileList())
		{
			FillFileList(listFilePath);
		}
	}
	else
	{
		msg.Format("<node %d> Getting file-list", m_mainIndex);
		ShowMessage(msg);

		// Download the list of files...
		if(ftpSocket->GetFileList())
		{
			FillFileList(listFilePath);
		}
	}
	
	// Close the connection
	ftpSocket->Disconnect();
	delete ftpSocket;

	// Count the number of files in the instrument
	pakFileSum = m_fileInfoList.GetCount();
	
	msg.Format("<node %d> %d files found on disk", m_mainIndex, pakFileSum);
	ShowMessage(msg);

	return pakFileSum;
}

bool CFTPHandler::GetDiskFileList(int disk)
{
	CString fileList,listFilePath;
	CFTPSocket* ftpSocket = new CFTPSocket();
	char ipAddr[16];
	sprintf(ipAddr,"%s", (LPCSTR)m_ftpInfo.IPAddress);
	listFilePath.Format("%sfileList.txt", (LPCSTR)m_storageDirectory);
	ftpSocket->SetLogFileName(listFilePath);


	if(disk == 1){
		if(!ftpSocket->Login(ipAddr, m_ftpInfo.userName, m_ftpInfo.password))
			return false;
	}else{
		if(!ftpSocket->Login(ipAddr, m_ftpInfo.adminUserName, m_ftpInfo.adminPassword))
			return false;
	}

	if(ftpSocket->GetFileList())
	{
		if(disk == 1)
			FillFileList(ftpSocket->m_listFileName, 'B');
		else
			FillFileList(ftpSocket->m_listFileName, 'A');
		ftpSocket->Disconnect();
		ShowMessage("File list was downloaded.");
		delete ftpSocket;
		if(m_fileInfoList.GetCount() > 0)
			return true;
		else
			return false;
	}
	else
	{
		ftpSocket->Disconnect();
		ShowMessage("File list was not downloaded. It may be caused by slow or broken Ethernet connection.");
		delete ftpSocket;
		EmptyFileInfo();
		return false;
	}
}

void CFTPHandler::EmptyFileInfo()
{
	m_fileInfoList.RemoveAll();
	m_rFolderList.RemoveAll();
}

int  CFTPHandler::FillFileList(CString& fileName, char disk)
{
	CString resToken,str,msg;
	int curPos = 0;
	int round = 0;
	int tokenLength = 0;
	CStdioFile file;
	CFileException fileException;
	if(!file.Open(fileName, CFile::modeRead | CFile::typeText, &fileException))
	{
		msg.Format("Can not open %s", (LPCSTR)fileName);
		ShowMessage(msg);
		return false;
	}
	EmptyFileInfo(); //empty m_fileInfoList to fill in new info
	while(file.ReadString(str))
	{
		if(str.GetLength()==0)
			break;

		resToken= str;
		tokenLength = resToken.GetLength();
	
		if(resToken.Find("Í")!=-1)
			break;
		
		if(tokenLength > 0)
		{
			ParseFileInfo(resToken, disk);
			round++;
		}
	}	// token length should be bigger than 56 bytes//!= 0);
	file.Close();
	return round;
}

void CFTPHandler::AddFolderInfo(CString& line)
{
	CString folderName;

	int folderNameLength = 4; // <-- all RXXX folders contains 4 characters!

	folderName.Format("%s", (LPCSTR)line.Right(folderNameLength));
	if(folderName.Left(1) != _T("R") && folderName.Left(1) != _T("r"))
		return; // The folder is not a RXXX - folder, do not insert it into the list!

	m_rFolderList.AddTail(folderName);	
}

//to fill file names and other information into m_fileList
void CFTPHandler::ParseFileInfo(CString line, char disk)
{
	CString resToken,subResToken, resultStr,fileName,fileSubfix, month,date,time,mmdd,dotSubfix;
	long fileSize;
	int curPos = 0;

	line.Remove('\n');
	line.Remove('\r');
//	if(line.Find("drw-------") != -1)
	if(line.Find("drw") != -1) // Changed 2008.06.30
	{
		AddFolderInfo(line);
		return;
	}
	for(int i= 0;i < 9;i++)
	{
		resToken = line.Tokenize(" ",curPos);
		if(curPos < 0)
			return;
		if(i== 4)
		{
			fileSize  = atoi(resToken);
		}
		else if(i== 5)
			month.Format("%s", (LPCSTR)resToken);
		else if(i== 6)
			date.Format("%s", (LPCSTR)resToken);
		else if(i== 7)
			time.Format("%s", (LPCSTR)resToken);
		else if(i== 8)
		{
			fileName.Format("%s", (LPCSTR)resToken);
			fileName.Remove('\n');
			fileName.Remove('\r');
			fileSubfix.Format("%s", (LPCSTR)fileName);
			GetSuffix(fileName,fileSubfix);
		}
	}
	mmdd.Format("%s %s", (LPCSTR)month, (LPCSTR)date);

	if(Equals(fileName, "..") || Equals(fileName, "."))
		return; // no use with these

	if(m_electronicsBox == BOX_VERSION_1 && fileSubfix == _T("PAK"))	
		m_fileInfoList.AddTail(CScannerFileInfo(disk, fileName,fileSubfix,fileSize,mmdd,time));
	if(m_electronicsBox == BOX_VERSION_2 && fileSubfix == _T("pak"))	
		m_fileInfoList.AddTail(CScannerFileInfo(disk, fileName,fileSubfix,fileSize,mmdd,time));

}
void CFTPHandler::GetSuffix(CString& fileName, CString& fileSubfix)
{
	int position  = fileName.ReverseFind('.'); 
	int length    = CString::StringLength(fileName);	
	if(length < 5 || position < 0)
		return;
	fileName   = fileName.Left(position);
	fileSubfix = fileSubfix.Right(length - position - 1);
}

bool CFTPHandler::DownloadSpectra(const CString &remoteFile, const CString &savetoPath)
{
	CString msg;
	m_localFileFullPath.Format("%s%s", (LPCSTR)savetoPath, (LPCSTR)remoteFile);

	//connect to the ftp server
	if(!DownloadFile(remoteFile,savetoPath))
	{
		m_statusMsg.Format("Can not download file from remote scanner (%s) by FTP", (LPCSTR)m_ftpInfo.IPAddress);
		ShowMessage(m_statusMsg);
		return false;
	}

	// Check that the size on disk is same as the size in the remote computer
	if(Common::RetrieveFileSize(m_localFileFullPath) != m_remoteFileSize)
		return false;

	// Check the contents of the file and make sure it's an ok file
	if(1 == m_pakFileHandler->ReadDownloadedFile(m_localFileFullPath))
	{
		msg.Format("CPakFileHandler found an error with the file %s. Will try to download again", (LPCSTR)m_localFileFullPath);
		ShowMessage(msg);

		// Download the file again
		if(!DownloadFile(remoteFile, savetoPath))
			return false;

		if(1 == m_pakFileHandler->ReadDownloadedFile(m_localFileFullPath))
		{
			ShowMessage("The pak file is corrupted");
			//DELETE remote file
			if(0 == DeleteRemoteFile(remoteFile)){
				msg.Format("<node %d> Remote File %s could not be removed", m_mainIndex, (LPCSTR)remoteFile);
				ShowMessage(msg);
			}
			return false;
		}
	}

	//DELETE remote file
	msg.Format("%s has been downloaded", (LPCSTR)remoteFile);
	ShowMessage(msg);
	if(0 == DeleteRemoteFile(remoteFile)){
		msg.Format("<node %d> Remote File %s could not be removed", m_mainIndex, (LPCSTR)remoteFile);
		ShowMessage(msg);
	}

	// Tell the world that we've done with one download
	pView->PostMessage(WM_FINISH_DOWNLOAD, (WPARAM)&m_spectrometerSerialID, (LPARAM)&m_dataSpeed);

	return true;
}

//delete remote file in ftp server ( scanner)
BOOL CFTPHandler::DeleteRemoteFile(const CString& remoteFile)
{
	BOOL result = FALSE;
	CString localCopyOfRemoteFileName;

	if(m_FtpConnection == NULL)
	{
		if(Connect(m_ftpInfo.IPAddress, m_ftpInfo.userName, m_ftpInfo.password)!=1)
		{
			pView->PostMessage(WM_SCANNER_NOT_CONNECT,(WPARAM)&(m_spectrometerSerialID),0);
			return FALSE;
		}
	}
	if(m_electronicsBox != BOX_VERSION_2){
		localCopyOfRemoteFileName.Format(remoteFile);
		if(!FindFile(localCopyOfRemoteFileName)) // This does not work with the axis-system, for some reason...
		{
			return FALSE;
		}
	}
	result = m_FtpConnection->Remove((LPCTSTR)remoteFile);

	return result;
}


//download file from ftp server
bool CFTPHandler::DownloadFile(const CString &remoteFileName, const CString &savetoPath)
{
	CString msg, fileFullName;

	// High resolution counter
	LARGE_INTEGER lpFrequency, timingStart, timingStop;
	BOOL useHighResolutionCounter = QueryPerformanceFrequency(&lpFrequency);

	// Low resolution counter, used as a backup
	clock_t timing_Stop, timing_Start; // <-- timing of the upload speed

	// The filename
	fileFullName.Format("%s%s", (LPCSTR)savetoPath, (LPCSTR)remoteFileName);

	//check local file,if a file with same name exists, delete it
	if(IsExistingFile(fileFullName)){
		DeleteFile(fileFullName);
	}
	
	msg.Format("Begin to download file from %s", (LPCSTR)m_ftpInfo.IPAddress);
	ShowMessage(msg);

	//show running lamp on interface
	pView->PostMessage(WM_SCANNER_RUN,(WPARAM)&(m_spectrometerSerialID),0);
	
	timing_Start = clock(); // <-- timing...
	useHighResolutionCounter = QueryPerformanceCounter(&timingStart);

	if(!DownloadAFile(remoteFileName, fileFullName))
	{
		return false;
	}

	// Timing...
	useHighResolutionCounter = QueryPerformanceCounter(&timingStop);
	timing_Stop = clock();

	// Remember the speed of the upload
	double clocksPerSec	= CLOCKS_PER_SEC;
	double elapsedTime	= max(1.0 / clocksPerSec, (double)(timing_Stop - timing_Start) / clocksPerSec);
	double elapsedTime2 = ((double)timingStop.LowPart - (double)timingStart.LowPart) / (double)lpFrequency.LowPart;

	if(useHighResolutionCounter)
		m_dataSpeed = m_remoteFileSize / (elapsedTime * 1024.0);
	else
		m_dataSpeed = m_remoteFileSize / (elapsedTime2 * 1024.0);

	m_statusMsg.Format("Finished downloading file %s from %s @ %.1lf kb/s", (LPCSTR)fileFullName, (LPCSTR)m_spectrometerSerialID, m_dataSpeed);
	ShowMessage(m_statusMsg);
	
	return true;
}

bool CFTPHandler::MakeCommandFile(char* cmdString)
{
	CString fileName;
	fileName.Format("%scommand.txt", (LPCSTR)m_storageDirectory);
	FILE *f = fopen(fileName,"w");
	if(f == NULL)
		return false; // could not open file for writing

	fprintf(f,cmdString);
	fclose(f);
	return true;
}

bool CFTPHandler::SendCommand(char* cmd)
{
	CString localFileFullPath,remoteFile;
	remoteFile = _T("command.txt");
	localFileFullPath.Format("%scommand.txt", (LPCSTR)m_storageDirectory);
	if(false == MakeCommandFile(cmd))
		return false;

	//connect to the ftp server
	if(!Connect(m_ftpInfo.IPAddress, m_ftpInfo.userName, m_ftpInfo.password)==1)
		return false;

	//upload file to the server
	if(UploadFile(localFileFullPath,remoteFile) == 0)
	{
		Disconnect();
		m_statusMsg.Format("Can not upload command file to the remote scanner (%s) by FTP", (LPCSTR)m_ftpInfo.IPAddress);
		ShowMessage(m_statusMsg);
		return false;
	}

	Disconnect();
	return true;
}

void CFTPHandler::GotoSleep()
{
	CString cmdFile = TEXT("command.txt");
	if(0 == DeleteRemoteFile(cmdFile)){
		//		ShowMessage("Remote File command.txt could not be removed");
	}
	SendCommand("pause\npoweroff");
	pView->PostMessage(WM_SCANNER_SLEEP,(WPARAM)&(m_spectrometerSerialID),0);
	//download old pak files during sleeping time
	DownloadOldPak(14400);
	//disconnect when finish downloading 2007-09-23
	Disconnect();
}

void CFTPHandler::WakeUp()
{
	bool success = false;
	int iterations = 0;

	CString cmdFile = TEXT("command.txt");
	if(0 == DeleteRemoteFile(cmdFile)){
		//	ShowMessage("Remote File command.txt could not be removed");
	}
	success = SendCommand("poweron\nresume");

	// If we failed to upload the command-file then try again, at most 5 times
	while(success == false && iterations < 5){
		Sleep(1000);
		success = SendCommand("poweron\nresume");
		++iterations;
	}

	Disconnect();

	if(success == false){
		ShowMessage("Failed to wake instrument up");
	}else{
		// Try to download the cfg.txt - file from the instrument. This is done 
		//	both to test the connection and to get information from the
		//	file (such as motor-steps-comp)
		this->DownloadCfgTxt();
	}
}

void CFTPHandler::Reboot()
{
	CString cmdFile = TEXT("command.txt");
	if(0 == DeleteRemoteFile(cmdFile)){
		//		ShowMessage("Remote File command.txt could not be removed");
	}
	SendCommand("reboot");
}

/** Download cfg.txt */
int CFTPHandler::DownloadCfgTxt(){
	CString remoteFileName, localFileName, copyFileName;
	
	// Connect to the administrators account
	if(Connect(m_ftpInfo.IPAddress, m_ftpInfo.adminUserName, m_ftpInfo.adminPassword) != 1){
		return 0; // failed to connect
	}

	// The names of the local and remote files
	remoteFileName.Format("cfg.txt");
	localFileName.Format("%scfg.txt", (LPCSTR)m_storageDirectory);
	
	// Download the file.
	if(!DownloadAFile(remoteFileName, localFileName)){
		Disconnect();
		return 0;
	}
	
	// Tell the user about what we've done!
	m_statusMsg.Format("Downloaded cfg.txt from %s", (LPCSTR)m_spectrometerSerialID);
	ShowMessage(m_statusMsg);
	
	// Remember to Disconnect from the server
	Disconnect();
	
	// Try to parse the cfg.txt - file in order to get the paramters...
	FileHandler::CCfgTxtFileHandler cfgTxtReader;
	if(0 == cfgTxtReader.ReadCfgTxt(localFileName)){
		return 0;
	}
	//if(cfgTxtReader.m_motorStepsComp[0] != 0){ // If we've found a motor steps comp...
	//	g_settings.scanner[m_mainIndex].motor[0].motorStepsComp = cfgTxtReader.m_motorStepsComp[0];
	//	g_settings.scanner[m_mainIndex].motor[0].stepsPerRound = cfgTxtReader.m_motorStepsPerRound[0];
	//	if(cfgTxtReader.m_nMotors == 2){
	//		g_settings.scanner[m_mainIndex].motor[1].motorStepsComp = cfgTxtReader.m_motorStepsComp[1];
	//		g_settings.scanner[m_mainIndex].motor[1].stepsPerRound = cfgTxtReader.m_motorStepsPerRound[1];
	//	}
	//}

	// if todays date is dividable by 7 then try to upload the file
	int todaysDate = Common::GetDay();
	if(todaysDate % 7 == 0){
		// Copy the file to a new name.
		copyFileName.Format("%scfg_%s.txt", (LPCSTR)m_storageDirectory, (LPCSTR)m_spectrometerSerialID);
		if(0 != CopyFile(localFileName, copyFileName, FALSE)){
			// Get the index of this volcano
			int volcanoIndex = Common::GetMonitoredVolcano(m_spectrometerSerialID);
			
			if(-1 != volcanoIndex){
				UploadToNOVACServer(copyFileName, volcanoIndex, true);
			}
		}
	}
	
	return 1;
}
