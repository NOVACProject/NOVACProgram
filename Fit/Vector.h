/**
 * Contains the CVector class.
 *
 * @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
 * @version		1.0 @ 2001/09/09
 */
#if !defined(VECTOR_H_010908) 
#define VECTOR_H_010908

// in the debug version include the heap checking functions
#if defined(_DEBUG)
#include <crtdbg.h>
#endif

#include <iostream>
#include <math.h>
#include "FitBasic.h"
#include "FitException.h"

#pragma warning (push, 3)

namespace MathFit
{
	/**
	* Exception class thrown at mismatch in the vector sizes of two operants.
	*
	* @author		\item \URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
	* @author		\item \URL[Silke Humbert]{mailto:silke.humbert@iup.uni-heidelberg.de} @ \URL[IUP, Satellite Data Group]{http://giger.iup.uni-heidelberg.de}
	* @version		1.0 @ 2001/09/09
	*/
	class CVectorSizeMismatchException : public CFitException
	{
	public:
		CVectorSizeMismatchException(int iLine, const char* szModule, const char* szName) : CFitException(iLine, szModule, szName, "Vector size mismatch!") {}
	};

	/**
	* Exception class for an forbidden operation.
	*
	* @author		\item \URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
	* @version		1.0 @ 2003/01/21
	*/
	class CVectorOperationNotAllowedException : public CFitException
	{
	public:
		CVectorOperationNotAllowedException(int iLine, const char* szModule, const char* szName) : CFitException(iLine, szModule, szName, "Operation not allowed!") {}
	};

	/**
	* This class encapsulates the basic functions needed to handle one dimensional vectors.
	*
	* @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
	* @version		1.0 @ 2001/09/09
	*/
	class CVector
	{
	public:
		/**
		* Creates an empty vector object.
		*/
		CVector()
		{
			mData = NULL;
			mLength = 0;
			mStepSize = 1;
			mAutoRelease = true;

			mFloatPtr = NULL;
			mDoublePtr = NULL;
		}

		/**
		* Create a new vector object and fills it with the content of the given vector.
		*
		* @param vRight	The originating vector object.
		*/
		CVector(const CVector &vRight)
		{
			mData = NULL;
			mLength = 0;
			mStepSize = 1;
			mAutoRelease = true;

			mFloatPtr = NULL;
			mDoublePtr = NULL;

			Copy(vRight);
		}

		/**
		* Create a vector object with the given size.
		*
		* @param iSize		The number of elements in the vector.
		*/
		CVector(int iSize)
		{
			mData = NULL;
			mLength = 0;
			mStepSize = 1;
			mAutoRelease = true;

			mFloatPtr = NULL;
			mDoublePtr = NULL;

			SetSize(iSize);
		}

		/**
		* Create a vector from a given data object using the offset and length specified.
		*
		* @param vSecond	The vector object from which we should create a subvector.
		* @param iOffset	The offset from which the subvector should start
		* @param iSize		The number of elements in the subvector.
		*/
		CVector(CVector& vSecond, int iOffset, int iSize)
		{
			MATHFIT_ASSERT(iOffset + iSize <= vSecond.GetSize());
			MATHFIT_ASSERT(iSize > 0);

			mFloatPtr = NULL;
			mDoublePtr = NULL;

			mStepSize = vSecond.mStepSize;

			// get correct data pointer
			mData = &vSecond.GetSafePtr()[iOffset * mStepSize];

			// set new length
			mLength = iSize;

			// we are not allowed to release the buffer!
			mAutoRelease = false;
		}

		/**
		* Create a vector from a given data array.
		*
		* @param fData			The array containing the data elements.
		* @param iSize			The number of elements in the array.
		* @param iStepSize		The offset between two vector elements given in TFitData elements.
		* @param bAutoRelease	If TRUE the array is freed during destruction of the vector.
		*/
		CVector(TFitData* fData, int iSize, int iStepSize = 1, bool bAutoRelease = true)
		{
			mData = NULL;
			mLength = 0;
			mStepSize = 1;
			mAutoRelease = true;

			mFloatPtr = NULL;
			mDoublePtr = NULL;

			Attach(fData, iSize, iStepSize, bAutoRelease);
		}

