#include "stdafx.h"
#include "../NovacMasterProgram.h"
#include "GeometryEvaluator.h"

// We must be able to read the evaluation-log files
#include "../Common/EvaluationLogFileHandler.h"

// .. and to perform geometrical calculations
#include "GeometryCalculator.h"

// the version of the program
#include "../Common/Version.h"

// the list of all known volcanoes
#include "../VolcanoInfo.h"

// the results of the geometry calculations can be saved in a CGeometryResult - object
#include "GeometryResult.h"

extern CVolcanoInfo					g_volcanoes;	// <-- A list of all known volcanoes
extern CFormView *pView;									// <-- The screen

using namespace Geometry;

IMPLEMENT_DYNCREATE(CGeometryEvaluator, CWinThread)



CGeometryEvaluator::CGeometryEvaluator()
{
}

CGeometryEvaluator::~CGeometryEvaluator()
{
}

BOOL CGeometryEvaluator::InitInstance()
{
	// TODO:  perform and per-thread initialization here
	return TRUE;
}

void CGeometryEvaluator::OnQuit(WPARAM wp, LPARAM lp){
}

/** Called when the message queue is empty */
BOOL CGeometryEvaluator::OnIdle(LONG lCount){
  return CCombinerThread::OnIdle(lCount);
}
BEGIN_MESSAGE_MAP(CGeometryEvaluator, CCombinerThread)
	ON_THREAD_MESSAGE(WM_NEW_SCAN_EVALLOG, OnEvaluatedScan)	
END_MESSAGE_MAP()


// CGeometryEvaluator message handlers

/** Called when the CEvaluationController has evaluated a normal scan
		from one connected spectrometer.
		@param wp is a pointer to a CString-object telling the file-name of the 
			evaluation log containing the scan.
		@param lp - not used. 
*/
void CGeometryEvaluator::OnEvaluatedScan(WPARAM wp, LPARAM lp){
	if(wp == NULL)
		return;

	// 1. Go through the list of old eval-log names, if they are too old - remove them!
	CleanEvalLogList();

	// 2a. Get the filename of the evaluation-log
	CString *fn = (CString *)wp;
	CString fileName = CString(*fn);
	delete fn;

	// 2b. Get the volcano-index
	int	volcanoIndex = (int)lp;

	// 3. Make sure that the filename is valid and that the file exists
	//     The name of an evaluation-log file is given on the form:
	//       SerialNumber_Date_StartTime_ChannelNumber.txt
	if(strlen(fileName) < 4)
		return;
	if(!IsExistingFile(fileName))
		return;
	if(!Equals(fileName.Right(4), ".txt"))
		return;
	if(fileName.Find('_') == -1) 
		return;

	// 4. Find matching evaluation-logs, to make plume-height measurements
	CString match[MAX_MATCHING_FILES];
	int nFiles = FindMatchingEvalLog(fileName, match,  volcanoIndex);

	// 5. Insert the filename itself into the list.
	InsertIntoList(fileName, volcanoIndex);

	// 6. If there were no matching evaluation-logs in the list, quit it now...
	if(nFiles == 0)
		return;

	// 7. For each pair of files, make a geometry-calculation
	for(int i = 0; i < nFiles; ++i){
		CalculateGeometry(fileName, match[i], volcanoIndex);
	}
}

/** Searches the list of evaluation logs and tries to find a log-file
		which matches the given evaluation-log. 
		@return - the number of matching files found. */
