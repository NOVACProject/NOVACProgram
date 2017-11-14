#include "StdAfx.h"
#include "scannerfolderinfo.h"

CScannerFolderInfo::CScannerFolderInfo(void)
{
}

CScannerFolderInfo::~CScannerFolderInfo(void)
{
	m_fileList.RemoveAll();
}

CScannerFolderInfo::CScannerFolderInfo(char pDiskName,	CString pFolderName, CString pDate, CString pTime)
{
		diskName		= pDiskName;
		folderName	= pFolderName;
		date				= pDate;
		time				= pTime;

		m_fileList.RemoveAll();
}

