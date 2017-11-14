/**
 * Contains a defintion of an aperture function object.
 *
 * @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
 * @version		1.0 @ 2002/05/23
 */
#if !defined(APERTUREFUNCTION_H_020523)
#define APERTUREFUNCTION_H_020523

#if !defined(PI)
#define PI 3.14152965
#endif

#include "ConvolutionCoreFunction.h"
#include "PolynomialFunction.h"
#include "StatisticVector.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

namespace MathFit
{
	/**
	* This object represents an aperture function.
	*
	* @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
	* @version		1.0 @ 2002/05/23
	*/
	class CApertureFunction : public IConvolutionCoreFunction
	{
	public:
		/**
		* Creates the object and sets the default parameter values.
		*
		* @param bNormAmp	If TRUE the area under the function will always be one.
		*/
		CApertureFunction(int iOrderAmp, int iOrderWidth, int iOrderAsym) :  mInverseSqrt2PI(1 / sqrt(2 * PI)), mScalePoly(iOrderAmp), mWidthPoly(iOrderWidth), mAsymmetryPoly(iOrderAsym)
		{
			mAsymmetryPoly.SetDefaultCoefficient(0, 0.5);
			mWidthPoly.SetDefaultCoefficient(0, 1);
			mScalePoly.SetDefaultCoefficient(0, 1);

			// set default parameters
			ResetNonlinearParameter();

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
			if(!mCoreValid)
				return 0;

			TFitData fResult;
			TFitData fVal = fXValue;
			if(fVal < 0)
				fResult = exp(fVal * fVal * mSigmaLeft);
			else
				fResult = exp(fVal * fVal * mSigmaRight);

			return fResult * mScaleInfinite * mScale;
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
			if(!mCoreValid)
			{
				vYTargetVector.Zero();
				return vYTargetVector;
			}

			// since we want to have a normalized function and we're doing a discretization
			// of the function, we need to count the number of samples we take. The correction
			// factor mScaleInfinite is only valid for an infinte number of samples!
			const int iXSize = vXValues.GetSize();
			TFitData fSum = 0;
			int i;
			for(i = 0; i < iXSize; i++)
			{
				TFitData fVal = vXValues.GetAt(i);
				TFitData fCurve;
				if(fVal < 0)
					fCurve = exp(fVal * fVal * mSigmaLeft);
				else
					fCurve = exp(fVal * fVal * mSigmaRight);
				fSum += fCurve;
				vYTargetVector.SetAt(i, fCurve);
			}
			vYTargetVector.Mul(mScale / fSum);

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
			if(!mCoreValid)
				return 0;

			TFitData fSlopeValue = GetValue(fXValue);

			TFitData fVal = fXValue;

			if(fVal < 0)
				fSlopeValue = -fSlopeValue * fVal / mSigmaLeft;
			else
				fSlopeValue = -fSlopeValue * fVal / mSigmaRight;

			return fSlopeValue;
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
			if(!mCoreValid)
			{
				vSlopeVector.Zero();
				return vSlopeVector;
			}

			GetValues(vXValues, vSlopeVector);

			const iXSize = vXValues.GetSize();
			int i;
			for(i = 0; i < iXSize; i++)
			{
				TFitData fVal = vXValues.GetAt(i);

				if(fVal < 0)
					vSlopeVector.SetAt(i, -vSlopeVector.GetAt(i) * fVal / mSigmaLeft);
				else
					vSlopeVector.SetAt(i, -vSlopeVector.GetAt(i) * fVal / mSigmaRight);
			}

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
			throw(EXCEPTION(CNotImplementedException));
		}

		/**
		* Returns the center parameter of the Gauss Bell function.
		*
		* @return	The center parameter of the Gauss Bell function.
		*/
		CPolynomialFunction& GetScalePolynomial()
		{
			return mScalePoly;
		}

		/**
		* Returns the sigma parameter.
		*
		* @return	The sigma parameter.
		*/
		CPolynomialFunction& GetWidthPolynomial()
		{
			return mWidthPoly;
		}

		/**
		* Returns the sigma parameter.
		*
		* @return	The sigma parameter.
		*/
		CPolynomialFunction& GetAsymmetryPolynomial()
		{
			return mAsymmetryPoly;
		}

		enum ECoefficientType
		{
			SCALE = 0,
			WIDTH,
			ASYMMETRY
		};

		bool FixParameter(ECoefficientType eType, int iCoefficientID, TFitData fValue)
		{
			switch(eType)
			{
			case SCALE:
				if(!IParamFunction::FixParameter(iCoefficientID, fValue))
					return false;

				return mScalePoly.FixParameter(iCoefficientID, fValue);
			case WIDTH:
				if(!IParamFunction::FixParameter(mScalePoly.GetCoefficients().GetSize() + iCoefficientID, fValue))
					return false;

				return mWidthPoly.FixParameter(iCoefficientID, fValue);
			case ASYMMETRY:
				if(!IParamFunction::FixParameter(mScalePoly.GetCoefficients().GetSize() + mWidthPoly.GetCoefficients().GetSize() + iCoefficientID, fValue))
					return false;

				return mAsymmetryPoly.FixParameter(iCoefficientID, fValue);
			}
			return false;
		}

