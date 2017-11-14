/**
* DataSet.h
*
* Contains a simple interface to handle data sets.
*
* @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
* @version		1.0 @ 2001/09/09
*/
#if !defined(DATASET_H_011206)
#define DATASET_H_011206

#include "Vector.h"
#include "Matrix.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

namespace MathFit
{
	/**
	* Interface to represent data sets. 
	* This interface is just used to represent a set of measured samples.
	*
	* @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
	* @version		1.0 @ 2001/09/09
	*/
	class IDataSet
	{
	public:
		/**
		* Creates an empty object
		*/
		IDataSet()
		{
		}

		/**
		* Constructs the object and initializes the data sets.
		*
		* @param vXValues		A vector object containing the X values of the data set.
		* @param vYValues		A vector object containing the Y values of the data set.
		* @param vError		A vector object containing the error values of the data set (sigma).
		*/
		IDataSet(CVector& vXValues, CVector& vYValues, CVector& vError)
		{
			IDataSet::SetData(vXValues, vYValues, vError);
		}

		/**
		* Constructs the object and initializes the data sets. 
		* The error is set to one.
		*
		* @param vXValues		A vector object containing the X values of the data set.
		* @param vYValues		A vector object containing the Y values of the data set.
		*/
		IDataSet(CVector& vXValues, CVector& vYValues)
		{
			IDataSet::SetData(vXValues, vYValues);
		}

		/**
		* Copies the given data into the object. 
		* The error is set to one.
		*
		* @param vXValues		A vector object containing the X values of the data set.
		* @param vYValues		A vector object containing the Y values of the data set.
		*
		* @return TRUE is successful, false if the vector sizes do not match.
		*/
		virtual bool SetData(CVector& vXValues, CVector& vYValues)
		{
			// create a neutral error vector
			CVector vError(vXValues.GetSize());
			vError.Wedge(1, 0);

			return IDataSet::SetData(vXValues, vYValues, vError);
		}

		/**
		* Copies the given data into the object.
		* The data set can by normalized by its amplitude. This might be useful to prevent numerical problems.
		* Its also possible to shift the data set to a new origin. To get more useful results during fitting a given data set
		* to a selected range of data, its useful to set the origin to either the beginning or the middle of the fitting range.
		* so the model parameters get independent of the position of the fit window.
		*
		* @param vXValues		A vector object containing the X values of the data set.
		* @param vYValues		A vector object containing the Y values of the data set.
		* @param vError			A vector object containing the <B>sigma</B> error values of the data set.
		*
		* @return TRUE is successful, false if the vector sizes do not match.
		*/
		virtual bool SetData(CVector& vXValues, CVector& vYValues, CVector& vError)
		{
			MATHFIT_ASSERT(vXValues.GetSize() == vYValues.GetSize() && vXValues.GetSize() == vError.GetSize());

			mXData.Copy(vXValues);
			mYData.Copy(vYValues);

			mError.Copy(vError);

			return true;
		}

		/**
		* Sets the X value vector.
		*
		* @param vXValues		A vector object containing the X values of the data set.
		*
		* @return TRUE is successful, FALSE otherwise.
		*/
		virtual bool SetXData(CVector& vXValues)
		{
			mXData.Copy(vXValues);

			return true;	
		}

		/**
		* Returns the vector object containing the X values of the data set.
		*
		* @return A vector object with the X values.
		*/
		virtual CVector& GetXData()
		{
			return mXData;
		}

		/**
		* Sets the Y value vector.
		*
		* @param vYValues		A vector object containing the Y values of the data set.
		*
		* @return TRUE is successful, FALSE otherwise.
		*/
		virtual bool SetYData(CVector& vYValues)
		{
			mYData.Copy(vYValues);

			return true;
		}

		/**
		* Returns the vector object containing the Y values of the data set.
		*
		* @return A vector object with the Y values.
		*/
		virtual CVector& GetYData()
		{
			return mYData;
		}

		/**
		* Sets the sigma error value vector.
		*
		* @param vError		A vector object containing the <B>sigma</B> error values of the data set.
		*
		* @return TRUE is successful, FALSE otherwise.
		*/
		virtual bool SetErrorData(CVector& vError)
		{
			mError.Copy(vError);

			return true;
		}

		/**
		* Returns the vector object containing the <B>sigma</B> error values of the data set.
		*
		* @return A vector object with the sigma error values.
		*/
		virtual CVector& GetErrorData()
		{
			return mError;
		}

	protected:
		/**
		* Stores the Y values.
		*/
		CVector mYData;
		/**
		* Stores the X values.
		*/
		CVector mXData;
		/**
		* Stores the sigma error values.
		*/
		CVector mError;
	};
}
#endif // !defined(AFX_IFUNCTION_H__7852C2C7_0389_41DB_B169_3C30252ADD47__INCLUDED_)