		/**
		* Frees and allocated resources.
		*/
		~CVector()
		{
			if(mAutoRelease && mData != 0)
				delete mData;
			ReleaseDoublePtr();
			ReleaseFloatPtr();
		}

		/**
		* Copies the content of the given vector into the current object.
		*
		* @param vSecond	The originating vector object.
		*
		* @return A reference to the current object.
		*/
		CVector& Copy(const CVector& vSecond)
		{
			const iSecSize = vSecond.GetSize();

			SetSize(iSecSize);

			if(iSecSize <= 0)
				return *this;

			if(vSecond.mStepSize == 1 && mStepSize == 1)
				memcpy(GetSafePtr(), vSecond.GetSafePtr(), sizeof(TFitData) * iSecSize);
			else
			{
				int i;
				for(i = 0; i < iSecSize; i++)
					SetAt(i, vSecond.GetAt(i));
			}
			return *this;
		}

		/**
		* Copies the content of the given vector into the current object.
		*
		* @param fData			The array containing the data points.
		* @param iStepSize		The offset between two vector elements given in TFitData elements.
		* @param iSize			The number of elements in the array.
		*
		* @return A reference to the current object.
		*/
		CVector& Copy(TFitData* fData, int iSize, int iStepSize = 1)
		{
			SetSize(iSize);

			if(iSize <= 0)
				return *this;

			if(iStepSize == 1)
				memcpy(GetSafePtr(), fData, sizeof(TFitData) * iSize);
			else
			{
				int i;
				for(i = 0; i < GetSize(); i++)
					SetAt(i, fData[i * iStepSize]);
			}
			return *this;
		}

#if defined(MATHFIT_FITDATAFLOAT)
		CVector& Copy(double* fData, int iSize, int iStepSize = 1)
		{
			SetSize(iSize);

			if(iSize <= 0)
				return *this;

			int i;
			for(i = 0; i < GetSize(); i++)
				SetAt(i, (TFitData)fData[i * iStepSize]);
			return *this;
		}
#else
		CVector& Copy(float* fData, int iSize, int iStepSize = 1)
		{
			SetSize(iSize);

			if(iSize <= 0)
				return *this;

			int i;
			for(i = 0; i < GetSize(); i++)
				SetAt(i, (TFitData)fData[i * iStepSize]);
			return *this;
		}
#endif
		/**
		* Copies the content of a subvetor into the current vector starting at the given offset.
		*
		* @param iOffset	The starting index within the current vector.
		* @param vSub		The subvector, which content should be copied.
		*
		* @return A reference to the current object.
		*/
		CVector& Copy(int iOffset, CVector& vSub)
		{
			const int iSubSize = vSub.GetSize();

			if(iSubSize <= 0)
				return *this;

			MATHFIT_ASSERT((iOffset + iSubSize) <= mLength);

			if(mStepSize == 1 && vSub.mStepSize == 1)
				memcpy(&GetSafePtr()[iOffset], vSub.GetSafePtr(), sizeof(TFitData) * iSubSize);
			else
			{
				int i;
				for(i = 0; i < iSubSize; i++)
					SetAt(i + iOffset, vSub.GetAt(i));
			}

			return *this;
		}

		/**
		* Attaches the content of another CVector object to the current one
		*
		* @param vSecond		The originating object.
		* @param bAutoRelease	If TRUE the vector data is freed on destructuion of the vector.
		*
		* @return	A reference to the current object.
		*/
		CVector& Attach(CVector& vSecond, bool bAutoRelease = true)
		{
			// first clear the old data
			if(mData && mAutoRelease)
				delete mData;

			// get the data pointer
			mData = vSecond.mData;
			mLength = vSecond.mLength;
			mStepSize = vSecond.mStepSize;
			if(bAutoRelease)
			{
				// so the current object will no take care about destruction of the vector data
				mAutoRelease = vSecond.mAutoRelease;
				vSecond.mAutoRelease = false;
			}
			else
				mAutoRelease = false;

			return *this;
		}

