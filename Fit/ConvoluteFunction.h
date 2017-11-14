/**
* Contains a defintion of a concolution function object.
*
* @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
* @version		1.0 @ 2002/05/23
*/
#if !defined(CONVOLUTEFUNCTION_H_020523)
#define CONVOLUTEFUNCTION_H_020523

#include "StatisticVector.h"
#include "ParamFunction.h"
#include "ConvolutionCoreFunction.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

namespace MathFit
{
	/**
	* This object represents a convolution function.
	*
	* @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
	* @version		1.0 @ 2002/05/23
	*/
	class CConvoluteFunction : public IParamFunction
	{
	public:
		/**
		* Creates the object and associates an exponent function object.
		*
		* @param ipfExponent	The function object that should be used as exponent function.
		*/
		CConvoluteFunction(IParamFunction& ipfBase, IConvolutionCoreFunction& ipfCore) : mBase(ipfBase), mCore(ipfCore)
		{
			BuildLinearParameter();
			BuildNonlinearParameter();
		}

		/**
		* Returns the exponential value of the exponent function.
		*
		* \begin{verbatim}f(x)=exp(exponent(x))\end{verbatim}
		*
		* @param fXValue	The X data point.
		*
		* @return	The value of the function at the given data point.
		*/
		virtual TFitData GetValue(TFitData fXValue)
		{
			// set the current core location
			mCore.SetCoreCenter(fXValue);

			// get the core boundaries
			TFitData fCoreLowBound = mCore.GetCoreLowBound();
			TFitData fCoreHighBound = mCore.GetCoreHighBound();
			if(fCoreLowBound == fCoreHighBound)
			{
				// if the core size is not defined, use some defaults
				fCoreLowBound = -50;
				fCoreHighBound = 50;
			}
			else
			{
				// correct the core boundaries to base coordinates
				fCoreLowBound += fXValue;
				fCoreHighBound += fXValue;
			}

			CVector vXData;
			if(mBase.GetXData().GetSize() <= 0)
			{
				// if no base data is sampled, assume that 100 samples will do the trick
				vXData.SetSize(100);
				vXData.Wedge(fCoreLowBound, (fCoreHighBound - fCoreLowBound) / vXData.GetSize());
			}
			else
				vXData.Attach(mBase.GetXData(), false);

			// get the core windows indicies of the base function
			int iBaseLowIndex = vXData.FindIndex(fCoreLowBound, CVector::LESS);
			if(iBaseLowIndex < 0)
				iBaseLowIndex = 0;
			int iBaseHighIndex = vXData.FindIndex(fCoreHighBound, CVector::GREATER);
			if(iBaseHighIndex < 0)
				iBaseHighIndex = vXData.GetSize() - 1;

			const int iRangeSize = iBaseHighIndex - iBaseLowIndex + 1;

			// get X range
			CVector& vRange = vXData.SubVector(iBaseLowIndex, iRangeSize);

			CStatisticVector vBase(iRangeSize);
			CVector vCore(iRangeSize);

			// get values of the base- and the core-function 
			mBase.GetValues(vRange, vBase);

			// transform the range to core coordinates, get the current core and do a back-transform
			vRange.Sub(fXValue);
			mCore.GetValues(vRange, vCore);
			vRange.Add(fXValue);

			// do the convolution in the slow, but simple way
			vBase.MulSimple(vCore);
			return vBase.Sum();
		}

