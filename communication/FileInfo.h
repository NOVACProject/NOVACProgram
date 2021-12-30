#pragma once

class CFileInfo
{
public:
    CFileInfo();
    CFileInfo(CString fileName, long fileSize);

    CString	m_fileName;
    long    m_fileSize;
};
