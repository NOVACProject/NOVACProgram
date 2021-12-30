#include <afxtempl.h>

#pragma once

class FileMerger
{
public:
    FileMerger(void);
    ~FileMerger(void);

    /** Specifies the full path and filename
        of the file to output the data to */
    void SetOutputFile(CString& outputFile);

    /** Specifies the full path and filename of the
        files to merge */
    void SetFilesToMerge(CList <CString, CString&>& filesToMerge);

    /** Retrieves the last error message. This may be NULL!
        the returned pointer must not be deleted!
    */
    const CString* GetErrorMessage();

    /** This merges the specified files
        Possible return values are:
        0 - success
        1 - no input file specified
        2 - invalid output file
        3 - cannot open file for writing
        4 - one or more files could not be opened for reading,
            see 'GetErrorMessage' for complete error.
    */
    int MergeFiles();
protected:

    /** This is the full path and filename
        of the output file */
    CString	m_outputFile;

    /** This is a list containing the full
        path and filename of the files to merge*/
    CList <CString, CString&> m_filesToMerge;


    CString* m_errorMsg;
};
