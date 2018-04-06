/**
 * Matrix.h
 *
 * Contains the CMatrix class.
 *
 * @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
 * @version		1.0 @ 2001/09/09
 */
#if !defined(MATRIX_H_010908)
#define MATRIX_H_010908

#pragma warning (push, 3)

namespace MathFit
{
	/*@@
	* Summary:
	*	Defines wheter row or column orientated matrices are used.
	*
	* Description:
	*	If this define is set, the matrix data will be stored row by row. If the define is not set
	*	the matrix data is stored column by column.
	*/
//#define ROWMATRIX

	// Vector
#include "Vector.h"

	/**
	* Exception indicating a size mismatch of two matrices.
	*
	* @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
	* @version		1.0 @ 2001/09/09
	*/
	class CMatrixSizeMismatchException : public CFitException
	{
	public:
		CMatrixSizeMismatchException(int iLine, const char* szModule, const char* szName) : CFitException(iLine, szModule, szName, "Matrix size mismatch!") {}
	};

	/**
	* Exception indicating a failure during the solveing of a LES.
	*
	* @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
	* @version		1.0 @ 2001/09/09
	*/
	class CMatrixSolveFailedException : public CFitException
	{
	public:
		CMatrixSolveFailedException(int iLine, const char* szModule, const char* szName) : CFitException(iLine, szModule, szName, "Matrix solve failed!") {}
	};

	/**
	* Exception indicating a not square matrix used as operant.
	*
	* @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
	* @version		1.0 @ 2001/09/09
	*/
	class CMatrixNotSquareException : public CFitException
	{
	public:
		CMatrixNotSquareException(int iLine, const char* szModule, const char* szName) : CFitException(iLine, szModule, szName, "Matrix not square!") {}
	};

	/**
	* Exception indicating a singular matrix.
	*
	* @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
	* @version		1.0 @ 2001/09/09
	*/
	class CMatrixSingularException : public CFitException
	{
	public:
		CMatrixSingularException(int iLine, const char* szModule, const char* szName) : CFitException(iLine, szModule, szName, "Matrix is singular!") {}
	};

	/**
	* Exception indicating that the matrix is not a LU decomposed matrix.
	*
	* @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
	* @version		1.0 @ 2002/11/12
	*/
	class CMatrixNotLUDecomposedException : public CFitException
	{
	public:
		CMatrixNotLUDecomposedException(int iLine, const char* szModule, const char* szName) : CFitException(iLine, szModule, szName, "Matrix is not LU decomposed!") {}
	};

	/**
	* Exception class for an forbidden operation.
	*
	* @author		\item \URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
	* @version		1.0 @ 2003/01/21
	*/
	class CMatrixOperationNotAllowedException : public CFitException
	{
	public:
		CMatrixOperationNotAllowedException(int iLine, const char* szModule, const char* szName) : CFitException(iLine, szModule, szName, "Operation not allowed!") {}
	};

	/**
	* Basic methods for working with two dimensional matrices.
	*
	* @author		\item \URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
	* @author		\item \URL[Silke Humbert]{mailto:silke.humbert@iup.uni-heidelberg.de} @ \URL[IUP, Satellite Data Group]{http://giger.iup.uni-heidelberg.de}
	* @version		1.0 @ 2001/09/09
	*/
	class CMatrix
	{
	public:
		/**
		* Creates an empty matrix object.
		*/
		CMatrix()
		{
			mRows = NULL;
			mCols = NULL;
			mData = NULL;
			mAutoRelease = true;
			mSizeX = mSizeY = 0;
			mLineOffset = 0;

			mDoublePtr = NULL;
			mFloatPtr = NULL;
			mLUIndex = NULL;
		}

		/**
		* Create a new matrix object and fills it with the content of the given matrix.
		*
		* @param mRight	The originating matrix object.
		*/
		CMatrix(const CMatrix &mRight)
		{
			mRows = NULL;
			mCols = NULL;
			mData = NULL;
			mAutoRelease = true;
			mSizeX = mSizeY = 0;
			mLineOffset = 0;

			mDoublePtr = NULL;
			mFloatPtr = NULL;
			mLUIndex = NULL;

			Copy(mRight);
		}

		/**
		* Create a matrix object with the given size and width of the matrix.
		*
		* @param iCols		The number of columns in the matrix.
		* @param iRows		The number of rows in the matrix.
		*/
		CMatrix(int iCols, int iRows)
		{
			mRows = NULL;
			mCols = NULL;
			mData = NULL;
			mAutoRelease = true;
			mSizeX = mSizeY = 0;
			mLineOffset = 0;

			mDoublePtr = NULL;
			mFloatPtr = NULL;
			mLUIndex = NULL;

			SetSize(iCols, iRows);
		}

		/**
		* Create a submatrix object with the given size and width from the given matrix.
		*
		* @param mSecond	The originating matrix object
		* @param iStartCol	The index of the first column in the submatrix
		* @param iStartRow	The index of the first row in the submatrix
		* @param iCols		The number of columns in the submatrix.
		* @param iRows		The number of rows in the submatrix.
		*/
		CMatrix(CMatrix& mSecond, int iStartCol, int iStartRow, int iCols, int iRows)
		{
			MATHFIT_ASSERT((iStartCol + iCols) <= mSecond.GetNoColumns() && (iStartRow + iCols) <= mSecond.GetNoRows());
			MATHFIT_ASSERT(iCols > 0 && iRows > 0);

			mDoublePtr = NULL;
			mFloatPtr = NULL;
			mLUIndex = NULL;

			// create new vector array
			mRows = new CVector[iRows];
			mCols = new CVector[iCols];

			// get the subvector objects and attach them to our objects
			int i;
			for(i = 0; i < iRows; i++)
				mRows[i].Attach(mSecond.GetRow(iStartRow + i).SubVector(iStartCol, iCols));
			for(i = 0; i < iCols; i++)
				mCols[i].Attach(mSecond.GetCol(iStartCol + i).SubVector(iStartRow, iRows));

			mData = mRows[0].GetSafePtr();
			mAutoRelease = false;

			mSizeX = iCols;
			mSizeY = iRows;

			mLineOffset = max(mSecond.mSizeX, mSecond.mLineOffset);
		}

