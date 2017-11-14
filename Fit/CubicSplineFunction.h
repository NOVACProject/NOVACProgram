#if !defined(CUBICSPLINEFUNCTION_H_020817)
#define CUBICSPLINEFUNCTION_H_020817

#include "../Fit/Function.h"

namespace MathFit
{
	class CCubicSplineFunction : public IFunction
	{
	public:
		/**
		* Create an empty object.
		*/
		CCubicSplineFunction()
		{
		}

		/**
		* Create a cubic spline object using the given data.
		*
		* @param vXValues			The vector containing the X values.
		* @param vYValues			The vector containing the Y values in regard to the X values.
		*/
		CCubicSplineFunction(CVector& vXValues, CVector& vYValues)
		{
			CCubicSplineFunction::SetData(vXValues, vYValues);
		}

		/**
		* Create a cubic spline object using the given data.
		*
		* @param vXValues			The vector containing the X values.
		* @param vYValues			The vector containing the Y values in regard to the X values.
		* @param vError				The vector containing the errors of the Y values. This vector will not be interpolated!
		*/
		CCubicSplineFunction(CVector& vXValues, CVector& vYValues, CVector& vError)
		{
			CCubicSplineFunction::SetData(vXValues, vYValues, vError);
		}

		/**
		* @param vXValues			The vector containing the X values.
		* @param vYValues			The vector containing the Y values in regard to the X values.
		*
		* @return	TRUE if successful, FALSE otherwise
		*/
		virtual bool SetData(CVector& vXValues, CVector& vYValues, CVector& vError)
		{
			if(!IFunction::SetData(vXValues, vYValues, vError))
				return false;

			return InitializeSpline();
		}

		/**
		* @param vXValues			The vector containing the X values.
		* @param vYValues			The vector containing the Y values in regard to the X values.
		* @param vError			The vector containing the errors of the Y values. This vector will not be interpolated!
		*
		* @return	TRUE if successful, FALSE otherwise
		*/
		virtual bool SetData(CVector& vXValues, CVector& vYValues)
		{
			if(!IFunction::SetData(vXValues, vYValues))
				return false;

			return InitializeSpline();
		}

		/**
		* Returns the value of the cubic spline at the given X value.
		*
		* @param fXValue	The X value at which to evaluate the spline
		*
		* @return	The evaluated spline value.
		*/
		virtual TFitData GetValue(TFitData fXValue)
		{
			return EvaluateSpline(fXValue);
		}

		/**
		* Calculates the function values at a set of given data points.
		*
		* @param vXValues			A vector object containing the X values at which the function has to be evaluated.
		* @param vYTargetVector	A vector object which receives the resulting function values.
		*
		* @return	A reference to the Y vector object
		*/
		virtual CVector& GetValues(CVector& vXValues, CVector& vYTargetVector)
		{
			return EvaluateSplineVector(vXValues, vYTargetVector);
		}

		/**
		* Returns the first derivative of the spline at the given data point.
		*
		* @param fXValue	The X value at which the slope is needed.
		*
		* @return	The slope of the B-Spline at the given data point.
		*/
		virtual TFitData GetSlope(TFitData fXValue)
		{
			return SlopeSpline(fXValue);
		}

		/**
		* Calculates the first derivative of the function at a set of given data points.
		*
		* @param vXValues		A vector object containing the X values at which the function has to be evaluated.
		* @param vSlopeVector	A vector object which receives the resulting function values.
		*
		* @return	A reference to the slope vector object.
		*/
		virtual CVector& GetSlopes(CVector& vXValues, CVector& vSlopeVector)
		{
			return SlopeSplineVector(vXValues, vSlopeVector);
		}

