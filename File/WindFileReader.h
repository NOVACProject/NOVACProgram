#pragma once

#include "../Meteorology/WindFieldDatabase.h"

// Include synchronization classes
#include <afxmt.h>

namespace FileHandler
{

/** This class reads a wind field from a text file with 
    white space separated columns. */
class CWindFileReader
{
public:
    CWindFileReader() = default;

    /** The name and path of the wind-information file */
    CString m_windFile;

    // ------------------- PUBLIC METHODS -------------------------

    /** Reads the wind file and will on successful return fill in the contents
        of the provided database. */
    RETURN_CODE ReadWindFile(CWindFieldDatabase& result);

private:

    // this is to keep track of which column in the file corresponds to which value.
    typedef struct LogColumns
    {
        int altitude;
        int windSpeed;
        int windSpeedError;
        int windDirection;
        int windDirectionError;
        int plumeHeight;
        int plumeHeightError;
        int date;
        int time;
    }LogColumns;

    // ------------------- PRIVATE METHODS -------------------------

    /** Reads the header line for the file and retrieves which
      column represents which value. */
    void ParseFileHeader(const char szLine[8192], CWindFieldDatabase& database);

    /** Resets the information about which column data is stored in */
    void ResetColumns();

    /** Parses the section containing the source of the wind-field
        The string should be converted to lower-case characters before
        calling this function */
    void ParseSourceString(const char szLine[8192], MET_SOURCE &source);

    // ------------------- PRIVATE DATA -------------------------

    /** Keeping track of which column contains what */
    LogColumns m_col;

    /** This class contains critical sections of code */
    CCriticalSection m_critSect;

};

}
