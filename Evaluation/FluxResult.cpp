#include "StdAfx.h"
#include "FluxResult.h"

using namespace Evaluation;
using namespace novac;

CFluxResult::CFluxResult()
    : ScanFluxResult()
{
    m_windDirectionSource = MET_DEFAULT;
    m_windSpeedSource = MET_DEFAULT;
    m_plumeHeightSource = MET_DEFAULT;
}

void CFluxResult::Clear()
{
    m_flux = 0.0;
    m_fluxOk = true;
    m_windDirection = -999.0;
    m_windSpeed = -999.0;
    m_plumeHeight = -999.0;

    m_coneAngle = -999.0;
    m_tilt = -999.0;
    m_compass = -999.0;
    m_volcano = -1;

    m_windDirectionSource = MET_DEFAULT;
    m_windSpeedSource = MET_DEFAULT;
    m_plumeHeightSource = MET_DEFAULT;
}


