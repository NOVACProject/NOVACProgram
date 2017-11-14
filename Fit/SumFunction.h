/**
 * Contains an object that builds the sum of the given operants.
 *
 * @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
 * @version		1.0 @ 2001/09/21
 */
#if !defined(SUMFUNCTION_H_011206)
#define SUMFUNCTION_H_011206

#include "ParamFunction.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning (push, 3)

namespace MathFit
{
	/**
	* Builds the sum of the given operants.
	*
	* @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
	* @version		1.0 @ 2001/09/23
	*/
	class CSumFunction : public IParamFunction
	{
	public:
		/**
		* Creates an empty reference object.
		*/
		CSumFunction()
		{
			mMaxOperands = DEFAULTNOOPERANDS;
			mOperands = new IParamFunction*[mMaxOperands];
			mOperandsCount = 0;
		}

		/**
		* Releases any alloced resources.
		*/
		~CSumFunction()
		{
			if(mOperands)
				delete(mOperands);
		}

		/**
		* Adds the given function to the object.
		* If neccessary, the internal buffer are adapted to hold the complete set of functions.
		*
		* @param ipfRef	The reference function object to be added.
		*
		* @return	TRUE is successful, FALSE otherwise.
		*/
		virtual bool AddOperand(IParamFunction& ipfRef)
		{
			// if we exceed the maximum number of operands, increase the array
			if(mOperandsCount >= mMaxOperands)
			{
				// create a new list
				IParamFunction** pNewList = new IParamFunction*[mMaxOperands * 2];
				memset(pNewList, 0, mMaxOperands * 2 * sizeof(IParamFunction*));

				// copy the old elements
				memmove(pNewList, mOperands, mOperandsCount * sizeof(IParamFunction*));

				// replace old list
				IParamFunction** pTemp = mOperands;
				mOperands = pNewList;
				mMaxOperands *= 2;

				// get rid of the old list
				delete(pTemp);
			}

			// search, wheter the operand already exists
			int i;
			for(i = 0; i < mOperandsCount; i++)
				if(mOperands[i] == &ipfRef)
					return true;

			mOperands[mOperandsCount++] = &ipfRef;

			BuildLinearParameter();
			BuildNonlinearParameter();

			return true;
		}

		/**
		* Removes the given function from the object.
		* If neccessary, the buffer size of adjusted, if there is plenty of space left
		*
		* @param ipfRef	The reference function object to be removed.
		*
		* @return	TRUE if successful, FALSE otherwise.
		*/
		virtual bool RemoveOperand(IParamFunction& ipfRef)
		{
			int i;
			for(i = 0; i < mOperandsCount; i++)
				if(mOperands[i] == &ipfRef)
				{
					int j;
					for(j = i; j < mOperandsCount - 1; j++)
						mOperands[j] = mOperands[j + 1];
					mOperandsCount--;
					break;
				}

				// operand not found?
				if(i == mOperandsCount)
					return false;

				// build new parameter vectors
				BuildLinearParameter();
				BuildNonlinearParameter();

				// check for wasted space
				if(mMaxOperands > DEFAULTNOOPERANDS)
				{
					int iMinArraySize = mMaxOperands / 3;

					// if we have less than a third of the whole array filled, we decrease its size by half.
					if(mOperandsCount <= iMinArraySize)
					{
						// create a new list
						IParamFunction** pNewList = new IParamFunction*[mMaxOperands / 2];
						memset(pNewList, 0, mMaxOperands * sizeof(IParamFunction*) / 2);

						// copy the old elements
						memmove(pNewList, mOperands, mOperandsCount * sizeof(IParamFunction*));

						// replace old list
						IParamFunction** pTemp = mOperands;
						mOperands = pNewList;
						mMaxOperands /= 2;

						// get rid of the old list
						delete(pTemp);
					}
				}

				return true;
		}

		/**
		* Returns the value of the model function.
		* The model function is just the sum of all references.
		*
		* \begin{verbatim}f(x)=sum(ref_i(x))
		*
		* where
		\begin{verbatim}
		ref_i := The i-th reference
		\end{verbatim}
		*
		* @param fXValue	The X data point.
		*
		* @return	The value of the model at the given data point.
		*/
		virtual TFitData GetValue(TFitData fXValue)
		{
			TFitData fSum = 0;
			int i;
			for(i = 0; i < mOperandsCount; i++)
				fSum += mOperands[i]->GetValue(fXValue);

			return fSum;
		}

