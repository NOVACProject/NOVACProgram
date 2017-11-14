/**
* Contains a defintion of a negator function object.
*
* @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
* @version		1.0 @ 2002/02/01
*/
#if !defined(NEGATEFUNCTION_H_020201)
#define NEGATEFUNCTION_H_020201

#include "ParamFunction.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

namespace MathFit
{
	/**
	* This object represents a negator function.
	*
	* @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
	* @version		1.0 @ 2002/02/01
	*/
	class CNegateFunction : public IParamFunction
	{
	public:
		/**
		* Creates the object and sets the function object that should be negated.
		*
		* @param ipfNegate	The function object, which is the parameter of the negator function.
		*/
		CNegateFunction(IParamFunction& ipfNegate) : mNegate(ipfNegate)
		{
			BuildLinearParameter();
			BuildNonlinearParameter();
		}

		/**
		* Returns the negative value of the given parameter function.
		*
		* \begin{verbatim}f(x)=-(parameter function(x))\end{verbatim}
		*
		* @param fXValue	The X data point.
		*
		* @return	The value of the function at the given data point.
		*/
		virtual TFitData GetValue(TFitData fXValue)
		{
			return -mNegate.GetValue(fXValue);
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
			return -mNegate.GetSlope(fXValue);
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
			return -mNegate.GetLinearBasisFunction(fXValue, iParamID, bFixedID);
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

			mNegate.SetNonlinearParameter(vParam);

			return true;
		}

		/**
		* Sets the linear parameters of the function object.
		*
		* @param vLinParam	A vector object containing the new unfixed linear parameters.
		*
		* @return TRUE if successful, FALSE if the vector sizes do not match.
		*/
		virtual bool SetLinearParameter(CVector& vLinParam)
		{
			mLinearParams.SetParameters(vLinParam);

			mNegate.SetLinearParameter(vLinParam);

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

			mNegate.SetNonlinearCovarMatrix(mCovar);
		}

		/**
		* Sets the correlation matrix of the nonlinear parameters.
		*
		* @param mCorrel	The correlation matrix.
		*/
		virtual void SetNonlinearCorrelMatrix(CMatrix& mCorrel)
		{
			mNonlinearParams.SetCorrelMatrix(mCorrel);

			mNegate.SetNonlinearCorrelMatrix(mCorrel);
		}

		/**
		* Sets the error of the nonlinear parameters.
		*
		* @param vError	The error vector.
		*/
		virtual void SetNonlinearError(CVector& vError)
		{
			mNonlinearParams.SetError(vError);

			mNegate.SetNonlinearError(vError);
		}

		/**
		* Sets the covariance matrix of the linear parameters.
		*
		* @param mCovar	The covariance matrix.
		*/
		virtual void SetLinearCovarMatrix(CMatrix& mCovar)
		{
			mLinearParams.SetCovarMatrix(mCovar);

			mNegate.SetLinearCovarMatrix(mCovar);
		}

		/**
		* Sets the correlation matrix of the linear parameters.
		*
		* @param mCorrel	The correlation matrix.
		*/
		virtual void SetLinearCorrelMatrix(CMatrix& mCorrel)
		{
			mLinearParams.SetCorrelMatrix(mCorrel);

			mNegate.SetLinearCorrelMatrix(mCorrel);
		}

		/**
		* Sets the error of the linear parameters.
		*
		* @param vError	The error vector.
		*/
		virtual void SetLinearError(CVector& vError)
		{
			mLinearParams.SetError(vError);

			mNegate.SetLinearError(vError);
		}

		/**
		* Resets the linear parameters to default values.
		* The default implementation sets every parameter to zero.
		*/
		virtual void ResetLinearParameter()
		{
			mNegate.ResetLinearParameter();
			BuildLinearParameter();
		}

		/**
		* Resets the nonlinear parameters to default values.
		* The default implementation sets every parameter to zero.
		*/
		virtual void ResetNonlinearParameter()
		{
			mNegate.ResetNonlinearParameter();
			BuildNonlinearParameter();
		}

		virtual TFitData GetLinearPenalty(TFitData fChiSquare)
		{
			return mNegate.GetLinearPenalty(fChiSquare);
		}

		virtual TFitData GetNonlinearPenalty(TFitData fChiSquare)
		{
			return mNegate.GetNonlinearPenalty(fChiSquare);
		}

		virtual void SetFitRange(CVector& vFitRange)
		{
			mFitRange.Copy(vFitRange);
			mNegate.SetFitRange(vFitRange);
		}

	private:
		/**
		* Builds the linear parameter vector from the operands parameters.
		*/
		void BuildLinearParameter()
		{
			// get the original parameters
			mLinearParams.SetSize(mNegate.GetLinearParameter().GetSize());
			mLinearParams.SetParameters(mNegate.GetLinearParameter());
		}

		/**
		* Builds the nonlinear parameter vector from the operands parameters.
		*/
		void BuildNonlinearParameter()
		{
			// get the original parameters
			mNonlinearParams.SetSize(mNegate.GetNonlinearParameter().GetSize());
			mNonlinearParams.SetParameters(mNegate.GetNonlinearParameter());
		}

		/**
		* Holds the negator's parameter function.
		*/
		IParamFunction& mNegate;
	};
}
#endif
