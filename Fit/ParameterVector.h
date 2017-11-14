/**
 * Contains a parameter vector object object.
 *
 * @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
 * @version		1.0 @ 2002/02/09
 */
#if !defined(PARAMETERVECTOR_H_20020209)
#define PARAMETERVECTOR_H_20020209

#include "Vector.h"
#include "Matrix.h"
#include "ParameterLinkItem.h"

namespace MathFit
{
	/**
	* Represents a parameter vector.
	* Using this object, its possible to fix and/or link single parameters.
	* The neccessary shrinking and expansion methods are implemented here.
	* Also all other parameter types (error, correlation and covariance) are
	* administered by this object.
	* To achief this functionality two sets of parameter vectors are used. One parameter vector
	* always stores the complete set of parameters regardless wheter they're linked or fixed. This
	* vector is called the internal parameter vector. Any {\bf global} parameter IDs needed are the
	* indicies of the parameter in this internal parameter vector. The second parameter vector is
	* the so called exported parameter vector. This exported vector only consits of the freely
	* modifiable parameters. No fixed or linked parameters are listed in this vector. If you do not
	* need to known whetere parameters are fixed or not, you don't have to take care about the
	* internal parameters.
	*
	* @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
	* @version		1.0 @ 2001/09/21
	*/
	class CParameterVector
	{
	public:
		/**
		* Constructs an empty parameter vector.
		*/
		CParameterVector()
		{
			mSize = mSizeInternal = 0;
			mIndex = NULL;
			mBackIndex = NULL;
			mLink = NULL;
		}

		/**
		* Constructs a parameter vector with a specific size.
		*
		* @param iSize	The number of parameters in the parameter vector.
		*/
		CParameterVector(int iSize)
		{
			SetSize(iSize);
		}

		/**
		* Destructs the object.
		* All allocated resources are released.
		*/
		~CParameterVector()
		{
			if(mIndex)
				delete(mIndex);
			if(mBackIndex)
				delete(mBackIndex);

			CParameterLinkItem* pItem = mLink;
			while(pItem)
			{
				CParameterLinkItem* pNext = pItem->GetNextItem();
				delete(pItem);
				pItem = pNext;
			}
		}

		/**
		* Sets the number of parameter.
		*
		* @param iSize	The number of parameters.
		*
		* @return	TRUE if successful, FALSE otherwise.
		*/
		bool SetSize(int iSize)
		{
			mSize = mSizeInternal = iSize;

			if(mIndex)
				delete(mIndex);
			mIndex = NULL;
			if(mBackIndex)
				delete(mBackIndex);
			mBackIndex = NULL;
			mLink = NULL;

			mParams.SetSize(iSize);
			mParamsInternal.SetSize(iSize);
			mParamsInternalLowLimit.SetSize(iSize);
			mParamsInternalHighLimit.SetSize(iSize);
			mParamsInternalFactorLimit.SetSize(iSize);
			mDefaultParameter.SetSize(iSize);
			mError.SetSize(iSize);
			mErrorInternal.SetSize(iSize);
			mCovar.SetSize(iSize, iSize);
			mCovarInternal.SetSize(iSize, iSize);
			mCorrel.SetSize(iSize, iSize);
			mCorrelInternal.SetSize(iSize, iSize);

			if(iSize > 0 )
			{
				mIndex = new int[iSize];
				mBackIndex = new int[iSize];

				// clear default parameters
				mDefaultParameter.Zero();

				const int iParamSize = mParams.GetSize();
				int i;
				for(i = 0; i < iParamSize; i++)
					mIndex[i] = mBackIndex[i] = i;
			}

			return true;
		}

