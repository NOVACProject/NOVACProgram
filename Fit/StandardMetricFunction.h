/**
 * Contains the implementation of a standard norm function.
 *
 * @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
 * @version		1.0 @ 2001/09/09
 */
#if !defined(STANDARDMETRIC_H_011206)
#define STANDARDMETRIC_H_011206

#include "ParamFunction.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning (push, 3)

namespace MathFit
{
	/**
	* Implements a function that calculates the difference between the given target data and the model function.
	* The sum of these function values should be minimized and therefore
	* the model be fitted to the target function. Most methods just map back to the model function object.
	*
	* @author		\item \URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
	* @author		\item \URL[Silke Humbert]{mailto:silke.humbert@iup.uni-heidelberg.de} @ \URL[IUP, Satellite Data Group]{http://giger.iup.uni-heidelberg.de}
	* @version		1.0 @ 2001/09/09
	*/
	class CStandardMetricFunction : public IParamFunction
	{
	public:
		/**
		* Sets the model function and the target data.
		*
		* @param ipfTarget	The data sample to which the model should be fitted
		* @param ipfModel	The model function object.
		*/
		CStandardMetricFunction(IFunction& ipfTarget, IParamFunction& ipfModel) : mModel(ipfModel), mTarget(ipfTarget)
		{
			CStandardMetricFunction::SetData(mTarget.GetXData(), mTarget.GetYData(), mTarget.GetErrorData());
		}

		/**
		* This function is not implemented.
		* Since we do not need it for fitting and we do not have 
		* a simple algorithm to map the given X value to one of the target data points.
		* Always throws an exception!
		*
		* @param fXValue	The Xvalue at which the function should be evaluated.
		*
		* @return The function value at the given point.
		*/
		virtual TFitData GetValue(TFitData fXValue)
		{
			return mTarget.GetValue(fXValue) - mModel.GetValue(fXValue);
		}

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
			mTarget.GetValues(vXValues, vYTargetVector);

			CVector vTemp(vXValues.GetSize());
			mModel.GetValues(vXValues, vTemp);

			vYTargetVector.Sub(vTemp);

			return vYTargetVector;
		}

		/**
		* Like GetValue this function is also not needed and therefore not implemented.
		*
		* @param fXValue	The X value at which the slope should be determined.
		*
		* @return The slope of the function at the given point.
		*/
		virtual TFitData GetSlope(TFitData fXValue)
		{
			throw(EXCEPTION(CNotImplementedException));
		}

		/**
		* Maps to the model object.
		*
		* @return The vector object containing the nonlinear model parameters.
		*
		* @see	IParamFunction::GetNonlinearParams
		*/
		virtual CVector& GetNonlinearParameter()
		{
			return mModel.GetNonlinearParameter();
		}

		/**
		* Maps to the model object
		*
		* @param vNonlinParam	The vector object containing the new parameters.
		*
		* @return TRUE if successful, FALSE otherwise.
		*
		* @see	IParamFunction::SetNonlinearParams
		*/
		virtual bool SetNonlinearParameter(CVector& vNonlinParam)
		{
			return mModel.SetNonlinearParameter(vNonlinParam);
		}

		/**
		* Maps to model object
		*
		* @see	IParamFunction::BackupNonlinearParams
		*/
		virtual void BackupNonlinearParameter()
		{
			mModel.BackupNonlinearParameter();
		}

		/**
		* Maps to model object
		*
		* @see	IParamFunction::RestoreNonlinearParams
		*/
		virtual void RestoreNonlinearParameter()
		{
			mModel.RestoreNonlinearParameter();
		}

		/**
		* Maps to the model object.
		*
		* @return The vector object containing the linear model parameters.
		*
		* @see	IParamFunction::GetLinearParams
		*/
		virtual CVector& GetLinearParameter()
		{
			return mModel.GetLinearParameter();
		}

		/**
		* Maps to the model object
		*
		* @param vLinParam	The vector object containing the new parameters.
		*
		* @return TRUE if successful, FALSE otherwise.
		*
		* @see	IParamFunction::SetLinearParams
		*/
		virtual bool SetLinearParameter(CVector& vLinParam)
		{
			return mModel.SetLinearParameter(vLinParam);
		}

		/**
		* Maps to model object
		*
		* @see	IParamFunction::BackupLinearParams
		*/
		virtual void BackupLinearParameter()
		{
			mModel.BackupLinearParameter();
		}

		/**
		* Maps to model object
		*
		* @see	IParamFunction::RestoreLinearParams
		*/
		virtual void RestoreLinearParameter()
		{
			mModel.RestoreLinearParameter();
		}

