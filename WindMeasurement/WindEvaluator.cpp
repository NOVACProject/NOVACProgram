#include "stdafx.h"
#include "../NovacMasterProgram.h"
#include "WindEvaluator.h"
#include "WindSpeedResult.h"
#include <SpectralEvaluation/Utils.h>

// We must be able to read the evaluation-log files
#include "../Common/EvaluationLogFileHandler.h"

// we need the settings for the scanners
#include "../Configuration/Configuration.h"

// we also need the meterological data
#include "../MeteorologicalData.h"

extern CConfigurationSetting g_settings;	// <-- The settings
extern CFormView *pView;									// <-- The screen
extern CMeteorologicalData g_metData;			// <-- The meteorological data

using namespace WindSpeedMeasurement;


IMPLEMENT_DYNCREATE(CWindEvaluator, CWinThread)

CWindEvaluator::CWindEvaluator()
{
}

CWindEvaluator::~CWindEvaluator()
{
}

BOOL CWindEvaluator::InitInstance()
{
	// TODO:  perform and per-thread initialization here
	return TRUE;
}

void CWindEvaluator::OnQuit(WPARAM wp, LPARAM lp){
	m_evalLogs.RemoveAll();
}

/** Called when the message queue is empty */
BOOL CWindEvaluator::OnIdle(LONG lCount){
  return CCombinerThread::OnIdle(lCount);
}

BEGIN_MESSAGE_MAP(CWindEvaluator, CCombinerThread)
	ON_THREAD_MESSAGE(WM_NEW_WIND_EVALLOG, OnEvaluatedWindMeasurement)
END_MESSAGE_MAP()

/** Called when the CEvaluationController has evaluated a measured
		time-series from a wind-speed measurement from one spectrometer channel
		@param wp is a pointer to a CString-object telling the file-name of the 
			evaluation log containing the time-series.
		@param lp - not used. 
*/
void CWindEvaluator::OnEvaluatedWindMeasurement(WPARAM wp, LPARAM lp){
	if(wp == NULL)
		return;

	// 1. Go through the list of old eval-log names, if they are too old - remove them!
	CleanEvalLogList();

	// 2a. Get the filename of the evaluation-log
	CString *fn			 = (CString *)wp;
	CString fileName = CString(*fn);
	delete fn;

	// 2b. Get the volcano-index
	int	volcanoIndex = (int)lp;

	// 3. Make sure that the filename is valid and that the file exists
	//		The name of an evaluation-log file is given on the form:
	//			SerialNumber_Date_StartTime_ChannelNumber.txt
	if(!IsExistingFile(fileName)){
		return;
	}
	if(!Equals(fileName.Right(4), ".txt")){
		return;
	}
	if(fileName.Find('_') == -1){
		return;
	}

	// 4. If this is a wind-measurement made with a Heidelberg instrument, 
	//		then it does not have to be matched with any other log-file
	//		the wind-speed can be calculated directly.
	FileHandler::CEvaluationLogFileHandler reader;
	reader.m_evaluationLog.Format("%s", (LPCTSTR)fileName);
	if(SUCCESS != reader.ReadEvaluationLog())
		return;
	const std::string serialNumber = reader.m_scan[0].GetSerial();
	bool isHeidelbergInstrument = false;
	for(unsigned int it = 0; it < g_settings.scannerNum; ++it)
    {
        const std::string scannerSerial = std::string((LPCSTR)g_settings.scanner[it].spec[0].serialNumber);
		if(EqualsIgnoringCase(serialNumber, scannerSerial)){
			if(g_settings.scanner[it].instrumentType == INSTR_HEIDELBERG)
				isHeidelbergInstrument = true;
			else
				isHeidelbergInstrument = false;
			break;
		}
	}

	if(isHeidelbergInstrument && reader.IsWindSpeedMeasurement_Heidelberg(0)){
		CalculateCorrelation_Heidelberg(fileName, volcanoIndex);
		return;
	}

	// 5. Find matching evaluation-logs, to make wind-speed measurements
	CString match[MAX_MATCHING_FILES];
	int nFiles = FindMatchingEvalLog(fileName, match, volcanoIndex);

	// 6. Insert the filename itself into the list.
	InsertIntoList(fileName, volcanoIndex);

	// 7. If there were no matching evaluation-logs in the list, quit it now...
	if(nFiles == 0){
		return;
	}

	// 8. For each pair of files, make a correlation-calculation
	for(int i = 0; i < nFiles; ++i){
		CalculateCorrelation(fileName, match[i], volcanoIndex);
	}
}