		/**
		* Attaches the content of another CVector object to the current one
		*
		* @param fData			The data array that contains the values.
		* @param iSize			The number of elements in the array.
		* @param iStepSize		The offset between two vector elements given in TFitData elements.
		* @param bAutoRelease	If TRUE the array is freed on destructuion of the vector.
		*
		* @return	A reference to the current object.
		*/
		CVector& Attach(TFitData* fData, int iSize, int iStepSize = 1, bool bAutoRelease = true)
		{
			MATHFIT_ASSERT(iSize > 0);

			// first clear the old data
			if(mData && mAutoRelease)
				delete mData;

			// get the data pointer
			mData = fData;
			mLength = iSize;
			mAutoRelease = bAutoRelease;
			mStepSize = iStepSize;

			return *this;
		}

		/**
		* Detaches the data object from the current object so the data isn't freed anymore when the object is destroyed.
		*
		* @return	A reference to the current object
		*/
		CVector& Detach()
		{
			mData = NULL;
			mLength = 0;
			mStepSize = 1;
			mAutoRelease = true;

			ReleaseDoublePtr();
			ReleaseFloatPtr();

			return *this;
		}

		/**
		* Returns a vector object containing a sub sequence of the current vector object.
		* The data associated with the sub vector object is invalid as soon as the current
		* object is deleted! Always destroy all sub vector objects before the originating
		* vector object is destroyed.
		*
		* @param iOffset	The index of the first element in the subvector.
		* @param iSize		The number of elements in the subvector.
		*
		* @return	A vector object representing the selected subvector.
		*/
		CVector SubVector(int iOffset, int iSize)
		{
			// create new subclassed vector object
			return CVector(*this, iOffset, iSize);
		}

		/**
		* Exchanges the content of the current object with the content of another object.
		*
		* @param vSecond	The originating object.
		*
		* @return	A reference to the current object
		*/
		CVector& Exchange(CVector& vSecond)
		{
			int iLength = vSecond.mLength;
			int iStepSize = vSecond.mStepSize;
			TFitData* fData = vSecond.mData;
			bool bAutoRelease = vSecond.mAutoRelease;
			double* fDoublePtr = vSecond.mDoublePtr;
			float* fFloatPtr = vSecond.mFloatPtr;

			vSecond.mLength = mLength;
			vSecond.mStepSize = mStepSize;
			vSecond.mData = mData;
			vSecond.mAutoRelease = mAutoRelease;
			vSecond.mDoublePtr = mDoublePtr;
			vSecond.mFloatPtr = mFloatPtr;

			mLength = iLength;
			mStepSize = iStepSize;
			mData = fData;
			mAutoRelease = bAutoRelease;
			mDoublePtr = fDoublePtr;
			mFloatPtr = fFloatPtr;

			return *this;
		}

		/**
		* Exchanges two elements in the vector.
		*
		* @param iFirst	The index of the first element.
		* @param iSec		The index of the second element.
		*
		* @return	A reference to the current object.
		*/
		CVector& ExchangeElements(int iFirst, int iSec)
		{
			TFitData fTemp = GetAt(iFirst);
			SetAt(iFirst, GetAt(iSec));
			SetAt(iSec, fTemp);

			return *this;
		}

		/**
		* Returns a pointer to the data array which can be used for directly accessing the vector data.
		*
		* @return	A pointer to the data array. NULL if no array is associated.
		*/
		TFitData* GetSafePtr() const
		{
			MATHFIT_ASSERT(mLength > 0);
			MATHFIT_ASSERT(_CrtIsValidPointer(mData, sizeof(mData[0]) * mLength, TRUE));

			return mData;
		}

		/**
		* Returns the desired element of the vector.
		*
		* @param iIndex	The index of the element.
		*
		* @return	A reference to the vector element.
		*/
		TFitData GetAt(int iIndex) const
		{
			MATHFIT_ASSERT(iIndex >= 0 && iIndex < mLength);

			return GetSafePtr()[iIndex * mStepSize];
		}

