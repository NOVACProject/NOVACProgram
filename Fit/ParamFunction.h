/**
 * ParamFunction.h
 *
 * Contains the parametric function interface and its default implementation.
 *
 * @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
 * @version		1.0 @ 2001/09/09
 */
#if !defined(PARAMFUNCTION_H_011206)
#define PARAMFUNCTION_H_011206

#include "Vector.h"
#include "Matrix.h"
#include "Function.h"
#include "ParameterVector.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning (push, 3)

namespace MathFit
{
	/**
	* Exception class to indicate a wrong parameter ID used to access the nonlinear parameters.
	* Use with the EXCEPTION macro.
	*
	* @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
	* @version		1.0 @ 2001/09/09
	*/
	class CWrongParamIDException : public CFitException
	{
	public:
		CWrongParamIDException(int iLine, const char* szModule, const char* szName) : CFitException(iLine, szModule, szName, "Wrong parameter ID!") {}
	};

	/**
	* Exception class to indicate a not implemented function body.
	* Use with the EXCEPTION macro.
	*
	* @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
	* @version		1.0 @ 2001/09/09
	*/
	class CNotImplementedException : public CFitException
	{
	public:
		CNotImplementedException(int iLine, const char* szModule, const char* szName) : CFitException(iLine, szModule, szName, "Not implemented!") {}
	};

	/**
	* Interface to represent a parametric function. 
	* The object can contain both linear and nonlinear parameters. The interface is derivated from IFunction
	* so all the function handling methods are available two
	*
	* @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
	* @version		1.0 @ 2001/09/09
	*/
	class IParamFunction : public IFunction
	{
	public:
		/**
		* Creates an empty object
		*/
		IParamFunction()
		{
			mNearlyZero = MATHFIT_NEARLYZERO;
			mDelta = MATHFIT_DELTA;
			mStopAutoTune = false;
		}

		/**
		* Constructs the object and stores the given data into the internal buffers.
		*
		* @param vXValues		A vector object containing the X values of the data set.
		* @param vYValues		A vector object containing the Y values of the data set.
		*
		* @see	IFunction constructor
		*/
		IParamFunction(CVector& vXValues, CVector& vYValues) : IFunction(vXValues, vYValues)
		{
			mNearlyZero = MATHFIT_NEARLYZERO;
			mDelta = MATHFIT_DELTA;
			mStopAutoTune = false;
		}

		/**
		* Constructs the object and stores the given data into the internal buffers.
		*
		* @param vXValues		A vector object containing the X values of the data set.
		* @param vYValues		A vector object containing the Y values of the data set.
		* @param vError			A vector object containing the <B>sigma</B> error values of the data set.
		*
		* @see	IFunction constructor
		*/
		IParamFunction(CVector& vXValues, CVector& vYValues, CVector& vError) : IFunction(vXValues, vYValues, vError)
		{
			mNearlyZero = MATHFIT_NEARLYZERO;
			mDelta = MATHFIT_DELTA;
			mStopAutoTune = false;
		}

		/**
		* Returns the vector object containing the linear parameters of the function object.
		* This vector must contain the unfixed model parameters only! The number of unfixed parameters maybe
		* determined by the length of this vector.
		*
		* @return Vector object containing the linear parameters.
		*/
		virtual CVector& GetLinearParameter()
		{
			return mLinearParams.GetParameter();
		}

		/**
		* Sets the linear parameters of the function object.
		*
		* @param vLinParam	A vector object containing the new unfixed linear parameters.
		*
		* @return TRUE if successful, FALSE if the vector sizes do not match.
		*/
		virtual bool SetLinearParameter(CVector& vLinParam)
		{
			MATHFIT_ASSERT(vLinParam.GetSize() == mLinearParams.GetSize());

			mLinearParams.SetParameters(vLinParam);
			return true;
		}

		/**
		* Makes an internal backup of the unfixed linear parameters.
		*/
		virtual void BackupLinearParameter()
		{
			mBackupLinearParams.Copy(mLinearParams.GetParameter());
		}