		/**
		* Frees and allocated resources.
		*/
		~CMatrix()
		{
			if(mRows)
				delete[] mRows;
			if(mCols)
				delete[] mCols;
			if(mData && mAutoRelease)
				delete mData;
			if(mLUIndex && mAutoRelease)
				delete mLUIndex;

			ReleaseDoublePtr();
			ReleaseFloatPtr();
		}

		/**
		* Copies the content of the given matrix into the current object.
		*
		* @param mOperand	The originating matrix object.
		*
		* @return A reference to the current object.
		*/
		CMatrix& Copy(const CMatrix& mOperand)
		{
			ClearLUDecomposed();

			SetSize(mOperand.GetNoColumns(), mOperand.GetNoRows());

			if(mSizeX <= 0 || mSizeY <= 0)
				return *this;

			memcpy(GetSafePtr(), mOperand.GetSafePtr(), sizeof(TFitData) * mSizeX * mSizeY);
			if(mOperand.IsLUDecomposed())
			{
				// okay. we also need to copy the LU index
				mLUIndex = new int[GetNoColumns()];
				memcpy(mLUIndex, mOperand.mLUIndex, sizeof(int) * GetNoColumns());
			}

			return *this;
		}

		CMatrix& Copy(TFitData* fData, int iRows, int iCols, bool bRowmajor = true)
		{
			SetSize(iCols, iRows);

			if(iCols <= 0 || iRows <= 0)
				return *this;

#if defined(ROWMATRIX)
			// check wheter the matrix is in rowmajor style
			if(bRowmajor)
				memcpy(GetSafePtr(), fData, sizeof(TFitData) * mSizeX * mSizeY);
			else
			{
				// we use columnmajor style, so we have to copy element by element
				int i, j;
				for(i = 0; i < iRows; i++)
					for(j = 0; j < iCols; j++)
						SetAt(i, j, fData[j * iRows + i]);
			}
#else
			// the default matrix style is columnmajor here
			if(!bRowmajor)
				memcpy(GetSafePtr(), fData, sizeof(TFitData) * mSizeX * mSizeY);
			else
			{
				int i, j;
				for(i = 0; i < iRows; i++)
					for(j = 0; j < iCols; j++)
						SetAt(i, j, fData[i * iCols + j]);
			}
#endif

			return *this;
		}

#if defined(MATHFIT_FITDATAFLOAT)
		CMatrix& Copy(double* fData, int iRows, int iCols)
		{
			SetSize(iCols, iRows);

			int i, j;
			for(i = 0; i < iRows; i++)
				for(j = 0; j < iCols; j++)
					SetAt(i, j, (TFitData)fData[i * iCols + j]);

			return *this;
		}
#else
		CMatrix& Copy(float* fData, int iRows, int iCols)
		{
			SetSize(iCols, iRows);

			int i, j;
			for(i = 0; i < iRows; i++)
				for(j = 0; j < iCols; j++)
					SetAt(i, j, (TFitData)fData[i * iCols + j]);

			return *this;
		}
#endif

		/**
		* Attaches the content of another CVector object to the current one
		*
		* @param mSecond		The originating object.
		* @param bAutoRelease	If TRUE the matrix data is freed on destructuion.
		*
		* @return	A reference to the current object.
		*/
		CMatrix& Attach(CMatrix& mSecond, bool bAutoRelease = true)
		{
			// first clear the old data
			if(mRows)
				delete[] mRows;
			if(mCols)
				delete[] mCols;
			if(mData && mAutoRelease)
				delete mData;
			if(mLUIndex && mAutoRelease)
				delete mLUIndex;

			// get the data pointer
			mRows = mSecond.mRows;
			mCols = mSecond.mCols;
			mData = mSecond.mData;
			mLUIndex = mSecond.mLUIndex;
			mSizeX = mSecond.mSizeX;
			mSizeY = mSecond.mSizeY;
			mLineOffset = mSecond.mLineOffset;
			if(bAutoRelease)
			{
				// so the current object will no take care about destruction of the vector data
				mAutoRelease = mSecond.mAutoRelease;
				mSecond.mAutoRelease = false;
			}
			else
				mAutoRelease = false;

			return *this;
		}

		/**
		* Attaches the content of another CVector object to the current one
		*
		* @param mSecond		The originating object.
		* @param bAutoRelease	If TRUE the matrix data is freed on destructuion.
		*
		* @return	A reference to the current object.
		*/
		CMatrix& Attach(TFitData* fData, int iRows, int iCols, bool bAutoRelease = true)
		{
			// first clear the old data
			if(mRows)
				delete[] mRows;
			if(mCols)
				delete[] mCols;
			if(mData && mAutoRelease)
				delete mData;
			if(mLUIndex && mAutoRelease)
				delete mLUIndex;
			mLUIndex = NULL;

			// get the data pointer
			mData = fData;
			mSizeX = iCols;
			mSizeY = iRows;
			mAutoRelease = bAutoRelease;

			mRows = new CVector[mSizeY];
			mCols = new CVector[mSizeX];

#if defined(ROWMATRIX)
			mLineOffset = mSizeX;

			int i;
			for(i = 0; i < mSizeY; i++)
				mRows[i].Attach(&GetSafePtr()[i * mLineOffset], mSizeX, 1, false);
			for(i = 0; i < mSizeX; i++)
				mCols[i].Attach(&GetSafePtr()[i], mSizeY, mLineOffset, false);
#else
			mLineOffset = mSizeY;

			int i;
			for(i = 0; i < mSizeY; i++)
				mRows[i].Attach(&GetSafePtr()[i], mSizeX, mLineOffset, false);
			for(i = 0; i < mSizeX; i++)
				mCols[i].Attach(&GetSafePtr()[i * mLineOffset], mSizeY, 1, false);
#endif

			return *this;
		}

