#include "StdAfx.h"
#include "evaluateddatastorage.h"
#include "Configuration/Configuration.h"
#include "VolcanoInfo.h"
#include "UserSettings.h"
#include <SpectralEvaluation/StringUtils.h>
#include <SpectralEvaluation/Spectra/SpectrometerModel.h>

extern CConfigurationSetting g_settings;   // <-- The settings
extern CVolcanoInfo g_volcanoes;           // <-- The global database of volcanoes
extern CUserSettings g_userSettings;       // <-- The users preferences

// ------------------------ CSCANDATA ------------------------------
CEvaluatedDataStorage::CScanData::CScanData(){
	m_flux		= 0.0;
	m_fluxOk	= true;
	m_time		= 0.0;
	m_date		= 0;
	m_battery	= 0.0;
	m_temp		= 0.0;
	m_expTime	= 0;
}

CEvaluatedDataStorage::CScanData::~CScanData(){
}

CEvaluatedDataStorage::CScanData &CEvaluatedDataStorage::CScanData::operator=(const CEvaluatedDataStorage::CScanData &sd){
	this->m_flux    = sd.m_flux;
	this->m_fluxOk  = sd.m_fluxOk;
	this->m_time    = sd.m_time;
	this->m_date    = sd.m_date;
	this->m_battery = sd.m_battery;
	this->m_temp    = sd.m_temp;
	this->m_expTime = sd.m_expTime;

	return *this;
}

// ------------------------ CSPECTRUMDATA ------------------------------

CEvaluatedDataStorage::CSpectrumData::CSpectrumData(){
	m_time           = 0;
	m_column         = 0.0;
	m_columnError    = 0.0;
	m_peakSaturation = 0.0;
	m_fitSaturation  = 0.0;
	m_angle          = 0.0;
	m_isBadFit       = false;
}

CEvaluatedDataStorage::CSpectrumData::~CSpectrumData(){
}

CEvaluatedDataStorage::CSpectrumData &CEvaluatedDataStorage::CSpectrumData::operator=(const CEvaluatedDataStorage::CSpectrumData &sd){
	m_time           = sd.m_time;
	m_column         = sd.m_column;
	m_columnError    = sd.m_columnError;
	m_peakSaturation = sd.m_peakSaturation;
	m_fitSaturation  = sd.m_fitSaturation;
	m_angle          = sd.m_angle;
	m_isBadFit       = sd.m_isBadFit;

	return *this;
}

// ------------------------ CSCANDATA ------------------------------
CEvaluatedDataStorage::CWindMeasData::CWindMeasData(){
	m_scannerIndex = 0;
	m_time         = 0;
	m_date         = 0;
	m_duration     = 0;
	m_correlation  = 0.0;
	m_windSpeed    = 0.0;
	m_windSpeedErr = 0.0;
}

CEvaluatedDataStorage::CWindMeasData::~CWindMeasData(){
}

CEvaluatedDataStorage::CWindMeasData &CEvaluatedDataStorage::CWindMeasData::operator=(const CEvaluatedDataStorage::CWindMeasData &wd){
	this->m_scannerIndex = wd.m_scannerIndex;
	this->m_time         = wd.m_time;
	this->m_date         = wd.m_date;
	this->m_duration     = wd.m_duration;
	this->m_correlation  = wd.m_correlation;
	this->m_windSpeed    = wd.m_windSpeed;
	this->m_windSpeedErr = wd.m_windSpeedErr;

	return *this;
}

CEvaluatedDataStorage::CEvaluatedDataStorage(void)
{
	memset(m_positionsNum, 0, MAX_NUMBER_OF_SCANNING_INSTRUMENTS * sizeof(int));
	memset(m_offset, 0, MAX_NUMBER_OF_SCANNING_INSTRUMENTS * sizeof(double));
	memset(m_plumeCentre, 0, MAX_NUMBER_OF_SCANNING_INSTRUMENTS * sizeof(double));

	memset(m_temperatureRange[0], 999, MAX_NUMBER_OF_SCANNING_INSTRUMENTS * sizeof(double));
	memset(m_temperatureRange[1], -999, MAX_NUMBER_OF_SCANNING_INSTRUMENTS * sizeof(double));

	memset(m_fluxIndex, 0, MAX_NUMBER_OF_SCANNING_INSTRUMENTS * sizeof(int));
	memset(m_specIndex, 0, MAX_NUMBER_OF_SCANNING_INSTRUMENTS * sizeof(int));

	for(int i = 0; i < MAX_NUMBER_OF_SCANNING_INSTRUMENTS; ++i)
		this->m_serials[i].Format("");

	m_serialNum = 0;
}

CEvaluatedDataStorage::~CEvaluatedDataStorage(void)
{
	m_windData.RemoveAll();
}

