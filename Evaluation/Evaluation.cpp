// Evaluation.cpp: implementation of the CEvaluation class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "../NovacMasterProgram.h"
#include "Evaluation.h"
#include <iostream>
#include <conio.h>
// include all required fit objects
#include "../SpectralEvaluation/Fit/ReferenceSpectrumFunction.h"
#include "../SpectralEvaluation/Fit/SimpleDOASFunction.h"
#include "../SpectralEvaluation/Fit/StandardMetricFunction.h"
#include "../SpectralEvaluation/Fit/StandardFit.h"
#include "../SpectralEvaluation/Fit/ExpFunction.h"
#include "../SpectralEvaluation/Fit/LnFunction.h"
#include "../SpectralEvaluation/Fit/PolynomialFunction.h"
#include "../SpectralEvaluation/Fit/NegateFunction.h"
#include "../SpectralEvaluation/Fit/MulFunction.h"
#include "../SpectralEvaluation/Fit/DivFunction.h"
#include "../SpectralEvaluation/Fit/GaussFunction.h"
#include "../SpectralEvaluation/Fit/DiscreteFunction.h"
#include "../SpectralEvaluation/Fit/DOASVector.h"
#include "../SpectralEvaluation/Fit/NonlinearParameterFunction.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
// use the MathFit namesapce, since all fit objects are contained in this namespace
using namespace MathFit;
// also use the Evaluation namespace, since all datastructures for saving the results are contained there
using namespace Evaluation;

using namespace std;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEvaluation::CEvaluation()
{
	m_referenceNum = 1;

	for(int i = 0; i < MAX_N_REFERENCES; ++i)
		ref[i] = NULL;

	solarSpec = NULL;
	m_numberOfReferencesToUse = 0;
}

CEvaluation::CEvaluation(const CEvaluation &eval2){
	this->m_referenceNum = eval2.m_referenceNum;
	this->m_numberOfReferencesToUse = eval2.m_numberOfReferencesToUse;

	this->m_result = eval2.m_result;

	for(int i = 0; i < MAX_N_REFERENCES; ++i){
		m_crossSection[i] = eval2.m_crossSection[i];
	}
	
	m_solarSpectrumData = eval2.m_solarSpectrumData;
	this->vXData.Copy(eval2.vXData);
}

CEvaluation::~CEvaluation()
{
}


int CEvaluation::Evaluate(const CSpectrum &sky, const CSpectrum &measured, int numSteps){
	return Evaluate(sky, measured, m_window, numSteps);
}