		/**
		* Fixes the given parameter to a specific value.
		* The parameter given will be set to the specified value and will be removed
		* from the exported parameter list. The original parameter value will not be modified.
		*
		* @param iParamID	The index of the parameter in regard to the whole number of parameters.
		*
		* @return	TRUE if successful, FALSE otherwise.
		*/
		bool FixParameter(int iParamID)
		{
			const int iParamInternalSize = mParamsInternal.GetSize();
			if(iParamID < 0 || iParamID >= iParamInternalSize)
				return false;

			// check wheter the parameter is already fixed (index == -1)
			int iIndex = mIndex[iParamID];
			if(iIndex != -1)
			{
				// no, its not, so prepare index
				mIndex[iParamID] = -1;

				// shift the upper indices downwards
				int i;
				for(i = iParamInternalSize - 1; i >= iParamID + 1; i--)
				{
					// correct neccessary indices
					if(mIndex[i] != -1)
					{
						mIndex[i] -= 1;
						mBackIndex[iIndex] = i;
					}
				}

				// decrease the size of the fit parameter vector
				mParams.SetSize(mParams.GetSize() - 1);
				mSize--;
			}
			// clear the error, covariance and correlation values
			ClearParameterLimits(iParamID);
			mErrorInternal.SetAt(iParamID, 0);
			int i, j;
			for(i = 0; i < mSize; i++)
				for(j = 0; j < mSize; j++)
				{
					mCovarInternal.SetAt(i, j, 0);
					mCovarInternal.SetAt(j, i, 0);
					mCorrelInternal.SetAt(i, j, 0);
					mCorrelInternal.SetAt(j, i, 0);
				}

				return true;
		}

		/**
		* Fixes the given parameter to a specific value.
		* The parameter given will be set to the specified value and will be removed
		* from the exported parameter list.
		*
		* @param iParamID	The index of the parameter in regard to the whole number of parameters.
		* @param fValue	The value to what the parameter should be set.
		*
		* @return	TRUE if successful, FALSE otherwise.
		*/
		bool FixParameter(int iParamID, TFitData fValue)
		{
			// fix the parameter
			if(!FixParameter(iParamID))
				return false;

			// set the parameter to the desired value
			mParamsInternal.SetAt(iParamID, fValue);

			// update all linked object's parameters
			CParameterLinkItem* pItem = mLink;
			while(pItem)
			{
				// we only need to update the internal parameter list, since all linked parameters should be set to fixed mode.
				if(pItem->GetSrcID() == iParamID)
					pItem->GetTargetObj()->GetAllParameter().SetAt(pItem->GetTargetID(), mParamsInternal.GetAt(pItem->GetSrcID()));
				pItem = pItem->GetNextItem();
			}

			return true;
		}

		/**
		* Releases a fixed parameter.
		* A fixed parameter will be added to the exported parameter vector
		* and can therefore be modified again.
		*
		* @param iParamID	The index of the parameter in regard to the whole number of parameters.
		*
		* @return	TRUE if successful, FALSE otherwise.
		*/
		bool ReleaseParameter(int iParamID)
		{
			const int iParamInternalSize = mParamsInternal.GetSize();
			if(iParamID < 0 || iParamID >= iParamInternalSize)
				return false;

			if(mIndex[iParamID] != -1)
				return true;

			// search for next available index
			int iIndex = mParams.GetSize();
			int i;
			for(i = iParamInternalSize - 1; i >= iParamID + 1; i--)
				if(mIndex[i] != -1)
					iIndex = mIndex[i]++;

			// set the new index
			mIndex[iParamID] = iIndex;
			mBackIndex[iIndex] = iParamID;

			mSize++;

			// increase fit parameter vector
			mParams.SetSize(mParams.GetSize() + 1);

			return true;
		}

		/**
		* Check wheter a parameter if fixed or not.
		*
		* @param iParamID	The index of the parameter in regard to the whole number of parameters.
		*
		* @return	TRUE if the parameter if fixed, FALSE otherwise.
		*/
		bool IsParamFixed(int iParamID)
		{
			MATHFIT_ASSERT(iParamID >= 0 && iParamID < mParamsInternal.GetSize());

			return mIndex[iParamID] == -1;
		}

		/**
		* Converts the parameter index from the exported parameter ID to the real parameter ID.
		* Normally the exported and real parameter IDs are the same. But if a parameter is fixed
		* it's removed from the exported parameter vector. Therefore all parameter IDs that are
		* above the fixed parameter ID will be decreased by one, since the whole size of the 
		* exportet vector also is decreased.
		*
		* @param iParamID	The parameter index in the exported parameter vector.
		*
		* @return	The parameter index in the whole parameter vector.
		*/
		int GetFixed2AllIndex(int iParamID)
		{
			MATHFIT_ASSERT(iParamID >= 0 && iParamID < mParams.GetSize());

			return mBackIndex[iParamID];
		}

