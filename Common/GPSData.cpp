#include "StdAfx.h"
#include "gpsdata.h"

#include <math.h>

CGPSData::CGPSData(void)
{
  this->m_altitude  = 0;
  this->m_latitude  = 0.0f;
  this->m_longitude = 0.0f;
}

CGPSData::CGPSData(const CGPSData &gps2){
  this->m_altitude = gps2.m_altitude;
  this->m_latitude = gps2.m_latitude;
  this->m_longitude = gps2.m_longitude;
}

CGPSData::CGPSData(double lat, double lon, double alt){
  this->m_altitude = alt;
  this->m_latitude = lat;
  this->m_longitude = lon;
}

CGPSData::~CGPSData(void)
{
}

CGPSData &CGPSData::operator =(const CGPSData &gps2){
  this->m_altitude = gps2.Altitude();
  this->m_latitude = gps2.Latitude();
  this->m_longitude= gps2.Longitude();
  return *this;
}

/* The GPS reports latitude and longitude in the format ddmm.mmmm
  , this function converts this to the format dd.dddd */
double CGPSData::DoubleToAngle(double rawData){
	int degree;
	double remainder, fDegree;

	remainder	= fmod(rawData,100.0);
	degree		= (int)(rawData/100);
	fDegree		= degree + remainder/60.0;

	return fDegree;
}