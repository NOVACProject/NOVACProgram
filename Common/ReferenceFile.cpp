#include "StdAfx.h"
#include "referencefile.h"

#include "../Evaluation/Evaluation.h"

using namespace Evaluation;

CReferenceFile::CReferenceFile(void)
{
	this->m_path.Format("");
	this->m_specieName.Format("");
	this->m_columnOption = SHIFT_FREE;
	this->m_columnValue = 0.0;
	this->m_columnMaxValue = 0.0;
	this->m_shiftOption = SHIFT_FIX;
	this->m_shiftValue = 0.0;
	this->m_shiftMaxValue = 0.0;
	this->m_squeezeOption = SHIFT_FIX;
	this->m_squeezeValue = 1.0;
	this->m_squeezeMaxValue = 1.0;
}
  
CReferenceFile::~CReferenceFile(void)
{
}

/** assignment operator */
CReferenceFile &CReferenceFile::operator=(const CReferenceFile &ref2){
	this->m_path.Format("%s", ref2.m_path);
	this->m_specieName.Format("%s", ref2.m_specieName);

	this->m_columnOption		= ref2.m_columnOption;
	this->m_columnValue			= ref2.m_columnValue;
	this->m_columnMaxValue	= ref2.m_columnMaxValue;
	this->m_shiftOption			= ref2.m_shiftOption;
	this->m_shiftValue			= ref2.m_shiftValue;
	this->m_shiftMaxValue		= ref2.m_shiftMaxValue;
	this->m_squeezeOption		= ref2.m_squeezeOption;
	this->m_squeezeValue		= ref2.m_squeezeValue;
	this->m_squeezeMaxValue	= ref2.m_squeezeMaxValue;

	return *this;
}

/** Setting the column */
void CReferenceFile::SetColumn(SHIFT_TYPE option, double value, double value2){
	this->m_columnOption = option;
	this->m_columnValue  = value;

	if(SHIFT_LIMIT == option)
		this->m_columnMaxValue = value2;
}

/** Setting the shift */
void CReferenceFile::SetShift(SHIFT_TYPE option, double value, double value2){
	this->m_shiftOption = option;
	this->m_shiftValue  = value;

	if(SHIFT_LIMIT == option)
		this->m_shiftMaxValue = value2;
}

/** Setting the squeeze */
void CReferenceFile::SetSqueeze(SHIFT_TYPE option, double value, double value2){
	this->m_squeezeOption = option;
	this->m_squeezeValue  = value;

	if(SHIFT_LIMIT == option)
		this->m_squeezeMaxValue = value2;
}