		/**
		* Restores the unfixed linear parameters back to ones stored in the internal backup.
		*/
		virtual void RestoreLinearParameter()
		{
			mLinearParams.SetParameters(mBackupLinearParams);
		}

		/**
		* Returns the vector object containing the nonlinear parameters of the function object.
		* This vector must contain the unfixed model parameters only! The number of unfixed parameters maybe
		* determined by the length of this vector.
		*
		* @return Vector object containing the nonlinear parameters.
		*/
		virtual CVector& GetNonlinearParameter()
		{
			return mNonlinearParams.GetParameter();
		}

		/**
		* Sets the nonlinear parameters of the function object.
		*
		* @param vNonlinParam	A vector object containing the new unfixed nonlinear parameters.
		*
		* @return TRUE if successful, FALSE if the vector sizes do not match.
		*/
		virtual bool SetNonlinearParameter(CVector& vNonlinParam)
		{
			MATHFIT_ASSERT(vNonlinParam.GetSize() == mNonlinearParams.GetSize());

			mNonlinearParams.SetParameters(vNonlinParam);
			return true;
		}

		/**
		* Makes an internal backup of the unfixed nonlinear parameters.
		*/
		virtual void BackupNonlinearParameter()
		{
			mBackupNonlinearParams.Copy(mNonlinearParams.GetParameter());
		}

		/**
		* Restores the unfixed nonlinear parameters back to ones stored in the internal backup.
		*/
		virtual void RestoreNonlinearParameter()
		{
			mNonlinearParams.SetParameters(mBackupNonlinearParams);
		}