/** Evaluate the supplied spectrum using the parameters set in the supplied fit window
*/
int CEvaluation::Evaluate(const CSpectrum &sky, const CSpectrum &meas, const CFitWindow &window, int numSteps) {
	CString message;
	int fitLow = window.fitLow;
	int fitHigh = window.fitHigh;

	// Check the fit region
	if (fitHigh < fitLow) {
		int tmp = fitLow;
		fitLow = fitHigh;
		fitHigh = tmp;
	}

	// The length of the sky spectrum (should be same as the length of all spectra)
	int specLen = sky.m_length;

	// Check so that the length of the spectra agree with each other
	if (specLen != meas.m_length)
		return(1);

	// If the spectra are longer than the references, then something is wrong!
	if (specLen > window.specLength)
		return(1);

	if (sky.m_info.m_startChannel != 0 || sky.m_length < window.specLength) {
		// Partial spectra
		fitLow -= sky.m_info.m_startChannel;
		fitHigh -= sky.m_info.m_startChannel;
		if (fitLow < 0 || fitHigh > sky.m_length)
			return 1;
	}

	// initialize the reference-spectrum functions
	for (int k = 0; k < MAX_N_REFERENCES; ++k)
		ref[k] = new CReferenceSpectrumFunction();

	// Vectors to store the data
	CVector vMeas, vSky;

	// Make local copies of the data
	double *measArray = (double *)calloc(meas.m_length, sizeof(double));
	memcpy(measArray, meas.m_data, meas.m_length * sizeof(double));

	_Post_ _Notnull_ double *skyArray = (double *)calloc(sky.m_length, sizeof(double));
	memcpy(skyArray, sky.m_data, sky.m_length * sizeof(double));

	//----------------------------------------------------------------
	// --------- prepare the spectrum for evaluation -----------------
	//----------------------------------------------------------------

	PrepareSpectra(skyArray, measArray, window);

	//----------------------------------------------------------------

	// Copy the measured spectrum to vMeas
	vMeas.Copy(measArray, specLen, 1);

	// To perform the fit we need to extract the wavelength (or pixel)
	//	information from the vXData-vector
	CVector vXSec(fitHigh - fitLow);
	vXSec.Copy(vXData.SubVector(fitLow, fitHigh - fitLow));

	////////////////////////////////////////////////////////////////////////////
	// now we start building the model function needed for fitting.
	//
	// First we create a function object that represents our measured spectrum. Since we do not
	// need any interpolation on the measured data its enough to use a CDiscreteFunction object.
	CDiscreteFunction dataTarget;

	// now set the data of the measured spectrum in regard to the wavelength information
	dataTarget.SetData(vXData.SubVector(window.startChannel, window.specLength), vMeas);

	// since the DOAS model function consists of the sum of all reference spectra and a polynomial,
	// we first create a summation object
	CSimpleDOASFunction cRefSum;

	// now we create the required CReferenceSpectrumFunction objects that actually represent the 
	// reference spectra used in the DOAS model function

	if (0 != CreateReferenceSpectrum(window, sky.m_info.m_startChannel)) {
		free(skyArray);
		free(measArray);
		for (int k = 0; k < MAX_N_REFERENCES; ++k)
			delete ref[k];
		return 1;
	}
	for (int i = 0; i < window.nRef; ++i) {
		cRefSum.AddReference(*ref[i]); // <-- at last add the reference to the summation object
	}

	// create the additional polynomial with the correct order
	//	and add it to the summation object, too
	CPolynomialFunction cPoly(window.polyOrder);
	cRefSum.AddReference(cPoly);

	// the last step in the model function will be to define how the difference between the measured data and the modeled
	// data will be determined. In this case we will use the CStandardMetricFunction which actually just calculate the difference
	// between the measured data and the modeled data channel by channel. The fit will try to minimize these differences.
	// So we create the metric object and set the measured spectrum function object and the DOAS model function object as parameters
	CStandardMetricFunction cDiff(dataTarget, cRefSum);

	/////////////////////////////////////////////////////////////////
	// Now its time to create the fit object. The CStandardFit object will 
	// provide a combination of a linear Least Square Fit and a nonlinear Levenberg-Marquardt Fit, which
	// should be sufficient for most needs.
	CStandardFit cFirstFit(cDiff);

	// don't forget to the the already extracted fit range to the fit object!
	// without a valid fit range you'll get an exception.
	cFirstFit.SetFitRange(vXSec);

	// limit the number of fit iteration to 5000. This can still take a long time! More convinient values are
	// between 100 and 1000
	cFirstFit.GetNonlinearMinimizer().SetMaxFitSteps(numSteps);
	cFirstFit.GetNonlinearMinimizer().SetMinChiSquare(0.0001);

	try
	{
		// prepare everything for fitting
		cFirstFit.PrepareMinimize();

		// actually do the fitting
		if (!cFirstFit.Minimize()) {
			message.Format("Fit Failed!");
			ShowMessage(message);
			free(skyArray);
			free(measArray);
			for (int k = 0; k < MAX_N_REFERENCES; ++k)
				delete ref[k];
			return 1;
		}

		// finalize the fitting process. This will calculate the error measurements and other statistical stuff
		cFirstFit.FinishMinimize();

		// get the basic fit results
		m_result.m_stepNum = (long)cFirstFit.GetFitSteps();
		m_result.m_chiSquare = (double)cFirstFit.GetChiSquare();
		m_result.m_referenceResult.resize(window.nRef);

		for (int tmpInt = 0; tmpInt <= window.polyOrder; ++tmpInt)
			m_result.m_polynomial[tmpInt] = (double)cPoly.GetCoefficient(tmpInt);

		// get residuum vector and expand it to a DOAS vector object. Do NOT assign the vector data to the new object!
		// display some statistical stuff about the residual data
		CDOASVector vResiduum;
		vResiduum.Attach(cFirstFit.GetResiduum(), false);
		m_residual.SetSize(vResiduum.GetSize());
		m_residual.Zero();
		m_residual.Add(vResiduum);

		m_result.m_delta = (double)vResiduum.Delta();

		// get the fitResult for the polynomial
		CVector tmpVector;
		tmpVector.SetSize(specLen);
		cPoly.GetValues(vXData.SubVector(window.startChannel, window.specLength), tmpVector);
		m_fitResult[0].Set(tmpVector, specLen);

		// finally display the fit results for each reference spectrum including their appropriate error
		for (int i = 0; i < window.nRef; i++)
		{
			m_result.m_referenceResult[i].m_specieName = std::string(window.ref[i].m_specieName);

			m_result.m_referenceResult[i].m_column          = (double)ref[i]->GetModelParameter(CReferenceSpectrumFunction::CONCENTRATION);
			m_result.m_referenceResult[i].m_columnError     = (double)ref[i]->GetModelParameterError(CReferenceSpectrumFunction::CONCENTRATION);
			m_result.m_referenceResult[i].m_shift           = (double)ref[i]->GetModelParameter(CReferenceSpectrumFunction::SHIFT);
			m_result.m_referenceResult[i].m_shiftError      = (double)ref[i]->GetModelParameterError(CReferenceSpectrumFunction::SHIFT);
			m_result.m_referenceResult[i].m_squeeze         = (double)ref[i]->GetModelParameter(CReferenceSpectrumFunction::SQUEEZE);
			m_result.m_referenceResult[i].m_squeezeError    = (double)ref[i]->GetModelParameterError(CReferenceSpectrumFunction::SQUEEZE);

			// get the final fit result
			CVector tmpVector;
			tmpVector.SetSize(specLen);
			ref[i]->GetValues(vXData.SubVector(window.startChannel, window.specLength), tmpVector);
			m_fitResult[i + 1].Set(tmpVector, specLen);
		}

		// clean up the evaluation
		free(skyArray);
		free(measArray);
		for (int k = 0; k < MAX_N_REFERENCES; ++k) {
			delete ref[k];
		}
		return (0);
	}
	catch (CFitException e)
	{
		// in case that something went wrong, display the error to the user.
		// normally you will get error in two cases:
		//
		// 1. You forgot to set a valid fit range before you start fitting
		//
		// 2. A matrix inversion failed for some reason inside the fitting loop. Matrix inversions
		//    normally fail when there are linear dependecies in the matrix respectrively you have linear
		//    dependencies in your reference spectrum. Eg. you tried to fit the same reference spectrum twice at once.

	//  e.ReportError();
	//	std::cout << "Failed: " << ++iFalseCount << std::endl;
	//	std::cout << "Steps: " << cFirstFit.GetNonlinearMinimizer().GetFitSteps() << " - Chi: " << cFirstFit.GetNonlinearMinimizer().GetChiSquare() << std::endl;

		message.Format("A Fit Exception has occured. Are the reference files OK?");
		ShowMessage(message);

		// clean up the evaluation
		free(skyArray);
		free(measArray);
		for (int k = 0; k < MAX_N_REFERENCES; ++k) {
			delete ref[k];
		}
		delete solarSpec;

		return (1);
	}
}

