#include "StdAfx.h"
#include "fluxresult.h"

using namespace Evaluation;

CFluxResult::CFluxResult(void)
{
	this->Clear();
}

void CFluxResult::Clear(){
	m_flux          = 0.0;
	m_fluxOk        = true;
	m_windDirection = -999.0;
	m_windSpeed     = -999.0;
	m_plumeHeight   = -999.0;

	m_windDirectionSource = MET_DEFAULT;
	m_windSpeedSource     = MET_DEFAULT;
	m_plumeHeightSource   = MET_DEFAULT;

	m_coneAngle = -999.0;
	m_tilt      = -999.0;
	m_compass   = -999.0;
	m_volcano   = -1;
}

CFluxResult::~CFluxResult(void)
{
}

/** Assignment operator */
CFluxResult &CFluxResult::operator=(const CFluxResult &res){
	m_flux          = res.m_flux;
	m_fluxOk        = res.m_fluxOk;
	m_windDirection = res.m_windDirection;
	m_windSpeed     = res.m_windSpeed;
	m_plumeHeight   = res.m_plumeHeight;

	m_windDirectionSource = res.m_windDirectionSource;
	m_windSpeedSource     = res.m_windSpeedSource;
	m_plumeHeightSource   = res.m_plumeHeightSource;

	m_coneAngle    = res.m_coneAngle;
	m_tilt         = res.m_tilt;
	m_compass      = res.m_compass;
	m_volcano      = res.m_volcano;

	m_startTime    = res.m_startTime;

	return *this;
}