int CEvaluatedDataStorage::AddData(const CString &serial, Evaluation::CScanResult *result) {
	CDateTime tid;
	Common common;

	// get the scanner index
	int scannerIndex = GetScannerIndex(serial);

	if ((scannerIndex < 0)) {
		if (m_serialNum < MAX_NUMBER_OF_SCANNING_INSTRUMENTS) {
			// if the scanner is not in the list then insert it
			scannerIndex = m_serialNum;
			m_serials[m_serialNum].Format("%s", (LPCSTR)serial);

			// find the serial-number in the global configuration, to find the spectrometer-model
			bool found = false;
			for (unsigned int k = 0; k < g_settings.scannerNum; ++k) {
				if (found)	break;

				for (unsigned int j = 0; j < g_settings.scanner[k].specNum; ++j) {
					if (Equals(g_settings.scanner[k].spec[j].serialNumber, serial)) {
						m_models[m_serialNum] = g_settings.scanner[k].spec[j].modelName;

						// Get the distance to GMT...
						m_hoursToGMT[m_serialNum] = 0;
						CString volcano;
						volcano.Format(g_settings.scanner[k].volcano);
						for (unsigned int it = 0; it < g_volcanoes.m_volcanoNum; ++it) {
							if (Equals(volcano, g_volcanoes.m_name[it])) {
								m_hoursToGMT[m_serialNum] = g_volcanoes.m_hoursToGMT[it];
								break;
							}
						}
						found = true;
						break;
					}
				}
			}
			if (!found)
				m_models[m_serialNum] = "";

			++m_serialNum;
		}
		else {
			// could not insert the serial number
			return -1;
		}
	}

	if (result == NULL) {
		return 0;
	}

	// add the flux result, if the flux comes from a measurement today...
	if (MODE_FLUX == result->CheckMeasurementMode()) {
		// Get scan end time
		CDateTime scanTime;
		result->GetStopTime(0, scanTime);
		if (common.Epoch() - common.Epoch(scanTime) <= 86400) {
			AppendFluxResult(scannerIndex, scanTime, result->GetFlux(), result->IsFluxOk(), result->GetBatteryVoltage(), result->GetTemperature(), result->GetSkySpectrumInfo().m_exposureTime);
		}
	}
	if (MODE_WINDSPEED == result->CheckMeasurementMode()) {
		result->GetStopTime(0, m_scanTime[scannerIndex]);
	}

	// Find the maximum intensity for this spectrometer
	double maxIntensity = GetDynamicRange(serial);
	if (fabs(maxIntensity) < 1e-5) {
		maxIntensity = 1;
	}

	// add the evaluated column values and their corresponding elevation angle and saturation-level
	int nIgnored = 0;
	for(unsigned long i = 0; i < result->GetEvaluatedNum(); ++i){
		
		// Check if this is a dark measurement, if so then don't include it...
		// 1. Clean the spectrum name from special characters...
        std::string spectrumName = CleanString(result->GetSpectrumInfo(i).m_name);
		Trim(spectrumName, " \t");
		if(fabs(result->GetScanAngle(i)) - 180.0 < 1e-3 && (EqualsIgnoringCase(spectrumName, "offset") || EqualsIgnoringCase(spectrumName, "dark_cur") || EqualsIgnoringCase(spectrumName, "dark"))){
			++nIgnored;
			continue;
		}

		int nSpec = max(1, result->GetSpecNum(i));

		// All seems ok
		int time = 0;
		if (SUCCESS == result->GetStartTime(i, tid)) {
			time = tid.hour * 3600 + tid.minute * 60 + tid.second;
		}
		double column = result->GetColumn(i, 0);
		double columnError = result->GetColumnError(i, 0);
		double angle = result->GetScanAngle(i);
		double peakSaturation = result->GetPeakIntensity(i) / maxIntensity / nSpec;
		double fitSaturation = result->GetFitIntensity(i) / maxIntensity / nSpec;
		bool isBadFit = result->IsBad(i);

		// Add data point for current day
		CDateTime scanTime;
		result->GetStopTime(i, scanTime);
		int now = common.Epoch();
		int scanEpoch = common.Epoch(scanTime);
		if ((now-scanEpoch) <= 86400) {
			AppendSpecDataHistory(scannerIndex, scanEpoch, column, columnError, peakSaturation, fitSaturation, angle, isBadFit);
		}

		// Check so that we don't add too many data points here
		if (i - nIgnored >= MAX_SPEC_PER_SCAN) {
			break;
		}

		// Add the data point for last scan
		m_specData[scannerIndex][i - nIgnored].m_time		   = time;
		m_specData[scannerIndex][i-nIgnored].m_column          = column;
		m_specData[scannerIndex][i-nIgnored].m_columnError     = columnError;
		m_specData[scannerIndex][i-nIgnored].m_angle           = angle;
		m_specData[scannerIndex][i-nIgnored].m_peakSaturation  = peakSaturation;
		m_specData[scannerIndex][i-nIgnored].m_fitSaturation   = fitSaturation;
		m_specData[scannerIndex][i-nIgnored].m_isBadFit        = isBadFit;
	}

	// the number of positions in the scan
	m_positionsNum[scannerIndex] = min(result->GetEvaluatedNum() - nIgnored, MAX_SPEC_PER_SCAN);

	// add the offset
	m_offset[scannerIndex] = result->GetOffset();

	// add the temperature
	double curTemp = result->GetTemperature();
	m_data[scannerIndex][m_fluxIndex[scannerIndex]].m_temp = curTemp;

	// add the exposure-time
	long curExpTime	= result->GetExposureTime(0);
	m_data[scannerIndex][m_fluxIndex[scannerIndex]].m_expTime = curExpTime;

	// if this is the highest or the lowest temperature today, then remember it
	if(fabs(curTemp) < 100.0){
		m_temperatureRange[0][scannerIndex] = min(curTemp, m_temperatureRange[0][scannerIndex]);
		m_temperatureRange[1][scannerIndex] = max(curTemp, m_temperatureRange[1][scannerIndex]);
	}

	// and the calculated plume-centre position
	m_plumeCentre[scannerIndex] = result->GetCalculatedPlumeCentre();

	// Add the date and time that the scan was started

	return 0;
}

