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
    char* NextToken();

    /** General parsing of a single, simple string item */
    int Parse_StringItem(const CString& label, CString& str);
    int Parse_StringItem(const CString& label, std::string& str);

    // Parsing of a path to a directory.
    // The output directory will contain a trailing backslash.
    int Parse_Directory(const CString& label, CString& str);

    /** General parsing of a single, simple float item */
    int Parse_FloatItem(const CString& label, double& number);

    /** General parsing of a single, simple integer item */
    int Parse_IntItem(const CString& label, int& number);

    /** General parsing of a single, simple long integer item */
    int Parse_LongItem(const CString& label, long& number);

    /** General parsing of a single, simple long integer item */
    int Parse_IPNumber(const CString& label, BYTE& ip0, BYTE& ip1, BYTE& ip2, BYTE& ip3);

    // Parses an attribute from an already read in token string.
    // This will attempt to read the attribute from the token string, but not read anything more from file, nor change any members.
    static std::string ParseAttribute(std::string token, std::string attributeName);

    // Returns true if the given token is an empty line, or the start of a comment line.
    static bool IsEmptyLineOrStartOfComment(const char* token);

    /** The tokenizer */
    char* szToken;

    void Close();

    /** Set the opened file pointer */
    void SetFile(CStdioFile* file);

private:

    /** A handle to the file to read from. */
    CStdioFile* m_File;

    /** The number of lines that has been read from the file */
    long nLinesRead;

    /** The string that was read from the file */
    char szLine[4096];

};
}