#pragma once

namespace Evaluation
{
	/** CReferenceFitResult is a class to store the evaluated parameters of a 
		reference file	from evaluating a spectrum */
	class CReferenceFitResult
	{
	public:
		CReferenceFitResult(void);
		~CReferenceFitResult(void);

		/** The resulting column */
		double m_column;

		/** The error in the resulting column */
		double m_columnError;

		/** The shift that was applied to the reference in the evaluation */
		double m_shift;

		/** The uncertainty in the applied shift */
		double m_shiftError;

		/** The squeeze that was applied to the reference in the evaluation */
		double m_squeeze;

		/** The uncertainty in the applied squeeze */
		double m_squeezeError;

		/** The name of the specie that the reference identifies */
		CString m_specieName;
	};

}