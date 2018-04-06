#include "StdAfx.h"
#include "spectruminfo.h"

CSpectrumInfo::CSpectrumInfo(void)
{
	m_batteryVoltage	= 0.0f;
	m_channel			= 0;
	m_compass			= 0;
	m_coneAngle			= 90.0f;

	memset(m_date, 0, 3*sizeof(unsigned short));
	m_device.Format("");

	m_exposureTime			= 0;

	m_fitIntensity			= 0.0f;
	m_flag					= 0;

	memset(&m_gps, 0, sizeof(CGPSData));

	m_interlaceStep			= 1;
	
	m_name.Format("");
	m_numSpec				= 0;

	m_offset				= 0.0f;

	m_peakIntensity			= 0.0f;
	m_pitch					= 0.0f;

	m_roll					= 0.0f;

	m_scanAngle				= 0;
	m_scanAngle2			= 0.0;
	m_scanIndex				= 0;
	m_scanSpecNum			= 0;
	m_specModel				= S2000;
	m_startChannel  = 0;
	memset(&m_startTime, 0, sizeof(CSpectrumTime));
	memset(&m_stopTime, 0, sizeof(CSpectrumTime));

	m_temperature		= 0.0f;
}

CSpectrumInfo::CSpectrumInfo(const CSpectrumInfo &spec){
	m_batteryVoltage	= spec.m_batteryVoltage;
	m_channel			= spec.m_channel;
	m_compass			= spec.m_compass;
	m_coneAngle			= spec.m_coneAngle;
	
	m_date[0]         = spec.m_date[0];
	m_date[1]         = spec.m_date[1];
	m_date[2]         = spec.m_date[2];
	m_device.Format("%s", (LPCSTR)spec.m_device);

	m_exposureTime		= spec.m_exposureTime;

	m_fitIntensity		= spec.m_fitIntensity;
	m_flag				= spec.m_flag;

	m_gps				= spec.m_gps;

	m_interlaceStep				= spec.m_interlaceStep;

	m_name.Format("%s", (LPCSTR)spec.m_name);
	m_volcano.Format("%s", (LPCSTR)spec.m_volcano);
	m_site.Format("%s", (LPCSTR)spec.m_site);
	m_observatory.Format("%s", (LPCSTR)spec.m_observatory);

	m_numSpec			= spec.m_numSpec;

	m_offset			= spec.m_offset;

	m_peakIntensity		= spec.m_peakIntensity;
	m_pitch				= spec.m_pitch;

	m_roll				= spec.m_roll;

	m_scanAngle			= spec.m_scanAngle;
	m_scanAngle2		= spec.m_scanAngle2;
	m_scanIndex			= spec.m_scanIndex;
	m_scanSpecNum		= spec.m_scanSpecNum;
	m_specModel			= spec.m_specModel;
	m_startChannel		= spec.m_startChannel;
	m_startTime			= spec.m_startTime;
	m_stopTime			= spec.m_stopTime;

	m_temperature		= spec.m_temperature;
}

CSpectrumInfo::~CSpectrumInfo(void)
{
}