		/**
		* Calculates the function values at a set of given data points.
		*
		* @param vXValues			A vector object containing the X values at which the function has to be evaluated.
		* @param vYTargetVector	A vector object which receives the resulting function values.
		*
		* @return	A reference to the Y vector object.
		*
		* @see	GetValue
		*/
		virtual CVector& GetValues(CVector& vXValues, CVector& vYTargetVector)
		{
			const int iXSize = vXValues.GetSize();

			// check wheter we should use the base's samples data or the currently given sample vector.
			// in case we have a higher resolution base, its better to use the base's resolution for convolution.
			CVector vXData;
			if(mBase.GetXData().GetSize() <= 0)
				vXData.Attach(vXValues, false);
			else
				vXData.Attach(mBase.GetXData(), false);

			CVector vCore;
			CStatisticVector vBase;

			int i;
			for(i = 0; i < iXSize; i++)
			{
				// set current center coordinate
				TFitData fXCenter = vXValues.GetAt(i);
				mCore.SetCoreCenter(fXCenter);

				// get the data from the convolution core
				TFitData fCoreLowBound = mCore.GetCoreLowBound();
				TFitData fCoreHighBound = mCore.GetCoreHighBound();
				if(fCoreLowBound == fCoreHighBound)
				{
					// if the core size is not given, use the maximum and minimum values possible:
					fCoreLowBound = vXData.GetAt(0);
					fCoreHighBound = vXData.GetAt(vXData.GetSize() - 1);
				}
				else
				{
					fCoreLowBound += fXCenter;
					fCoreHighBound += fXCenter;
				}

				// get the core windows indicies of the base function
				int iBaseLowIndex = vXData.FindIndex(fCoreLowBound, CVector::LESS);
				if(iBaseLowIndex < 0)
					iBaseLowIndex = 0;
				int iBaseHighIndex = vXData.FindIndex(fCoreHighBound, CVector::GREATER);
				if(iBaseHighIndex < 0)
					iBaseHighIndex = vXData.GetSize() - 1;

				const int iRangeSize = iBaseHighIndex - iBaseLowIndex + 1;
				
				// get X range
				CVector& vRange = vXData.SubVector(iBaseLowIndex, iRangeSize);

				vBase.SetSize(iRangeSize);
				vCore.SetSize(iRangeSize);

				// get values of the base function at the given range
				mBase.GetValues(vRange, vBase);

				// adapt the range of the core extend and get the core values
				vRange.Sub(fXCenter);
				mCore.GetValues(vRange, vCore);
				vRange.Add(fXCenter);

				// do the simple convolution algorithm
				vBase.MulSimple(vCore);

				vYTargetVector.SetAt(i, vBase.Sum());
			}

			return vYTargetVector;
		}

		/**
		* Returns the first derivative of the function at the given data point.
		*
		* @param fXValue	The X data point at which the first derivation is needed.
		*
		* @return	The slope of the function at the given data point.
		*/
		virtual TFitData GetSlope(TFitData fXValue)
		{
			throw(EXCEPTION(CNotImplementedException));
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
			throw(EXCEPTION(CNotImplementedException));
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

			// split up vevctor into linear and nonlinear parts.
			int iSize, iOffset;
			iOffset = 0;
			iSize = mBase.GetLinearParameter().GetSize();
			if(iSize > 0)
				mBase.SetLinearParameter(vParam.SubVector(iOffset, iSize));
			iOffset += iSize;
			iSize = mBase.GetNonlinearParameter().GetSize();
			if(iSize > 0)
				mBase.SetNonlinearParameter(vParam.SubVector(iOffset, iSize));
			iOffset += iSize;
			iSize = mCore.GetLinearParameter().GetSize();
			if(iSize > 0)
				mCore.SetLinearParameter(vParam.SubVector(iOffset, iSize));
			iOffset += iSize;
			iSize = mCore.GetNonlinearParameter().GetSize();
			if(iSize > 0)
				mCore.SetNonlinearParameter(vParam.SubVector(iOffset, iSize));

			return true;
		}

		/**
		* Sets the covariance matrix of the nonlinear parameters.
		*
		* @param mCovar	The covariance matrix.
		*/
		virtual void SetNonlinearCovarMatrix(CMatrix& mCovar)
		{
			mNonlinearParams.SetCovarMatrix(mCovar);

			// split up vector into linear and nonlinear parts.
			int iSize, iOffset;
			iOffset = 0;
			iSize = mBase.GetLinearParameter().GetSize();
			if(iSize > 0)
				mBase.SetLinearCovarMatrix(mCovar.SubMatrix(iOffset, iOffset, iSize, iSize));
			iOffset += iSize;
			iSize = mBase.GetNonlinearParameter().GetSize();
			if(iSize > 0)
				mBase.SetNonlinearCovarMatrix(mCovar.SubMatrix(iOffset, iOffset, iSize, iSize));
			iOffset += iSize;
			iSize = mCore.GetLinearParameter().GetSize();
			if(iSize > 0)
				mCore.SetLinearCovarMatrix(mCovar.SubMatrix(iOffset, iOffset, iSize, iSize));
			iOffset += iSize;
			iSize = mCore.GetNonlinearParameter().GetSize();
			if(iSize > 0)
				mCore.SetNonlinearCovarMatrix(mCovar.SubMatrix(iOffset, iOffset, iSize, iSize));
		}