		/**
		* Returns the desired element of the vector at any index.
		* The borders of the vector are constantly expanded so the given index must
		* not be inside the given boundaries.
		*
		* @param iIndex	The index of the element.
		*
		* @return	A reference to the vector element.
		*/
		TFitData GetAtSafe(int iIndex) const
		{
			// check for boundary condition
			if(iIndex < 0)
				return GetAt(0);
			else if(iIndex >= mLength)
				return GetAt(mLength - 1);
			else
				return GetAt(iIndex);
		}

		/**
		* Sets the content of a single element.
		*
		* @param iIndex	The index of the element.
		* @param fParam	The new content of the element.
		*
		* @return	A reference to the current object.
		*/
		CVector& SetAt(int iIndex, TFitData fParam)
		{
			MATHFIT_ASSERT(iIndex >= 0 && iIndex < mLength);

			GetSafePtr()[iIndex * mStepSize] = fParam;

			return *this;
		}

		/**
		* Returns the number of elements in the vector.
		*
		* @return	The number of elements in the vector.
		*/
		const int GetSize() const
		{
			return mLength;
		}

		/**
		* Sets the size of the vector. 
		* If the vector needs to be resized the neccessary buffer is reallocated.
		*
		* @param iNewSize	The new number of elements.
		*/
		void SetSize(const int iNewSize)
		{
			if(iNewSize != mLength || !mData)
			{
				if(mData != 0)
					delete mData;
				ReleaseFloatPtr();
				ReleaseDoublePtr();

				mData = NULL;
				mStepSize = 1;
				mLength = iNewSize;
				if(mLength <= 0)
					return;

				mData = new TFitData[mLength];

				// indicate that we have created this data object
				mAutoRelease = true;

				memset(mData, 0, mLength * sizeof(TFitData));
			}
		}

		/**
		* Resizes the vector and keeps the data content.
		* If the vector is enlarged, the original elements are copied to the beginning
		* of the new vector. The newly added elements are set to zero. If the
		* vector shrinks, only the first \Ref{iNewSize} elements are copied.
		*
		* @param iNewSize	The new number of elements in the vector.
		*
		* @return A reference to the current object.
		*/
		CVector& Resize(int iNewSize)
		{
			// create a new vector
			CVector vNew(iNewSize);

			// copy the data elements
			int i;
			int iNumElem = min(iNewSize, GetSize());
			for(i = 0; i < iNumElem; i++)
				vNew.SetAt(i, GetAt(i));

			// exchange the data arrays
			Attach(vNew);
			vNew.Detach();

			return *this;
		}

		/**
		* Appends the given vector to the current one.
		*
		* @param vApp	The vector to be appended to the current vector.
		*
		* @return	A reference to the current vector.
		*/
		CVector& Append(CVector& vApp)
		{
			int iOldSize = GetSize();
			Resize(iOldSize + vApp.GetSize());
			Copy(iOldSize, vApp);

			return *this;
		}

		/**
		* Appends a single data element to the vector.
		* The new element will be added after the last index of the current vector.
		*
		* @param fValue	The value of the new vector element.
		*
		* @return	A reference to the current vector.
		*/
		CVector& Append(TFitData fValue)
		{
			int iOldSize = GetSize();
			Resize(iOldSize + 1);
			SetAt(GetSize() - 1, fValue);

			return *this;
		}

		/**
		* Fills the vector with zeros.
		*
		* @return The cleared vector.
		*/
		CVector& Zero()
		{
			MATHFIT_ASSERT(mData != NULL);

			if(mStepSize == 1)
				memset(mData, 0, sizeof(TFitData) * mLength);
			else
			{
				int i;
				for(i = 0; i < mLength; i++)
					SetAt(i, 0);
			}

			return *this;
		}

		/**
		* Checks wheter all elements are zero.
		*
		* @return TRUE is all elements are zero, FALSE otherwise.
		*/
		bool IsZero()
		{
			int i;
			for(i = 0; i < mLength; i++)
				if(GetAt(i) != 0.0)
					return false;
			return true;
		}