		/**
		* Calculates the function values at a set of given data points.
		*
		* @param vXValues			A vector object containing the X values at which the function has to be evaluated.
		* @param vYTargetVector	A vector object which receives the resulting function values.
		*
		* @return	A reference to the Y vector object.
		*
		* @see GetValue
		*/
		virtual CVector& GetValues(CVector& vXValues, CVector& vYTargetVector)
		{
			const int iXSize = vXValues.GetSize();

			vYTargetVector.SetSize(iXSize);
			vYTargetVector.Zero();

			CVector vBuffer(iXSize);

			int i;
			for(i = 0; i < mOperandsCount; i++)
			{
				mOperands[i]->GetValues(vXValues, vBuffer);
				vYTargetVector.Add(vBuffer);
			}

			return vYTargetVector;
		}

		/**
		* Returns the first derivative of the model function at the given data point.
		* Since the model itself is just the sum of all references, the first derivative is the sum of all reference derivatives.
		*
		* \begin{verbatim}f'(x)=sum(ref_i'(x))
		*
		* where
		\begin{verbatim}
		ref_i' := the first derivative of the i-th reference
		\end{verbatim}
		*
		* @param fXValue	The X data point at which the first derivation is needed.
		*
		* @return	The slope of the model at the given data point.
		*/
		virtual TFitData GetSlope(TFitData fXValue)
		{
			TFitData  fSum = 0;
			int i;
			for(i = 0; i < mOperandsCount; i++)
				fSum += mOperands[i]->GetSlope(fXValue);

			return fSum;
		}

		/**
		* Calculates the first derivative of the function at a set of given data points.
		*
		* @param vXValues		A vector object containing the X values at which the function has to be evaluated.
		* @param vSlopeVector	A vector object which receives the resulting function values.
		*
		* @return	A reference to the slope vector object.
		*
		* @see	GetSlope
		*/
		virtual CVector& GetSlopes(CVector& vXValues, CVector& vSlopeVector)
		{
			vSlopeVector.Zero();
			CVector vBuffer(vXValues.GetSize());

			int i;
			for(i = 0; i < mOperandsCount; i++)
			{
				mOperands[i]->GetSlopes(vXValues, vBuffer);
				vSlopeVector.Add(vBuffer);
			}
			return vSlopeVector;
		}

		/**
		* Returns the first derivative of the function in regard to a given nonlinear parameter.
		* Since the model is just the sum of all references, the first dervative in regard to a given nonlinear
		* parameter is just the sum of all derivatives of all references that are connected to this nonlinear parameter.
		* 
		* @param fXValue	The data point at which the slope should be determined.
		* @param iParamID	The index within the nonlinear parameter vector of the nonlinear parameter.
		* @param bFixedID	If TRUE the given parameter ID is the parameter ID without all fixed parameter.
		*
		* @return	The slope of the function in regard to the given nonlinear parameter.
		*/
		virtual TFitData GetNonlinearParamSlope(TFitData fXValue, int iParamID, bool bFixedID = true)
		{
			MATHFIT_ASSERT((bFixedID && iParamID >= 0 && iParamID < mNonlinearParams.GetSize()) || (!bFixedID && iParamID >= 0 && iParamID < mNonlinearParams.GetAllSize()));

			TFitData fSum = 0;

			int i;
			for(i = 0; i < mOperandsCount; i++)
			{
				IParamFunction& ipfItem = *mOperands[i];
				CParameterVector& pvItem = ipfItem.GetNonlinearParameterVector();

				int iParamSize = bFixedID ? pvItem.GetSize() : pvItem.GetAllSize();

				// this should onlz happen once, since onlz one operand can contain the desired parameter!
				if(iParamID < iParamSize)
				{
					fSum += ipfItem.GetNonlinearParamSlope(fXValue, iParamID, bFixedID);

					int iSrcParamID = bFixedID ? pvItem.GetFixed2AllIndex(iParamID) : iParamID;

					int j;
					for(j = 0; j < mOperandsCount; j++)
					{
						if(j != i)
						{
							IParamFunction& ipfTarget = *mOperands[i];
							CParameterVector& pvTarget = ipfTarget.GetNonlinearParameterVector();

							int iTargetParamID = pvItem.GetLinkTargetParamID(iSrcParamID, pvTarget);
							while(iTargetParamID >= 0)
							{
								fSum += ipfTarget.GetNonlinearParamSlope(fXValue, iTargetParamID, false);

								iTargetParamID = pvItem.GetLinkTargetParamID(iSrcParamID, pvTarget, iTargetParamID);
							}
						}
					}
					break;
				}
				else
					iParamID -= iParamSize;
			}

			return fSum;
		}