int CEvaluatedDataStorage::AddWindData(const CString &serial, WindSpeedMeasurement::CWindSpeedResult *result){
	CString spectrumName;
	CDateTime tid;
	Common common;

	// get the scanner index
	int scannerIndex = GetScannerIndex(serial);

	if((scannerIndex < 0)){
		if(m_serialNum < MAX_NUMBER_OF_SCANNING_INSTRUMENTS){
			// if the scanner is not in the list then insert it
			scannerIndex = m_serialNum;
			m_serials[m_serialNum].Format("%s", (LPCSTR)serial);
			
			// find the serial-number in the global configuration, to find the spectrometer-model
			bool found = false;
			for(unsigned int k = 0; k < g_settings.scannerNum; ++k){
				if(found)	break;

				for(unsigned int j = 0; j < g_settings.scanner[k].specNum; ++j){
					if(Equals(g_settings.scanner[k].spec[j].serialNumber, serial)){
						m_models[m_serialNum] = g_settings.scanner[k].spec[j].modelName;

						// Get the distance to GMT...
						m_hoursToGMT[m_serialNum] = 0;
						CString volcano;
						volcano.Format(g_settings.scanner[k].volcano);
						for(unsigned int it = 0; it < g_volcanoes.m_volcanoNum; ++it){
							if(Equals(volcano, g_volcanoes.m_name[it])){
								m_hoursToGMT[m_serialNum] = g_volcanoes.m_hoursToGMT[it];
								break;
							}
						}
						found = true;
						break;
					}
				}
			}
			if(!found)
				m_models[m_serialNum] = "";

			++m_serialNum;
		}else{
			// could not insert the serial number
			return -1;
		}
	}

	if(result == NULL)
		return 0;

	// Add the wind-speed result, if the measurement was made today...
	if(common.GetDay() == result->m_date){
		CWindMeasData wmd;
		wmd.m_scannerIndex = scannerIndex;
		wmd.m_time         = result->m_startTime;
		wmd.m_date         = result->m_date;
		wmd.m_duration     = result->m_duration;
		wmd.m_correlation  = result->m_corrAvg;
		wmd.m_windSpeed    = result->m_distance / result->m_delayAvg;

		m_windData.AddTail(wmd);
	}

	return 0;
}


/** Returns the smallest and the largest flux in the data bank */
void CEvaluatedDataStorage::GetFluxRange(const CString &serial, double &minFlux, double &maxFlux){
	// get the scanner index
	int scannerIndex = GetScannerIndex(serial);
	if(scannerIndex < 0){
		minFlux = maxFlux = 0;
		return;
	}

	// The unit conversion
	double unitConversionFactor = 1.0;
	if(g_userSettings.m_fluxUnit == UNIT_KGS)
		unitConversionFactor = 1.0;
	else if(g_userSettings.m_fluxUnit == UNIT_TONDAY)
		unitConversionFactor = 3.6 * 24.0;

	// find the maximum flux
	maxFlux = m_data[scannerIndex][0].m_flux;
	minFlux = m_data[scannerIndex][0].m_flux;

	for(int i = 0; i < m_fluxIndex[scannerIndex]; ++i){
		maxFlux = max(maxFlux, m_data[scannerIndex][i].m_flux);
		minFlux = min(minFlux, m_data[scannerIndex][i].m_flux);
	}

	// convert to the correct unit
	minFlux *= unitConversionFactor;
	maxFlux *= unitConversionFactor;
}

/** Returns the smallest and the largest columns in the data bank */
void CEvaluatedDataStorage::GetColumnRange(const CString &serial, double &minColumn, double &maxColumn, bool fullDay){
	// get the scanner index
	int scannerIndex = GetScannerIndex(serial);
	if(scannerIndex < 0){
		maxColumn = minColumn = 0;
		return;
	}

	maxColumn = -1e9;
	minColumn = 1e9;

	if (fullDay) {
		for (int i = 0; i < m_specIndex[scannerIndex]; ++i) {
			if (!m_specDataDay[scannerIndex][i].m_isBadFit) {
				maxColumn = max(maxColumn, m_specDataDay[scannerIndex][i].m_column);
				minColumn = min(minColumn, m_specDataDay[scannerIndex][i].m_column);
			}
		}
	}
	else {
		for (int i = 0; i < m_positionsNum[scannerIndex]; ++i) {
			if (!m_specData[scannerIndex][i].m_isBadFit) {
				maxColumn = max(maxColumn, m_specData[scannerIndex][i].m_column);
				minColumn = min(minColumn, m_specData[scannerIndex][i].m_column);
			}
		}
	}

	// The unit conversion
	double unitConversionFactor = 1.0;
	if (g_userSettings.m_columnUnit == UNIT_PPMM)
		unitConversionFactor = 1.0;
	else if (g_userSettings.m_columnUnit == UNIT_MOLEC_CM2)
		unitConversionFactor = 2.5e15;

	// convert to the correct unit
	minColumn *= unitConversionFactor;
	maxColumn *= unitConversionFactor;
}

