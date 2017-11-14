/**
 * Contains the CDOASVector class.
 *
 * @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
 * @version		1.0 @ 2002/02/22
 */
#if !defined(DOASVECTOR_H_010908) 
#define DOASVECTOR_H_010908

#include "StatisticVector.h"

#pragma warning (push, 3)

namespace MathFit
{
	/**
	* This class extends the statistical vector class with some DOAS specific methods.
	*
	* @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
	* @version		1.0 @ 2002/02/22
	*/
	class CDOASVector : public CStatisticVector
	{
	public:
		/**
		* Creates an empty vector object.
		*/
		CDOASVector()
			: CStatisticVector()
		{
		}

		/**
		* Create a new vector object and fills it with the content of the given vector.
		*
		* @param vRight	The originating vector object.
		*/
		CDOASVector(const CVector &vRight)
			: CStatisticVector(vRight)
		{
		}

		/**
		* Create a vector object with the given size.
		*
		* @param iSize		The number of elements in the vector.
		*/
		CDOASVector(int iSize)
			: CStatisticVector(iSize)
		{
		}

		/**
		* Create a vector from a given data object using the offset and length specified.
		*
		* @param vSecond	The vector object from which we should create a subvector.
		* @param iOffset	The offset from which the subvector should start
		* @param iSize		The number of elements in the subvector.
		*/
		CDOASVector(CVector& vSecond, int iOffset, int iSize)
			: CStatisticVector(vSecond)
		{
		}

		/**
		* Create a vector from a given data array.
		*
		* @param fData			The array containing the data elements.
		* @param iSize			The number of elements in the array.
		* @param bAutoRelease	If TRUE the array is freed during destruction of the vector.
		*/
		CDOASVector(TFitData* fData, int iSize, bool bAutoRelease = true)
			: CStatisticVector(fData, iSize, bAutoRelease)
		{
		}

		/**
		* Returns the maximum difference of all vector elements.
		* A range can be specified.
		*
		* @param iOffset	The index of the first element.
		* @param iLength	The number of elements to inspect.
		*
		* @return	The delta of the vector elements.
		*/
		TFitData Delta(int iOffset = 0, int iLength = -1)
		{
			if(iLength < 0)
				iLength = mLength;

			MATHFIT_ASSERT(iOffset >= 0 && (iOffset + iLength) <= mLength && iLength > 0);

			return (TFitData)fabs(Max(iOffset, iLength) - Min(iOffset, iLength));
		}

		/**
		* Returns the relative delta value of all vector elements.
		* A range can be specified.
		* The relative delta is given by the absolute delte divided by the average
		* value of the given range.
		*
		* @param iOffset	The index of the first element.
		* @param iLength	The number of elements to inspect.
		*
		* @return	The delta of the vector elements.
		*/
		TFitData DeltaRel(int iOffset = 0, int iLength = -1)
		{
			if(iLength < 0)
				iLength = mLength;

			MATHFIT_ASSERT(iOffset >= 0 && (iOffset + iLength) <= mLength && iLength > 0);

			TFitData fAverage = (TFitData)fabs(Average(iOffset, iLength));
			if(fAverage == 0)
				return 0;

			return Delta(iOffset, iLength) / fAverage;
		}

		/**
		* Determines the optical density of the given absorption band.
		* {\bf NOTE:} The quotient will be logarithmed, so the information
		* is only valid on NON-logarithmed spectra.
		*
		* @param iLeftIndex	The index where the absorption band starts.
		* @param iMidIndex		The index where the absorption band has its minimum.
		* @param iRightIndex	The index where the absorption band ends.
		*
		* @return	The logarithm of the quotient of the interpolated source intensity and the absorption intensity.
		*/
		TFitData OpticalDensity(int iLeftIndex, int iMidIndex, int iRightIndex)
		{
			// ensure all indicies are positive
			MATHFIT_ASSERT(iLeftIndex >= 0 && iMidIndex >= 0 && iRightIndex >= 0);

			// ensure we have the indicies ordered properly
			MATHFIT_ASSERT(iLeftIndex <= iMidIndex && iMidIndex <= iRightIndex);

			// ensure the highest index is still inside the vector boundaries
			MATHFIT_ASSERT(iRightIndex < mLength);

			TFitData fRange = (TFitData)(iRightIndex - iLeftIndex);
			if(fRange == 0)
				return 0;

			// check for valid Y range
			TFitData fOffset = 0;

			// if we have negative values, shift the data upwards just a little above the ordinate.
			if(Min() <= 0)
				fOffset = -Min() + 1e-10;

			TFitData fScale = (TFitData)(iMidIndex - iLeftIndex);

			TFitData fLeft = GetAt(iLeftIndex) + fOffset;
			TFitData fMid = GetAt(iMidIndex) + fOffset;
			TFitData fRight = GetAt(iRightIndex) + fOffset;

			// interpolate I_o
			TFitData fIo = fLeft + fScale * (fRight - fLeft) / fRange;
			TFitData fI = GetAt(iMidIndex) + fOffset;

			// calculate the optical density
			return (TFitData)log(fIo / fI);
		}

