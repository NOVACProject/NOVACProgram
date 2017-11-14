/**
 * Contains the a reference spectrum function object.
 *
 * @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
 * @version		1.0 @ 2001/09/21
 */
#if !defined(REFERENCESPECTRUM_H_011206)
#define REFERENCESPECTRUM_H_011206

#include "ParamFunction.h"
#include "CubicSplineFunction.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning (push, 3)

namespace MathFit
{
	/**
	* Represents one single reference spectrum used in the DOAS models.
	* The evaluation of the spectral data is done using the given basis function
	* object, which is by default a Cubic Spline interpolation object. The reference itself can be scaled and might be
	* shifted and squeezed.
	*
	* @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
	* @version		1.1 @ 2003/05/18
	*/
	class CReferenceSpectrumFunction : public IParamFunction
	{
	public:
		/**
		* Model parameter IDs
		*/
		enum EModelParam
		{
			/**
			* ID of the linear fit coefficient parameter.
			*/
			CONCENTRATION = 0,
			/**
			* ID of the nonlinear shift parameter.
			*/
			SHIFT = 1,
			/**
			* ID of the nonlinear squeeze parameter.
			*/
			SQUEEZE = 2,
		};

		/**
		* Creates an empty reference object.
		*
		* An empty reference spectrum object is created that will use a Cubic Spline
		* interpolation as basis function.
		*/
		CReferenceSpectrumFunction()
		{
			SetBasisFunction(mBSpline);

			InitObject();
		}

		/**
		* Creates an empty reference object with the given basis function.
		*
		* @param ifBasisFunction	The basis function object that should be used to evaluate the spectral data.
		*/
		CReferenceSpectrumFunction(IFunction& ifBasisFunction)
		{
			SetBasisFunction(ifBasisFunction);

			InitObject();
		}

		/**
		* Sets the new function values.
		*
		* @param vXValues		A vector object containing the X values of the data set.
		* @param vYValues		A vector object containing the Y values of the data set.
		* @param vError			A vector object containing the <B>sigma</B> error values of the data set.
		*
		* @return TRUE is successful, false if the vector sizes do not match.
		*
		* @see	IDataPoints::SetData
		*/
		virtual bool SetData(CVector& vXValues, CVector& vYValues, CVector& vError)
		{
			// first copy data into internal buffers
			if(!IDataSet::SetData(vXValues, vYValues, vError))
				return false;

			// do the normalization
			if(mNormalize)
				mAmplitudeScale = mYData.Normalize();

			if(!mBasisFunction->SetData(mXData, mYData, vError))
				return false;
			return true;
		}

		/**
		* Sets the new function values.
		*
		* @param vXValues		A vector object containing the X values of the data set.
		* @param vYValues		A vector object containing the Y values of the data set.
		*
		* @return TRUE is successful, false if the vector sizes do not match.
		*
		* @see	IDataPoints::SetData
		*/
		virtual bool SetData(CVector& vXValues, CVector& vYValues)
		{
			CVector vError(vYValues.GetSize());
			vError.Wedge(1, 0);

			return CReferenceSpectrumFunction::SetData(vXValues, vYValues, vError);
		}

		/**
		* Returns the value of the reference spectrum at the given data point.
		*
		* \begin{verbatim}f(x)=c*o(w*x+v)\end{verbatim}
		*
		* where
		\begin{verbatim}
		c := concentration
		w := squeeze
		v := shift
		\end{verbatim}
		*
		* @param fXValue	The X data point.
		*
		* @return	The value of the reference at the given data point.
		*/
		virtual TFitData GetValue(TFitData fXValue)
		{
			TFitData fX = mNonlinearParams.GetAllParameter().CalcPoly(fXValue - mFitRangeLow) + mFitRangeLow;

			// evaluate the spline and scale by the linear factor
			return mLinearParams.GetAllParameter().GetAt(0) * mBasisFunction->GetValue(fX);
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

			// it makes more sens to first modify the X values and then call the B-Spline
			CVector vBuffer(iXSize);

			int i;
			for(i = 0; i < iXSize; i++)
				vBuffer.SetAt(i, mNonlinearParams.GetAllParameter().CalcPoly(vXValues.GetAt(i) - mFitRangeLow ) + mFitRangeLow);

			// get the BSpline data
			mBasisFunction->GetValues(vBuffer, vYTargetVector);
			vYTargetVector.Mul(mLinearParams.GetAllParameter().GetAt(0));

			return vYTargetVector;
		}