/** Returns the smallest and the largest angle in the data bank */
void CEvaluatedDataStorage::GetAngleRange(const CString &serial, double &minAngle, double &maxAngle){
	// get the scanner index
	int scannerIndex = GetScannerIndex(serial);
	if(scannerIndex < 0){
		minAngle = maxAngle = 0;
		return;
	}

	maxAngle = m_specData[scannerIndex][0].m_angle;
	minAngle  = m_specData[scannerIndex][0].m_angle;

	for(int i = 0; i < m_positionsNum[scannerIndex]; ++i){
		maxAngle  = max(maxAngle, m_specData[scannerIndex][i].m_angle);
		minAngle  = min(minAngle, m_specData[scannerIndex][i].m_angle);
	}
}

void CEvaluatedDataStorage::AppendSpecDataHistory(int scannerIndex, int time, double column, double columnError, 
	double peakSaturation, double fitSaturation, double angle, bool isBadFit) {
	// If there are any old values in the array then remove them before appending more data.
	RemoveOldSpec(scannerIndex);

	// m_specIndex[scannerIndex] is the number of data-points there are in
	//	the array of col-values
	if (m_specIndex[scannerIndex] < MAX_SPEC_PER_SCAN * 300) {
		// insert the datapoint into the array
		int index = m_specIndex[scannerIndex];
		m_specDataDay[scannerIndex][index].m_time = time;
		m_specDataDay[scannerIndex][index].m_column = column;
		m_specDataDay[scannerIndex][index].m_columnError = columnError;
		m_specDataDay[scannerIndex][index].m_peakSaturation = peakSaturation;
		m_specDataDay[scannerIndex][index].m_fitSaturation = fitSaturation;
		m_specDataDay[scannerIndex][index].m_angle = angle;
		m_specDataDay[scannerIndex][index].m_isBadFit = isBadFit;
		++m_specIndex[scannerIndex];
	}
}
/** Removes old spec data */
void  CEvaluatedDataStorage::RemoveOldSpec(int scannerIndex) {
	Common common;
	for (unsigned int scannerIndex = 0; scannerIndex < m_serialNum; ++scannerIndex) {
		int k = 0;
		// count how many records to remove
		while (k < m_specIndex[scannerIndex]) {
			if (common.Epoch() - m_specDataDay[scannerIndex][k].m_time <= 86400) {
				++k;
			}
			else {
				// if this measurement is not from today, then remove it
				for (int j = k; j < m_specIndex[scannerIndex] - 1; ++j) {
					m_specDataDay[scannerIndex][j] = m_specDataDay[scannerIndex][j + 1];
				}
				--m_specIndex[scannerIndex];
			}
		}
	}
}

void CEvaluatedDataStorage::AppendFluxResult(int scannerIndex, const CDateTime &time, double fluxValue, bool fluxOk, double batteryVoltage, double temp, long expTime){
	// If there are any old values in the array then remove them before appending more data.
	RemoveOldFluxResults();
	// m_fluxIndex[scannerIndex] is the number of data-points there are in
	//	the array of flux-values

	Common common;
	if(m_fluxIndex[scannerIndex] < MAX_HISTORY){
		// insert the datapoint into the array
		int index = m_fluxIndex[scannerIndex];
		m_data[scannerIndex][index].m_flux      = fluxValue;
		m_data[scannerIndex][index].m_fluxOk    = fluxOk;
		m_data[scannerIndex][index].m_time = common.Epoch(time);
		m_data[scannerIndex][index].m_date      = time.day;
		m_data[scannerIndex][index].m_battery   = batteryVoltage;
		m_data[scannerIndex][index].m_temp      = temp;
		m_data[scannerIndex][index].m_expTime   = expTime;
		++m_fluxIndex[scannerIndex];
	}
}

/** Removes old flux results */
void  CEvaluatedDataStorage::RemoveOldFluxResults(){
	Common common;
	for (unsigned int scannerIndex = 0; scannerIndex < m_serialNum; ++scannerIndex) {
		int k = 0;
		// count how many records to remove
		while (k < m_fluxIndex[scannerIndex]) {
			if (common.Epoch() - m_data[scannerIndex][k].m_time <= 86400) {
				++k;
			}
			else {
				// if this measurement is not from today, then remove it
				for (int j = k; j < m_fluxIndex[scannerIndex] - 1; ++j) {
					m_data[scannerIndex][j] = m_data[scannerIndex][j + 1];
				}
				--m_fluxIndex[scannerIndex];
			}
		}
	}

	// other clean up below (for new UTC day)
	static int lastDate; // the day of month when this part of function was last called

	// todays date
	int today = common.GetDay();

	// don't check this several times every day
	if(lastDate == today)
		return;

	// clear the minimum and maximum temperatures
	for(unsigned int scannerIndex = 0; scannerIndex < m_serialNum; ++scannerIndex){
		m_temperatureRange[0][scannerIndex] = 999.0;
		m_temperatureRange[1][scannerIndex] = -999.0;
	}

	// ----------- Clear wind-speed measurements -----------------
	POSITION pos = m_windData.GetHeadPosition();
	while(pos != NULL){
		POSITION oldPos		= pos;
		CWindMeasData &wd = m_windData.GetNext(pos);
		
		// if the result is not from today, then remove it...
		if(wd.m_date != today){
			m_windData.RemoveAt(oldPos);
			pos	= m_windData.GetHeadPosition(); // restart the search - not very efficient but easy and safe
		}

	}

	lastDate = today;
}

