#pragma once

#include "WindField.h"
#include "../File/WindFileReader.h"
#include "../Common/Common.h"

#ifndef METEROLOGY_H
#define METEROLOGY_H

/** <b>CMeteorologicalData</b> is the class which holds all the meterological data
    which is needed in the program. */
class CMeteorologicalData
{
public:
    CMeteorologicalData();
    ~CMeteorologicalData();

    /** Set the wind field for a scanner */
    int SetWindField(const CString &serialNumber, const CWindField &windField);

    /** Tries to read in a wind-field from a file. If this is successfull
                then all wind-data returned will be first searched for in the wind-field
                file and secondly from the user given or default values.
                @return 0 on success */
    int ReadWindFieldFromFile(const CString &fileName);

    /** Get the wind field for a scanner.
            The wind-field will first of all be taken from a read-in wind-field file
                if no valid data can be found in the wind-field file then the
                user provided or default wind-field will be returned. */
    int GetWindField(const CString &serialNumber, const CDateTime &dt, CWindField &windField);

    /** The default wind field for the calculations */
    CWindField defaultWindField;

private:

    // This class is not copyable because of the owned pointer.
    //  better to make this explicit here than to discover it later in a crash.
    CMeteorologicalData(const CMeteorologicalData& ) = delete;
    CMeteorologicalData& operator=(const CMeteorologicalData& ) = delete;

    /** The wind-field file reader. If any wind-field has been read in from a
            file, then the data can be found here. */
    FileHandler::CWindFileReader* m_wfReader;

    /** The windfield at each of the scanningInstruments
        the windfield at scanning Instrument 'm_scanner[i]' is given
        by 'm_windFieldAtScanner[i]' */
    CWindField m_windFieldAtScanner[MAX_NUMBER_OF_SCANNING_INSTRUMENTS];

    /** The scanningInstruments for which we know the windfield.
        the windfield at scanning Instrument 'm_scanner[i]' is given
        by 'm_windFieldAtScanner[i]' */
    CString m_scanner[MAX_NUMBER_OF_SCANNING_INSTRUMENTS];

    /** How many scanners that we have defined the wind field for */
    long m_scannerNum;
};

#endif