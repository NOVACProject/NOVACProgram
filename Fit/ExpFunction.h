/**
* Contains a defintion of a exponential function object.
*
* @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
* @version		1.0 @ 2001/12/10
*/
#if !defined(EXPFUNCTION_H_011206)
#define EXPFUNCTION_H_011206

#include "ParamFunction.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning (push, 3)

namespace MathFit
{
	/**
	* This object represents a exponential function.
	*
	* @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
	* @version		1.0 @ 2001/11/15
	*/
	class CExpFunction : public IParamFunction
	{
	public:
		/**
		* Creates the object and associates an exponent function object.
		*
		* @param ipfExponent	The function object that should be used as exponent function.
		*/
		CExpFunction(IParamFunction& ipfExponent) : mExponent(ipfExponent)
		{
			BuildLinearParameter();
			BuildNonlinearParameter();
		}

		/**
		* Returns the exponential value of the exponent function.
		*
		* \begin{verbatim}f(x)=exp(exponent(x))\end{verbatim}
		*
		* @param fXValue	The X data point.
		*
		* @return	The value of the function at the given data point.
		*/
		virtual TFitData GetValue(TFitData fXValue)
		{
			return (TFitData)exp(mExponent.GetValue(fXValue));
		}

		/**
		* Returns the first derivative of the function at the given data point.
		*
		* @param fXValue	The X data point at which the first derivation is needed.
		*
		* @return	The slope of the function at the given data point.
		*/
		virtual TFitData GetSlope(TFitData fXValue)
		{
			return (TFitData)exp(mExponent.GetValue(fXValue)) * mExponent.GetSlope(fXValue);
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

			// split up vector into linear and nonlinear parts.
			const int iLinSize = mExponent.GetLinearParameter().GetSize();
			if(iLinSize > 0)
				mExponent.SetLinearParameter(vParam.SubVector(0, iLinSize));

			const int iNonlinSize = mExponent.GetNonlinearParameter().GetSize();
			if(iNonlinSize > 0)
				mExponent.SetNonlinearParameter(vParam.SubVector(iLinSize, iNonlinSize));

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

			// split up vector into linear and nonlinear parts.
			const int iLinSize = mExponent.GetLinearParameter().GetSize();
			if(iLinSize > 0)
				mExponent.SetLinearCovarMatrix(mCovar.SubMatrix(0, 0, iLinSize, iLinSize));

			const int iNonlinSize = mExponent.GetNonlinearParameter().GetSize();
			if(iNonlinSize > 0)
				mExponent.SetNonlinearCovarMatrix(mCovar.SubMatrix(iLinSize, iLinSize, iNonlinSize, iNonlinSize));
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
			const int iLinSize = mExponent.GetLinearParameter().GetSize();
			if(iLinSize > 0)
				mExponent.SetLinearCorrelMatrix(mCorrel.SubMatrix(0, 0, iLinSize, iLinSize));

			const int iNonlinSize = mExponent.GetNonlinearParameter().GetSize();
			if(iNonlinSize > 0)
				mExponent.SetNonlinearCorrelMatrix(mCorrel.SubMatrix(iLinSize, iLinSize, iNonlinSize, iNonlinSize));
		}

		/**
		* Sets the error of the nonlinear parameters.
		*
		* @param vError	The error vector.
		*/
		virtual void SetNonlinearError(CVector& vError)
		{
			mNonlinearParams.SetError(vError);

			// split up vev´ctor into linear and nonlinear parts.
			const int iLinSize = mExponent.GetLinearParameter().GetSize();
			if(iLinSize > 0)
				mExponent.SetLinearError(vError.SubVector(0, iLinSize));

			const int iNonlinSize = mExponent.GetNonlinearParameter().GetSize();
			if(iNonlinSize > 0)
				mExponent.SetNonlinearError(vError.SubVector(iLinSize, iNonlinSize));
		}

		/**
		* Resets the linear parameters to default values.
		* The default implementation sets every parameter to zero.
		*/
		virtual void ResetLinearParameter()
		{
			mExponent.ResetLinearParameter();
			BuildLinearParameter();
		}

		/**
		* Resets the nonlinear parameters to default values.
		* The default implementation sets every parameter to zero.
		*/
		virtual void ResetNonlinearParameter()
		{
			mExponent.ResetNonlinearParameter();
			BuildNonlinearParameter();
		}

		virtual TFitData GetNonlinearPenalty(TFitData fChiSquare)
		{
			TFitData fSum = mExponent.GetNonlinearPenalty(fChiSquare);
			fSum += mExponent.GetLinearPenalty(fChiSquare);

			return fSum;
		}

		virtual void SetFitRange(CVector& vFitRange)
		{
			mFitRange.Copy(vFitRange);
			mExponent.SetFitRange(vFitRange);
		}

	private:
		/**
		* Builds the linear parameter vector from the operands parameters.
		*/
		void BuildLinearParameter()
		{
			// no linear parameters, since everything gets nonlinear
			mLinearParams.SetSize(0);
		}

		/**
		* Builds the nonlinear parameter vector from the operands parameters.
		*/
		void BuildNonlinearParameter()
		{
			CVector vBuf;

			vBuf.Append(mExponent.GetLinearParameter());
			vBuf.Append(mExponent.GetNonlinearParameter());
			mNonlinearParams.SetSize(vBuf.GetSize());
			mNonlinearParams.SetParameters(vBuf);
		}

		/**
		* Holds the exponent function.
		*/
		IParamFunction& mExponent;
	};
}

#pragma warning (pop)
#endif