		/**
		* Returns the first derivative of the reference spectrum at the given data point.
		* The first derivative is given by
		*
		* \begin{verbatim}f'(x)=c*o(w*x+v)*w\end{verbatim}
		*
		* where
		\begin{verbatim}
		c := concentration
		w := squeeze
		v := shift
		\end{verbatim}
		*
		* @param fXValue	The X data point at which the first derivation is needed.
		*
		* @return	The slope of the reference at the given data point.
		*/
		virtual TFitData GetSlope(TFitData fXValue)
		{
			// get modified X value
			TFitData fX = mNonlinearParams.GetAllParameter().CalcPoly(fXValue - mFitRangeLow) + mFitRangeLow;

			// get slope of X polynomial
			TFitData fDerive = mNonlinearParams.GetAllParameter().CalcPolySlope(fXValue - mFitRangeLow);

			// the slope of the function in regard to the X value is given by
			// df/dx=f'(x)=c*f'(v*x+w)*v
			return mLinearParams.GetAllParameter().GetAt(0) * mBasisFunction->GetSlope(fX) * fDerive;
		}

		/**
		* Calculates the first derivative of the function at a set of given data points.
		*
		* @param vXValues		A vector object containing the X values at which the function has to be evaluated.
		* @param vSlopeVector	A vector object which receives the resulting function values.
		*
		* @return	A reference to the slope vector object.
		*
		* @see GetSlope
		*/
		virtual CVector& GetSlopes(CVector& vXValues, CVector& vSlopeVector)
		{
			const int iXSize = vXValues.GetSize();

			// it makes more sens to first modify the X values and then call the B-Spline
			CVector vBuffer(iXSize);

			int i;
			for(i = 0; i < iXSize; i++)
				vBuffer.SetAt(i, mNonlinearParams.GetAllParameter().CalcPoly(vXValues.GetAt(i) - mFitRangeLow) + mFitRangeLow);

			// get the BSpline data
			mBasisFunction->GetSlopes(vBuffer, vSlopeVector);
			vSlopeVector.Mul(mLinearParams.GetAllParameter().GetAt(0));

			// now multiply by the appropriate factor
			for(i = 0; i < iXSize; i++)
				vSlopeVector.SetAt(i, vSlopeVector.GetAt(i) * mNonlinearParams.GetAllParameter().CalcPolySlope(vXValues.GetAt(i) - mFitRangeLow));

			return vSlopeVector;
		}

