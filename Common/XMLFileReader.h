#pragma once

#include <string>

namespace FileHandler
{
    class CXMLFileReader
    {
    public:
        CXMLFileReader();
        virtual ~CXMLFileReader();

    protected:

        /**retrieve the next token from the xml file*/
        char *NextToken();

        /** General parsing of a single, simple string item */
        int Parse_StringItem(const CString &label, CString &str);
        int Parse_StringItem(const CString &label, std::string &str);

        /** General parsing of a single, simple float item */
        int Parse_FloatItem(const CString &label, double &number);

        /** General parsing of a single, simple integer item */
        int Parse_IntItem(const CString &label, int &number);

        /** General parsing of a single, simple long integer item */
        int Parse_LongItem(const CString &label, long &number);

        /** General parsing of a single, simple long integer item */
        int Parse_IPNumber(const CString &label, BYTE &ip0, BYTE &ip1, BYTE &ip2, BYTE &ip3);

        /** Set the opened file pointer*/
        void SetFile(CStdioFile* file);

        /** The tokenizer */
        char *szToken;

    private:

        /** A handle to the file to read from. */
        CStdioFile *m_File;

        /** The number of lines that has been read from the file */
        long nLinesRead;

        /** The string that was read from the file */
        char szLine[4096];

    };
}