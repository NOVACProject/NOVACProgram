#pragma once

#include "../Meteorology/WindFieldDatabase.h"
#include <SpectralEvaluation/GPSData.h>

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
        of the provided database. 
        As a net-cdf file often contains multiple locations, the wind field will be
        retrieved a the provided position (through linear interpolation). */
    RETURN_CODE ReadWindFile(const CGPSData& position, CWindFieldDatabase& result);

};

}