		/**
		* Detaches the data object from the current object so the data isn't freed anymore when
		* the object is destroyed.
		*
		* @return	A reference to the current object
		*/
		CMatrix& Detach()
		{
			mRows = NULL;
			mCols = NULL;
			mSizeX = mSizeY = 0;
			mLineOffset = 0;
			mData = NULL;
			mLUIndex = NULL;
			mAutoRelease = true;

			ReleaseDoublePtr();
			ReleaseFloatPtr();

			return *this;
		}

		/**
		* Creates a matrix object representing a submatrix of the current object.
		* The data allocated is destroyed when the originating object is destroyed.
		* Therefore first remove all submatrix object before the originating matrix
		* object is deleted.
		*
		* @param iStartCol	The first column in the submatrix.
		* @param iStartRow	The first row in the submatrix.
		* @param iCols		The number of columns in the submatrix.
		* @param iRows		The number of rows in the submatrix.
		*
		* @return	A matrix object representing a submatrix of the given matrix
		*/
		CMatrix SubMatrix(int iStartCol, int iStartRow, int iCols, int iRows)
		{
			return CMatrix(*this, iStartCol, iStartRow, iCols, iRows);
		}

		/**
		* Exchanges the content of the current object with the content of another object.
		*
		* @param mSecond	The originating object.
		*
		* @return	A reference to the current object
		*/
		CMatrix& Exchange(CMatrix& mSecond)
		{
			int iSizeX = mSecond.mSizeX;
			int iSizeY = mSecond.mSizeY;
			int iLineOffset = mSecond.mLineOffset;
			CVector* vRows = mSecond.mRows;
			CVector* vCols = mSecond.mCols;
			TFitData* fData = mSecond.mData;
			int* iLUIndex = mSecond.mLUIndex;
			bool bAutoRelease = mSecond.mAutoRelease;
			double* fDoublePtr = mSecond.mDoublePtr;
			float* fFloatPtr = mSecond.mFloatPtr;

			mSecond.mSizeX = mSizeX;
			mSecond.mSizeY = mSizeY;
			mSecond.mLineOffset = mLineOffset;
			mSecond.mRows = mRows;
			mSecond.mCols = mCols;
			mSecond.mData = mData;
			mSecond.mLUIndex = mLUIndex;
			mSecond.mAutoRelease = mAutoRelease;
			mSecond.mDoublePtr = mDoublePtr;
			mSecond.mFloatPtr = mFloatPtr;

			mSizeX = iSizeX;
			mSizeY = iSizeY;
			mLineOffset = iLineOffset;
			mRows = vRows;
			mCols = vCols;
			mData = fData;
			mLUIndex = iLUIndex;
			mAutoRelease = bAutoRelease;
			mDoublePtr = fDoublePtr;
			mFloatPtr = fFloatPtr;

			return *this;
		}

		/**
		* Exchanges two rows of the matrix.
		*
		* @param iFirst	The index of the first row.
		* @param iSec		The index of the second row.
		* 
		* @return	A reference to the current object.
		*/
		CMatrix& ExchangeRows(int iFirst, int iSec)
		{
			// we need to physically copy the data. otherwise we get in trouble with some indicies
			CVector& vFirst = GetRow(iFirst);
			CVector& vSec = GetRow(iSec);

			CVector vTemp(vFirst);

			vFirst.Copy(vSec);

			vSec.Copy(vTemp);

			return *this;
		}

		/**
		* Exchanges two columns of the matrix.
		*
		* @param iFirst	The index of the first column.
		* @param iSec		The index of the second column.
		* 
		* @return	A reference to the current object.
		*/
		CMatrix& ExchangeCols(int iFirst, int iSec)
		{
			// we need to physically copy the data. otherwise we get in trouble with some indicies
			CVector& vFirst = GetCol(iFirst);
			CVector& vSec = GetCol(iSec);

			CVector vTemp(vFirst);

			vFirst.Copy(vSec);

			vSec.Copy(vTemp);

			return *this;
		}

		/**
		* Returns the desired row vector.
		*
		* @param iRow	The row you want to access.
		*
		* @return	A reference to the selected row vector.
		*/
		CVector& GetRow(const int iRow) const
		{
			MATHFIT_ASSERT(iRow >= 0 && iRow < mSizeY);

			return mRows[iRow];
		}

		CVector& GetCol(const int iCol) const
		{
			MATHFIT_ASSERT(iCol >= 0 && iCol < mSizeX);

			return mCols[iCol];
		}

		/**
		* Returns the desired element.
		*
		* @param iRow	The row you want to access.
		* @param iCol	The column you want to access.
		*
		* @return	The selected matrix element.
		*/
		TFitData GetAt(const int iRow, const int iCol) const
		{
			MATHFIT_ASSERT(iRow >= 0 && iRow < mSizeY && iCol >= 0 && iCol < mSizeX);

#if defined(ROWMATRIX)
			return GetSafePtr()[iRow * mLineOffset + iCol];
#else
			return GetSafePtr()[iRow + iCol * mLineOffset];
#endif
		}

		TFitData GetAtSafe(int iRow, int iCol) const
		{
			if(iRow < 0)
				iRow = 0;
			else if(iRow >= mSizeY)
				iRow = mSizeY - 1;

			if(iCol < 0)
				iCol = 0;
			else if(iCol >= mSizeX)
				iCol = mSizeX - 1;

#if defined(ROWMATRIX)
			return GetSafePtr()[iRow * mLineOffset + iCol];
#else
			return GetSafePtr()[iRow + iCol * mLineOffset];
#endif
		}

