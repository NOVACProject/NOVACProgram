// BasicMath.cpp: implementation of the CBasicMath class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "../resource.h"
#include "BasicMath.h"
#include "../SpectralEvaluation/Fit/GaussFunction.h"
#include "../SpectralEvaluation/Fit/CubicSplineFunction.h"
#include "../SpectralEvaluation/Fit/StandardMetricFunction.h"
#include "../SpectralEvaluation/Fit/PolynomialFunction.h"
#include "../SpectralEvaluation/Fit/DiscreteFunction.h"
#include "../SpectralEvaluation/Fit/StandardFit.h"
#include "../SpectralEvaluation/Fit/DOASVector.h"
#include <math.h>
#include <vector>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

using namespace MathFit;

#pragma warning (push, 3)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

bool CBasicMath::mDoNotUseMathLimits = false;

CBasicMath::CBasicMath()
{
}

CBasicMath::~CBasicMath()
{

}

double* CBasicMath::LowPassBinomial(double *fData, int iSize, int iNIterations)
{
	std::vector<double> fBuffer(iSize);
	double *fOut = fData;
	double *fIn = fBuffer.data();
	const int iLast = iSize - 1;
	const int iFirst = 0;

	int j, i;
	for(j = 0; j < iNIterations; j++)
	{
		// now swap buffers
		double *fTemp = fIn;
		fIn = fOut;
		fOut = fTemp;

		for(i = iFirst; i < iSize; i++)
		{
			double lMid, lLeft, lRight;

			lMid = fIn[i];
			if(i == iFirst)
				lLeft = fIn[i];
			else
				lLeft = fIn[i - 1];

			if(i == iLast)
				lRight = fIn[i];
			else
				lRight = fIn[i + 1];
			fOut[i] = 0.5 * lMid + 0.25 * lLeft + 0.25 * lRight;
		}
	}
	if(fOut != fData) {
		memcpy(fData, fOut, sizeof(double) * iSize);
    }
    
	return(fData);
}

/*void CBasicMath::LowPassBinomial(ISpectrum& dispData, int iNIterations)
{
	CDoubleMonitoredArrayData dmadData(dispData.Data);

	int iMeaStart, iMeaEnd, iSize;

	// calculate the real size
	iSize = CheckLimits(dispData, iMeaStart, iMeaEnd);

	double* fData = dmadData.GetBuffer();
	LowPassBinomial(&fData[iMeaStart], iSize, iNIterations);

	dispData.Data = dmadData.GetMonitoredArray();

	AddHistory(dispData, LOWPASS);
}*/

double* CBasicMath::HighPassBinomial(double *fData, int iSize, int iNIterations)
{
	double *fBuffer = new double[iSize];
	int i;

	// create copy of original data
	memcpy(fBuffer, fData, sizeof(double) * iSize);

	// create low pass filtered data
	LowPassBinomial(fBuffer, iSize, iNIterations);

	// remove low pass part from data
	for(i = 0; i < iSize; i++)
	{
		if(fBuffer[i] != 0.0)
			fData[i] /= fBuffer[i];
		else
			fData[i] = 0;
	}

	// free the buffer
	delete[] fBuffer;

	return(fData);
}

/*void CBasicMath::HighPassBinomial(ISpectrum& dispData, int iNIterations)
{
	CDoubleMonitoredArrayData dmadData(dispData.Data);
	double* fData = dmadData.GetBuffer();

	int iMeaStart, iMeaEnd, iSize;

	// calculate the real size
	iSize = CheckLimits(dispData, iMeaStart, iMeaEnd);

	HighPassBinomial(&fData[iMeaStart], iSize, iNIterations);

	dispData.Data = dmadData.GetMonitoredArray();

	AddHistory(dispData, HIGHPASS);
}*/

double* CBasicMath::Log(double *fData, int iSize)
{
	int i;

	for(i = 0; i < iSize; i++)
		fData[i] = fData[i] <= 0 ? 0.0 : log(fData[i]);
	return(fData);
}

/*void CBasicMath::Log(ISpectrum& dispData)
{
	CDoubleMonitoredArrayData dmadData(dispData.Data);
	double* fData = dmadData.GetBuffer();

	int iMeaStart, iMeaEnd, iSize;

	// calculate the real size
	iSize = CheckLimits(dispData, iMeaStart, iMeaEnd);

	Log(&fData[iMeaStart], iSize);

	dispData.Data = dmadData.GetMonitoredArray();

	AddHistory(dispData, LOGSPEC);
}*/

double* CBasicMath::Delog(double *fData, int iSize)
{
	int i;

	for(i = 0; i < iSize; i++)
		fData[i] = exp(fData[i]);
	return(fData);
}

/*void CBasicMath::Delog(ISpectrum& dispData)
{
	CDoubleMonitoredArrayData dmadData(dispData.Data);
	double* fData = dmadData.GetBuffer();

	int iMeaStart, iMeaEnd, iSize;

	// calculate the real size
	iSize = CheckLimits(dispData, iMeaStart, iMeaEnd);

	Delog(&fData[iMeaStart], iSize);

	dispData.Data = dmadData.GetMonitoredArray();

	AddHistory(dispData, EXPSPEC);
}*/

double* CBasicMath::CalcMeasuredSpec(double* fRes, double* fMea, int iMeaScans, double fMeaExpTime, double* fLamp, int iLampScans, double fLampExpTime, double* fBack, int iBackScans, double fBackExpTime, double fOffset, double fOffsetExpTime, int iSize)
{
	if(iMeaScans == 0 || iBackScans == 0 || iLampScans == 0)
		return(fRes);

	int i;
	if(fBackExpTime == fOffsetExpTime)
		fBackExpTime += 1.0;
	double fTau = (fMeaExpTime - fOffsetExpTime) / (fBackExpTime - fOffsetExpTime);

	double fMin = 1;
	for(i = 0; i < iSize; i++)
	{
		double fB = fTau * ((fBack[i] / iBackScans) - fOffset);
		double fM = (fMea[i] / iMeaScans) - fOffset;
		double fL = (fLamp[i] / iLampScans) - fOffset;

		fRes[i] = (fM - fB) / (fL != 0 ? fL : 1.0);
		if(fRes[i] < fMin)
			fMin = fRes[i];
	}

	// if we have negative values or zero values, move the whole spectrum into the positive space
	if(fMin <= 0)
	{
		// generate positive offset
		fMin = -fMin;

		// make sure to be non zero
		fMin += 1;

		for(i = 0; i < iSize; i++)
			fRes[i] += fMin;
	}

	return(fRes);
}