/** Evaluate the supplied spectrum using the solarReference found in 'window'
		@param measured - the spectrum for which to determine the shift & squeeze
				relative to the solarReference-spectrum found in 'window'
		@param window - the settings for the fit. The shift and squeeze between
				the measured spectrum and the solarReference-spectrum will be determined
				for the pixel-range 'window.fitLow' and 'window.fitHigh'
		@return 0 if the fit succeeds and the shift & squeeze could be determined
		@return 1 if any error occured. */
int CEvaluation::EvaluateShift(const CSpectrum &measured, const CFitWindow &window, double &shift, double &shiftError, double &squeeze, double &squeezeError){
	CString message;
	int i;
	CVector vMeas;
	CVector yValues;
	int fitLow	= window.fitLow;
	int fitHigh	= window.fitHigh;

	// Check the fit region
	if(fitHigh < fitLow){
		int tmp	= fitLow;
		fitLow	= fitHigh;
		fitHigh	= tmp;
	}

	// Check that we have a solar-spectrum to check against
	int specLen = measured.m_length;
	if(window.fraunhoferRef.m_path.size() < 6)
		return 1;

	// If the spectra are longer than the references, then something is wrong!
	if(specLen > window.specLength)
		return 1;

	if(measured.m_info.m_startChannel != 0 || measured.m_length < window.specLength){
		// Partial spectra
		fitLow	-= measured.m_info.m_startChannel;
		fitHigh	-= measured.m_info.m_startChannel;
		if(fitLow < 0 || fitHigh > measured.m_length)
			return 1;
	}

	// initialize the solar-spectrum function
	solarSpec = new CReferenceSpectrumFunction();

	// initialize the reference-spectrum functions
	for(unsigned int k = 0; k < MAX_N_REFERENCES; ++k)
		ref[k] = new CReferenceSpectrumFunction();

	// Make a local copy of the data
	double *measArray = (double *)calloc(measured.m_length, sizeof(double));
	memcpy(measArray, measured.m_data, measured.m_length*sizeof(double));

	// Make a local copy of the solar-spectrum, this for the filtering...
	double *solarArray = (double *)calloc(m_solarSpectrumData.GetSize(), sizeof(double));
	for(unsigned int k = 0; k < m_solarSpectrumData.GetSize(); ++k){
		solarArray[k] = m_solarSpectrumData.GetAt(k);
	}

	//----------------------------------------------------------------
	// --------- prepare the spectrum for evaluation -----------------
	//----------------------------------------------------------------

	RemoveOffset(measArray, window.specLength, window.UV);
	if(window.fitType == FIT_HP_DIV || window.fitType == FIT_HP_SUB){
		HighPassBinomial(measArray,	window.specLength,	500);
	}
	Log(measArray,window.specLength);

	// --------- also prepare the solar-spectrum for evaluation -----------------
	if(window.fitType == FIT_HP_DIV || window.fitType == FIT_HP_SUB){
		HighPassBinomial(solarArray,	window.specLength,	500);
	}
	Log(solarArray,	window.specLength);
	CVector localSolarSpectrumData;
	localSolarSpectrumData.SetSize(m_solarSpectrumData.GetSize());
	for(int k = 0; k < window.specLength; ++k){
		localSolarSpectrumData.SetAt(k, solarArray[k]);
	}

	//----------------------------------------------------------------

	// Copy the measured spectrum to vMeas
	vMeas.Copy(measArray,specLen,1);

	// To perform the fit we need to extract the wavelength (or pixel)
	//	information from the vXData-vector
	CVector vXSec(fitHigh - fitLow);
	vXSec.Copy(vXData.SubVector(fitLow, fitHigh - fitLow));

	////////////////////////////////////////////////////////////////////////////
	// now we start building the model function needed for fitting.
	//
	// First we create a function object that represents our measured spectrum. Since we do not
	// need any interpolation on the measured data its enough to use a CDiscreteFunction object.
	CDiscreteFunction dataTarget;

	// now set the data of the measured spectrum in regard to the wavelength information
	dataTarget.SetData(vXData.SubVector(window.startChannel, window.specLength), vMeas);
	
	// since the DOAS model function consists of the sum of all reference spectra and a polynomial,
	// we first create a summation object
	CSimpleDOASFunction cRefSum;

	// reset all reference's parameters
	solarSpec->ResetLinearParameter();
	solarSpec->ResetNonlinearParameter();

	// enable amplitude normalization. This should normally be done in order to avoid numerical
	// problems during fitting.
	solarSpec->SetNormalize(true);

	// set the spectral data of the reference spectrum to the object. This also causes an internal
	// transformation of the spectral data into a B-Spline that will be used to interpolate the 
	// reference spectrum during shift and squeeze operations
	if(!solarSpec->SetData(vXData.SubVector(0, localSolarSpectrumData.GetSize()), localSolarSpectrumData))
	{
		Error0("Error initializing spline object!");
		free(measArray);
		delete solarSpec;
		for(int k = 0; k < MAX_N_REFERENCES; ++k)
			delete ref[k];
		return(1);
	}

	// Set the column value to 1
	solarSpec->FixParameter(CReferenceSpectrumFunction::CONCENTRATION, 1.0 * solarSpec->GetAmplitudeScale());

	// Free the shift
	//solarSpec->SetParameterLimits(CReferenceSpectrumFunction::SHIFT,	(TFitData)-6.0, (TFitData)6.0, (TFitData)1e25);
	//solarSpec->FixParameter(CReferenceSpectrumFunction::SHIFT,	(TFitData)1.4);

	// Fix the squeeze
	solarSpec->FixParameter(CReferenceSpectrumFunction::SQUEEZE, (TFitData)1.0);

	// Add the solar-reference to the fit
	cRefSum.AddReference(*solarSpec); // <-- at last add the reference to the summation object

	// Also add all the 'normal' cross sections to the fit
	for(i = 0; i < window.nRef; ++i){
		// reset all reference's parameters
		ref[i]->ResetLinearParameter();
		ref[i]->ResetNonlinearParameter();

		// enable amplitude normalization. This should normally be done in order to avoid numerical
		// problems during fitting.
		ref[i]->SetNormalize(true);

		// set the spectral data of the reference spectrum to the object. This also causes an internal
		// transformation of the spectral data into a B-Spline that will be used to interpolate the 
		// reference spectrum during shift and squeeze operations
		yValues.SetSize(m_crossSection[i].GetSize());
		for(unsigned int k = 0; k < m_crossSection[i].GetSize(); ++k){
			yValues.SetAt(k, m_crossSection[i].GetAt(k));
		}
		if(!ref[i]->SetData(vXData.SubVector(0, m_crossSection[i].GetSize()), yValues))
		{
			Error0("Error initializing spline object!");
			free(measArray);
			delete solarSpec;
			for(int k = 0; k < MAX_N_REFERENCES; ++k)
				delete ref[k];
			return(1);
		}

		// Link the shift and squeeze to the solar-reference
		solarSpec->LinkParameter(CReferenceSpectrumFunction::SHIFT,	  *ref[i], CReferenceSpectrumFunction::SHIFT);
		solarSpec->LinkParameter(CReferenceSpectrumFunction::SQUEEZE, *ref[i], CReferenceSpectrumFunction::SQUEEZE);

		cRefSum.AddReference(*ref[i]); // <-- at last add the reference to the summation object
	}

	// create the additional polynomial with the correct order
	//	and add it to the summation object, too
	CPolynomialFunction cPoly(2);
	cRefSum.AddReference(cPoly);

	// the last step in the model function will be to define how the difference between the measured data and the modeled
	// data will be determined. In this case we will use the CStandardMetricFunction which actually just calculate the difference
	// between the measured data and the modeled data channel by channel. The fit will try to minimize these differences.
	// So we create the metric object and set the measured spectrum function object and the DOAS model function object as parameters
	CStandardMetricFunction cDiff(dataTarget, cRefSum);

	/////////////////////////////////////////////////////////////////
	// Now its time to create the fit object. The CStandardFit object will 
	// provide a combination of a linear Least Square Fit and a nonlinear Levenberg-Marquardt Fit, which
	// should be sufficient for most needs.
	CStandardFit cFirstFit(cDiff);

	// don't forget to the the already extracted fit range to the fit object!
	// without a valid fit range you'll get an exception.
	cFirstFit.SetFitRange(vXSec);

	// limit the number of fit iteration to 5000.
	cFirstFit.GetNonlinearMinimizer().SetMaxFitSteps(500);
	cFirstFit.GetNonlinearMinimizer().SetMinChiSquare(0.0001);

	try
	{
		// prepare everything for fitting
		cFirstFit.PrepareMinimize();

		// actually do the fitting
		if(!cFirstFit.Minimize()){
			message.Format("Fit Failed!");
			ShowMessage(message);
			free(measArray);
			delete solarSpec;
			for(int k = 0; k < MAX_N_REFERENCES; ++k)
				delete ref[k];
			return 1;
		}

		// finalize the fitting process. This will calculate the error measurements and other statistical stuff
		cFirstFit.FinishMinimize();

		// get the basic fit results
		long stepNum				= (long)cFirstFit.GetFitSteps();
		double chiSquare			= (double)cFirstFit.GetChiSquare();
		unsigned long speciesNum	= (unsigned long)window.nRef;

		// get residuum vector and expand it to a DOAS vector object. Do NOT assign the vector data to the new object!
		// display some statistical stuff about the residual data
		CDOASVector vResiduum;
		vResiduum.Attach(cFirstFit.GetResiduum(), false);
		m_residual.SetSize(vResiduum.GetSize());
		m_residual.Zero();
		m_residual.Add(vResiduum);

		m_result.m_delta = (double)vResiduum.Delta();

		// finally get the fit-result
		double column       = (double)solarSpec->GetModelParameter(CReferenceSpectrumFunction::CONCENTRATION);
		double columnError  = (double)solarSpec->GetModelParameterError(CReferenceSpectrumFunction::CONCENTRATION);
		shift               = (double)solarSpec->GetModelParameter(CReferenceSpectrumFunction::SHIFT);
		shiftError          = (double)solarSpec->GetModelParameterError(CReferenceSpectrumFunction::SHIFT);
		squeeze             = (double)solarSpec->GetModelParameter(CReferenceSpectrumFunction::SQUEEZE);
		squeezeError        = (double)solarSpec->GetModelParameterError(CReferenceSpectrumFunction::SQUEEZE);

#ifdef _DEBUG
		for(i = 0; i < window.nRef; ++i){
			double rcolumn              = (double)ref[i]->GetModelParameter(CReferenceSpectrumFunction::CONCENTRATION);
			double rcolumnError         = (double)ref[i]->GetModelParameterError(CReferenceSpectrumFunction::CONCENTRATION);
			double rshift               = (double)ref[i]->GetModelParameter(CReferenceSpectrumFunction::SHIFT);
			double rshiftError          = (double)ref[i]->GetModelParameterError(CReferenceSpectrumFunction::SHIFT);
			double rsqueeze             = (double)ref[i]->GetModelParameter(CReferenceSpectrumFunction::SQUEEZE);
			double rsqueezeError        = (double)ref[i]->GetModelParameterError(CReferenceSpectrumFunction::SQUEEZE);
		}
#endif

		// clean up the evaluation
		free(measArray);
		delete solarSpec;
		for(int k = 0; k < MAX_N_REFERENCES; ++k)
			delete ref[k];
			return (0);
		}
		catch(CFitException e)
		{
			// in case that something went wrong, display the error to the user.
			// normally you will get error in two cases:
			//
			// 1. You forgot to set a valid fit range before you start fitting
			//
			// 2. A matrix inversion failed for some reason inside the fitting loop. Matrix inversions
			//    normally fail when there are linear dependecies in the matrix respectrively you have linear
			//    dependencies in your reference spectrum. Eg. you tried to fit the same reference spectrum twice at once.

			//  e.ReportError();
			//	std::cout << "Failed: " << ++iFalseCount << std::endl;
			//	std::cout << "Steps: " << cFirstFit.GetNonlinearMinimizer().GetFitSteps() << " - Chi: " << cFirstFit.GetNonlinearMinimizer().GetChiSquare() << std::endl;

			message.Format("A Fit Exception has occured. Are the reference files OK?");
			ShowMessage(message);

#ifdef _DEBUG
			FILE *f = fopen("C:\\temp\\solarSpectrum.txt", "w");
			for(int it = 1; it < specLen; ++it){
				fprintf(f, "%lf\n", localSolarSpectrumData.GetAt(it));
			}
			fclose(f);

			f = fopen("C:\\temp\\measuredSpectrum.txt", "w");
			for(int it = 1; it < specLen; ++it){
				fprintf(f, "%lf\n", measArray[it]);
			}
			fclose(f);

			CString fileName;
			for(i = 0; i < window.nRef; ++i){
				fileName.Format("C:\\temp\\reference_%d.txt", i);
				f = fopen(fileName, "w");
				for(int it = 1; it < specLen; ++it){
					fprintf(f, "%lf\t%lf\n", m_crossSection[i].GetWavelengthAt(i), m_crossSection[i].GetAt(it));
				}
				fclose(f);
			}
#endif

			// clean up the evaluation
			free(measArray);
			delete solarSpec;
			for(int k = 0; k < MAX_N_REFERENCES; ++k)
				delete ref[k];

			return (1);
		}
}