		/**
		* Returns the first derivative of the function in regard to a given nonlinear parameter.
		* The derivative is calculates analytically in this case. The function derivation can directly be done
		* using the B-Spline object.
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

			TFitData fX = mNonlinearParams.GetAllParameter().CalcPoly(fXValue - mFitRangeLow) + mFitRangeLow;

			switch(bFixedID ? mNonlinearParams.GetFixed2AllIndex(iParamID) : iParamID)
			{
			case 0:
				{
					// we want the slope for the shift parameter
					// the slope of the function in regard to the shift value is given by
					// df/dw=f'(x)=c*f'(v*x+w)
					return mLinearParams.GetAllParameter().GetAt(0) * mBasisFunction->GetSlope(fX);
				}
			case 1:
				{
					// and now for the squeeze parameters
					// the slope of the function in regard to the squeeze value is given by
					// df/dv=f'(x)=c*f'(v*x+w)*x
					return mLinearParams.GetAllParameter().GetAt(0) * mBasisFunction->GetSlope(fX) * fXValue;
				}
			}

			return 0;
		}

		/**
		* Returns the first derivative of the function in regard to a given nonlinear parameter.
		* The derivative is calculates analytically in this case. The function derivation can directly be done
		* using the B-Spline object.
		* 
		* @param vXValue	The data points at which the slope should be determined.
		* @param vSlopes	The vector object which will receive the slope values.
		* @param iParamID	The index within the nonlinear parameter vector of the nonlinear parameter.
		* @param bFixedID	If TRUE the given parameter ID is the parameter ID without all fixed parameter.
		*/
		virtual void GetNonlinearParamSlopes(CVector& vXValues, CVector& vSlopes, int iParamID, bool bFixedID = true)
		{
			MATHFIT_ASSERT((bFixedID && iParamID >= 0 && iParamID < mNonlinearParams.GetSize()) || (!bFixedID && iParamID >= 0 && iParamID < mNonlinearParams.GetAllSize()));

			const int iXSize = vXValues.GetSize();

			switch(bFixedID ? mNonlinearParams.GetFixed2AllIndex(iParamID) : iParamID)
			{
			case 0:
				{
					// we want the slope for the shift parameter
					CVector vXTemp(vXValues);

					vXTemp.Sub(mFitRangeLow);
					int i;
					for(i = 0; i < iXSize; i++)
						vXTemp.SetAt(i, mNonlinearParams.GetAllParameter().CalcPoly(vXTemp.GetAt(i)));
					vXTemp.Add(mFitRangeLow);
					mBasisFunction->GetSlopes(vXTemp, vSlopes);
					vSlopes.Mul(mLinearParams.GetAllParameter().GetAt(0));
					break;
				}
			case 1:
				{
					// and now for the squeeze parameters
					CVector vXTemp(vXValues);

					vXTemp.Sub(mFitRangeLow);
					int i;
					for(i = 0; i < iXSize; i++)
						vXTemp.SetAt(i, mNonlinearParams.GetAllParameter().CalcPoly(vXTemp.GetAt(i)));
					vXTemp.Add(mFitRangeLow);
					mBasisFunction->GetSlopes(vXTemp, vSlopes);
					vSlopes.Mul(mLinearParams.GetAllParameter().GetAt(0));
					vSlopes.MulSimple(vXValues);
					break;
				}
			}
		}

		/**
		* Returns the first derivative of the function in regard to all nonlinear parameters.
		* We do not want to use the default implementation, which directly calculates the
		* first derivative using the differntial quotient method. We want to use your analytically
		* implemented derivation method.
		* 
		* @param iParamID	The index within the nonlinear parameter vector of the nonlinear parameter.
		* @param vXValue	The data points at which the slope should be determined.
		* @param mDyDa		The matrix object receiving the derivative values of the function at the given data points.
		*/
		virtual void GetNonlinearDyDa(CVector& vXValues, CMatrix& mDyDa)
		{
			const int iParamSize = mNonlinearParams.GetSize();
			const int iXSize = vXValues.GetSize();

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

			// process shift and squeeze
			TFitData fX = mNonlinearParams.GetAllParameter().CalcPoly(fXValue - mFitRangeLow) + mFitRangeLow;

			// evaluate the spline
			return mBasisFunction->GetValue(fX);
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
			MATHFIT_ASSERT((bFixedID && iParamID >= 0 && iParamID < mLinearParams.GetSize()) || (!bFixedID && iParamID >= 0 && iParamID < mLinearParams.GetAllSize()));

			const int iXSize = vXValues.GetSize();

			// we only have one linear parameter: the concentration
			// therefore we can only fill the vector with the appropriate B-Spline coefficients
			CVector vBuffer(iXSize);

			// process shift and squeeze
			int i;
			for(i = 0; i < iXSize; i++)
				vBuffer.SetAt(i, mNonlinearParams.GetAllParameter().CalcPoly(vXValues.GetAt(i) - mFitRangeLow) + mFitRangeLow);

			mBasisFunction->GetValues(vBuffer, vBasisFunctions);
		}