		/**
		* Maps to model object
		*
		* @param fXValue	The data point at which the slope should be determined.
		* @param iParamID	The index within the nonlinear parameter vector of the nonlinear parameter.
		* @param bFixedID	If TRUE the given parameter ID is the parameter ID without all fixed parameter.
		*
		* @return	The slope of the function in regard to the given nonlinear parameter.
		*
		* @see	IParamFunction::GetNonlinearParamSlope
		*/
		virtual TFitData GetNonlinearParamSlope(TFitData fXValue, int iParamID, bool bFixedID = true)
		{
			return mModel.GetNonlinearParamSlope(fXValue, iParamID, bFixedID);
		}

		/**
		* Maps to model object.
		* 
		* @param vXValue	The data points at which the slope should be determined.
		* @param vSlopes	The vector object which will receive the slope values.
		* @param iParamID	The index within the nonlinear parameter vector of the nonlinear parameter.
		* @param bFixedID	If TRUE the given parameter ID is the parameter ID without all fixed parameter.
		*
		* @see	IParamFunction::GetNonlinearParamSlopes
		*/
		virtual void GetNonlinearParamSlopes(CVector& vXValue, CVector& vSlopes, int iParamID, bool bFixedID = true)
		{
			mModel.GetNonlinearParamSlopes(vXValue, vSlopes, iParamID, bFixedID);
		}

		/**
		* Maps to model object.
		*
		* @param iParamID	The index within the nonlinear parameter vector of the nonlinear parameter.
		* @param vXValue	The data points at which the slope should be determined.
		* @param mDyDa		The matrix object receiving the derivative values of the function at the given data points.
		*
		* @see	IParamFunction::GetNonlinearDyDa
		*/
		virtual void GetNonlinearDyDa(CVector& vXValues, CMatrix& mDyDa)
		{
			mModel.GetNonlinearDyDa(vXValues, mDyDa);
		}

		/**
		* Maps to model object
		*
		* @param fXValue	The data point at which the basis function should be determined.
		* @param iParamID	The index within the linear parameter vector of the linear parameter.
		* @param bFixedID	If TRUE the given parameter ID is the parameter ID without all fixed parameter.
		*
		* @return	The basis function in regard to the given linear parameter.
		*
		* @see	IParamFunction::GetLinearBasisFunction
		*/
		virtual TFitData GetLinearBasisFunction(TFitData fXValue, int iParamID, bool bFixedID = true)
		{
			return mModel.GetLinearBasisFunction(fXValue, iParamID, bFixedID);
		}
		/**
		* Maps to model object
		*
		* @param vXValues			The vector containing the X values at which to determine the basis functions.
		* @param vBasisFunctions	The vector receiving the basis functions.
		* @param iParamID			The index within the linear parameter vector of the linear parameter.
		* @param bFixedID			If TRUE the given parameter ID is the parameter ID without all fixed parameter.
		*
		* @see	IParamFunction::GetLinearBasisFunctions
		*/
		virtual void GetLinearBasisFunctions(CVector& vXValues, CVector& vBasisFunctions, int iParamID, bool bFixedID = true)
		{
			mModel.GetLinearBasisFunctions(vXValues, vBasisFunctions, iParamID, bFixedID);
		}

		/**
		* Maps to model object
		*
		* @param vXValues	The vector object containing the X values at which we need the A matrix.
		* @param mA		The matrix object receiving the resulting matrix.
		* @param vB		The vector object receiving the constant values of the equations.
		*
		* @see	IParamFunction::GetLinearAMatrix
		*/
		virtual void GetLinearAMatrix(CVector& vXValues, CMatrix& mA, CVector& vB)
		{
			// set the target data as B vector
			mTarget.GetValues(vXValues, vB);

			mModel.GetLinearAMatrix(vXValues, mA, vB);
		}

		/**
		* Maps to model object.
		*
		* @param fChiSquare	The current chi square value. Maybe used by this function.
		*
		* @return The penalty for the current nonlinear parameters.
		*
		* @see	IParamFunction::GetNonlinearPenalty
		*/
		virtual TFitData GetNonlinearPenalty(TFitData fChiSquare)
		{
			return mModel.GetNonlinearPenalty(fChiSquare);
		}

		/**
		* Maps to model object.
		*
		* @param fChiSquare	The current chi square value. Maybe used by this function.
		*
		* @return The penalty for the current linear parameters.
		*
		* @see	IParamFunction::GetLinearPenalty
		*/
		virtual TFitData GetLinearPenalty(TFitData fChiSquare)
		{
			return mModel.GetLinearPenalty(fChiSquare);
		}

