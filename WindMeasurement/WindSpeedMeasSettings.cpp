#include "StdAfx.h"
#include "windspeedmeassettings.h"
#include "../Common/Common.h"

using namespace WindSpeedMeasurement;

CWindSpeedMeasSettings::CWindSpeedMeasSettings(void)
{
	lowPassFilterAverage	= 20;
	shiftMax							= 90;		// maximum shift is 1.5 minutes
	testLength						= 120;	// compare over 2-minute intervals
	columnMin							= -300; // normally not used...
	sigmaMin							= 0.1;
	plumeHeight						= 1000.0;
	angleSeparation				= 0.08 / DEGREETORAD;	// in degrees
}

CWindSpeedMeasSettings::~CWindSpeedMeasSettings(void)
{
}