		/**
		* Sets the correlation matrix of the nonlinear parameters.
		*
		* @param mCorrel	The correlation matrix.
		*/
		virtual void SetNonlinearCorrelMatrix(CMatrix& mCorrel)
		{
			mNonlinearParams.SetCorrelMatrix(mCorrel);

			// split up vector into linear and nonlinear parts.
			int iSize, iOffset;
			iOffset = 0;
			iSize = mBase.GetLinearParameter().GetSize();
			if(iSize > 0)
				mBase.SetLinearCorrelMatrix(mCorrel.SubMatrix(iOffset, iOffset, iSize, iSize));
			iOffset += iSize;
			iSize = mBase.GetNonlinearParameter().GetSize();
			if(iSize > 0)
				mBase.SetNonlinearCorrelMatrix(mCorrel.SubMatrix(iOffset, iOffset, iSize, iSize));
			iOffset += iSize;
			iSize = mCore.GetLinearParameter().GetSize();
			if(iSize > 0)
				mCore.SetLinearCorrelMatrix(mCorrel.SubMatrix(iOffset, iOffset, iSize, iSize));
			iOffset += iSize;
			iSize = mCore.GetNonlinearParameter().GetSize();
			if(iSize > 0)
				mCore.SetNonlinearCorrelMatrix(mCorrel.SubMatrix(iOffset, iOffset, iSize, iSize));
		}

		/**
		* Sets the error of the nonlinear parameters.
		*
		* @param vError	The error vector.
		*/
		virtual void SetNonlinearError(CVector& vError)
		{
			mNonlinearParams.SetError(vError);

			// split up vevctor into linear and nonlinear parts.
			int iSize, iOffset;
			iOffset = 0;
			iSize = mBase.GetLinearParameter().GetSize();
			if(iSize > 0)
				mBase.SetLinearError(vError.SubVector(iOffset, iSize));
			iOffset += iSize;
			iSize = mBase.GetNonlinearParameter().GetSize();
			if(iSize > 0)
				mBase.SetNonlinearError(vError.SubVector(iOffset, iSize));
			iOffset += iSize;
			iSize = mCore.GetLinearParameter().GetSize();
			if(iSize > 0)
				mCore.SetLinearError(vError.SubVector(iOffset, iSize));
			iOffset += iSize;
			iSize = mCore.GetNonlinearParameter().GetSize();
			if(iSize > 0)
				mCore.SetNonlinearError(vError.SubVector(iOffset, iSize));
		}

		/**
		* Resets the linear parameters to default values.
		* The default implementation sets every parameter to zero.
		*/
		virtual void ResetLinearParameter()
		{
			mBase.ResetLinearParameter();
			mCore.ResetLinearParameter();
			BuildLinearParameter();
		}

		/**
		* Resets the nonlinear parameters to default values.
		* The default implementation sets every parameter to zero.
		*/
		virtual void ResetNonlinearParameter()
		{
			mBase.ResetNonlinearParameter();
			mCore.ResetNonlinearParameter();
			BuildNonlinearParameter();
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
			TFitData fSum = mBase.GetNonlinearPenalty(fChiSquare);
			fSum += mCore.GetNonlinearPenalty(fChiSquare);
			fSum += mBase.GetLinearPenalty(fChiSquare);
			fSum += mCore.GetLinearPenalty(fChiSquare);

			return fSum;
		}

		virtual void SetFitRange(CVector& vFitRange)
		{
			mFitRange.Copy(vFitRange);

			mBase.SetFitRange(vFitRange);
			mCore.SetFitRange(vFitRange);
		}

	private:
		/**
		* Builds the linear parameter vector from the operands parameters.
		*/
		void BuildLinearParameter()
		{
			// no linear parameters, since everything gets nonlinear
			mLinearParams.SetSize(0);
		}

		/**
		* Builds the nonlinear parameter vector from the operands parameters.
		*/
		void BuildNonlinearParameter()
		{
			CVector vBuf;

			vBuf.Append(mBase.GetLinearParameter());
			vBuf.Append(mBase.GetNonlinearParameter());
			vBuf.Append(mCore.GetLinearParameter());
			vBuf.Append(mCore.GetNonlinearParameter());
			mNonlinearParams.SetSize(vBuf.GetSize());
			mNonlinearParams.SetParameters(vBuf);
		}

		/**
		* Holds the exponent function.
		*/
		IParamFunction& mBase;
		IConvolutionCoreFunction& mCore;
	};
}
#endif
