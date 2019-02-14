#pragma once

#include "../Common.h"
#include "../../SpectralEvaluation/Spectra/Spectrum.h"

namespace SpectrumIO
{

	/** <b>CSTDFile</b> is a simple class for reading/writing spectra 
			from/to .std-files */
	class CSTDFile
	{
	public:
		CSTDFile(void);
		~CSTDFile(void);

		/** Reads a spectrum from a STD-file */
		static RETURN_CODE ReadSpectrum(CSpectrum &spec, const CString &fileName);

		/** Writes a spectrum to a STD-file */
		static RETURN_CODE WriteSpectrum(const CSpectrum &spec, const CString &fileName, int extendedFormat = 0);

		/** Writes a spectrum to a STD-file */
		static RETURN_CODE WriteSpectrum(const CSpectrum *spec, const CString &fileName, int extendedFormat = 0);
	};
}