		/**
		* Returns the first derivative of the function in regard to a given nonlinear parameter.
		* The vector is filled using the method to get the slope for one nonlinear parameter at a 
		* given data point.
		* 
		* @param vXValue	The data points at which the slope should be determined.
		* @param vSlopes	The vector object which will receive the slope values.
		* @param iParamID	The index within the nonlinear parameter vector of the nonlinear parameter.
		* @param bFixedID	If TRUE the given parameter ID is the parameter ID without all fixed parameter.
		*/
		virtual void GetNonlinearParamSlopes(CVector& vXValues, CVector& vSlopes, int iParamID, bool bFixedID = true)
		{
			MATHFIT_ASSERT((bFixedID && iParamID >= 0 && iParamID < mNonlinearParams.GetSize()) || (!bFixedID && iParamID >= 0 && iParamID < mNonlinearParams.GetAllSize()));

			vSlopes.Zero();

			CVector vBuffer(vXValues.GetSize());

			int i;
			for(i = 0; i < mOperandsCount; i++)
			{
				IParamFunction& ipfItem = *mOperands[i];
				CParameterVector& pvItem = ipfItem.GetNonlinearParameterVector();

				int iParamSize = bFixedID ? pvItem.GetSize() : pvItem.GetAllSize();

				// this should onlz happen once, since onlz one operand can contain the desired parameter!
				if(iParamID < iParamSize)
				{
					ipfItem.GetNonlinearParamSlopes(vXValues, vSlopes, iParamID, bFixedID);

					int iSrcParamID = bFixedID ? pvItem.GetFixed2AllIndex(iParamID) : iParamID;
					int j;
					for(j = 0; j < mOperandsCount; j++)
					{
						if(j != i)
						{
							IParamFunction& ipfTarget = *mOperands[i];
							CParameterVector& pvTarget = ipfTarget.GetNonlinearParameterVector();

							int iTargetParamID = pvItem.GetLinkTargetParamID(iSrcParamID, pvTarget);
							while(iTargetParamID >= 0)
							{
								ipfTarget.GetNonlinearParamSlopes(vXValues, vBuffer, iTargetParamID, false);
								vSlopes.Add(vBuffer);

								iTargetParamID = pvItem.GetLinkTargetParamID(iSrcParamID, pvTarget, iTargetParamID);
							}
						}
					}
					break;
				}
				else
					iParamID -= iParamSize;
			}
		}

		/**
		* Returns the first derivative of the function in regard to all nonlinear parameters.
		* The matrix is filled using the method to get the slope vector for one nonlinear parameters.
		* 
		* @param iParamID	The index within the nonlinear parameter vector of the nonlinear parameter.
		* @param vXValue	The data points at which the slope should be determined.
		* @param mDyDa		The matrix object receiving the derivative values of the function at the given data points.
		*/
		virtual void GetNonlinearDyDa(CVector& vXValues, CMatrix& mDyDa)
		{
			const int iXSize = vXValues.GetSize();
			const int iParamSize = mNonlinearParams.GetSize();

			int iParamID;
			for(iParamID = 0; iParamID < iParamSize; iParamID++)
				GetNonlinearParamSlopes(vXValues, mDyDa.GetCol(iParamID), iParamID);
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
			MATHFIT_ASSERT((bFixedID && iParamID >= 0 && iParamID < mLinearParams.GetSize()) || (!bFixedID && iParamID >= 0 && iParamID < mLinearParams.GetAllSize()));

			// the basis function if the sum of all coefficients of the reference spectra in regard
			// to the parameter
			// process every reference given
			TFitData fRes = 0;
			int i;
			for(i = 0; i < mOperandsCount; i++)
			{
				IParamFunction& ipfItem = *mOperands[i];
				CParameterVector& pvItem = ipfItem.GetLinearParameterVector();

				int iParamSize = bFixedID ? pvItem.GetSize() : pvItem.GetAllSize();

				if(iParamID < iParamSize)
				{
					fRes = ipfItem.GetLinearBasisFunction(fXValue, iParamID);

					int iSrcParamID = bFixedID ? pvItem.GetFixed2AllIndex(iParamID) : iParamID;

					// search for linked items and add them as well
					int j;
					for(j = 0; j < mOperandsCount; j++)
					{
						if(j != i)
						{
							IParamFunction& ipfTarget = *mOperands[i];
							CParameterVector& pvTarget = ipfTarget.GetLinearParameterVector();

							int iTargetParamID = pvItem.GetLinkTargetParamID(iSrcParamID, pvTarget);
							while(iTargetParamID >= 0)
							{
								fRes += ipfTarget.GetLinearBasisFunction(fXValue, iTargetParamID, false);
								iTargetParamID = pvItem.GetLinkTargetParamID(iSrcParamID, pvTarget, iTargetParamID);
							}
						}
					}
					break;
				}
				else
					iParamID -= iParamSize;
			}
			return fRes;
		}