		/**
		* Returns the first derivative of the function in regard to a given nonlinear parameter. 
		* The default implementation uses the difference quotient method to calculate the first derivative.
		* Any parameter value less than fNearlyZero is set to fNearlyZero. This can be used to avoid
		* zero parameters which would result in a zero slope.
		* fDelta defines the width of the function sample used to calculate the slope.
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

			TFitData fOrig = GetValue(fXValue);

			// UPD010920 Stefan:
			// With this loop we autotune the nearly zero parameter. This is neccessary, since
			// we cannot accept a complete zero derivative of one parameters. This would cause a
			// unsolvable matrix in the main loop. Therefore we tune this value on the first call
			// to get at least some slopes.
			mDelta = MATHFIT_DELTA;
			mNearlyZero = MATHFIT_NEARLYZERO;
			TFitData fDiff, fNew;
			bool bDone = false;
			CVector& vNonlinearParams = bFixedID ? mNonlinearParams.GetParameter() : mNonlinearParams.GetAllParameter();
			do
			{
				TFitData fTemp = vNonlinearParams.GetAt(iParamID);

				if(fabs(vNonlinearParams.GetAt(iParamID)) < mNearlyZero)
					vNonlinearParams.SetAt(iParamID, mNearlyZero);
				fDiff = vNonlinearParams.GetAt(iParamID) * mDelta;
				vNonlinearParams.SetAt(iParamID, vNonlinearParams.GetAt(iParamID) + fDiff);

				// we have to call the set parameter function in order to achive correct parameter mapping
				// that my depend on the current function
				SetNonlinearParameter(vNonlinearParams);
				fNew = GetValue(fXValue) - fOrig;

				vNonlinearParams.SetAt(iParamID, fTemp);
				SetNonlinearParameter(vNonlinearParams);

#if defined(MATHFIT_AUTOTUNE)
				// if we only have zeros in the vector, tune our border values
				if(fNew != 0)
					bDone = true;
				else
				{
					mNearlyZero /= mDelta;
					if(mNearlyZero >= 1)
						bDone = true;
				}
#else
				bDone = true;
#endif
			}while(!bDone && !mStopAutoTune);

			return fNew / fDiff;
		}

		/**
		* Returns the first derivative of the function in regard to a given nonlinear parameter. 
		* The default implementation uses the difference quotient method to calculate the first derivative.
		* Any parameter value less than fNearlyZero is set to fNearlyZero. This can be used to avoid
		* zero parameters which would result in a zero slope.
		* fDelta defines the width of the function sample used to calculate the slope.
		* This method calculates a whole set of derivation samples.
		* 
		* @param vXValue	The data points at which the slope should be determined.
		* @param vSlopes	The vector object which will receive the slope values.
		* @param iParamID	The index within the nonlinear parameter vector of the nonlinear parameter.
		* @param bFixedID	If TRUE the given parameter ID is the parameter ID without all fixed parameter.
		*/
		virtual void GetNonlinearParamSlopes(CVector& vXValues, CVector& vSlopes, int iParamID, bool bFixedID = true)
		{
			MATHFIT_ASSERT((bFixedID && iParamID >= 0 && iParamID < mNonlinearParams.GetSize()) || (!bFixedID && iParamID >= 0 && iParamID < mNonlinearParams.GetAllSize()));

			// get original function values
			CVector vOrig(vXValues.GetSize());
			GetValues(vXValues, vOrig);

			// UPD010920 Stefan:
			// With this loop we autotune the nearly zero parameter. This is neccessary, since
			// we cannot accept a complete zero derivative of one parameters. This would cause a
			// unsolvable matrix in the main loop. Therefore we tune this value on the first call
			// to get at least some slopes.
			TFitData fDiff;
			mDelta = MATHFIT_DELTA;
			mNearlyZero = MATHFIT_NEARLYZERO;
			bool bDone = false;
			CVector& vNonlinearParams = bFixedID ? mNonlinearParams.GetParameter() : mNonlinearParams.GetAllParameter();
			do
			{
				// backup original parameter
				TFitData fTemp = vNonlinearParams.GetAt(iParamID);

				// modify the parameter
				if(fabs(vNonlinearParams.GetAt(iParamID)) < mNearlyZero)
					vNonlinearParams.SetAt(iParamID, mNearlyZero);
				fDiff = vNonlinearParams.GetAt(iParamID) * mDelta;
				vNonlinearParams.SetAt(iParamID, vNonlinearParams.GetAt(iParamID) + fDiff);

				// get new values
				SetNonlinearParameter(vNonlinearParams);
				GetValues(vXValues, vSlopes);

				// restore parameter
				vNonlinearParams.SetAt(iParamID, fTemp);
				SetNonlinearParameter(vNonlinearParams);

				// calculate the numerical difference 
				vSlopes.Sub(vOrig);

#if defined(MATHFIT_AUTOTUNE)
				// if we only have zeros in the vector, tune our border values
				if(vSlopes.IsZero())
				{
					mNearlyZero /= mDelta;
					if(mNearlyZero >= 1)
						bDone = true;
				}
				else
					bDone = true;
#else
				bDone = true;
#endif
			}while(!bDone && !mStopAutoTune);

			vSlopes.Div(fDiff);
		}

