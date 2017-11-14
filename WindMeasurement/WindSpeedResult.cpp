#include "StdAfx.h"
#include "windspeedresult.h"

using namespace WindSpeedMeasurement;

CWindSpeedResult::CWindSpeedResult(void)
{
	// The information about the measurement
	m_serial.Format("");
	m_date			= 0;
	m_startTime = 0;
	m_duration	= 0;

	// The settings
	m_distance	= 0.0;

	// The results
	m_delayAvg	= 0.0;
	m_delayStd	= 0.0;
	m_corrAvg		= 0.0;
	m_corrStd		= 0.0;
}

CWindSpeedResult::~CWindSpeedResult(void)
{
}

/** Assignement operator */
CWindSpeedResult &CWindSpeedResult::operator=(const CWindSpeedResult &wr){
	// The information about the measurement
	m_serial.Format(wr.m_serial);
	m_date			= wr.m_date;
	m_startTime = wr.m_startTime;
	m_duration	= wr.m_duration;

	// The settings
	m_distance	= wr.m_distance;

	// The results
	m_delayAvg	= wr.m_delayAvg;
	m_delayStd	= wr.m_delayStd;
	m_corrAvg		= wr.m_corrAvg;
	m_corrStd		= wr.m_corrStd;

	return *this;
}