		/**
		* Returns the basis functions of the specified linear parameter at the given X values.
		*
		* @param vXValues			The vector containing the X values at which to determine the basis functions.
		* @param vBasisFunctions	The vector receiving the basis functions.
		* @param iParamID	The index within the linear parameter vector of the linear parameter.
		* @param bFixedID	If TRUE the given parameter ID is the parameter ID without all fixed parameter.
		*/
		virtual void GetLinearBasisFunctions(CVector& vXValues, CVector& vBasisFunctions, int iParamID, bool bFixedID = true)
		{
			MATHFIT_ASSERT(iParamID >= 0 && iParamID < mLinearParams.GetSize());

			vBasisFunctions.Zero();

			CVector vBasis(vXValues.GetSize());
			vBasis.Zero();

			// the basis function if the sum of all coefficients of the reference spectra in regard
			// to the parameter
			// process every reference given
			int i;
			for(i = 0; i < mOperandsCount; i++)
			{
				IParamFunction& ipfItem = *mOperands[i];
				CParameterVector& pvItem = ipfItem.GetLinearParameterVector();

				int iParamSize = bFixedID ? pvItem.GetSize() : pvItem.GetAllSize();

				if(iParamID < iParamSize)
				{
					ipfItem.GetLinearBasisFunctions(vXValues, vBasisFunctions, iParamID, bFixedID);

					int iSrcParamID = pvItem.GetFixed2AllIndex(iParamID);

					// search for linked items and add them as well
					int j;
					for(j = 0; j < mOperandsCount; j++)
					{
						if(j != i)
						{
							IParamFunction& ipfTarget = *mOperands[i];
							CParameterVector& pvTarget = ipfTarget.GetLinearParameterVector();

							int iTargetParamID = pvItem.GetLinkTargetParamID(iSrcParamID, pvTarget);
							while(iTargetParamID >= 0)
							{
								ipfTarget.GetLinearBasisFunctions(vXValues, vBasis, iTargetParamID, false);
								vBasisFunctions.Add(vBasis);

								iTargetParamID = pvItem.GetLinkTargetParamID(iSrcParamID, pvTarget, iTargetParamID);
							}
						}
					}
					break;
				}
				else
					iParamID -= iParamSize;
			}
		}

		/**
		* Fills the matrix with the basis function of the reference spectrum.
		* The model represens just a sum of all references, therefore we have to fill the matrix columns reference by reference
		*
		* @param vXValues	The vector containing the X values to be contained in the matrix.
		* @param mA		The matrix which will receive the basis functions.
		* @param vB		The vector that hold the contant values of the normal equations.
		*/
		virtual void GetLinearAMatrix(CVector& vXValues, CMatrix& mA, CVector& vB)
		{
			const int iXSize = vXValues.GetSize();

			// create buffer
			CVector vBuffer(iXSize);

			// process every reference given
			int iParamID = 0;
			int i;
			for(i = 0; i < mOperandsCount; i++)
			{
				IParamFunction& ipfItem = *mOperands[i];

				int iLinParamCount = ipfItem.GetLinearParameter().GetSize();
				if(iLinParamCount > 0)
				{
					while(iLinParamCount)
					{
						GetLinearBasisFunctions(vXValues, mA.GetCol(iParamID), iParamID);

						iParamID++;
						iLinParamCount--;
					}
				}
				else
				{
					// if there are no linear parameters, the reference is just a constant offset, that we have to subtract
					ipfItem.GetValues(vXValues, vBuffer);
					vB.Sub(vBuffer);
				}
			}
		}

