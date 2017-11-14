#include "StdAfx.h"
#include "windspeedcalculator.h"

using namespace WindSpeedMeasurement;

CWindSpeedCalculator::CMeasurementSeries::CMeasurementSeries(){
	column = NULL;
	time = NULL;
	length = 0;
}

CWindSpeedCalculator::CMeasurementSeries::CMeasurementSeries(int len){
	column = new double[len];
	time = new double[len];
	length = len;
}

RETURN_CODE CWindSpeedCalculator::CMeasurementSeries::SetLength(int len){
	if(column != NULL){
		delete[] column;
		delete[] time;
	}
	column = new double[len];
	if(column == NULL)
		return FAIL;
	time = new double[len];
	if(time == NULL)
		return FAIL;
	length = len;

	return SUCCESS;
}

double CWindSpeedCalculator::CMeasurementSeries::AverageColumn(int from, int to) const{
	double sum = 0.0;

	// check input
	if(from > to || from < 0 || to < 0 || from >= length || to >= length)
		return 0.0;

	for(int k = from; k <= to; ++k){
		sum += column[k];
	}
	return sum / (to - from + 1);
}

// calculates and returns the average time between two measurements
double CWindSpeedCalculator::CMeasurementSeries::SampleInterval(){
	if(length <= 0)
		return 0.0;

	double totalTime = time[length-1] - time[0];
	return totalTime / (length - 1);
}

CWindSpeedCalculator::CMeasurementSeries::~CMeasurementSeries(){
	if(length != NULL){
		delete[] column;
		delete[] time;
	}
}


CWindSpeedCalculator::CWindSpeedCalculator(void)
{
	shift = NULL;
	corr = NULL;
	used = NULL;
	delays = NULL;
	m_length = 0;
}

CWindSpeedCalculator::~CWindSpeedCalculator(void)
{
	delete[] shift;
	delete[] corr;
	delete[] used;
}

RETURN_CODE CWindSpeedCalculator::CalculateDelay(
	double &delay, 
	const CMeasurementSeries *upWindSerie,
	const CMeasurementSeries *downWindSerie,
	const CWindSpeedMeasSettings &settings){

	CMeasurementSeries modifiedUpWind;
	CMeasurementSeries modifiedDownWind;

	// 0. Error checking of the input
	if(upWindSerie == NULL || upWindSerie->length == 0)
		return FAIL;
	if(downWindSerie == NULL || downWindSerie->length == 0)
		return FAIL;

	// 1. Start with the low pass filtering
	if(SUCCESS != LowPassFilter(upWindSerie,		&modifiedUpWind,		settings.lowPassFilterAverage))
		return FAIL;
	if(SUCCESS != LowPassFilter(downWindSerie,	&modifiedDownWind, settings.lowPassFilterAverage))
		return FAIL;

	// 1b. Get the sample time
	double sampleInterval = modifiedDownWind.SampleInterval();
	if(fabs(modifiedUpWind.SampleInterval() - sampleInterval) > 0.5){
		return FAIL; // <-- we cannot have different sample intervals of the two time series
	}

	// 1c. Calculate the length of the comparison-interval
	//		in data-points instead of in seconds
	int		comparisonLength = (int)round(settings.testLength / sampleInterval);

	// 1d. Calculate the how many pixels that we should shift maximum
	int		maximumShift = (int)round(settings.shiftMax / sampleInterval);

	// 1e. check that the resulting series is long enough
	if(modifiedDownWind.length - maximumShift - comparisonLength < maximumShift + 1)
		return FAIL; // <-- data series to short to use current settings of test length and shiftmax

	// 2. Allocate some assistance arrays 
	//		(Note that it is the down wind data series that is shifted)
	m_length		= modifiedDownWind.length;
	InitializeArrays();

	// The number of datapoints skipped because we cannot see the plume.
	int skipped = 0;

	// 3. Iterate over the set of sub-arrays in the down-wind data series
	//		Offset is the starting-point in this sub-array whos length is 'comparisonLength'
	for(int offset = 0; offset < m_length-(int)maximumShift - comparisonLength; ++offset){
		double highestCorr;
		int bestShift;
		
		// 3a. Pick out the sub-vectors
		double	*series1						= modifiedUpWind.column + offset;
		double	*series2						= modifiedDownWind.column	+ offset;
		unsigned int series1Length	= modifiedUpWind.length - offset;
		unsigned int series2Length	= comparisonLength;

		// 3b. The midpoint in the subvector
		int midPoint = (int)round(offset + comparisonLength / 2);

		//	3c. Check if we see the plume...
		if(upWindSerie->AverageColumn(offset, offset + comparisonLength) < settings.columnMin){
			// we don't see the plume. 
			skipped = skipped + 1;
			continue;
		}

		// 3d. Do a shifting...
		FindBestCorrelation(series1, series1Length, series2, series2Length, maximumShift, highestCorr, bestShift);

		// 3e. Calculate the time-shift
		delays[midPoint]			= bestShift * sampleInterval;
		corr[midPoint]				= highestCorr;
		shift[midPoint]				= bestShift - 1;
		used[midPoint]				= 1;
	}

	return SUCCESS;
}

/** Performs a low pass filtering on the supplied measurement series. 
	The number of iterations in the filtering is given by 'nIterations'
	if nIterations is zero, nothing will be done. */
