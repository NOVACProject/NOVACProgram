#pragma once

#include "../Meteorology/WindFieldDatabase.h"

namespace FileHandler
{

/** This class reads a wind field from a net cdf data file */
class CNetCdfWindFileReader
{
public:

    /** The name and path of the wind-information file */
    CString m_windFile = "";

    // ------------------- PUBLIC METHODS -------------------------

    /** Reads the wind file and will on successful return fill in the contents
    of the provided database. */
    RETURN_CODE ReadWindFile(CWindFieldDatabase& result);

};

}