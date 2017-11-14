#pragma once

class CScannerFileInfo
{
public:
	CScannerFileInfo(void);
	~CScannerFileInfo(void);
	CScannerFileInfo(char pDiskName,CString pFileName,CString pFileSubfix, long pFileSize, CString pDate,CString pTime);
	CScannerFileInfo(const CScannerFileInfo &info2);

	CString info;

	char		diskName;

	CString	fileName;

	CString fileSubfix;

	long		fileSize;

	CString date;

	CString time;
};