	private:
		bool InitializeSpline()
		{
			// we need at least 3 nodes
			MATHFIT_ASSERT(mXData.GetSize() >= 3);

			int i;
			TFitData fSig;
			TFitData fP;

			const int iSize = mXData.GetSize();
			if(iSize <= 3)
				return false;

			mY2ndDerivates.SetSize(iSize);

			CVector vU(iSize - 1);

			// we only use the natural cubic spline boundary conditions
			mY2ndDerivates.SetAt(0, 0);
			mY2ndDerivates.SetAt(iSize - 1, 0);

			// the loop below is highly optimized in reagrds of array access minimization
			// and calculation costs. The original algorithm can be found in Numerical Recipes.
			vU.SetAt(0, 0);
			TFitData fULast = vU.GetAt(0);
			
			TFitData fXCur = mXData.GetAt(0);
			TFitData fXNext = mXData.GetAt(1);
			TFitData fXRangeHigh = fXNext -fXCur;
			TFitData fXRangeLow;
			TFitData fXRangeAll;

			TFitData fYCur = mYData.GetAt(0);
			TFitData fYNext = mYData.GetAt(1);
			TFitData fYRangeHigh = fYNext - fYCur;
			TFitData fYRangeLow;

			TFitData fY2ndDerivePrev = mY2ndDerivates.GetAt(0);

			for(i = 1; i < iSize - 1; i++) 
			{
				// rotate the distance values
				fXCur = fXNext;
				fXNext = mXData.GetAt(i + 1);
				fXRangeLow = fXRangeHigh;
				fXRangeHigh = fXNext - fXCur;
				fXRangeAll = fXRangeLow + fXRangeHigh;

				// calculate the second derivates
				fSig = fXRangeLow / fXRangeAll;
				fP = fSig * fY2ndDerivePrev + 2;
				fY2ndDerivePrev = (fSig - 1) / fP;
				mY2ndDerivates.SetAt(i, fY2ndDerivePrev);

				fYCur = fYNext;
				fYNext = mYData.GetAt(i + 1);
				fYRangeLow = fYRangeHigh;
				fYRangeHigh = fYNext - fYCur;

				TFitData fU = fYRangeHigh / fXRangeHigh;
				fU -= fYRangeLow / fXRangeLow;
				fULast = (6 * fU / fXRangeAll - fSig * fULast) / fP;
				vU.SetAt(i, fULast);
			}
			TFitData fY2ndDeriveNext = mY2ndDerivates.GetAt(iSize - 1);
			for(i = iSize - 2; i >= 0; i--)
			{
				fY2ndDeriveNext = mY2ndDerivates.GetAt(i) * fY2ndDeriveNext + vU.GetAt(i);
				mY2ndDerivates.SetAt(i, fY2ndDeriveNext);
			}

			// do as much preprocessing as possible 
			mH.SetSize(iSize);
			mSlopeInvariant.SetSize(iSize);
			mDeltaHSquareLow.SetSize(iSize);
			mDeltaHSquareHigh.SetSize(iSize);
			mSlopeDeltaHSquareLow.SetSize(iSize);
			mSlopeDeltaHSquareHigh.SetSize(iSize);

			mH.SetAt(0, 0);
			mSlopeInvariant.SetAt(0, 0);
			mDeltaHSquareLow.SetAt(0, 0);
			mDeltaHSquareHigh.SetAt(0, 0);
			mSlopeDeltaHSquareLow.SetAt(0, 0);
			mSlopeDeltaHSquareHigh.SetAt(0, 0);
			for(i = 1; i < iSize; i++)
			{
				TFitData fTemp = mXData.GetAt(i) - mXData.GetAt(i - 1);
				mH.SetAt(i, fTemp);

				TFitData fTempSlope = fTemp / (TFitData)6.0;
				mSlopeInvariant.SetAt(i, (mYData.GetAt(i) - mYData.GetAt(i - 1)) / fTemp);

				fTemp = fTemp * fTemp / (TFitData)6.0;

				mDeltaHSquareLow.SetAt(i, mY2ndDerivates.GetAt(i - 1) * fTemp);
				mDeltaHSquareHigh.SetAt(i, mY2ndDerivates.GetAt(i) * fTemp);

				mSlopeDeltaHSquareLow.SetAt(i, mY2ndDerivates.GetAt(i - 1) * fTempSlope);
				mSlopeDeltaHSquareHigh.SetAt(i, mY2ndDerivates.GetAt(i) * fTempSlope);
			}

			return true;
		}

