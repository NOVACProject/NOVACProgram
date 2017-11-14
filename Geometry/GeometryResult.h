#pragma once

namespace Geometry{
	class CGeometryResult
	{
	public:
		/** Default constructor */
		CGeometryResult(void);

		/** Default destructor */
		~CGeometryResult(void);

		/** Assignement operator */
		CGeometryResult &operator=(const CGeometryResult &gr);

		/** The day of month when the measurement was made */
		int m_date;

		/** The average of the starting-time of the
				two scans that were combined to generate the result
				(seconds since midnight, UTC) */
		int m_startTime;

		/** The calculated plume height (in meters above sea level) */
		double m_plumeHeight;

		/** The estimated error in plume height (in meters) */
		double	m_plumeHeightError;

		/** The calculated wind-direction (degrees from north) */
		double	m_windDirection;

		/** The estimated error in the calculated wind-direction (degrees) */
		double	m_windDirectionError;
	};
}