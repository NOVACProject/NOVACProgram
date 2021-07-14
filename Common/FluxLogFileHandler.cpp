#include "StdAfx.h"
#include "fluxlogfilehandler.h"

using namespace FileHandler;
using namespace novac;

CFluxLogFileHandler::CFluxLogFileHandler(void)
{
	m_fluxesNum = 0;
}

CFluxLogFileHandler::~CFluxLogFileHandler(void)
{
}

/** Reads the flux log */
RETURN_CODE CFluxLogFileHandler::ReadFluxLog(){
	char  scanstarttime[]				= _T("scanstarttime");						// this string only exists in the header line.
	CString str;
	char szLine[8192];
	int measNr = 0;
	double fValue;
	bool fReadingScan = false;
	double flux = 0.0;
	Evaluation::CFluxResult curResult;
	CSpectrumInfo curScanInfo;
	curScanInfo.m_batteryVoltage = -999.0;
	curScanInfo.m_temperature = -999;
	curScanInfo.m_exposureTime = -999;

	// If no flux log selected, quit
	if(strlen(m_fluxLog) <= 1)
		return FAIL;

	// Open the flux log
	FILE *f = fopen(m_fluxLog, "r");
	if(NULL == f)
		return FAIL;

	// Reset the column info
	ResetColumns();

	// Reset the data
	this->m_fluxes.RemoveAll();
	this->m_scanInfo.RemoveAll();
	this->m_fluxesNum = 0;

	// Read the file, one line at a time
	while(fgets(szLine, 8192, f)){

		// ignore empty lines
		if(strlen(szLine) < 2){
			if(fReadingScan){
				fReadingScan = false;
				// Reset the column information
				ResetColumns();
			}
			continue;
		}

		// convert the string to all lower-case letters
		for(unsigned int it = 0; it < strlen(szLine); ++it){
			szLine[it] = tolower(szLine[it]);
		}

		// find the next start of a measurement 
		if(NULL != strstr(szLine, scanstarttime)){
			ParseScanHeader(szLine);

			fReadingScan = true;

			// read the next line, which is the first scan
			continue; 
		} // end if(NULL != strstr...)

		// ignore comment lines
		if(szLine[0] == '#')
			continue;

		// if we're not reading a scan, let's read the next line
		if(!fReadingScan)
			continue;

		// Split the flux table up into tokens and parse them. 
		int nColumnsParsed = 0;
		char* szToken = (char*)(LPCSTR)szLine;
		int curCol = -1;
		while(szToken = strtok(szToken, " \t")){
			++curCol;

			// First check the starttime
			if(curCol == m_col.starttime){
				int fValue1, fValue2, fValue3, ret;
				if (strstr(szToken, ":")) {
					ret = sscanf(szToken, "%d:%d:%d", &fValue1, &fValue2, &fValue3);
				}
				else {
					ret = sscanf(szToken, "%d.%d.%d", &fValue1, &fValue2, &fValue3);
				}
				if (ret == 3) {
					curResult.m_startTime.hour = fValue1;
					curResult.m_startTime.minute = fValue2;
					curResult.m_startTime.second = fValue3;
					szToken = NULL;
					++nColumnsParsed;
				}
				continue;
			}

			// Then check the date
			if(curCol == m_col.date){
				int fValue1, fValue2, fValue3, ret;
				if (strstr(szToken, ":")) {
					ret = sscanf(szToken, "%d:%d:%d", &fValue1, &fValue2, &fValue3);
				}
				else if (strstr(szToken, "-")) {
					ret = sscanf(szToken, "%d-%d-%d", &fValue1, &fValue2, &fValue3);
				}
				else {
					ret = sscanf(szToken, "%d.%d.%d", &fValue1, &fValue2, &fValue3);
				}
				if (ret == 3) {
					curResult.m_startTime.year = fValue1;
					curResult.m_startTime.month = fValue2;
					curResult.m_startTime.day = fValue3;
					szToken = NULL;
					++nColumnsParsed;
				}
				continue;
			}
		
			// Check if this is the source of the wind-data
			if(curCol == m_col.windDataSrc){
				if(Equals(szToken, "user")){
					curResult.m_windDirectionSource = MET_USER;
					curResult.m_windSpeedSource = MET_USER;
				}else if(Equals(szToken, "default")){
					curResult.m_windDirectionSource = MET_DEFAULT;
					curResult.m_windSpeedSource = MET_DEFAULT;
				}else if(Equals(szToken, "ecmwf_forecast")){
					curResult.m_windDirectionSource = MET_ECMWF_FORECAST;
					curResult.m_windSpeedSource = MET_ECMWF_FORECAST;
				}else if(Equals(szToken, "ecmwf_analysis")){
					curResult.m_windDirectionSource = MET_ECMWF_ANALYSIS;
					curResult.m_windSpeedSource = MET_ECMWF_ANALYSIS;
				}else if(Equals(szToken, "dual_beam_measurement")){
					curResult.m_windDirectionSource = MET_DUAL_BEAM_MEASUREMENT;
					curResult.m_windSpeedSource = MET_DUAL_BEAM_MEASUREMENT;
				}
			}

			// Check if this is the source of the plume-height
			if(curCol == m_col.plumeHeightSrc){
				if(Equals(szToken, "user")){
					curResult.m_plumeHeightSource = MET_USER;
				}else if(Equals(szToken, "default")){
					curResult.m_plumeHeightSource = MET_DEFAULT;
				}else if(Equals(szToken, "ecmwf_forecast")){
					curResult.m_plumeHeightSource = MET_ECMWF_FORECAST;
				}else if(Equals(szToken, "ecmwf_analysis")){
					curResult.m_plumeHeightSource = MET_ECMWF_ANALYSIS;
				}else if(Equals(szToken, "triangulation")){
					curResult.m_plumeHeightSource = MET_GEOMETRY_CALCULATION;
				}
			}


			// ignore columns whose value cannot be parsed into a float
			if(1 != sscanf(szToken, "%lf", &fValue)){
				szToken = NULL;
				++nColumnsParsed;
				continue;
			}

			// The flux-value
			if(curCol == m_col.flux){
				curResult.m_flux = (float)fValue;
				szToken = NULL;
				++nColumnsParsed;
				continue;
			}

			// The quality of the flux
			if(curCol == m_col.fluxOk){
				curResult.m_fluxOk = (fValue >= 0.5) ? true : false;
				szToken = NULL;
				++nColumnsParsed;
				continue;
			}

			// The wind-speed
			if(curCol == m_col.windSpeed){
				curResult.m_windSpeed = (float)fValue;
				szToken = NULL;
				++nColumnsParsed;
				continue;
			}

			// The wind-direction
			if(curCol == m_col.windDirection){
				curResult.m_windDirection = (float)fValue;
				szToken = NULL;
				++nColumnsParsed;
				continue;
			}

			// The plume height
			if(curCol == m_col.plumeHeight){
				curResult.m_plumeHeight = (float)fValue;
				szToken = NULL;
				++nColumnsParsed;
				continue;
			}

			// The compass direction
			if(curCol == m_col.compass){
				curResult.m_compass = (float)fValue;
				szToken = NULL;
				++nColumnsParsed;
				continue;
			}

			// The cone angle
			if(curCol == m_col.coneAngle){
				curResult.m_coneAngle = (float)fValue;
				szToken = NULL;
				++nColumnsParsed;
				continue;
			}

			// The tilt
			if(curCol == m_col.tilt){
				curResult.m_tilt = (float)fValue;
				szToken = NULL;
				++nColumnsParsed;
				continue;
			}

			// The battery voltage
			if(curCol == m_col.batteryVoltage){
				curScanInfo.m_batteryVoltage = (float)fValue;
				szToken = NULL;
				++nColumnsParsed;
				continue;
			}

			// The temperature
			if(curCol == m_col.temperature){
				curScanInfo.m_temperature = (float)fValue;
				szToken = NULL;
				++nColumnsParsed;
				continue;
			}

			// The exposure-time
			if(curCol == m_col.expTime){
				curScanInfo.m_exposureTime = (long)fValue;
				szToken = NULL;
				++nColumnsParsed;
				continue;
			}

			// nothing understandable found, continue with the next token
			szToken = NULL;
		} // end while szToken...

		if(nColumnsParsed > 0){
			m_fluxes.SetAtGrow(m_fluxesNum,		curResult);
			m_scanInfo.SetAtGrow(m_fluxesNum,	curScanInfo);
			++m_fluxesNum;
			nColumnsParsed = 0;
		}
	} // end while(fgets...)

    fclose(f);

	return SUCCESS;
}

