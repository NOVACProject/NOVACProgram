/**
* Contains a cubic B-Spline defintion.
*
* @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
* @version		1.0 @ 2001/09/21
*/
#if !defined(CUBICBSPLINE_H_011206)
#define CUBICBSPLINE_H_011206

#include "Function.h"
#include "BSplineImpl.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

namespace MathFit
{
#undef MATHFIT_USEDOUBLESPLINETYPE

	/**
	* Exception class indicating a failed construction of a B-Spline object.
	*
	* @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
	* @version		1.0 @ 2001/09/21
	*/
	class CCubicBSplineFailedException : public CFitException
	{
	public:
		CCubicBSplineFailedException(int iLine, const char* szModule, const char* szName) : CFitException(iLine, szModule, szName, "BSpline initialization failed for reference spectrum!") {};
	};

	/**
	* This object is a cubic B-Spline function.
	* It uses the B-Spline implementation of Gary Granger, \URL[University Corporation for Atmospheric Research, UCAR]{http://www.atd.ucar.edu}
	*
	* @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
	* @version		1.0 @ 2001/09/21
	*/
	class CCubicBSplineFunction : public IFunction
	{
	public:
		/**
		* Create an empty object.
		*/
		CCubicBSplineFunction()
		{
			InitClear();
		}

		/**
		* Create a cubic B-Spline object using the given data.
		*
		* @param vXValues			The vector containing the X values.
		* @param vYValues			The vector containing the Y values in regard to the X values.
		* @param fDownSampleSize	Defines the number of data nodes that are used to build one B-Spline node. Zero means no downsampling. Default: 0
		*
		* @exception	CBSplineFailed
		*/
		CCubicBSplineFunction(CVector& vXValues, CVector& vYValues, TFitData fDownSampleSize = 0)
		{
			InitClear();

			SetDownSampleSize(fDownSampleSize);

			if(!CCubicBSplineFunction::SetData(vXValues, vYValues))
				throw(EXCEPTION(CCubicBSplineFailedException));
		}

		/**
		* Create a cubic B-Spline object using the given data.
		*
		* @param vXValues			The vector containing the X values.
		* @param vYValues			The vector containing the Y values in regard to the X values.
		* @param vError			The vector containing the errors of the Y values. This vector will not be interpolated!
		* @param fDownSampleSize	Defines the number of data nodes that are used to build one B-Spline node. Zero means no downsampling. Default: 0
		*
		* @exception	CBSplineFailed
		*/
		CCubicBSplineFunction(CVector& vXValues, CVector& vYValues, CVector& vError, TFitData fDownSampleSize = 0)
		{
			InitClear();

			SetDownSampleSize(fDownSampleSize);

			if(!CCubicBSplineFunction::SetData(vXValues, vYValues, vError))
				throw(EXCEPTION(CCubicBSplineFailedException));
		}

		/**
		* Release the B-Spline object.
		*/
		~CCubicBSplineFunction()
		{
			ClearBSpline();
		}

		/**
		* Sets the given values into the apropriate vector members and creates the
		* B-Spline object.
		*
		* @param vXValues			The vector containing the X values.
		* @param vYValues			The vector containing the Y values in regard to the X values.
		*
		* @return	TRUE if successful, FALSE otherwise
		*/
		virtual bool SetData(CVector& vXValues, CVector& vYValues, CVector& vError)
		{
			if(!IFunction::SetData(vXValues, vYValues, vError))
				return false;

			return MakeBSpline(mDownSampleSize);
		}

		/**
		* Sets the given values into the apropriate vector members and creates the
		* B-Spline object.
		*
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

			return MakeBSpline(mDownSampleSize);
		}

		/**
		* Returns the value of the B-Spline at the given X value.
		*
		* @param fXValue	The X value at which to evaluate the B-Spline
		*
		* @return	The evaluated B-Spline value.
		*/
		virtual TFitData GetValue(TFitData fXValue)
		{
			return mBspSpec->evaluate(fXValue);
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
			const int iXSize = vXValues.GetSize();
			int i;
			for(i = 0; i < iXSize; i++)
				vYTargetVector.SetAt(i, mBspSpec->evaluate(vXValues.GetAt(i)));

			return vYTargetVector;
		}