/*void CBasicMath::CalcMeasuredSpec(ISpectrum& dispRes, ISpectrum& dispMea, ISpectrum& dispLamp, ISpectrum& dispBack, double fOffset, double fOffsetExpTime)
{
	int iSize = min(dispBack.GetNChannel(), min(dispMea.GetNChannel(), dispLamp.GetNChannel()));

	// create the result array
	CDoubleMonitoredArrayData dmadMea(dispMea.Data);
	CDoubleMonitoredArrayData dmadLamp(dispLamp.Data);
	CDoubleMonitoredArrayData dmadBack(dispBack.Data);
	CDoubleMonitoredArrayData dmadRes(iSize);

	int iMeaStart, iMeaEnd, iNewSize;

	// calculate the real size
	iNewSize = min(CheckLimits(dispMea, iMeaStart, iMeaEnd), iSize);

	double* fRes = dmadRes.GetBuffer();
	double* fLamp = dmadLamp.GetBuffer();
	double* fBack = dmadBack.GetBuffer();
	double* fMea = dmadMea.GetBuffer();

	CalcMeasuredSpec(&fRes[iMeaStart], &fMea[iMeaStart], dispMea.GetNumScans(), dispMea.GetExposureTime(), &fLamp[iMeaStart], dispLamp.GetNumScans(), dispLamp.GetExposureTime(), &fBack[iMeaStart], dispBack.GetNumScans(), dispBack.GetExposureTime(), fOffset, fOffsetExpTime, iNewSize);

	dispRes.Data = dmadRes.GetMonitoredArray();

	AddHistory(dispRes, CALCAIR);
}*/

double CBasicMath::CalcExposureTime(double fSaturation, double fEOffsetExpTime, double fCurrentExpTime, double fCurrentAvg, double fBackExpTime, double fBackAvg, double fIntTime)
{
	// if no spec given reduce time 
	if(fCurrentAvg == 0)
		return(fCurrentExpTime * 0.8);

	// the currently used exposure time is the exposure time without the electronic background exposure time
	double fCurExpTime = std::max(fCurrentExpTime - fEOffsetExpTime, 0.0);

	// we need only the average of the real measured signal level without the background
	// ensure the difference is at least 1 unit big, so we switch to maximum exposure time
	double fDiffAvg = fabs(fCurrentAvg - fBackAvg) + 1;

	// ratio of current signal average and desired signal average
	double fSignalRatio = fSaturation * 65536.0 / fDiffAvg;

	// the new exposure time is the sum of the electronic offset exposure time and 
	// the old exposure time derived from the ratio calculated
	double fRes = std::max((fCurExpTime * fSignalRatio) + fEOffsetExpTime, 0.0);

	if(fIntTime > 0)
		return(std::min(fRes, fIntTime));
	else
		return(fRes);
}

/*double CBasicMath::CalcExposureTime(ISpectrum& dispMea, ISpectrum& dispBack, double fSaturation, double fIntTime)
{
	return(CalcExposureTime(fSaturation, 0, dispMea.GetExposureTime(), dispMea.GetAverage() / dispMea.GetNumScans(), dispBack.GetExposureTime(), dispBack.GetAverage() / dispBack.GetNumScans(), fIntTime));
}*/

/*
 * CheckLimits
 *
 * Checks for valid math limit settings and returns the array boundaries as zero based
 * indices as well as the number of elements contained in the selected array.
 */
/*int CBasicMath::CheckLimits(ISpectrum &dispSpec, int &iLowLimit, int &iHighLimit)
{
	int iSize = dispSpec.GetNChannel();

	// no limits wanted, so no limts set
	if(mDoNotUseMathLimits)
	{
		iLowLimit = 0;
		iHighLimit = iSize - 1;
		return(iSize);
	}

	// check math limits
	iLowLimit = (int)min(dispSpec.GetMathLow(), iSize - 1);
	if(iLowLimit < 0)
		iLowLimit = 0;

	iHighLimit = (int)min(dispSpec.GetMathHigh(), iSize - 1);
	if(iHighLimit < 0)
		iHighLimit = 0;

	if(iLowLimit >= iHighLimit)
		iHighLimit = iLowLimit + 1;

	// calculate the real size
	return(min(iHighLimit - iLowLimit + 1, iSize));
}*/

void CBasicMath::BiasAdjust(double *fData, int iSize)
{
	double fAverage;
	int i;

	// get average value
	for(fAverage = i = 0; i < iSize; i++)
		fAverage += fData[i];
	fAverage /= (double)iSize;

	// remove average
	for(i = 0; i < iSize; i++)
		fData[i] -= fAverage;
}

/*void CBasicMath::BiasAdjust(ISpectrum &dispSpec)
{
	CDoubleMonitoredArrayData dmadData(dispSpec.Data);
	double* fData = dmadData.GetBuffer();

	int iMeaStart, iMeaEnd, iSize;

	// calculate the real size
	iSize = CheckLimits(dispSpec, iMeaStart, iMeaEnd);

	BiasAdjust(&fData[iMeaStart], iSize);

	dispSpec.Data = dmadData.GetMonitoredArray();

	AddHistory(dispSpec, BIASADJUST);
}
*/
void CBasicMath::Zero(double *fData, int iSize, double fZeroLimit)
{
	int i;

	fZeroLimit = fabs(fZeroLimit);
	for(i = 0; i < iSize; i++)
		if(fabs(fData[i]) < fZeroLimit)
			fData[i] = 0;
}

/*void CBasicMath::Zero(ISpectrum &dispSpec, double fZeroLimit)
{
	CDoubleMonitoredArrayData dmadData(dispSpec.Data);
	double* fData = dmadData.GetBuffer();

	int iMeaStart, iMeaEnd, iSize;

	// calculate the real size
	iSize = CheckLimits(dispSpec, iMeaStart, iMeaEnd);

	Zero(&fData[iMeaStart], iSize, fZeroLimit);

	dispSpec.Data = dmadData.GetMonitoredArray();

	AddHistory(dispSpec, ZEROSPEC);
}*/

void CBasicMath::NormalizeAmplitude(double *fData, int iSize, double fMaxAmplitude)
{
	int i;
	double fMax;

	// search pivot
	fMax = fData[0];
	for(i = 1; i < iSize; i++)
		fMax = std::max(fMax, fabs(fData[i]));

	// nothing to do here
	if(fMax == 0)
		return;

	double fFactor = fMaxAmplitude / fMax;
	for(i = 0; i < iSize; i++)
		fData[i] *= fFactor;
}