		/**
		* Builds the additive inverse of the vector.
		*
		* @return	A reference to the current object.
		*/
		CVector& Invert()
		{
			int i;
			for(i = 0; i < mLength; i++)
				SetAt(i, -GetAt(i));

			return *this;
		}

		/**
		* Builds the multilicative inverse of the vector.
		*
		* @return	A reference to the current object.
		*/
		CVector& Inverse()
		{
			int i;
			for(i = 0; i < mLength; i++)
				SetAt(i, 1 / GetAt(i));

			return *this;
		}

		/**
		* Fills the vector with values defined.
		* To fill the vector the following formular is used:
		* a(i) = fStart + i * fSlope
		*
		* @param fStart	The value of the first element.
		* @param fSlope	The increasement factor for the next element.
		*
		* @return	A reference to the current object.
		*/
		CVector& Wedge(TFitData fStart, TFitData fSlope)
		{
			TFitData fVal = fStart;
			int i;
			for(i = 0; i < mLength; i++, fVal += fSlope)
				SetAt(i, fVal);

			return *this;
		}

		/**
		* Adds the content of two vectors element by element.
		*
		* @param vOperant	The vector which should be added to the current one.
		*
		* @return	A reference to the current object.
		*/
		CVector& Add(CVector& vOperant)
		{
			MATHFIT_ASSERT(mLength == vOperant.GetSize());

			int i;
			for(i = 0; i < mLength; i++)
				SetAt(i, GetAt(i) + vOperant.GetAt(i));
			return *this;
		} 

		/**
		* Adds the content of two vectors element by element.
		* The second operand is scaled by the given factor.
		*
		* @param vOperant	The vector which should be added to the current one.
		* @param fFactor	The factor used to scale the second operand.
		*
		* @return	A reference to the current object.
		*/
		CVector& Add(CVector& vOperant, TFitData fFactor)
		{
			MATHFIT_ASSERT(mLength == vOperant.GetSize());

			int i;
			for(i = 0; i < mLength; i++)
				SetAt(i, GetAt(i) + fFactor * vOperant.GetAt(i));
			return *this;
		} 

		/**
		* Subtracts the content of two vectors element by element.
		*
		* @param vOperant	The vector which should be subtracted to the current one.
		*
		* @return	A reference to the current object.
		*/
		CVector& Sub(CVector& vOperant)
		{
			MATHFIT_ASSERT(mLength == vOperant.GetSize());

			int i;
			for(i = 0; i < mLength; i++)
				SetAt(i, GetAt(i) - vOperant.GetAt(i));
			return *this;
		}

		/**
		* Subtracts the content of two vectors element by element.
		* The second operand is scaled by the given factor.
		*
		* @param vOperant	The vector which should be subtracted to the current one.
		* @param fFactor	The factor used to scale the second operand.
		*
		* @return	A reference to the current object.
		*/
		CVector& Sub(CVector& vOperant, TFitData fFactor)
		{
			MATHFIT_ASSERT(mLength == vOperant.GetSize());

			int i;
			for(i = 0; i < mLength; i++)
				SetAt(i, GetAt(i) - fFactor * vOperant.GetAt(i));
			return *this;
		}

		/**
		* Multiplies two vector by building the scalar product.
		*
		* @param vOperant	The vector with which the scalar product should be determined.
		*
		* @return	The scalar product of the two vectors.
		*/
		TFitData Mul(CVector& vOperant)
		{
			MATHFIT_ASSERT(mLength == vOperant.GetSize());

			TFitData fResult = 0;

			int i;
			for(i = 0; i < mLength; i++)
				fResult += GetAt(i) * vOperant.GetAt(i);

			return fResult;
		}

		/**
		* Multiplies the content of two vectors element by element.
		*
		* @param vOperant	The vector which should be multiplicated to the current one.
		*
		* @return	A reference to the current object.
		*/
		CVector& MulSimple(CVector& vOperant)
		{
			MATHFIT_ASSERT(mLength == vOperant.GetSize());

			int i;
			for(i = 0; i < mLength; i++)
				SetAt(i, GetAt(i) * vOperant.GetAt(i));
			return *this;
		}

