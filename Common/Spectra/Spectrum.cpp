#include "StdAfx.h"
#include "spectrum.h"

#include <cstdarg>

CSpectrum::CSpectrum(void)
{
	// reset everything 
	memset(m_data, 0, sizeof(SpecData) * MAX_SPECTRUM_LENGTH);
	m_length        = 0;
}

CSpectrum::CSpectrum(const CSpectrum &spec){
	this->m_info    = spec.m_info;
	this->m_length  = spec.m_length;
	memcpy(this->m_data, &spec.m_data, sizeof(SpecData) * MAX_SPECTRUM_LENGTH);
}

CSpectrum::~CSpectrum(void)
{
}


int CSpectrum::AssertRange(long &fromPixel, long &toPixel) const{
	/* Check the input */
	ASSERT(fromPixel >= 0 && toPixel >= 0);

	toPixel = min(toPixel, m_length - 1);
	ASSERT(fromPixel <= toPixel);

	return 0;
}

SpecData CSpectrum::MaxValue(long fromPixel, long toPixel) const{
	/* Check the input */
	AssertRange(fromPixel, toPixel);

	SpecData maxv = m_data[fromPixel];
	for(long i = fromPixel+1; i <= toPixel; ++i){
		maxv = max(maxv, m_data[i]);
	}
	return maxv;
}

SpecData CSpectrum::MinValue(long fromPixel, long toPixel) const{
	/* Check the input */
	AssertRange(fromPixel, toPixel);
	  
	SpecData minv = m_data[fromPixel];
	for(long i = fromPixel+1; i <= toPixel; ++i){
		minv = min(minv, m_data[i]);
	}
	return minv;
}

SpecData CSpectrum::AverageValue(long fromPixel, long toPixel) const{
	/* Check the input */
	AssertRange(fromPixel, toPixel);

	SpecData avg = m_data[fromPixel];
	for(long i = fromPixel+1; i <= toPixel; ++i){
		avg += m_data[i];
	}
	return (avg / (SpecData)(toPixel - fromPixel + 1));
}

// Addition
int CSpectrum::Add(const CSpectrum &spec){
	if(m_length != spec.m_length)
		return 1;

	m_length = spec.m_length;
	m_info.m_numSpec += spec.m_info.m_numSpec;

	CSpectrumTime localCopy = spec.m_info.m_startTime;
	if(localCopy < m_info.m_startTime)
		m_info.m_startTime = localCopy;

	localCopy = spec.m_info.m_stopTime;
	if(m_info.m_stopTime < localCopy)
		m_info.m_stopTime = localCopy;

	return PixelwiseOperation(spec, &CSpectrum::Plus);
}
int CSpectrum::Add(const SpecData value){
	return PixelwiseOperation(value, &CSpectrum::Plus);
}

// Subtraction
int CSpectrum::Sub(const CSpectrum &spec){
	return PixelwiseOperation(spec, &CSpectrum::Minus);
}

int CSpectrum::Sub(const SpecData value){
	return PixelwiseOperation(value, &CSpectrum::Minus);
}

// Multiplication
int CSpectrum::Mult(const CSpectrum &spec){
	return PixelwiseOperation(spec, &CSpectrum::Multiply);
}

int CSpectrum::Mult(const SpecData value){
	return PixelwiseOperation(value, &CSpectrum::Multiply);
}

// Division
int CSpectrum::Div(const CSpectrum &spec){
	return PixelwiseOperation(spec, &CSpectrum::Divide);
}
int CSpectrum::Div(const SpecData value){
	return PixelwiseOperation(value, &CSpectrum::Divide);
}

// performe the supplied operation on all pixels in the two spectra
int CSpectrum::PixelwiseOperation(const CSpectrum &spec, SpecData f(SpecData, SpecData)){
	if(spec.m_length != m_length)
		return 1;

	for(long i = 0; i < m_length; ++i){
		m_data[i] = f(m_data[i], spec.m_data[i]);
	}

	return 0;
}
//  Performs the supplied function to every pixel in the current spectrum with the supplied constant 
int CSpectrum::PixelwiseOperation(const SpecData value, SpecData f(SpecData, SpecData)){
	for(long i = 0; i < m_length; ++i){
		m_data[i] = f(m_data[i], value);
	}

	return 0;
}

