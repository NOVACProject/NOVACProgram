/**
 * Contains the fit interfaces and their default implementations.
 *
 * @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
 * @version		1.0 @ 2001/09/09
 */
#if !defined(MINIMIZER_H_011206)
#define MINIMIZER_H_011206

#include "StatisticVector.h"
#include "ParamFunction.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

namespace MathFit
{
	/**
	* Exception class indicating a missing fit range defintion.
	*
	* @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
	* @version		1.0 @ 2002/02/14
	*/
	class CNoFitRangeException : public CFitException
	{
	public:
		CNoFitRangeException(int iLine, const char* szModule, const char* szName) : CFitException(iLine, szModule, szName, "Not fit range defined!") {};
	};

	/**
	* The minimizer interface represents a nonlinear fit. 
	* It tries to minimize the overall sum of the given
	* model function values at the given data points.
	*
	* A normal fit should look like:
	*
	* PrepareMinimize();		// prepare everything for the minimizing
	* while(Minimize());		// run the minimize loop
	* FinishMinimize();		// do post processing
	*
	* @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
	* @version		1.0 @ 2001/09/09
	*/
	class IMinimizer 
	{
	public:
		/**
		* Constructs the object and initializes the variables.
		*
		* @param ipfModel	The parametric function object which parameters should be defined.
		*/
		IMinimizer(IParamFunction& ipfModel) : mModel(ipfModel)
		{
			mMaxFitSteps = 10000;
			mCheckChiSquare = -1;
			mFitSteps = 0;
			mSolutionFitSteps = 0;
			mChiSquare = -1;
		}

		/**
		* Call this function once before you start the minimize loop. 
		* Here neccessary preparations can be done.
		*
		* @return TRUE if successful, FALSE otherwise.
		*/
		virtual bool PrepareMinimize() = 0;

		/**
		* This is the main minimize function. 
		* It performs one single step of the minimization.
		* Call this function until it returns TRUE.
		*
		* @return TRUE if minimum reached, FALSE otherwise.
		*/
		virtual bool Minimize() = 0;

		/**
		* Call this function after the minimize loop exited.
		*
		* @return TRUE if successful, FALSE otherwise.
		*/
		virtual bool FinishMinimize()
		{
			mDiff.SetSize(mFitRange.GetSize());
			mModel.GetValues(mFitRange, mDiff);

			// now calculate the chi square and variance values
			CVector vErr(mFitRange.GetSize());
			mModel.GetFunctionErrors(mFitRange, vErr);

			mChiSquare = mDiff.SquareSumErrorWeighted(vErr);
			mChiSquare += mModel.GetLinearPenalty(mChiSquare);
			mChiSquare += mModel.GetNonlinearPenalty(mChiSquare);

			return true;
		}

		/**
		* Returns the number of fit steps of the last fit done.
		*
		* @return The number of steps of the last fit.
		*/
		virtual int GetFitSteps()
		{
			return mFitSteps;
		}

		/**
		* Returns the number of fit steps needed to find the final solution of the last fit done.
		*
		* @return The number of steps for the best solution of the last fit.
		*/
		virtual int GetSolutionFitSteps()
		{
			return mSolutionFitSteps;
		}

		/**
		* Returns ChiSquare of the last fit done.
		*
		* @return The ChiSquare value of the last fit.
		*/
		virtual TFitData GetChiSquare ()
		{
			return mChiSquare;
		}

		/**
		* Sets the maximum number of steps for the fit loop.
		*
		* @param iMaxSteps	The maximum number of steps.
		*/
		virtual void SetMaxFitSteps(int iMaxSteps)
		{
			mMaxFitSteps = iMaxSteps;
		}

		/**
		* Sets the minimum ChiSquare value at which the fit should stop.
		*
		* @param fMinChiSquare	The minimum ChiSquare value.
		*/
		virtual void SetMinChiSquare(TFitData fMinChiSquare)
		{
			mCheckChiSquare = fMinChiSquare;
		}

		/**
		* Returns the residuum of after the fit.
		*
		* @return	A reference to a vector containing the residuum.
		*/
		virtual CStatisticVector& GetResiduum()
		{
			return mDiff;
		}

		/**
		* Sets the support values used for fitting.
		* The vector given here defines the X values at which the model function
		* will be evaluated during fitting.
		*
		* @param vFitRange	A vector that contains the support values of the fit range.
		*/
		virtual void SetFitRange(CVector& vFitRange)
		{
			mFitRange.Copy(vFitRange);
			mModel.SetFitRange(vFitRange);
		}

		/**
		* Returns the fit range vector used for fitting.
		* The vector contains the support values used for discretization.
		*
		* @return	A reference to the support value's vector.
		*/
		CVector& GetFitRange()
		{
			return mFitRange;
		}

	protected:
		/**
		* The number of iteration loops done in the fit.
		*/
		int mFitSteps;
		/**
		 * Holds the number of steps needed to find the final solution.
		 */
		int mSolutionFitSteps;
		/**
		* The ChiSquare value (sum over all samples of the model function).
		*/
		TFitData mChiSquare;
		/**
		* Contains the model function.
		*/
		IParamFunction& mModel;
		/**
		* The minimum ChiSquare value.
		*/
		TFitData mCheckChiSquare;
		/**
		* The maximum number of steps.
		*/
		int mMaxFitSteps;
		/**
		* Contains the residuum after the fit.
		*/
		CStatisticVector mDiff;
		/**
		* Contains the fit range's support values.
		*/
		CVector mFitRange;
	};
}
#endif // !defined(AFX_FIT_H__F0C94500_BA3A_42EA_9B39_3BAF247BD44E__INCLUDED_)