/*void CBasicMath::NormalizeAmplitude(ISpectrum &dispSpec, double fMaxAmplitude)
{
	CDoubleMonitoredArrayData dmadData(dispSpec.Data);
	double* fData = dmadData.GetBuffer();

	int iMeaStart, iMeaEnd, iSize;

	// calculate the real size
	iSize = CheckLimits(dispSpec, iMeaStart, iMeaEnd);

	NormalizeAmplitude(&fData[iMeaStart], iSize, fMaxAmplitude);

	dispSpec.Data = dmadData.GetMonitoredArray();

	AddHistory(dispSpec, NORMAMP);
}*/

void CBasicMath::NormalizeEnergy(double *fData, int iSize, double fEnergy)
{
	int i;
	double fCurEnergy;

	// get current energy
	for(fCurEnergy = i = 0; i < iSize; i++)
		fCurEnergy += fabs(fData[i]);

	// nothing to divide
	if(fCurEnergy == 0)
		return;

	// normalize energy level
	double fFactor = fEnergy / fCurEnergy;
	for(i = 0; i < iSize; i++)
		fData[i] *= fFactor;
}

/*void CBasicMath::NormalizeEnergy(ISpectrum &dispSpec, double fEnergy)
{
	CDoubleMonitoredArrayData dmadData(dispSpec.Data);
	double* fData = dmadData.GetBuffer();

	int iMeaStart, iMeaEnd, iSize;

	// calculate the real size
	iSize = CheckLimits(dispSpec, iMeaStart, iMeaEnd);

	NormalizeEnergy(&fData[iMeaStart], iSize, fEnergy);

	dispSpec.Data = dmadData.GetMonitoredArray();

	AddHistory(dispSpec, NORMENERGY);
}
*/
void CBasicMath::Invert(double *fData, int iSize)
{
	int i;

	for(i = 0; i < iSize; i++)
		fData[i] *= -1;
}

/*void CBasicMath::Invert(ISpectrum &dispSpec)
{
	CDoubleMonitoredArrayData dmadData(dispSpec.Data);
	double* fData = dmadData.GetBuffer();

	int iMeaStart, iMeaEnd, iSize;

	// calculate the real size
	iSize = CheckLimits(dispSpec, iMeaStart, iMeaEnd);

	Invert(&fData[iMeaStart], iSize);

	dispSpec.Data = dmadData.GetMonitoredArray();

	AddHistory(dispSpec, INVERT);
}*/

void CBasicMath::Reciprocal(double *fData, int iSize)
{
	int i;

	for(i = 0; i < iSize; i++)
		fData[i] = 1 / fData[i];
}

/*void CBasicMath::Reciprocal(ISpectrum &dispSpec)
{
	CDoubleMonitoredArrayData dmadData(dispSpec.Data);
	double* fData = dmadData.GetBuffer();

	int iMeaStart, iMeaEnd, iSize;

	// calculate the real size
	iSize = CheckLimits(dispSpec, iMeaStart, iMeaEnd);

	Reciprocal(&fData[iMeaStart], iSize);

	dispSpec.Data = dmadData.GetMonitoredArray();

	AddHistory(dispSpec, RECIPROC);
}*/

void CBasicMath::Convolute(double *fFirst, int iSize, double *fCore, int iCoreSize)
{
	int iCoreMid = iCoreSize / 2;
	int i, j;

	// backup the original data
	double* fBuffer = new double[iSize];
	memcpy(fBuffer, fFirst, iSize * sizeof(double));

	// process every pixel
	for(i = 0; i < iSize; i++)
	{
		fFirst[i] = 0;

		int iIndex = -iCoreMid;

		// convolute with core, boundaries will be constanstly extended
		for(j = 0; j < iCoreSize; j++, iIndex++)
		{
			// the index to the base data is the core index plus the base index
			int iRealIndex = iIndex + i;
			if(iRealIndex < 0)
				iRealIndex = 0;
			else if(iRealIndex >= iSize)
				iRealIndex = iSize - 1;

			fFirst[i] += fBuffer[iRealIndex] * fCore[j];
		}
	}
	delete[] fBuffer;
}

/*void CBasicMath::Convolute(ISpectrum &dispFirst, ISpectrum &dispCore)
{
	CDoubleMonitoredArrayData dmadData(dispFirst.Data);
	double* fData = dmadData.GetBuffer();

	CDoubleMonitoredArrayData dmadCore(dispCore.Data);
	double* fCore = dmadCore.GetBuffer();

	int iMeaStart, iMeaEnd, iSize;

	// calculate the real size
	iSize = CheckLimits(dispFirst, iMeaStart, iMeaEnd);

	// get core limits
	int iCoreStart, iCoreEnd, iCoreSize;
	iCoreSize = CheckLimits(dispCore, iCoreStart, iCoreEnd);

	Convolute(&fData[iMeaStart], iSize, &fCore[iCoreStart], iCoreSize);

	dispFirst.Data = dmadData.GetMonitoredArray();

	AddHistory(dispFirst, CONVOLUTION);
}
*/
void CBasicMath::Reverse(double *fData, int iSize)
{
	double* fBuffer = new double[iSize];
	memcpy(fBuffer, fData, iSize * sizeof(double));

	int i;
	for(i = 0; i < iSize; i++)
		fData[i] = fBuffer[iSize - i - 1];
	delete[] fBuffer;
}

/*void CBasicMath::Reverse(ISpectrum &dispData)
{
	CDoubleMonitoredArrayData dmadData(dispData.Data);
	double* fData = dmadData.GetBuffer();

	int iMeaStart, iMeaEnd, iSize;

	// calculate the real size
	iSize = CheckLimits(dispData, iMeaStart, iMeaEnd);

	Reverse(&fData[iMeaStart], iSize);

	dispData.Data = dmadData.GetMonitoredArray();

	AddHistory(dispData, REVERSE);
}*/

void CBasicMath::Add(double *fFirst, double *fSec, int iSize, double fFactor)
{
	if(fFactor != 0)
	{
		int i;
		for(i = 0; i < iSize; i++)
			fFirst[i] += fFactor * fSec[i];
	}
	else
	{
		int i;
		for(i = 0; i < iSize; i++)
			fFirst[i] += fSec[i];
	}
}

