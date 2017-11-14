/**
 * Contains the simple DOAS function object.
 *
 * @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
 * @version		1.0 @ 2001/09/21
 */
#if !defined(SIMPLEDOAS_H_011206)
#define SIMPLEDOAS_H_011206

#include "SumFunction.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

namespace MathFit
{
	/**
	* Represents the simple DOAS model.
	* The simple DOAS model just consists of the sum of all references given. Thus you can add any reference object and this
	* object will build the sum and adjust the neccessary parameter offset automatically.
	*
	* @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
	* @version		1.0 @ 2001/09/23
	*/
	class CSimpleDOASFunction : public CSumFunction
	{
	public:
		/**
		* Creates an empty reference object.
		*/
		CSimpleDOASFunction()
		{
		}

		/**
		* Adds the given reference to the current model.
		* If neccessary, the internal buffer are adapted to hold the complete set of references.
		*
		* @param ipfRef	The reference function object to be added.
		*/
		virtual void AddReference(IParamFunction& ipfRef)
		{
			AddOperand(ipfRef);
		}

		/**
		* Removes the given reference from the model.
		* If neccessary, the buffer size of adjusted, if there is plenty of space left
		*
		* @param ipfRef	The reference function object to be removed.
		*/
		virtual void RemoveReference(IParamFunction& ipfRef)
		{
			RemoveOperand(ipfRef);
		}
	};
}
#endif
