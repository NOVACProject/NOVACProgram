#pragma once
#include <SpectralEvaluation/DateTime.h>
#include "MeteorologySource.h"

#ifndef WINDFIELD_H
#define WINDFIELD_H

/** <b>CWindField</b> is a simle data structure which contains
    the wind speed, wind direction and the plume height at a
    specified point in time and a specific location */
class CWindField
{
public:
    CWindField() = default;

    /** Copying */
    CWindField& operator=(const CWindField& other);
    CWindField(const CWindField& other);

    /** Sets the wind-speed */
    void SetWindSpeed(double ws, MET_SOURCE source, double wsError = 0.0);

    /** Sets the wind-direction */
    void SetWindDirection(double wd, MET_SOURCE source, double wdError = 0.0);

    /** Sets the plume height */
    void SetPlumeHeight(double ph, MET_SOURCE source, double phError = 0.0);

    /** Sets the time and/or date the wind-field is valid for */
    void SetTimeAndDate(const CDateTime& dt);

    /** Sets the time the wind-field is valid for, the date is not touched */
    void SetTime(int hour, int minute, int second);

    /** Sets the date the wind-field is valid for, the time of day is not touched */
    void SetDate(int year, int month, int day);

    /** Gets the wind-speed */
    double GetWindSpeed() const;

    /** Gets the estimate for the total error in the wind-field */
    double GetWindError() const;

    /** Sets the estimate for the total error in the wind-field */
    void SetWindError(double err);

    /** Gets the source of the wind-speed */
    MET_SOURCE GetWindSpeedSource() const;

    /** Gets the source of the wind-speed */
    void GetWindSpeedSource(CString &str) const;

    /** Gets the wind-direction */
    double GetWindDirection() const;

    /** Gets the source of the wind-direction */
    MET_SOURCE GetWindDirectionSource() const;

    /** Gets the source of the wind-direction */
    void GetWindDirectionSource(CString &str) const;

    /** Gets the plume-height */
    double GetPlumeHeight() const;

    /** Gets the source of the plume-height */
    MET_SOURCE GetPlumeHeightSource() const;

    /** Gets the source of the plume-height */
    void GetPlumeHeightSource(CString& sourceStr) const;

    /** Gets the time and date for which this wind-field is valid */
    const CDateTime& GetTimeAndDate() const;

private:

    /** The speed of the wind */
    double m_windSpeed = 10.0;

    /** The uncertainty in the wind-speed */
    double m_windSpeedError = 3.0;

    /** The source of the wind-speed data */
    MET_SOURCE m_windSpeedSource = MET_DEFAULT;

    /** The direction of the wind, in degrees */
    double m_windDirection = 0.0;

    /** The uncertainty in the wind-direction */
    double m_windDirectionError = 10.0;

    /** The source of the wind-direction data */
    MET_SOURCE m_windDirectionSource = MET_DEFAULT;

    /** The altitude (in meters above sea level)
        for which this wind-field is valid
        If this wind-field is valid for all altitudes then this
            value is equal to -1.0 */
    double m_altitude = -1.0;

    /** The height of the plume. It is a bit unlogical to store
        the plume height here but quite practial... */
    double m_plumeHeight = 1000.0;

    /** The uncertainty in the plume height */
    double m_plumeHeightError = 500.0;

    /** The source of the plume-height data */
    MET_SOURCE m_plumeHeightSource = MET_DEFAULT;

    /** The estimated error for the wind */
    double m_windError = 30.0;

    /** The date and time for which this wind-information is/was valid */
    CDateTime m_time;
};

#endif