/**
* Contains the implementation of the nonlinear Levenberg-Marquardt fit.
*
* @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
* @version		1.0 @ 2001/09/09
*/
#if !defined(LEVENBERGMARQUARDT_H_011206)
#define LEVENBERGMARQUARDT_H_011206

#include <float.h>
#include "Minimizer.h"

#define STARTLAMBDA		0.01
#define MINLAMBDA		1e-20
#define MAXLAMBDA		1e20
#define EPSILON			1e-5
#define CHISQUAREMIN	1e-20

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning (push, 3)

namespace MathFit
{
	/**
	* Implements the nonlinear minimizer interface using the Levenberg Marquardt algorithm.
	* Uses a iterative method to minimize the given function.
	*
	* @author		\item \URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
	* @author		\item \URL[Silke Humbert]{mailto:silke.humbert@iup.uni-heidelberg.de} @ \URL[IUP, Satellite Data Group]{http://giger.iup.uni-heidelberg.de}
	* @version		1.0 @ 2001/09/09
	*/
	class CLevenbergMarquardtFit : public IMinimizer
	{
	public:
		/**
		* Constructs the object and sets the model function.
		*
		* @param ipfModel	The model function which parametes should be fitted.
		*/
		CLevenbergMarquardtFit(IParamFunction& ipfModel) : IMinimizer(ipfModel),
			mSTARTLAMBDA((TFitData)STARTLAMBDA),
			mMINLAMBDA((TFitData)MINLAMBDA),
			mMAXLAMBDA((TFitData)MAXLAMBDA),
			mEPSILON((TFitData)EPSILON),
			mCHISQUAREMIN((TFitData)CHISQUAREMIN)
		{
			// nothing to do in here, since everything is done with the constructors
		}

		virtual bool PrepareMinimize()
		{
			if(mModel.GetNonlinearParameter().GetSize() <= 0)
				return true;

			if(mFitRange.GetSize() <= 0)
				throw(EXCEPTION(CNoFitRangeException));

			mLambda = mSTARTLAMBDA;
			mSolutionFitSteps = mFitSteps = 0;

			// set the appropriate matrix sizes
			mDyDa.SetSize(mModel.GetNonlinearParameter().GetSize(), mFitRange.GetSize());

			// first call of Analyze()
			if(!Analyze())
				return false;

			// prepare a lower border for the chi square
			if(mCheckChiSquare < 0)
				mCheckChiSquare = mChiSquare * mCHISQUAREMIN;
			if(!_finite(mCheckChiSquare))
				mCheckChiSquare = mCHISQUAREMIN;

			if(mMaxFitSteps < 0)
				mMaxFitSteps = 1000;

			return true;
		}