/*void CBasicMath::Add(ISpectrum &dispFirst, ISpectrum &dispSec, int iMode)
{
	CDoubleMonitoredArrayData dmadData(dispFirst.Data);
	double* fData = dmadData.GetBuffer();
	CDoubleMonitoredArrayData dmadSec(dispSec.Data);
	double* fSec = dmadSec.GetBuffer();

	int iMeaStart, iMeaEnd, iSize;

	// calculate the real size
	iSize = CheckLimits(dispFirst, iMeaStart, iMeaEnd);
	if(dispSec.GetNChannel() < iMeaEnd)
		return;

	double fCorrectFactor = GetCorrectFactor(dispFirst, dispSec, iMode);
	Add(&fData[iMeaStart], &fSec[iMeaStart], iSize, fCorrectFactor);

	dispFirst.Data = dmadData.GetMonitoredArray();

	AddHistory(dispFirst, ADDSPEC);
}*/

void CBasicMath::Add(double *fFirst, int iSize, double fConst)
{
	int i;
	for(i = 0; i < iSize; i++)
		fFirst[i] += fConst;
}

/*void CBasicMath::Add(ISpectrum &dispFirst, double fConst)
{
	CDoubleMonitoredArrayData dmadData(dispFirst.Data);
	double* fData = dmadData.GetBuffer();

	int iMeaStart, iMeaEnd, iSize;

	// calculate the real size
	iSize = CheckLimits(dispFirst, iMeaStart, iMeaEnd);

	Add(&fData[iMeaStart], iSize, fConst);

	dispFirst.Data = dmadData.GetMonitoredArray();

	AddHistory(dispFirst, ADDSCALAR);
}*/

void CBasicMath::Sub(double *fFirst, double *fSec, int iSize, double fFactor)
{
	if(fFactor != 0)
	{
		int i;
		for(i = 0; i < iSize; i++)
			fFirst[i] -= fFactor * fSec[i];
	}
	else
	{
		int i;
		for(i = 0; i < iSize; i++)
			fFirst[i] -= fSec[i];
	}
}

/*void CBasicMath::Sub(ISpectrum &dispFirst, ISpectrum &dispSec, int iMode)
{
	CDoubleMonitoredArrayData dmadData(dispFirst.Data);
	double* fData = dmadData.GetBuffer();
	CDoubleMonitoredArrayData dmadSec(dispSec.Data);
	double* fSec = dmadSec.GetBuffer();

	int iMeaStart, iMeaEnd, iSize;

	// calculate the real size
	iSize = CheckLimits(dispFirst, iMeaStart, iMeaEnd);
	if(dispSec.GetNChannel() < iMeaEnd)
		return;

	double fCorrectFactor = GetCorrectFactor(dispFirst, dispSec, iMode);
	Sub(&fData[iMeaStart], &fSec[iMeaStart], iSize, fCorrectFactor);

	dispFirst.Data = dmadData.GetMonitoredArray();

	AddHistory(dispFirst, SUBSPEC);
}
*/
void CBasicMath::Sub(double *fFirst, int iSize, double fConst)
{
	int i;
	for(i = 0; i < iSize; i++)
		fFirst[i] -= fConst;
}

/*void CBasicMath::Sub(ISpectrum &dispFirst, double fConst)
{
	CDoubleMonitoredArrayData dmadData(dispFirst.Data);
	double* fData = dmadData.GetBuffer();

	int iMeaStart, iMeaEnd, iSize;

	// calculate the real size
	iSize = CheckLimits(dispFirst, iMeaStart, iMeaEnd);

	Sub(&fData[iMeaStart], iSize, fConst);

	dispFirst.Data = dmadData.GetMonitoredArray();

	AddHistory(dispFirst, SUBSCALAR);
}
*/
void CBasicMath::Mul(double *fFirst, double *fSec, int iSize, double fFactor)
{
	if(fFactor != 0)
	{
		int i;
		for(i = 0; i < iSize; i++)
			fFirst[i] *= fFactor * fSec[i];
	}
	else
	{
		int i;
		for(i = 0; i < iSize; i++)
			fFirst[i] *= fSec[i];
	}
}

/*void CBasicMath::Mul(ISpectrum &dispFirst, ISpectrum &dispSec, int iMode)
{
	CDoubleMonitoredArrayData dmadData(dispFirst.Data);
	double* fData = dmadData.GetBuffer();
	CDoubleMonitoredArrayData dmadSec(dispSec.Data);
	double* fSec = dmadSec.GetBuffer();

	int iMeaStart, iMeaEnd, iSize;

	// calculate the real size
	iSize = CheckLimits(dispFirst, iMeaStart, iMeaEnd);
	if(dispSec.GetNChannel() < iMeaEnd)
		return;

	double fCorrectFactor = GetCorrectFactor(dispFirst, dispSec, iMode);
	Mul(&fData[iMeaStart], &fSec[iMeaStart], iSize, fCorrectFactor);

	dispFirst.Data = dmadData.GetMonitoredArray();

	AddHistory(dispFirst, MULSPEC);
}*/

void CBasicMath::Mul(double *fFirst, int iSize, double fConst)
{
	int i;
	for(i = 0; i < iSize; i++)
		fFirst[i] *= fConst;
}

/*void CBasicMath::Mul(ISpectrum &dispFirst, double fConst)
{
	CDoubleMonitoredArrayData dmadData(dispFirst.Data);
	double* fData = dmadData.GetBuffer();

	int iMeaStart, iMeaEnd, iSize;

	// calculate the real size
	iSize = CheckLimits(dispFirst, iMeaStart, iMeaEnd);

	Mul(&fData[iMeaStart], iSize, fConst);

	dispFirst.Data = dmadData.GetMonitoredArray();

	AddHistory(dispFirst, MULSCALAR);
}*/

void CBasicMath::Div(double *fFirst, double *fSec, int iSize, double fFactor)
{
	if(fFactor != 0)
	{
		int i;
		for(i = 0; i < iSize; i++)
			if(fSec[i] != 0)
				fFirst[i] /= fFactor * fSec[i];
			else
				fFirst[i] = 0;
	}
	else
	{
		int i;
		for(i = 0; i < iSize; i++)
			if(fSec[i] != 0)
				fFirst[i] /= fSec[i];
			else
				fFirst[i] = 0;
	}
}

/*void CBasicMath::Div(ISpectrum &dispFirst, ISpectrum &dispSec, int iMode)
{
	CDoubleMonitoredArrayData dmadData(dispFirst.Data);
	double* fData = dmadData.GetBuffer();
	CDoubleMonitoredArrayData dmadSec(dispSec.Data);
	double* fSec = dmadSec.GetBuffer();

	int iMeaStart, iMeaEnd, iSize;

	// calculate the real size
	iSize = CheckLimits(dispFirst, iMeaStart, iMeaEnd);
	if(dispSec.GetNChannel() < iMeaEnd)
		return;

	double fCorrectFactor = GetCorrectFactor(dispFirst, dispSec, iMode);
	Div(&fData[iMeaStart], &fSec[iMeaStart], iSize, fCorrectFactor);

	dispFirst.Data = dmadData.GetMonitoredArray();

	AddHistory(dispFirst, DIVSPEC);
}*/