		/**
		* Returns the first derivative of the B-Spline at the given data point.
		*
		* @param fXValue	The X value at which the slope is needed.
		*
		* @return	The slope of the B-Spline at the given data point.
		*/
		virtual TFitData GetSlope(TFitData fXValue)
		{
			return mBspSpec->slope(fXValue);
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
			const int iXSize = vXValues.GetSize();
			int i;
			for(i = 0; i < iXSize; i++)
				vSlopeVector.SetAt(i, mBspSpec->slope(vXValues.GetAt(i)));

			return vSlopeVector;
		}

		void SetDownSampleSize(double fDownSampleSize)
		{
			mDownSampleSize = fDownSampleSize;
		}

		double GetDownSampleSize()
		{
			return mDownSampleSize;
		}

	private:
		/**
		* Initializes a empty B-Spline.
		*
		* @return	TRUE if successful, FALSE otherwise
		*/
		bool InitClear()
		{
			mBspSpec = NULL;
			mDownSampleSize = 0;

			return true;
		}

		/**
		* Clears the whole B-Spline settings and releases the internal used B-Spline object.
		*
		* @return	TRUE if successful, FALSE otherwise.
		*/
		bool ClearBSpline()
		{
			if(mBspSpec)
				delete(mBspSpec);
			mBspSpec = NULL;

			return true;
		}

		/**
		* Creates a B-Spline object from the already set X and Y values.
		* To downsample a function the \Ref{fDownSampleSize} parameter can be used.
		* If this parameter is zero, no downsampling is applied. Otherwise \Ref{fDownSampleSize}
		* of data points are merged to make up one single node of the B-Spline. So 
		* \Ref{fDownSampleSize} data points are averaged for downsampling.
		*
		* @param fDownSampleSize	Defines the number of data nodes that are used to build one B-Spline node. Zero means no downsampling. Default: 0
		*
		* @return	TRUE if successful, FALSE otherwise
		*/
		bool MakeBSpline(TFitData fDownSampleSize = 0)
		{
			// clear BSpline object
			ClearBSpline();

			// disable debugging
			BSpline<double>::Debug(false);			// no debugging info please

			const int iXSize = mXData.GetSize();

			// check wheter we have currently the double type active
#if defined(MATHFIT_FITDATAFLOAT) && defined(MATHFIT_USEDOUBLESPLINETYPE)
			// otherwise convert the fit data to double values
			double* fXData = new double[mXData.GetSize()];
			double* fYData = new double[mYData.GetSize()];
			int i;
			for(i = 0; i < iXSize; i++)
			{
				fXData[i] = (double)mXData.GetAt(i);
				fYData[i] = (double)mYData.GetAt(i);
			}

			// create new BSpline object
			// we want the second derivative at the endpoints to be zero
			mBspSpec = new BSpline<double>(fXData, iXSize, fYData, fDownSampleSize, BSplineBase<double>::BC_ZERO_SECOND);

			delete(fXData);
			delete(fYData);
#else
			// create new BSpline object
			// we want the second derivative at the endpoints to be zero
			mBspSpec = new BSpline<TFitData>(mXData.GetSafePtr(), iXSize, mYData.GetSafePtr(), fDownSampleSize, BSplineBase<double>::BC_ZERO_SECOND);
#endif

			// check wheter the BSpline works or not
			if(!mBspSpec->ok())
			{
				ClearBSpline();
				return false;
			}

			return true;
		}

	private:
		/**
		* Stores the internaly used B-Spline object.
		*/
#if defined(USEDOUBLESPLINETYPE)
		BSpline<double>* mBspSpec;
#else
		BSpline<TFitData>* mBspSpec;
#endif
		double mDownSampleSize;
	};
}
#endif