		/**
		* This is the main function that performs one step in the fitting algorithm.
		*
		* @return TRUE if minimization is finished, FALSE otherwise.
		*/
		virtual bool Minimize()
		{
			if(mModel.GetNonlinearParameter().GetSize() <= 0)
				return false;

			// check lambda
			if(mLambda >= mMAXLAMBDA)
				return false;

			// check for already optimal solution
			if(mCheckChiSquare > mChiSquare)
				return false;

			mOldChiSquare = mChiSquare;

			// backup old values
			mAlphaOld.Copy(mAlpha);
			mBetaOld.Copy(mBeta);

			// alter alpha by augmenting diagonal elements
			mAlpha.MulDiag(1 + mLambda);

			// solve the linear equations
#if defined(MATHFIT_USELUDECOMPOSITION)
			mAlpha.LUDecomposition();
			mAlpha.LUBacksubstitution(mBeta);

#if defined(MATHFIT_IMPROVEEQSSOLVE)
			// we want to correct the numerical errors by applying
			// the 'iterative solution improvement' as described in Numerical Recipes.

			// Use the solution to calculate once again the result vector of the EQS
			CVector vSolutionError(mBeta);
			mAlphaOld.Mul(vSolutionError);

			// subtract the old result vector from the one calculated using the EQS result
			// (the result should be nearly zero at all)
			vSolutionError.Sub(mBetaOld);

			// solve the EQS once again but use the solution error as result vector
			mAlpha.LUBacksubstitution(vSolutionError);

			// subtract the solution error from the original solution
			mBeta.Sub(vSolutionError);
#endif
#else
			mAlpha.GaussJordanSolve(mBeta);

#if defined(MATHFIT_IMPROVEEQSSOLVE)
			// we want to correct the numerical errors by applying
			// the 'iterative solution improvement' as described in Numerical Recipes.

			// Use the solution to calculate once again the result vector of the EQS
			CVector vSolutionError(mBeta);
			mAlphaOld.Mul(vSolutionError);

			// subtract the old result vector from the one calculated using the EQS result
			// (the result should be nearly zero at all)
			vSolutionError.Sub(mBetaOld);

			// solve the EQS once again but use the solution error as result vector
			CMatrix mBackupAlpha(mAlphaOld);
			mBackupAlpha.GaussJordanSolve(vSolutionError);

			// subtract the solution error from the original solution
			mBeta.Sub(vSolutionError);
#endif
#endif

			// set new parameter vector
			mModel.BackupNonlinearParameter();
			mModel.SetNonlinearParameter(mModel.GetNonlinearParameter().Add(mBeta));

			// analyze new parameters
			if(!Analyze())
				return false;

			mFitSteps++;

			// UPD010425: we decrease the lambda and keep the new parameters when we have a better chi square!
			// if the chi square penalty is set to hard limits, the chi square can be set to infinite, so check this
			// condition, too!
			if(_finite(mChiSquare) && mOldChiSquare >= mChiSquare)
			{
				// keep the current iteration count as best steps
				mSolutionFitSteps = mFitSteps;

				if(mLambda > mMINLAMBDA)
					mLambda /= 10;

				// chi square doesn't differ to much anymore, so we're finished
				TFitData fDiff = (mOldChiSquare - mChiSquare)/mChiSquare;
				if(fDiff < mEPSILON)
					return false;			
			}
			else 
			{
				// worse result, so restore model parameters and matrices
				mModel.RestoreNonlinearParameter();
				mAlpha.Copy(mAlphaOld);
				mBeta.Copy(mBetaOld);
				mLambda *= 10;
			}

			// check fit steps
			// the check is done after the parameter reconfiguration, so that only valid parameter's are
			// available.
			if(mMaxFitSteps > 0 && mFitSteps >= mMaxFitSteps)
				return false;

			return true;
		}

		/**
		* Just set the covariance matrix in the model function object.
		*
		* @return Always TRUE.
		*/
		virtual bool FinishMinimize()
		{
			const int iParams = mModel.GetNonlinearParameter().GetSize();
			if(iParams <= 0)
				return true;

			mDiff.SetSize(mFitRange.GetSize());
			mModel.GetValues(mFitRange, mDiff);

			// now calculate the chi square and variance values
			CVector vErr(mFitRange.GetSize());
			mModel.GetFunctionErrors(mFitRange, vErr);

			// get the sum of squares weighted by the error vector
			mChiSquare = mDiff.SquareSumErrorWeighted(vErr);
			mChiSquare += mModel.GetNonlinearPenalty(mChiSquare);

			// calculate the normalization factor for all statistical values
			TFitData fNorm = (TFitData)sqrt(mChiSquare / (mFitRange.GetSize() - iParams));

			// inverse the alpha matrix
#if defined(MATHFIT_USELUDECOMPOSITION)
			if(!mAlpha.IsLUDecomposed())
				mAlpha.LUDecomposition();
			mAlpha.LUInverse();
#else
			mAlpha.Inverse();
#endif

			// set the covariance matrix
			CMatrix mCovar(iParams, iParams);
			mCovar.Copy(mAlpha);
			mModel.SetNonlinearCovarMatrix(mCovar);

			// calculate the parameter errors
			CVector vError(iParams);
			int i;
			for(i = 0; i < iParams; i++)
				vError.SetAt(i, (TFitData)sqrt(mCovar.GetAt(i, i)));

			// calculate the correlation matrix
			CMatrix mCorrel(iParams, iParams);

			int j;
			for(i = 0; i < iParams; i++)
				for(j = 0; j < iParams; j++)
					mCorrel.SetAt(i, j, mCovar.GetAt(i, j) / (vError.GetAt(i) * vError.GetAt(j)));
			mModel.SetNonlinearCorrelMatrix(mCorrel);

			// now we have to 'normalize' the parameter errors to chi square.
			vError.Mul(fNorm);
			mModel.SetNonlinearError(vError);

			return true;
		}