void CBasicMath::Div(double *fFirst, int iSize, double fConst)
{
	if(fConst == 0)
		return;

	int i;
	for(i = 0; i < iSize; i++)
		fFirst[i] /= fConst;
}

/*void CBasicMath::Div(ISpectrum &dispFirst, double fConst)
{
	CDoubleMonitoredArrayData dmadData(dispFirst.Data);
	double* fData = dmadData.GetBuffer();

	int iMeaStart, iMeaEnd, iSize;

	// calculate the real size
	iSize = CheckLimits(dispFirst, iMeaStart, iMeaEnd);

	Div(&fData[iMeaStart], iSize, fConst);

	dispFirst.Data = dmadData.GetMonitoredArray();

	AddHistory(dispFirst, DIVSCALAR);
}*/

bool CBasicMath::ShiftAndSqueeze(CVector& vXData, CVector& vYData, double fOrigin, double fShift, double fSqueeze)
{
	MATHFIT_ASSERT(vXData.GetSize() == vYData.GetSize());

	bool bResult = false;

	try
	{
		CCubicSplineFunction cbfSpec(vXData, vYData);

		int i;
		for(i = 0; i < vXData.GetSize(); i++)
		{
			const double fXValue = (vXData.GetAt(i) - fOrigin) * fSqueeze + fShift + fOrigin;
			vYData.SetAt(i, cbfSpec.GetValue(fXValue));
		}

		bResult = true;
	}
	catch(CAssertFailedException e)
	{
		e.ReportError();
	}
	catch(CFitException e)
	{
	}

	return bResult;
}

/*bool CBasicMath::ShiftAndSqueeze(ISpectrum &dispSpec, double fShift, double fSqueeze)
{
	CDoubleMonitoredArrayData dmadData(dispSpec.Data);
	double* fData = dmadData.GetBuffer();

	int iMeaStart, iMeaEnd, iSize;

	// calculate the real size
	iSize = CheckLimits(dispSpec, iMeaStart, iMeaEnd);

	CVector vXData(dispSpec.GetNChannel());
	vXData.Wedge(0, 1);
	CVector vYData(fData, dispSpec.GetNChannel(), 1, false);

	bool bRet = ShiftAndSqueeze(vXData, vYData, vXData.GetAt(iMeaStart), fShift, fSqueeze);

	dispSpec.Data = dmadData.GetMonitoredArray();

	AddHistory(dispSpec, SHIFTSQUEEZE);

	return(bRet);
}*/

/**
 * FitRes2ppb
 *
 * Converts the result of the non linear dfit given in molecules/cm^2 into
 * ppb using the following formular:
 *
 * The gas formular gives us the number or molecules in the volume:
 * n = p * V / (Rm * T)
 * where Rm is the universal gas constant which is given by:
 * Rm = pn * Vmn / Tn
 * pn = 101.325 kPa
 * Vmn = 22.413e3 cm^3/mol
 * Tn = 273.15 K
 *
 * The number of molecules of the fit result is given by:
 * nfit = fit result * 1 cm^2 = fit result
 *
 * The number of molecules in the gas cylinder is given by:
 * ngas = n * avogadro
 * where avogadro is 6.022137e23
 *
 * So to get ppb (particles per billion) we now just build the releation between the
 * two amounts of molecules:
 *
 * ppb = nfit * 1e9 / ngas
 *
 * @param fFitResult		The concentration in molecules/cm^2
 * @param fLightPathLength	The length of the light path in cm
 * @param fTemperature		The temperature in Celcius degrees
 * @param fPreasure			The preasure in hPa
 *
 * @return the concentration in ppb
 */
double CBasicMath::FitRes2ppb(double fFitResult, double fLightPathLength, double fTemperature, double fPreasure)
{
	const double fRm = 101.325 * 22.413e3 / 273.15;	// universal gas constant
	const double fT = fTemperature * 273.15; // convert from Celcuis to Kelvin
	const double fP = fPreasure / 10;	// convert from hPa to kPa!
	const double fV = 1 * 1 * fLightPathLength; // normalized volume of cm^2 * light path length

	// get the mol number of the current gas cyclinder
	double fMol = fP * fV / (fRm * fT); 

	// determine the number of molcules from the fit divided by the 
	// whole number of molecules in the volume
	double fPPB = fFitResult / (fMol * 6.022137e14);
	return(fPPB);
}

/**
 * FitRes2MicroGrammPerCubicMeter
 *
 * Converts the result of the non linear dfit given in molecules/cm^2 into
 * µg/m^3 using the following formular:
 *
 * c = fit * mol.-weight * 1e6 / (light path length * avogadro * 1e-6)
 * where avogadro is 6.022137e23
 *
 * @param fFitResult		The concentration in molecules/cm^2
 * @param fLightPathLength	The length of the light path in cm
 * @param fMolecularWeight	The molecular weight in g/mol
 *
 * @return the concentration in µg/cm^3
 */
double CBasicMath::FitRes2MicroGrammPerCubicMeter(double fFitResult, double fLightPathLength, double fMolecularWeight)
{
	return(fFitResult * fMolecularWeight / (fLightPathLength * 6.022137e11));
}

/*void CBasicMath::PolyFill(ISpectrum &dispSpec, int iStartChannel, int iNumChannels, double *fPolyCoeff, int iPolyDegree)
{
	int iMaxChannel = dispSpec.GetNChannel();

	CDoubleMonitoredArrayData dmadData(iMaxChannel);
	double* fData = dmadData.GetBuffer();

	int i;
	for(i = 0; i < iNumChannels; i++)
	{
		int iChannel = iStartChannel + i - 1;

		if(iChannel < iMaxChannel)
			fData[iChannel] = CalcPoly(fPolyCoeff, iPolyDegree, iChannel);
		else
			break;
	}

	// write the new data set back to the spectrum
	dispSpec.Data = dmadData.GetMonitoredArray();

	AddHistory(dispSpec, POLYFILL);
}*/

