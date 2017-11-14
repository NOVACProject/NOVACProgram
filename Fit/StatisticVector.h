/**
 * Contains the CStatVector class.
 *
 * @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
 * @version		1.0 @ 2001/12/06
 */
#if !defined(STATVECTOR_H_010908) 
#define STATVECTOR_H_010908

#include "Vector.h"

namespace MathFit
{
	/**
	* This class extends the basic vector class with some statistical methods.
	*
	* @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
	* @version		1.0 @ 2001/12/06
	*/
	class CStatisticVector : public CVector
	{
	public:
		/**
		* Creates an empty vector object.
		*/
		CStatisticVector()
			: CVector()
		{
		}

		/**
		* Create a new vector object and fills it with the content of the given vector.
		*
		* @param vRight	The originating vector object.
		*/
		CStatisticVector(const CVector &vRight)
			: CVector(vRight)
		{
		}

		/**
		* Create a vector object with the given size.
		*
		* @param iSize		The number of elements in the vector.
		*/
		CStatisticVector(int iSize)
			: CVector(iSize)
		{
		}

		/**
		* Create a vector from a given data object using the offset and length specified.
		*
		* @param vSecond	The vector object from which we should create a subvector.
		* @param iOffset	The offset from which the subvector should start
		* @param iSize		The number of elements in the subvector.
		*/
		CStatisticVector(CVector& vSecond, int iOffset, int iSize)
			: CVector(vSecond, iOffset, iSize)
		{
		}

		/**
		* Create a vector from a given data array.
		*
		* @param fData			The array containing the data elements.
		* @param iSize			The number of elements in the array.
		* @param bAutoRelease	If TRUE the array is freed during destruction of the vector.
		*/
		CStatisticVector(TFitData* fData, int iSize, bool bAutoRelease = true)
			: CVector(fData, iSize, bAutoRelease)
		{
		}

		/**
		* Sorts the vector elements in the specified order.
		* A simple bubble sort algorithm is used.
		* A range can be specified.
		*
		* @param bAscend	If TRUE the elements are sorted in ascending order, otherwise in descending order.
		* @param iOffset	The index of the first element starting to search for the minimum.
		* @param iLength	The number of elements to inspect.
		*
		* @return	A reference to the sorted vector.
		*/

		CStatisticVector& Sort(bool bAscend = true, int iOffset = 0, int iLength = -1)
		{
			if(iLength <= 0)
				iLength = mLength;

			MATHFIT_ASSERT(iOffset >= 0 && (iOffset + iLength) <= mLength && iLength > 0);

			int iOffsetStop = iOffset + iLength;
			int i, j;
			if(bAscend)
			{
				for(i = iOffset; i < iOffsetStop; i++)
				{
					for(j = i + 1; j < iOffsetStop; j++)
						if(GetAt(i) > GetAt(j))
						{
							TFitData fTemp = GetAt(i);
							SetAt(i, GetAt(j));
							SetAt(j, fTemp);
						}
				}
			}
			else
			{
				for(i = iOffset; i < iOffsetStop; i++)
				{
					for(j = i + 1; j < iOffsetStop; j++)
						if(GetAt(i) < GetAt(j))
						{
							TFitData fTemp = GetAt(i);
							SetAt(i, GetAt(j));
							SetAt(j, fTemp);
						}
				}
			}

			return *this;
		}

		/**
		* Builds the power of the vector elements.
		* The power is determined by building the square of each vector element.
		* A range can be specified.
		*
		* @param iOffset	The index of the first element starting to search for the minimum.
		* @param iLength	The number of elements to inspect.
		*
		* @return	A reference to the vector.
		*/
		CStatisticVector& Power(int iOffset = 0, int iLength = -1)
		{
			if(iLength <= 0)
				iLength = mLength;

			MATHFIT_ASSERT(iOffset >= 0 && (iOffset + iLength) <= mLength && iLength > 0);

			int iOffsetStop = iOffset + iLength;
			int i;
			for(i = iOffset; i < iOffsetStop; i++)
				SetAt(i, GetAt(i) * GetAt(i));

			return *this;
		}

		/**
		* Builds the norm of the vector elements.
		* The norm is determined by building the norm of each vector element.
		* A range can be specified.
		*
		* @param iOffset	The index of the first element starting to search for the minimum.
		* @param iLength	The number of elements to inspect.
		*
		* @return	A reference to the vector.
		*/
		CStatisticVector& Norm(int iOffset = 0, int iLength = -1)
		{
			if(iLength <= 0)
				iLength = mLength;

			MATHFIT_ASSERT(iOffset >= 0 && (iOffset + iLength) <= mLength && iLength > 0);

			int iOffsetStop = iOffset + iLength;
			int i;
			for(i = iOffset; i < iOffsetStop; i++)
				SetAt(i, (TFitData)fabs(GetAt(i)));

			return *this;
		}

		/**
		* Returns the sum of all vector elements.
		* A range can be specified.
		*
		* @param iOffset	The index of the first element.
		* @param iLength	The number of elements to inspect.
		*
		* @return	The sum of the vector elements.
		*/
		TFitData Sum(int iOffset = 0, int iLength = -1)
		{
			if(iLength <= 0)
				iLength = mLength;

			MATHFIT_ASSERT(iOffset >= 0 && (iOffset + iLength) <= mLength && iLength > 0);

			int iOffsetStop = iOffset + iLength;
			TFitData fSum = 0;
			int i;
			for(i = iOffset; i < iOffsetStop; i++)
				fSum += GetAt(i);

			return fSum;
		}