/** Get Column data for last scan. 
    @param scannerIndex - the scanner for which the data should be retrieved.
    @param dataBuffer - the column data will be copied into this buffer.
    @param dataErrorBuffer - the column error data will be copied into this buffer.
    @param bufferSize - the maximum number of data points that the buffer can handle.
	@param fullDay - 0 if for last scan; 1 if for full day
    @return the number of data points copied into the dataBuffer*/
long CEvaluatedDataStorage::GetColumnData(const CString &serial, double *dataBuffer, double *dataErrorBuffer, long bufferSize, bool fullDay) {
	// get the scanner index
	int scannerIndex = GetScannerIndex(serial);

	if ((scannerIndex < 0) || (scannerIndex > MAX_NUMBER_OF_SCANNING_INSTRUMENTS))
		return 0;

	// The unit conversion
	double unitConversionFactor = 0;
	if (g_userSettings.m_columnUnit == UNIT_PPMM)
		unitConversionFactor = 1.0;
	else if (g_userSettings.m_columnUnit == UNIT_MOLEC_CM2)
		unitConversionFactor = 2.5e15;

	int nCopy;
	if (fullDay) {
		// full day
		nCopy = min(bufferSize, m_specIndex[scannerIndex]);
		for (int i = 0; i < nCopy; ++i) {
			dataBuffer[i] = m_specDataDay[scannerIndex][i].m_column * unitConversionFactor;
			dataErrorBuffer[i] = m_specDataDay[scannerIndex][i].m_columnError * unitConversionFactor;
		}
	}
	else {
		// last scan
		nCopy = min(bufferSize, m_positionsNum[scannerIndex]);
		for (int i = 0; i < nCopy; ++i) {
			dataBuffer[i] = m_specData[scannerIndex][i].m_column * unitConversionFactor;
			dataErrorBuffer[i] = m_specData[scannerIndex][i].m_columnError * unitConversionFactor;
		}

	}

	return nCopy;
}

/** Get Time data. 
    @param scannerIndex - the scanner for which the data should be retrieved.
    @param dataBuffer - the time data will be copied into this buffer.
    @param bufferSize - the maximum number of data points that the buffer can handle.
	@param fullDay - 0 if for last scan; 1 if for full day
    @return the number of data points copied into the dataBuffer*/
long CEvaluatedDataStorage::GetTimeData(const CString &serial, double *dataBuffer, long bufferSize, bool fullDay){
	// get the scanner index
	int scannerIndex = GetScannerIndex(serial);

	if((scannerIndex < 0) || (scannerIndex > MAX_NUMBER_OF_SCANNING_INSTRUMENTS))
		return 0;

	int nCopy;
	if (fullDay) {
		nCopy = min(bufferSize, m_specIndex[scannerIndex]);
		for (int i = 0; i < nCopy; ++i) {
			dataBuffer[i] = (double)m_specDataDay[scannerIndex][i].m_time;
		}
	}
	else {
		nCopy = min(bufferSize, m_positionsNum[scannerIndex]);
		for (int i = 0; i < nCopy; ++i) {
			dataBuffer[i] = (double)m_specData[scannerIndex][i].m_time;
		}
	}

	return nCopy;
}

/** Get the Column data for the measurements which are considered bad.
    @param serial - the serial number of the spectrometer for which the data should be retrieved.
    @param dataBuffer - the column data of the bad measurements will be copied into this buffer.
    @param bufferSize - the maximum number of data points that the buffer can handle.
	@param fullDay - if data for full day (true) or just last scan (false)
    @return the number of data points copied into the dataBuffer*/
long CEvaluatedDataStorage::GetBadColumnData(const CString &serial, double *dataBuffer, long bufferSize, bool fullDay){
	// get the scanner index
	int scannerIndex = GetScannerIndex(serial);

	if((scannerIndex < 0) || (scannerIndex > MAX_NUMBER_OF_SCANNING_INSTRUMENTS))
		return 0;

	// The unit conversion
	double unitConversionFactor=0.0;
	if(g_userSettings.m_columnUnit == UNIT_PPMM)
		unitConversionFactor = 1.0;
	else if(g_userSettings.m_columnUnit == UNIT_MOLEC_CM2)
		unitConversionFactor = 2.5e15;

	int nCopy;
	if (fullDay) {
		nCopy = min(bufferSize, m_specIndex[scannerIndex]);
		for (int k = 0; k < nCopy; ++k) {
			if (m_specDataDay[scannerIndex][k].m_isBadFit)
				dataBuffer[k] = m_specDataDay[scannerIndex][k].m_column * unitConversionFactor;
			else
				dataBuffer[k] = 0;
		}
	}
	else {
		nCopy = min(bufferSize, m_positionsNum[scannerIndex]);
		for (int k = 0; k < nCopy; ++k) {
			if (m_specData[scannerIndex][k].m_isBadFit)
				dataBuffer[k] = m_specData[scannerIndex][k].m_column * unitConversionFactor;
			else
				dataBuffer[k] = 0;
		}
	}

	return nCopy;
}

