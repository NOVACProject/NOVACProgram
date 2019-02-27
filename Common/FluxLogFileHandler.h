#pragma once

#include "../Common/Common.h"
#include "../Evaluation/FluxResult.h"
#include <SpectralEvaluation/Spectra/SpectrumInfo.h>

#include <Afxtempl.h>

namespace FileHandler
{

	class CFluxLogFileHandler
	{
	public:
		CFluxLogFileHandler(void);
		~CFluxLogFileHandler(void);

		/** The name of the flux log */
		CString m_fluxLog;

		// ------------------- PUBLIC METHODS -------------------------

		/** Reads the flux log */
		RETURN_CODE ReadFluxLog();

		// ------------------- PUBLIC DATA -------------------------

		/** Information of the measured fluxes */
		CArray<Evaluation::CFluxResult, Evaluation::CFluxResult&>	m_fluxes;

		/** Information of the instrument at the measurement occation */
		CArray<CSpectrumInfo, CSpectrumInfo&>											m_scanInfo;

		/** How many fluxes have been read in */
		int	m_fluxesNum;

	protected:
		// ------------------- PROTECTED DATA -------------------------

		typedef struct LogColumns{
			int date;				// <-- the date the scan was made
			int starttime;			// <-- the UTC-time when the measurement began
			int flux;				// <-- the calculated flux in kg/s
			int fluxOk;				// <-- the quality of the flux-value
			int windSpeed;			// <-- the wind-speed used
			int windSpeedSrc;		// <-- the wind-speed source
			int windDirection;		// <-- the wind-direction used
			int windDirectionSrc;	//<-- the wind-direction source
			int windDataSrc;		// <-- the source of all the wind-data used (speed+direction)
			int plumeHeight;		// <-- the plume-height used
			int plumeHeightSrc;		// <-- the plume-height source
			int compass;			// <-- the used compass-direction
			int coneAngle;			// <-- the cone-angle of the systems
			int tilt;				// <-- the tilt of the system
			int temperature;		// <-- the temperature of the system
			int batteryVoltage;		// <-- the battery voltage
			int expTime;			// <-- the exposure-time
		}LogColumns;

		/** Data structure to remember what column corresponds to which value in the evaluation log */
		LogColumns m_col;

		/** Reads the header line for the scan information and retrieves which 
			column represents which value. */
		void ParseScanHeader(const char szLine[8192]);

		/** Resets the information about which column data is stored in */
		void ResetColumns();

		/** Sorts the scans in order of collection */
		void SortScans();
	};
}