		bool ReleaseParameter(ECoefficientType eType, int iCoefficientID)
		{
			switch(eType)
			{
			case SCALE:
				if(!IParamFunction::ReleaseParameter(iCoefficientID))
					return false;

				return mScalePoly.ReleaseParameter(iCoefficientID);
			case WIDTH:
				if(!IParamFunction::ReleaseParameter(mScalePoly.GetCoefficients().GetSize() + iCoefficientID))
					return false;

				return mWidthPoly.ReleaseParameter(iCoefficientID);
			case ASYMMETRY:
				if(!IParamFunction::ReleaseParameter(mScalePoly.GetCoefficients().GetSize() + mWidthPoly.GetCoefficients().GetSize() + iCoefficientID))
					return false;

				return mAsymmetryPoly.ReleaseParameter(iCoefficientID);
			}
			return false;
		}

		bool SetParameter(ECoefficientType eType, int iCoefficientID, TFitData fValue)
		{
			switch(eType)
			{
			case SCALE:
				if(!IParamFunction::SetParameter(iCoefficientID, fValue))
					return false;

				return mScalePoly.SetParameter(iCoefficientID, fValue);
			case WIDTH:
				if(!IParamFunction::SetParameter(mScalePoly.GetCoefficients().GetSize() + iCoefficientID, fValue))
					return false;

				return mWidthPoly.SetParameter(iCoefficientID, fValue);
			case ASYMMETRY:
				if(!IParamFunction::SetParameter(mScalePoly.GetCoefficients().GetSize() + mWidthPoly.GetCoefficients().GetSize() + iCoefficientID, fValue))
					return false;

				return mAsymmetryPoly.SetParameter(iCoefficientID, fValue);
			}
			return false;
		}

		/**
		* Sets the new set of linear parameters.
		* The given parameter vector is split up into the pieces needed by the references.
		*
		* @param vParam	The vector containing the new unfixed parameters.
		*
		* @return	TRUE if successful, FALSE otherwise.
		*/
		virtual bool SetNonlinearParameter(CVector& vParam)
		{
			// keep internal copy
			mNonlinearParams.SetParameters(vParam);

			int iSize, iOffset;
			iOffset = 0;
			iSize = mScalePoly.GetLinearParameter().GetSize();
			if(iSize > 0)
				mScalePoly.SetLinearParameter(vParam.SubVector(iOffset, iSize));
			iOffset += iSize;
			iSize = mWidthPoly.GetLinearParameter().GetSize();
			if(iSize > 0)
				mWidthPoly.SetLinearParameter(vParam.SubVector(iOffset, iSize));
			iOffset += iSize;
			iSize = mAsymmetryPoly.GetLinearParameter().GetSize();
			if(iSize > 0)
				mAsymmetryPoly.SetLinearParameter(vParam.SubVector(iOffset, iSize));

			return true;
		}

		/**
		* Sets the covariance matrix of the nonlinear parameters.
		*
		* @param mCovar	The covariance matrix.
		*/
		virtual void SetNonlinearCovarMatrix(CMatrix& mCovar)
		{
			mNonlinearParams.SetCovarMatrix(mCovar);

			// split up vevctor into linear and nonlinear parts.
			int iSize, iOffset;
			iOffset = 0;
			iSize = mScalePoly.GetLinearParameter().GetSize();
			if(iSize > 0)
				mScalePoly.SetLinearCovarMatrix(mCovar.SubMatrix(iOffset, iOffset, iSize, iSize));
			iOffset += iSize;
			iSize = mWidthPoly.GetLinearParameter().GetSize();
			if(iSize > 0)
				mWidthPoly.SetLinearCovarMatrix(mCovar.SubMatrix(iOffset, iOffset, iSize, iSize));
			iOffset += iSize;
			iSize = mAsymmetryPoly.GetLinearParameter().GetSize();
			if(iSize > 0)
				mAsymmetryPoly.SetLinearCovarMatrix(mCovar.SubMatrix(iOffset, iOffset, iSize, iSize));
		}