		/**
		* Divides the content of two vectors element by element.
		*
		* @param vOperant	The vector which should be divided from the current one.
		*
		* @return	A reference to the current object.
		*/
		CVector& DivSimple(CVector& vOperant)
		{
			MATHFIT_ASSERT(mLength == vOperant.GetSize());

			int i;
			for(i = 0; i < mLength; i++)
				SetAt(i, GetAt(i) / vOperant.GetAt(i));
			return *this;
		}

		/**
		* Divides the content of two vectors element by element in a safe way.
		*
		* This mehtod check wheter the divisor is zero or not and therefore prevents
		* divisions by zero.
		*
		* @param vOperant	The vector which should be divided from the current one.
		*
		* @return	A reference to the current object.
		*/
		CVector& DivSimpleSafe(CVector& vOperant)
		{
			MATHFIT_ASSERT(mLength == vOperant.GetSize());

			int i;
			for(i = 0; i < mLength; i++)
				if(vOperant.GetAt(i) != 0)
                    SetAt(i, GetAt(i) / vOperant.GetAt(i));
				else
					SetAt(i, 0);

			return *this;
		}

		/**
		* Adds a scalar to every element of the vector.
		*
		* @param fOperant	The scalar to be added.
		*
		* @return	A reference to the current object.
		*/
		CVector& Add(TFitData fScalar)
		{
			int i;
			for(i = 0; i < mLength; i++)
				SetAt(i, GetAt(i) + fScalar);

			return *this;
		}

		/**
		* Subtracts a scalar from every element of the vector.
		*
		* @param fOperant	The scalar.
		*
		* @return	A reference to the current object.
		*/
		CVector& Sub(TFitData fScalar)
		{
			int i;
			for(i = 0; i < mLength; i++)
				SetAt(i, GetAt(i) - fScalar);

			return *this;
		}

		/**
		* Multiplies every element of the vector by the given scalar.
		*
		* @param fOperant	The scalar.
		*
		* @return	A reference to the current object.
		*/
		CVector& Mul(TFitData fScalar)
		{
			int i;
			for(i = 0; i < mLength; i++)
				SetAt(i, GetAt(i) * fScalar);

			return *this;
		}

		/**
		* Divide every element of the vector by the given scalar.
		*
		* @param fOperant	The scalar.
		*
		* @return	A reference to the current object.
		*/
		CVector& Div(TFitData fScalar)
		{
			int i;
			for(i = 0; i < mLength; i++)
				SetAt(i, GetAt(i) / fScalar);

			return *this;
		}

		/**
		* Implements a polynomial evaluation where the polynomial coefficients are given in the vector. 
		* The evaluation uses the horner scheme.
		*
		* @param fXValue	The X value at which to evaluate the polynomial.
		*
		* @return	The polynomial value at the given point.
		*/
		TFitData CalcPoly(TFitData fXValue)
		{
			TFitData fRes;
			int iGrade = GetSize() - 1;

			// if we do not have a polynomial vector at all (#elements == 0) just return the current X value
			if(iGrade < 0)
				return fXValue;

			// calculate the polynomial using the horner scheme
			fRes = GetAt(iGrade--);
			for(; iGrade >= 0; iGrade--)
				fRes = (fRes * fXValue) + GetAt(iGrade);

			return fRes;
		}

		/**
		* Implements the first derivation of a polynomial where the polynomial coefficients are given in the vector.
		* The evaluation uses the horner scheme.
		*
		* @param fXValue	The X value at which to evaluate the polynomial.
		*
		* @return	The polynomial derivation value at the given point.
		*/
		TFitData CalcPolySlope(TFitData fXValue)
		{
			TFitData fRes;
			int iGrade = GetSize() - 1;

			// if we do not have a polynomial at all (#elements == 0), just return zero
			if(iGrade <= 0)
				return 0;

			// calculate the polynomial using the horner scheme
			fRes = GetAt(iGrade--) * iGrade;
			for(; iGrade >= 1; iGrade--)
				fRes = (fRes * fXValue) + iGrade * GetAt(iGrade);

			return fRes;
		}