/** Reads the header line for the scan information and retrieves which 
    column represents which value. */
void CFluxLogFileHandler::ParseScanHeader(const char szLine[8192]){
	// reset some old information
	ResetColumns();

	char str[8192];
	if(szLine[0] == '#')
		strncpy(str, szLine+1, 8191*sizeof(char));
	else
		strncpy(str, szLine, 8192*sizeof(char));

	char* szToken = (char*)(LPCSTR)str;
	int curCol = -1;
	char scanDate[]			= _T("scandate");
	char starttime[]		= _T("scanstarttime");
	char flux[]				= _T("flux_[kg/s]");
	char fluxOk[]			= _T("okflux");
	char windspeed[]		= _T("windspeed_[m/s]");
	char winddir[]			= _T("winddirection_[deg]");
	char winddatasrc[]		= _T("winddatasource");
	char plumeheight[]		= _T("plumeheight_[m]");
	char plumeheightsrc[]	= _T("plumeheightsource");
	char compass[]			= _T("compassdirection_[deg]");
	char coneangle[]		= _T("coneangle");
	char tilt[]				= _T("tilt");
	char batteryVolt[]		= _T("batteryvoltage");
	char temperature[]		= _T("temperature");
	char exposureTime[]		= _T("exposuretime");

	while(szToken = strtok(szToken, "\t")){
		++curCol;

		// The scan date
		if(0 == _strnicmp(szToken, scanDate, strlen(scanDate))){
			m_col.date = curCol;
		szToken = NULL;
		continue;
		}

		// The start time
		if(0 == _strnicmp(szToken, starttime, strlen(starttime))){
			m_col.starttime = curCol;
		szToken = NULL;
		continue;
		}

		// The flux
		if(0 == _strnicmp(szToken, flux, strlen(flux))){
			m_col.flux = curCol;
		szToken = NULL;
		continue;
		}

		// The quality of the flux data
		if(0 == _strnicmp(szToken, fluxOk, strlen(fluxOk))){
			m_col.fluxOk = curCol;
		szToken = NULL;
		continue;
		}

		// The windspeed
		if(0 == _strnicmp(szToken, windspeed, strlen(windspeed))){
			m_col.windSpeed = curCol;
		szToken = NULL;
		continue;
		}

		// The winddirection
		if(0 == _strnicmp(szToken, winddir, strlen(winddir))){
			m_col.windDirection = curCol;
		szToken = NULL;
		continue;
		}

		// The wind-data source
		if(0 == _strnicmp(szToken, winddatasrc, strlen(winddatasrc))){
			m_col.windDataSrc	= curCol;
		szToken = NULL;
		continue;
		}

		// The plumeheight
		if(0 == _strnicmp(szToken, plumeheight, strlen(plumeheight))){
			m_col.plumeHeight = curCol;
		szToken = NULL;
		continue;
		}

		// The plumeheight source
		if(0 == _strnicmp(szToken, plumeheightsrc, strlen(plumeheightsrc))){
			m_col.plumeHeightSrc = curCol;
		szToken = NULL;
		continue;
		}

		// The compass
		if(0 == _strnicmp(szToken, compass, strlen(compass))){
			m_col.compass = curCol;
		szToken = NULL;
		continue;
		}

		// The coneangle
		if(0 == _strnicmp(szToken, coneangle, strlen(coneangle))){
			m_col.compass = curCol;
		szToken = NULL;
		continue;
		}

		// The tilt
		if(0 == _strnicmp(szToken, tilt, strlen(tilt))){
			m_col.tilt = curCol;
		szToken = NULL;
		continue;
		}

		// The battery voltage
		if(0 == _strnicmp(szToken, batteryVolt, strlen(batteryVolt))){
			m_col.batteryVoltage = curCol;
		szToken = NULL;
		continue;
		}

		// The temperature
		if(0 == _strnicmp(szToken, temperature, strlen(temperature))){
			m_col.temperature = curCol;
		szToken = NULL;
		continue;
		}

		// The exposure-time
		if(0 == _strnicmp(szToken, exposureTime, strlen(exposureTime))){
			m_col.expTime = curCol;
		szToken = NULL;
		continue;
		}

		szToken = NULL;
	}

	return;

}

