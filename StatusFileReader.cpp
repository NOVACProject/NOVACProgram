#include "stdafx.h"
#include "StatusFileReader.h"
#include "Common/Common.h"

using namespace FileHandler;

CStatusFileReader::CStatusFileReader(void)
{
    Common commonObj;
    commonObj.GetExePath();
    m_workingPath = commonObj.m_exePath;
    memset(m_statusList, 0, sizeof(char) * 1024);
}

CStatusFileReader::~CStatusFileReader(void)
{
}

void CStatusFileReader::SetWorkingPath(CString path)
{
    m_workingPath = path;
}

int CStatusFileReader::IsPoweroff()
{
    int status = 0;
    int startIndex, exitIndex;
    exitIndex = 0;
    startIndex = 0;
    Reorder();
    exitIndex = CheckStatus(POWEROFF);
    startIndex = CheckStatus(POWERON);
    if (startIndex < exitIndex)
    {
        status = 1;
    }
    return status;
}

int CStatusFileReader::CheckStatus(unsigned char status)
{
    int i;
    for (i = 1023; i >= 4; i--)
    {
        if (m_statusList[i] == status)
        {
            return i;
        }
    }
    return 0;
}

void CStatusFileReader::Reorder()
{
    int i;
    unsigned short last = 0;
    unsigned short first = 0;
    char txt[1024];
    memset(txt, 0, sizeof(char) * 1024);
    unsigned short* p = (unsigned short*)txt;
    CString fileFullName;
    fileFullName.Format("%sstatus.dat", (LPCTSTR)m_workingPath);
    FILE* f = fopen(fileFullName, "rb");
    if (f > (FILE*)0)
    {
        fread(p, 1024, 1, f);
        fclose(f);
        first = p[0];
        last = p[1]; //last is the latest status
    }
    printf("first = %d,last = %d\n", first, last);
    int count = 4;
    if (first > last)
    {
        for (i = first; i < 1024; i++)
        {
            ASSERT(count < 1024);
            m_statusList[count] = txt[i];
            count++;
        }
        for (i = 4; i <= last; i++)
        {
            ASSERT(count < 1024);
            m_statusList[count] = txt[i];
            count++;
        }
    }
    else
    {
        for (i = first; i <= last; i++)
        {
            ASSERT(count < 1024);
            m_statusList[count] = txt[i];
            count++;
        }
    }
}
