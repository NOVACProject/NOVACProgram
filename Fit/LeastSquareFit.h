/**
* Contains the implementation of a least square fit.
*
* @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
* @version		1.0 @ 2001/09/09
*/
#if !defined(LEASTSQUARE_H_011206)
#define LEASTSQUARE_H_011206

#include "Minimizer.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning (push, 3)

namespace MathFit
{
	/**
	* Implements a least square solver for a linear minimization problem. 
	* Its based on the assumption that
	* \begin{verbatim}A*x=b\end{verbatim}
	* where x are the linear model parameters, A is the matrix with the respective model coefficients and b is the result vector.
	*
	* @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
	* @version		1.0 @ 2001/09/09
	*/
	class CLeastSquareFit : public IMinimizer
	{
	public:
		/**
		* Constructs an object and sets the model function.
		* 
		* @param ipfModel	The model function object.
		*
		*/
		CLeastSquareFit(IParamFunction& ipfModel) : IMinimizer(ipfModel)
		{
		}

		/**
		* Nothing to do for this kind of linear minimzation.
		*
		* @return Always TRUE.
		*/
		virtual bool PrepareMinimize()
		{
			if(mFitRange.GetSize() <= 0)
				throw(EXCEPTION(CNoFitRangeException));

			return true;
		}

		/**
		* Get the solution for the minimization problem.
		* The leat square method with the pseudo inverse is used for minimization
		*
		* @return FALSE when finished successfully.
		*/
		virtual bool Minimize()
		{
			if(mModel.GetLinearParameter().GetSize() <= 0)
				return false;

			// bring the matrix and vector to their appropriate sizes
			mA.SetSize(mModel.GetLinearParameter().GetSize(), mFitRange.GetSize());
			mB.SetSize(mFitRange.GetSize());

			// get the A matrix and a modified B vector according to the model function
			mModel.GetLinearAMatrix(mFitRange, mA, mB);

			// add data errors to model matrix
			CVector vError(mFitRange.GetSize());
			mModel.GetFunctionErrors(mFitRange, vError);
			const int iBSize = mB.GetSize();
			int i;
			for(i = 0; i < iBSize; i++)
			{
				mA.GetRow(i).Div(vError.GetAt(i));
				mB.SetAt(i, mB.GetAt(i) / vError.GetAt(i));
			}

			// build transposed A matrix
			// At
			mTranspose.Copy(mA);
			mTranspose.Transpose();

			// build the new result vector
			// At * b
			mTranspose.Mul(mB);

			// build inverse pseudo
			// (At*A)
			mA.PseudoInverse();

#if defined(MATHFIT_IMPROVEEQSSOLVE)
			// to apply the iterative solution improvement, we need backups of the original
			// result vector and EQS matrix
			CVector vBackupB(mB);
			CMatrix mBackupA(mA);
#endif

			// Solve linear equations
#if defined(MATHFIT_USELUDECOMPOSITION)
			mA.LUDecomposition();
			mA.LUBacksubstitution(mB);

#if defined(MATHFIT_IMPROVEEQSSOLVE)
			// we want to correct the numerical errors by applying
			// the 'iterative solution improvement' as described in Numerical Recipes.

			// Use the solution to calculate once again the result vector of the EQS
			CVector vSolutionError(mB);
			mBackupA.Mul(vSolutionError);

			// subtract the old result vector from the one calculated using the EQS result
			// (the result should be nearly zero at all)
			vSolutionError.Sub(vBackupB);

			// solve the EQS once again but use the solution error as result vector
			mA.LUBacksubstitution(vSolutionError);

			// subtract the solution error from the original solution
			mB.Sub(vSolutionError);
#endif
#else
			mA.GaussJordanSolve(mB);

#if defined(MATHFIT_IMPROVEEQSSOLVE)
			// we want to correct the numerical errors by applying
			// the 'iterative solution improvement' as described in Numerical Recipes.

			// Use the solution to calculate once again the result vector of the EQS
			CVector vSolutionError(mB);
			mBackupA.Mul(vSolutionError);

			// subtract the old result vector from the one calculated using the EQS result
			// (the result should be nearly zero at all)
			vSolutionError.Sub(vBackupB);

			// solve the EQS once again but use the solution error as result vector
			mBackupA.GaussJordanSolve(vSolutionError);

			// subtract the solution error from the original solution
			mB.Sub(vSolutionError);
#endif
#endif

			// and set the result
			mModel.SetLinearParameter(mB);			

			return false;
		}

		/**
		* Set the covariance matrix into the model object.
		*
		* @return Always TRUE.
		*/
		virtual bool FinishMinimize()
		{
			const int iParams = mModel.GetLinearParameter().GetSize();
			if(iParams <= 0)
				return true;

			mDiff.SetSize(mFitRange.GetSize());
			mModel.GetValues(mFitRange, mDiff);

			// now calculate the chi square and variance values
			CVector vErr(mFitRange.GetSize());
			mModel.GetFunctionErrors(mFitRange, vErr);

			// get the sum of squares weighted by the sigma error vector
			mChiSquare = mDiff.SquareSumErrorWeighted(vErr);
			mChiSquare += mModel.GetLinearPenalty(mChiSquare);

			// calculate the normalization factor for all statistical parameters
			TFitData fNorm = (TFitData)sqrt(mChiSquare / (mDiff.GetSize() - iParams));

			// set the covariance matrix, which is the inverse of A after the Gauss Jordan elimination
#if defined(MATHFIT_USELUDECOMPOSITION)
			mA.LUInverse();
#endif
			CMatrix mCovar(mA);

			// set the covariance matrix
			mModel.SetLinearCovarMatrix(mCovar);

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
			mModel.SetLinearCorrelMatrix(mCorrel);

			// now we have to 'normalize' the parameter errors to chi square.
			vError.Mul(fNorm);
			mModel.SetLinearError(vError);

			return true;
		}

	private:
		/**
		* This matrix contains the pseudo inverse.
		*/
		CMatrix mPseudoInverse;
		/**
		* Contains the A matrix.
		*/
		CMatrix mA;
		/**
		* Contains the column index of the A matrix after LU decomposition
		*/
		CVector mAIndex;
		/**
		* Contains the B vector
		*/
		CVector mB;
		/**
		* Contains the transposed A matrix
		*/
		CMatrix mTranspose;
	};
}
#pragma warning (pop)
#endif