int	CGeometryEvaluator::FindMatchingEvalLog(const CString &evalLog, CString match[MAX_MATCHING_FILES], int volcanoIndex){
	int nFound = 0;
	CDateTime startTime1, startTime2; // <-- the start-times of the two scans
	CString serial1, serial2;     // <-- the serial-numbers of the two spectrometers
	CString message;

	// The name of an evaluation-log file is given by:
	//	SerialNumber_Date_StartTime_ChannelNumber.txt
	// this function compares everything exept the part: '_ChannelNumber.txt'
	// The two files are considered to match if :
	//		1 - the serial-numbers are different
	//		2 - the dates are the same
	//		3 - the start-times do not differ by more than MAX_TIME_DIFFERENCE seconds
	//		4 - the measurements are made on the same volcano

	// 1. Get the serial and start-time 
	if(!GetInfoFromFileName(evalLog, startTime1, serial1)){
		return 0;
	}

	// 2. Go through the list of evaluation-logs and note how many have a 
	//      matching filename to this evallog
	POSITION pos = m_evalLogs.GetHeadPosition();
	while(pos != NULL){
		// 1a. Get (a copy) of the filename
		CString name = m_evalLogs.GetAt(pos).fileName;

		// 1b. Get the serial and start-time 
		if(!GetInfoFromFileName(name, startTime2, serial2)){
			m_evalLogs.GetNext(pos);
			continue;
		}

		// 1c. Get the volcanoIndex of the second evaluation-log
		int	volcanoIndex2 = m_evalLogs.GetAt(pos).volcanoIndex;

		// 1d. The volcanoIndeces must be the same
		if(volcanoIndex != volcanoIndex2){
			m_evalLogs.GetNext(pos);
			continue;
		}

		// 1e. The serial-numbers must be different
		if(Equals(serial1, serial2)){
			m_evalLogs.GetNext(pos);
			continue;
		}

		// 1f. The time elapsed between the two measurements must not be more than MAX_TIME_DIFFERENCE - seconds
		double timeDifference = fabs(CDateTime::Difference(startTime1, startTime2));
		if(timeDifference > MAX_TIME_DIFFERENCE){
			m_evalLogs.GetNext(pos);
			continue;
		}

		// 1g. This is a match
		match[nFound++].Format("%s", (LPCSTR)name);
		if(nFound >= MAX_MATCHING_FILES){
			message.Format("File: %s can be matched with %d other files", (LPCSTR)evalLog, nFound);
			ShowMessage(message);
			return nFound;
		}

		// 1f. Go to the next item in the list
		m_evalLogs.GetNext(pos);
	}

	return nFound;
}

/** Takes the filename of an evaluation log and extracts the 
		Serial-number of the spectrometer, the date the scan was performed
		and the start-time of the scan from the filename. */
bool CGeometryEvaluator::GetInfoFromFileName(const CString fileName, CDateTime &start, CString &serial){
	CString name, sDate, sTime, resToken;
	int iDate, iTime;
	int curPos = 0;

	// make a local copy of the filename
	name.Format(fileName);

	// remove the name of the path
	Common::GetFileName(name);

	// Tokenize the file-name using the underscores as separators

	// The first part is the serial-number of the spectrometer
	resToken = name.Tokenize("_", curPos);
	if(resToken == "")
		return false;
	serial.Format(resToken);

	// The second part is the date
	resToken = name.Tokenize("_", curPos);
	if(resToken == "")
		return false;
	if (sscanf(resToken, "%d", &iDate) == 1) {
		start.year = (unsigned char)(iDate / 10000);
		start.month = (unsigned char)((iDate - start.year * 10000) / 100);
		start.day = (unsigned char)(iDate % 100);
		start.year += 2000;
	}
	else {
		return false;
	}

	// The third part is the time
	resToken = name.Tokenize("_", curPos);
	if(resToken == "")
		return false;
	if (sscanf(resToken, "%d", &iTime) == 1) {
		start.hour = (unsigned char)(iTime / 100);
		start.minute = (unsigned char)((iTime - start.hour * 100));
		start.second = 0;
	}
	else {
		return false;
	}

	return true;
}

/** Calculate the plume-height using the two scans found in the 
		given evaluation-files. */
