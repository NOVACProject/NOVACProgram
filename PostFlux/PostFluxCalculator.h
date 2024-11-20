#pragma once

#include "../Meteorology/WindField.h"
#include <SpectralEvaluation/Spectra/SpectrumInfo.h>
#include "../Common/EvaluationLogFileHandler.h"

namespace PostFlux
{

class CPostFluxCalculator : public FileHandler::CEvaluationLogFileHandler
{
public:
    CPostFluxCalculator();
    ~CPostFluxCalculator();

    // ------------------- DATA -------------------------

    /** Information about the wind field */
    CWindField m_wind;

    /** The compass direction of the scanner */
    float  m_compass;

    /** The coneAngle (the opening angle for the scanning 'cone')
                This is 90 degrees for the old scanner and varying between
                45 and 90 degrees for the new Chalmers scanner. */
    float m_coneAngle;

    /** The tilt of the system */
    float   m_tilt;

    /** The resulting flux, in kg/s */
    double  m_flux;

    /** The gasfactor for the current gas, converts from ppm*m to mg/m^2 */
    double  m_gasFactor;

    /** The post-flux log */
    CString m_logFile;

    // ------------------ METHODS ----------------------- 

    /** Calculates the flux from the given scan with the given
        metrological conditions. */
    double CalculateFlux(int scanNr);

    /** Calculates the offset for the given scan. If anyone of the parameters
        'chi2Limit', 'intensAbove', or 'intensBelow' is supplied it will be
        used instead of the default parameter. */
    double CalculateOffset(int scanNr, const CString& specie, float chi2Limit = -1, float intensAbove = -1, float intensBelow = -1);

    /** Manually sets the offset for the given scan */
    void  SetOffset(double offset, int scanNr);

    /** Writes the header of the post-flux log file */
    void  WriteFluxLogHeader(int scanNr);

    /** Appends a result to the end of the post-flux log file */
    void  AppendToFluxLog(int scanNr);

};
}