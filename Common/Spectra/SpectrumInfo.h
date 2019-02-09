#pragma once

#include "../../SpectralEvaluation/Spectra/GPSData.h"
#include "../../SpectralEvaluation/Spectra/DateTime.h"
#include "../SpectrometerModel.h"

/** <b>CSpectrumInfo</b> is a class that contains all auxilliary information about
		 a spectrum, such as exposure time, number of spectra averaged, time when the spectrum
		 was collected and GPS-coordinates for where it was collected.
		 Every instance of a CSpectrum should have an instance of CSpectrumInfo associated with it
		 to hold all other information than the spectral data. */
class CSpectrumInfo
{
public:
	/** Default constructor */
	CSpectrumInfo();

	/** Copy constructor */
	CSpectrumInfo(const CSpectrumInfo &spec);

	/** Default destructor */
	~CSpectrumInfo();

	/** The number of exposures that are added together */
	long m_numSpec = 0;

	/** The exposure time for each co added spectrum [ms] */
	long m_exposureTime = 0;

	/** The geographical information on where the spectrum was collected. 
		(see {@link CGPSData}) */
	CGPSData m_gps;

	/** The scan angle for the first motor when the spectrum was collected. 
			For the old (flat) Mark1 scanner this is defined as the angle from zenith [degrees]*/
	float m_scanAngle;

	/** The scan angle for the second motor when the spectrum was collected. 
			For spectra from the single-motor system, this is 0.0. */
	float m_scanAngle2;

	/** The compass direction for the scanner system that collected 
			this spectrum in degrees from north. */
	float m_compass;

	/** The battery voltage when this spectrum was read out. */
	float m_batteryVoltage;

	/** The time the spectrum collection began */
    CDateTime m_startTime;

	/** The time the spectrum collection stopped */
    CDateTime m_stopTime;

	/** The spectrometer which collected the spectrum. 
		For OceanOptics Spectrometers this is the serial number */
	CString m_device;

	/** The spectrometer model which was used to collect this spectrum */
	SPECTROMETER_MODEL  m_specModel;

	/** The name of the spectrum */
	CString m_name;

	/** Some of the settings of the setup of the scanning - instrument*/
	CString m_volcano;
	CString m_site;
	CString m_observatory;

	/** The channel with which the spectrum was collected.
			Values range from 0 (Master), 1 (1:st slave) to 7 (7:th slave).
			When using multichannel spectrometers the channel values ranges
				from 129 (Master+Slave1) to 136 (Master+slave1+...+slave7) and
				the spectra are then stored as first pixel from master, second from
				first slave, third from second slave, etc.
			This is same as described in OceanOptics manual for the S2000, with the 
			exception that numbers > 256 are subtracted with 128 (to fit into a 'char'). 
			This format is expanded with numbers... 
			Channel = 16 --> Master channel spectrum read out as every other pixel
			Channel = 17 --> Slave1 channel spectrum read out as every other pixel
			...
			Channel	= 23 --> Slave7 channel spectrum read out as every other pixel
			Channel = 32 --> Master channel spectrum read out as every third pixel
			Channel = 33 --> Slave1 channel spectrum read out as every third pixel
			...
			Channel = 39 --> Slave7 channel spectrum read out as every third pixel */
	unsigned char m_channel;

	/** This spectrum's position in the scan. 
			0 means that this is the first spectrum in the scan. */
	short  m_scanIndex;

	/** This shows how many spectrum there were in the scan. */
	short  m_scanSpecNum;

	/** I don't know the use of this variable, 
			but it is stored with the spectra, so let's keep it.*/
	unsigned char  m_flag;

	/** The start channel of the spectrum. The spectra can be read out
			partially, e.g. a read-out spectrum can contain the data from pixel
			129 to 540 on the detector. 0 by default. */
	unsigned short		m_startChannel;

	/** Larger than 1 if the spectrum has been read-out in an interlaced way. 
			E.g. for a OceanOptics spectrometer, it is possible to read out
			only every second or every third pixel in one spectrometer. 
			This is 1 by default (every pixel is read out) 
			If every other pixel is read out, this has the value of 2.
			If three spectrometers are connected to one adc, this can have a value of 3. */
	int  m_interlaceStep;

	/** The maximum intensity of the spectrum */
	float  m_peakIntensity;

	/** The maximum intensity in the fit region of the spectrum */
	float  m_fitIntensity;

	/** The opening angle of the scanner that generated this spectrum.
			The opening angle is defined as 90.0 degrees for the old scanner,
			and variable between 45 and 90 degrees for the new Chalmers scanner. 
			This is 90 by default. */
	float  m_coneAngle;

	/** The offset of the spectrum */
	float  m_offset;

	/** The temperature when the spectrum was collected */
	float  m_temperature;

	/** The roll (the 'leaning' of the box in the direction perpendicular 
			to the scanning unit). In degrees from the horizontal plane. */
	float  m_roll;

	/** The pitch (the 'leaning' of the box in the direction of the scanning unit).
			 In degrees from the horizontal plane. */
	float  m_pitch;

};
