/**
* Contains a defintion of a logarithm function object.
*
* @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
* @version		1.0 @ 2002/02/01
*/
#if !defined(LNFUNCTION_H_020201)
#define LNFUNCTION_H_020201

#include "ParamFunction.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning (push, 3)

namespace MathFit
{
	/**
	* This object represents a logarithm function.
	*
	* @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
	* @version		1.0 @ 2002/02/01
	*/
	class CLnFunction : public IParamFunction
	{
	public:
		/**
		* Creates the object and associates a logarithm function object.
		*
		* @param ipfOperand	The function object, which is the parameter of the logarithm function.
		*/
		CLnFunction(IParamFunction& ipfOperand) : mOperand(ipfOperand)
		{
			mInvalidOperand = false;
			BuildLinearParameter();
			BuildNonlinearParameter();
		}

		/**
		* Returns the logarithm value of the given parameter function.
		*
		* \begin{verbatim}f(x)=ln(parameter function(x))\end{verbatim}
		*
		* @param fXValue	The X data point.
		*
		* @return	The value of the function at the given data point.
		*/
		virtual TFitData GetValue(TFitData fXValue)
		{
			TFitData fOperand = mOperand.GetValue(fXValue);

			mInvalidOperand = false;
			if(fOperand <= 0)
			{
				mInvalidOperand = true;
				return MATHFIT_NEARLYZERO;
			}
			return (TFitData)log(fOperand);
		}

		virtual CVector& GetValues(CVector& vXValues, CVector& vYValues)
		{
			mOperand.GetValues(vXValues, vYValues);

			mInvalidOperand = false;
			int i;
			for(i = 0; i < vYValues.GetSize(); i++)
			{
				TFitData fVal = vYValues.GetAt(i);
				
				if(fVal <= 0)
				{
					mInvalidOperand = true;
					vYValues.SetAt(i, MATHFIT_NEARLYZERO);
				}
				else
					vYValues.SetAt(i, (TFitData)log(fVal));
			}

			return vYValues;
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
			TFitData fOperand = mOperand.GetValue(fXValue);

			mInvalidOperand = false;
			if(fOperand == 0)
			{
				mInvalidOperand = true;
				return MATHFIT_NEARLYZERO;
			}

			// the first derivative is given by f'(x)=(1/f(o(x)))*o'(x)
			return mOperand.GetSlope(fXValue) / fOperand;
		}

		virtual CVector& GetSlopes(CVector& vXValues, CVector& vSlopes)
		{
			mOperand.GetValues(vXValues, vSlopes);

			mInvalidOperand = false;
			int i;
			for(i = 0; i < vSlopes.GetSize(); i++)
			{
				TFitData fVal = vSlopes.GetAt(i);
				
				if(fVal <= 0)
				{
					mInvalidOperand = true;
					vSlopes.SetAt(i, MATHFIT_NEARLYZERO);
				}
				else
					vSlopes.SetAt(i, (TFitData)log(fVal));
			}

			return vSlopes;
		}

		virtual TFitData GetNonlinearParamSlope(TFitData fXValue, int iParamID, bool bFixedID = true)
		{
			MATHFIT_ASSERT((bFixedID && iParamID >= 0 && iParamID < mNonlinearParams.GetSize()) || (!bFixedID && iParamID >= 0 && iParamID < mNonlinearParams.GetAllSize()));

			// get original function values
			TFitData fOrig = mOperand.GetValue(fXValue);
			mInvalidOperand = false;
			if(fOrig <= 0)
			{
				mInvalidOperand = true;
				return MATHFIT_NEARLYZERO;
			}

			int iLinSize = bFixedID ? mOperand.GetLinearParameterVector().GetSize() : mOperand.GetLinearParameterVector().GetAllSize();

			TFitData fRes;
			if(iParamID < iLinSize)
				fRes = mOperand.GetLinearBasisFunction(fXValue, iParamID, bFixedID);
			else
				fRes = mOperand.GetNonlinearParamSlope(fXValue, iParamID - iLinSize, bFixedID);

			return fRes / fOrig;
		}

