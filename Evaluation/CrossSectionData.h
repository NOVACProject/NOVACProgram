#pragma once
#include <afxtempl.h>
#include "../FIT\Vector.h"

namespace Evaluation{
	/** 
		The <b>CReferenceData</b> class holds information on the cross sections
		used in the fitting procedure. Each instance of this class holds
		the information of one reference used.
		The references can be either differential or not.
	*/

	class CCrossSectionData
	{
	public:
		CCrossSectionData(void);
		~CCrossSectionData(void);
		
		/** Sets the cross-section information at the given pixel */
		void SetAt(int index, double wavel, double value);

		/** Sets the cross-section information to the values in the 
			supplied array */
		void Set(double *wavelength, double *crossSection, unsigned long pointNum);

		/** Sets the cross-section information to the values in the 
			supplied array */
		void Set(double *crossSection, unsigned long pointNum);

		/** Sets the cross-section information to the values in the 
			supplied array */
		void Set(MathFit::CVector &crossSection, unsigned long pointNum);

		/** Gets the cross section at the given pixel */
		double GetAt(unsigned int index) const;

		/** Gets the length of this cross section */
		unsigned long GetSize() const;

		/** Gets the wavelength at the given pixel */
		double GetWavelengthAt(unsigned int index) const;

		/** Reads the cross section from a file 
			@return 0 on success
			@return non-zero value on fail */
		int ReadCrossSectionFile(const CString &fileName);

		/** Assignment operator*/
		CCrossSectionData &operator=(const CCrossSectionData &xs2);

	private:
		/** An array containing the wavelength information.*/
		CArray <double, double &> m_waveLength;
		
		/** An array containing the actual cross-section */
		CArray <double, double &> m_crossSection;
		
		/** The length of the cross section */
		unsigned long m_length;
	};
}