/** Get Intensity data. 
    @param scannerIndex - the scanner for which the data should be retrieved.
    @param peakSat - the peak saturation data will be copied into this buffer.
    @param fitSat	- the fit saturation data will be copied into this buffer.
    @param bufferSize - the maximum number of data points that the buffer can handle.
	@param fullDay - whether to get for full day (true) or just last scan (false)
    @return the number of data points copied into the dataBuffer*/
long CEvaluatedDataStorage::GetIntensityData(const CString &serial, double *peakSat, double *fitSat, long bufferSize, bool fullDay){
	// get the scanner index
	int scannerIndex = GetScannerIndex(serial);

	if((scannerIndex < 0) || (scannerIndex > MAX_NUMBER_OF_SCANNING_INSTRUMENTS))
		return 0;

	int nCopy;
	if (fullDay) {
		nCopy = min(bufferSize, m_specIndex[scannerIndex]);
		for (int i = 0; i < nCopy; ++i) {
			peakSat[i] = m_specDataDay[scannerIndex][i].m_peakSaturation;
			fitSat[i] = m_specDataDay[scannerIndex][i].m_fitSaturation;
		}
	}
	else {
		nCopy = min(bufferSize, m_positionsNum[scannerIndex]);
		for (int i = 0; i < nCopy; ++i) {
			peakSat[i] = m_specData[scannerIndex][i].m_peakSaturation;
			fitSat[i] = m_specData[scannerIndex][i].m_fitSaturation;
		}
	}

	return nCopy;
}

/** Get Angle data. 
    @param scannerIndex - the scanner for which the data should be retrieved.
    @param dataBuffer - the column data will be copied into this buffer.
    @param bufferSize - the maximum number of data points that the buffer can handle.
    @return the number of data points copied into the dataBuffer*/
long CEvaluatedDataStorage::GetAngleData(const CString &serial, double *dataBuffer, long bufferSize){
	// get the scanner index
	int scannerIndex = GetScannerIndex(serial);

	if((scannerIndex < 0) || (scannerIndex > MAX_NUMBER_OF_SCANNING_INSTRUMENTS))
		return 0;

	int nCopy = min(bufferSize, m_positionsNum[scannerIndex]);
	for(int i = 0; i < nCopy; ++i){
		dataBuffer[i] = m_specData[scannerIndex][i].m_angle;
	}

	return nCopy;
}


/** Get Flux data. 
    @param scannerIndex - the scanner for which the data should be retrieved.
    @param timeBuffer - the time-data will be copied into this buffer. The time format is epoch (seconds since 1/1/1970).
    @param dataBuffer - the column data will be copied into this buffer.
    @param qualityBuffer - the quality of the data will be copied into this buffer
    @param bufferSize - the maximum number of data points that the buffer can handle.
    @return the number of data points copied into the dataBuffer*/
long CEvaluatedDataStorage::GetFluxData(const CString &serial, double *timeBuffer, double *dataBuffer, int *qualityBuffer, long bufferSize){
	// get the scanner index
	int scannerIndex = GetScannerIndex(serial);

	if((scannerIndex < 0) || (scannerIndex > MAX_NUMBER_OF_SCANNING_INSTRUMENTS))
		return 0;

	// The unit conversion
	double unitConversionFactor = 0;
	switch (g_userSettings.m_fluxUnit) {
	case UNIT_TONDAY:
		unitConversionFactor = 3.6 * 24.0;
		break;
	case UNIT_KGS:
		unitConversionFactor = 1.0;
		break;
	}

	// Copy the flux data
	int nCopy = min(bufferSize, m_fluxIndex[scannerIndex]);
	for(int i = 0; i < nCopy; ++i){
		dataBuffer[i]     = m_data[scannerIndex][i].m_flux * unitConversionFactor;
		qualityBuffer[i]  = (m_data[scannerIndex][i].m_fluxOk) ? 1 : 0;
		timeBuffer[i]     = m_data[scannerIndex][i].m_time;
	}

	return nCopy;
}

/** Get flux statistics
    @param serial - the serial number of the spectrometer for which the data should be retrieved.
    @param average - the average flux-value will be copied to this parameter
    @param std - the standard deviation of the fluxes will be copied to this parameter
    @return the number of data points used */
long CEvaluatedDataStorage::GetFluxStat(const CString &serial, double &average, double &std){
	const int BUFFER_SIZE = 16384;
	double timeBuffer[BUFFER_SIZE];
	double fluxBuffer[BUFFER_SIZE];
	int    fluxOkBuffer[BUFFER_SIZE];

	// Get the flux-data
	int nDataPoints = GetFluxData(serial, timeBuffer, fluxBuffer, fluxOkBuffer, BUFFER_SIZE);

	// special cases: there's no or only one datapoint
	if(nDataPoints == 0){
		average = 0;
		std			= 0;
		return 0;
	}else if(nDataPoints == 1){
		average = fluxBuffer[0];
		std			= 0;
		return 1;
	}else{
		// sort out the bad fluxes
		int nOkFluxes = 0;
		double *okFluxes = new double[nDataPoints];
		for(int k = 0; k < nDataPoints; ++k){
			if(fluxOkBuffer[k]){
				okFluxes[nOkFluxes++] = fluxBuffer[k];
			}
		}
		
		if(nOkFluxes == 0){
			average = 0;
			std = 0;
		}else if(nOkFluxes == 1){
			average = okFluxes[0];
			std = 0;
		}else{
			average = Average(okFluxes, nOkFluxes);
			std = Std(okFluxes, nOkFluxes);
		}
		delete [] okFluxes;
	}
	return nDataPoints;
}