RETURN_CODE CWindSpeedCalculator::LowPassFilter(const CMeasurementSeries *series, CMeasurementSeries *result, unsigned int nIterations){
	
	// 1. Check for errors in input
	if(series == NULL || series->length == 0 || result == NULL)
		return FAIL;

	// 2. Calculate the old and the new data series lengths
	int length			= series->length;							// <-- the length of the old data series
	int newLength		= length - nIterations - 1;		// <-- the length of the new data series

	if(newLength <= 0)
		return FAIL;

	// 3. If no iterations should be done, the output should be a copy of the input...
	if(nIterations == 0){
		if(SUCCESS != result->SetLength(length))
			return FAIL;

		for(int i = 0; i < length; ++i){
			result->column[i] = series->column[i];
			result->time[i]		= series->time[i];
		}
		result->length = series->length;
		return SUCCESS;
	}

	// 4. Change the length of the resulting series.
	if(SUCCESS != result->SetLength(newLength))
		return FAIL;

	// 5. Calculate the factorials
	double *factorial = new double[nIterations];
	factorial[0] = 1;
	if(nIterations > 1)
		factorial[1] = 1;
	for(unsigned int k = 2; k < nIterations; ++k)
		factorial[k] = factorial[k-1] * (double)k;		

	double coeffSum = 0; // <-- the sum of all coefficients, to make sure that we dont increase the values...

	// 6. Allocate temporary arrays for the time-stamps and the column values
	double *tmpCol	= new double[newLength];
	double *tmpTime = new double[newLength];
	memset(tmpCol, 0, newLength * sizeof(double));
	memset(tmpTime, 0, newLength * sizeof(double));

	// 7. Do the filtering...
	for(k = 1; k < nIterations + 1; ++k){
		// 7a. The coefficient in the binomial - expansion
		double coefficient = factorial[nIterations - 1] / (factorial[nIterations - k] * factorial[k - 1]);
		coeffSum	+= coefficient;	

		// 7b. Do the filtering for all data points in the new time series
		for(int i = 0; i < newLength; ++i){
			tmpCol[i] += coefficient * series->column[k-1 + i];
			tmpTime[i] += coefficient * series->time[k-1 + i];
		}
	}

	// 8. Divide by the coeffsum to preserve the energy...
	for(int i = 0; i < newLength; ++i){
		result->time[i]		= tmpTime[i] / coeffSum;
		result->column[i] = tmpCol[i] / coeffSum;
	}
	result->length = newLength;	

	delete[] factorial;
	delete[] tmpCol;
	delete[] tmpTime;

	return SUCCESS;
}

/** Shifts the vector 'shortVector' against the vector 'longVector' and returns the
			shift for which the correlation between the two is highest. */
RETURN_CODE CWindSpeedCalculator::FindBestCorrelation(
	const double *longVector, unsigned long longLength, 
	const double *shortVector, unsigned long shortLength,
	unsigned int maximumShift, 
	double &highestCorr, int &bestShift){

	// 0. Check for errors in the input
	if(longLength == 0 || shortLength == 0)
		return FAIL;
	if(longVector == NULL || shortVector == NULL)
		return FAIL;
	if(longLength <= shortLength)
		return FAIL;

	// Reset
	highestCorr = 0;
	bestShift = 0;

	// To calculate the correlation, we need to pick out a subvector (with length 'shortLength)
	//	from the longVector and compare this with 'shortVector' and calculate the correlation.
	// left is the startingpoint of this subvector
	int left	= 0;

	// 1. Start shifting
	while((left+shortLength) < (int)longLength && left < (int)maximumShift ){
		double C = correlation(shortVector, longVector + left, shortLength);
		if(C > highestCorr){
			highestCorr = C;
			bestShift		= left;
		}
		++left;
	}

	return SUCCESS;
}

/** Calculates the correlation between the two vectors 'x' and 'y', both of length 'length' 
		@return - the correlation between the two vectors. */
double CWindSpeedCalculator::correlation(const double *x, const double *y, long length){
	double s_xy = 0; // <-- the dot-product X*Y
	double s_x2 = 0; // <-- the dot-product X*X
	double s_x  = 0; // <-- sum of all elements in X
	double s_y	= 0; // <-- sum of all elements in Y
	double s_y2 = 0; // <-- the dot-product Y*Y
	double c		= 0; // <-- the final correlation
	double eps = 1e-5;

	if(length <= 0)
		return 0;

	for(int k = 0; k < length; ++k){
		s_xy += x[k] * y[k];
		s_x2 += x[k] * x[k];
		s_x  += x[k];
		s_y  += y[k];
		s_y2 += y[k] * y[k];
	}

	double nom = (length * s_xy - s_x*s_y);
	double denom = sqrt(( (length*s_x2 - s_x*s_x) * (length*s_y2 - s_y*s_y) ));

	if((fabs(nom - denom) < eps) && (fabs(denom) < eps))
			c = 1.0;
	else
			c = nom / denom;
	
	return c;
}

void CWindSpeedCalculator::InitializeArrays(){
	delete[]	shift, corr, used, delays;
	shift				= new double[m_length];
	corr				= new double[m_length];
	used				= new double[m_length];
	delays			= new double[m_length]; // <-- the delays
	memset(corr,	0, m_length*sizeof(double));
	memset(shift, 0, m_length*sizeof(double));
	memset(used,	0, m_length*sizeof(double));
	memset(delays,0, m_length*sizeof(double));
}