CSpectrum &CSpectrum::operator =(const CSpectrum &s2){
	this->m_length = min(s2.m_length, MAX_SPECTRUM_LENGTH);
	this->m_length = max(this->m_length, 0);
	this->m_info = s2.m_info;
	memcpy(m_data, s2.m_data, sizeof(SpecData) * m_length);
	return *this;
}

SpecData CSpectrum::GetOffset() const{
	SpecData avg;

	if(m_info.m_startChannel > 20){
		return 0; // no idea...
	}

	// The covered pixels, where the offset is calculated
	int from	= 2		/ m_info.m_interlaceStep; 
	int to		= 20	/ m_info.m_interlaceStep;

	// the offset is the average of the eighteen first pixels minus the highest one
	avg = AverageValue(from, to) * (to - from + 1);
	avg -= MaxValue(from, to);
	avg /= (to - from);

	return avg;
}

bool CSpectrum::IsDark() const{
	// take the highest part of the (normal) spectrum
	int low =		1130 / m_info.m_interlaceStep;
	int high =	1158 / m_info.m_interlaceStep;

	// check the ranges.
	if(low > m_length){
		int diff = high - low;
		high	= m_length - 1;
		low		= m_length - 1 - diff;
	}else if(high > m_length){
		high = m_length;
	}

	double average = this->AverageValue(low, high);
	double offset  = this->GetOffset();

	if(offset > 1e-2){
		if(fabs(average - offset) < 4){
			return true;
		}else{
			return false;
		}
	}else{
		// offset == 0 means that we could not calculate the offset
		double maxV = this->MaxValue();
		double minV = this->MinValue();
		if(maxV - minV > 20)
			return false;
		else
			return true;
	}
}

void CSpectrum::Clear(){
	memset(m_data, 0, MAX_SPECTRUM_LENGTH*sizeof(SpecData));
	
	m_length = 0;
	// uchar
	m_info.m_channel = m_info.m_flag = 0;
	// ushort
	m_info.m_startChannel = 0;
	// float
	m_info.m_compass = m_info.m_scanAngle = m_info.m_scanAngle2 = m_info.m_peakIntensity = 0;
	// long
	m_info.m_exposureTime =  m_info.m_numSpec = 0;
	// date
	m_info.m_date[0] = m_info.m_date[1] = m_info.m_date[2] = 0;
	// CString
	m_info.m_device.Format("");
	m_info.m_name.Format("");
	// short
	m_info.m_scanIndex = m_info.m_scanSpecNum = 0;
	// GPS
	m_info.m_gps.m_altitude = 0;
	m_info.m_gps.m_latitude = m_info.m_gps.m_longitude = 0;
	// Time
	m_info.m_startTime.hr = m_info.m_startTime.m = m_info.m_startTime.msec = m_info.m_startTime.sec = 0;
	m_info.m_stopTime.hr = m_info.m_stopTime.m = m_info.m_stopTime.msec = m_info.m_stopTime.sec = 0;
}

int	CSpectrum::Split(CSpectrum *spec[MAX_CHANNEL_NUM]) const{
	int i;

	// If the spectrum is collected using a single channel, do nothing....
	if(Channel() < 128)
		return 0;

	// The number of spectra that this spectrum can be separated into
	int NSpectra = (Channel() - 127);

	// Check for illegal numbers
	if(NSpectra <= 0 || NSpectra > MAX_CHANNEL_NUM)
		return 0;

	// Copy the spectrum information
	for(i = 0; i < NSpectra; ++i){
		spec[i]->m_info                 = m_info;
		spec[i]->m_info.m_interlaceStep = NSpectra;
		spec[i]->m_info.m_channel       = i + 16 * (spec[i]->m_info.m_interlaceStep - 1);
		spec[i]->m_length               = 0;
	}
		
	// Which spectrum to start with, the master or the slave channel
	//	This depends on the start-channel if a partial spectrum has been
	//	read out. By default, the odd data-points belong to the master
	//	channel and the even to the 1:st slave channel. This since the first
	//	data-point in the spectrum always is 0, and then follows a data-point
	//	from the master, one from the slave, one from the master etc....
	// TODO!!! Test this with the tripple spectormeter

	int specIndex = m_info.m_startChannel % NSpectra;

	for(i = 1; i < m_length; ++i){
		if(specIndex >= 0 && specIndex < MAX_CHANNEL_NUM){
			// Set the pixel data of the correct spectrum
			spec[specIndex]->m_data[spec[specIndex]->m_length] = m_data[i];

			// Change the length of the spectrum
			spec[specIndex]->m_length++;
		}else{
			ShowMessage("CSpectrum::Split was called with an illegal start-channel");
		}

		// Take the next spectrum to update
		specIndex += 1;
		specIndex %= NSpectra;
	}

	return NSpectra;
}