		/**
		* Converts the real parameter ID to the exported parameter ID.
		* Details about the dependence of exported and real parameter ID see at
		* \Ref{GetFixed2AllIndex}.
		*
		* @param iParamID	The parameter index in the whole parameter vector.
		*
		* @return	The parameter index in the exported parameter vector.
		*/
		int GetAll2FixedIndex(int iParamID)
		{
			MATHFIT_ASSERT(iParamID >= 0 && iParamID < mParamsInternal.GetSize());

			return mIndex[iParamID];
		}

		/**
		* Returns the internal parameter vector.
		*
		* @return	A reference to the internal parameter vector.
		*/
		CVector& GetAllParameter()
		{
			return mParamsInternal;
		}

		/**
		* Sets the internal parameter vector.
		*
		* @param vParams	The new internal parameter vector.
		*
		* @return	TRUE if successful, FALSE otherwise.
		*/
		bool SetAllParameter(CVector& vParams)
		{
			mParamsInternal.Copy(vParams);

			return true;
		}

		CVector& GetAllDefaultParameter()
		{
			return mDefaultParameter;
		}

		bool SetAllDefaultParameters(CVector& vDefData)
		{
			mDefaultParameter.Copy(vDefData);

			// reset all parameter lists and update the parameter data
			Reset();

			return true;
		}

		bool SetAllDefaultParameter(int iParamID, TFitData fDefValue)
		{
			if(IsParamFixed(iParamID))
				return false;

			mDefaultParameter.SetAt(iParamID, fDefValue);

			// reset all parameter lists and update the parameter data
			Reset();

			return true;
		}

		/**
		* Returns the internal error vector.
		*
		* @return	A reference to the internal error vector.
		*/
		CVector& GetAllError()
		{
			return mErrorInternal;
		}

		/**
		* Sets the internal error vector.
		*
		* @param vError	The new internal error vector.
		*
		* @return	TRUE if successful, FALSE otherwise.
		*/
		bool SetAllError(CVector& vError)
		{
			mErrorInternal.Copy(vError);

			return true;
		}

		/**
		* Returns the internal covariance matrix.
		*
		* @return	A reference to the internal covariance matrix.
		*/
		CMatrix& GetAllCovarMatrix()
		{
			return mCovarInternal;
		}

		/**
		* Sets the internal covariance matrix.
		*
		* @param mCovarMatrix	The new internal covariance matrix.
		*
		* @return	TRUE if successful, FALSE otherwise.
		*/
		bool SetAllCovarMatrix(CMatrix& mCovarMatrix)
		{
			mCovarInternal.Copy(mCovarMatrix);

			return true;
		}

		/**
		* Returns the internal correlation matrix.
		*
		* @return	A reference to the internal correlation matrix.
		*/
		CMatrix& GetAllCorrelMatrix()
		{
			return mCorrelInternal;
		}

		/**
		* Sets the internal correlation matrix.
		*
		* @param mCorrelMatrix	The new internal correlation matrix.
		*
		* @return	TRUE if successful, FALSE otherwise.
		*/
		bool SetAllCorrelMatrix(CMatrix& mCorrelMatrix)
		{
			mCorrelInternal.Copy(mCorrelMatrix);

			return true;
		}

		/**
		* Sets a single parameter value.
		* 
		* @param iParamID	The index of the parameter given as global parameter ID. (See \Ref{FixParameter})
		* @param fValue	The new value of the parameter.
		*
		* @return	TRUE if successful, FALSE otherwise.
		*/
		bool SetParameter(int iParamID, TFitData fValue)
		{
			if(IsParamFixed(iParamID))
				return false;

			mParams.SetAt(GetAll2FixedIndex(iParamID), fValue);
			mParamsInternal.SetAt(iParamID, fValue);

			// update all linked object's parameters
			CParameterLinkItem* pItem = mLink;
			while(pItem)
			{
				// we only need to update the internal parameter list, since all linked parameters should be set to fixed mode.
				if(pItem->GetSrcID() == iParamID)
					pItem->GetTargetObj()->GetAllParameter().SetAt(pItem->GetTargetID(), mParamsInternal.GetAt(pItem->GetSrcID()));
				pItem = pItem->GetNextItem();
			}

			return true;
		}

