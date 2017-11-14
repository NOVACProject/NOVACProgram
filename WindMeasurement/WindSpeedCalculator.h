#pragma once

#include "../Common/Common.h"
#include "windspeedmeassettings.h"

namespace WindSpeedMeasurement{

	/** The <b>CWindSpeedCalculator</b> class contains the basic
			algorithms for calculating wind speeds from measured data series
			of column variations using the correlation between the two 
			data series. The idea being that the two time series have been
			measured in such a way that one of them measured on a more up-wind
			position along the plume than the other one. Assuming a certain
			altitude of the plume, the speed of the plume can be calculated
			by calculating the temporal delay between the two time series. */
	class CWindSpeedCalculator
	{
	public:

		class CMeasurementSeries{
		public:
			CMeasurementSeries();			// <-- Creates an empty measurement series
			CMeasurementSeries(int len);	// <-- Creates a measurement series of length 'len'
			~CMeasurementSeries();
			RETURN_CODE SetLength(int len); // <-- changes the length of the measurment series to 'len'
			double	AverageColumn(int from, int to) const; // <-- calculated the average column value between 'from' and 'to'
			double	SampleInterval();		// <-- calculates and returns the average time between two measurements
			double	*column;
			double	*time;
			long		length;
		};

		CWindSpeedCalculator(void);
		~CWindSpeedCalculator(void);

		/** Calculate the time delay between the two provided time series 
			@param delay - The return value of the function. Will be set to the time
				delay between the two data series, in seconds. Will be positive if the
				'upWindColumns' comes temporally before the 'downWindColumns', otherwise negative.
			@param upWindSerie - the measurement for the more upwind time series
			@param downWindSerie - the measurement for the more downwind time series
			@param settings - The settings for how the calculation should be done
		*/
		RETURN_CODE CalculateDelay(double &delay,
			const CMeasurementSeries *upWindSerie,
			const CMeasurementSeries *downWindSerie,
			const CWindSpeedMeasSettings &settings);

		/** The calculated values. These will be filled in after a call to 'CalculateDelay'
				Before that they are null and cannot be used. The length of these arrays are 'm_length' */
		double	*shift, *corr, *used, *delays;
		int			m_length;

		/** Intializes the arrays 'shift', 'corr', 'used' and 'delays' before they are used*/
		void InitializeArrays();

		/** Performs a low pass filtering on the supplied measurement series. 
				The number of iterations in the filtering is given by 'nIterations'
				if nIterations is zero, nothing will be done. */
		static RETURN_CODE LowPassFilter(const CMeasurementSeries *series, CMeasurementSeries *result, unsigned int nIterations);

	protected:

		/** Shifts the vector 'shortVector' against the vector 'longVector' and returns the
					shift for which the correlation between the two is highest. 
					The length of the longVector must be larger than the length of the short vector! */
		static RETURN_CODE FindBestCorrelation(
			const double *longVector, unsigned long longLength, 
			const double *shortVector, unsigned long shortLength,
			unsigned int maximumShift,
			double &highestCorr, int &bestShift);


		/** Calculates the correlation between the two vectors 'x' and 'y', both of length 'length' 
				@return - the correlation between the two vectors. */
		static double	correlation(const double *x, const double *y, long length);
	};
}