/** Get wind-measurement data
		@param serial - the serial number of the spectrometer for which we want to retrieve todays wind-measurements
		@param timeBuffer - the time-data will be copied into this buffer. Times are in seconds since midnight.
		@param wsBuffer - the calculated wind-speeds will be copied into this buffer. Unit is m/s
		@param wseBuffer - the estimated errors in the calculated wind-speed will be copied into this buffer. 
		@param bufferSize - the maximum number of data points that the buffers can handle.
		@return the number of datapoints copied into the buffers. */
long	CEvaluatedDataStorage::GetWindMeasurementData(const CString &serial, double *timeBuffer, double *wsBuffer, double *wseBuffer, long bufferSize){
	// get the scanner index
	int scannerIndex = GetScannerIndex(serial);

	if((scannerIndex < 0) || (scannerIndex > MAX_NUMBER_OF_SCANNING_INSTRUMENTS))
		return 0;

	long nCopy = 0; // the number of data-points copied

	// loop through the list of wind-measurements
	POSITION pos = m_windData.GetHeadPosition();
	while(pos != NULL){
		CWindMeasData &wd = m_windData.GetNext(pos);
		if(wd.m_scannerIndex == scannerIndex){
			timeBuffer[nCopy] = wd.m_time;
			if(wd.m_correlation > 0.0){
				wsBuffer[nCopy] = wd.m_windSpeed;
				wseBuffer[nCopy] = wd.m_windSpeedErr;
			}else{
				wsBuffer[nCopy] = 0.0;
				wseBuffer[nCopy] = 0.0;
			}
			++nCopy;
			if(nCopy == bufferSize - 1){
				return nCopy;
			}
		}
	}

	return nCopy;
}

/** Get the offset of the last scan */
double CEvaluatedDataStorage::GetOffset(const CString &serial){
	// get the scanner index
	int scannerIndex = GetScannerIndex(serial);

	if((scannerIndex < 0) || (scannerIndex > MAX_NUMBER_OF_SCANNING_INSTRUMENTS))
		return 0;

	// The unit conversion
	double unitConversionFactor = 1.0;
	if(g_userSettings.m_columnUnit == UNIT_PPMM)
		unitConversionFactor = 1.0;
	else if(g_userSettings.m_columnUnit == UNIT_MOLEC_CM2)
		unitConversionFactor = 2.5e15;

	return (m_offset[scannerIndex] * unitConversionFactor);
}

/** Get the calculated plume-centre of the last scan */
double CEvaluatedDataStorage::GetPlumeCentre(const CString &serial){
	// get the scanner index
	int scannerIndex = GetScannerIndex(serial);

	if((scannerIndex < 0) || (scannerIndex > MAX_NUMBER_OF_SCANNING_INSTRUMENTS))
		return 0;

	return m_plumeCentre[scannerIndex];
}

/** Set the status of the spectrometer 
    @param serial - the serial number of the spectrometer which status should be updated */
int CEvaluatedDataStorage::SetStatus(const CString &serial, SPECTROMETER_STATUS status){
	int scannerIndex = GetScannerIndex(serial);
	if(scannerIndex < 0)
		return 1;

	this->m_spectrometerStatus[scannerIndex] = status;

	return 0;
}

/** Get the status of the spectrometer 
    @param serial - the serial number of the spectrometer which status should be updated */
int CEvaluatedDataStorage::GetStatus(const CString &serial, SPECTROMETER_STATUS &status){
	int scannerIndex = GetScannerIndex(serial);
	if(scannerIndex < 0)
		return -1;

	status = m_spectrometerStatus[scannerIndex];

	return 0;
}

/** Gets the dynamic range for the spectrometer. Returns 0 if unknown */
double CEvaluatedDataStorage::GetDynamicRange(const CString &serial){
	int scannerIndex = GetScannerIndex(serial);
	if(scannerIndex < 0)
		return -1;
	return CSpectrometerDatabase::GetInstance().GetModel(m_models[scannerIndex]).maximumIntensity;
}

/** Gets the temperature of the last scan */
double CEvaluatedDataStorage::GetTemperature(const CString &serial){
	int scannerIndex = GetScannerIndex(serial);
	if(scannerIndex < 0)
		return -1;

	if(m_fluxIndex[scannerIndex] == 0)
		return 0.0;
	else
		return m_data[scannerIndex][m_fluxIndex[scannerIndex]-1].m_temp;
}

