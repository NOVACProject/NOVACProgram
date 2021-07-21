#include "StdAfx.h"
#include "WindFieldDatabase.h"

using namespace novac;

void CWindFieldDatabase::InsertWindField(const CWindField& wind)
{
    m_windField.push_back(wind);
}

long CWindFieldDatabase::GetRecordNum() const
{
    return (long)m_windField.size();
}

void CWindFieldDatabase::Clear()
{
    m_windField.clear();
    m_containsWindDirection = false;
    m_containsWindSpeed = false;
    m_containsPlumeHeight = false;
}

RETURN_CODE CWindFieldDatabase::InterpolateWindField(const CDateTime& desiredTime, CWindField &desiredWindField) const
{
    // First check if there's any records at all in the database
    if (m_windField.size() == 0)
    {
        return FAIL;
    }

    // Loop through all wind-fields in the database and see if we can find an exact match
    //  otherwise extract the wind-fields which are the closest records before and after
    //  the desired one.
    CWindField closestBefore, closestAfter;
    bool foundClosestBefore = false; // <-- if no wind-field was found before the desired one then return false
    bool foundClosestAfter = false; // <-- if no wind-field was found after the desired one then return false
    for (size_t i = 0; i < m_windField.size(); ++i)
    {
        const CWindField &currentRecord = m_windField[i];

        // First check if we've found an exact match, if so
        //  then return the current wind-field
        if (currentRecord.GetTimeAndDate() == desiredTime)
        {
            desiredWindField = currentRecord;
            return SUCCESS; // we're done!
        }

        // Then check if this is earlier than the desired one
        if (currentRecord.GetTimeAndDate() < desiredTime)
        {
            // currentRecord is valid before the desired time
            if (foundClosestBefore)
            {
                // We have already found another record which is valid before the desired time
                //  compare if the currentRecord is collected after the previously found
                //  record, if it is then use the currentRecord as the closest one before
                if (closestBefore.GetTimeAndDate() < currentRecord.GetTimeAndDate())
                {
                    closestBefore = currentRecord;
                }
                continue;
            }
            else
            {
                // We have found the first record in the database which is collected
                //  before the desired time, remember this and then continue the search
                foundClosestBefore = true;
                closestBefore = currentRecord;
                continue;
            }
        }
        else
        {
            // currentRecord is valid after the desired time
            if (foundClosestAfter)
            {
                // We have already found another record which is valid after the desired time
                //  compare if the currentRecord is collected before the previously found
                //  record, if it is then use the currentRecord as the closest one after
                if (currentRecord.GetTimeAndDate() < closestAfter.GetTimeAndDate())
                {
                    closestAfter = currentRecord;
                }
                continue;
            }
            else
            {
                // We have found the first record in the database which is collected
                //  before the desired time, remember this and then continue the search
                foundClosestAfter = true;
                closestAfter = currentRecord;
                continue;
            }
        }
    } // end for...

    // If the desired time is not in between any two wind-fields in the database,
    //  then we can not interpolate. If the time difference between the desried time
    //  and the closest record in the database is less than 3 hours then return the
    //  closest record, otherwise return FAIL.
    if (foundClosestAfter == false)
    {
        if (foundClosestBefore == false)
        {
            return FAIL; // should actually not happen
        }

        double timeDifference = fabs(CDateTime::Difference(closestBefore.GetTimeAndDate(), desiredTime));
        if (timeDifference > 3 * 3600)
        {
            return FAIL;
        }
        else
        {
            desiredWindField = closestBefore;
            return SUCCESS; // we're done!
        }
    }

    if (foundClosestBefore == false)
    {
        if (foundClosestAfter == false)
        {
            return FAIL; // should actually not happen
        }

        double timeDifference = fabs(CDateTime::Difference(closestAfter.GetTimeAndDate(), desiredTime));
        if (timeDifference > 3 * 3600)
        {
            return FAIL;
        }
        else
        {
            desiredWindField = closestAfter;
            return SUCCESS; // we're done!
        }
    }

    // If we are to interpolate the wind-field between two data points which are 
    //  separated by more than 24 hours, the interpolation will not be meaningful
    //  return FALSE
    double totalTimeDifference = CDateTime::Difference(closestAfter.GetTimeAndDate(), closestBefore.GetTimeAndDate());
    if (totalTimeDifference > 24.0*3600.0)
    {
        return FAIL;
    }

    // Interpolate the wind-field between the data-point closest before and
    //  the data-point closest after the desired time and return success!
    double timeDifference1 = CDateTime::Difference(desiredTime, closestBefore.GetTimeAndDate());
    double alpha = timeDifference1 / totalTimeDifference;

    // interpolate the wind speed and direction vectorially
    double x_comp = (1.0 - alpha) * closestBefore.GetWindSpeed() * cos(DEGREETORAD * closestBefore.GetWindDirection())
        + alpha * closestAfter.GetWindSpeed()  * cos(DEGREETORAD * closestAfter.GetWindDirection());
    double y_comp = (1.0 - alpha) * closestBefore.GetWindSpeed() * sin(DEGREETORAD * closestBefore.GetWindDirection())
        + alpha * closestAfter.GetWindSpeed()  * sin(DEGREETORAD * closestAfter.GetWindDirection());
    double wd = RADTODEGREE * atan2(y_comp, x_comp);
    double ws = sqrt(x_comp * x_comp + y_comp * y_comp);
    double ph = (1.0 - alpha) * closestBefore.GetPlumeHeight() + alpha * closestAfter.GetPlumeHeight();

    // Clamp the wind-direction to the interval 0->360 degrees
    //  instead of -180 -> +180 as the atan2 returns
    while (wd < 0)
    {
        wd += 360.0;
    }
    while (wd > 360)
    {
        wd -= 360.0;
    }

    // Set the time
    desiredWindField.SetTimeAndDate(desiredTime);

    // Get the sources of the wind-information
    MET_SOURCE wsSource = closestBefore.GetWindSpeedSource();
    MET_SOURCE wdSource = closestBefore.GetWindDirectionSource();
    MET_SOURCE phSource = closestBefore.GetPlumeHeightSource();


    // Set the wind-speed
    desiredWindField.SetWindSpeed(ws, wsSource);

    // Set the wind-direction
    desiredWindField.SetWindDirection(wd, wdSource);

    // Set the plume height
    desiredWindField.SetPlumeHeight(ph, phSource);

    return SUCCESS;
}
