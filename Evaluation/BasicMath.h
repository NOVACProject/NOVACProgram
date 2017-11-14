// BasicMath.h: interface for the CBasicMath class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BASICMATH_H__1DEB20E2_5D81_11D4_866C_00E098701FA6__INCLUDED_)
#define AFX_BASICMATH_H__1DEB20E2_5D81_11D4_866C_00E098701FA6__INCLUDED_

#include "../fit/Vector.h"
#include "../fit/FitException.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

using namespace MathFit;
//using namespace DoasCore;

class CBasicMath  
{
public:
//	double OpticalDensity(ISpectrum& dispSpec);
//	void FindStrongestAbsorption(ISpectrum& dispSpec, double fRange = 1);
	void FFT(double* fData, double* fReal, double* fImaginary, int iLength);
//	void FFT(ISpectrum& dispSpec, ISpectrum& dispReal, ISpectrum& dispImaginary);
	void InverseFFT(double* fReal, double* fImaginary, double* fData, int iLength);
//	void InverseFFT(ISpectrum& dispReal, ISpectrum& dispImaginary, ISpectrum& dispSpec);
	bool GaussFit(double *fXData, int iNXValues, double *fYData, double& fCenter, double& fSigma, double& fScale);
//	LPDISPATCH GaussFit(ISpectrum &dispSpec);
	bool PolynomialFit(double *fXData, int iNXValues, double *fYData, double *fCoeff, int iOrder);
//	LPDISPATCH PolynomialFit(ISpectrum &dispSpec, int iDegree);
	void CrossCorrelate(double *fFirst, int iLengthFirst, double *fSec, int iLengthSec);
//	void CrossCorrelate(ISpectrum &dispFirst, ISpectrum &dispSec);
	void FillGauss(double* fData, int iSize, double fA, double fSigma, double fScale = 0);
//	void FillGauss(ISpectrum& dispSpec, double fA, double fSigma);
	void FillRandom(double* fData, int iLength, double fVariance);
//	void FillRandom(ISpectrum& dispSpec, double fVariance);
	enum{ NOWEIGHT = 0, SCANWEIGHT, TIMEWEIGHT };

//	void PolyFill(ISpectrum& dispSpec, int iStartChannel, int iNumChannels, double* fPolyCoeff, int iPolyDegree);
	double FitRes2MicroGrammPerCubicMeter(double fFitResult, double fLightPathLength, double fMolecularWeight);
	double FitRes2ppb(double fFitResult, double fLightPathLength, double fTemperature, double fPreasure);
	bool ShiftAndSqueeze(CVector& vXData, CVector& vYData, double fOrigin, double fShift, double fSqueeze);
//	bool ShiftAndSqueeze(ISpectrum& dispSpec, double fShift, double fSqueeze);
	void Div(double* fFirst, int iSize, double fConst);
//	void Div(ISpectrum& dispFirst, double fConst);
	void Mul(double* fFirst, int iSize, double fConst);
//	void Mul(ISpectrum& dispFirst, double fConst);
	void Sub(double* fFirst, int iSize, double fConst);
//	void Sub(ISpectrum& dispFirst, double fConst);
	void Add(double* fFirst, int iSize, double fConst);
//	void Add(ISpectrum& dispFirst, double fConst);
	void Div(double* fFirst, double* fSec, int iSize, double fFactor = 0.0);
//	void Div(ISpectrum& dispFirst, ISpectrum& dispSec, int iMode = NOWEIGHT);
	void Mul(double* fFirst, double* fSec, int iSize, double fFactor = 0.0);
//	void Mul(ISpectrum& dispFirst, ISpectrum& dispSec, int iMode = NOWEIGHT);
	void Sub(double* fFirst, double* fSec, int iSize, double fFactor = 0.0);
//	void Sub(ISpectrum& dispFirst, ISpectrum& dispSec, int iMode = NOWEIGHT);
	void Add(double* fFirst, double* fSec, int iSize, double fFactor = 0.0);
//	void Add(ISpectrum& dispFirst, ISpectrum& dispSec, int iMode = NOWEIGHT);
//	void Reverse(ISpectrum& dispData);
	void Reverse(double* fData, int iSize);
//	void Convolute(ISpectrum& dispFirst, ISpectrum& dispCore);
	void Convolute(double* fFirst, int iSize, double* fCore, int iCoreSize);
//	void Invert(ISpectrum& dispSpec);
	void Invert(double* fData, int iSize);
//	void Reciprocal(ISpectrum& dispSpec);
	void Reciprocal(double* fData, int iSize);
//	void NormalizeEnergy(ISpectrum& dispSpec, double fEnergy);
	void NormalizeEnergy(double* fData, int iSize, double fEnergy);
//	void NormalizeAmplitude(ISpectrum& dispSpec, double fMaxAmplitude);
	void NormalizeAmplitude(double* fData, int iSize, double fMaxAmplitude);
//	void Zero(ISpectrum& dispSpec, double fZeroLimit);
	void Zero(double* fData, int iSize, double fZeroLimit);
	void BiasAdjust(double* fData, int iSize);
//	void BiasAdjust(ISpectrum& dispSpec);
//	void LowPassBinomial(ISpectrum& dispData, int iNIterations);
//	void HighPassBinomial(ISpectrum& dispData, int iNIterations);
//	void Log(ISpectrum& dispData);
//	void Delog(ISpectrum& dispData);
//	void CalcMeasuredSpec(ISpectrum& dispRes, ISpectrum& dispMea, ISpectrum& dispLamp, ISpectrum& dispBack, double fOffset, double fOffsetExpTime);
//	double CalcExposureTime(ISpectrum& dispMea, ISpectrum& dispBack, double fSaturation, double fIntTime);
	double CalcExposureTime(double fSaturation, double fEOffsetExpTime, double fCurrentExpTime, double fCurrentAvg, double fBackExpTime, double fBackAvg, double fIntTime);
	double* CalcMeasuredSpec(double* fRes, double* fMea, int iMeaScans, double fMeaExpTime, double* fLamp, int iLampScans, double fLampExpTime, double* fBack, int iBackScans, double fBackExpTime, double fOffset, double fOffsetExpTime, int iSize);
	double* Log(double* fData, int iSize);
	double* Delog(double* fData, int iSize);
	double* HighPassBinomial(double* fData, int iSize, int iNIterations);
	double* LowPassBinomial(double* fData, int iSize, int iNIterations);
//	static int CheckLimits(ISpectrum& dispSpec, int& iLowLimit, int& iHighLimit);
	CBasicMath();
	virtual ~CBasicMath();

private:
	void DFourier(double data[], unsigned long nn, int isign);
//	double GetCorrectFactor(ISpectrum& dispFirst, ISpectrum& dispSec, int iMode);
	static bool mDoNotUseMathLimits;
};

#endif // !defined(AFX_BASICMATH_H__1DEB20E2_5D81_11D4_866C_00E098701FA6__INCLUDED_)

