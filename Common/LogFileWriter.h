#pragma once

namespace FileHandler
{
    /** <b>FileHandler::CLogFileWriter</b> is a generic class to handle simple output from a program.
        It can manage three kinds of log-files. One error-logfile, and
        up to two result-logfile. The filename of all of these can be changed dynamically. */
    class CLogFileWriter
    {
    public:
        /** Default constructor */
        CLogFileWriter(void);

        /** Sets the path and filename of the error log file.
            @param path - the path of the new error-log file. If this path
                does not exist it will be created.
            @param fileName - the filename of the new error log file.
            @return 0 if all is ok, else 1.*/
        int SetErrorLogFile(const CString& path, const CString& fileName);

        /** Sets the path and filename of the result log file.
            @param path - the path of the new result-log file. If this path
                does not exist it will be created.
            @param fileName - the filename of the new result log file.
            @paran resultLogFileNum - which result log file will be set.
            @return 0 if all is ok, else 1.*/
        int SetResultLogFile(const CString& path, const CString& fileName, unsigned int resultLogFileNum = 0);

        /** Returns the name of the error log file */
        const CString& GetErrorLogFile() const;

        /** Returns the name of the desired result log file */
        const CString& GetResultLogFile(unsigned int resultLogFileNum = 0) const;

        /** Appends a string to the error log file.
            @param text - the string that should be written to the log file.
            @return 0 if all is ok. @return 1 if any error occurs. */
        int WriteErrorMessage(const CString& text) const;

        /** Appends a string to the result log file.
            @param text - the string that should be written to the log file.
            @param resultLogFileNum - what resultfile should be updated (zero based index)
            @return 0 if all is ok. @return 1 if any error occurs. */
        int WriteResultMessage(const CString& text, unsigned int resultLogFileNum = 0) const;

    private:
        /** The file name of the result log file(s) */
        CString m_resultFile[2];

        /** The file name of the error log file */
        CString m_errorFile;

        /** Common handler to write to a specific file */
        int WriteToFile(const CString& str, const int file, bool timeStamp = true) const;

        /** Common handler to create the first, comment, line in the newly created file.
            @param str - str will when the function returns contain the header string to be written to file. */
        int GetHeaderString(CString& str, const int file) const;

        /** flag for the error log */
        static const int ERROR_LOG = 0;

        /** flag for the first result log */
        static const int RESULT_LOG1 = 2;

        /** flag for the second result log */
        static const int RESULT_LOG2 = 3;
    };
}