/** Returns the evaluation result for the last spectrum
  @return a reference to the evaluation result */
const CEvaluationResult& CEvaluation::GetEvaluationResult() const{
	return m_result;
}


/** Returns the polynomial that was fitted to the last evaluation result */
double *CEvaluation::GetPolynomial(){
	return m_result.m_polynomial;
}

/** read data from the references defined in the fit window 'm_window'. */
BOOL CEvaluation::ReadReferences(){
	if(m_window.nRef == 0)
		return FALSE;

	// read in the cross sections to use
	for(int k = 0; k < m_window.nRef; ++k){
		if(m_crossSection[k].ReadCrossSectionFile(m_window.ref[k].m_path))
			return FALSE;
	}

	// read the solar spectrum, if any...
	if(m_window.fraunhoferRef.m_path.size() > 6){
		if(m_solarSpectrumData.ReadCrossSectionFile(m_window.fraunhoferRef.m_path))
			return FALSE;
	}

	// Added 2005-12-13 to increase the speed of evaluation /MJ
	InitializeVectors(MAX_SPECTRUM_LENGTH);
	
	return TRUE;
}

void CEvaluation::InitializeVectors(int sumChn){
	int i; //iterator

	vXData.SetSize(sumChn);
	for(i = 0; i < sumChn; ++i)
		vXData.SetAt(i, (TFitData)(1.0f + (double)i));
}

