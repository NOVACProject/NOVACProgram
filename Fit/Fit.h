/**
 * Contains the implementation of a combined linear and non linear fit.
 *
 * @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
 * @version		1.0 @ 2001/09/09
 */
#if !defined(FIT_H_011206)
#define FIT_H_011206

#include "Minimizer.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning (push, 3)

namespace MathFit
{
	/**
	* Performs a fit of the nonlinear and linear parameters of the model object.
	* The fit itself is done doing the nonlinear optimization loop where during each iteration
	* step a linear fit is done to get the best linear parameters for the current set of nonlinear
	* ones.
	*
	* @author		\item \URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
	* @author		\item \URL[Silke Humbert]{mailto:silke.humbert@iup.uni-heidelberg.de} @ \URL[IUP, Satellite Data Group]{http://giger.iup.uni-heidelberg.de}
	* @version		1.0 @ 2001/09/09
	*/
	class CFit : public IMinimizer
	{
	public:
		/**
		* Sets the model object to the neccessary minimizer objects.
		*
		* @param ipfModel		The model object used for fitting.
		* @param ipmLinear		The minimizer object to be used for linear fitting.
		* @param ipmNonlinear	The minimizer object used for nonlinear fitting.
		*/
		CFit(IParamFunction& ipfModel, IMinimizer& ipmLinear, IMinimizer& ipmNonlinear) : IMinimizer(ipfModel), mMinimizer(ipmNonlinear), mLinearMinimizer(ipmLinear)
		{
		}

		/**
		* Nothing to do for this kind of linear minimzation.
		*
		* @return Always TRUE.
		*/
		virtual bool PrepareMinimize()
		{
			// if no fit range is given, we can't do the fit!
			if(mFitRange.GetSize() <= 0)
				throw(EXCEPTION(CNoFitRangeException));

			return true;
		}

		/**
		* Just copy the number of iterations and call the super classes FinishMinimize method.
		*
		* @return TRUE if successful, FALSE otherwise.
		*/
		virtual bool FinishMinimize()
		{
			if(!IMinimizer::FinishMinimize())
				return false;

			mFitSteps = mMinimizer.GetFitSteps();
			mSolutionFitSteps = mMinimizer.GetSolutionFitSteps();

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
			// prepare the linear fit
			if(!mLinearMinimizer.PrepareMinimize())
				return false;

			// do the first linear fit to initialy set the linear parameters
			while(mLinearMinimizer.Minimize());

			// prepare the nonlinear fit
			if(!mMinimizer.PrepareMinimize())
				return false;

			// repeat until the nonlinear fit aborts
			while(mMinimizer.Minimize())
			{
				// do the linear fit
				while(mLinearMinimizer.Minimize());
			}

			// finish the nonlinear fit
			if(!mMinimizer.FinishMinimize())
				return false;

			// finish the linear fit
			if(!mLinearMinimizer.FinishMinimize())
				return false;

			return true;
		}

		/**
		* Returns the nonlinear minimizer object.
		*
		* @return	The nonlinear minimizer object.
		*/
		virtual IMinimizer& GetNonlinearMinimizer()
		{
			return mMinimizer;
		}

		/**
		* Returns the linear minimizer object.
		*
		* @return	The linear minimizer object.
		*/
		virtual IMinimizer& GetLinearMinimizer()
		{
			return mLinearMinimizer;
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

			mLinearMinimizer.SetFitRange(vFitRange);
			mMinimizer.SetFitRange(vFitRange);
		}

		virtual void SetMaxFitSteps(int iMaxSteps)
		{
			mMaxFitSteps = iMaxSteps;

			mMinimizer.SetMaxFitSteps(iMaxSteps);
			mLinearMinimizer.SetMaxFitSteps(iMaxSteps);
		}

		virtual void SetMinChiSquare(TFitData fMinChiSquare)
		{
			mCheckChiSquare = fMinChiSquare;

			mMinimizer.SetMinChiSquare(fMinChiSquare);
			mLinearMinimizer.SetMinChiSquare(fMinChiSquare);
		}

	private:
		/**
		* The nonlinear minimizer object.
		*/
		IMinimizer& mMinimizer;
		/**
		* The linear minimizer object.
		*/
		IMinimizer& mLinearMinimizer;
	};
}

#pragma warning (pop)
#endif
