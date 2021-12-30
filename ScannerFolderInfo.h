#pragma once

#include "ScannerFileInfo.h"
#include "afxtempl.h"

class CScannerFolderInfo
{
public:
    CScannerFolderInfo(void);
    ~CScannerFolderInfo(void);

    CScannerFolderInfo(char pDiskName, CString pFolderName, CString pDate, CString pTime);

    char    diskName;
    CString	folderName;
    CString date;
    CString time;

    /** The  list of files inside this folder */
    CList <CScannerFileInfo*, CScannerFileInfo*> m_fileList;
};
