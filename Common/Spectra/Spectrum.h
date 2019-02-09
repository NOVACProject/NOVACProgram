#pragma once


#include "../Common.h"
#include "SpectrumInfo.h"

/**
<b>CSpectrum</b> is an implementation of a spectrum. 
  The class contains the spectral data and a CSpectrumInfo object that contains
  all auxilliary data about the spectrum.
*/
class CSpectrum
{
public:
	/** Default constructor. */
	CSpectrum(void);

	/** Copies the contents of 'spec' into this (new) spectrum. */
	CSpectrum(const CSpectrum &spec);

	/** Default destructor */
	~CSpectrum(void);

	// ----------------------------------------------------------------------
	// ------------------------ PUBLIC DATA ---------------------------------
	// ----------------------------------------------------------------------

	/** The spectral data */
	SpecData  m_data[MAX_SPECTRUM_LENGTH];

	/** The length of the spectrum */
	long    m_length;

	/** The auxilliary information about the spectrum */
	CSpectrumInfo m_info;

	// ----------------------------------------------------------------------
	// ----------------------- PUBLIC METHODS -------------------------------
	// ----------------------------------------------------------------------

	/** Returns the maximum value in the range [fromPixel, toPixel], inclusive. */
	SpecData MaxValue(long fromPixel = 0, long toPixel = MAX_SPECTRUM_LENGTH-1) const;

	/** Returns the minimum value in the range [fromPixel, toPixel], inclusive. */
	SpecData MinValue(long fromPixel = 0, long toPixel = MAX_SPECTRUM_LENGTH-1) const;

	/** Returns the average value in the range [fromPixel, toPixel], inclusive */
	SpecData AverageValue(long fromPixel = 0, long toPixel = MAX_SPECTRUM_LENGTH-1) const;

	/** Clears the supplied spectrum. This erases all data in the spectrum */
	void  Clear();

	/** Adds the provided spectrum to the current. This spectrum will afterwards
		contain the sum of the two spectra. Both spectra must have the same length. 
		@return 1 if the spectra have different length. */
	int Add(const CSpectrum &spec);

	/** Adds the provided constant to the current spectrum. All pixels in the current spectrum
		will be augmented by the provided constant 'value' */
	int Add(SpecData value);

	/** Subtracts the provided spectrum from the current. This spectrum will afterwards
		contain the difference of the two spectra. Both spectra must have the same length. 
		@return 1 if the spectra have different length. */
	int Sub(const CSpectrum &spec);

	/** Subtracts the provided constant from the current spectrum. All pixels in the current spectrum
		will be decreased by the provided constant 'value' */
	int Sub(SpecData value);

	/** Divides the current spectrum with the provided spectrum. All pixels in the current spectrum
		will be divided by the corresponding pixel in 'spec'. Both spectra must have the same length. 
		@return 1 if the spectra have different length. */
	int Div(const CSpectrum &spec);

	/** Divides the current spectrum with the provided constant. All pixels in the current spectrum
		will be divided by the provided constant 'value' */
	int Div(SpecData value);

	/** Multiplies the current spectrum with the provided spectrum. All pixels in the current spectrum
		will be multiplied by the corresponding pixel in 'spec'. Both spectra must have the same length. 
		@return 1 if the spectra have different length. */
	int Mult(const CSpectrum &spec);

	/** Multiplies the current spectrum with the provided constant. All pixels in the current spectrum
		will be multiplied by the provided constant 'value' */
	int Mult(SpecData value);

	/** Returns the electronic offset of the spectrum */
	SpecData GetOffset() const;

	/** If the channel number of this spectrum is larger than 128 then this function
			will split the current spectrum up into several spectra 
			(the maximum number of spectra that can be generated is (channel - 127) with a maximum of MAX_CHANNEL_NUM. 
			Explanation: When collecting spectra with a multichannel spectrometer
				the spectra can be collected simultaneously and read out through the ADC
				in one reading. However, the spectra will then be saved into one spectrum.
				If there are N channels being used simultaneously then the first pixel
				in the read out spectrum is the first pixel on the master channel, the 
				second pixel is the secon pixel in the first slave channel, the third pixel
				is the third pixel in the second slave, ... the N:th pixel is the N:th
				pixel in the (N-1):th slave. The (N+1):th pixel is the (N+1):th pixel
				in the master channel.
				This function separates spectra saved in this way into N different spectra.
			If the channel number is below 129 nothing will be done. 
			@return - the number of spectra generated
			@param spec1 - the spectrum to contain the master channel spectrum
			@param spec2 - the spectrum to contain the spectrum from the first slave
			@param ... - the spectra to contain spectra form slave 2, 3, 4, etc...
			*/
	int	Split(CSpectrum *spec[MAX_CHANNEL_NUM]) const;

	/** Interpolate the spectrum originating from the channel number 'channel'
			to a full '2048' sized spectrum */
	RETURN_CODE	InterpolateSpectrum();

	/** Interpolate the spectrum originating from the channel number 'channel'
			to a full '2048' sized spectrum. Output is saved in provided spectrum 'spec' */
	RETURN_CODE	InterpolateSpectrum(CSpectrum &spec) const;

	/** Short-cut to getting the number of co-added spectra */
	inline long NumSpectra() const { return this->m_info.m_numSpec; }

	/** Short-cut to getting the number of co-added spectra */
	inline long ExposureTime() const { return this->m_info.m_exposureTime; }

	/** Short-cut to getting the position of the first motor */
	inline double ScanAngle() const { return this->m_info.m_scanAngle; }

	/** Short-cut to getting the position of the second motor */
	inline double ScanAngle2() const { return this->m_info.m_scanAngle2; }

	/** Short-cut to getting the latitude */
	inline double Latitude() const { return this->m_info.m_gps.m_latitude; }
	  
	/** Short-cut to getting the longitude */
	inline double Longitude() const { return this->m_info.m_gps.m_longitude; }

	/** Short-cut to getting the altitude */
	inline double Altitude() const { return this->m_info.m_gps.m_altitude; }

	/** Short-cut to getting the compass direction */
	inline float Compass() const { return this->m_info.m_compass; }

	/** Short-cut to getting the gps-data */
	const CGPSData &GPS() const { return this->m_info.m_gps; }

	/** Short-cut to getting the channel */
	unsigned char Channel() const { return this->m_info.m_channel; }

	/** Short-cut to getting this spectrum's index in a scan */
	short ScanIndex() const { return this->m_info.m_scanIndex; }

	/** Short-cut to getting the total number of spectra in this scan */
	short SpectraPerScan() const { return this->m_info.m_scanSpecNum; }

	/** Assignment operator */
	CSpectrum &operator=(const CSpectrum &s2);

	/** Returns true if this spectrum is dark */
	bool  IsDark() const;

private:
	/** Asserts that the range [fromPixel, toPixel] (inclusive) is a valid range for this spectrum. */
	int AssertRange(long &fromPixel, long &toPixel) const;

	/** Performs the supplied function to every pixel in the two spectra */
	int PixelwiseOperation(const CSpectrum &spec, SpecData f(SpecData, SpecData));

	/** Performs the supplied function to every pixel in the current spectrum with the supplied constant */
	int PixelwiseOperation(const SpecData value, SpecData f(SpecData, SpecData));

	static SpecData Plus(SpecData a, SpecData b){return a+b;}
	static SpecData Minus(SpecData a, SpecData b){return a-b;}
	static SpecData Divide(SpecData a, SpecData b){return a/b;}
	static SpecData Multiply(SpecData a, SpecData b){return a*b;}

};