		/**
		* Calculates the alpha matrix and coneAngle vector needed by the Levenberg Marquardt algorithm.
		*
		* @return TRUE is successful, FALSE otherwise
		*/
		bool Analyze()
		{
			// get the first derivatives of the model function
			mModel.GetNonlinearDyDa(mFitRange, mDyDa);

			// setting mBeta, mAlpha to its proper size
			const int iParamCount = mModel.GetNonlinearParameter().GetSize();
			mBeta.SetSize(iParamCount);
			mAlpha.SetSize(iParamCount, iParamCount);

			// initialize to zero
			mBeta.Zero();
			mAlpha.Zero();

			mChiSquare = 0;

			int i,j,k;

			mDiff.SetSize(mFitRange.GetSize());
			mModel.GetValues(mFitRange, mDiff);
			CVector vError(mFitRange.GetSize());
			mModel.GetFunctionErrors(mFitRange, vError);

			// 1. LOOP
			// =======
			// summation over all data
			const int iDiffSize = mDiff.GetSize();
			for(i = 0; i < iDiffSize; i++)
			{
				// calculate the difference between the real y-values and the modelled y-values
				TFitData fDifferenceInY = mDiff.GetAt(i) * mDiff.GetAt(i);

				TFitData fSigmaSquare = vError.GetAt(i) * vError.GetAt(i);

				// chiSquare
				mChiSquare += fDifferenceInY / fSigmaSquare;

				// 2. LOOP
				// =======
				// summation over all parameters
				for(j = 0; j < iParamCount; j++)
				{
					TFitData fWT = mDyDa.GetAt(i, j) / fSigmaSquare;

					// calculating coneAngle
					mBeta.SetAt(j, mBeta.GetAt(j) + mDiff.GetAt(i) * fWT);

					// 3. LOOP
					// =======
					// relevant only for alpha
					for(k = 0; k <= j; k++)
						mAlpha.SetAt(j, k, mAlpha.GetAt(j, k) + mDyDa.GetAt(i, k) * fWT);
				} // end of second loop
			} // end of first loop	 

			// fill in the symmetric side of alpha and ensure that we do not have zeros on the diagonal. Otherwise
			// the LEQ can't be solved!
			for(i = 0; i < iParamCount; i++)
			{
				if(mAlpha.GetAt(i, i) == 0)
					mAlpha.SetAt(i, i, MATHFIT_NEARLYZERO);

				for(j = 0; j < i; j++)
					mAlpha.SetAt(j, i, mAlpha.GetAt(i, j));
			}

			// add the penalyt of the new parameters
			mChiSquare += mModel.GetNonlinearPenalty(mChiSquare);

			return true;
		}

	private:
		/**
		* Contains the coneAngle vector of the fit algorithm.
		*/
		CVector mBeta;
		/**
		* Contains the old coneAngle vector of the fit algorithm.
		*/
		CVector mBetaOld;
		/**
		* Contains the alpha matrix of the algorithm.
		*/
		CMatrix mAlpha;
		/**
		* Contains the old alpha matrix of the algorithm.
		*/
		CMatrix mAlphaOld;
		/**
		* Contains the DyDa matrix of the model function.
		*/
		CMatrix mDyDa;
		/**
		* The current lambda value.
		*/
		TFitData mLambda;
		/**
		* The ChiSquare value of the last loop.
		*/
		TFitData mOldChiSquare;
		/**
		* The start value for the lambda parameter.
		*/
		const TFitData mSTARTLAMBDA;
		/**
		* The minimum lambda value.
		*/
		const TFitData mMINLAMBDA;
		/**
		* The maximum lambda value.
		*/
		const TFitData mMAXLAMBDA;
		/**
		* We abort, of Chi square doesn't differ more than EPSILON anymore.
		*/
		const TFitData mEPSILON;
		/**
		* we already have an optimal solution, if chi square is smaller than this value.
		*/
		const TFitData mCHISQUAREMIN;
	};
}

#pragma warning (pop)
#endif
