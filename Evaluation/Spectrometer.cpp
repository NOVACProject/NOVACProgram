#include "StdAfx.h"
#include "spectrometer.h"
#include "../SpectralEvaluation/Spectra/SpectrometerModel.h"

using namespace Evaluation;

CSpectrometer::CSpectrometer(void)
{
	for(int i = 0; i < MAX_FIT_WINDOWS; ++i)
		m_evaluator[i] = new CEvaluation();

	m_history = NULL;
	m_fitWindowNum = 0;
	m_channel = 0;
	m_gpsReadingsNum = 0;
}

CSpectrometer::~CSpectrometer(void)
{
  for(int i = 0; i < MAX_FIT_WINDOWS; ++i)
    delete m_evaluator[i];

	//if(this->m_history != NULL){
	//	delete m_history;
	//}
	m_history = NULL;
}

/** assignment operator */
CSpectrometer &CSpectrometer::operator=(const CSpectrometer &spec2){
	for(int i = 0; i < MAX_FIT_WINDOWS; ++i){
		delete m_evaluator[i];
		m_evaluator[i]           = new CEvaluation(*spec2.m_evaluator[i]);
	}

	this->m_settings            = spec2.m_settings;
	this->m_scanner             = spec2.m_scanner;
	this->m_logFileHandler      = spec2.m_logFileHandler;
	this->m_fitWindowNum        = spec2.m_fitWindowNum;
	this->m_channel							= spec2.m_channel;
	this->m_gpsReadingsNum			= spec2.m_gpsReadingsNum;
	return *this;
}

const CString &CSpectrometer::SerialNumber() const{
	return this->m_settings.serialNumber;
}

double CSpectrometer::GetMaxIntensity() const{
	return CSpectrometerModel::GetMaxIntensity(m_settings.model);
}

/** Adds the result from the supplied evaluation to the history
		of evaluations. */
void CSpectrometer::RememberResult(CScanResult &lastResult){
	std::string specie = "SO2";

	// 0. Some checking of reasonability
	if(lastResult.GetEvaluatedNum() <= 1)
		return;

	// 1. Check the mode of the measurement
	MEASUREMENT_MODE lastMode = lastResult.CheckMeasurementMode();

	// 2. Get the position of the plume centre, if there is any plume
	double centre1, centre2, plumeCompleteness, plumeEdge_low, plumeEdge_high;
	bool inPlume = lastResult.CalculatePlumeCentre(specie, centre1, centre2, plumeCompleteness, plumeEdge_low, plumeEdge_high);

	// 3. Get date and time when the scan started
	unsigned short date[3];
	lastResult.GetDate(0, date);
	const CDateTime *starttid = lastResult.GetStartTime(0);
	CDateTime tid = CDateTime(date[0], date[1], date[2], starttid->hour, starttid->minute, starttid->second);

	// 4. Get the maximum column value
	double maxColumn = lastResult.GetMaxColumn(specie);

	// 5. Append the results to the history
	if(m_history != NULL){
		if(MODE_FLUX == lastMode){
			m_history->AppendScanResult(lastResult, specie);
		}else if(MODE_COMPOSITION == lastMode){
			m_history->AppendCompMeasurement(tid.year, tid.month, tid.day, tid.hour, tid.minute, tid.second);
		}else if(MODE_WINDSPEED == lastMode){
			m_history->AppendWindMeasurement(tid.year, tid.month, tid.day, tid.hour, tid.minute, tid.second);
		}
	}
}