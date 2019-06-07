#pragma once

/** The struct CScannerFileInfo stores information regarding one file in a remote scanner device */
struct CScannerFileInfo
{
public:
    CScannerFileInfo();

    CScannerFileInfo(char diskName, CString pFileName, CString pFileSubfix, long pFileSize, CString pDate, CString pTime);

    CScannerFileInfo(const CScannerFileInfo &info2);
    CScannerFileInfo& operator=(const CScannerFileInfo& info2);

    char diskName;

    CString	fileName;

    CString fileSubfix;

    long fileSize;

    CString date;

    CString time;
};
