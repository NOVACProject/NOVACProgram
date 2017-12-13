#pragma once

#include "Common.h"

/** The class <b>CSpectrometerModel</b> contains a collection of the 
		the spectrometer types/models that can be connected with the scanning
		instruments. */

enum SPECTROMETER_MODEL {
	S2000, 
	USB2000, 
	USB4000, 
	HR2000,
	HR4000, 
	QE65000, 
	MAYAPRO,
	UNKNOWN_SPECTROMETER, 
	NUM_CONF_SPEC_MODELS // the number of spectrometers that are configured
};

class CSpectrometerModel
{
public:
	CSpectrometerModel(void);
	~CSpectrometerModel(void);

	/** Retrieves the maximum intensity for the supplied spectrometer model */
	static double	GetMaxIntensity(const CString modelNumber);
	static double	GetMaxIntensity(const SPECTROMETER_MODEL modelNumber);

	/** Converts a SPECTROMETER_MODEL to a string item */
	static RETURN_CODE		ToString(SPECTROMETER_MODEL model, CString &str);

	/** Converts a string item to a SPECTROMETER_MODEL */
	static SPECTROMETER_MODEL GetModel(const CString &str);

	/** Gets the number of configured spectrometer models */
	static int	GetNumSpectrometerModels();

};