		/**
		* Sets the content of a single row.
		*
		* @param iRow	The row which should be modified.
		* @param vRow	The vector object with the new row data.
		*
		* @return	A reference to the current object.
		*/
		CMatrix& SetRow(int iRow, CVector& vRow)
		{
			MATHFIT_ASSERT(iRow >= 0 && iRow < mSizeY);

			GetRow(iRow).Copy(vRow);

			return *this;
		}

		CMatrix& SetCol(int iCol, CVector& vCol)
		{
			MATHFIT_ASSERT(iCol >= 0 && iCol < mSizeX);

			GetCol(iCol).Copy(vCol);

			return *this;
		}

		/**
		* Sets the content of a single element.
		*
		* @param iRow	The row which should be modified.
		* @param iCol	The column which element should be modified.
		* @param fData	The content of the element.
		*
		* @return	A reference to the current object.
		*/
		CMatrix& SetAt(int iRow, int iCol, TFitData fData)
		{
			MATHFIT_ASSERT(iRow >= 0 && iRow < mSizeY && iCol >= 0 && iCol < mSizeX);

#if defined(ROWMATRIX)
			GetSafePtr()[iRow * mLineOffset + iCol] = fData;
#else
			GetSafePtr()[iRow + iCol * mLineOffset] = fData;
#endif

			return *this;
		}

		/**
		* Returns the number of columns of the matrix.
		*
		* @return	The number of columns.
		*/
		const int GetNoColumns() const
		{
			return mSizeX;
		}

		/**
		* Returns the number of rows of the matrix.
		*
		* @return	The number of rows.
		*/
		const int GetNoRows() const
		{
			return mSizeY;
		}

		/**
		* Sets the size of the matrix. 
		* If the matrxi needs to be resized the neccessary buffer is reallocated.
		*
		* @param iXSize	The number of columns of the matrix.
		* @param iYSize	The number of rows of the matrix.
		*/
		void SetSize(int iXSize, int iYSize)
		{
			MATHFIT_ASSERT(iXSize >= 0 && iYSize >= 0);

			if(iYSize != mSizeY || iXSize != mSizeX || !mData)
			{
				ClearLUDecomposed();

				if(mRows)
					delete[] mRows;
				mRows = NULL;
				if(mCols)
					delete[] mCols;
				mCols = NULL;
				if(mData && mAutoRelease)
					delete mData;
				mData = NULL;

				mSizeY = iYSize;
				mSizeX = iXSize;

				if(mSizeX <= 0 || mSizeY <= 0)
					return;

				mRows = new CVector[iYSize];
				mCols = new CVector[iXSize];
				mData = new TFitData[iXSize * iYSize];

#if defined(ROWMATRIX)
				mLineOffset = mSizeX;

				int i;
				for(i = 0; i < mSizeY; i++)
					mRows[i].Attach(&GetSafePtr()[i * mLineOffset], mSizeX, 1, false);
				for(i = 0; i < mSizeX; i++)
					mCols[i].Attach(&GetSafePtr()[i], mSizeY, mLineOffset, false);
#else
				mLineOffset = mSizeY;

				int i;
				for(i = 0; i < mSizeY; i++)
					mRows[i].Attach(&GetSafePtr()[i], mSizeX, mLineOffset, false);
				for(i = 0; i < iXSize; i++)
					mCols[i].Attach(&GetSafePtr()[i * mLineOffset], mSizeY, 1, false);
#endif
			}
		}

		/**
		* Fills the matrix with zeros except of the diagonal elements which are set to one.
		*
		* @return	A reference to the current object.
		*/
		CMatrix& Idendity()
		{
			Zero();

			int i;
			for(i = 0; i < mSizeY; i++)
				SetAt(i, i, 1);

			return *this;
		}

		/**
		* Fills the matrix with values defined.
		* To fill the matrix the following formular is used:
		* a(i,j) = fStart + i * fColSlope + j * fRowSlope
		*
		* @param fStart	The value of the first element.
		* @param fColSlope	The increasement factor for the next column.
		* @param fRowSlope	The increasement factor for the next row.
		*
		* @return	A reference to the current object.
		*/
		CMatrix& Wedge(TFitData fStart, TFitData fColSlope, TFitData fRowSlope)
		{
			TFitData fVal = fStart;

			int i;
			for(i = 0; i < mSizeY; i++, fVal += fRowSlope)
				GetRow(i).Wedge(fVal, fColSlope);

			return *this;
		}

		/**
		* Adds the content of two matrices element by element.
		*
		* @param mOperant	The matrix which should be added to the current one.
		*
		* @return	A reference to the current object.
		*/
		CMatrix& Add(CMatrix& mOperant)
		{
			MATHFIT_ASSERT(mSizeY == mOperant.GetNoRows() && mSizeX == mOperant.GetNoColumns());

			int i;
			for(i = 0; i < mSizeY; i++)
				GetRow(i).Add(mOperant.GetRow(i));

			return *this;
		}

		/**
		* Subtracts the content of two matrices element by element.
		*
		* @param mOperant	The matrix which should be subtracted to the current one.
		*
		* @return	A reference to the current object.
		*/
		CMatrix& Sub(CMatrix& mOperant)
		{
			MATHFIT_ASSERT(mSizeY == mOperant.GetNoRows() && mSizeX == mOperant.GetNoColumns());

			int i;
			for(i = 0; i < mSizeY; i++)
				GetRow(i).Sub(mOperant.GetRow(i));

			return *this;
		}

