#pragma once

class CFileInfo
{
public:
	CFileInfo(void);
	~CFileInfo(void);
	CFileInfo(CString fileName, long fileSize);
	
	CString	m_fileName;
	long	m_fileSize;
};