		/**
		* Sets the exported parameter vector.
		*
		* @param vParams	The new exported parameter vector.
		*
		* @return	TRUE if successful, FALSE otherwise.
		*/
		bool SetParameters(CVector& vParams)
		{
			// get local copy
			mParams.Copy(vParams);

			// process fixed parameters
			const int iParamSize = mParams.GetSize();
			if(mParamsInternal.GetSize() != iParamSize)
			{
				// copy all unfixed parameters in the internal list
				int i;
				for(i = 0; i < iParamSize; i++)
					mParamsInternal.SetAt(mBackIndex[i], mParams.GetAt(i));
			}
			else
				mParamsInternal.Copy(mParams);

			// update all linked object's parameters
			CParameterLinkItem* pItem = mLink;
			while(pItem)
			{
				// we only need to update the internal parameter list, since all linked parameters should be set to fixed mode.
				pItem->GetTargetObj()->GetAllParameter().SetAt(pItem->GetTargetID(), mParamsInternal.GetAt(pItem->GetSrcID()));
				pItem = pItem->GetNextItem();
			}

			return true;
		}

		/**
		* Returns the exported parameter vector.
		* This vector does not contain any fixed parameters.
		*
		* @return	A reference to the exported parameter vector.
		*/
		CVector& GetParameter()
		{
			const int iParamSize = mParams.GetSize();
			if(mParamsInternal.GetSize() != iParamSize)
			{
				int i;
				for(i = 0; i < iParamSize; i++)
					mParams.SetAt(i, mParamsInternal.GetAt(mBackIndex[i]));
			}
			else
				mParams.Copy(mParamsInternal);

			return mParams;
		}

		/**
		* Sets a single error value.
		* 
		* @param iParamID	The index of the parameter given as global parameter ID. (See \Ref{FixParameter})
		* @param fValue	The new error value.
		*
		* @return	TRUE if successful, FALSE otherwise.
		*/
		bool SetError(int iParamID, TFitData fValue)
		{
			if(IsParamFixed(iParamID))
				return false;

			mError.SetAt(GetAll2FixedIndex(iParamID), fValue);
			mErrorInternal.SetAt(iParamID, fValue);

			// update all linked object's parameters
			CParameterLinkItem* pItem = mLink;
			while(pItem)
			{
				if(pItem->GetSrcID() == iParamID)
					pItem->GetTargetObj()->GetAllError().SetAt(pItem->GetTargetID(), mErrorInternal.GetAt(pItem->GetSrcID()));
				pItem = pItem->GetNextItem();
			}

			return true;
		}

		/**
		* Sets the error vector of the exported parameters.
		*
		* @param vError	The new error vector.
		*
		* @return	TRUE if successful, FALSE otherwise.
		*/
		bool SetError(CVector& vError)
		{
			// get local copy
			mError.Copy(vError);

			// process fixed parameters
			const int iErrorSize = mError.GetSize();
			if(mErrorInternal.GetSize() != iErrorSize)
			{
				// copy all unfixed parameters in the internal list
				int i;
				for(i = 0; i < iErrorSize; i++)
					mErrorInternal.SetAt(mBackIndex[i], mError.GetAt(i));
			}
			else
				mErrorInternal.Copy(mError);

			// update all linked object's parameters
			CParameterLinkItem* pItem = mLink;
			while(pItem)
			{
				pItem->GetTargetObj()->GetAllError().SetAt(pItem->GetTargetID(), mErrorInternal.GetAt(pItem->GetSrcID()));
				pItem = pItem->GetNextItem();
			}

			return true;
		}

		/**
		* Returns the error vector of the exported parameters.
		*
		* @return	A reference to the error vector of the exported parameters.
		*/
		CVector& GetError()
		{
			const int iErrorSize = mError.GetSize();
			if(mErrorInternal.GetSize() != iErrorSize)
			{
				int i;
				for(i = 0; i < iErrorSize; i++)
					mError.SetAt(i, mErrorInternal.GetAt(mBackIndex[i]));
			}
			else
				mError.Copy(mErrorInternal);

			return mParams;
		}