		TFitData EvaluateSpline(TFitData fXValue)
		{
			// check wheter we have a valid spline
			MATHFIT_ASSERT(mY2ndDerivates.GetSize() >= 3);

			// get indicies of the tabulated function coefficients that contain the correct interpolation polynomial
			int iIndexLow = mXData.FindIndex(fXValue, CVector::LESSEQUAL);

			// correct the indicies, if necessary
			if(iIndexLow < 0)
				iIndexLow = 0;
			int iIndexHigh = iIndexLow + 1;
			if(iIndexHigh >= mXData.GetSize())
			{
				iIndexHigh = mXData.GetSize() - 1;
				iIndexLow = iIndexHigh - 1;
			}

			// difference between the data points
			const TFitData fH = mXData.GetAt(iIndexHigh) - mXData.GetAt(iIndexLow);

			// linear interpolation coefficient
			const TFitData fA = (mXData.GetAt(iIndexHigh) - fXValue) / fH;
			const TFitData fB = (1 - fA);

			// first interpolate linearily
			TFitData fResult = fA * mYData.GetAt(iIndexLow) + fB * mYData.GetAt(iIndexHigh);

			// add the polynomial's coefficients to fulfill the second derivative constrain
			fResult += ((fA * fA * fA - fA) * mY2ndDerivates.GetAt(iIndexLow) + (fB * fB * fB - fB) * mY2ndDerivates.GetAt(iIndexHigh)) * fH * fH / (TFitData)6.0;

			return fResult;
		}

		CVector& EvaluateSplineVector(CVector& vXData, CVector& vYData)
		{
			// check wheter we have a valid spline
			MATHFIT_ASSERT(mY2ndDerivates.GetSize() >= 3);

			// get indicies of the tabulated function coefficients that contain the correct interpolation polynomial
			int iIndexLow = mXData.FindIndex(vXData.GetAt(0), CVector::LESSEQUAL);

			const int iMaxIndex = mXData.GetSize() - 1;

			// correct the indicies, if necessary
			if(iIndexLow < 0)
				iIndexLow = 0;
			int iIndexHigh = iIndexLow + 1;
			if(iIndexHigh > iMaxIndex)
			{
				iIndexHigh = iMaxIndex;
				iIndexLow = iIndexHigh - 1;
			}

			int iCount = 0;
			const int iXEvalSize = vXData.GetSize();
			do
			{
				// loop invariants
				const TFitData fXHigh = mXData.GetAt(iIndexHigh);
				const TFitData fYLow = mYData.GetAt(iIndexLow);
				const TFitData fYHigh = mYData.GetAt(iIndexHigh);

				// get preprepared coefficients
				const TFitData fH = mH.GetAt(iIndexHigh);

				const TFitData fYDeltaLow = mDeltaHSquareLow.GetAt(iIndexHigh);
				const TFitData fYDeltaHigh = mDeltaHSquareHigh.GetAt(iIndexHigh);

				// repeat until we cross the current polynomial's boundaries
				TFitData fXData = vXData.GetAt(iCount);

				while(fXData < fXHigh || iIndexHigh >= iMaxIndex)
				{
					// linear interpolation coefficient
					const TFitData fA = (fXHigh - fXData) / fH;
					const TFitData fB = (1 - fA);

					// first interpolate linearily
					TFitData fResult = fA * fYLow + fB * fYHigh;

					// add the polynomial's coefficients to fulfill the second derivative constrain
					fResult += ((fA * fA * fA - fA) * fYDeltaLow + (fB * fB * fB - fB) * fYDeltaHigh);
					vYData.SetAt(iCount++, fResult);

					if(iCount >= iXEvalSize)
						return vYData;

					fXData = vXData.GetAt(iCount);
				}

				// find next valid indicies
				do
				{
					iIndexHigh++;
					if(iIndexHigh > iMaxIndex)
					{
						iIndexHigh = iMaxIndex;
						break;
					}
				}while(mXData.GetAt(iIndexHigh) < fXData);
				iIndexLow = iIndexHigh - 1;
			}while(iCount < iXEvalSize);

			return vYData;
		}