/*double CBasicMath::GetCorrectFactor(ISpectrum &dispFirst, ISpectrum &dispSec, int iMode)
{
	switch(iMode)
	{
	case SCANWEIGHT:
		return((double)dispFirst.GetNumScans() / (double)dispSec.GetNumScans());
	case TIMEWEIGHT:
		return((double) dispFirst.GetNumScans() * (double)dispFirst.GetExposureTime() / ((double)dispSec.GetNumScans() * (double)dispSec.GetExposureTime()));
	default:
		return(0);
	}

	// should never get here!
	return(0);
}
*/
/*void CBasicMath::FillRandom(ISpectrum &dispSpec, double fVariance)
{
	CDoubleMonitoredArrayData dmadData(dispSpec.Data);
	double* fData = dmadData.GetBuffer();

	int iMeaStart, iMeaEnd, iSize;

	// calculate the real size
	iSize = CheckLimits(dispSpec, iMeaStart, iMeaEnd);

	FillRandom(&fData[iMeaStart], iSize, fVariance);

	dispSpec.Data = dmadData.GetMonitoredArray();

	AddHistory(dispSpec, RANDOM);
}
*/
void CBasicMath::FillRandom(double *fData, int iLength, double fVariance)
{
	int i;
	for(i = 0; i < iLength; i++)
		fData[i] = fVariance - 2 * fVariance * (double)rand() / (double)RAND_MAX;
}

/*void CBasicMath::FillGauss(ISpectrum &dispSpec, double fA, double fSigma)
{
	CDoubleMonitoredArrayData dmadData(dispSpec.Data);
	double* fData = dmadData.GetBuffer();

	int iMeaStart, iMeaEnd, iSize;

	// calculate the real size
	iSize = CheckLimits(dispSpec, iMeaStart, iMeaEnd);

	FillGauss(&fData[iMeaStart], iSize, fA, fSigma);

	dispSpec.Data = dmadData.GetMonitoredArray();

	AddHistory(dispSpec, GAUSS);
}
*/
void CBasicMath::FillGauss(double *fData, int iSize, double fA, double fSigma, double fScale)
{
	int i;
	for(i = 0; i < iSize; i++)
	{
		fData[i] = pow(i - fA, 2);
		fData[i] /= -2 * fSigma * fSigma;
		fData[i] = exp(fData[i]);
		if(fScale == 0)
			fData[i] /= fSigma * 2.506628275 /* := sqrt(2 * PI) */;
		else
			fData[i] *= fScale;
	}
}

/*
 * PolynomialFit
 *
 * Fits serveral polynomial to several given data sets at once.
 *
 * @param fXData	Points to an array containing the X values
 * @param iNXValues	The number of X values
 * @param fYData	Points to a matrix containing the Y values (must be of size [iNYCols][iNXValues])
 * @param fCoeff	Points to a buffer that receives the polynomial coefficients after the fit was successful (must be of size [iNYCols][iOrder + 1])
 * @param iOrder	Specifies the order of the polynomials to fit
 *
 * @return	TRUE if successful, FALSE if the fit fails.
 *
 * @author Stefan Kraus
 * @date 11.10.00
 * @version 1.0
 */
bool CBasicMath::PolynomialFit(double *fXData, int iNXValues, double *fYData, double *fCoeff, int iOrder)
{
	// convert to internal vector objects
	CVector vXData, vYData;
	vXData.Copy(fXData, iNXValues);
	vYData.Copy(fYData, iNXValues);

	// create target object
	CDiscreteFunction dataTarget;
	if(!dataTarget.SetData(vXData, vYData))
		return(false);

	// create polynomial object
	CPolynomialFunction cPoly(iOrder);

	// create difference object
	CStandardMetricFunction cDiff(dataTarget, cPoly);

	// create fit object and run fit
	CStandardFit cFit(cDiff);
	cFit.SetFitRange(vXData);

	try
	{
		// do the fitting
		if(!cFit.PrepareMinimize())
			return(false);
		if(!cFit.Minimize())
			return(false);
		cFit.FinishMinimize();

		// copy the polynomial coeffs back to the array given
		int i;
		for(i = 0; i <= iOrder; i++)
			fCoeff[i] = cPoly.GetLinearParameter().GetAt(i);
	}
	catch(CFitException exFit)
	{
		return(false);
	}

	return(true);
}

/*LPDISPATCH CBasicMath::PolynomialFit(ISpectrum &dispSpec, int iDegree)
{
	CDoubleMonitoredArrayData dmadData(dispSpec.Data);
	double* fData = dmadData.GetBuffer();

	int iMeaStart, iMeaEnd, iSize;

	// calculate the real size
	iSize = CheckLimits(dispSpec, iMeaStart, iMeaEnd);

	double* fXValue = new double[iSize];
	double* fYValue = new double[iSize];
	double* fPolyCoeff = new double[iDegree + 1];

	int i;
	for(i = 0; i < iSize; i++)
	{
		fXValue[i] = iMeaStart + i;
		fYValue[i] = fData[iMeaStart + i];
	}

	PolynomialFit(fXValue, iSize, fYValue, fPolyCoeff, iDegree);

	delete(fXValue);
	delete(fYValue);

	IDoasTools dispTools;
	COleException exExcep;
	if(!dispTools.CreateDispatch("WinDoasTools.DoasTools", &exExcep))
	{
		exExcep.ReportError();
		return(NULL);
	}

	LPDISPATCH lpSpec = dispTools.GetSpectrum("Polynomial Fit");
	ISpectrumPtr dispResPtr(lpSpec);
	ISpectrum& dispRes = *dispResPtr;

	// set the appropriate number of channels
	dispRes.PutNChannel(dispSpec.GetNChannel());

	memset(fData, 0, sizeof(double) * dispSpec.GetNChannel());
	for(i = iMeaStart; i <= iMeaEnd; i++)
		fData[i] = CalcPoly(fPolyCoeff, iDegree, i);
	delete(fPolyCoeff);

	dispRes.Data = dmadData.GetMonitoredArray();

	dispTools.SetCurrentSpectrum(dispResPtr.GetInterfacePtr());

	return(lpSpec);
}*/

