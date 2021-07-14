#pragma once

#include "WindField.h"
#include "WindFieldDatabase.h"
#include "../Common/Common.h"
#include <memory>
#include <mutex>

#ifndef METEROLOGY_H
#define METEROLOGY_H

/** <b>CMeteorologicalData</b> is the class which holds all the meterological data
    which is needed in the program. */
class CMeteorologicalData
{
public:
    CMeteorologicalData() = default;
    ~CMeteorologicalData() = default;

    /** Set the wind field for a scanner */
    int SetWindField(const CString& serialNumber, const CWindField& windField);

    /** Sets up the list of volcanoes monitored, this is necessary for the wind-field reading
        to know to which locations the wind data should be monitored */
    void SetVolcanoes(const std::vector<novac::CNamedLocation>& volcanoes);

    /** Tries to read in a wind-field from a file. If this is successful
        then all wind-data returned will be first searched for in the wind-field
        file and secondly from the user given or default values.
        @return 0 on success */
    int ReadWindFieldFromFile(const CString& fileName);

    /** Get the wind field for a scanner.
        The wind-field will first of all be taken from a read-in wind-field file
            if no valid data can be found in the wind-field file then the
            user provided or default wind-field will be returned. */
    int GetWindField(const CString& serialNumber, const novac::CDateTime& dt, CWindField& windField);

    /** The default wind field for the calculations */
    CWindField defaultWindField;

private:

    // This class is not copyable because of the owned pointer.
    //  better to make this explicit here than to discover it later in a crash.
    CMeteorologicalData(const CMeteorologicalData& ) = delete;
    CMeteorologicalData& operator=(const CMeteorologicalData& ) = delete;

    /** The wind-field database read from file.
        If any wind-field has been read in from a file, 
        then the data can be found here. */
    std::unique_ptr<CWindFieldDatabase> m_wfDatabaseFromFile = nullptr;

    /** This is to protect the m_wfDatabaseFromFile from being accessed from two
        threads simultaneously */
    std::mutex m_wfDatabaseMutex;

    /** The windfield at each of the scanningInstruments
        the windfield at scanning Instrument 'm_scanner[i]' is given
        by 'm_windFieldAtScanner[i]' */
    CWindField m_windFieldAtScanner[MAX_NUMBER_OF_SCANNING_INSTRUMENTS];

    /** The scanningInstruments for which we know the windfield.
        the windfield at scanning Instrument 'm_scanner[i]' is given
        by 'm_windFieldAtScanner[i]' */
    CString m_scanner[MAX_NUMBER_OF_SCANNING_INSTRUMENTS];

    /** This is the list of monitored volcanoes, including their positions.
        Used by the net-cdf reading routine to figure out where to extract the data. */
    std::vector<novac::CNamedLocation> m_volcanoes;

    /** How many scanners that we have defined the wind field for */
    long m_scannerNum = 0;

    bool ReadWindFieldFromTextFile(const CString& fileName);
};

#endif