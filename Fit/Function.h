/**
* Function.h
*
* Contains the function interface and its default implementation.
*
* @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
* @version		1.0 @ 2001/09/09
*/
#if !defined(FUNCTION_H_011206)
#define FUNCTION_H_011206

#include "Vector.h"
#include "Matrix.h"
#include "DataSet.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

namespace MathFit
{
	/**
	* Interface to represent a continous function. 
	* The interface inherits the IDataPoints interface, so also basic data storage functionality is available.
	*
	* @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
	* @version		1.0 @ 2001/09/09
	*/
	class IFunction : public IDataSet
	{
	public:
		/**
		* Creates an empty object
		*/
		IFunction()
		{
		}

		/**
		* Constructs the object and stores the given data into the internal buffers.
		*
		* @param vXValues		A vector object containing the X values of the data set.
		* @param vYValues		A vector object containing the Y values of the data set.
		*
		* @see	IDataPoints constructor
		*/
		IFunction(CVector& vXValues, CVector& vYValues) : IDataSet(vXValues, vYValues)
		{
		}

		/**
		* Constructs the object and stores the given data into the internal buffers.
		*
		* @param vXValues		A vector object containing the X values of the data set.
		* @param vYValues		A vector object containing the Y values of the data set.
		* @param vError			A vector object containing the <B>sigma</B> error values of the data set.
		*
		* @see	IDataPoints constructor
		*/
		IFunction(CVector& vXValues, CVector& vYValues, CVector& vError) : IDataSet(vXValues, vYValues, vError)
		{
		}

		/**
		* Calculates the function value at a given point. 
		* This method is declared abstract, so it must be
		* implemented by any inherited object.
		*
		* @param fXValue	The X value at which the function has be evaluated.
		*
		* @return The function value at the given point.
		*/
		virtual TFitData GetValue(TFitData fXValue) = 0;

		/**
		* Calculates the function values at a set of given data points.
		*
		* @param vXValues			A vector object containing the X values at which the function has to be evaluated.
		* @param vYTargetVector	A vector object which receives the resulting function values.
		*
		* @return	A reference to the Y vector object.
		*/
		virtual CVector& GetValues(CVector& vXValues, CVector& vYTargetVector)
		{
			const int iXSize = vXValues.GetSize();
			int i;
			for(i = 0; i < iXSize; i++)
				vYTargetVector.SetAt(i, GetValue(vXValues.GetAt(i)));

			return vYTargetVector;
		}

		/**
		* Caculates the first derivative of the function at a given data point. 
		* This method is declared as abstract, so it must be implemented by any inherited object.
		*
		* @param fXValue	The X value at which the function has be evaluated.
		*
		* @return The function slope at the given point.
		*/
		virtual TFitData GetSlope(TFitData fXValue) = 0;

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
			const int iXSize = vXValues.GetSize();
			int i;
			for(i = 0; i < iXSize; i++)
				vSlopeVector.SetAt(i, GetSlope(vXValues.GetAt(i)));

			return vSlopeVector;
		}

		/**
		* Returns the sigma error of the function value at the given point.
		* If no direct error information is available for the given data point
		* a linear interpolation will be done to determine the error.
		*
		* @param fXValue	The data point at which the function error is needed.
		*
		* @return	The function's sigma error in respect to the given data point.
		*/
		virtual TFitData GetFunctionError(TFitData fXValue)
		{
			if(mXData.GetSize() == 0)
				return 1;

			// search for the valid index
			int i = 0;
			while(mXData.GetAt(i) < fXValue)
				++i;

			// no valid error available
			if(i >= mXData.GetSize())
				return 1;

			// if we have a direct match, just return the stored error value
			if(mXData.GetAt(i) == fXValue)
				return mError.GetAt(i);

			// if we can interpolate, return the error as the weighted mean of the two affected error values.
			if(i < mXData.GetSize() - 1)
			{
				int iLow = i;
				int iHigh = i + 1;

				TFitData fWeight = (fXValue - mXData.GetAt(iLow)) / (mXData.GetAt(iHigh) - mXData.GetAt(iLow));

				return (1 - fWeight) * mError.GetAt(iLow) + fWeight * mError.GetAt(iHigh);
			}
			else
				return mError.GetAt(i);
		}

		/**
		* Returns the sigma error of the function value at the given points.
		* If no direct error information is available for the given data point
		* a linear interpolation will be done to determine the error.
		*
		* @param vXValues	A vector that contains the data points at which the function errors are needed.
		* @param vError		A vector that will hold the error values.
		*
		* @return	A reference to a vector that contains the function's sigma errors in regard to the given data points.
		*/
		virtual CVector& GetFunctionErrors(CVector& vXValues, CVector& vError)
		{
			vError.SetSize(vXValues.GetSize());

			if(mXData.GetSize() == 0)
			{
				vError.Wedge(1, 0);
				return vError;
			}

			const int iErrorSize = vError.GetSize();
			const int iXSizeBase = mXData.GetSize();
			int i, j;
			for(i = j = 0; j < iErrorSize; j++)
			{
				while(mXData.GetAt(i) < vXValues.GetAt(j))
					++i;

				// no valid error available
				if(i >= iXSizeBase)
				{
					vError.SetAt(j, 1);
					continue;
				}

				// if we have a direct match, just return the stored error value
				if(mXData.GetAt(i) == vXValues.GetAt(j))
				{
					vError.SetAt(j, mError.GetAt(i));
					continue;
				}

				// if we can interpolate, return the error as the weighted mean of the two affected error values.
				if(i < iXSizeBase - 1)
				{
					int iLow = i;
					int iHigh = i + 1;

					TFitData fWeight = (vXValues.GetAt(j) - mXData.GetAt(iLow)) / (mXData.GetAt(iHigh) - mXData.GetAt(iLow));

					vError.SetAt(j, (1 - fWeight) * mError.GetAt(iLow) + fWeight * mError.GetAt(iHigh));
				}
				else
					vError.SetAt(j, mError.GetAt(i));
			}
			return vError;
		}
	};
}
#endif // !defined(AFX_IFUNCTION_H__7852C2C7_0389_41DB_B169_3C30252ADD47__INCLUDED_)