/** Resets the information about which column data is stored in */
void CFluxLogFileHandler::ResetColumns(){
	m_col.date				= 0;		// <-- the date the scan was made
	m_col.starttime			= 1;		// <-- the UTC-time when the measurement began
	m_col.flux				= 3;		// <-- the calculated flux in kg/s
	m_col.fluxOk			= -1;		// <-- the quality of the flux-data
	m_col.windSpeed			= 4;		// <-- the wind-speed used
	m_col.windSpeedSrc		= -1;		// <-- the wind-speed source
	m_col.windDirection		= 5;		// <-- the wind-direction used
	m_col.windDirectionSrc	=-1;	// <-- the wind-direction source
	m_col.windDataSrc		= -1;		// <-- the wind-data source
	m_col.plumeHeight		= 8;		// <-- the plume-height used
	m_col.plumeHeightSrc	= -1;		// <-- the plume-height source
	m_col.compass			= 10;		// <-- the used compass-direction
	m_col.coneAngle			= 12;		// <-- the cone-angle of the systems
	m_col.tilt				= -1;		// <-- the tilt of the system
	m_col.batteryVoltage	= -1;		// <-- the battery voltage of the systems	
	m_col.temperature		= -1;		// <-- the temperature of the systems
	m_col.expTime			= -1;		// <-- the exposure-time of the measurement
}

/** Sorts the scans in order of collection */
void CFluxLogFileHandler::SortScans(){
}

