#pragma once

#include <Afxtempl.h>

namespace Evaluation
{
	const enum CORRECTION{
		TEMPERATURE_SLF_VER1 // Correcting for the changing slit-function with temperature, version 1
	};
	/** The class <b>CColumnCorrection</b> holds parameters to describe
			what corrections have been applied to a retrieved slant column and in which order. */
	class CColumnCorrection
	{
	public:
		CColumnCorrection(void);
		~CColumnCorrection(void);

		/** The list of corrections and their versions */
		CList <CORRECTION, CORRECTION&> m_corrections;

		/** The actual correction-factors */
		CList <double, double&> m_correctionValues;

		// ------------------------------------------------------------------
		// ---------- Calculation of all available corrections --------------
		// ------------------------------------------------------------------
		static double GetCorrectionFactor(CORRECTION correction, double *parameters, long nParameters);
	};
}