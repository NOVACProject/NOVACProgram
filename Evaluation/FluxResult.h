#pragma once

#include <SpectralEvaluation/Flux/ScanFluxResult.h>
#include "../Meteorology/MeteorologySource.h"

/** The class <b>CFluxResult</b> is a generic class for storing the results
        from a flux-calculation of a scan. The class holds the values of all the
        parameters used in the calculation (except for the measurment itself) and
        the result of the measurement. All non-initialized variables are set to -999 */

namespace Evaluation
{
class CFluxResult : public novac::ScanFluxResult
{
public:
    CFluxResult();

    /** Clears the results */
    virtual void Clear() override;

    /** Copying */
    CFluxResult& operator=(const CFluxResult& other) = default;
    CFluxResult(const CFluxResult& other) = default;

    /** The source for the wind-direction */
    MET_SOURCE m_windDirectionSource;

    /** The source for the wind-speed */
    MET_SOURCE m_windSpeedSource;

    /** The source for the plume height */
    MET_SOURCE m_plumeHeightSource;
};
}