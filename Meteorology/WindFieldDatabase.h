#pragma once

#ifndef WINDFIELDRECORD_H
#define WINDFIELDRECORD_H

#include <vector>

#include "../Common/Common.h"
#include "WindField.h"

/** The CWindFieldDatabase class contains a series of known CWindField records
    for a specific location (typically the latitude and longitude of the volcano). 
    Each record is valid for a given period of time. */
class CWindFieldDatabase
{
public:
    CWindFieldDatabase() = default;

    // ------------------- PUBLIC DATA -------------------------

    // The m_contains... flags are used to indicate the type of data contained in this database
    //  These are to be filled in whenever this database is populated.
    bool m_containsWindDirection = false; // True if the last wind-field file read contains a wind-direction
    bool m_containsWindSpeed = false;     // True if the last wind-field file read contains a wind-speed
    bool m_containsPlumeHeight = false;   // True if the last wind-field file read contains a plume-height

    // ------------------- PUBLIC METHODS -------------------------

    /** Inserts a given wind-field into the record */
    void InsertWindField(const CWindField& wind);

    /** Searches through the read-in data and looks for the wind-field at the
        given time.
        If the given time lies between to times in the 'database',
            the interpolated wind-field is returned if the difference in time between the
            two data-points is not exceeding 24hours.
        If the given time exactly matches the time of one wind-field in the 'database'
            the wind-field at that time will be returned
        If the given time is before or after all times in the 'database' the function
            will return the closest known wind-value if the time difference is not larger
            than 3 hours, otherwise it will return FAIL.
        @param desiredTime - the time and date at which the wind-field is to be extracted
        @param desiredWindField - will on successful return be filled with the parameters of the wind-field at the given time.
        @return SUCCESS - if the wind could be interpolated
        @return FAIL - if the wind lies outside of the time-range of the 'database',
                or the distance between the two data-points to interpolate is larger than 24 hours. */
    RETURN_CODE InterpolateWindField(const novac::CDateTime& desiredTime, CWindField &desiredWindField) const;

    /** Returns the number of points in the database */
    long GetRecordNum() const;

    /** Clears all the records of this database */
    void Clear();

private:

    // ------------------- PRIVATE DATA -------------------------

    /** Information about the wind */
    std::vector<CWindField> m_windField;
};

#endif