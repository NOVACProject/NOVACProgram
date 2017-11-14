#pragma once

#include <Afxtempl.h>

#include "../Common/Common.h"
#include "ReferenceFitResult.h"
#include "../Common/Spectra/SpectrumInfo.h"
#include "ColumnCorrection.h"

namespace Evaluation
{

#define MARK_BAD_EVALUATION 0x0001
#define MARK_DELETED 0x0002

	/** <b>CEvaluationResult</b> is a generic class to describe the results after evaluating
			a single spectrum. It contains an array of CReferenceFitResult:s - one for each
			reference spectrum that was included in the fit, and other information about the fit - 
			such as Delta, ChiSquare, the number of iterations required and the fitted polynomial. */
	class CEvaluationResult
	{
	public:
		/** Default constructor */
		CEvaluationResult(void);

		/** Copy constructor */
		CEvaluationResult(const CEvaluationResult &b);

		/** Default destructor */
		~CEvaluationResult(void);

		/** The evaluated result for the reference files */
		CArray<CReferenceFitResult, CReferenceFitResult&> m_ref;

		/** The Corrections applied to each of the evaluation results */
		CArray <CColumnCorrection, CColumnCorrection&> m_correction;

		/** The result for the polynomial. The polynomial is stored so that m_polynomial[i] is
			the i:th order term */
		double m_polynomial[6];

		/** The delta of the fit */
		double m_delta;

		/** The Chi-square of the fit */
		double m_chiSquare;

		/** The number of steps required to make the fitting */
		long	m_stepNum;

		/** The number of species that was used in the evaluation */
		unsigned long	m_speciesNum;

		/** The status of the evaluation.
			if(m_evaluationStatus & BAD_EVALUATION) then the spectrum is marked as a bad evaluation 
			if(m_evaluationStatus & DELETED) then the spectrum is marked as deleted (used in post-flux calculations) 
		*/
		int	 m_evaluationStatus;

		// --------------- PUBLIC METHODS ---------------------
		RETURN_CODE InsertSpecie(const CString &name);

		/** Checks the goodness of fit for this spectrum.
			@param info - the information about this spectrum
			@param chi2Limit - (optional) the limit of chi2 to use
			@param upperLimit - (optional) the upper limit to use on the saturation level
			@param lowerLimit - (optional) the lower limit to use on the saturation level */
		bool CheckGoodnessOfFit(const CSpectrumInfo& info, float chi2Limit = -1, float upperLimit = -1, float lowerLimit = -1);

		/** Returns true if this spectrum is judges as being an ok spectrum */
		int	IsOK() const {return !(m_evaluationStatus & MARK_BAD_EVALUATION); }

		/** Returns false if this spectrum is judges as being a bad spectrum */
		int	IsBad() const {return (m_evaluationStatus & MARK_BAD_EVALUATION); }

		/** Returns true if this spectrum is judges as being an ok spectrum */
		int	IsDeleted() const {return (m_evaluationStatus & MARK_DELETED); }

		/** Marks the current spectrum with the supplied mark_flag.
			Mark flag must be MARK_BAD_EVALUATION, or MARK_DELETED
			@return SUCCESS on success. */
		RETURN_CODE	MarkAs(int MARK_FLAG);

		/** Removes the current mark from the desired spectrum
			Mark flag must be MARK_BAD_EVALUATION, or MARK_DELETED
			@return SUCCESS on success. */
		RETURN_CODE	RemoveMark(int MARK_FLAG);

		// --------------- OPERATORS ---------------------
		CEvaluationResult &operator=(const CEvaluationResult &b);
	};
}