		/**
		* Fills the matrix with the basis function of the reference spectrum.
		* The basis function is just the values of the B-Spline at the shifted and squeezed X value.
		*
		* @param vXValues	The vector containing the X values to be contained in the matrix.
		* @param mA		The matrix which will receive the basis functions.
		* @param vB		The vector that hold the contant values of the normal equations.
		*/
		virtual void GetLinearAMatrix(CVector& vXValues, CMatrix& mA, CVector& vB)
		{
			const int iXSize = vXValues.GetSize();

			// if we have a fixed concentraction value, we only have to subtract the current function from the target vB
			if(mLinearParams.IsParamFixed(0))
			{
				CVector vBuffer(iXSize);

				// get the current function
				GetValues(vXValues, vBuffer);

				// subtract it from the B vector
				vB.Sub(vBuffer);

				return;
			}
			// for each X value given determine the basis function
			int i;
			for(i = 0; i < iXSize; i++)
				mA.SetAt(i, 0, mBasisFunction->GetValue(mNonlinearParams.GetAllParameter().CalcPoly(vXValues.GetAt(i) - mFitRangeLow) + mFitRangeLow));
		}

		/**
		* Returns the specified model parameter.
		* Any neccessary parameter mapping and scaling is done before the resulting value is returned.
		*
		* @param iParamID	The ID of the model parameter.
		*
		* @return	The value of the specified model parameter.
		*
		* @see {Parameter IDs}
		*/
		virtual TFitData GetModelParameter(EModelParam iParamID)
		{
			switch(iParamID)
			{
			case CONCENTRATION:
				// the concentration is given by the first linear parameter and the scaling factor of the B-Spline amplitude
				return mLinearParams.GetAllParameter().GetAt(0) / GetAmplitudeScale();
			case SHIFT:
				// shift is the first nonlinear parameter
				return mNonlinearParams.GetAllParameter().GetAt(0);
			case SQUEEZE:
				// squeeze if the second nonlinear parameter
				return mNonlinearParams.GetAllParameter().GetAt(1);
			}
			return 0;
		}

		virtual bool SetModelParameter(EModelParam iParamID, TFitData fValue)
		{
			switch(iParamID)
			{
			case CONCENTRATION:
				// the concentration is given by the first linear parameter and the scaling factor of the B-Spline amplitude
				return mLinearParams.SetParameter(0, fValue * GetAmplitudeScale());
			case SHIFT:
				// shift is the first nonlinear parameter
				return mNonlinearParams.SetParameter(0, fValue);
			case SQUEEZE:
				// squeeze if the second nonlinear parameter
				return mNonlinearParams.SetParameter(1, fValue);
			}
			return true;
		}

		virtual bool SetModelDefaultParameter(EModelParam iParamID, TFitData fValue)
		{
			switch(iParamID)
			{
			case CONCENTRATION:
				// the concentration is given by the first linear parameter and the scaling factor of the B-Spline amplitude
				return mLinearParams.SetAllDefaultParameter(0, fValue * GetAmplitudeScale());
			case SHIFT:
				// shift is the first nonlinear parameter
				return mNonlinearParams.SetAllDefaultParameter(0, fValue);
			case SQUEEZE:
				// squeeze if the second nonlinear parameter
				return mNonlinearParams.SetAllDefaultParameter(1, fValue);
			}
			return true;
		}

		/**
		* Returns the specified model parameter's error.
		* Any neccessary parameter mapping and scaling is done before the resulting value is returned.
		*
		* @param iParamID	The ID of the model parameter.
		*
		* @return	The value of the specified model parameter's error.
		*
		* @see {Parameter IDs}
		*/
		virtual TFitData GetModelParameterError(EModelParam iParamID)
		{
			switch(iParamID)
			{
			case CONCENTRATION:
				// the concentration is given by the first linear parameter and the scaling factor of the B-Spline amplitude
				return mLinearParams.GetAllError().GetAt(0) / GetAmplitudeScale();
			case SHIFT:
				// shift is the first nonlinear parameter
				return mNonlinearParams.GetAllError().GetAt(0);
			case SQUEEZE:
				// squeeze if the second nonlinear parameter
				return mNonlinearParams.GetAllError().GetAt(1);
			}
			return 0;
		}