		/**
		* Maps to model object.
		*
		* @param mCovar	The covariance matrix.
		*
		* @see	IParamFunction::SetNonlinearCovarMatrix
		*/
		virtual void SetNonlinearCovarMatrix(CMatrix& mCovar)
		{
			mModel.SetNonlinearCovarMatrix(mCovar);
		}

		/**
		* Maps to model object.
		*
		* @return	The covariance matrix of the nonlinear model parameters.
		*
		* @see IParamFunction::GetNonlinearCovarMatrix
		*/
		virtual CMatrix& GetNonlinearCovarMatrix()
		{
			return mModel.GetNonlinearCovarMatrix();
		}

		/**
		* Maps to model object.
		*
		* @param mCorrel	The correlation matrix.
		*
		* @see	IParamFunction::SetNonlinearCorrelMatrix
		*/
		virtual void SetNonlinearCorrelMatrix(CMatrix& mCorrel)
		{
			mModel.SetNonlinearCorrelMatrix(mCorrel);
		}

		/**
		* Maps to model object.
		*
		* @return	The correlation matrix of the nonlinear model parameters.
		*
		* @see	IParamFunction::GetNonlinearCorrelMatrix
		*/
		virtual CMatrix& GetNonlinearCorrelMatrix()
		{
			return mModel.GetNonlinearCorrelMatrix();
		}

		/**
		* Maps to model object.
		*
		* @param vError	The error vector.
		*
		* @see IParamFunction::SetNonlinearError
		*/
		virtual void SetNonlinearError(CVector& vError)
		{
			mModel.SetNonlinearError(vError);
		}

		/**
		* Maps to model object.
		*
		* @return	The error vector of the nonlinear model parameters.
		*
		* @see	IParamFunction::GetNonlinearError
		*/
		virtual CVector& GetNonlinearError()
		{
			return mModel.GetNonlinearError();
		}

		/**
		* Maps to model object.
		*
		* @param mCovar	The covariance matrix.
		*
		* @see	IParamFunction::SetLinearCovarMatrix
		*/
		virtual void SetLinearCovarMatrix(CMatrix& mCovar)
		{
			mModel.SetLinearCovarMatrix(mCovar);
		}

		/**
		* Maps to model object.
		*
		* @return	The covariance matrix of the linear model parameters.
		*
		* @see	IParamFunction::GetLinearCovarMatrix
		*/
		virtual CMatrix& GetLinearCovarMatrix()
		{
			return mModel.GetLinearCovarMatrix();
		}

		/**
		* Maps to model object.
		*
		* @param mCorrel	The correlation matrix.
		*
		* @see	IParamFunction::SetLinearCorrelMatrix
		*/
		virtual void SetLinearCorrelMatrix(CMatrix& mCorrel)
		{
			mModel.SetLinearCorrelMatrix(mCorrel);
		}

		/**
		* Maps to model object.
		*
		* @return	The correlation matrix of the linear model parameters.
		*
		* @see	IParamFunction::GetLinearCorrelMatrix
		*/
		virtual CMatrix& GetLinearCorrelMatrix()
		{
			return mModel.GetLinearCorrelMatrix();
		}

		/**
		* Maps to model object.
		*
		* @param vError	The error vector.
		*
		* @see	IParamFunction::SetLinearError
		*/
		virtual void SetLinearError(CVector& vError)
		{
			mModel.SetLinearError(vError);
		}

		/**
		* Maps to model object.
		*
		* @return	The error vector of the linear model parameters.
		*
		* @see	IParamFunction::GetLinearError
		*/
		virtual CVector& GetLinearError()
		{
			return mModel.GetLinearError();
		}

		/**
		* Resets the linear parameters to default values.
		* The default implementation sets every parameter to zero.
		*/
		virtual void ResetLinearParameter()
		{
			mModel.ResetLinearParameter();

		}

		/**
		* Resets the nonlinear parameters to default values.
		* The default implementation sets every parameter to zero.
		*/
		virtual void ResetNonlinearParameter()
		{
			mModel.ResetNonlinearParameter();
		}

		virtual void SetFitRange(CVector& vFitRange)
		{
			mFitRange.Copy(vFitRange);
			mModel.SetFitRange(vFitRange);
		}

	private:
		/**
		* Represents the model function object. Its parameters should be fitted against the given target function values.
		*/
		IParamFunction& mModel;
		/**
		* The target function.
		*/
		IFunction& mTarget;
	};
}

#pragma warning (pop)
#endif