		/**
		* Sets the correlation matrix of the exported parameters.
		*
		* @param mCorrelMatrix	The new correlation matrix.
		*
		* @return	TRUE if successful, FALSE otherwise.
		*/
		bool SetCorrelMatrix(CMatrix& mCorrelMatrix)
		{
			// get local copy
			mCorrel.Copy(mCorrelMatrix);

			// process fixed parameters
			if(mCorrelInternal.GetNoRows() != mCorrel.GetNoRows())
			{
				// copy all unfixed parameters in the internal list
				int i;
				for(i = 0; i < mCorrel.GetNoRows(); i++)
				{
					int j;
					for(j = 0; j < mCorrel.GetNoColumns(); j++)
					{
						mCorrelInternal.SetAt(mBackIndex[i], mBackIndex[j], mCorrel.GetAt(i, j));
						mCorrelInternal.SetAt(mBackIndex[j], mBackIndex[i], mCorrel.GetAt(j, i));
					}
				}
			}
			else
				mCorrelInternal.Copy(mCorrel);

			// an update of the linked parameters correlation is not useful, since the real
			// correlation of the linked object's parameters is unknown!

			return true;
		}

		/**
		* Returns the correlation matrix of the exported parameters.
		*
		* @return	A reference to the correlation matrix of the exported parameters.
		*/
		CMatrix& GetCorrelMatrix()
		{
			if(mCorrelInternal.GetNoRows() != mCorrel.GetNoRows())
			{
				int i;
				for(i = 0; i < mCorrel.GetNoRows(); i++)
				{
					int j;
					for(j = 0; i < mCorrel.GetNoColumns(); j++)
					{
						mCorrel.SetAt(i, j, mCorrelInternal.GetAt(mBackIndex[i], mBackIndex[j]));
						mCorrel.SetAt(j, i, mCorrelInternal.GetAt(mBackIndex[j], mBackIndex[i]));
					}
				}
			}
			else
				mCorrel.Copy(mCorrelInternal);

			return mCorrel;
		}

		/**
		* Sets the new covariance matrix of the exported parameters.
		*
		* @param mCovarMatrix	The new covariance matrix.
		*
		* @return	TRUE if successful, FALSE otherwise.
		*/
		bool SetCovarMatrix(CMatrix& mCovarMatrix)
		{
			// get local copy
			mCovar.Copy(mCovarMatrix);

			// process fixed parameters
			if(mCovarInternal.GetNoRows() != mCovar.GetNoRows())
			{
				// copy all unfixed parameters in the internal list
				int i;
				for(i = 0; i < mCovar.GetNoRows(); i++)
				{
					int j;
					for(j = 0; j < mCovar.GetNoColumns(); j++)
					{
						mCovarInternal.SetAt(mBackIndex[i], mBackIndex[j], mCovar.GetAt(i, j));
						mCovarInternal.SetAt(mBackIndex[j], mBackIndex[i], mCovar.GetAt(j, i));
					}
				}
			}
			else
				mCovarInternal.Copy(mCovar);

			// an update of the linked parameters correlation is not useful, since the real
			// correlation of the linked object's parameters is unknown!

			return true;
		}

		/**
		* Returns the covariance matrix of the exported parameters.
		*
		* @return	A reference to the covariance matrix of the exported parameters.
		*/
		CMatrix& GetCovarMatrix()
		{
			if(mCovarInternal.GetNoRows() != mCovar.GetNoRows())
			{
				int i;
				for(i = 0; i < mCovar.GetNoRows(); i++)
				{
					int j;
					for(j = 0; i < mCovar.GetNoColumns(); j++)
					{
						mCovar.SetAt(i, j, mCovarInternal.GetAt(mBackIndex[i], mBackIndex[j]));
						mCovar.SetAt(j, i, mCovarInternal.GetAt(mBackIndex[j], mBackIndex[i]));
					}
				}
			}
			else
				mCovar.Copy(mCovarInternal);

			return mCovar;
		}

		/**
		* Returns the number of exported parameters.
		*
		* @return	The number of exported parametes.
		*/
		int GetSize()
		{
			return mSize;
		}

		/**
		* Returns the number of all parameters including fixed parameters.
		*
		* @return	The number of all parameters.
		*/
		int GetAllSize()
		{
			return mSizeInternal;
		}

