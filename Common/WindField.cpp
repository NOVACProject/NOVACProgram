#include "StdAfx.h"
#include "windfield.h"

CWindField::CWindField(void)
{
	this->m_plumeHeight			= 1000.0;
	this->m_plumeHeightError	= 500.0;
	this->m_plumeHeightSource	= MET_DEFAULT;
	
	this->m_windDirection		= 0.0;
	this->m_windDirectionError	= 10.0;
	this->m_windDirectionSource	= MET_DEFAULT;

	this->m_windSpeed			= 10.0;
	this->m_windSpeedError		= 3.0;
	this->m_windSpeedSource		= MET_DEFAULT;

	this->m_altitude			= -1.0; // all altitudes

	this->m_windError			= 30.0;
}

CWindField::~CWindField(void)
{
}

/** assignment operator */
CWindField &CWindField::operator=(const CWindField &wf2){
	this->m_plumeHeight				= wf2.m_plumeHeight;
	this->m_plumeHeightError		= wf2.m_plumeHeightError;
	this->m_plumeHeightSource		= wf2.m_plumeHeightSource;

	this->m_windDirection			= wf2.m_windDirection;
	this->m_windDirectionError		= wf2.m_windDirectionError;
	this->m_windDirectionSource		= wf2.m_windDirectionSource;

	this->m_windSpeed				= wf2.m_windSpeed;
	this->m_windSpeedError			= wf2.m_windSpeedError;
	this->m_windSpeedSource			= wf2.m_windSpeedSource;

	this->m_altitude				= wf2.m_altitude;

	this->m_time					= wf2.m_time;

	return *this;
}

/** Sets the wind-speed */
void CWindField::SetWindSpeed(double ws, MET_SOURCE source){
	this->m_windSpeed				= ws;
	this->m_windSpeedSource			= source;
}

/** Sets the wind-speed */
void CWindField::SetWindSpeed(double ws, MET_SOURCE source, double wsError){
	this->m_windSpeed				= ws;
	this->m_windSpeedSource			= source;
	this->m_windSpeedError			= wsError;
}

/** Sets the wind-direction */
void CWindField::SetWindDirection(double wd, MET_SOURCE source){
	this->m_windDirection				= wd;
	this->m_windDirectionSource			= source;
}

/** Sets the wind-speed */
void CWindField::SetWindDirection(double wd, MET_SOURCE source, double wdError){
	this->m_windDirection				= wd;
	this->m_windDirectionSource			= source;
	this->m_windDirectionError			= wdError;
}

/** Sets the plume height */
void CWindField::SetPlumeHeight(double ph, MET_SOURCE source){
	this->m_plumeHeight				= ph;
	this->m_plumeHeightSource		= source;
}

/** Sets the plume height */
void CWindField::SetPlumeHeight(double ph, MET_SOURCE source, double phError){
	this->m_plumeHeight				= ph;
	this->m_plumeHeightSource		= source;
	this->m_plumeHeightError		= phError;
}

/** Sets the altitude for which this wind-field is valid */
void CWindField::SetAltitude(double altitude){
	this->m_altitude = altitude;
}

/** Sets the time and/or date the wind-field is valid for */
void CWindField::SetTimeAndDate(const CDateTime &dt){
	this->m_time = dt;
}

/** Sets the time the wind-field is valid for */
void CWindField::SetTime(int hour, int minute, int second){
	m_time.hour		= hour;
	m_time.minute	= minute;
	m_time.second	= second;
}

/** Sets the date the wind-field is valid for */
void CWindField::SetDate(int year, int month, int day){
	m_time.year		= year;
	m_time.month	= month;
	m_time.day		= day;
}

/** Gets the wind-speed */
double CWindField::GetWindSpeed() const{
	return this->m_windSpeed;
}

/** Gets the source of the wind-speed */
MET_SOURCE CWindField::GetWindSpeedSource() const{
	return this->m_windSpeedSource;
}

/** Gets the source of the wind-speed */
void CWindField::GetWindSpeedSource(CString &str) const{
	
	if(MET_DEFAULT == m_windSpeedSource)
		str.Format("default");
	else if(MET_USER == m_windSpeedSource)
		str.Format("user");
	else if(MET_ECMWF_FORECAST == m_windSpeedSource)
		str.Format("ecmwf_forecast");
	else if(MET_ECMWF_ANALYSIS == m_windSpeedSource)
		str.Format("ecmwf_analysis");
	else if(MET_DUAL_BEAM_MEASUREMENT == m_windSpeedSource)
		str.Format("dual_beam_measurement");
	else
		str.Format("unknown");

	return;
}

/** Gets the wind-direction */
double CWindField::GetWindDirection() const{
	return this->m_windDirection;
}

/** Gets the source of the wind-direction */
MET_SOURCE CWindField::GetWindDirectionSource() const{
	return this->m_windDirectionSource;
}

/** Gets the source of the wind-direction */
void CWindField::GetWindDirectionSource(CString &str) const{

	if(MET_DEFAULT == m_windDirectionSource)
		str.Format("default");
	else if(MET_USER == m_windDirectionSource)
		str.Format("user");
	else if(MET_ECMWF_FORECAST == m_windDirectionSource)
		str.Format("ecmwf_forecast");
	else if(MET_ECMWF_ANALYSIS == m_windDirectionSource)
		str.Format("ecmwf_analysis");
	else if(MET_GEOMETRY_CALCULATION == m_windDirectionSource)
		str.Format("triangulation");
	else
		str.Format("unknown");

}

/** Gets the plume-height */
double CWindField::GetPlumeHeight() const{
	return this->m_plumeHeight;
}

/** Gets the source of the plume-height */
MET_SOURCE CWindField::GetPlumeHeightSource() const{
	return this->m_plumeHeightSource;
}

/** Gets the source of the plume-height */
void CWindField::GetPlumeHeightSource(CString &str) const{

	if(MET_DEFAULT == m_plumeHeightSource)
		str.Format("default");
	else if(MET_USER == m_plumeHeightSource)
		str.Format("user");
	else if(MET_ECMWF_FORECAST == m_plumeHeightSource)
		str.Format("ecmwf_forecast");
	else if(MET_ECMWF_ANALYSIS == m_plumeHeightSource)
		str.Format("ecmwf_analysis");
	else if(MET_GEOMETRY_CALCULATION == m_plumeHeightSource)
		str.Format("triangulation");
	else
		str.Format("unknown");

	return;
}

/** Gets the time and date for which this wind-field is valid */
const CDateTime &CWindField::GetTimeAndDate() const{
	return this->m_time;
}

/** Gets the estimate for the total error in the wind-field */
double	CWindField::GetWindError() const{
	return m_windError;
}

/** Sets the estimate for the total error in the wind-field */
void	CWindField::SetWindError(double err){
	m_windError = err;
}
