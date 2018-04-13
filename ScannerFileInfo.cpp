#include "StdAfx.h"
#include "scannerfileinfo.h"

CScannerFileInfo::CScannerFileInfo(void)
{
}

CScannerFileInfo::~CScannerFileInfo(void)
{
}
CScannerFileInfo::CScannerFileInfo(char pDiskName,CString pFileName,CString pFileSubfix, long pFileSize, CString pDate,CString pTime)
{
		diskName = pDiskName;
		fileName = pFileName;
		fileSubfix = pFileSubfix;
		fileSize = pFileSize;
		date = pDate;
		time = pTime;
		info.Format("%d bytes, created %s %s", fileSize, (LPCTSTR)date, (LPCTSTR)time);
}

CScannerFileInfo::CScannerFileInfo(const CScannerFileInfo &info2){
	this->date.Format(info2.date);
	this->diskName = info2.diskName;
	this->fileName.Format(info2.fileName);
	this->fileSize = info2.fileSize;
	this->fileSubfix.Format(info2.fileSubfix);
	this->info.Format(info2.info);
	this->time.Format(info2.time);
}