		virtual void GetNonlinearParamSlopes(CVector& vXValues, CVector& vSlopes, int iParamID, bool bFixedID = true)
		{
			MATHFIT_ASSERT((bFixedID && iParamID >= 0 && iParamID < mNonlinearParams.GetSize()) || (!bFixedID && iParamID >= 0 && iParamID < mNonlinearParams.GetAllSize()));

			// get original function values
			CVector vOrig(vXValues.GetSize());

			mOperand.GetValues(vXValues, vOrig);

			int iLinSize = bFixedID ? mOperand.GetLinearParameterVector().GetSize() : mOperand.GetLinearParameterVector().GetAllSize();

			if(iParamID < iLinSize)
				mOperand.GetLinearBasisFunctions(vXValues, vSlopes, iParamID, bFixedID);
			else
				mOperand.GetNonlinearParamSlopes(vXValues, vSlopes, iParamID - iLinSize, bFixedID);

			mInvalidOperand = false;
			int i;
			for(i = 0; i < vXValues.GetSize(); i++)
			{
				TFitData fVal = vOrig.GetAt(i);

				if(fVal <= 0)
				{
					mInvalidOperand = true;
					vSlopes.SetAt(i, MATHFIT_NEARLYZERO);
				}
				else
					vSlopes.SetAt(i, vSlopes.GetAt(i) / fVal);
			}
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

			// split up vev´ctor into linear and nonlinear parts.
			const int iLinSize = mOperand.GetLinearParameter().GetSize();
			if(iLinSize > 0)
				mOperand.SetLinearParameter(vParam.SubVector(0, iLinSize));

			const int iNonlinSize = mOperand.GetNonlinearParameter().GetSize();
			if(iNonlinSize > 0)
				mOperand.SetNonlinearParameter(vParam.SubVector(iLinSize, iNonlinSize));

			mInvalidOperand = false;

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

			// split up vev´ctor into linear and nonlinear parts.
			const int iLinSize = mOperand.GetLinearParameter().GetSize();
			if(iLinSize > 0)
				mOperand.SetLinearCovarMatrix(mCovar.SubMatrix(0, 0, iLinSize, iLinSize));

			const int iNonlinSize = mOperand.GetNonlinearParameter().GetSize();
			if(iNonlinSize > 0)
				mOperand.SetNonlinearCovarMatrix(mCovar.SubMatrix(iLinSize, iLinSize, iNonlinSize, iNonlinSize));
		}

		/**
		* Sets the correlation matrix of the nonlinear parameters.
		*
		* @param mCorrel	The correlation matrix.
		*/
		virtual void SetNonlinearCorrelMatrix(CMatrix& mCorrel)
		{
			mNonlinearParams.SetCorrelMatrix(mCorrel);

			// split up vector into linear and nonlinear parts.
			const int iLinSize = mOperand.GetLinearParameter().GetSize();
			if(iLinSize > 0)
				mOperand.SetLinearCorrelMatrix(mCorrel.SubMatrix(0, 0, iLinSize, iLinSize));

			const int iNonlinSize = mOperand.GetNonlinearParameter().GetSize();
			if(iNonlinSize > 0)
				mOperand.SetNonlinearCorrelMatrix(mCorrel.SubMatrix(iLinSize, iLinSize, iNonlinSize, iNonlinSize));
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
			const int iLinSize = mOperand.GetLinearParameter().GetSize();
			if(iLinSize > 0)
				mOperand.SetLinearError(vError.SubVector(0, iLinSize));

			const int iNonlinSize = mOperand.GetNonlinearParameter().GetSize();
			if(iNonlinSize > 0)
				mOperand.SetNonlinearError(vError.SubVector(iLinSize, iNonlinSize));
		}

		/**
		* Resets the linear parameters to default values.
		* The default implementation sets every parameter to zero.
		*/
		virtual void ResetLinearParameter()
		{
			mOperand.ResetLinearParameter();
			BuildLinearParameter();
		}

		/**
		* Resets the nonlinear parameters to default values.
		* The default implementation sets every parameter to zero.
		*/
		virtual void ResetNonlinearParameter()
		{
			mOperand.ResetNonlinearParameter();
			BuildNonlinearParameter();
		}

		virtual TFitData GetNonlinearPenalty(TFitData fChiSquare)
		{
			if(mInvalidOperand)
				return (TFitData)HUGE_VAL;

			TFitData fSum = mOperand.GetNonlinearPenalty(fChiSquare);
			fSum += mOperand.GetLinearPenalty(fChiSquare);

			return fSum;
		}

		virtual void SetFitRange(CVector& vFitRange)
		{
			mFitRange.Copy(vFitRange);
			mOperand.SetFitRange(vFitRange);
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
			// append the two parameter vectors
			CVector vBuf;

			vBuf.Append(mOperand.GetLinearParameter());
			vBuf.Append(mOperand.GetNonlinearParameter());
			mNonlinearParams.SetSize(vBuf.GetSize());
			mNonlinearParams.SetParameters(vBuf);
		}

		/**
		* Hold the logarithm's parameter function.
		*/
		IParamFunction& mOperand;
		bool mInvalidOperand;
	};
}

#pragma warning (pop)
#endif
