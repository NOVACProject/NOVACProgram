/**
* Contains a defintion of a constant function object.
*
* @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
* @version		1.0 @ 2002/02/01
*/
#if !defined(CONSTFUNCTION_H_020201)
#define CONSTFUNCTION_H_020201

#include "ParamFunction.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

namespace MathFit
{
	/**
	* This object represents a constant function.
	*
	* @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
	* @version		1.0 @ 2002/02/01
	*/
	class CConstFunction : public IParamFunction
	{
	public:
		/**
		* Creates the object and sets the constant value.
		*
		* @param fConst	The constant value of the function.
		*/
		CConstFunction(TFitData fConst)
		{
			// no parameters
			mLinearParams.SetSize(0);
			mNonlinearParams.SetSize(0);

			mConst = fConst;
		}

		/**
		* Returns the function's value given by the two multiplicative operands.
		*
		* \begin{verbatim}f(x)=c\end{verbatim}
		*
		* @param fXValue	The X data point.
		*
		* @return	The value of the function at the given data point.
		*/
		virtual TFitData GetValue(TFitData fXValue)
		{
			return mConst;
		}

		/**
		* Returns the first derivative of the function at the given data point.
		*
		* \begin{verbatim}f'(x)=0\end{verbatim}
		*
		* @param fXValue	The X data point at which the first derivation is needed.
		*
		* @return	The slope of the function at the given data point.
		*/
		virtual TFitData GetSlope(TFitData fXValue)
		{
			return 0;
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

	private:
		/**
		* Hold the constant function value.
		*/
		TFitData mConst;
	};
}
#endif
