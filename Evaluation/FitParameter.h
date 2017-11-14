#pragma once

namespace Evaluation
{
    /** constants for selection of fit-parameters*/
    const enum FIT_PARAMETER{ 
        COLUMN, 
        COLUMN_ERROR, 
        SHIFT, 
        SHIFT_ERROR, 
        SQUEEZE, 
        SQUEEZE_ERROR,
        DELTA};

  /** <b>CFitParameter</b> is a class to describe the parameters that are used in fitting
      of a spectrum. */
  class CFitParameter
  {
  public:
    CFitParameter(void);
    ~CFitParameter(void);

  };
}