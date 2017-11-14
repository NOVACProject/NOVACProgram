/**
* ParamFunction.h
*
* Contains the parametric function interface of a concolution core.
*
* @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
* @version		1.0 @ 2002/05/23
*/
#if !defined(CONVOLUTIONCOREFUNCTION_H_020523)
#define CONVOLUTIONCOREFUNCTION_H_020523

#include "ParameterVector.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

namespace MathFit
{
	/**
	* Interface to represent a parametric convolution core function. 
	* A convolution core can use the current core center to adapt its function values to the core's offset during convolution.
	* The interface is derivated from \Ref{IParamFunction} so all the function handling methods are available two.
	*
	* @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
	* @version		1.0 @ 2002/05/23
	*/
	class IConvolutionCoreFunction : public IParamFunction
	{
	public:
		/**
		* Creates an empty object
		*/
		IConvolutionCoreFunction()
		{
			mXCenter = 0;
		}

		/**
		* Constructs the object and stores the given data into the internal buffers.
		*
		* @param vXValues		A vector object containing the X values of the data set.
		* @param vYValues		A vector object containing the Y values of the data set.
		*
		* @see	IFunction constructor
		*/
		IConvolutionCoreFunction(CVector& vXValues, CVector& vYValues) : IParamFunction(vXValues, vYValues)
		{
			mXCenter = 0;
		}

		/**
		* Constructs the object and stores the given data into the internal buffers.
		*
		* @param vXValues		A vector object containing the X values of the data set.
		* @param vYValues		A vector object containing the Y values of the data set.
		* @param vError		A vector object containing the error values of the data set.
		*
		* @see	IFunction constructor
		*/
		IConvolutionCoreFunction(CVector& vXValues, CVector& vYValues, CVector& vError) : IParamFunction(vXValues, vYValues, vError)
		{
			mXCenter = 0;
		}

		virtual void SetCoreCenter(TFitData fCenter)
		{
			mXCenter = fCenter;
		}

		virtual TFitData GetCoreCenter()
		{
			return mXCenter;
		}

		virtual TFitData GetCoreLowBound()
		{
			return 0;
		}

		virtual TFitData GetCoreHighBound()
		{
			return 0;
		}

	protected:
		TFitData mXCenter;
	};
}
#endif // !defined(AFX_IFUNCTION_H__7852C2C7_0389_41DB_B169_3C30252ADD47__INCLUDED_)