		/**
		* Tries to find the strongest absorption line.
		* The algorithm tries to find the strongest optical density by subsequently search for a sequence
		* of a local maximum followed by a local minimum followed by another local maximum. This sequence is
		* expceted to be an absorption line. To really find the major absorption lines, the search range for an extrema
		* can be flued by specifying a range of pixels that all have to match the extrema criteriac.
		* Eg. the algorithm searches for the maximum pixel from the current offset by increasing the pixel index
		* as long as the next pixel value is larger than the last one. As soon as a smaller value is found,
		* the algorithms expects the last pixel to be the current maximum value. If a search range is specified,
		* the algorithm will also look at the next \Ref{fRange} pixels to see wheter there is another maximum.
		* If in the given search range another maximum is found, the search for the local maximum is restarted from
		* the newly found maximum. If no other maximum is found the already identified maximum is expected to be
		* the current local maximum.
		* A global search range can also be defined.
		*
		* @param iLeftIndex	Will receive the left index of the absorption line.
		* @param iMidIndex		Will receive the mid index of the absorption line.
		* @param iRightIndex	Will receive the right index of the absorption line.
		* @param fRange		An additional number of pixels to weaken the boundaries for extremas found. Default is 1% of all available pixels.
		* @param iOffset		The beginning of the search range.
		* @param iLength		The length of the search range. If -1 the whole vector will be searched.
		*/
		void FindStrongestAbsorption(int& iLeftIndex, int& iMidIndex, int& iRightIndex, TFitData fRange = 1, int iOffset = 0, int iLength = -1)
		{
			if(iLength < 0)
				iLength = GetSize();

			MATHFIT_ASSERT(iOffset + iLength <= GetSize());

			iLeftIndex = iMidIndex = iRightIndex = iOffset;
			int iMaxOffset = iOffset + iLength;

			int iRange = (int)(GetSize() * fRange / 100.0);

			TFitData fLast = GetAt(iLeftIndex);

			int iLeft = iLeftIndex;
			int iMid = iMidIndex;
			int iRight = iRightIndex;

			// get initial optical density
			TFitData fOptDen = OpticalDensity(iLeft, iMid, iRight);

			while(iRight < iMaxOffset)
			{
				bool bRedo = false;
				do
				{
					bRedo = false;
					// search for the first local maxima. Use this as the left border
					while(GetAtSafe(iLeft + 1) > fLast)
					{
						iLeft++;
						if(iLeft >= iMaxOffset)
							return;
						fLast = GetAtSafe(iLeft);
					}
					// try to move across a small local maxima
					// search the next iRange pixels for another maxima and restart search
					int i;
					for(i = 0; i < iRange && (iLeft + i) < iMaxOffset; i++)
						if(GetAtSafe(iLeft + i) > fLast)
						{
							iLeft += i;
							fLast = GetAtSafe(iLeft);
							bRedo = true;
						}
				}while(bRedo);
				if(iLeft >= iMaxOffset)
					return;

				// use the left index as start point for the mid index
				iMid = iLeft;
				fLast = GetAtSafe(iMid);

				// look for the local minimum
				do
				{
					bRedo = false;
					// search for the first local maxima. Use this as the left border
					while(GetAtSafe(iMid + 1) < fLast)
					{
						iMid++;
						if(iMid >= iMaxOffset)
							return;
						fLast = GetAtSafe(iMid);
					}
					// try to move across a small local maxima
					// search the next iRange pixels for another maxima and restart search
					int i;
					for(i = 0; i < iRange && (iMid + i) < iMaxOffset; i++)
						if(GetAtSafe(iMid + i) < fLast)
						{
							iMid += i;
							fLast = GetAtSafe(iMid);
							bRedo = true;
						}
				}while(bRedo);
				if(iMid >= iMaxOffset)
					return;

				// use the mid index as start point for the right index
				iRight = iMid;
				fLast = GetAtSafe(iRight);

				// search for the next local maximum
				do
				{
					bRedo = false;
					// search for the first local maxima. Use this as the left border
					while(GetAtSafe(iRight + 1) > fLast)
					{
						iRight++;
						if(iRight >= iMaxOffset)
							return;
						fLast = GetAtSafe(iRight);
					}
					// try to move across a small local maxima
					// search the next iRange pixels for another maxima and restart search
					int i;
					for(i = 0; i < iRange && (iRight + i) < iMaxOffset; i++)
						if(GetAtSafe(iRight + i) > fLast)
						{
							iRight += i;
							fLast = GetAtSafe(iRight);
							bRedo = true;
						}
				}while(bRedo);
				if(iRight >= iMaxOffset)
					iRight = iMaxOffset - 1;

				// check for bigger optical density
				TFitData fOptDenNew = OpticalDensity(iLeft, iMid, iRight);
				if(fOptDenNew > fOptDen)
				{
					iLeftIndex = iLeft;
					iMidIndex = iMid;
					iRightIndex = iRight;
					fOptDen = fOptDenNew;
				}

				// no absorption found
				if(iLeft == iMid && iMid == iRight)
					return;

				// go to next possible aborption line
				iLeft = iRight;
			}
		}
	};
}

#pragma warning (pop)
#endif