		/**
		* Multiplies two matrices using the standard matrix multiplication.
		*
		* @param mOperant	The matrix with which the current one should be multiplicated.
		*
		* @return	A reference to the current object.
		*/
		CMatrix& Mul(CMatrix& mOperant)
		{
			MATHFIT_ASSERT(mSizeY == mOperant.GetNoColumns() && mSizeX == mOperant.GetNoRows());

			CMatrix mRes(mSizeY, mSizeY);

			int i, j;
			for(i = 0; i < mSizeY; i++)
				for(j = 0; j < mSizeY; j++)
				{
					int k;
					TFitData fSum = 0;
					for(k = 0; k < mSizeX; k++)
						fSum += GetAt(i, k) * mOperant.GetAt(k, j);
					mRes.SetAt(i, j, fSum);
				}

				// get data from temporary object
				Attach(mRes);
				mRes.Detach();

				return *this;
		}

		/**
		* Multiplies the matrix with a vector.
		*
		* @param mOperant	The vector with which the matrix should be mutiplied. This vector also contains the result.
		*
		* @return	A reference to the given vector object, that will contain the result.
		*/
		CVector& Mul(CVector& mOperant)
		{
			MATHFIT_ASSERT(mSizeX == mOperant.GetSize());

			CVector vTemp(mSizeY);

			int i, j;
			for(i = 0; i < mSizeY; i++)
			{
				TFitData fSum = 0;
				for(j = 0; j < mSizeX; j++)
					fSum += GetAt(i, j) * mOperant.GetAt(j);
				vTemp.SetAt(i, fSum);
			}

			mOperant.Attach(vTemp);
			vTemp.Detach();

			return mOperant;
		}

		/**
		* Multipies the matrix with the inverse of the given operant.
		*
		* @param mOperant	The matrix by which the current one should be divided.
		*
		* @return	A reference to the current object.
		*/
		CMatrix& Div(CMatrix& mOperant)
		{
			CMatrix mTemp(mOperant);
			mTemp.Inverse();
			return Mul(mTemp);
		}

		/**
		* Adds a scalar to every element of the matrix.
		*
		* @param fOperant	The scalar to be added.
		*
		* @return	A reference to the current object.
		*/
		CMatrix& Add(TFitData fOperant)
		{
			int i;
			for(i = 0; i < mSizeY; i++)
				GetRow(i).Add(fOperant);

			return *this;
		}

		/**
		* Subtracts a scalar from every element of the matrix.
		*
		* @param fOperant	The scalar.
		*
		* @return	A reference to the current object.
		*/
		CMatrix& Sub(TFitData fOperant)
		{
			int i;
			for(i = 0; i < mSizeY; i++)
				GetRow(i).Sub(fOperant);

			return *this;
		}

		/**
		* Multiplies every element of the matrix by the given scalar.
		*
		* @param fOperant	The scalar.
		*
		* @return	A reference to the current object.
		*/
		CMatrix& Mul(TFitData fOperant)
		{
			int i;
			for(i = 0; i < mSizeY; i++)
				GetRow(i).Mul(fOperant);

			return *this;
		}

		/**
		* Divide every element of the matrix by the given scalar.
		*
		* @param fOperant	The scalar.
		*
		* @return	A reference to the current object.
		*/
		CMatrix& Div(TFitData fOperant)
		{
			int i;
			for(i = 0; i < mSizeY; i++)
				GetRow(i).Div(fOperant);

			return *this;
		}

		/**
		* Adds the content of two matrices element by element and scaling the second operand by the given factor.
		*
		* @param mOperant	The matrix which should be added to the current one.
		* @param fFactor	The factor used to multiply the second operand.
		*
		* @return	A reference to the current object.
		*/
		CMatrix& Add(CMatrix& mOperant, TFitData fFactor)
		{
			MATHFIT_ASSERT(mSizeY == mOperant.GetNoRows() && mSizeX == mOperant.GetNoColumns());

			int i;
			for(i = 0; i < mSizeY; i++)
				GetRow(i).Add(mOperant.GetRow(i), fFactor);

			return *this;
		}

		/**
		* Subtracts the content of two matrices element by element and scaling the second operand by the given factor.
		*
		* @param mOperant	The matrix which should be subtracted to the current one.
		* @param fFactor	The factor used to multiply the second operand.
		*
		* @return	A reference to the current object.
		*/
		CMatrix& Sub(CMatrix& mOperant, TFitData fFactor)
		{
			MATHFIT_ASSERT(mSizeY == mOperant.GetNoRows() && mSizeX == mOperant.GetNoColumns());

			int i;
			for(i = 0; i < mSizeY; i++)
				GetRow(i).Sub(mOperant.GetRow(i), fFactor);

			return *this;
		}

		/**
		* Adds the given vector to the diagonal elements of the matrix.
		*
		* @param vSec	The vector to be added.
		*
		* @return	A reference to the current object.
		*/
		CMatrix& AddDiag(const CVector& vSec)
		{
			const int iSecSize = vSec.GetSize();

			MATHFIT_ASSERT(iSecSize == mSizeX && iSecSize == mSizeY);

			int i;
			for(i = 0; i < iSecSize; i++)
				SetAt(i, i, GetAt(i, i) + vSec.GetAt(i));

			return *this;
		}

		/**
		* Subtractss the given vector from the diagonal elements of the matrix.
		*
		* @param vSec	The vector to be substracted.
		*
		* @return	A reference to the current object.
		*/
		CMatrix& SubDiag(const CVector& vSec)
		{
			const int iSecSize = vSec.GetSize();

			MATHFIT_ASSERT(iSecSize == mSizeX && iSecSize == mSizeY);

			int i;
			for(i = 0; i < iSecSize; i++)
				SetAt(i, i, GetAt(i, i) - vSec.GetAt(i));

			return *this;
		}

		/**
		* Multiplies the diagonal elements of the matrix by the given vector.
		*
		* @param vSec	The vector to be multiplied.
		*
		* @return	A reference to the current object.
		*/
		CMatrix& MulDiag(const CVector& vSec)
		{
			const int iSecSize = vSec.GetSize();

			MATHFIT_ASSERT(vSec.GetSize() == mSizeX && vSec.GetSize() == mSizeY);

			int i;
			for(i = 0; i < iSecSize; i++)
				SetAt(i, i, GetAt(i, i) * vSec.GetAt(i));

			return *this;
		}

