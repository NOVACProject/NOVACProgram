// Evaluation.h: interface for the CEvaluation class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EVALUATION_H__DB88EE51_7ED0_4131_AE07_79F0F0C3106C__INCLUDED_)
#define AFX_EVALUATION_H__DB88EE51_7ED0_4131_AE07_79F0F0C3106C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "BasicMath.h"
#include "EvaluationResult.h"
#include "FitWindow.h"
#include "CrossSectionData.h"

#include "../FIT\Vector.h"	// Added by ClassView

#include "../Common/Spectra/Spectrum.h"
#include "../Fit/ReferenceSpectrumFunction.h"

namespace Evaluation
{
	/** constants for selection of fit-parameters*/
	const enum FIT_PARAMETER{ 
			COLUMN, 
			COLUMN_ERROR, 
			SHIFT, 
			SHIFT_ERROR, 
			SQUEEZE, 
			SQUEEZE_ERROR,
			DELTA};

	// The options for how to ignore spectra (spectra which are too dark or saturated)
	enum IgnoreType { IGNORE_DARK, IGNORE_LIMIT, IGNORE_NOTHING};
	typedef struct IgnoreOption{
		IgnoreType m_type;
		double     m_intensity;
		long       m_channel;
	}IgnoreOption;

	/** An object of the <b>CEvaluation</b>-class contains the settings to evaluate
		spectra from <b>one</b> spectrometer. The class contains the parameters 
		necessary to define the fit and a function to perform the actual evaluation. 
	*/
	class CEvaluation : public CBasicMath	
	{
	public:
		/** Default constructor */
		CEvaluation();

		/** Copy constructor */
		CEvaluation(const CEvaluation &eval2);

		/** Default destructor */
		virtual ~CEvaluation();

		/** The fit window, defines the parameters for the fit */
		CFitWindow m_window;

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

		/** Evaluate using the parameters set the supplied fit window
			@param sky - The Fraunhofer reference
			@param measured - The spectrum to evaluate
			@param window - A CFitWindow object, defines the parameters for the fit
			@return 0 if all is ok.
			@return 1 if any error occured. */
		int Evaluate(const CSpectrum &sky, const CSpectrum &measured, const CFitWindow &window, int numSteps = 400);

		/** Evaluate the supplied spectrum using the solarReference found in 'window'
			@param measured - the spectrum for which to determine the shift & squeeze
			relative to the solarReference-spectrum found in 'window'
			@param window - the settings for the fit. The shift and squeeze between
			the measured spectrum and the solarReference-spectrum will be determined
			for the pixel-range 'window.fitLow' and 'window.fitHigh'
			@return 0 if the fit succeeds and the shift & squeeze could be determined
			@return 1 if any error occured. */
		int EvaluateShift(const CSpectrum &measured, const CFitWindow &window, double &shift, double &shiftError, double &squeeze, double &squeezeError);

		/** Returns the evaluation result for the last spectrum
			@return a reference to a 'CEvaluationResult' - data structure which holds the information from the last evaluation */
		const CEvaluationResult& GetEvaluationResult() const;

		/** Returns the polynomial that was fitted in the last evaluation */
		double *GetPolynomial();

		/** Returns the number of references that are read in */
		int NumberOfReferences() const {return m_referenceNum;}

		/** Includes an array as a reference file */
		BOOL IncludeAsReference(double *array, int sumChn, int refNum = -1);

		/** Removes the offset from the supplied spectrum */
		void RemoveOffset(double *spectrum, int sumChn, BOOL UV = TRUE);
	
		/** Assignment operator */
		CEvaluation &operator = (const CEvaluation &e2);

	private:
		/** The result */
		CEvaluationResult m_result;

		/** The number of reference spectra that has been read in */
		int m_referenceNum;

		/** The reference files */
		CCrossSectionData m_crossSection[MAX_N_REFERENCES];

		/** The solar spectrum data */
		CCrossSectionData m_solarSpectrumData;

		/** The number of reference spectra that will be used for a call to 'Evaluate(CSpe...)' */
		int m_numberOfReferencesToUse;

		/** Simple vector for holding the channel number information */
		CVector vXData;

		/** Simple function for initializing the vectors used in the evaluation */
		void InitializeVectors(int sumChn);

		// Prepares the spectra for evaluation
		void PrepareSpectra(double *sky, double *meas, const CFitWindow &window);

		// Prepares the spectra for evaluation
		void PrepareSpectra_HP_Div(double *sky, double *meas, const CFitWindow &window);

		// Prepares the spectra for evaluation
		void PrepareSpectra_HP_Sub(double *sky, double *meas, const CFitWindow &window);

		// Prepares the spectra for evaluation
		void PrepareSpectra_Poly(double *sky, double *meas, const CFitWindow &window);

		// The CReferenceSpectrumFunctions are used in the evaluation process to model
		// the reference spectra for the different species that are being fitted.
		CReferenceSpectrumFunction *ref[MAX_N_REFERENCES];
		CReferenceSpectrumFunction *solarSpec;

		// Creates the appropriate CReferenceSpectrumFunction for the fitting
		int CreateReferenceSpectrum(const CFitWindow &window, int startChannel);

	};
}
#endif // !defined(AFX_EVALUATION_H__DB88EE51_7ED0_4131_AE07_79F0F0C3106C__INCLUDED_)
