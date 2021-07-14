#include "StdAfx.h"
#include "WindField.h"

CWindField::CWindField(const CWindField& other) :
    m_windSpeed(other.m_windSpeed),
    m_windSpeedError(other.m_windSpeedError),
    m_windSpeedSource(other.m_windSpeedSource),
    m_windDirection(other.m_windDirection),
    m_windDirectionError(other.m_windDirectionError),
    m_windDirectionSource(other.m_windDirectionSource),
    m_altitude(other.m_altitude),
    m_plumeHeight(other.m_plumeHeight),
    m_plumeHeightError(other.m_plumeHeightError),
    m_plumeHeightSource(other.m_plumeHeightSource),
    m_windError(other.m_windError),
    m_time(other.m_time)
{
}

CWindField& CWindField::operator=(const CWindField& other)
{
    this->m_plumeHeight = other.m_plumeHeight;
    this->m_plumeHeightError = other.m_plumeHeightError;
    this->m_plumeHeightSource = other.m_plumeHeightSource;

    this->m_windDirection = other.m_windDirection;
    this->m_windDirectionError = other.m_windDirectionError;
    this->m_windDirectionSource = other.m_windDirectionSource;

    this->m_windSpeed = other.m_windSpeed;
    this->m_windSpeedError = other.m_windSpeedError;
    this->m_windSpeedSource = other.m_windSpeedSource;

    this->m_altitude = other.m_altitude;

    this->m_time = other.m_time;

    return *this;
}

void CWindField::SetWindSpeed(double ws, MET_SOURCE source, double wsError)
{
    this->m_windSpeed = ws;
    this->m_windSpeedSource = source;
    this->m_windSpeedError = wsError;
}

void CWindField::SetWindDirection(double wd, MET_SOURCE source, double wdError)
{
    this->m_windDirection = wd;
    this->m_windDirectionSource = source;
    this->m_windDirectionError = wdError;
}

void CWindField::SetPlumeHeight(double ph, MET_SOURCE source, double phError)
{
    this->m_plumeHeight = ph;
    this->m_plumeHeightSource = source;
    this->m_plumeHeightError = phError;
}

void CWindField::SetTimeAndDate(const novac::CDateTime &dt)
{
    this->m_time = dt;
}

void CWindField::SetTime(int hour, int minute, int second)
{
    m_time.hour = hour;
    m_time.minute = minute;
    m_time.second = second;
}

void CWindField::SetDate(int year, int month, int day)
{
    m_time.year = year;
    m_time.month = month;
    m_time.day = day;
}

double CWindField::GetWindSpeed() const
{
    return this->m_windSpeed;
}

MET_SOURCE CWindField::GetWindSpeedSource() const
{
    return this->m_windSpeedSource;
}

void CWindField::GetWindSpeedSource(CString &str) const
{
    if (MET_DEFAULT == m_windSpeedSource)
        str = "default";
    else if (MET_USER == m_windSpeedSource)
        str = "user";
    else if (MET_ECMWF_FORECAST == m_windSpeedSource)
        str = "ecmwf_forecast";
    else if (MET_ECMWF_ANALYSIS == m_windSpeedSource)
        str = "ecmwf_analysis";
    else if (MET_DUAL_BEAM_MEASUREMENT == m_windSpeedSource)
        str = "dual_beam_measurement";
    else
        str = "unknown";

    return;
}

double CWindField::GetWindDirection() const
{
    return this->m_windDirection;
}

MET_SOURCE CWindField::GetWindDirectionSource() const
{
    return this->m_windDirectionSource;
}

void CWindField::GetWindDirectionSource(CString &str) const
{
    if (MET_DEFAULT == m_windDirectionSource)
        str = "default";
    else if (MET_USER == m_windDirectionSource)
        str = "user";
    else if (MET_ECMWF_FORECAST == m_windDirectionSource)
        str = "ecmwf_forecast";
    else if (MET_ECMWF_ANALYSIS == m_windDirectionSource)
        str = "ecmwf_analysis";
    else if (MET_GEOMETRY_CALCULATION == m_windDirectionSource)
        str = "triangulation";
    else
        str = "unknown";
}

double CWindField::GetPlumeHeight() const
{
    return this->m_plumeHeight;
}

MET_SOURCE CWindField::GetPlumeHeightSource() const
{
    return this->m_plumeHeightSource;
}

void CWindField::GetPlumeHeightSource(CString& sourceStr) const
{
    if (MET_DEFAULT == m_plumeHeightSource)
        sourceStr = "default";
    else if (MET_USER == m_plumeHeightSource)
        sourceStr = "user";
    else if (MET_ECMWF_FORECAST == m_plumeHeightSource)
        sourceStr = "ecmwf_forecast";
    else if (MET_ECMWF_ANALYSIS == m_plumeHeightSource)
        sourceStr = "ecmwf_analysis";
    else if (MET_GEOMETRY_CALCULATION == m_plumeHeightSource)
        sourceStr = "triangulation";
    else
        sourceStr = "unknown";

    return;
}

const novac::CDateTime &CWindField::GetTimeAndDate() const
{
    return this->m_time;
}

double CWindField::GetWindError() const
{
    return m_windError;
}

void CWindField::SetWindError(double err)
{
    m_windError = err;
}