void CBasicMath::CrossCorrelate(double *fFirst, int iLengthFirst, double *fSec, int iLengthSec)
{
	double* fResult = new double[iLengthFirst];

	int i;
	for(i = 1; i <= iLengthFirst; i++)
	{
		double fSum = 0.0;
		int iSampleCount = 0;

		int iStartIndexSec = std::max(-iLengthSec / 2, -i);	// we either have to start at the end of the second data array or we can start before the zero index of the first array
		int iStopIndexSec = std::min(iLengthSec / 2, iLengthFirst - i);	// we can go further than iLengthFirst
		int iFirstIndex = i + iStartIndexSec;
		
		iStartIndexSec += iLengthSec / 2;
		iStopIndexSec += iLengthSec / 2;

		int iSecIndex;
		for(iSecIndex = iStartIndexSec; iSecIndex < iStopIndexSec; iFirstIndex++, iSecIndex++)
			fSum += fFirst[iFirstIndex] * fSec[iSecIndex];

		fResult[i - 1] = fSum / (double)(iStopIndexSec - iStartIndexSec);
	}

	memcpy(fFirst, fResult, sizeof(double) * iLengthFirst);
	delete[] fResult;
}

/*void CBasicMath::CrossCorrelate(ISpectrum &dispFirst, ISpectrum &dispSec)
{
	CDoubleMonitoredArrayData dmadData(dispFirst.Data);
	double* fData = dmadData.GetBuffer();
	CDoubleMonitoredArrayData dmadSec(dispSec.Data);
	double* fSec = dmadSec.GetBuffer();

	int iMeaStart, iMeaEnd, iSize;

	// calculate the real size
	iSize = CBasicMath::CheckLimits(dispFirst, iMeaStart, iMeaEnd);

	int iSecMeaStart, iSecMeaEnd, iSecSize;

	// calculate the real size
	iSecSize = CBasicMath::CheckLimits(dispSec, iSecMeaStart, iSecMeaEnd);

	CrossCorrelate(&fData[iMeaStart], iSize, &fSec[iSecMeaStart], iSecSize);

	dispFirst.Data = dmadData.GetMonitoredArray();

	AddHistory(dispFirst, CROSSCORREL);
}*/

/*
 * GaussFit
 *
 * Fits a Gauss Bell function the given data sets at once.
 *
 * @param fXData	Points to an array containing the X values
 * @param iNXValues	The number of X values
 * @param fYData	Points to a matrix containing the Y values (must be of size [iNYCols][iNXValues])
 * @param fCoeff	Points to a buffer that receives the polynomial coefficients after the fit was successful (must be of size [iNYCols][iOrder + 1])
 * @param iOrder	Specifies the order of the polynomials to fit
 *
 * @return	TRUE if successful, FALSE if the fit fails.
 *
 * @author Stefan Kraus
 * @date 11.10.00
 * @version 1.0
 */
bool CBasicMath::GaussFit(double *fXData, int iNXValues, double *fYData, double& fCenter, double& fSigma, double& fScale)
{
	// convert to internal vector objects
	CVector vXData, vYData;
	vXData.Copy(fXData, iNXValues);
	vYData.Copy(fYData, iNXValues);

	// create target object
	CDiscreteFunction dataTarget;
	if(!dataTarget.SetData(vXData, vYData))
		return(false);

	if(fCenter == 0)
		fCenter = (vXData.GetAt(0) + vXData.GetAt(vXData.GetSize() - 1)) / 2;
	if(fSigma == 0)
		fSigma = 1;
	if(fScale == 0)
		fScale = 1;

	// create polynomial object
	CGaussFunction cGauss;
	cGauss.SetCenter(fCenter);
	cGauss.SetSigma(fSigma);
	cGauss.SetScale(fScale);

	// create difference object
	CStandardMetricFunction cDiff(dataTarget, cGauss);

	// create fit object and run fit
	CStandardFit cFit(cDiff);

	try
	{
		// do the fitting
		if(!cFit.PrepareMinimize())
			return(false);
		if(!cFit.Minimize())
			return(false);
		cFit.FinishMinimize();

		fCenter = cGauss.GetCenter();
		fSigma = cGauss.GetSigma();
		fScale = cGauss.GetScale();
	}
	catch(CFitException exFit)
	{
		return(false);
	}

	return(true);
}

/*LPDISPATCH CBasicMath::GaussFit(ISpectrum &dispSpec)
{
	CDoubleMonitoredArrayData dmadData(dispSpec.Data);
	double* fData = dmadData.GetBuffer();

	int iMeaStart, iMeaEnd, iSize;

	// calculate the real size
	iSize = CheckLimits(dispSpec, iMeaStart, iMeaEnd);

	double* fXValue = new double[iSize];
	double* fYValue = new double[iSize];

	int i;
	for(i = 0; i < iSize; i++)
	{
		fXValue[i] = iMeaStart + i;
		fYValue[i] = fData[iMeaStart + i];
	}

	// do the gauss fit
	double fCenter;
	int iMarker = (int)dispSpec.GetMarker() - 1;
	if(iMarker <= iMeaStart || iMarker >= iMeaEnd)
		fCenter = (iMeaStart + iMeaEnd) / 2;
	else
		fCenter = iMarker;
	double fSigma = 1;
	double fScale = 1;
	GaussFit(fXValue, iSize, fYValue, fCenter, fSigma, fScale);

	delete(fXValue);
	delete(fYValue);

	// create new spectrum
	IDoasTools dispTools;
	COleException exExcep;
	if(!dispTools.CreateDispatch("WinDoasTools.DoasTools", &exExcep))
	{
		exExcep.ReportError();
		return(NULL);
	}

	LPDISPATCH lpSpec = dispTools.GetSpectrum("Gauss Fit");
	ISpectrumPtr dispResPtr(lpSpec);
	ISpectrum& dispRes = *dispResPtr;

	// fill with new parameters
	FillGauss(fData, iSize, fCenter, fSigma, fScale);

	dispRes.Data = dmadData.GetMonitoredArray();

	dispTools.SetCurrentSpectrum(dispResPtr.GetInterfacePtr());

	return(lpSpec);
}*/

/**
 * dfour1
 *
 * Build the fourier transform or the inverse fourier transform of a given funtcion.
 * The data array will contain the result of the transform.
 * (C) Copr. 1986-92 Numerical Recipes Software. p. 502-510
 *
 * @param	data	Array that contains either 2*nn real values of the real function
 *					or nn complex values of the frequency function
 * @param	nn		The number of values in the array, must be an integer power of 2
 * @param	isign	A value of 1 indicates to do the fourier transfor, set to -1 of inverse.
 */
