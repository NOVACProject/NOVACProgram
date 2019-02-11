// Evaluation.h: interface for the CEvaluation class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EVALUATION_H__DB88EE51_7ED0_4131_AE07_79F0F0C3106C__INCLUDED_)
#define AFX_EVALUATION_H__DB88EE51_7ED0_4131_AE07_79F0F0C3106C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "../SpectralEvaluation/Evaluation/EvaluationBase.h"
#include "../SpectralEvaluation/Evaluation/CrossSectionData.h"
#include "../SpectralEvaluation/Evaluation/FitParameter.h"
#include "../SpectralEvaluation/Evaluation/EvaluationResult.h"
#include "../SpectralEvaluation/Spectra/Spectrum.h"

#include "../SpectralEvaluation/Fit/Vector.h"

#include "../SpectralEvaluation/Fit/ReferenceSpectrumFunction.h"

namespace Evaluation
{
	/** An object of the <b>CEvaluation</b>-class contains the settings to evaluate
		spectra from <b>one</b> spectrometer. The class contains the parameters 
		necessary to define the fit and a function to perform the actual evaluation.  */
	class CEvaluation : public CEvaluationBase
	{
	public:
		/** Default constructor */
		CEvaluation();

		/** Copy constructor */
		CEvaluation(const CEvaluation &eval2);

		/** Default destructor */
		virtual ~CEvaluation();

		/** The residual of the latest fit */
		CVector m_residual;

		/** The scaled reference spectra. The first reference spectrum is the fitted
			polynomial and the following 'MAX_N_REFERENCES + 1' are the scaled references. */
		CCrossSectionData m_fitResult[MAX_N_REFERENCES + 2];

		/** Reads the references defined in the parameter 'm_window'.
			@return FALSE if any error occurs, (eg. no references defined in 'm_window')
			@return TRUE if all is ok. */
		BOOL ReadReferences();

		/** Evaluate using the parameters set in the local parameter 'm_window'.
			@return 0 if all is ok.
			@return 1 if any error occured, or if the window is not defined. */
		int Evaluate(const CSpectrum &sky, const CSpectrum &measured, int numSteps = 1000);
        
		/** Evaluate the supplied spectrum using the solarReference found in 'window'
			@param measured - the spectrum for which to determine the shift & squeeze
			relative to the solarReference-spectrum found in 'window'
			@param window - the settings for the fit. The shift and squeeze between
			the measured spectrum and the solarReference-spectrum will be determined
			for the pixel-range 'window.fitLow' and 'window.fitHigh'
			@return 0 if the fit succeeds and the shift & squeeze could be determined
			@return 1 if any error occured. */
		int EvaluateShift(const CSpectrum &measured, const CFitWindow &window, double &shift, double &shiftError, double &squeeze, double &squeezeError);

		/** Returns the number of references that are read in */
		int NumberOfReferences() const {return m_referenceNum;}

		/** Includes an array as a reference file */
		BOOL IncludeAsReference(double *array, int sumChn, int refNum = -1);

		/** Assignment operator */
		CEvaluation &operator = (const CEvaluation &e2);

	private:

		/** The number of reference spectra that has been read in */
		int m_referenceNum;

		/** The reference files */
		CCrossSectionData m_crossSection[MAX_N_REFERENCES];

		/** The solar spectrum data */
		CCrossSectionData m_solarSpectrumData;

		/** The number of reference spectra that will be used for a call to 'Evaluate(CSpe...)' */
		int m_numberOfReferencesToUse;

		// Creates the appropriate CReferenceSpectrumFunction for the fitting
		int CreateReferenceSpectrum(const CFitWindow &window, int startChannel);

	};
}
#endif // !defined(AFX_EVALUATION_H__DB88EE51_7ED0_4131_AE07_79F0F0C3106C__INCLUDED_)
