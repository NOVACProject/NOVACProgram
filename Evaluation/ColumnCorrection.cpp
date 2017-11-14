#include "StdAfx.h"
#include "columncorrection.h"

using namespace Evaluation;

CColumnCorrection::CColumnCorrection(void)
{
}

CColumnCorrection::~CColumnCorrection(void)
{
}


double CColumnCorrection::GetCorrectionFactor(CORRECTION correction, double *parameters, long nParameters){
	double correctionFactor;

	if(correction == TEMPERATURE_SLF_VER1){
		double T = parameters[0]; // this only takes one parameter, the temperature, in degrees Celsius
		
		correctionFactor = 0.0001468 * T * T - 0.0095221 * T +  1.1467;

		return correctionFactor;
	}else{
		return 1.0; // Default, unknown correction factor...
	}
}