void CBasicMath::DFourier(double data[], unsigned long nn, int isign)
{
	unsigned long n,mmax,m,j,istep,i;
	double wtemp,wr,wpr,wpi,wi,theta;
	double tempr,tempi;
#define SWAP(a,b) { tempr = (a); (a) = (b); (b) = tempr; }

	n=nn << 1;
	j=1;
	for (i=1;i<n;i+=2) {
		if (j > i) {
			SWAP(data[j],data[i]);
			SWAP(data[j+1],data[i+1]);
		}
		m=n >> 1;
		while (m >= 2 && j > m) {
			j -= m;
			m >>= 1;
		}
		j += m;
	}
	mmax=2;
	while (n > mmax) {
		istep=mmax << 1;
		theta=isign*(6.28318530717959/mmax);
		wtemp=sin(0.5*theta);
		wpr = -2.0*wtemp*wtemp;
		wpi=sin(theta);
		wr=1.0;
		wi=0.0;
		for (m=1;m<mmax;m+=2) {
			for (i=m;i<=n;i+=istep) {
				j=i+mmax;
				tempr=wr*data[j]-wi*data[j+1];
				tempi=wr*data[j+1]+wi*data[j];
				data[j]=data[i]-tempr;
				data[j+1]=data[i+1]-tempi;
				data[i] += tempr;
				data[i+1] += tempi;
			}
			wr=(wtemp=wr)*wpr-wi*wpi+wr;
			wi=wi*wpr+wtemp*wpi+wi;
		}
		mmax=istep;
	}
}
#undef SWAP

/*void CBasicMath::FFT(ISpectrum &dispSpec, ISpectrum &dispReal, ISpectrum &dispImaginary)
{
	CDoubleMonitoredArrayData dmadData(dispSpec.Data);
	double* fData = dmadData.GetBuffer();
	CDoubleMonitoredArrayData dmadReal(dispReal.Data);
	double* fReal = dmadReal.GetBuffer();
	CDoubleMonitoredArrayData dmadImaginary(dispImaginary.Data);
	double* fImaginary = dmadImaginary.GetBuffer();

	int iMeaStart, iMeaEnd, iSize;

	iSize = dispSpec.GetNChannel();
	iMeaStart = 0;
	iMeaEnd = iSize - 1;
	if(dispSpec.GetNChannel() < iMeaEnd)
		return;

	FFT(&fData[iMeaStart], &fReal[iMeaStart], &fImaginary[iMeaStart], iSize);

	dispReal.Data = dmadReal.GetMonitoredArray();
	dispImaginary.Data = dmadImaginary.GetMonitoredArray();
}*/

void CBasicMath::FFT(double *fData, double *fReal, double *fImaginary, int iLength)
{
	double* fBuffer = new double[iLength * 2];

	memset(fReal, 0, iLength * sizeof(double));
	memset(fImaginary, 0, iLength * sizeof(double));
	memset(fBuffer, 0, iLength * 2 * sizeof(double));

	int i;
	for(i = 0; i < iLength; i++)
		fBuffer[i * 2] = fData[i];

	DFourier(fBuffer - 1, iLength, 1);

	for(i = 0; i < iLength / 2; i++)
	{
		fReal[i + iLength / 2] = fBuffer[i * 2];
		fImaginary[i + iLength / 2] = fBuffer[i * 2 + 1];
	}
	for(i = iLength / 2; i < iLength; i++)
	{
		fReal[i - iLength / 2] = fBuffer[i * 2];
		fImaginary[i - iLength / 2] = fBuffer[i * 2 + 1];
	}

	delete[] fBuffer;
}

void CBasicMath::InverseFFT(double* fReal, double* fImaginary, double* fData, int iLength)
{
	double* fBuffer = new double[iLength * 2];

	memset(fData, 0, sizeof(double) * iLength);

	int i;
	for(i = 0; i < iLength / 2; i++)
	{
		fBuffer[i * 2] = fReal[i + iLength / 2];
		fBuffer[i * 2 + 1] = fImaginary[i + iLength / 2];
	}
	for(i = iLength / 2; i < iLength; i++)
	{
		fBuffer[i * 2] = fReal[i - iLength / 2];
		fBuffer[i * 2 + 1] = fImaginary[i - iLength / 2];
	}

	DFourier(fBuffer - 1, iLength, -1);

	for(i = 0; i < iLength; i++)
		fData[i] = fBuffer[i * 2] / (double)iLength;

	delete[] fBuffer;
}

/*void CBasicMath::InverseFFT(ISpectrum& dispReal, ISpectrum& dispImaginary, ISpectrum& dispSpec)
{
	CDoubleMonitoredArrayData dmadData(dispSpec.Data);
	double* fData = dmadData.GetBuffer();
	CDoubleMonitoredArrayData dmadReal(dispReal.Data);
	double* fReal = dmadReal.GetBuffer();
	CDoubleMonitoredArrayData dmadImaginary(dispImaginary.Data);
	double* fImaginary = dmadImaginary.GetBuffer();

	int iMeaStart, iMeaEnd, iSize;

	iSize = dispSpec.GetNChannel();
	iMeaStart = 0;
	iMeaEnd = iSize - 1;
	if(dispSpec.GetNChannel() < iMeaEnd)
		return;

	InverseFFT(&fReal[iMeaStart], &fImaginary[iMeaStart], &fData[iMeaStart], iSize);

	dispSpec.Data = dmadData.GetMonitoredArray();
}
*/
/*void CBasicMath::FindStrongestAbsorption(ISpectrum &dispSpec, double fRange)
{
	CDoubleMonitoredArrayData dmadData(dispSpec.Data);
	double* fData = dmadData.GetBuffer();

	int iMeaStart, iMeaEnd, iSize;

	// calculate the real size
	iSize = CheckLimits(dispSpec, iMeaStart, iMeaEnd);

	// create vector object. Do NOT release the data array upon destruction of the vector
	CDOASVector vDOAS(&fData[iMeaStart], iSize, false);

	// find absorbtion line
	int iLeft, iMid, iRight;
	vDOAS.FindStrongestAbsorption(iLeft, iMid, iRight, fRange);

	dispSpec.OpticalDensityLeft = iLeft + iMeaStart;
	dispSpec.OpticalDensityCenter = iMid + iMeaStart;
	dispSpec.OpticalDensityRight = iRight + iMeaStart;
}*/

/*double CBasicMath::OpticalDensity(ISpectrum &dispSpec)
{
	CDoubleMonitoredArrayData dmadData(dispSpec.Data);
	double* fData = dmadData.GetBuffer();

	// create vector object. Do NOT release the data array upon destruction of the vector
	CDOASVector vDOAS(fData, dispSpec.GetNChannel(), false);

	// find absorbtion line
	double fRes = vDOAS.OpticalDensity((int)dispSpec.OpticalDensityLeft, (int)dispSpec.OpticalDensityCenter, (int)dispSpec.OpticalDensityRight);

	return(fRes);
}
*/

#pragma warning (pop)