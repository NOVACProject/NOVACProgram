#include "StdAfx.h"
#include "geometryresult.h"

using namespace Geometry;

CGeometryResult::CGeometryResult(void)
{
	m_date							= -1;
	m_startTime					= -1;
	m_plumeHeight				= 0.0;
	m_plumeHeightError	= 0.0;
	m_windDirection			= 0.0;
	m_windDirectionError= 0.0;
}

CGeometryResult::~CGeometryResult(void)
{
}

/** Assignement operator */
CGeometryResult &CGeometryResult::operator=(const CGeometryResult &gr){
	m_date								= gr.m_date;
	m_startTime						= gr.m_startTime;
	m_plumeHeight					= gr.m_plumeHeight;
	m_plumeHeightError		= gr.m_plumeHeightError;
	m_windDirection				= gr.m_windDirection;
	m_windDirectionError	= gr.m_windDirectionError;

	return *this;
}