		/**
		* Links two specified parameters together.
		* A link between the given source and target parameter will be set, so any modification of the
		* source parameter will be forwarded to the target parameter. The link needs to be established 
		* from the source's parameter vector object, so the source's parameter ID must be a valid global
		* ID within the current object.
		*
		* @param iSrcID	The global parameter ID of the link source's parameter within the current object. (See \Ref{FixParameter})
		* @param pvTarget	The parameter vector object belonging to the link target.
		* @param iTargetID	The global parameter ID of the link target's parameter within the \Ref{pvTarget} parameter object. (See \Ref{FixParameter})
		*
		* @return	TRUE if successful, FALSE otherwise.
		*/
		bool LinkParameter(int iSrcID, CParameterVector& pvTarget, int iTargetID)
		{
			// validate parameters
			if(iSrcID < 0 || iSrcID >= mParamsInternal.GetSize())
				return false;

			if(iTargetID < 0 || iTargetID >= pvTarget.GetAllSize())
				return false;

			// we can only link unfixed parameters
			if(pvTarget.IsParamFixed(iTargetID))
				return false;

			// create chain item
			CParameterLinkItem* pItem = new CParameterLinkItem(iSrcID, &pvTarget, iTargetID, NULL, mLink);
			if(mLink)
				mLink->SetPreviousItem(pItem);
			mLink = pItem;

			// set the parameter to fixed state
			pvTarget.FixParameter(iTargetID, mParamsInternal.GetAt(iSrcID));

			return true;
		}

		/**
		* Releases the link between the two defined parameters.
		*
		* @param iSrcID	The global parameter ID of the link source's parameter within the current object. (See \Ref{FixParameter})
		* @param pvTarget	The parameter vector object belonging to the link target.
		* @param iTargetID	The global parameter ID of the link target's parameter within the \Ref{pvTarget} parameter object. (See \Ref{FixParameter})
		*
		* @return	TRUE if successful, FALSE otherwise.
		*/
		bool UnlinkParameter(int iSrcID, CParameterVector& pvTarget, int iTargetID)
		{
			if(iSrcID < 0 || iSrcID >= mParamsInternal.GetSize())
				return false;

			if(iTargetID < 0 || iTargetID >= pvTarget.GetAllSize())
				return false;

			CParameterLinkItem* pRem = new CParameterLinkItem(iSrcID, &pvTarget, iTargetID);
			CParameterLinkItem* pItem = mLink;

			// go through the whole list
			while(pItem)
			{
				if(pRem->IsEqual(pItem))
				{
					CParameterLinkItem* pPrev = pItem->GetPreviousItem();

					if(pPrev)
						pPrev->SetNextItem(pItem->GetNextItem());
					else
						mLink = pItem->GetNextItem();
					pItem->GetTargetObj()->ReleaseParameter(pItem->GetTargetID());
					delete(pItem);
					break;
				}
				pItem = pItem->GetNextItem();
			}
			delete(pRem);
			return pItem ? true : false;
		}

		/**
		* Resets the parameters to zero.
		*/
		void Reset()
		{
			// only reset the non fixed, non linked parameters
			int i;
			for(i = 0; i < GetSize(); i++)
				SetParameter(i, mDefaultParameter.GetAt(GetFixed2AllIndex(i)));
		}

		/**
		* Returns the global parameter ID of a linked parameter.
		* This method can be used to check wheter a given parameter is linked to the specified
		* source parameter. Succesive calls to this function with the last valid target parameter
		* ID as last parameter can be used to find all linked parameter of the given source
		* parameter.
		*
		\begin{verbatim}
		Example:
		while((iLastID = GetLinkTargetParamID(0, pvRef, iLastID) >= 0)
		pvRef.SetParam(iLastID, 0);
		\end{verbatim}
		*
		* This example sets all parameters of the parameter vector {\bf pvRef} to zero, if they're linked
		* to parameter {\bf 0} of the current object.
		*
		* @param iSrcID		The global parameter ID of the link source. (See \Ref{FixParameter})
		* @param pvTarget		The parameter vector object of the link targetto be checked for linked parameters.
		* @param iLastTargetID	The parameter ID of the last link target found. If set to {\bf -1} the first parameter ID found is returned.
		*
		* @return	The next parameter ID found in \Ref{pvTarget} that is linked to the given source parameter after \Ref{iLastTargetID}. If no link target found {\bf -1} will be returned.
		*/
		int GetLinkTargetParamID(int iSrcID, CParameterVector& pvTarget, int iLastTargetID = -1)
		{
			bool bFoundLast = false;

			if(iLastTargetID < 0)
				bFoundLast = true;

			CParameterLinkItem* pNext = mLink;
			while(pNext)
			{
				if(pNext->IsLinked(iSrcID, &pvTarget))
				{
					if(bFoundLast)
						return pNext->GetTargetID();
					else
					{
						if(iLastTargetID == pNext->GetTargetID())
							bFoundLast = true;
					}
				}
				pNext = pNext->GetNextItem();
			}
			return -1;
		}