		/**
		* Divides the diagonal elements of the matrix by the given vector.
		*
		* @param vSec	The vector to be divided.
		*
		* @return	A reference to the current object.
		*/
		CMatrix& DivDiag(const CVector& vSec)
		{
			const int iSecSize = vSec.GetSize();

			MATHFIT_ASSERT(iSecSize == mSizeX && iSecSize == mSizeY);

			int i;
			for(i = 0; i < iSecSize; i++)
				SetAt(i, i, GetAt(i, i) / vSec.GetAt(i));

			return *this;
		}

		/**
		* Adds the given constant to the diagonal elements of the matrix.
		*
		* @param fSec	The constant to be added.
		*
		* @return	A reference to the current object.
		*/
		CMatrix& AddDiag(const TFitData fSec)
		{
			// matrix needs to be square
			MATHFIT_ASSERT(mSizeX == mSizeY);

			int i;
			for(i = 0; i < mSizeX; i++)
				SetAt(i, i, GetAt(i, i) + fSec);

			return *this;
		}

		/**
		* Subtractss the given constant from the diagonal elements of the matrix.
		*
		* @param fSec	The constant to be substracted.
		*
		* @return	A reference to the current object.
		*/
		CMatrix& SubDiag(const TFitData fSec)
		{
			MATHFIT_ASSERT(mSizeX == mSizeY);

			int i;
			for(i = 0; i < mSizeX; i++)
				SetAt(i, i, GetAt(i, i) - fSec);

			return *this;
		}

		/**
		* Multiplies the diagonal elements of the matrix by the given constant.
		*
		* @param fSec	The constant to be multiplied.
		*
		* @return	A reference to the current object.
		*/
		CMatrix& MulDiag(const TFitData fSec)
		{
			MATHFIT_ASSERT(mSizeX == mSizeY);

			int i;
			for(i = 0; i < mSizeX; i++)
				SetAt(i, i, GetAt(i, i) * fSec);

			return *this;
		}

		/**
		* Divides the diagonal elements of the matrix by the given constant.
		*
		* @param fSec	The constant to be divided.
		*
		* @return	A reference to the current object.
		*/
		CMatrix& DivDiag(const TFitData fSec)
		{
			MATHFIT_ASSERT(mSizeX == mSizeY);

			int i;
			for(i = 0; i < mSizeX; i++)
				SetAt(i, i, GetAt(i, i) / fSec);

			return *this;
		}

		/**
		* Solves the linear equation system using the Gauss-Jordan elimination method.
		*
		* \begin{verbatim}LES: a*x=b\end{verbatim}
		*
		* The Algorithm here is based on the Gauss-Jordan Elimination from Numerical Recipes in C, p. 36-40
		* The current matrix will be inverted after the algorithm and mBeta will contain the result.
		* The result will also be placed in \Ref{mBeta}.
		*
		* @param mBeta		The matrix containing the (b) part of the LES, that will receive the result.
		*
		* @return A reference to the given matrix object.
		*
		* @exception CVectorSizeMismatch 
		* @exception CMatrixSolveFailed 
		* @exception CMatrixNotSquare
		*/
		CMatrix& GaussJordanSolve(CMatrix& mBeta)
		{
			const int iCols = GetNoColumns();
			const int iRows = GetNoRows();

			if(iCols != iRows)
				throw(EXCEPTION(CMatrixNotSquareException));

			if(iRows != mBeta.GetNoRows())
				throw(EXCEPTION(CVectorSizeMismatchException));

			int* iIndexCol = new int[iCols];
			memset(iIndexCol, 0, sizeof(int) * iCols);
			int* iIndexRow = new int[iRows];
			memset(iIndexRow, 0, sizeof(int) * iRows);
			int* iPivotDone = new int[iCols];
			memset(iPivotDone, 0, sizeof(int) * iCols);
			int iR, iC;

			for(int i = 0; i < iCols; i++)
			{
				// find pivot
				iR = iC = i;
				TFitData fMag = 0;
				for(int j = 0; j < iRows; j++)
				{
					if(iPivotDone[j] != 1)
					{
						int k;
						for(k = 0; k < iRows; k++)
						{
							if(iPivotDone[k] == 0)
							{
								if(fabs(GetAt(j, k)) >= fMag)
								{
									fMag = (TFitData)fabs(GetAt(j, k));
									iR = j;
									iC = k;
								}
							}
							else if(iPivotDone[k] > 1)
							{
								// this pivot was selected more than once. Shit!!
								delete[] iPivotDone;
								delete[] iIndexRow;
								delete[] iIndexCol;

								throw(EXCEPTION(CMatrixSolveFailedException));
							}
						}
					}
				}
				iPivotDone[iC]++;

				// move pivot row into position
				if(iR != iC)
				{
					ExchangeRows(iR, iC);
					mBeta.ExchangeRows(iR, iC);
				}	

				// store indices
				iIndexRow[i] = iR;
				iIndexCol[i] = iC;

				// get scaling of pivot row
				fMag = GetAt(iC, iC);

				// no pivot: error
				if(fMag == 0)
				{
					// zero pivot => not a singular matrix
					delete[] iPivotDone;
					delete[] iIndexRow;
					delete[] iIndexCol;

					throw(EXCEPTION(CMatrixSolveFailedException));
				}

				SetAt(iC, iC, 1);
				GetRow(iC).Div(fMag);
				mBeta.GetRow(iC).Div(fMag);

				// eliminate pivot row component from other rows
				for(int i2 = 0; i2 < iRows; i2++)
				{
					if(i2 == iC)
						continue;

					// get factor to eliminate pivot column
					TFitData fMag2 = GetAt(i2, iC);
					SetAt(i2, iC, 0);
					GetRow(i2).Sub(GetRow(iC), fMag2);
					mBeta.GetRow(i2).Sub(mBeta.GetRow(iC), fMag2);
				}
			}

			// reorder matrix
			for (int l = iRows - 1; l >= 0; l--) {
				if (iIndexRow[l] != iIndexCol[l]) {
					ExchangeCols(iIndexRow[l], iIndexCol[l]);
				}
			}

			delete[] iPivotDone;
			delete[] iIndexRow;
			delete[] iIndexCol;

			return mBeta;
		}