BOOL CEvaluation::IncludeAsReference(double *array, int sumChn, int refNum){

	if(refNum == -1){
		//m_referenceData[m_referenceNum].SetSize(sumChn);
		//for(int index = 0; index < sumChn; ++index)
		//	m_referenceData[index].SetAt(index, (TFitData)array[index]);
		m_crossSection[m_referenceNum].Set(array, (unsigned long)sumChn);

		++m_referenceNum;
	}else{
		if(refNum > m_referenceNum - 1)
			++m_referenceNum;

		//m_referenceData[refNum].SetSize(sumChn);
		//for(int index = 0; index < sumChn; ++index)
		//	m_referenceData[refNum].SetAt(index, (TFitData)array[index]);
		m_crossSection[refNum].Set(array, (unsigned long)sumChn);
	}
	return TRUE;
}

void CEvaluation::RemoveOffset(double *spectrum, int sumChn, BOOL UV){
	int offsetFrom  = (UV) ? 50 : 2;
	int offsetTo    = (UV) ? 200 : 20;

	//  remove any remaining offset in the measured spectrum
	double avg = 0;
	for(int i = offsetFrom; i < offsetTo; i++){
		avg += spectrum[i];
	}
	avg = avg/(double)(offsetTo - offsetFrom);

	Sub(spectrum, sumChn, avg);

	return;
}

