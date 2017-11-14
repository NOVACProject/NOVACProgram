#pragma once

class CGPSData
{
public:
	/** Default constructor */
	CGPSData(void);

	/** Make a new CGPSData object with the supplied latitude, longitude and altitude */
	CGPSData(double lat, double lon, double alt);

	/** Copy constructor */
	CGPSData(const CGPSData &gps2);

	/** Default destructor */
	~CGPSData(void);

	/** Returns the latitude */
	double  Latitude() const {return m_latitude; }
	  
	/** Returns the longitude */
	double  Longitude() const {return m_longitude; }

	/** Returns the altitude */
	double    Altitude() const {return m_altitude; }

	double  m_altitude;
	double  m_latitude;
	double  m_longitude;

	/** Assignment operator */
	CGPSData &operator=(const CGPSData &gps2);

	/* The GPS reports latitude and longitude in the format ddmm.mmmm
			this function converts this to the format dd.dddd */
	static double DoubleToAngle(double rawData);
};