/** Interpolate the spectrum originating from the channel number 'channel' */
RETURN_CODE CSpectrum::InterpolateSpectrum(){
	SpecData	data[MAX_SPECTRUM_LENGTH];
	memset(data, 0, MAX_SPECTRUM_LENGTH * sizeof(SpecData));
	int step = 2, start = 0;	// start is the first data-point we know in the spectrum

	// If this is not an partial spectrum, then return false
	if(m_info.m_channel < MAX_CHANNEL_NUM)
		return FAIL;

	// Get the channel number of this spectrum
	switch(m_info.m_channel){
		case 16:	start = 0; step = 2; break;
		case 17:	start = 1; step = 2; break;
		default:	return FAIL;
	}

	// Get the length of this spectrum
	int newLength = m_length * step;

	// Copy the data we have
	for(int k = 0; k < m_length; ++k){
		data[step*k + start]	= m_data[k];
	}

	// Interpolate the data we don't have
	if(start == 1){
		data[0]		= data[1];
	}
	for(int k = start + 1; k < (newLength - 1); k += step){
		data[k]		= (data[k-1] + data[k+1]) * 0.5;
	}
	if(start == 0){
		data[newLength-1]	= data[newLength-2];
	}

	// Get the data back
	memcpy(m_data, data, newLength*sizeof(SpecData));
	m_length = m_length * step;

	// Correct the channel number
	switch(m_info.m_channel){
		case 16:	m_info.m_channel = 0; break;
		case 17:	m_info.m_channel = 1; break;
		case 18:	m_info.m_channel = 2; break;
	}

	return SUCCESS;
}

/** Interpolate the spectrum originating from the channel number 'channel' */
RETURN_CODE CSpectrum::InterpolateSpectrum(CSpectrum &spec) const{
	SpecData	data[MAX_SPECTRUM_LENGTH];
	memset(data, 0, MAX_SPECTRUM_LENGTH * sizeof(SpecData));
	int step = 2, start = 0;	// start is the first data-point we know in the spectrum

	// If this is not an partial spectrum, then return false
	if(m_info.m_channel < MAX_CHANNEL_NUM)
		return FAIL;

	// Get the channel number of this spectrum
	switch(m_info.m_channel){
		case 16:	start = 0; step = 2; break;
		case 17:	start = 1; step = 2; break;
		default:	return FAIL;
	}

	// Get the 'new' length of this spectrum
	int newLength = m_length * step;

	// Copy the data we have
	for(int k = 0; k < m_length; ++k){
		data[step*k + start]	= m_data[k];
	}

	// Interpolate the data we don't have
	if(start == 1){
		data[0]		= data[1];
	}
	for(int k = start + 1; k < (newLength - 1); k += step){
		data[k]		= (data[k-1] + data[k+1]) * 0.5;
	}
	if(start == 0){
		data[newLength-1]	= data[newLength-2];
	}

	// Get the data back
	spec						= *this;
	spec.m_length				= m_length * step;
	memcpy(spec.m_data,			data,	newLength*sizeof(SpecData));

	// Correct the channel number
	switch(m_info.m_channel){
		case 16:	spec.m_info.m_channel = 0; break;
		case 17:	spec.m_info.m_channel = 1; break;
		case 18:	spec.m_info.m_channel = 2; break;
	}

	return SUCCESS;
}
