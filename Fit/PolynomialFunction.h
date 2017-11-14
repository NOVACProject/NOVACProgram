/**
* Contains a polynomial defintion.
*
* @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
* @version		1.0 @ 2001/09/21
*/
#if !defined(POLY_H_011206)
#define POLY_H_011206

#include "ParamFunction.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning (push, 3)

namespace MathFit
{
	/**
	* Implements a polynomial function which parameters are all linear.
	*
	* @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
	* @version		1.0 @ 2001/09/21
	*/
	class CPolynomialFunction : public IParamFunction
	{
	public:
		CPolynomialFunction()
		{
		}

		/**
		* Create a polynomial function object, that has the given order.
		*
		* @param iOrder	The order of the polynomial (iOrder + 1 coefficients)
		*/
		CPolynomialFunction(int iOrder)
		{
			SetOrder(iOrder);
		}

		/**
		* @see IFunction::GetValue
		* @see CVector::CalcPoly
		*/
		virtual TFitData GetValue(TFitData fXValue)
		{
			return mLinearParams.GetAllParameter().CalcPoly(fXValue);
		}

		/**
		* @see IFunction::GetValues
		* @see CVector::CalcPoly
		*/
		virtual CVector& GetValues(CVector& vXValues, CVector& vYValues)
		{
			const int iXSize = vXValues.GetSize();
			int i;
			for(i = 0; i < iXSize; i++)
				vYValues.SetAt(i, mLinearParams.GetAllParameter().CalcPoly(vXValues.GetAt(i)));

			return vYValues;
		}

		/**
		* @see IFunction::GetSlope
		* @see CVector::CalcPolySlope
		*/
		virtual TFitData GetSlope(TFitData fXValue)
		{
			return mLinearParams.GetAllParameter().CalcPolySlope(fXValue);
		}

		/**
		* @see IFunction::GetValue
		* @see CVector::CalcPoly
		*/
		virtual CVector& GetSlopes(CVector& vXValues, CVector& vSlopes)
		{
			const int iXSize = vXValues.GetSize();
			int i;
			for(i = 0; i < iXSize; i++)
				vSlopes.SetAt(i, mLinearParams.GetAllParameter().CalcPolySlope(vXValues.GetAt(i)));

			return vSlopes;
		}

		/**
		* @see IParamFunction::GetLinearBasisFunction
		*/
		virtual TFitData GetLinearBasisFunction(TFitData fXValue, int iParamID, bool bFixedID = true)
		{
			MATHFIT_ASSERT((bFixedID && iParamID >= 0 && iParamID < mLinearParams.GetSize()) || (!bFixedID && iParamID >= 0 && iParamID < mLinearParams.GetAllSize()));

			// since this a polynomial, the basis function is the X value powerde by the coefficients index
			TFitData fRes = 1;
			int i;
			for(i = 0; i < iParamID; i++)
				fRes *= fXValue;

			return fRes;
		}

		/**
		* @see IParamFunction::GetLinearAMatrix
		*/
		virtual void GetLinearAMatrix(CVector& vXValues, CMatrix& mA, CVector& vB)
		{
			const int iXSize = vXValues.GetSize();
			const int iParamSize = mLinearParams.GetSize();
			int i;
			for(i = 0; i < iXSize; i++)
			{
				TFitData fX = vXValues.GetAt(i);

				// the first basis function if always 1 since its just the constant offset
				TFitData fBasisFunction = 1;

				int k;
				for(k = 0; k < iParamSize; k++)
				{
					mA.SetAt(i, k, fBasisFunction);

					// calculate the next basis function
					fBasisFunction *= fX;
				}
			}
		}

		/**
		* Returns the vector that contains the polynomial coefficients.
		* The index of the vector element also is the index of the polynomial coefficient.
		* The resulting vector includes all fixed and linked coefficients.
		*
		* @return A reference to the vector that contains the polynomial coefficients.
		*/
		CVector& GetCoefficients()
		{
			return mLinearParams.GetAllParameter();
		}

		/**
		* Sets the polynomial coefficients at once.
		* The index of the given vector elements must match the index of the polynomial coefficients.
		* {\bf The coefficient vector must include all fixed and linked coefficients!}
		*
		* @param vCoeffs	The vector that contains the new polynomial coefficients.
		*/
		void SetCoefficients(CVector& vCoeff)
		{
			mLinearParams.SetAllParameter(vCoeff);
		}

		/**
		* Returns the polynomial coefficient that has the given index.
		* The index of the coefficient is based on the indicies of {\bf ALL} parameters including
		* fixed and linked ones.
		*
		* @param iIndex	The coefficient index that's value should be returned.
		*
		* @return The value of the selected polynomial coefficient.
		*/
		TFitData GetCoefficient(int iIndex)
		{
			return GetCoefficients().GetAt(iIndex);
		}

		/**
		* Sets the polynomial coefficient of the given index.
		* The index is based on the indicies of {\bf ALL} parameters including
		* fixed and linked ones. Setting a fixed or linked parameter will fail and the
		* desired parameter will not be modified. If you want to modify a fixed parameter use
		* \Ref{IParamFunction::FixParameter} instead.
		*
		* @param iIndex	The index of the polynomial coefficient that should be modified.
		* @param fNewValue	The new coefficient value.
		*/
		void SetCoefficient(int iIndex, TFitData fNewValue)
		{
			mLinearParams.SetParameter(iIndex, fNewValue);
		}

		void SetDefaultCoefficients(CVector& vCoeff)
		{
			mLinearParams.SetAllDefaultParameters(vCoeff);
		}

		void SetDefaultCoefficient(int iIndex, TFitData fNewValue)
		{
			mLinearParams.SetAllDefaultParameter(iIndex, fNewValue);
		}

		TFitData GetCoefficientError(int iIndex)
		{
			return mLinearParams.GetError().GetAt(iIndex);
		}

		void SetOrder(int iOrder)
		{
			mLinearParams.SetSize(iOrder + 1);
		}

		int GetOrder()
		{
			return mLinearParams.GetSize() > 0 ? mLinearParams.GetSize() - 1 : 0;
		}
	};
}

#pragma warning (pop)
#endif
