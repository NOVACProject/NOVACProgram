#include "StdAfx.h"
#include "fileinfo.h"

CFileInfo::CFileInfo(void)
{
}

CFileInfo::~CFileInfo(void)
{
}
CFileInfo::CFileInfo(CString fileName, long fileSize)
{
	m_fileName = fileName;
	m_fileSize = fileSize;
}