		TFitData SlopeSpline(TFitData fXValue)
		{
			// check wheter we have a valid spline
			MATHFIT_ASSERT(mY2ndDerivates.GetSize() >= 3);

			// get indicies of the tabulated function coefficients that contain the correct interpolation polynomial
			int iIndexLow = mXData.FindIndex(fXValue, CVector::LESSEQUAL);

			// correct the indicies, if necessary
			if(iIndexLow < 0)
				iIndexLow = 0;
			int iIndexHigh = iIndexLow + 1;
			if(iIndexHigh >= mXData.GetSize())
			{
				iIndexHigh = mXData.GetSize() - 1;
				iIndexLow = iIndexHigh - 1;
			}

			// difference between the data points
			const TFitData fH = mXData.GetAt(iIndexHigh) - mXData.GetAt(iIndexLow);

			// linear interpolation coefficient
			const TFitData fA = (mXData.GetAt(iIndexHigh) - fXValue) / fH;
			const TFitData fB = (1 - fA);

			// calculate the first derivative
			TFitData fResult = (mYData.GetAt(iIndexHigh) - mYData.GetAt(iIndexLow)) / fH;
			fResult += (((3 * fB * fB - 1) * mY2ndDerivates.GetAt(iIndexHigh)) - ((3 * fA * fA - 1) * mY2ndDerivates.GetAt(iIndexLow))) * fH / (TFitData)6.0;

			return fResult;
		}

		CVector& SlopeSplineVector(CVector& vXData, CVector& vYData)
		{
			// check wheter we have a valid spline
			MATHFIT_ASSERT(mY2ndDerivates.GetSize() >= 3);

			// get indicies of the tabulated function coefficients that contain the correct interpolation polynomial
			int iIndexLow = mXData.FindIndex(vXData.GetAt(0), CVector::LESSEQUAL);

			const int iMaxIndex = mXData.GetSize() - 1;

			// correct the indicies, if necessary
			if(iIndexLow < 0)
				iIndexLow = 0;
			int iIndexHigh = iIndexLow + 1;
			if(iIndexHigh > iMaxIndex)
			{
				iIndexHigh = iMaxIndex;
				iIndexLow = iIndexHigh - 1;
			}

			int iCount = 0;
			const int iXEvalSize = vXData.GetSize();
			do
			{
				// loop invariants
				const TFitData fXHigh = mXData.GetAt(iIndexHigh);

				// get preprepared coefficients
				const TFitData fH = mH.GetAt(iIndexHigh);
				const TFitData fResultInvariant = mSlopeInvariant.GetAt(iIndexHigh);

				const TFitData fYDeltaLow = mSlopeDeltaHSquareLow.GetAt(iIndexHigh);
				const TFitData fYDeltaHigh = mSlopeDeltaHSquareHigh.GetAt(iIndexHigh);

				TFitData fXData = vXData.GetAt(iCount);

				// repeat until we cross the current polynomial's boundaries
				while(fXData < fXHigh || iIndexHigh >= iMaxIndex)
				{
					// linear interpolation coefficient
					const TFitData fA = (fXHigh - fXData) / fH;
					const TFitData fB = (1 - fA);

					// calculate the first derivative
					TFitData fResult = fResultInvariant + (((3 * fB * fB - 1) * fYDeltaHigh) - ((3 * fA * fA - 1) * fYDeltaLow));
					vYData.SetAt(iCount++, fResult);

					if(iCount >= iXEvalSize)
						return vYData;

					fXData = vXData.GetAt(iCount);
				}

				// find next valid indicies
				do
				{
					iIndexHigh++;
					if(iIndexHigh > iMaxIndex)
					{
						iIndexHigh = iMaxIndex;
						break;
					}
				}while(mXData.GetAt(iIndexHigh) < fXData);
				iIndexLow = iIndexHigh - 1;
			}while(iCount < iXEvalSize);

			return vYData;
		}

		CVector mY2ndDerivates;

		CVector mH;
		CVector mSlopeInvariant;
		CVector mDeltaHSquareLow;
		CVector mDeltaHSquareHigh;
		CVector mSlopeDeltaHSquareLow;
		CVector mSlopeDeltaHSquareHigh;
	};
}

#endif
