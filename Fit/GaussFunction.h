/**
* Contains a defintion of a Gauss function object.
*
* @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
* @version		1.0 @ 2001/09/21
*/
#if !defined(GAUSSFUNCTION_H_011206)
#define GAUSSFUNCTION_H_011206

#include "ParamFunction.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

namespace MathFit
{
	/**
	* This object represents a Gauss function.
	*
	* @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
	* @version		1.0 @ 2001/11/15
	*/
	class CGaussFunction : public IParamFunction
	{
	public:
		/**
		* Creates the object and sets the default parameter values.
		*
		* @param bNormAmp	If TRUE the area under the function will always be one.
		*/
		CGaussFunction(bool bNormAmp = false) : fSqrPI((TFitData)sqrt(2 * MATHFIT_PI))
		{
			// set default parameters
			ResetNonlinearParameter();

			// we can only scale the amplitude, if we should not keep the area size fixed
			mNormAmp = bNormAmp;
			ResetLinearParameter();
		}

		/**
		* Returns the value of the reference spectrum at the given data point.
		*
		* \begin{verbatim}f(x)=s*exp(-1/2*(x-a)^2/o^2)\end{verbatim}
		*
		* where
		\begin{verbatim}
		s := amplitude scale
		a := center
		o := sigma
		\end{verbatim}
		*
		* @param fXValue	The X data point.
		*
		* @return	The value of the reference at the given data point.
		*/
		virtual TFitData GetValue(TFitData fXValue)
		{
			TFitData fVal = fXValue - GetCenter();
			fVal *= fVal;
			fVal /= GetSigma() * GetSigma();
			fVal *= -0.5;

			return GetScale() * (TFitData)exp(fVal);
		}

		/**
		* Calculates the function values at a set of given data points.
		*
		* @param vXValues			A vector object containing the X values at which the function has to be evaluated.
		* @param vYTargetVector	A vector object which receives the resulting function values.
		*
		* @return	A reference to the Y vector object.
		*
		* @see	GetValue
		*/
		virtual CVector& GetValues(CVector& vXValues, CVector& vYTargetVector)
		{
			TFitData fSigma = (TFitData)-0.5 / (GetSigma() * GetSigma());

			const int iXSize = vXValues.GetSize();
			int i;
			for(i = 0; i < iXSize; i++)
			{
				TFitData fVal = vXValues.GetAt(i) - GetCenter();
				fVal *= fVal;
				vYTargetVector.SetAt(i, GetScale() * (TFitData)exp(fVal * fSigma));
			}

			return vYTargetVector;
		}

		/**
		* Returns the first derivative of the reference spectrum at the given data point.
		* The first derivative is given by
		*
		* \begin{verbatim}f'(x)=-s*(x-a)/o^2*exp(-1/2*(x-a)^2/o^2)\end{verbatim}
		*
		* where
		\begin{verbatim}
		c := concentration
		w := squeeze
		v := shift
		\end{verbatim}
		*
		* @param fXValue	The X data point at which the first derivation is needed.
		*
		* @return	The slope of the reference at the given data point.
		*/
		virtual TFitData GetSlope(TFitData fXValue)
		{
			TFitData fVal = GetValue(fXValue);

			fVal *= -(fXValue - GetCenter());
			fVal /= GetSigma() * GetSigma();

			return fVal;
		}

		/**
		* Calculates the first derivative of the function at a set of given data points.
		*
		* @param vXValues		A vector object containing the X values at which the function has to be evaluated.
		* @param vSlopeVector	A vector object which receives the resulting function values.
		*
		* @return	A reference to the slope vector object.
		*
		* @see GetSlope
		*/
		virtual CVector& GetSlopes(CVector& vXValues, CVector& vSlopeVector)
		{
			GetValues(vXValues, vSlopeVector);

			TFitData fSigma = GetSigma() * GetSigma();

			const int iXSize = vXValues.GetSize();
			int i;
			for(i = 0; i < iXSize; i++)
				vSlopeVector.SetAt(i, -vSlopeVector.GetAt(i) * (vXValues.GetAt(i) - GetCenter()) / fSigma);

			return vSlopeVector;
		}

