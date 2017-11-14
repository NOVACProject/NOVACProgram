/**
 * Contains a fit object, that uses the least square and Levenberg-Marquardt fitting algorithms.
 *
 * @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
 * @version		1.0 @ 2002/03/17
 */
#if !defined(STANDARDFIT_H_020317)
#define STANDARDFIT_H_020317

#include "Fit.h"
#include "LeastSquareFit.h"
#include "LevenbergMarquardtFit.h"
#include "ParamFunction.h"

namespace MathFit
{
	class CStandardFit : public CFit
	{
	public:
		CStandardFit(IParamFunction& ipfModel) :
		  mLeastSquare(ipfModel),
			  mLevenberg(ipfModel),
			  CFit(ipfModel, mLeastSquare, mLevenberg)
		  {
		  }

	private:
		CLeastSquareFit mLeastSquare;
		CLevenbergMarquardtFit mLevenberg;
	};
}
#endif