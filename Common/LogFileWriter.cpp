#include "stdafx.h"
#include "LogFileWriter.h"
#include "Version.h"
#include "Common.h"

namespace FileHandler
{

CLogFileWriter::CLogFileWriter(void)
{
}

int CLogFileWriter::SetErrorLogFile(const CString& path, const CString& fileName)
{
    CString str;
    int ret = CreateDirectoryStructure(path);
    if (ret)
    {
        return 1; // error
    }
    m_errorFile.Format("%s\\%s", (LPCSTR)path, (LPCSTR)fileName);

    // try to open the file and make sure that the file can be written to
    GetHeaderString(str, ERROR_LOG);
    return WriteToFile(str, ERROR_LOG, false);
}

int CLogFileWriter::WriteErrorMessage(const CString& text) const
{
    if (strlen(m_errorFile) == 0)
    {
        return 1;
    }

    return WriteToFile(text, ERROR_LOG);
}

int CLogFileWriter::WriteToFile(const CString& str, const int file, bool timeStamp) const
{
    static CString newLine = TEXT("\n");

    FILE* f = NULL;
    switch (file)
    {
    case ERROR_LOG:
        f = fopen(m_errorFile, "a+");
        break;
    case RESULT_LOG1:
        f = fopen(m_resultFile[0], "a+");
        break;
    case RESULT_LOG2:
        f = fopen(m_resultFile[1], "a+");
        break;
    default: return 1;
    }
    if (f == NULL)
    {
        return 1;
    }

    if (timeStamp)
    {
        CString time;
        Common::GetTimeText(time);
        fprintf(f, time);
        fprintf(f, TEXT("\t"));
    }
    int ret = fprintf(f, str);
    fprintf(f, "\n"); // add a new-line character

    fclose(f);

    return (ret >= 0);
}

/** Common handler to create the first, comment, line in the newly created file.
    @param str - str will when the function returns contain the header string to be written to file. */
int CLogFileWriter::GetHeaderString(CString& str, const int file) const
{
    CString time, date;
    Common::GetDateText(date);
    Common::GetTimeText(time);
    switch (file)
    {
    case ERROR_LOG: str.Format("#Error Log "); break;
    case RESULT_LOG1: str.Format("#Result Log "); break;
    case RESULT_LOG2: str.Format("#Result Log "); break;
    default: return 1;
    }

    str.AppendFormat("file for the Novac Master Program version %d.%d. Created on: %s at %s",
        CVersion::majorNumber, CVersion::minorNumber, (LPCSTR)date, (LPCSTR)time);

    return 0;
}
}