		/**
		* Returns the smalles element of the vector.
		* A range can be specified.
		*
		* @param iOffset	The index of the first element starting to search for the minimum.
		* @param iLength	The number of elements to inspect.
		*
		* @return	The smalles element of the vector.
		*/
		TFitData Min(int iOffset = 0, int iLength = -1)
		{
			if(iLength < 0)
				iLength = mLength;

			MATHFIT_ASSERT(iOffset >= 0 && (iOffset + iLength) <= mLength && iLength > 0);

			TFitData fMin = GetAt(iOffset);
			int iOffsetStop = iOffset + iLength;
			int i;
			for(i = iOffset + 1; i < iOffsetStop; i++)
				if(GetAt(i) < fMin)
					fMin = GetAt(i);

			return fMin;
		}

		/**
		* Returns the biggest element of the vector.
		* A range can be specified.
		*
		* @param iOffset	The index of the first element starting to search for the maximum.
		* @param iLength	The number of elements to inspect.
		*
		* @return	The biggest element of the vector.
		*/
		TFitData Max(int iOffset = 0, int iLength = -1)
		{
			if(iLength < 0)
				iLength = mLength;

			MATHFIT_ASSERT(iOffset >= 0 && (iOffset + iLength) <= mLength && iLength > 0);

			TFitData fMax = GetAt(iOffset);
			int iOffsetStop = iOffset + iLength;
			int i;
			for(i = iOffset + 1; i < iOffsetStop; i++)
				if(GetAt(i) > fMax)
					fMax = GetAt(i);

			return fMax;
		}

		/**
		* Subtracts the minimum value from all vector elements.
		* This operation causes all vector elements to be positive afterwards.
		* A range can be specified on which the minimum will be searched. The operation itself
		* will always work on the whole vector!
		*
		* @param iOffset	The index of the first element starting to search for the maximum.
		* @param iLength	The number of elements to inspect.
		*
		* @return	The former minimum value.
		*/
		TFitData BiasAdjust(int iOffset = 0, int iLength = -1)
		{
			if(iLength < 0)
				iLength = mLength;

			MATHFIT_ASSERT(iOffset >= 0 && (iOffset + iLength) <= mLength && iLength > 0);

			TFitData fMin = Min(iOffset, iLength);
			Sub(fMin);
			return fMin;
		}

		/**
		* Normalizes the vector elements to a maximum value of one.
		* Normalization is done by dividing all vector elements by the biggest element value.
		* The signs of the vector elements are keept.
		* A range can be specified on which the maximum will be searched. The operation itself
		* will always work on the whole vector!
		*
		* @param iOffset	The index of the first element starting to search for the maximum.
		* @param iLength	The number of elements to inspect.
		*
		* @return	The former maximum value.
		*/
		TFitData Normalize(int iOffset = 0, int iLength = -1)
		{
			if(iLength < 0)
				iLength = mLength;

			MATHFIT_ASSERT(iOffset >= 0 && (iOffset + iLength) <= mLength && iLength > 0);

			TFitData fMax = (TFitData)max(fabs(Max(iOffset, iLength)), fabs(Min(iOffset, iLength)));
			Div(fMax);
			return fMax;
		}

		float* GetFloatPtr()
		{
			ReleaseFloatPtr();

			mFloatPtr = new float[GetSize()];

			int i;
			for(i = 0; i < GetSize(); i++)
				mFloatPtr[i] = (float)GetAt(i);

			return mFloatPtr;
		}

		void ReleaseFloatPtr()
		{
			if(mFloatPtr)
				delete mFloatPtr;
			mFloatPtr = NULL;
		}

		double* GetDoublePtr()
		{
			ReleaseDoublePtr();

			mDoublePtr = new double[GetSize()];

			int i;
			for(i = 0; i < GetSize(); i++)
				mDoublePtr[i] = (double)GetAt(i);

			return mDoublePtr;
		}