/** Searches the list of evaluation logs and tries to find a log-file
		which matches the given evaluation-log. 
		@return - the number of matching files found. */
int	CWindEvaluator::FindMatchingEvalLog(const CString &evalLog, CString match[MAX_MATCHING_FILES], int volcanoIndex){
	int nFound = 0;
	CString fileName;

	// The name of an evaluation-log file is given by:
	//	SerialNumber_Date_StartTime_ChannelNumber.txt
	// this function compares everything exept the part: '_ChannelNumber.txt'
	// if they are same, then the files are considered to match

	// 1. Remove the name of the directory and the part: '_ChannelNumber.txt'
	fileName.Format(evalLog);
	Common::GetFileName(fileName);
	fileName = fileName.Left(fileName.ReverseFind('_'));

	// 2. Go through the list of evaluation-logs and note how many have a 
	//		matching filename to this evallog
	POSITION pos = m_evalLogs.GetHeadPosition();
	while(pos != NULL){
		// 1a. Get (a copy) of the filename
		CString name = m_evalLogs.GetAt(pos).fileName;

		// 1b. Remove the path and the the part: '_ChannelNumber.txt'
		Common::GetFileName(name);
		name = name.Left(name.ReverseFind('_'));

		// 1c. The left-overs should match
		if(Equals(name, fileName)){
			match[nFound++].Format("%s", (LPCSTR)m_evalLogs.GetAt(pos).fileName);
			if(nFound == MAX_MATCHING_FILES)
				return nFound;
		}

		// 1d. Go to the next item in the list
		m_evalLogs.GetNext(pos);
	}

	return nFound;
}

/** Calculate the correlation between the two time-series found in the 
		given evaluation-files. */
RETURN_CODE CWindEvaluator::CalculateCorrelation(const CString &evalLog1, const CString &evalLog2, int volcanoIndex){
	WindSpeedMeasurement::CWindSpeedCalculator	calc; // <-- The actual calculator
	FileHandler::CEvaluationLogFileHandler reader[2];
	CDateTime startTime_dt, stopTime;
	CWindField wf;
	WindSpeedMeasurement::CWindSpeedCalculator::CMeasurementSeries *series[2];
	int scanIndex[2], k;
	double delay;
	// information about the measurement
	unsigned short date[3];

	// 1. Read the evaluation-logs
	reader[0].m_evaluationLog.Format("%s", (LPCTSTR)evalLog1);
	reader[1].m_evaluationLog.Format("%s", (LPCTSTR)evalLog2);
	if(SUCCESS != reader[0].ReadEvaluationLog())
		return FAIL;
	if(SUCCESS != reader[1].ReadEvaluationLog())
		return FAIL;

	// 2. Find the wind-speed measurement series in the log-files
	for(k = 0; k < 2; ++k){
		for(scanIndex[k] = 0; scanIndex[k] < reader[k].m_scanNum; ++scanIndex[k])
			if(reader[k].IsWindSpeedMeasurement(scanIndex[k]))
				break;
		if(scanIndex[k] == reader[k].m_scanNum)
			return FAIL;		// <-- no wind-speed measurement found
	}
	// 2a. Find the start and stop-time of the measurement
	Evaluation::CScanResult &scan = reader[0].m_scan[scanIndex[0]];
	scan.GetStartTime(0, startTime_dt);
	scan.GetStopTime(scan.GetEvaluatedNum()- 1, stopTime);

	// 2b. Find out what plume-height to use
    const CString scannerSerialNumber = CString(scan.GetSerial().c_str());
	g_metData.GetWindField(scannerSerialNumber, startTime_dt, wf);
	m_settings.plumeHeight = wf.GetPlumeHeight();

	// 3. Create the wind-speed measurement series
	for(k = 0; k < 2; ++k){
		// 3a. The scan we're looking at
		Evaluation::CScanResult &scan = reader[k].m_scan[scanIndex[k]];

		// 3b. The start-time of the whole measurement
		const CDateTime *startTime = scan.GetStartTime(0);

		// 3c. The length of the measurement
		int	length = scan.GetEvaluatedNum();

		// 3d. Allocate the memory for the series
		series[k] = new CWindSpeedCalculator::CMeasurementSeries(length);

		// 3e. Copy the relevant data in the scan
		for(int i = 0; i < length; ++i){
			const CDateTime *time = scan.GetStartTime(i);

			// get the column value
			series[k]->column[i]	= scan.GetColumn(i, 0);

			// calculate the time-difference between the start of the
			//	time-series and this measurement
			series[k]->time[i] = 3600.0 * (time->hour - startTime->hour) +
														60.0 * (time->minute - startTime->minute) +
															1.0	* (time->second - startTime->second);
		}
	}

	// 4. Perform the correlation calculations...

	// 4a. Calculate the correlation, assuming that series[0] is the upwind series
	if(SUCCESS != calc.CalculateDelay(delay, series[0], series[1], m_settings)){
		ShowMessage("Failed to correlate time-series, no windspeed could be derived");

		// Tell the world that we've tried to make a correlation calculation but failed
		Evaluation::CScanResult &scan = reader[0].m_scan[scanIndex[0]];
		scan.GetDate(0, date);
		PostWindMeasurementResult(0, 0, 0, startTime_dt, stopTime, scannerSerialNumber);

		// Clean up and return
		delete series[0];		delete series[1];
		return FAIL;
	}

	// 4b. Calculate the average correlation
	double avgCorr1 = Average(calc.corr, calc.m_length);

	// 4c. Calculate the correlation, assuming that series[1] is the upwind series
	if(SUCCESS != calc.CalculateDelay(delay, series[1], series[0], m_settings)){
		ShowMessage("Failed to correlate time-series, no windspeed could be derived");

		// Tell the world that we've tried to make a correlation calculation but failed
		Evaluation::CScanResult &scan = reader[0].m_scan[scanIndex[0]];
		scan.GetDate(0, date);
		PostWindMeasurementResult(0, 0, 0, startTime_dt, stopTime, scannerSerialNumber);

		// Clean up and return
		delete series[0];		delete series[1];
		return FAIL;
	}

	// 4d. Calculate the average correlation
	double avgCorr2 = Average(calc.corr, calc.m_length);

	// 4e. Use the result which gave the higest correlation
	if(avgCorr1 > avgCorr2){
		if(SUCCESS != calc.CalculateDelay(delay, series[0], series[1], m_settings)){
			// this should never happen
			ShowMessage("Failed to correlate time-series, no windspeed could be derived");
			delete series[0];		delete series[1];
			return FAIL;
		}
	}

	// 5. Write the results of our calculations to file
	WriteWindMeasurementLog(calc, evalLog1, reader[0].m_scan[scanIndex[0]], volcanoIndex, INSTR_GOTHENBURG);

	// 6. Clean up a little bit.
	delete series[0];
	delete series[1];

	return SUCCESS;
}

