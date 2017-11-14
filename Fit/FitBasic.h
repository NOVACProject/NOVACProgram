/**
 * Copyright and Legacy Notice
 *
 * Copyright (c) 2001
 * \\\URL[Interdisciplinary Center for Scientific Computing (IWR)]{http://iwr.uni-heidelberg.de} and
 * \URL[Institute of Environmental Physics (IUP)]{http://iup.uni-heidelberg.de},
 * \URL[University of Heidelberg]{http://www.uni-heidelberg.de}
 * \hline
 *
 * Permission to use, copy, and this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation.  IWR makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 * 
 * Where possible, you are encouraged to follow the GNU General Public
 * License, or at least the spirit of the license, for the distribution and
 * licensing of this software and any derived works.  See
 * \URL[http://www.gnu.org/copyleft/gpl.html]{http://www.gnu.org/copyleft/gpl.html}.
 *
 * @author		\item \URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
 * @author		\item \URL[Silke Humbert]{mailto:silke.humbert@iup.uni-heidelberg.de} @ \URL[IUP, Satellite Data Group]{http://giger.iup.uni-heidelberg.de}
 * @version		1.0 @ 2001/09/09
 */

/**
 * Example
 *
 * Here follows an example about how to use the function and fit objects.
 * \hline
 \begin{verbatim}
// expected variables:
// vX:		Vector that contains the X axis information for all spectra
// vXSec:	Vector that contains the X values of the fitting range
// vY:		Vector that contains the Y values of the measurment stepctrum
// vYData[]:Vectors that contain the Y values of the some reference 
//			spectra that will be fitted against the measurement spectrum.

// first create a reference spectrum object that receives the measurement data
CDiscreteFunction dataTarget;

// set the data of the measurement spectrum
dataTarget.SetData(vX, vY);

////////////////////////////////////////////////////////////////////////////
// now start to create the model function.
// our model consists of the sum of all reference spectra and an additional polynomial

// create the sum object that adds together all given operands
CSimpleDOASFunction cRefSum;

// create the array with the reference spectra objects.
CReferenceSpectrumFunction ref[iNumSpec];

// add the reference spectra to our model function
for(i = 0; i < iNumSpec; i++)
{
	// set the appropriate fit range
	ref[i].SetFitRange(vXSec.GetAt(0), vXSec.GetAt(vXSec.GetSize() - 1));

	// we want an amplitude normalization of the reference spectrum
	ref[i].SetNormalize(true);

	// set the data of the reference spectrum
	ref[i].SetData(vX, vYData[i]);

	// fix the squeeze parameter of the reference spectrum to one
	ref[i].FixParameter(CReferenceSpectrum::SQUEEZE, 1);

	// the shift parameter of the 2nd, 3rd and 4th reference spectrum
	// should be linked to the shift parameter of the 1st reference spectrum
	if(i > 0 && i != 3)
		ref[0].LinkParameter(CReferenceSpectrum::SHIFT, ref[i], CReferenceSpectrum::SHIFT);

	// add the reference spectrum to the model function object
	cRefSum.AddReference(ref[i]);
}

// create a polynomial of degree 2
CPolynomialFunction cPoly(2);

// add the polynomial to the model function object
cRefSum.AddReference(cPoly);

// create the minimization function object. In this case we use a standard metric function
// object. This object just builds the difference between the given measurement data and
// the model function object. The fit will try to minimize the square sum of all values of 
// this object within the fit range.
CStandardMetricFunction cDiff(dataTarget, cRefSum);

// create the fit object and set the model function as minimization function
CFit cFirstFit(cDiff);

// set the fit range
cFirstFit.SetFitRange(vXSec);

// try to do the fit.
try
{
	// do the pre-processing of the fit object
	cFirstFit.PrepareMinimize();

	// limit the number of steps in the nonlinear fit
	cFirstFit.GetNonlinearMinimizer().SetMaxFitSteps(5000);

	// try to minimize the model function
	if(!cFirstFit.Minimize())
		::cout << "Fit failed!" << endl;

	// do the post processing of the fit (eg. calculate the parameter errors)
	cFirstFit.FinishMinimize();

	// now print the parameter results of each reference spectrum
	for(i = 0; i < iNumSpec; i++)
	{
		szOut.Format("ref %d : c:%.2e +/-%.2e shift:%.2g +/-%.2g squeeze:%.2g +/-%.2g",
					  i + 1,
					  ref[i].GetModelParameter(CReferenceSpectrum::CONCENTRATION),
					  ref[i].GetModelParameterError(CReferenceSpectrum::CONCENTRATION),
					  ref[i].GetModelParameter(CReferenceSpectrum::SHIFT),
					  ref[i].GetModelParameterError(CReferenceSpectrum::SHIFT),
					  ref[i].GetModelParameter(CReferenceSpectrum::SQUEEZE),
					  ref[i].GetModelParameterError(CReferenceSpectrum::SQUEEZE));

		::cout << szOut << endl;
	}
}
// if an unrecoverable error occured during fitting, we catch it here
catch(CFitException e)
{
	// on error, print the apropriate message
	e.ReportError();
	::cout << "Failed: " << ++iFalseCount << endl;
	::cout << "Steps: " << cFirstFit.GetNonlinearMinimizer().GetFitSteps() << " - Chi: " << cFirstFit.GetNonlinearMinimizer().GetChiSquare() << endl;
}
 \end{verbatim}
 * \hline
 */