		/**
		* Returns the first derivative of the function in regard to all nonlinear parameters. 
		* The default implementation uses the difference quotient method to calculate the first derivative.
		* Any parameter value less than fNearlyZero is set to fNearlyZero. This can be used to avoid
		* zero parameters which would result in a zero slope.
		* fDelta defines the width of the function sample used to calculate the slope.
		* 
		* @param iParamID	The index within the nonlinear parameter vector of the nonlinear parameter.
		* @param vXValue	The data points at which the slope should be determined.
		* @param mDyDa		The matrix object receiving the derivative values of the function at the given data points.
		*/
		virtual void GetNonlinearDyDa(CVector& vXValues, CMatrix& mDyDa)
		{
			// get original function values
			CVector vOrig(vXValues.GetSize());
			GetValues(vXValues, vOrig);

			CVector& vNonlinearParams = mNonlinearParams.GetParameter();

			const int iParamSize = mNonlinearParams.GetSize();
			const int iXSize = vXValues.GetSize();

			int iAutoTuneCount = 0;
			int iParamID;
			for(iParamID = 0; iParamID < iParamSize; iParamID++)
			{
				CVector& vParamColumn = mDyDa.GetCol(iParamID);

				// UPD010920 Stefan:
				// With this loop we autotune the nearly zero parameter. This is neccessary, since
				// we cannot accept a complete zero derivative of one parameters. This would cause a
				// unsolvable matrix in the main loop. Therefore we tune this value on the first call
				// to get at least some slopes.
				mDelta = MATHFIT_DELTA;
				mNearlyZero = MATHFIT_NEARLYZERO;
				TFitData fDiff;
				bool bDone = false;
				do
				{
					// backup original parameter
					TFitData fTemp = vNonlinearParams.GetAt(iParamID);

					// modify the parameter
					if(fabs(vNonlinearParams.GetAt(iParamID)) < mNearlyZero)
						vNonlinearParams.SetAt(iParamID, mNearlyZero);
					fDiff = vNonlinearParams.GetAt(iParamID) * mDelta;
					vNonlinearParams.SetAt(iParamID, vNonlinearParams.GetAt(iParamID) + fDiff);

					// get new values
					SetNonlinearParameter(vNonlinearParams);
					GetValues(vXValues, vParamColumn);

					// restore parameter
					vNonlinearParams.SetAt(iParamID, fTemp);
					SetNonlinearParameter(vNonlinearParams);

					// calc slopes for whole vector
					vParamColumn.Sub(vOrig);

#if defined(MATHFIT_AUTOTUNE)
					// if we only have zeros in the vector, tune our border values
					if(vParamColumn.IsZero())
					{
						mNearlyZero /= mDelta;
						if(mNearlyZero >= 1)
						{
							iAutoTuneCount++;
							if(iAutoTuneCount >= iParamID / 2)
								mStopAutoTune = true;
							bDone = true;
						}
					}
					else
						bDone = true;
#else
					bDone = true;
#endif
				}while(!bDone && !mStopAutoTune);

				vParamColumn.Div(fDiff);
			}
			//check wheter all parameters are unusable
			if(iAutoTuneCount < iParamSize / 2)
				mStopAutoTune = false;
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
		virtual TFitData GetLinearBasisFunction(TFitData fXValue, int iParamID, bool bFixedID = true) = 0;

		/**
		* Returns the basis functions of the specified linear parameter at the given X values.
		*
		* @param vXValues			The vector containing the X values at which to determine the basis functions.
		* @param vBasisFunctions	The vector receiving the basis functions.
		* @param iParamID			The index of the linear parameter.
		* @param bFixedID			If TRUE the given parameter ID is the parameter ID without all fixed parameter.
		*/
		virtual void GetLinearBasisFunctions(CVector& vXValues, CVector& vBasisFunctions, int iParamID, bool bFixedID = true)
		{
			const int iXSize = vXValues.GetSize();
			int i;
			for(i = 0; i < iXSize; i++)
				vBasisFunctions.SetAt(i, GetLinearBasisFunction(vXValues.GetAt(i), iParamID, bFixedID));
		}

		/**
		* Returns the A matrix containing the coefficients of the linear model.
		* Since the matrix is defined by the function itself, the default implementation
		* throws an exception.
		* The A matrix consists of rows containing the aproptiate basis functions of the model that build the so called
		* Normal Equations. vB should be initialized with the target function values and this function will modify these values according
		* to the maybe additional constant components of the model.
		*
		* The linear model is given by:
		* A*x=b
		*
		* @param vXValues		The vector object containing the X values.
		* @param mA			The matrix object receiving the A matrix
		* @param vB			The vector object modified by the constant values of the equations.
		*/
		virtual void GetLinearAMatrix(CVector& vXValues, CMatrix& mA, CVector& vB)
		{
			const int iXSize = vXValues.GetSize();
			const int iParamSize = mLinearParams.GetSize();

			int i;
			for(i = 0; i < iParamSize; i++)
			{
				int iRow;
				for(iRow = 0; iRow < iXSize; iRow++)
					mA.SetAt(iRow, i, GetLinearBasisFunction(vXValues.GetAt(iRow), i));
			}
		}

		/**
		* Returns a value greater than zero if the current linear parameters exceed their bounds.
		* The default implementation returns always zero.
		*
		* @param fChiSquare	The current chi square value. Maybe used by this function.
		*
		* @return The penalty value for the current linear parameters.
		*/
		virtual TFitData GetLinearPenalty(TFitData fChiSquare)
		{
			return mLinearParams.GetPenalty(fChiSquare);
		}

		virtual bool SetLinearParameterLimits(int iParamID, TFitData fLowLimit, TFitData fHighLimit, TFitData fPenaltyFactor)
		{
			return mLinearParams.SetParameterLimits(iParamID, fLowLimit, fHighLimit, fPenaltyFactor);
		}

		virtual bool ClearLinearParameterLimits(int iParamID)
		{
			return mLinearParams.ClearParameterLimits(iParamID);
		}

		/**
		* Returns a value greater than zero if the current nonlinear parameters exceed their bounds.
		* The default implementation returns always zero.
		*
		* @param fChiSquare	The current chi square value. Maybe used by this function.
		*
		* @return The penalty value for the current nonlinear parameters.
		*/
		virtual TFitData GetNonlinearPenalty(TFitData fChiSquare)
		{
			return mNonlinearParams.GetPenalty(fChiSquare);
		}

		virtual bool SetNonlinearParameterLimits(int iParamID, TFitData fLowLimit, TFitData fHighLimit, TFitData fPenaltyFactor)
		{
			return mNonlinearParams.SetParameterLimits(iParamID, fLowLimit, fHighLimit, fPenaltyFactor);
		}

		virtual bool ClearNonlinearParameterLimits(int iParamID)
		{
			return mNonlinearParams.ClearParameterLimits(iParamID);
		}

		/**
		* Sets the covariance matrix of the nonlinear parameters.
		*
		* @param mCovar	The covariance matrix.
		*/
		virtual void SetNonlinearCovarMatrix(CMatrix& mCovar)
		{
			mNonlinearParams.SetCovarMatrix(mCovar);
		}

		/**
		* Returns the models covariance matrix of the nonlinear parameters.
		*
		* @return	The covariance matrix of the nonlinear model parameters.
		*/
		virtual CMatrix& GetNonlinearCovarMatrix()
		{
			return mNonlinearParams.GetCovarMatrix();
		}

		/**
		* Sets the correlation matrix of the nonlinear parameters.
		*
		* @param mCorrel	The correlation matrix.
		*/
		virtual void SetNonlinearCorrelMatrix(CMatrix& mCorrel)
		{
			mNonlinearParams.SetCorrelMatrix(mCorrel);
		}

		/**
		* Returns the models correlation matrix of the nonlinear parameters.
		*
		* @return	The correlation matrix of the nonlinear model parameters.
		*/
		virtual CMatrix& GetNonlinearCorrelMatrix()
		{
			return mNonlinearParams.GetCorrelMatrix();
		}

		/**
		* Sets the error of the nonlinear parameters.
		*
		* @param vError	The error vector.
		*/
		virtual void SetNonlinearError(CVector& vError)
		{
			mNonlinearParams.SetError(vError);
		}

		/**
		* Returns the error of the nonlinear parameters.
		*
		* @return	The error vector of the nonlinear model parameters.
		*/
		virtual CVector& GetNonlinearError()
		{
			return mNonlinearParams.GetError();
		}

		/**
		* Sets the covariance matrix of the linear parameters.
		*
		* @param mCovar	The covariance matrix.
		*/
		virtual void SetLinearCovarMatrix(CMatrix& mCovar)
		{
			mLinearParams.SetCovarMatrix(mCovar);
		}

		/**
		* Returns the models covariance matrix of the linear parameters.
		*
		* @return	The covariance matrix of the linear model parameters.
		*/
		virtual CMatrix& GetLinearCovarMatrix()
		{
			return mLinearParams.GetCovarMatrix();
		}

		/**
		* Sets the correlation matrix of the linear parameters.
		*
		* @param mCorrel	The correlation matrix.
		*/
		virtual void SetLinearCorrelMatrix(CMatrix& mCorrel)
		{
			mLinearParams.SetCorrelMatrix(mCorrel);
		}

		/**
		* Returns the models correlation matrix of the linear parameters.
		*
		* @return	The correlation matrix of the linear model parameters.
		*/
		virtual CMatrix& GetLinearCorrelMatrix()
		{
			return mLinearParams.GetCorrelMatrix();
		}

		/**
		* Sets the error of the linear parameters.
		*
		* @param vError	The error vector.
		*/
		virtual void SetLinearError(CVector& vError)
		{
			mLinearParams.SetError(vError);
		}

		/**
		* Returns the error of the linear parameters.
		*
		* @return	The error vector of the linear model parameters.
		*/
		virtual CVector& GetLinearError()
		{
			return mLinearParams.GetError();
		}

		/**
		* Resets the linear parameters to default values.
		* The default implementation sets every parameter to zero.
		*/
		virtual void ResetLinearParameter()
		{
			mLinearParams.Reset();
		}

		/**
		* Resets the nonlinear parameters to default values.
		* The default implementation sets every parameter to zero.
		*/
		virtual void ResetNonlinearParameter()
		{
			mNonlinearParams.Reset();
		}

		/**
		* Fixes the specified parameter.
		* The original parameter value will be kept.
		* Any function parameter can be set to a fixes value. A fixed parameter will not be modified anymore.
		* The parameter ID given here is the {\bf global} parameter ID. The global parameter ID
		* starts first with the linear parameters followed by the nonlinear parameters:
		*
		* Global Parameter ID:
		* [0..mLinearParams.GetSize()-1]							: IDs of the linear parameter.
		* [mLinearParams.GetSize()..mNonlinearParams.GetSize()]	: IDs of the nonlinear parameter.
		*
		* @param iParamID	The global ID of the parameter to fix.
		*
		* @return	TRUE if successful, FALSE if the parameter couldn't be fixed.
		*/
		virtual bool FixParameter(int iParamID)
		{
			if(iParamID >= mLinearParams.GetAllSize())
			{
				iParamID -= mLinearParams.GetAllSize();
				return mNonlinearParams.FixParameter(iParamID);
			}
			else
			{
				return mLinearParams.FixParameter(iParamID);
			}

			return false;
		}

		/**
		* Fixes the specified parameter.
		* Any function parameter can be set to a fixes value. A fixed parameter will not be modified.
		* The parameter ID given here is the {\bf global} parameter ID. The global parameter ID
		* starts first with the linear parameters followed by the nonlinear parameters:
		*
		* Global Parameter ID:
		* [0..mLinearParams.GetSize()-1]							: IDs of the linear parameter.
		* [mLinearParams.GetSize()..mNonlinearParams.GetSize()]	: IDs of the nonlinear parameter.
		*
		* @param iParamID	The global ID of the parameter to fix.
		* @param fValue	The value to which the parameter should be fixed.
		*
		* @return	TRUE if successful, FALSE if the parameter couldn't be fixed.
		*/
		virtual bool FixParameter(int iParamID, TFitData fValue)
		{
			if(iParamID >= mLinearParams.GetAllSize())
			{
				iParamID -= mLinearParams.GetAllSize();
				return mNonlinearParams.FixParameter(iParamID, fValue);
			}
			else
			{
				return mLinearParams.FixParameter(iParamID, fValue);
			}
		}

		/**
		* Releases a fixed parameter.
		* To set a fixed parameter back to a freely modifiable state, you have to release it using
		* this function.
		*
		* @param iParamID	The global ID of the parameter to be releases. (See \Ref{FixParameter}).
		*
		* @return	TRUE if successful, FALSE otherwise.
		*/
		virtual bool ReleaseParameter(int iParamID)
		{
			MATHFIT_ASSERT(iParamID >= 0 && iParamID < (mLinearParams.GetAllSize() + mNonlinearParams.GetAllSize()));

			if(iParamID >= mLinearParams.GetAllSize())
			{
				iParamID -= mLinearParams.GetAllSize();
				return mNonlinearParams.ReleaseParameter(iParamID);
			}
			else
			{
				return mLinearParams.ReleaseParameter(iParamID);
			}

			return false;
		}

		/**
		* Sets the specified parameter.
		* Any function parameter can be set to a defined start value.
		* The parameter ID given here is the {\bf global} parameter ID. The global parameter ID
		* starts first with the linear parameters followed by the nonlinear parameters:
		*
		* Global Parameter ID:
		* [0..mLinearParams.GetSize()-1]							: IDs of the linear parameter.
		* [mLinearParams.GetSize()..mNonlinearParams.GetSize()]	: IDs of the nonlinear parameter.
		*
		* @param iParamID	The global ID of the parameter to be set.
		* @param fValue	The value to which the parameter should be set.
		*
		* @return	TRUE if successful, FALSE if the parameter couldn't be fixed.
		*/
		virtual bool SetParameter(int iParamID, TFitData fValue)
		{
			if(iParamID >= mLinearParams.GetAllSize())
			{
				iParamID -= mLinearParams.GetAllSize();
				return mNonlinearParams.SetParameter(iParamID, fValue);
			}
			else
			{
				return mLinearParams.SetParameter(iParamID, fValue);
			}

			return false;
		}

		/**
		* Sets the specified parameter's default value.
		* Any function parameter can be set to a defined start value.
		* The parameter ID given here is the {\bf global} parameter ID. The global parameter ID
		* starts first with the linear parameters followed by the nonlinear parameters:
		*
		* Global Parameter ID:
		* [0..mLinearParams.GetSize()-1]							: IDs of the linear parameter.
		* [mLinearParams.GetSize()..mNonlinearParams.GetSize()]	: IDs of the nonlinear parameter.
		*
		* @param iParamID	The global ID of the parameter to be set.
		* @param fValue	The value to which the parameter should be set.
		*
		* @return	TRUE if successful, FALSE if the parameter couldn't be fixed.
		*/
		virtual bool SetDefaultParameter(int iParamID, TFitData fValue)
		{
			if(iParamID >= mLinearParams.GetAllSize())
			{
				iParamID -= mLinearParams.GetAllSize();
				return mNonlinearParams.SetAllDefaultParameter(iParamID, fValue);
			}
			else
			{
				return mLinearParams.SetAllDefaultParameter(iParamID, fValue);
			}

			return false;
		}

		/**
		* Links two function parameters together.
		* The specified parameter of the given target function will be linked to the 
		* given parameter of the current object. So any modification of the specified parameter
		* of the current object also will be forwarded to all linked parameters.
		* All Parameter IDs are given as {\bf global} IDs (see \Ref{FixParameter}!
		*
		* @param iSrcID	The global parameter ID of the current object's parameter which will be used as link source.
		* @param ipfTarget	The function object to which the source parameter should be forwarded.
		* @param iTargetID	The global parameter ID of the target object's parameter which will receive the source's modifications.
		*
		* @return	TRUE if successful, FALSE otherwise.
		*/
		virtual bool LinkParameter(int iSrcID, IParamFunction& ipfTarget, int iTargetID)
		{
			MATHFIT_ASSERT(iSrcID >= 0 && iSrcID < (mLinearParams.GetAllSize() + mNonlinearParams.GetAllSize()));
			MATHFIT_ASSERT(iTargetID >= 0 && iTargetID < (ipfTarget.mLinearParams.GetAllSize() + ipfTarget.mNonlinearParams.GetAllSize()));

			// determine the combination of the linear and nonlinear source-target parameters
			if(iSrcID >= mLinearParams.GetAllSize())
			{
				iSrcID -= mLinearParams.GetAllSize();

				if(iTargetID >= ipfTarget.mLinearParams.GetAllSize())
				{
					iTargetID -= ipfTarget.mLinearParams.GetAllSize();

					// we have to link nonlinear to nonlinear parameter
					return mNonlinearParams.LinkParameter(iSrcID, ipfTarget.mNonlinearParams, iTargetID);
				}
				else
				{
					// her we link nonlinear to linear parameter
					return mNonlinearParams.LinkParameter(iSrcID, ipfTarget.mLinearParams, iTargetID);
				}
			}
			else
			{
				if(iTargetID >= ipfTarget.mLinearParams.GetAllSize())
				{
					iTargetID -= ipfTarget.mLinearParams.GetAllSize();

					// link linear to nonlinear parameter
					return mLinearParams.LinkParameter(iSrcID, ipfTarget.mNonlinearParams, iTargetID);
				}
				else
				{
					// link linear to linear parameter
					return mLinearParams.LinkParameter(iSrcID, ipfTarget.mLinearParams, iTargetID);
				}
			}

			return false;
		}

		/**
		* Removes a link between two parameters.
		* The given link between the two parameters will be removed.
		* All parameter IDs are defined as {\bf global} IDs (see \Ref{FixParameter})!
		*
		* @param iSrcID	The global parameter ID of the current object's parameter which will be used as link source.
		* @param ipfTarget	The function object to which the source parameter should be forwarded.
		* @param iTargetID	The global parameter ID of the target object's parameter which will receive the source's modifications.
		*
		* @return	TRUE if successful, FALSE otherwise.
		*/
		virtual bool UnlinkParameter(int iSrcID, IParamFunction& ipfTarget, int iTargetID)
		{
			MATHFIT_ASSERT(iSrcID >= 0 && iSrcID < (mLinearParams.GetAllSize() + mNonlinearParams.GetAllSize()));
			MATHFIT_ASSERT(iTargetID >= 0 && iTargetID < (ipfTarget.mLinearParams.GetAllSize() + ipfTarget.mNonlinearParams.GetAllSize()));

			// determine the combination of the linear and nonlinear source-target parameters
			if(iSrcID >= mLinearParams.GetAllSize())
			{
				iSrcID -= mLinearParams.GetAllSize();

				if(iTargetID >= ipfTarget.mLinearParams.GetAllSize())
				{
					iTargetID -= ipfTarget.mLinearParams.GetAllSize();

					// we have to link nonlinear to nonlinear parameter
					return mNonlinearParams.UnlinkParameter(iSrcID, ipfTarget.mNonlinearParams, iTargetID);
				}
				else
				{
					// her we link nonlinear to linear parameter
					return mNonlinearParams.UnlinkParameter(iSrcID, ipfTarget.mLinearParams, iTargetID);
				}
			}
			else
			{
				if(iTargetID >= ipfTarget.mLinearParams.GetAllSize())
				{
					iTargetID -= ipfTarget.mLinearParams.GetAllSize();

					// link linear to nonlinear parameter
					return mLinearParams.UnlinkParameter(iSrcID, ipfTarget.mNonlinearParams, iTargetID);
				}
				else
				{
					// link linear to linear parameter
					return mLinearParams.UnlinkParameter(iSrcID, ipfTarget.mLinearParams, iTargetID);
				}
			}

			return false;
		}

		/**
		* Returns the parameter vector object for the linear parameters.
		*
		* @return	A reference to the linear parameter vector object.
		*/
		virtual CParameterVector& GetLinearParameterVector()
		{
			return mLinearParams;
		}

		/**
		* Returns the parameter vector object for the nonlinear parameters.
		*
		* @return	A reference to the nonlinear parameter vector object.
		*/
		virtual CParameterVector& GetNonlinearParameterVector()
		{
			return mNonlinearParams;
		}

		virtual void SetFitRange(CVector& vFitRange)
		{
			mFitRange.Copy(vFitRange);
		}

		virtual CVector& GetFitRange()
		{
			return mFitRange;
		}

	protected:
		/**
		* Contains the linear model parameters.
		*/
		CParameterVector mLinearParams;
		/**
		* Contains the backup of the linear parameters.
		*/
		CVector mBackupLinearParams;
		/**
		* Contains the nonlinear model parameters.
		*/
		CParameterVector mNonlinearParams;
		/**
		* Contains the backup of the nonlinear parameters.
		*/
		CVector mBackupNonlinearParams;
		/**
		* The nearly zero value. Anything below this value is set to the value, since we cannont allow nonlinear parameters to be zero at all.
		*/
		TFitData mNearlyZero;
		/**
		* This is the delta increase for differential slope determination.
		*/
		TFitData mDelta;
		CVector mFitRange;
		bool mStopAutoTune;
	};
}

#pragma warning (pop)
#endif // !defined(AFX_IFUNCTION_H__7852C2C7_0389_41DB_B169_3C30252ADD47__INCLUDED_)