		/**
		* Solves the linear equation system given using the Gauss-Jordan elimination method.
		*
		* \begin{verbatim}LES: a*x=b\end{verbatim}
		*
		* The result will be placed in \Ref{vBeta} and the matrix will be inverted.
		*
		* @param vBeta		The vector containing the (b) part of the LES, that will receive the result.
		*
		* @return A reference to the vector object.
		*
		* @exception CVectorSizeMismatch
		* @exception CMatrixSolveFailed
		*/
		CVector& GaussJordanSolve(CVector& vBeta)
		{
			MATHFIT_ASSERT(vBeta.GetStepSize() == 1);

			// attach the vector to a matrix object with just a single column
			CMatrix mBeta; 
			mBeta.Attach(vBeta.GetSafePtr(), vBeta.GetSize(), 1, false);

			// die geschichte lösen lassen
			GaussJordanSolve(mBeta);

			return vBeta;
		}

		/**
		* Calculates the multiplicative inverse of the current matrix.
		*
		* @return The inverted matrix.
		*/
		CMatrix& Inverse()
		{
			MATHFIT_ASSERT(mSizeX == mSizeY);

			CMatrix mResult;

			// create idendity matrix
			mResult.SetSize(1, mSizeY);

			// solve the LES
			GaussJordanSolve(mResult);

			return *this;
		}

		/**
		* Fills the matrix with zeros.
		*
		* @return The cleared matrix.
		*/
		CMatrix& Zero()
		{
			MATHFIT_ASSERT(mData != NULL);

			ClearLUDecomposed();

#if defined(ROWMATRIX)
			int i;
			for(i = 0; i < mSizeY; i++)
				GetRow(i).Zero();
#else
			int i;
			for(i = 0; i < mSizeX; i++)
				GetCol(i).Zero();
#endif

			return *this;
		}

		/**
		* Checks wheter all elements of the matrix are zero.
		*
		* @return	TRUE if all elements are zero, FALSE otherwise
		*/
		bool IsZero() const
		{
			int i;
			for(i = 0; i < mSizeY; i++)
				if(!GetRow(i).IsZero())
					return false;

			return true;
		}

		/**
		* Rotate the matrix by 90 degrees clockwise.
		*
		* @return The transposed matrix.
		*/
		CMatrix& Transpose()
		{
			CMatrix mNew(mSizeY, mSizeX);

			int iRow, iCol;
			for(iRow = 0; iRow < mSizeY; iRow++)
				for(iCol = 0; iCol < mSizeX; iCol++)
					mNew.SetAt(iCol, iRow, GetAt(iRow, iCol));

			Attach(mNew);
			mNew.Detach();

			return *this;
		}

		/**
		* Builds the pseudo inverse of a matrix.
		*
		* (A_t * A)
		*
		* @return The transposed matrix.
		*/
		CMatrix& PseudoInverse()
		{
			CMatrix mNew(mSizeX, mSizeX);

			// fill in the upper diagonal matrix

			int iRow, iCol;
			for(iRow = 0; iRow < mSizeX; iRow++)
				for(iCol = iRow; iCol < mSizeX; iCol++)
				{
					TFitData fSum = 0;
					int k;
					for(k = 0; k < mSizeY; k++)
						fSum += GetAt(k, iRow) * GetAt(k, iCol);
					mNew.SetAt(iRow, iCol, fSum);
				}

				// now we have to fill in the lowe matrix
				for(iRow = 0; iRow < mSizeX; iRow++)
					for(iCol = iRow; iCol < mSizeX; iCol++)
						mNew.SetAt(iCol, iRow, mNew.GetAt(iRow, iCol));

				Attach(mNew);
				mNew.Detach();

				return *this;
		}

		/**
		* Decomposes the matrix in a lower and upper triangular matrix.
		*
		* @return A matrix that is composed of a lower and an upper triangular matrix
		*/
		CMatrix& LUDecomposition()
		{
			if(GetNoColumns() != GetNoRows())
				throw(EXCEPTION(CMatrixNotSquareException));

			int i, iMax, j, k;
			int iN = GetNoColumns();
			TFitData fBig, fDum, fSum, fTemp;	

			if(mLUIndex && mAutoRelease)
				delete mLUIndex;
			mLUIndex = new int[iN];
			CVector vV(iN);

			iMax = 0;
			for(i = 0; i < iN; i++)
			{
				fBig = 0.0;
				for(j = 0; j < iN; j++)
					if((fTemp = (TFitData)fabs(GetAt(j, i))) > fBig) 
						fBig = fTemp;
				if(fBig == 0.0)
				{
					delete mLUIndex;
					mLUIndex = NULL;
					throw(EXCEPTION(CMatrixSingularException));
				}

				// No nonzero largest element
				vV.SetAt(i, (TFitData)1.0 / fBig);
			}

			for(j = 0; j < iN; j++)
			{
				for(i = 0; i < j; i++)
				{
					fSum = GetAt(i, j);
					for(k = 0; k < i; k++) 
						fSum -= GetAt(i, k) * GetAt(k, j);
					SetAt(i, j, fSum);
				}

				fBig = 0.0;
				for(i = j; i < iN; i++)
				{
					fSum = GetAt(i, j);
					for(k = 0; k < j; k++) 
						fSum -= GetAt(i, k) * GetAt(k, j);
					SetAt(i, j, fSum);
					if((fDum = vV.GetAt(i) * (TFitData)fabs(fSum)) >= fBig)
					{
						fBig = fDum;
						iMax = i;
					}
				}

				if(j != iMax)
				{
					for(k = 0; k < iN; k++)
					{
						fDum = GetAt(iMax, k);
						SetAt(iMax, k, GetAt(j, k));
						SetAt(j, k, fDum);
					}

					vV.SetAt(iMax, vV.GetAt(j));
				}

				mLUIndex[j] = iMax;
				if((GetAt(j, j)) == 0.0)
					SetAt(j, j, MATHFIT_NEARLYZERO);

				if(j != iN - 1)
				{
					fDum = (TFitData)1.0 / GetAt(j, j);
					for(i = j + 1; i < iN; i++) 
						SetAt(i, j, GetAt(i, j) * fDum);
				}
			}

			return *this;
		}