		TFitData GetPenalty(TFitData fChiSquare)
		{
			TFitData fSum = 0;
			int i;
			for(i = 0; i < mParamsInternal.GetSize(); i++)
			{
				TFitData fLow = mParamsInternalLowLimit.GetAt(i);
				TFitData fHigh = mParamsInternalHighLimit.GetAt(i);

				if(fLow != fHigh)
				{
					const TFitData fParam = mParamsInternal.GetAt(i);
					const TFitData fFactor = mParamsInternalFactorLimit.GetAt(i);

					if(fParam < fLow )
					{
						// if we have selected hard border, return FINITE
						if(fFactor == 0)
							return (TFitData)HUGE_VAL;

						double fError = fabs(fLow - fParam) * fFactor;
						fSum += (TFitData)(fChiSquare * exp(fError));
					}
					else if(fParam > fHigh)
					{
						// if we have selected hard border, return FINITE
						if(fFactor == 0)
							return (TFitData)HUGE_VAL;

						double fError = fabs(fParam - fHigh) * fFactor;
						fSum += (TFitData)(fChiSquare * exp(fError));
					}
				}
			}

			return fSum;
		}

		bool SetParameterLimits(int iParamID, TFitData fLowLimit, TFitData fHighLimit, TFitData fPenaltyFactor)
		{
			MATHFIT_ASSERT(iParamID >= 0 && iParamID < mParamsInternal.GetSize());

			if(IsParamFixed(iParamID))
				return false;

			mParamsInternalLowLimit.SetAt(iParamID, fLowLimit);
			mParamsInternalHighLimit.SetAt(iParamID, fHighLimit);
			mParamsInternalFactorLimit.SetAt(iParamID, fPenaltyFactor);

			return true;
		}

		bool ClearParameterLimits(int iParamID)
		{
			MATHFIT_ASSERT(iParamID >= 0 && iParamID < mParamsInternal.GetSize());

			mParamsInternalLowLimit.SetAt(iParamID, 0);
			mParamsInternalHighLimit.SetAt(iParamID, 0);
			mParamsInternalFactorLimit.SetAt(iParamID, 1);

			return true;
		}

	private:
		/**
		* Holds the exported parameters.
		*/
		CVector mParams;
		/**
		* Holds the internal parameters.
		*/
		CVector mParamsInternal;
		CVector mParamsInternalLowLimit;
		CVector mParamsInternalHighLimit;
		CVector mParamsInternalFactorLimit;
		/**
		 * The default parameters.
		 */
		CVector mDefaultParameter;
		/**
		* Holds the exported error values.
		*/
		CVector mError;
		/**
		* Holds the internal error values.
		*/
		CVector mErrorInternal;
		/**
		* Holds the exported covariance matrix.
		*/
		CMatrix mCovar;
		/**
		* Holds the internal covariance matrix.
		*/
		CMatrix mCovarInternal;
		/**
		* Holds the exported correlation matrix.
		*/
		CMatrix mCorrel;
		/**
		* Holds the internal correlation matrix.
		*/
		CMatrix mCorrelInternal;
		/**
		* Holds a list to map from global parameter IDs to exported parameter IDs.
		*/
		int* mIndex;
		/**
		* Holds a list to map from exported parameter IDs to global parameter IDs.
		*/
		int* mBackIndex;
		/**
		* Holds the number of exported parameters.
		*/
		int mSize;
		/**
		* Holds the number of all parameters used.
		*/
		int mSizeInternal;
		/**
		* Holds a list of linked parameter defintions used for forward updateing of linkes parameters.
		*/
		CParameterLinkItem* mLink;
	};
}
#endif