/**
 * Important Notes
 *
 * \hline
 * Ensure your projects fulfills the following constrains:
 *
 * \item	Make sure any fixing or linking of parameters is done prior any further usage/inheritage
 *			of an object. Otherwise the number of free parameters may not be determined correctly.
 * \item	Be careful with the link option of the function parameters. Due to the nature of the hierarchical
 *			structure of a model function linked parameter may not be regornized during caculating
 *			the derivations of some sub function objects.
 * \item	The fit needs the \Ref{IDataSet::mXData} member of the model function object in
 *			order to have the support data range for the fit. Without the \Ref{IDataSet::mXData} member set the
 *			fit {\em does not know} at what X values to determine the model function. So always make sure
 *			the model function object used for fitting has the \Ref{IDataSet::mXData} member contains at least
 *			twice the number of free model parameters data values that reside within the fit range!. The
 *			\Ref{CStandardMetric} object sets its \Ref{IDataSet::mXData} member automatically to the fit range of
 *			the given measurement data object. You can also set the \Ref{IDataSet::mXData} meber using the \Ref{IDataSet::SetXData}
 *			method of the \Ref{IDataSet} interface.
 * \hline
 */

/**
 * Contains the all the neccessary defines needed by the fit.
 * You can select the data precision and system dependend includes are loaded here.
 *
 * @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
 * @version		1.0 @ 2001/09/20
 */
#if !defined(AFX_FITBASIC_H__127D9ACD_261F_47B2_B62E_796F5487ABCD__INCLUDED_)
#define AFX_FITBASIC_H__127D9ACD_261F_47B2_B62E_796F5487ABCD__INCLUDED_

#include "FitException.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/**
* Check for system dependend ASSERT
*/
#if !defined(WIN32) || !defined(ASSERT)
#include <assert.h>
#define ASSERT	assert
#endif

namespace MathFit
{
	// enable to use single precission data. if disabled double precission is used.
//#define MATHFIT_FITDATAFLOAT

	/**
	* Here you can define wheter to use double precision data elements or single floats
	*/
#if defined(MATHFIT_FITDATAFLOAT)
	typedef float TFitData;
#else
	typedef double TFitData;
#endif

	/**
	* Defines the minimum value for a nonlinear parameter. 
	* Anything below this value is set to this value.
	*/
#define MATHFIT_NEARLYZERO	(TFitData)1e-25
	/**
	* The delta used for calculating the derivation.
	*/
#define MATHFIT_DELTA		(TFitData)1e-3

#define MATHFIT_PI			3.14159265358979323846

	// enable to use LU decomposition. if disabled Gauss Jordan Elimination is used
//#define MATHFIT_USELUDECOMPOSITION
//#define MATHFIT_IMPROVEEQSSOLVE

	class CAssertFailedException : public CFitException
	{
	public:
		CAssertFailedException(int iLine, const char* szModule, const char* szName, const char* szAssert) : CFitException(iLine, szModule, szName, "Assertion failed!\n\nASSERT(%s)", szAssert) {}
	};

	/*@@
	* Defined wheter an implementation of the ASSERT macro provided by the MathFit-library should be used, 
	* or if the standard ASSERT macro will be used.
	*/
#undef MATHFIT_USEMATHFIT_ASSERT

// check for debug session
#if defined(_DEBUG)
#if defined(MATHFIT_USEMATHFIT_ASSERT)
#define MATHFIT_ASSERT(a)	{ if(!(a)) throw(CAssertFailedException(__LINE__, __FILE__, "CAssertFailedException", #a)); }
#else
#define MATHFIT_ASSERT(a)	ASSERT(a)
#endif // defined(MATHFIT_USEMATHFIT_ASSERT)
#else
#define MATHFIT_ASSERT(a)
#endif // defined(_DEBUG)
}

#endif // !defined(AFX_FITBASIC_H__127D9ACD_261F_47B2_B62E_796F5487ABCD__INCLUDED_)
