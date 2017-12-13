#include "StdAfx.h"
#include "SpectrometerModel.h"


CSpectrometerModel::CSpectrometerModel(void)
{
}

CSpectrometerModel::~CSpectrometerModel(void)
{
}

/** Retrieves the maximum intensity for the supplied spectrometer model */
double	CSpectrometerModel::GetMaxIntensity(const CString modelNumber){
	return GetMaxIntensity(GetModel(modelNumber));
}
double  CSpectrometerModel::GetMaxIntensity(const SPECTROMETER_MODEL modelNumber){
	switch(modelNumber){
		case S2000:		return 4095;
		case USB2000: return 4095;
		case USB4000:	return 65535;
		case HR2000:	return 4095;
		case HR4000:	return 16535;
		case QE65000:	return 65535;
		case MAYAPRO:	return 65535;
		default:	return 4095;
	}
}

/** Converts a SPECTROMETER_MODEL to a string item */
RETURN_CODE CSpectrometerModel::ToString(SPECTROMETER_MODEL model, CString &str){
	if(S2000 == model){
		str.Format("S2000");
		return SUCCESS;
	}
	if(USB2000 == model){
		str.Format("USB2000");
		return SUCCESS;
	}
	if(USB4000 == model){
		str.Format("USB4000");
		return SUCCESS;
	}
	if(HR2000 == model){
		str.Format("HR2000");
		return SUCCESS;
	}
	if(HR4000 == model){
		str.Format("HR4000");
		return SUCCESS;
	}
	if(QE65000 == model){
		str.Format("QE65000");
		return SUCCESS;
	}
	if(MAYAPRO == model) {
		str.Format("MAYAPRO");
		return SUCCESS;
	}

	str.Format("Unknown");
	return FAIL;
}

/** Converts a string item to a SPECTROMETER_MODEL */
SPECTROMETER_MODEL CSpectrometerModel::GetModel(const CString &str){
	if(Equals(str, "S2000"))
		return S2000;
	if(Equals(str, "USB2000"))
		return USB2000;
	if(Equals(str, "HR2000"))
		return HR2000;
	if(Equals(str, "HR4000"))
		return HR4000;
	if(Equals(str, "USB4000"))
		return USB4000;
	if(Equals(str, "QE65000"))
		return QE65000;
	if (Equals(str, "MAYAPRO"))
		return MAYAPRO;

	// not defined
	return UNKNOWN_SPECTROMETER;
}

int	CSpectrometerModel::GetNumSpectrometerModels(){
	return NUM_CONF_SPEC_MODELS;
}