		void ReleaseDoublePtr()
		{
			if(mDoublePtr)
				delete mDoublePtr;
			mDoublePtr = NULL;
		}

		int GetStepSize()
		{
			return mStepSize;
		}

		void SetStepSize(int iStepSize)
		{
			mStepSize = iStepSize;
		}

		enum EIndexConditions
		{
			EQUAL = 1,
			GREATER,
			LESS,
			LESSEQUAL,
			GREATEREQUAL
		};

		int FindIndex(TFitData fValue, enum EIndexConditions eCondition = EQUAL) const
		{
			int iLow = 0;
			int iHigh = GetSize() - 1;

			// find the index of the first element that is less or equal than the given values.
			while(iHigh - iLow > 1)
			{
				const int iMid = iLow + (iHigh - iLow) / 2;
				const TFitData fData = GetAt(iMid);

				if(fData >= fValue)
					iHigh = iMid;
				else
					iLow = iMid;
			}

			const TFitData fLowData = GetAt(iLow);
			TFitData fHighData = GetAt(iHigh);

			// if the data at the higher and lower bound is equal, we need to find
			// the next high bound value that is greater than the lower bound
			while(fHighData == fLowData && iHigh < GetSize())
			{
				iHigh++;
				fHighData = GetAt(iHigh);
			}

			// test for conditions and return the proper index value
			if(eCondition == EQUAL || eCondition == LESSEQUAL || eCondition == GREATEREQUAL)
			{
				if(fLowData == fValue)
					return iLow;
				else if(fHighData == fValue)
					return iHigh;
			}
			if(eCondition == GREATER || eCondition == GREATEREQUAL)
			{
				if(fLowData > fValue)
					return iLow;

				// scan for constant data sets
				int iHigher = iHigh;
				while(iHigher < GetSize() && GetAt(iHigher) >= fHighData)
				{
					if(GetAt(iHigher) > fValue)
						return iHigher;
					iHigher++;
				}
			}
			if(eCondition == LESS || eCondition == LESSEQUAL)
			{
				if(fHighData < fValue)
					return iHigh;
				else if(fLowData < fValue)
					return iLow;
			}

			return -1;
		}

		/**
		 * Assignment operator
		 *
		 * This overload is present to prevent the user from accidently assigning vector objects without knowing
		 * wheter the content is copied or attached. Please use explicitly Copy or Attach instead.
		 */
		CVector& operator=(CVector& vOp)
		{
			// !! The direct assignment of vectors is not allowed, since we may get into ambigousity wheter we have to 
			// !! attach or copy the current vector.
			// !! Therefore explicitly define wheter you want to attach or copy one vector to another object.
			throw EXCEPTION(CVectorOperationNotAllowedException);
			return vOp;
		}

		/**
		* Prints the content of the vector to the given out stream.
		*
		* @param os	The output stream
		* @param vData	The vector which content should be displayed.
		*
		* @return A reference to the output stream itself.
		*/
		friend std::ostream& operator<<(std::ostream& os, CVector& vData)
		{
			const int iSize = vData.GetSize();

			int i;
			for(i = 0; i < iSize; i++)
			{
				if(i > 0)
					os << ' ';
				os << vData.GetAt(i);
			}

			return os;
		}

	protected:
		/**
		* Contains the length of the vector.
		*/
		int mLength;
		/**
		* Array containing the vector elements.
		*/
		TFitData* mData;
		/**
		* Contains a flag indicating wheter we have to release the data buffer on destruction or not.
		* This flag may be set to FALSE if the current object is a sub vector of another object.
		*/
		bool mAutoRelease;
		/*@@
		* Summary:
		*	Contains the offset between two vector elements.
		*
		* Description:
		*	If the vector elements are not stored one after another, this member holds the scaling factor
		*	of the element index to represent its real index in the memory array.
		*/
		int mStepSize;
		float* mFloatPtr;
		double* mDoublePtr;
	};
}

#pragma warning (pop)
#endif