RETURN_CODE CGeometryEvaluator::CalculateGeometry(const CString &evalLog1, const CString &evalLog2, int volcanoIndex){
	double plumeHeight, plumeHeightError, windDirection, windDirectionError;
	CString message;
	CDateTime startTime1, startTime2;
	CString serial1, serial2;
	CString fileName, directory;
	Geometry::CGeometryCalculator::CGeometryCalculationInfo *info = new Geometry::CGeometryCalculator::CGeometryCalculationInfo();
	Common common;

	// 0. Tell the world what is about to happen
	ShowMessage("Geometry: Begin calculation of plume-height");

	// 1a. Calculate the plume height
	if(false == CGeometryCalculator::CalculateGeometry(evalLog1, 0, evalLog2, 0, plumeHeight, plumeHeightError, windDirection, windDirectionError, info)){
		delete info;
		return FAIL;
	}
	message.Format("Geometry: Plume height calculated to: %.1lf m above lowest scanner", plumeHeight);
	ShowMessage(message);

	// 1b. Check the result, if it's reasonable or not...
	if(plumeHeightError > 1000.0 || plumeHeightError > (plumeHeight + min(info->scanner[0].m_altitude, info->scanner[1].m_altitude))){
		// the measurement is too bad... Don't even write it down...
		delete info;
		return FAIL;
	}

	// 2. Calculate the average start-time of the two scans
	GetInfoFromFileName(evalLog1, startTime1, serial1);
	GetInfoFromFileName(evalLog2, startTime2, serial2);
	double difference = fabs(CDateTime::Difference(startTime1, startTime2));
	startTime1.Increment((int)difference / 2);

	// 3. Generate a log-file of the successfull geometry calculation

	// 3a. Get the parent-parent-directory of the evaluation-log files
	directory.Format(evalLog1);
	Common::GetDirectory(directory);  // get the directory of the evaluation-log files
	directory = directory.Left((int)strlen(directory) - 1);
	Common::GetDirectory(directory);  // get the parent-directory to the evaluation-log files
	directory = directory.Left((int)strlen(directory) - 1);
	Common::GetDirectory(directory);  // get the parent-parent-directory to the evaluation-log files

	// 3b. Create the geometry log-file if it does not exist
	fileName.Format("%sGeometryLog_%04d.%02d.%02d.txt", (LPCSTR)directory, startTime1.year, startTime1.month, startTime1.day);
	int exists = IsExistingFile(fileName);
	FILE *f = fopen(fileName, "a+");
	if(f == NULL){
		delete info;
		return FAIL;
	}

	// 3c. If the file does not already exist, then create a small header for it
	if(!exists){
		fprintf(f, "# This is the GeometryLog of the NovacProgram version %d.%02d. Built: %s\n", CVersion::majorNumber, CVersion::minorNumber, __DATE__);
		fprintf(f, "# This file contains the result of combining two scans to calculate plume-height and/or wind-direction\n");
		fprintf(f, "Volcano\tEvaluationLog1\tEvaluationLog2\tAverageStartTime\tPlumeCentre1\tPlumeCentre2\tScannerDistance\tCalculatedPlumeHeight\tTotalPlumeHeight\tPlumeHeightError\tWindDirection\tWindDirectionError\n");
	}

	// 3d. Write down our nice calculation
	CString evLogName1, evLogName2, volcanoName;
	evLogName1.Format(evalLog1);		Common::GetFileName(evLogName1);
	evLogName2.Format(evalLog2);		Common::GetFileName(evLogName2);
	volcanoName.Format(g_volcanoes.m_name[volcanoIndex]);
	fprintf(f, "%s\t", (LPCSTR)volcanoName);
	fprintf(f, "%s\t%s\t", (LPCSTR)evLogName1, (LPCSTR)evLogName2);
	fprintf(f, "%02d:%02d:%02d\t", startTime1.hour, startTime1.minute, startTime1.second);
	fprintf(f, "%.1lf\t%.1lf\t",   info->plumeCentre[0], info->plumeCentre[1]);
	fprintf(f, "%.1lf\t",          common.GPSDistance(info->scanner[0].m_latitude, info->scanner[0].m_longitude, info->scanner[1].m_latitude, info->scanner[1].m_longitude));
	if(fabs(plumeHeight) < 1e4){
		fprintf(f, "%.1lf\t", plumeHeight);
		fprintf(f, "%.1lf\t", plumeHeight + min(info->scanner[0].m_altitude, info->scanner[1].m_altitude));
	}else{
		fprintf(f, "%.2e\t",  plumeHeight);
		fprintf(f, "%.2e\t",  plumeHeight + min(info->scanner[0].m_altitude, info->scanner[1].m_altitude));
	}
	if(plumeHeightError < 1e4){
		fprintf(f, "%.1lf\t", plumeHeightError);
	}else{
		fprintf(f, "%.2e\t",  plumeHeightError);
	}
	fprintf(f, "%.1lf\t", windDirection);
	fprintf(f, "%.1lf\n", windDirectionError);

	// 3e. Remember to close the file
	fclose(f);
	ShowMessage("Plume height written to GeometryLog.txt");

	// 3. Tell the world about what we've done
	CGeometryResult *result = new CGeometryResult();
	result->m_date               = startTime1.day;
	result->m_plumeHeight        = plumeHeight;
	result->m_plumeHeightError   = plumeHeightError;
	result->m_windDirection      = windDirection;
	result->m_windDirectionError = windDirectionError;
	result->m_startTime          = startTime1.hour * 3600 + startTime1.minute * 60 + startTime1.second;
	pView->PostMessage(WM_PH_SUCCESS, (WPARAM)result);

	// 4. Try to upload the log-file to the FTP-server
	UploadToNOVACServer(fileName, volcanoIndex);

	delete info;

	return SUCCESS;
}