		/**
		* Sets the correlation matrix of the nonlinear parameters.
		*
		* @param mCorrel	The correlation matrix.
		*/
		virtual void SetNonlinearCorrelMatrix(CMatrix& mCorrel)
		{
			mNonlinearParams.SetCorrelMatrix(mCorrel);

			// split up vevctor into linear and nonlinear parts.
			int iSize, iOffset;
			iOffset = 0;
			iSize = mScalePoly.GetLinearParameter().GetSize();
			if(iSize > 0)
				mScalePoly.SetLinearCorrelMatrix(mCorrel.SubMatrix(iOffset, iOffset, iSize, iSize));
			iOffset += iSize;
			iSize = mWidthPoly.GetLinearParameter().GetSize();
			if(iSize > 0)
				mWidthPoly.SetLinearCorrelMatrix(mCorrel.SubMatrix(iOffset, iOffset, iSize, iSize));
			iOffset += iSize;
			iSize = mAsymmetryPoly.GetLinearParameter().GetSize();
			if(iSize > 0)
				mAsymmetryPoly.SetLinearCorrelMatrix(mCorrel.SubMatrix(iOffset, iOffset, iSize, iSize));
		}

		/**
		* Sets the error of the nonlinear parameters.
		*
		* @param vError	The error vector.
		*/
		virtual void SetNonlinearError(CVector& vError)
		{
			// keep internal copy
			mNonlinearParams.SetError(vError);

			int iSize, iOffset;
			iOffset = 0;
			iSize = mScalePoly.GetLinearParameter().GetSize();
			if(iSize > 0)
				mScalePoly.SetLinearError(vError.SubVector(iOffset, iSize));
			iOffset += iSize;
			iSize = mWidthPoly.GetLinearParameter().GetSize();
			if(iSize > 0)
				mWidthPoly.SetLinearError(vError.SubVector(iOffset, iSize));
			iOffset += iSize;
			iSize = mAsymmetryPoly.GetLinearParameter().GetSize();
			if(iSize > 0)
				mAsymmetryPoly.SetLinearError(vError.SubVector(iOffset, iSize));

		}

		/**
		* Resets the linear parameters to default values.
		* The default implementation sets every parameter to zero.
		*/
		virtual void ResetLinearParameter()
		{
			// we do not have any linear parameters
			mLinearParams.SetSize(0);
		}

		/**
		* Resets the nonlinear parameters to default values.
		* The default implementation sets every parameter to zero.
		*/
		virtual void ResetNonlinearParameter()
		{
			mScalePoly.ResetLinearParameter();
			mWidthPoly.ResetLinearParameter();
			mAsymmetryPoly.ResetLinearParameter();

			CVector vPolyCoeff;

			vPolyCoeff.Append(mScalePoly.GetLinearParameter());
			vPolyCoeff.Append(mWidthPoly.GetLinearParameter());
			vPolyCoeff.Append(mAsymmetryPoly.GetLinearParameter());
			mNonlinearParams.SetSize(vPolyCoeff.GetSize());
			mNonlinearParams.SetParameters(vPolyCoeff);

			mCoreValid = false;
		}

		virtual void SetCoreCenter(TFitData fCenter)
		{
			mXCenter = fCenter;

			TFitData fSigmaFix = mWidthPoly.GetValue(mXCenter);
			if(fSigmaFix <= 0)
			{
				mCoreValid = false;
				return;
			}

			TFitData fAsymmetry = mAsymmetryPoly.GetValue(mXCenter);
			if(fAsymmetry < 0 || fAsymmetry > 1)
			{
				mCoreValid = false;
				return;
			}

			mSigmaLeftWidth = fSigmaFix * fAsymmetry;
			TFitData fSigmaLeftSqr = mSigmaLeftWidth * mSigmaLeftWidth;
			mSigmaLeft = -0.5 / fSigmaLeftSqr;
			mSigmaRightWidth = fSigmaFix * (1 - fAsymmetry);
			TFitData fSigmaRightSqr = mSigmaRightWidth * mSigmaRightWidth;
			mSigmaRight = -0.5 / fSigmaRightSqr;

			// get the scaling
			mScale = mScalePoly.GetValue(mXCenter);

			// now correct the scaling for an infinte number of samples to ensure that the integral
			// of the function is still 1.
			mScaleInfinite = mInverseSqrt2PI * (mSigmaLeftWidth + mSigmaRightWidth) / (fSigmaLeftSqr + fSigmaRightSqr);

			mCoreValid = true;
		}

		virtual TFitData GetCoreLowBound()
		{
			// we suppose that the convolution core is zero after a distance of 3 sigma from the center
			if(mCoreValid)
				return -mSigmaLeftWidth * 3;
			return 0;
		}

		virtual TFitData GetCoreHighBound()
		{
			// we suppose that the convolution core is zero after a distance of 3 sigma from the center
			if(mCoreValid)
				return mSigmaRightWidth * 3;
			return 0;
		}

	private:
		CPolynomialFunction mScalePoly;
		CPolynomialFunction mWidthPoly;
		CPolynomialFunction mAsymmetryPoly;
		TFitData mSigmaLeft;
		TFitData mSigmaRight;
		TFitData mSigmaLeftWidth;
		TFitData mSigmaRightWidth;
		TFitData mScale;
		TFitData mScaleInfinite;
		bool mCoreValid;
		const TFitData mInverseSqrt2PI;
	};
}
#endif