		/**
		* Solves the linear equation.
		* Given the decomposed matrix returned by LUDecomposition the linear equation system is solved, 
		*
		* @param vResult right-hand side vector
		*
		* @return Returns the solution vector.
		*/
		CVector& LUBacksubstitution(CVector& vResult)
		{
			MATHFIT_ASSERT(IsLUDecomposed());

			int iN = GetNoColumns();
			int i, j;
			int iP;
			int iI;
			TFitData fSum;

			// decrement n to keep original structure of program
			iI = -1;

			for(i = 0; i < iN; i++)
			{
				iP = mLUIndex[i];
				fSum = vResult.GetAt(iP);
				vResult.SetAt(iP, vResult.GetAt(i));
				if(iI >= 0)
				{
					for(j = iI; j < i; j++) 
						fSum -= GetAt(i, j) * vResult.GetAt(j);
				}
				else if(fSum)
					iI = i;
				vResult.SetAt(i, fSum);
			}

			for(i = iN - 1; i >= 0; i--)
			{	
				fSum = vResult.GetAt(i);
				for(j = i + 1; j < iN; j++) 
					fSum -= GetAt(i, j) * vResult.GetAt(j);
				vResult.SetAt(i, fSum / GetAt(i, i));
			}
			return vResult;
		}

		/**
		* Calculates the Inverse Matrix.
		* Given the decomposed Matrix and the index vector returned by LUDecomposition, the inverse matrix
		* will be computed. The inverse matrix is stored in the current matrix object. Therefore
		* the decomposed LU matrix is no longer avaliable!
		*
		* @return A reference to the current matrix object that now holds the inverse.
		*/
		CMatrix& LUInverse()
		{
			MATHFIT_ASSERT(IsLUDecomposed());

			int iN, j;
			iN = GetNoColumns();

			CMatrix mResult(iN, iN);
			mResult.Zero();

			for(j = 0; j < iN; j++)
			{
				mResult.SetAt(j, j, 1.0);
				LUBacksubstitution(mResult.GetCol(j));
			}
			Attach(mResult);
			mResult.Detach();

			return *this;
		}

		float* GetFloatPtr()
		{
			ReleaseFloatPtr();

			mFloatPtr = new float[GetNoColumns() * GetNoRows()];

			int i, j;
			for(i = 0; i < GetNoRows(); i++)
				for(j = 0; j < GetNoColumns(); j++)
				mFloatPtr[i * GetNoColumns() + j] = (float)GetAt(i, j);

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

			mDoublePtr = new double[GetNoColumns() * GetNoRows()];

			int i, j;
			for (i = 0; i < GetNoRows(); i++) {
				for (j = 0; j < GetNoColumns(); j++) {
					mDoublePtr[i * GetNoColumns() + j] = (double)GetAt(i, j);
				}
			}

			return mDoublePtr;
		}

		void ReleaseDoublePtr()
		{
			if(mDoublePtr)
				delete mDoublePtr;
			mDoublePtr = NULL;
		}

		TFitData* GetSafePtr() const
		{
			MATHFIT_ASSERT(mSizeX > 0 && mSizeY > 0);
			MATHFIT_ASSERT(_CrtIsValidPointer(mData, sizeof(mData[0]) * mSizeX * mSizeY, TRUE));

			return mData;
		}

		bool IsLUDecomposed() const
		{
			return mLUIndex != NULL ? true : false;
		}

		void ClearLUDecomposed()
		{
			if(mLUIndex && mAutoRelease)
				delete mLUIndex;
			mLUIndex = NULL;
		}

		/**
		 * Assignment operator
		 *
		 * This overload is present to prevent the user from accidently assigning matrix objects without knowing
		 * wheter the content is copied or attached. Please use explicitly Copy or Attach instead.
		 */
		CMatrix& operator=(CMatrix& vOp)
		{
			// !! The direct assignment of matrices is not allowed, since we may get into ambigousity wheter we have to 
			// !! attach or copy the current matrix.
			// !! Therefore explicitly define wheter you want to attach or copy one matrix to another object.
			throw EXCEPTION(CMatrixOperationNotAllowedException);
			return vOp;
		}

		/**
		* Prints the content of the matrix to the given out stream.
		*
		* @param os	The output stream
		* @param mMatr	The matrix which content should be displayed.
		*
		* @return A reference to the output stream itself.
		*/
		friend std::ostream& operator<<(std::ostream& os, CMatrix& mMatr)
		{
			int i;
			for(i = 0; i < mMatr.GetNoRows(); i++)
				os << mMatr.GetRow(i) << std::endl;
			return os;
		}

	private:
		/**
		* The number of columns in the matrix.
		*/
		int mSizeX;
		/**
		* The number of rows in the matrix.
		*/
		int mSizeY;
		/**
		* Array containing the row vectors.
		*/
		CVector* mRows;
		CVector* mCols;
		TFitData* mData;
		bool mAutoRelease;
		int mLineOffset;
		double* mDoublePtr;
		float* mFloatPtr;
		int* mLUIndex;
	};
}

#pragma warning (pop)
#endif
