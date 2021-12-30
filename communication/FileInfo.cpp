#include "StdAfx.h"
#include "FileInfo.h"

CFileInfo::CFileInfo(void)
    : m_fileName(""), m_fileSize(0)
{
}

CFileInfo::CFileInfo(CString fileName, long fileSize)
{
    m_fileName = fileName;
    m_fileSize = fileSize;
}