		/**
		* Sets the new set of linear parameters.
		* The given parameter vector is split up into the pieces needed by the references.
		*
		* @param vParam	The vector containing the new unfixed parameters.
		*
		* @return	TRUE if successful, FALSE otherwise.
		*/
		virtual bool SetLinearParameter(CVector& vParam)
		{
			// keep internal copy
			mLinearParams.SetParameters(vParam);

			int i, iOffset;
			for(iOffset = i = 0; i < mOperandsCount; i++)
			{
				const int iSize = mOperands[i]->GetLinearParameter().GetSize();
				if(iSize > 0)
					mOperands[i]->SetLinearParameter(vParam.SubVector(iOffset, iSize));
				iOffset += iSize;
			}

			return true;
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

			int i, iOffset;
			for(iOffset = i = 0; i < mOperandsCount; i++)
			{
				const int iSize = mOperands[i]->GetNonlinearParameter().GetSize();
				if(iSize > 0)
					mOperands[i]->SetNonlinearParameter(vParam.SubVector(iOffset, iSize));
				iOffset += iSize;
			}

			return true;
		}

		/**
		* Sets the covariance matrix back to the references.
		*
		* @param mCovar	The covariance matrix.
		*
		* @see	IParamFunction::SetNonlinearCovarMatrix
		*/
		virtual void SetNonlinearCovarMatrix(CMatrix& mCovar)
		{
			mNonlinearParams.SetCovarMatrix(mCovar);

			int i, iOffset;
			for(iOffset = i = 0; i < mOperandsCount; i++)
			{
				const int iSize = mOperands[i]->GetNonlinearParameter().GetSize();
				if(iSize > 0)
					mOperands[i]->SetNonlinearCovarMatrix(mCovar.SubMatrix(iOffset, iOffset, iSize, iSize));
				iOffset += iSize;
			}
		}

		/**
		* Sets the given correlation matrix in the references.
		*
		* @param mCorrel	The correlation matrix.
		*
		* @see	IParamFunction::SetNonlinearCorrelMatrix
		*/
		virtual void SetNonlinearCorrelMatrix(CMatrix& mCorrel)
		{
			mNonlinearParams.SetCorrelMatrix(mCorrel);

			int i, iOffset;
			for(iOffset = i = 0; i < mOperandsCount; i++)
			{
				const int iSize = mOperands[i]->GetNonlinearParameter().GetSize();
				if(iSize > 0)
					mOperands[i]->SetNonlinearCorrelMatrix(mCorrel.SubMatrix(iOffset, iOffset, iSize, iSize));
				iOffset += iSize;
			}
		}

		/**
		* Sets the given errors back to the references.
		*
		* @param vError	The error vector.
		*
		* @see IParamFunction::SetNonlinearError
		*/
		virtual void SetNonlinearError(CVector& vError)
		{
			mNonlinearParams.SetError(vError);

			int i, iOffset;
			for(iOffset = i = 0; i < mOperandsCount; i++)
			{
				const int iSize = mOperands[i]->GetNonlinearParameter().GetSize();
				if(iSize > 0)
					mOperands[i]->SetNonlinearError(vError.SubVector(iOffset, iSize));
				iOffset += iSize;
			}
		}

		/**
		* Sets the covariance matrix back to the references.
		*
		* @param mCovar	The covariance matrix.
		*
		* @see	IParamFunction::SetLinearCovarMatrix
		*/
		virtual void SetLinearCovarMatrix(CMatrix& mCovar)
		{
			mLinearParams.SetCovarMatrix(mCovar);

			int i, iOffset;
			for(iOffset = i = 0; i < mOperandsCount; i++)
			{
				const int iSize = mOperands[i]->GetLinearParameter().GetSize();
				if(iSize > 0)
					mOperands[i]->SetLinearCovarMatrix(mCovar.SubMatrix(iOffset, iOffset, iSize, iSize));
				iOffset += iSize;
			}
		}

