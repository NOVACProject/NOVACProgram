#pragma once

namespace WindSpeedMeasurement
{

class CWindSpeedResult
{
public:
    /** Default constructor */
    CWindSpeedResult(void);

    /** Default destructor */
    ~CWindSpeedResult(void);

    /** Assignement operator */
    CWindSpeedResult& operator=(const CWindSpeedResult& wr);

    /** The serial-number of the spectrometer that generated the time-series */
    CString	m_serial;

    /** The day of month when the measurement was made */
    int			m_date;

    /** The starting-time of the measurements (seconds since midnight, UTC) */
    int			m_startTime;

    /** The duration of the measurement (seconds) */
    int			m_duration;

    /** The distance between the two viewing-directions at plume-height */
    double	m_distance;

    /** The average delay */
    double	m_delayAvg;

    /** The standard-deviation of the delay */
    double	m_delayStd;

    /** The average correlation */
    double	m_corrAvg;

    /** The standard deviation of the correlation */
    double	m_corrStd;
};
}