void CEvaluation::PrepareSpectra(double *sky, double *meas, const CFitWindow &window){

	if(window.fitType == FIT_HP_DIV)
		return PrepareSpectra_HP_Div(sky, meas, window);
	if(window.fitType == FIT_HP_SUB)
		return PrepareSpectra_HP_Sub(sky, meas, window);
	if(window.fitType == FIT_POLY)
		return PrepareSpectra_Poly(sky, meas, window);
}

void CEvaluation::PrepareSpectra_HP_Div(double *skyArray, double *measArray, const CFitWindow &window){

	//  1. Remove any remaining offset
	RemoveOffset(measArray, window.specLength, window.UV);
	RemoveOffset(skyArray, window.specLength, window.UV);

	// 2. Divide the measured spectrum with the sky spectrum
	Div(measArray,skyArray,window.specLength,0.0);

	// 3. high pass filter
	HighPassBinomial(measArray,window.specLength,500);

	// 4. log(spec)
	Log(measArray,window.specLength);

	// 5. low pass filter
//	LowPassBinomial(measArray,window.specLength, 5);
}

void CEvaluation::PrepareSpectra_HP_Sub(double *skyArray, double *measArray, const CFitWindow &window){

	// 1. remove any remaining offset in the measured spectrum
	RemoveOffset(measArray, window.specLength, window.UV);

	// 2. high pass filter
	HighPassBinomial(measArray,window.specLength,500);

	// 3. log(spec)
	Log(measArray,window.specLength);
}