		/**
		* Returns the basis function of the specified linear parameter.
		* A basis function is defined as the term by which the linear parameter is multiplied.
		*
		* @param fXValue	The data point at which the basis function should be determined.
		* @param iParamID	The index within the linear parameter vector of the linear parameter.
		* @param bFixedID	If TRUE the given parameter ID is the parameter ID without all fixed parameter.
		*
		* @return	The basis function in regard to the given linear parameter.
		*/
		virtual TFitData GetLinearBasisFunction(TFitData fXValue, int iParamID, bool bFixedID = true)
		{
			MATHFIT_ASSERT((bFixedID && iParamID >= 0 && iParamID < mLinearParams.GetSize()) || (!bFixedID && iParamID >= 0 && iParamID < mLinearParams.GetAllSize()));

			// get the function value without the scale factor
			TFitData fVal = fXValue - GetCenter();
			fVal *= fVal;
			fVal /= GetSigma() * GetSigma();
			fVal *= -0.5;

			return (TFitData)exp(fVal);
		}

		/**
		* Returns the center parameter of the Gauss Bell function.
		*
		* @return	The center parameter of the Gauss Bell function.
		*/
		TFitData GetCenter()
		{
			return mNonlinearParams.GetAllParameter().GetAt(0);
		}

		/**
		* Returns the sigma parameter.
		*
		* @return	The sigma parameter.
		*/
		TFitData GetSigma()
		{
			return mNonlinearParams.GetAllParameter().GetAt(1);
		}

		/**
		* Returns the amplitude scal factor.
		* If the \Ref{mNormAmp} flag is set, the amplitude is scaled so that
		* the area size of the function will always be one. Otherwise an
		* appropriate amplitude scale factor is used.
		*
		* @return	The amplitude scale factor.
		*/
		TFitData GetScale()
		{
			if(!mNormAmp)
				return mLinearParams.GetAllParameter().GetAt(0);

			TFitData fVal = fSqrPI * GetSigma();
			return 1 / fVal;
		}

		/**
		* Sets the scale value of the function.
		*
		* @param fScale	The new scale value.
		*/
		void SetScale(TFitData fScale)
		{
			mLinearParams.SetParameter(0, fScale);
		}

		/**
		* Sets the center value of the function.
		*
		* @param fCenter	The new center value.
		*/
		void SetCenter(TFitData fCenter)
		{
			mNonlinearParams.SetParameter(0, fCenter);
		}

		/**
		* Sets the sigma value of the function.
		*
		* @param fSigma	The new sigma value.
		*/
		void SetSigma(TFitData fSigma)
		{
			mNonlinearParams.SetParameter(1, fSigma);
		}

		/**
		* Resets the linear parameters to default values.
		* The default implementation sets every parameter to zero.
		*/
		virtual void ResetLinearParameter()
		{
			mLinearParams.SetSize(1);

			if(mNormAmp)
				mLinearParams.FixParameter(0, 1);
			else
				mLinearParams.ReleaseParameter(0);
			mLinearParams.SetParameter(0, 1);
		}

		/**
		* Resets the nonlinear parameters to default values.
		* The default implementation sets every parameter to zero.
		*/
		virtual void ResetNonlinearParameter()
		{
			mNonlinearParams.SetSize(2);
			mNonlinearParams.SetParameter(0, 0);
			mNonlinearParams.SetParameter(1, 1);
		}

	private:
		/**
		* Indicates wheter the area size normalization is active or not.
		*/
		bool mNormAmp;
		/**
		* Holds a needed scale factor.
		*/
		const TFitData fSqrPI;
	};
}
#endif