		/**
		* Sets the given correlation matrix in the references.
		*
		* @param mCorrel	The correlation matrix.
		*
		* @see	IParamFunction::SetLinearCorrelMatrix
		*/
		virtual void SetLinearCorrelMatrix(CMatrix& mCorrel)
		{
			mLinearParams.SetCorrelMatrix(mCorrel);

			int i, iOffset;
			for(iOffset = i = 0; i < mOperandsCount; i++)
			{
				const int iSize = mOperands[i]->GetLinearParameter().GetSize();
				if(iSize > 0)
					mOperands[i]->SetLinearCorrelMatrix(mCorrel.SubMatrix(iOffset, iOffset, iSize, iSize));
				iOffset += iSize;
			}
		}

		/**
		* Sets the given errors back to the references.
		*
		* @param vError	The error vector.
		*
		* @see	IParamFunction::SetLinearError
		*/
		virtual void SetLinearError(CVector& vError)
		{
			mLinearParams.SetError(vError);

			int i, iOffset;
			for(iOffset = i = 0; i < mOperandsCount; i++)
			{
				const int iSize = mOperands[i]->GetLinearParameter().GetSize();
				if(iSize > 0)
					mOperands[i]->SetLinearError(vError.SubVector(iOffset, iSize));
				iOffset += iSize;
			}
		}

		/**
		* Resets the linear parameters to default values.
		*/
		virtual void ResetLinearParameter()
		{
			int i;
			for(i = 0; i < mOperandsCount; i++)
				mOperands[i]->ResetLinearParameter();
			BuildLinearParameter();
		}

		/**
		* Resets the nonlinear parameters to default values.
		*/
		virtual void ResetNonlinearParameter()
		{
			int i;
			for(i = 0; i < mOperandsCount; i++)
				mOperands[i]->ResetNonlinearParameter();
			BuildNonlinearParameter();
		}

		virtual TFitData GetLinearPenalty(TFitData fChiSquare)
		{
			TFitData fSum = 0;
			int i;
			for(i = 0; i < mOperandsCount; i++)
				fSum += mOperands[i]->GetLinearPenalty(fChiSquare);

			return fSum;
		}

		virtual TFitData GetNonlinearPenalty(TFitData fChiSquare)
		{
			TFitData fSum = 0;
			int i;
			for(i = 0; i < mOperandsCount; i++)
				fSum += mOperands[i]->GetNonlinearPenalty(fChiSquare);

			return fSum;
		}

		virtual void SetFitRange(CVector& vFitRange)
		{
			mFitRange.Copy(vFitRange);

			int i;
			for(i = 0; i < mOperandsCount; i++)
				mOperands[i]->SetFitRange(vFitRange);
		}

	private:
		/**
		* Creates the linear parameter vector as sum of all linear parameters of all operands.
		*/
		void BuildLinearParameter()
		{
			CVector vBuf;

			int i;
			for(i = 0; i < mOperandsCount; i++)
				vBuf.Append(mOperands[i]->GetLinearParameter());

			mLinearParams.SetSize(vBuf.GetSize());
			mLinearParams.SetParameters(vBuf);
		}

		/**
		* Creates the nonlinear parameter vector as sum of all nonlinear parameters of all operands.
		*/
		void BuildNonlinearParameter()
		{
			CVector vBuf;

			int i;
			for(i = 0; i < mOperandsCount; i++)
				vBuf.Append(mOperands[i]->GetNonlinearParameter());

			mNonlinearParams.SetSize(vBuf.GetSize());
			mNonlinearParams.SetParameters(vBuf);
		}

		/**
		* Global defines.
		*/
		enum EDefines
		{
			/**
			* Defines the minimum number of elements in the operands array.
			*/
			DEFAULTNOOPERANDS = 16
		};

		/**
		* List of all sumation operands.
		*/
		IParamFunction** mOperands;
		/**
		* Number of operands.
		*/
		int mOperandsCount;
		/**
		* Maximum number of operands.
		*/
		int mMaxOperands;
	};
}

#pragma warning (pop)
#endif // !defined(AFX_DOASFUNCTIONS_H__F778D400_2C41_4092_8BA9_F789F5766579__INCLUDED_)