		/**
		* Returns the sum of all vector elements weighted by their sigma errors.
		* The vector elements can be weigthed by a error vector given.
		* A range can be specified.
		*
		* @param vError		A vector, that contains the sigma error of the elements.
		* @param iOffset	The index of the first element.
		* @param iLength	The number of elements to inspect.
		*
		* @return	The sum of the weighted vector elements.
		*/
		TFitData SumErrorWeighted(CVector& vError, int iOffset = 0, int iLength = -1)
		{
			if(iLength <= 0)
				iLength = mLength;

			MATHFIT_ASSERT(iOffset >= 0 && (iOffset + iLength) <= mLength && iLength > 0);
			MATHFIT_ASSERT(mLength == vError.GetSize());

			int iOffsetStop = iOffset + iLength;
			TFitData fSum = 0;
			int i;
			for(i = iOffset; i < iOffsetStop; i++)
			{
				const TFitData fSigma = vError.GetAt(i);
				fSum += GetAt(i) / fSigma;
			}

			return fSum;
		}

		/**
		* Returns the square sum of all vector elements.
		* A range can be specified.
		*
		* @param iOffset	The index of the first element.
		* @param iLength	The number of elements to inspect.
		*
		* @return	The square sum of the vector elements.
		*/
		TFitData SquareSum(int iOffset = 0, int iLength = -1)
		{
			if(iLength <= 0)
				iLength = mLength;

			MATHFIT_ASSERT(iOffset >= 0 && (iOffset + iLength) <= mLength && iLength > 0);

			int iOffsetStop = iOffset + iLength;
			TFitData fSum = 0;
			int i;
			for(i = iOffset; i < iOffsetStop; i++)
				fSum += GetAt(i) * GetAt(i);

			return fSum;
		}

		/**
		* Returns the square sum of all vector elements weighted by their sigma errors.
		* The vector elements can be weigthed by a error vector given.
		* A range can be specified.
		*
		* @param vError		A vector, that contains the sigma error of the elements.
		* @param iOffset	The index of the first element.
		* @param iLength	The number of elements to inspect.
		*
		* @return	The square sum of the weighted vector elements.
		*/
		TFitData SquareSumErrorWeighted(CVector& vError, int iOffset = 0, int iLength = -1)
		{
			if(iLength <= 0)
				iLength = mLength;

			MATHFIT_ASSERT(iOffset >= 0 && (iOffset + iLength) <= mLength && iLength > 0);
			MATHFIT_ASSERT(mLength == vError.GetSize()); 

			int iOffsetStop = iOffset + iLength;
			TFitData fSum = 0;
			int i;
			for(i = iOffset; i < iOffsetStop; i++)
			{
				const TFitData fSigma = vError.GetAt(i) * vError.GetAt(i);
				const TFitData fData = GetAt(i) * GetAt(i);
				fSum += fData / fSigma;
			}

			return fSum;
		}

		/**
		* Returns the average of all vector elements.
		* A range can be specified.
		*
		* @param iOffset	The index of the first element.
		* @param iLength	The number of elements to inspect.
		*
		* @return	The average value of the vector elements.
		*/
		TFitData Average(int iOffset = 0, int iLength = -1)
		{
			if(iLength <= 0)
				iLength = mLength;

			if(iLength == 0)
				return 0;

			MATHFIT_ASSERT(iOffset >= 0 && (iOffset + iLength) <= mLength && iLength > 0);

			return Sum(iOffset, iLength) / (TFitData)iLength;
		}

		/**
		* Returns the variance of all vector elements.
		* A range can be specified.
		*
		* @param iOffset	The index of the first element.
		* @param iLength	The number of elements to inspect.
		*
		* @return	The variance of the vector elements.
		*/
		TFitData Variance(int iOffset = 0, int iLength = -1)
		{
			if(iLength <= 0)
				iLength = mLength;

			if(iLength == 0)
				return 0;

			MATHFIT_ASSERT(iOffset >= 0 && (iOffset + iLength) <= mLength && iLength > 0);

			double fAverage = Average(iOffset, iLength);

			int iOffsetStop = iOffset + iLength;
			TFitData fSum = 0;
			int i;
			for(i = iOffset; i < iOffsetStop; i++)
			{
				const TFitData fData = GetAt(i) - fAverage;
				fSum += fData * fData;
			}

			return fSum / (TFitData)iLength;
		}

		/**
		* Returns the standard deviation of all vector elements.
		* A range can be specified.
		*
		* @param iOffset	The index of the first element.
		* @param iLength	The number of elements to inspect.
		*
		* @return	The standard deviation of the vector elements.
		*/
		TFitData Deviation(int iOffset = 0, int iLength = -1)
		{
			return sqrt(Variance(iOffset, iLength));
		}

		/**
		* Returns the median of all vector elements.
		* A range can be specified.
		*
		* @param iOffset	The index of the first element.
		* @param iLength	The number of elements to inspect.
		*
		* @return	The median of the vector elements.
		*/
		TFitData Median(int iOffset = 0, int iLength = -1)
		{
			if(iLength <= 0)
				iLength = mLength;

			MATHFIT_ASSERT(iOffset >= 0 && (iOffset + iLength) <= mLength && iLength > 0);

			// create a copy of the specified elements
			CStatisticVector vBackup;
			vBackup.Copy(SubVector(iOffset, iLength));

			// sort them
			vBackup.Sort();

			TFitData fMedian;

			// check for an even or odd number of elements
			const int iBackupSize = vBackup.GetSize();
			if(iBackupSize & 0x01)
				fMedian = vBackup.GetAt(iBackupSize / 2);
			else
				fMedian = (vBackup.GetAt(iBackupSize / 2) + vBackup.GetAt(iBackupSize / 2 - 1)) / 2;

			return fMedian;
		}
	};
}
#endif