		virtual bool SetParameterLimits(EModelParam iParamID, TFitData fLowLimit, TFitData fHighLimit, TFitData fPenaltyFactor)
		{
			switch(iParamID)
			{
			case CONCENTRATION:
				return SetLinearParameterLimits(0, fLowLimit, fHighLimit, fPenaltyFactor);
			case SHIFT:
				return SetNonlinearParameterLimits(0, fLowLimit, fHighLimit, fPenaltyFactor);
			case SQUEEZE:
				return SetNonlinearParameterLimits(1, fLowLimit, fHighLimit, fPenaltyFactor);
			}
			return false;
		}

		virtual bool ClearParameterLimits(EModelParam iParamID)
		{
			switch(iParamID)
			{
			case CONCENTRATION:
				return ClearLinearParameterLimits(0);
			case SHIFT:
				return ClearNonlinearParameterLimits(0);
			case SQUEEZE:
				return ClearNonlinearParameterLimits(1);
			}
			return false;
		}

		/**
		* Returns the amplitude scale before normalization.
		* Use this value to rescale the function back to its original amplitude scaling.
		*
		* @return	The maximum amplitude scale before normalization.
		*/
		TFitData GetAmplitudeScale()
		{
			return mAmplitudeScale;
		}

		/**
		* Enables or disables the automatic normalization.
		* If this parameter is set to true, a amplitude normalization within the specified
		* fit range is done when \Ref{SetData} is called.
		*
		* @param bNormalize	TRUE if automatic normalization is needed, FALSE otherwise. Default: FALSE.
		*/
		void SetNormalize(bool bNormalize)
		{
			mNormalize = bNormalize;
		}

		/**
		* Returns the current automatic normalization setting.
		*
		* @return	TRUE if automatic normalization is enabled, FALSE otherwise.
		*/
		bool GetNormalize()
		{
			return mNormalize;
		}

		/**
		 * Sets the fit range used during evaluation.
		 *
		 * @param vFitRange	A vector object that represents the fit range.
		 */
		virtual void SetFitRange(CVector& vFitRange)
		{
			mFitRange.Copy(vFitRange);

			mFitRangeLow = mFitRange.GetAt(0);
		}

		/**
		 * Set the basis function object to be used for spectral data evaluation.
		 *
		 * By default a Cubic Spline interpolation will be used to evaluate the given
		 * spectral data. However its possible to change the basis function 
		 * to any kind of evaluation function using this method.
		 *
		 * @param ifBasisFunction	The basis function object that should be used to evaluate the spectral data.
		 */
		void SetBasisFunction(IFunction& ifBasisFunction)
		{
			mBasisFunction = &ifBasisFunction;
		}

		/**
		 * Returns the basis function object used currently.
		 */
		IFunction& GetBasisFunction()
		{
			return *mBasisFunction;
		}

	private:
		/**
		 * Initializes the internal data structures.
		 */
		void InitObject()
		{
			mLinearParams.SetSize(1);
			mNonlinearParams.SetSize(2);

			// set the default concentration and squeeze value to one
			mLinearParams.SetAllDefaultParameter(0, 1);
			mNonlinearParams.SetAllDefaultParameter(1, 1);

			ResetLinearParameter();
			ResetNonlinearParameter();

			mFitRangeLow = 0;

			// not normalization done now.
			mAmplitudeScale = 1;

			SetNormalize(false);
		}

		/**
		* Contains the Cubic Spline interpolation that will by used as basis function by default.
		*/
		CCubicSplineFunction mBSpline;
		/**
		* Holds a reference to the function object that is used as basis function.
		*/
		IFunction* mBasisFunction;
		/**
		* Flag wheter a amplitude normalization should be performed on the spectrum data.
		*/
		bool mNormalize;
		/**
		* Contains the amplitude scale, if normalization was applied.
		*/
		TFitData mAmplitudeScale;
		TFitData mFitRangeLow;
	};
}

#pragma warning (pop)
#endif // !defined(AFX_DOASFUNCTIONS_H__F778D400_2C41_4092_8BA9_F789F5766579__INCLUDED_)