/** Calculate the correlation between the two time-series found in the 
		given evaluation-file. */
RETURN_CODE CWindEvaluator::CalculateCorrelation_Heidelberg(const CString &evalLog, int volcanoIndex){
	WindSpeedMeasurement::CWindSpeedCalculator	calc; // <-- The actual calculator
	FileHandler::CEvaluationLogFileHandler reader;
	WindSpeedMeasurement::CWindSpeedCalculator::CMeasurementSeries *series[2];
	CWindField wf;
	CDateTime startTime_dt;
	int scanIndex;
	double delay;

	// 1. Read the evaluation-log
	reader.m_evaluationLog.Format("%s", (LPCSTR)evalLog);
	if(SUCCESS != reader.ReadEvaluationLog())
		return FAIL;

	// 2. Find the wind-speed measurement series in the log-files
	for(scanIndex = 0; scanIndex < reader.m_scanNum; ++scanIndex)
		if(reader.IsWindSpeedMeasurement_Heidelberg(scanIndex))
			break;
	if(scanIndex == reader.m_scanNum)
		return FAIL;		// <-- no wind-speed measurement found

	// 3. Create the wind-speed measurement series

	// 3a. The scan we're looking at
	Evaluation::CScanResult &scan = reader.m_scan[scanIndex];

	// 3b. The start-time of the whole measurement
	const CDateTime *startTime = scan.GetStartTime(0);
	scan.GetStartTime(0, startTime_dt);

	// 3b-2. Find out what plume-height to use
    const CString scannerSerialNumber = CString(scan.GetSerial().c_str());
	g_metData.GetWindField(scannerSerialNumber, startTime_dt, wf);
	m_settings.plumeHeight = wf.GetPlumeHeight();

	// 3c. The length of the measurement
	int	length = scan.GetEvaluatedNum();

	// 3d. Allocate the memory for the series
	series[0] = new CWindSpeedCalculator::CMeasurementSeries(length / 2);
	series[1] = new CWindSpeedCalculator::CMeasurementSeries(length / 2);

	// 3e. Copy the relevant data in the scan
	for(int k = 0; k < length; ++k){
		const CDateTime *time = scan.GetStartTime(k);

		series[k % 2]->column[k / 2] = scan.GetColumn(k, 0);

		// Save the time difference
		series[k % 2]->time[k / 2]		= 
			3600 * (time->hour - startTime->hour) +
			60	 * (time->minute - startTime->minute) +
			(time->second - startTime->second);
	}

	// 3f. Adjust the settings to have the correct angle
	int midpoint = (int)(length / 2);
	double d1 = scan.GetScanAngle(midpoint) - scan.GetScanAngle(midpoint + 1);
	double d2 = scan.GetScanAngle2(midpoint) - scan.GetScanAngle2(midpoint + 1);
	m_settings.angleSeparation = sqrt(d1 * d1 + d2 * d2);

	// 4. Perform the correlation calculations...

	// 4a. Calculate the correlation, assuming that series[0] is the upwind series
	if(SUCCESS != calc.CalculateDelay(delay, series[0], series[1], m_settings)){
		ShowMessage("Failed to correlate time-series, no windspeed could be derived");
		delete series[0];		delete series[1];
		return FAIL;
	}

	// 4b. Calculate the average correlation
	double avgCorr1 = Average(calc.corr, calc.m_length);

	// 4c. Calculate the correlation, assuming that series[1] is the upwind series
	if(SUCCESS != calc.CalculateDelay(delay, series[1], series[0], m_settings)){
		ShowMessage("Failed to correlate time-series, no windspeed could be derived");
		delete series[0];		delete series[1];
		return FAIL;
	}

	// 4d. Calculate the average correlation
	double avgCorr2 = Average(calc.corr, calc.m_length);

	// 4e. Use the result which gave the higest correlation
	if(avgCorr1 > avgCorr2){
		if(SUCCESS != calc.CalculateDelay(delay, series[0], series[1], m_settings)){
			// this should never happen
			ShowMessage("Failed to correlate time-series, no windspeed could be derived");
			delete series[0];		delete series[1];
			return FAIL;
		}
	}

	// 5. Write the results of our calculations to file
	WriteWindMeasurementLog(calc, evalLog, reader.m_scan[scanIndex], volcanoIndex, INSTR_HEIDELBERG);

	// 6. Clean up a little bit.
	delete series[0];
	delete series[1];

	return SUCCESS;
}

