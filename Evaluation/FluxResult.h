#pragma once

#include <SpectralEvaluation/DateTime.h>
#include "../Common/WindField.h"

/** The class <b>CFluxResult</b> is a generic class for storing the results
		from a flux-calculation of a scan. The class holds the values of all the
		parameters used in the calculation (except for the measurment itself) and 
		the result of the measurement. All non-initialized variables are set to -999
		*/

namespace Evaluation
{
	class CFluxResult
	{
	public:
		CFluxResult(void);
		~CFluxResult(void);

		/** Clears the results */
		void Clear();

		/** Assignment operator */
		CFluxResult &operator=(const CFluxResult &fl2);

		/** The calculated flux, in kg/s */
		double	m_flux;

		/** True if the flux-value is a good measurement */
		bool	m_fluxOk;

		/** The wind-direction used to calculate the flux */
		double	m_windDirection;

		/** The source for the wind-direction */
		MET_SOURCE	m_windDirectionSource;

		/** The wind-speed used to calculate the flux */
		double	m_windSpeed;

		/** The source for the wind-speed */
		MET_SOURCE	m_windSpeedSource;

		/** The plume-height used to calculate the flux */
		double	m_plumeHeight;

		/** The source for the plume height */
		MET_SOURCE	m_plumeHeightSource;

		/** The cone-angle of the scanner that collected this scan */
		double	m_coneAngle;

		/** The tilt of the scanner that collected this scan */
		double	m_tilt;

		/** The compass-direction of the scanner that collected this scan */
		double	m_compass;

		/** The date and time (UTC) when the measurement was started */
		CDateTime	m_startTime;

		/** The volcano that this measurement was made at. Set to -1 if unknown */
		int			m_volcano;
	};
}