void CEvaluation::PrepareSpectra_Poly(double *skyArray, double *measArray, const CFitWindow &window){

	// 1. remove any remaining offset in the measured spectrum
	RemoveOffset(measArray, window.specLength, window.UV);

	// 2. log(spec)
	Log(measArray,window.specLength);

	// 3. Multiply the spectrum with -1 to get the correct sign for everything
	for(int i = 0; i < window.specLength; ++i){
		measArray[i] *= -1.0;
	}
}

/** Assignment operator */
CEvaluation &CEvaluation::operator = (const CEvaluation &e2){
	m_window					= e2.m_window;
	m_numberOfReferencesToUse	= e2.m_numberOfReferencesToUse;

	return *this;
}

// Creates the appropriate CReferenceSpectrumFunction for the fitting
int CEvaluation::CreateReferenceSpectrum(const CFitWindow &window, int startChannel){
	CVector yValues;

	for(int i = 0; i < window.nRef; i++)
	{
		// reset all reference's parameters
		ref[i]->ResetLinearParameter();
		ref[i]->ResetNonlinearParameter();

		// enable amplitude normalization. This should normally be done in order to avoid numerical
		// problems during fitting.
		ref[i]->SetNormalize(true);

		// set the spectral data of the reference spectrum to the object. This also causes an internal
		// transformation of the spectral data into a B-Spline that will be used to interpolate the 
		// reference spectrum during shift and squeeze operations
		//if(!ref[i]->SetData(vXData.SubVector(0, m_referenceData[i].GetSize()), m_referenceData[i]))
		yValues.SetSize(m_crossSection[i].GetSize());
		for(unsigned int k = 0; k < m_crossSection[i].GetSize(); ++k){
			yValues.SetAt(k, m_crossSection[i].GetAt(k));
		}
		if(!ref[i]->SetData(vXData.SubVector(0, m_crossSection[i].GetSize()), yValues))
		{
			Error0("Error initializing spline object!");
			return(1);
		}

		// Chech the options for the column value
		switch(window.ref[i].m_columnOption){
			case SHIFT_FIX:   ref[i]->FixParameter(CReferenceSpectrumFunction::CONCENTRATION, window.ref[i].m_columnValue * ref[i]->GetAmplitudeScale()); break;
			case SHIFT_LINK:  ref[(int)window.ref[i].m_columnValue]->LinkParameter(CReferenceSpectrumFunction::CONCENTRATION, *ref[i], CReferenceSpectrumFunction::CONCENTRATION); break;
		}

		// Check the options for the shift
		switch(window.ref[i].m_shiftOption){
			case SHIFT_FIX:		ref[i]->FixParameter(CReferenceSpectrumFunction::SHIFT, window.ref[i].m_shiftValue); break;
			case SHIFT_LINK:	ref[(int)window.ref[i].m_shiftValue]->LinkParameter(CReferenceSpectrumFunction::SHIFT, *ref[i], CReferenceSpectrumFunction::SHIFT); break;
			case SHIFT_LIMIT:	ref[i]->SetParameterLimits(CReferenceSpectrumFunction::SHIFT,	(TFitData)window.ref[i].m_shiftValue, (TFitData)window.ref[i].m_shiftMaxValue, 1); break;
			default:			ref[i]->SetParameterLimits(CReferenceSpectrumFunction::SHIFT,	(TFitData)-10.0, (TFitData)10.0, (TFitData)1e0); break;
								ref[i]->SetDefaultParameter(CReferenceSpectrumFunction::SHIFT, (TFitData)0.0); 
		}

		// Check the options for the squeeze
		switch(window.ref[i].m_squeezeOption){
			case SHIFT_FIX:		ref[i]->FixParameter(CReferenceSpectrumFunction::SQUEEZE, window.ref[i].m_squeezeValue); break;
			case SHIFT_LINK:	ref[(int)window.ref[i].m_squeezeValue]->LinkParameter(CReferenceSpectrumFunction::SQUEEZE, *ref[i], CReferenceSpectrumFunction::SQUEEZE); break;
			case SHIFT_LIMIT:	ref[i]->SetParameterLimits(CReferenceSpectrumFunction::SQUEEZE,	(TFitData)window.ref[i].m_squeezeValue, (TFitData)window.ref[i].m_squeezeMaxValue, 1e7); break;
			default:			ref[i]->SetDefaultParameter(CReferenceSpectrumFunction::SQUEEZE, (TFitData)1.0); 
								ref[i]->SetParameterLimits(CReferenceSpectrumFunction::SQUEEZE,	(TFitData)0.98, (TFitData)1.02, (TFitData)1e0);break;
		}
	}

	return 0;
}
