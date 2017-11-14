#pragma once

#include "../Evaluation/Spectrometer.h"

namespace Geometry{
	class CRealTimeSetupChanger
	{
	public:
		CRealTimeSetupChanger(void);
		~CRealTimeSetupChanger(void);

		/** Runs through the history of the CSpectrometer and checks the settings
				if the instrument is scanning in a good way...
				@return true if the cfg.txt inside the instrument should be changed else return false */
		static bool	IsTimeToChangeCfg(const Evaluation::CSpectrometer *spectrometer, double &alpha_min, double &alpha_max, double &phi, double &beta, bool &flat);

		/** Changes the cfg.txt for the supplied spectrometer */
		static void	ChangeCfg(const Evaluation::CSpectrometer *spec, double alpha_min, double alpha_max, double phi, double beta, bool flat);

		/** Retrieves the parameters for the scan that we want to make
				for a scanner where the wind-direction is given by 'windDirection' and the 
					centre of mass of the last scan is found at the scanAngle 'alpha_cm' */
		static bool GetParameters(double windDirection, const CGPSData &scannerPos, const CString &source, double minAngle, double alpha_cm, double &alpha_min, double &alpha_max, double &phi_source, double &beta);
	};
}