/** Gets the temperature of saved scans.
    @param serial - the serial number of the spectrometer for which the data should be retrieved.
    @param timeBuffer - the time-data will be copied into this buffer. The time format is the number of seconds since midnight
    @param dataBuffer - the flux data will be copied into this buffer.
    @param bufferSize - the maximum number of data points that the buffer can handle.
    @return the number of data points copied into the dataBuffer*/
long CEvaluatedDataStorage::GetTemperatureData(const CString &serial, double *timeBuffer, double *dataBuffer, long bufferSize){
	int scannerIndex = GetScannerIndex(serial);
	if(scannerIndex < 0)
		return -1;

	long nCopy = min(m_fluxIndex[scannerIndex], bufferSize);
	for(int i = 0; i < nCopy; ++i){
		timeBuffer[i]	= m_data[scannerIndex][i].m_time;
		dataBuffer[i]	=	m_data[scannerIndex][i].m_temp;
	}

	return nCopy;
}

/** Gets the battery voltage of the last scan */
double CEvaluatedDataStorage::GetBatteryVoltage(const CString &serial){
	int scannerIndex = GetScannerIndex(serial);
	if(scannerIndex < 0)
		return -1;

	if(m_fluxIndex[scannerIndex] <= 0)
		return 0.0;
	else
		return m_data[scannerIndex][m_fluxIndex[scannerIndex]-1].m_battery;
}

/** Gets the battery-voltage of saved scans.
    @param serial - the serial number of the spectrometer for which the data should be retrieved.
    @param timeBuffer - the time-data will be copied into this buffer. The time format is the number of seconds since midnight
    @param dataBuffer - the flux data will be copied into this buffer.
    @param bufferSize - the maximum number of data points that the buffer can handle.
    @return the number of data points copied into the dataBuffer*/
long CEvaluatedDataStorage::GetBatteryVoltageData(const CString &serial, double *timeBuffer, double *dataBuffer, long bufferSize){
	int scannerIndex = GetScannerIndex(serial);
	if(scannerIndex < 0)
		return -1;

	long nCopy = min(m_fluxIndex[scannerIndex], bufferSize);
	for(int i = 0; i < nCopy; ++i){
		timeBuffer[i] = m_data[scannerIndex][i].m_time;
		dataBuffer[i] = m_data[scannerIndex][i].m_battery;
	}

	return nCopy;
}

/** Gets the exposure-times of saved scans.
    @param serial - the serial number of the spectrometer for which the data should be retrieved.
    @param timeBuffer - the time-data will be copied into this buffer. The time format is the number of seconds since midnight
    @param dataBuffer - the flux data will be copied into this buffer.
    @param bufferSize - the maximum number of data points that the buffer can handle.
    @return the number of data points copied into the dataBuffer*/
long CEvaluatedDataStorage::GetExposureTimeData(const CString &serial, double *timeBuffer, double *dataBuffer, long bufferSize){
	int scannerIndex = GetScannerIndex(serial);
	if(scannerIndex < 0)
		return -1;

	long nCopy = min(m_fluxIndex[scannerIndex], bufferSize);
	for(int i = 0; i < nCopy; ++i){
		timeBuffer[i] = m_data[scannerIndex][i].m_time;
		dataBuffer[i] = m_data[scannerIndex][i].m_expTime;
	}

	return nCopy;
}


/** Gets the lowest recorded temperature for the given spectrometer */
double	CEvaluatedDataStorage::GetMinTemperature(const CString &serial){
	int scannerIndex = GetScannerIndex(serial);
	if(scannerIndex < 0)
		return -1;

	return m_temperatureRange[0][scannerIndex];
}

/** Gets the highest recorded temperature for the given spectrometer */
double	CEvaluatedDataStorage::GetMaxTemperature(const CString &serial){
	int scannerIndex = GetScannerIndex(serial);
	if(scannerIndex < 0)
		return -1;

	return m_temperatureRange[1][scannerIndex];
}


/** Gets the lowest recorded battery voltage for the given spectrometer */
double	CEvaluatedDataStorage::GetMinBatteryVoltage(const CString &serial){
	int scannerIndex = GetScannerIndex(serial);
	if(scannerIndex < 0)
		return -1;

	double minVoltage = 999;
	for(int i = 0; i < m_fluxIndex[scannerIndex]; ++i){
		if(m_data[scannerIndex][i].m_battery > -990)
			minVoltage = min(minVoltage, m_data[scannerIndex][i].m_battery);
	}
	return minVoltage;
}
	
/** Gets the highest recorded battery voltage for the given spectrometer */
double	CEvaluatedDataStorage::GetMaxBatteryVoltage(const CString &serial){
	int scannerIndex = GetScannerIndex(serial);
	if(scannerIndex < 0)
		return -1;

	double maxVoltage = -999;
	for(int i = 0; i < m_fluxIndex[scannerIndex]; ++i){
		if(m_data[scannerIndex][i].m_battery < 1000)
			maxVoltage = max(maxVoltage, m_data[scannerIndex][i].m_battery);
	}
	return maxVoltage;
}


/** Returns the spectrometer index given a serial number */
int CEvaluatedDataStorage::GetScannerIndex(const CString &serial){

	// Search for the serial-number
	for(unsigned int i = 0; i < m_serialNum; ++i){
		if(Equals(serial, m_serials[i])){
			return (int)i;
		}
	}

	// not found
	return -1;
}