/** Writes the results of the windspeed measurement to a file */
void CWindEvaluator::WriteWindMeasurementLog(const CWindSpeedCalculator &calc, const CString &evalLog, const Evaluation::CScanResult &scan, int volcanoIndex, INSTRUMENT_TYPE instrType){
	CString fileName, directory, commonName, str;
	CDateTime startTime, stopTime;

	// 0. Get the start-time and stop-time of the measurement
	scan.GetStartTime(0, startTime);
	scan.GetStopTime(scan.GetEvaluatedNum()- 1, stopTime);

	// 1. Get the directory of the evaluation-log-file
	directory.Format(evalLog);
	Common::GetDirectory(directory);

	// 2. Get the common name for the two-log files
	commonName.Format(evalLog);
	Common::GetFileName(commonName);
	commonName = commonName.Left(commonName.ReverseFind('_'));

	// 3. Get the filename of the wind-speed log-file
	fileName.Format("%s%s_WindMeasurement_%04d.%02d.%02d.txt", (LPCTSTR)directory, (LPCTSTR)commonName, startTime.year, startTime.month, startTime.day);

	// 4. Make sure that we can open and write to the log-file
	FILE *f = fopen(fileName, "w");
	if(f == NULL)
		return;

	// 5. Calculate the distance at plume-height
	double distance;
	double coneAngle	= scan.GetConeAngle();
	double scanAngle	= scan.GetScanAngle(scan.GetEvaluatedNum() / 2);

	if(instrType == INSTR_GOTHENBURG){
		if(fabs(coneAngle - 90.0) < 1){
			// Flat scanner
			distance		= m_settings.plumeHeight * (1.0 / cos(DEGREETORAD * scanAngle)) * tan(DEGREETORAD * m_settings.angleSeparation);
		}else{
			// Cone scanner
			double angle	= DEGREETORAD * (90.0 - (coneAngle - fabs(scan.GetPitch())));
			distance			= m_settings.plumeHeight * fabs(tan(angle) - tan(angle - DEGREETORAD * m_settings.angleSeparation));
		}
		// If the scanners are not looking straight up then the distance between
		//		the two directions gets decreased with the scanAngle
		distance			*= cos(scanAngle * DEGREETORAD);
	}else{	//for Heidelberg instrument: recover the distance along the plume
					//from the plume height and the two measurement directions used
					//(alpha=angle from zenith; beta=azimuth angle)

		int halfLength= scan.GetEvaluatedNum() / 2;
		double alpha1 = DEGREETORAD * scan.GetScanAngle(halfLength);
		double alpha2 = DEGREETORAD * scan.GetScanAngle(halfLength+1);
		double beta1  = DEGREETORAD * scan.GetScanAngle2(halfLength);
		double beta2  = DEGREETORAD * scan.GetScanAngle2(halfLength+1);
		distance		= m_settings.plumeHeight *
									sqrt ( (tan(alpha1)*sin(beta1)-tan(alpha2)*sin(beta2))*
													(tan(alpha1)*sin(beta1)-tan(alpha2)*sin(beta2)) + 
													(tan(alpha1)*cos(beta1)-tan(alpha2)*cos(beta2))*
													(tan(alpha1)*cos(beta1)-tan(alpha2)*cos(beta2)) );
	}

	// 6. Write the header of the wind-log file
	fprintf(f, "<windcorrelationinformation>\n");
	fprintf(f, "\tdate=%04d.%02d.%02d\n",						startTime.year, startTime.month, startTime.day);
	fprintf(f, "\tstarttime=%02d:%02d:%02d\n",			startTime.hour, startTime.minute, startTime.second);
	fprintf(f, "\tstoptime=%02d:%02d:%02d\n",				stopTime.hour, stopTime.minute, stopTime.second);
	fprintf(f, "\tserieslength=%d\n",								scan.GetEvaluatedNum());
	fprintf(f, "\texposuretime=%d\n",								scan.GetExposureTime(0));
	fprintf(f, "\tlowpassfilter=%d\n",							m_settings.lowPassFilterAverage);
	fprintf(f, "\tassumedplumeheight=%1lf\n",				m_settings.plumeHeight);
	fprintf(f, "\tangleseparation=%.1lf\n",					m_settings.angleSeparation);
	fprintf(f, "\tseparationatplumeheight=%.2lf\n", distance);
	fprintf(f, "\ttestlength=%d\n",									m_settings.testLength);
	fprintf(f, "\tshiftmax=%d\n",										m_settings.shiftMax);
	fprintf(f, "</windcorrelationinformation>\n\n");
	fprintf(f, "correlation\tshift\twindspeed\n");

	// 7. Write the correlation data
	double avgDelay = 0.0;	int nDelayPoints = 0;
	double avgCorr  = 0.0;
	for(int k = 0; k < calc.m_length; ++k){
		if(calc.used[k]){
			fprintf(f, "%.2lf\t",		calc.corr[k]);
			fprintf(f, "%.2lf\t",		calc.delays[k]);
			fprintf(f, "%.2lf\n",		distance / calc.delays[k]);

			avgDelay	+= calc.delays[k];
			avgCorr		+= calc.corr[k];
			++nDelayPoints;
		}
	}

	fclose(f);

	// 8. Calculate the average delay and the average wind-speed
	avgDelay /= (double)nDelayPoints;
	avgCorr	 /= (double)nDelayPoints;
	double avgWS = distance / avgDelay;

	CString message;
	message.Format("Successfully correlated time-series. Derived a wind-speed of %.1lf [m/s]", avgWS);
	ShowMessage(message);

	// 9. Tell the rest of the program about what we've done
    const CString scannerSerialNumber = CString(scan.GetSerial().c_str());
    PostWindMeasurementResult(avgDelay, avgCorr, distance, startTime, stopTime, scannerSerialNumber);

	// 10. Upload the file to the FTP-server
	UploadToNOVACServer(fileName, volcanoIndex);
}

/** Tells the rest of the program about the result of the correlation - calculation.
		Either if the calculation was successful or not... */
void CWindEvaluator::PostWindMeasurementResult(double avgDelay, double avgCorr, double distance, const CDateTime startTime, const CDateTime stopTime, const CString &serial){
	CWindSpeedResult *result = new CWindSpeedResult();
	result->m_delayAvg = avgDelay;
	result->m_corrAvg	 = avgCorr;
	result->m_distance = distance;
	result->m_startTime= startTime.hour * 3600 + startTime.minute * 60 + startTime.second;
	result->m_duration = (stopTime.hour * 3600 + stopTime.minute * 60  + stopTime.second) - result->m_startTime;
	result->m_date		 = startTime.day;
	result->m_serial.Format(serial);
	pView->PostMessage(WM_CORR_SUCCESS, (WPARAM)result);
}
