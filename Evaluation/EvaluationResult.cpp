#include "StdAfx.h"
#include "evaluationresult.h"

using namespace Evaluation;

CEvaluationResult::CEvaluationResult(void)
{
  this->m_chiSquare = 0.0f;
  this->m_delta = 0.0f;
  this->m_speciesNum = 0;
  this->m_stepNum = 0;
  memset(m_polynomial, 0, 5*sizeof(float));
  m_evaluationStatus = 0;

  m_ref.SetSize(1, 2);
}

CEvaluationResult::CEvaluationResult(const CEvaluationResult &b)
{
  int nRef = (int)b.m_ref.GetSize();
  m_ref.SetSize(nRef);
  for(long i = 0; i < nRef; ++i){
    CReferenceFitResult ref;
    ref.m_column         = b.m_ref[i].m_column;
    ref.m_columnError    = b.m_ref[i].m_columnError;
    ref.m_shift          = b.m_ref[i].m_shift;
    ref.m_shiftError     = b.m_ref[i].m_shiftError;
    ref.m_squeeze        = b.m_ref[i].m_squeeze;
    ref.m_squeezeError   = b.m_ref[i].m_squeezeError;
    ref.m_specieName.Format("%s", b.m_ref[i].m_specieName);
    this->m_ref.SetAt(i, ref);
  }
  memcpy(this->m_polynomial, b.m_polynomial, 5*sizeof(float));

  this->m_chiSquare = b.m_chiSquare;
  this->m_delta			= b.m_delta;
  this->m_stepNum		= b.m_stepNum;
	this->m_evaluationStatus = b.m_evaluationStatus;
	this->m_speciesNum = b.m_speciesNum;
}

CEvaluationResult::~CEvaluationResult(void)
{
}

// makes this a copy of 'b'
CEvaluationResult &CEvaluationResult::operator =(const CEvaluationResult &b){
  int nRef = (int)b.m_ref.GetSize();
  m_ref.SetSize(nRef);
  for(long i = 0; i < nRef; ++i){
    CReferenceFitResult ref;
    ref.m_column         = b.m_ref[i].m_column;
    ref.m_columnError    = b.m_ref[i].m_columnError;
    ref.m_shift          = b.m_ref[i].m_shift;
    ref.m_shiftError     = b.m_ref[i].m_shiftError;
    ref.m_squeeze        = b.m_ref[i].m_squeeze;
    ref.m_squeezeError   = b.m_ref[i].m_squeezeError;
    ref.m_specieName.Format("%s", b.m_ref[i].m_specieName);
    this->m_ref.SetAt(i, ref);
  }
  memcpy(this->m_polynomial, b.m_polynomial, 5*sizeof(float));

  this->m_chiSquare = b.m_chiSquare;
  this->m_delta = b.m_delta;
  this->m_stepNum = b.m_stepNum;

  m_evaluationStatus = b.m_evaluationStatus;
  m_speciesNum = b.m_speciesNum;

  return *this;
}

RETURN_CODE CEvaluationResult::InsertSpecie(const CString &name){
  CReferenceFitResult ref;
  ref.m_specieName.Format(name);
  m_ref.SetAtGrow(m_speciesNum, ref);
  ++m_speciesNum;
  return SUCCESS;
}

bool CEvaluationResult::CheckGoodnessOfFit(const CSpectrumInfo& info, float chi2Limit, float upperLimit, float lowerLimit){
  // assume that this is an ok evaluation
  m_evaluationStatus &= ~MARK_BAD_EVALUATION; 

	// The maximum intensity for one spectrum (# bits in the ADC)
	double maxInt			= CSpectrometerModel::GetMaxIntensity(info.m_specModel);

	// The maximum saturation-level in the fit-region
	double fitSaturation = 0.0;
	if(info.m_fitIntensity <= 1.0){
		fitSaturation = info.m_fitIntensity;
	}else{
		if(info.m_numSpec > 0)
			fitSaturation = info.m_fitIntensity / (maxInt * info.m_numSpec);
		else{
			int numSpec = floor(info.m_peakIntensity / maxInt); // a guess for the number of co-adds
			fitSaturation = info.m_fitIntensity / (maxInt * numSpec);
		}
	}

	// The offset of the spectrum
	double offset = 0.0;
	if(info.m_numSpec > 0){
		offset	= info.m_offset / (maxInt * info.m_numSpec);
	}else{
		int numSpec = floor(info.m_peakIntensity / maxInt); // a guess for the number of co-adds
		offset = info.m_offset / (maxInt * numSpec);
	}

  // first check the intensity of the spectrum in the fit region
  if(upperLimit > -1){
    if(fitSaturation > upperLimit)
      m_evaluationStatus |= MARK_BAD_EVALUATION;
  }else{
    if(fitSaturation > 0.99)
      m_evaluationStatus |= MARK_BAD_EVALUATION;
  }

  // first check the intensity of the spectrum in the fit region
  if(lowerLimit > -1){
    if(fitSaturation < lowerLimit)
      m_evaluationStatus |= MARK_BAD_EVALUATION;
  }else{
    if((fitSaturation - offset) < 0.025)
      m_evaluationStatus |= MARK_BAD_EVALUATION;
  }

	// then check the chi2 of the fit
  if(chi2Limit > -1){
		if(m_chiSquare > chi2Limit)
      m_evaluationStatus |= MARK_BAD_EVALUATION;
  }else{
    if(m_chiSquare > 0.9)
      m_evaluationStatus |= MARK_BAD_EVALUATION;
  }

  return (m_evaluationStatus & MARK_BAD_EVALUATION);
}

/** Marks the current spectrum with the supplied mark_flag.
    Mark flag must be MARK_BAD_EVALUATION, or MARK_DELETED
    @return SUCCESS on success. */
RETURN_CODE  CEvaluationResult::MarkAs(int MARK_FLAG){
  // check the flag
  switch(MARK_FLAG){
    case MARK_BAD_EVALUATION: break;
    case MARK_DELETED: break;
    default: return FAIL;
  }

  // set the corresponding bit
  m_evaluationStatus |= MARK_FLAG;

  return SUCCESS;
}

/** Removes the current mark from the desired spectrum
    Mark flag must be MARK_BAD_EVALUATION, or MARK_DELETED
    @return SUCCESS on success. */
RETURN_CODE  CEvaluationResult::RemoveMark(int MARK_FLAG){
  // check the flag
  switch(MARK_FLAG){
    case MARK_BAD_EVALUATION: break;
    case MARK_DELETED: break;
    default: return FAIL;
  }

  // remove the corresponding bit
  m_evaluationStatus &= ~MARK_FLAG;

  return SUCCESS;
}
