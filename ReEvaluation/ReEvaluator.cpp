#include "StdAfx.h"
#include "../NovacMasterProgram.h"
#include "reevaluator.h"

#include "../Common/Version.h"
#include "../Evaluation/ScanEvaluation.h"
#include "../Dialogs/QueryStringDialog.h"
#include <SpectralEvaluation/Utils.h>

using namespace ReEvaluation;
using namespace Evaluation;

CReEvaluator::CReEvaluator(void)
{
	fRun = false;
	m_pause = 0;
	m_sleeping = false;
	
	// initialize the scan file list
	m_scanFileNum = 0;
	m_scanFile.SetSize(64); // <-- The initial guess of the number of pak-files

	// initialize the list of references 
	for(int i = 0; i < MAX_N_WINDOWS; ++i){
		for(int j = 1; j < MAX_N_REFERENCES; ++j){
			m_window[i].ref[j].SetShift(SHIFT_FIX, 0.0);
			m_window[i].ref[j].SetSqueeze(SHIFT_FIX, 1.0);
		}
	}
	m_windowNum = 1;
	m_curWindow = 0;

	pView = NULL;
	m_progress = 0;

	m_statusMsg.Format("");

	m_curScanFile = -1;

	// ignoring
	m_ignore_Lower.m_channel = 1144;
	m_ignore_Lower.m_intensity = 1000.0;
	m_ignore_Lower.m_type = IGNORE_DARK;

	m_ignore_Upper.m_channel = 1144;
	m_ignore_Upper.m_intensity = 4000.0;
	m_ignore_Upper.m_type = IGNORE_NOTHING;

	// The sky spectrum 
	m_skyOption = SKY_FIRST;
	m_skyIndex	= 0;

	// The default is that the spectra are summed, not averaged
	m_averagedSpectra = false;
}

CReEvaluator::~CReEvaluator(void)
{
}

/** Halts the current operation */
bool CReEvaluator::Stop(){
	fRun = false;
	return true;
}

/** This function takes care of the actual evaluation */
bool CReEvaluator::DoEvaluation(){
#ifdef _DEBUG
	// this is for searching for memory leaks
	CMemoryState newMem, oldMem, diffMem;
	oldMem.Checkpoint();
#endif

	long nDone		= 0;		// <-- number of scans evaluated
	double nToDo	= (double)m_scanFileNum;	// <-- number of scans to evaluate

	CString message;

	/* Check the settings before we start */
	if(!MakeInitialSanityCheck())
	{
		return false;
	}

	/** Prepare everything for evaluating */
	if(!PrepareEvaluation())
	{
		return false;
	}

	/* evaluate the spectra */
	m_progress = 0;

	// The CScanEvaluation-object handles the evaluation of one single scan.
	CScanEvaluation ev;
	ev.pView		= this->pView;
	ev.m_pause		= &m_pause;
	ev.m_sleeping	= &m_sleeping;

	// Set the options for the CScanEvaluation object
	ev.SetOption_Sky(m_skyOption, m_skyIndex, &m_skySpectrum);
	ev.SetOption_Ignore(m_ignore_Lower, m_ignore_Upper);
	ev.SetOption_AveragedSpectra(m_averagedSpectra);

	// loop through all the scan files
	for(m_curScanFile = 0; m_curScanFile < m_scanFileNum; ++m_curScanFile)
	{
		m_progress = m_curScanFile / (double)m_scanFileNum;
		if(pView != nullptr)
		{
			pView->PostMessage(WM_PROGRESS, (WPARAM)m_progress);
		}

		// The CScanFileHandler is a structure for reading the spectral information from the scan-file
		FileHandler::CScanFileHandler scan;

		// Check the scan file
        const std::string scanFileName((LPCSTR)m_scanFile[m_curScanFile]);
		if(SUCCESS != scan.CheckScanFile(scanFileName)){
			CString errStr;
			errStr.Format("Could not read scan-file %s", (LPCTSTR)m_scanFile[m_curScanFile]);
			MessageBox(NULL, errStr, "Error", MB_OK);
			continue;
		}

		// update the status window
		m_statusMsg.Format("Evaluating scan number %d", m_curScanFile);
		ShowMessage(m_statusMsg);

		// For each scanfile: loop through the fit windows
		for(m_curWindow = 0; m_curWindow < m_windowNum; ++m_curWindow){
			CFitWindow &thisWindow = m_window[m_curWindow];

			// Check the interlace steps
			CSpectrum skySpec;
			scan.GetSky(skySpec);

			if(skySpec.Channel() > MAX_CHANNEL_NUM) {
				// We should use an interlaced window instead
				if(-1 == Common::GetInterlaceSteps(skySpec.Channel(), skySpec.m_info.m_interlaceStep)) {
					return 0; // WRONG!!
				}

				thisWindow.interlaceStep	= skySpec.m_info.m_interlaceStep;
				thisWindow.specLength		= skySpec.m_length * skySpec.m_info.m_interlaceStep;
			}

			if(skySpec.m_info.m_startChannel > 0 || skySpec.m_length != thisWindow.specLength / thisWindow.interlaceStep) {
				// If the spectra are too short or the start channel is not zero
				//	then they are read out as partial spectra. Lets adapt the evaluator to that
				thisWindow.specLength		= skySpec.m_length;
				thisWindow.startChannel		= skySpec.m_info.m_startChannel;
			}

			if(skySpec.m_info.m_interlaceStep > 1) {
				skySpec.InterpolateSpectrum();
			}

			// check the quality of the sky-spectrum
			if(skySpec.AverageValue(thisWindow.fitLow, thisWindow.fitHigh) >= 4090 * skySpec.NumSpectra()) {
				if(skySpec.NumSpectra() > 0){
					message.Format("It seems like the sky-spectrum is saturated in the fit-region. Continue?");
					if(IDNO == MessageBox(NULL, message, "Saturated sky spectrum?", MB_YESNO)){
						break; // continue with the next scan-file
					}
				}
			}
		
			// Evaluate the scan-file
            // TODO: Don't create an CEvaluation unnecessarily, just create the fit window and pass in
			int success = ev.EvaluateScan(m_scanFile[m_curScanFile], m_evaluator[m_curWindow].FitWindow(), &fRun, &m_darkSettings);

			// Check if the user wants to stop
			if(!fRun) {
				return true;
			}

			// get the result of the evaluation and write them to file
			if(success) {
				std::unique_ptr<CScanResult> res = ev.GetResult();
				AppendResultToEvaluationLog(res.get(), &scan);
			}

		}//end for m_curWindow...
		m_curWindow = 0;

	} // end for(m_curScanFile...

	if(pView != NULL)
	{
		pView->PostMessage(WM_DONE);
	}

	fRun = false;

#ifdef _DEBUG
	// this is for searching for memory leaks
	newMem.Checkpoint();
	if(diffMem.Difference(oldMem, newMem)){
		diffMem.DumpStatistics(); 
//		diffMem.DumpAllObjectsSince();
	}
#endif

	return true;
}

/* Check the settings before we start */
bool CReEvaluator::MakeInitialSanityCheck(){
	// 1. Check so that there are any fit windows defined
	if(this->m_windowNum <= 0)
		return false;

	// 2. Check so that there are not too many fit windows defined
	if(m_windowNum > MAX_N_WINDOWS)
		return false;

	// 3. Check so that there are any spectrum files to evaluate
	if(this->m_scanFileNum <= 0)
		return false;

	return true;
}


//bool CReEvaluator::ReadReferences(CEvaluation *evaluator, CFitWindow &window){
//	CString refFileList[MAX_N_REFERENCES];
//	CString solarSpectrumFile;
//	int ret;
//
//	if(window.nRef == 0){
//		MessageBox(NULL, "No reference files selected, cannot reevaluate", "Error", MB_OK);
//		return false;
//	}
//
//	for(int i = 0; i < window.nRef; ++i){
//		refFileList[i].Format(window.ref[i].m_path);
//	}
//
//	// if we have a solar-spectrum then read that too...
//	if(window.fraunhoferRef.m_path.GetLength() > 6){
//		solarSpectrumFile.Format(window.fraunhoferRef.m_path);
//
//		ret = evaluator->ReadRefList(refFileList, &solarSpectrumFile, window.nRef, MAX_SPECTRUM_LENGTH);
//	}else{
//		// read the reference files
//		ret = evaluator->ReadRefList(refFileList, NULL, window.nRef, MAX_SPECTRUM_LENGTH);
//	}
//	if(!ret){
//		return false;
//	}
//
//	return true;
//}

bool CReEvaluator::CreateOutputDirectory(){
	CString cDateTime, path, fileName;
	struct tm *tim;
	time_t t;

	time(&t);
	tim = localtime(&t);
	cDateTime.Format("%04d%02d%02d_%02d%02d",tim->tm_year+1900,tim->tm_mon+1,tim->tm_mday,tim->tm_hour,tim->tm_min);

	// Get the directory name
	path.Format("%s", (LPCTSTR)m_scanFile[m_curScanFile]);
	fileName.Format("%s", (LPCTSTR)m_scanFile[m_curScanFile]);
	Common::GetDirectory(path); // gets the directory of the scan-file
	Common::GetFileName(fileName);
	m_outputDir.Format("%s\\ReEvaluation_%s_%s", (LPCTSTR)path, (LPCTSTR)fileName, (LPCTSTR)cDateTime);

	// Create the directory
	if(0 == CreateDirectory(m_outputDir, NULL)){
		DWORD errorCode = GetLastError();
		if(errorCode != ERROR_ALREADY_EXISTS){ /* We shouldn't quit just because the directory that we want to create already exists. */	
			CString tmpStr;
			tmpStr.Format("Could not create output directory. Error code returned %ld. Do you want to create an output directory elsewhere?", errorCode);
			int ret = MessageBox(NULL, tmpStr, "Could not create output directory", MB_YESNO);
			if(ret == IDNO)
				return false;
			else{
				// Create the output-directory somewhere else
				Dialogs::CQueryStringDialog pathDialog;
				pathDialog.m_windowText.Format("The path to the output directory?");
				pathDialog.m_inputString = &path;
				INT_PTR ret = pathDialog.DoModal();
				if(IDCANCEL == ret)
					return false;
				m_outputDir.Format("%s\\ReEvaluation_%s_%s", (LPCTSTR)path, (LPCTSTR)fileName, (LPCTSTR)cDateTime);
				if(0 == CreateDirectory(m_outputDir, NULL)){
					tmpStr.Format("Could not create output directory. ReEvaluation aborted.");
					MessageBox(NULL, tmpStr, "ERROR", MB_OK);
					return false;
				}
			}
		}
	}
	return true;
}

bool CReEvaluator::WriteEvaluationLogHeader(){
	CString time, date, name;
	Common::GetDateText(date);
	Common::GetTimeText(time);

	// simplify the syntax a little bit
	CFitWindow &window = m_window[m_curWindow];

	// Get the name of the evaluation log file
	m_evalLog[m_curWindow] = m_outputDir + "\\ReEvaluationLog_" + CString(window.name.c_str()) + ".txt";

	// Try to open the log file
	FILE *f = fopen(m_evalLog[m_curWindow], "w");
	if(f == 0){
		MessageBox(NULL, "Could not create evaluation-log file, evaluation aborted", "FileError", MB_OK);
		return false; // failed to open the file, quit it
	}

	// The common header
	fprintf(f, "#ReEvaluation Log File for the Novac Master Program version %d.%d. Created on: %s at %s\n", CVersion::majorNumber, CVersion::minorNumber, (LPCTSTR)date, (LPCTSTR)time);
	fprintf(f, "#***Settings Used in the Evaluation***\n");

	// Fit interval and polynomial order
	fprintf(f, "#FitFrom=%d\n#FitTo=%d\n#Polynom=%d\n", 
		window.fitLow, window.fitHigh, window.polyOrder);

	// Ignoring the dark spectra?
	if(m_ignore_Lower.m_type == IGNORE_LIMIT)
		fprintf(f, "#Ignore Spectra with intensity below=%.1lf @ channel %d\n", m_ignore_Lower.m_intensity, m_ignore_Lower.m_channel);
	else
		fprintf(f, "#Ignore Dark Spectra\n");

	// Ignoring saturated spectra?
	if(m_ignore_Upper.m_type == IGNORE_LIMIT)
		fprintf(f, "#Ignore Spectra with intensity above=%.1lf @ channel %d\n", m_ignore_Upper.m_intensity, m_ignore_Upper.m_channel);
	else
		fprintf(f, "#Ignore Saturated Spectra\n");

	// The sky-spectrum used
	switch(m_skyOption){
		case SKY_FIRST:						fprintf(f, "#Sky: First spectrum in scanFile\n");	break;
		case SKY_USER:						fprintf(f, "#Sky: %s\n", (LPCTSTR)m_skySpectrum); break;
		case SKY_INDEX:						fprintf(f, "#Sky: Spectrum number %d", m_skyIndex); break;
		case SKY_AVERAGE_OF_GOOD:	fprintf(f, "#Sky: Average of all good spectra in scanFile\n");
	}

	// Write the region used:
	if(window.UV)
		fprintf(f, "#Region: UV\n");
	else
		fprintf(f, "#Region: Visible\n");

	// If the spectra are averaged or not
	if(m_averagedSpectra)
		fprintf(f, "#Spectra are averaged, not summed\n");

	// The reference-files
	fprintf(f, "#nSpecies=%d\n", window.nRef);
	fprintf(f, "#Specie\tShift\tSqueeze\tReferenceFile\n");
	for(int i = 0; i < window.nRef; ++i){
		fprintf(f, "#%s\t", window.ref[i].m_specieName.c_str());
		switch(window.ref[i].m_shiftOption){
			case SHIFT_FIX:
				fprintf(f, "%0.3lf\t", window.ref[i].m_shiftValue); break;
			case SHIFT_LINK:
				fprintf(f, "linked to %s\t", window.ref[(int)window.ref[i].m_shiftValue].m_specieName.c_str()); break;
			case SHIFT_LIMIT:
				fprintf(f, "limited to +-%0.3lf\t", window.ref[i].m_shiftValue);
			default:
				fprintf(f, "free\t"); break;
		}

		switch(window.ref[i].m_squeezeOption){
			case SHIFT_FIX:
				fprintf(f, "%0.3lf\t", window.ref[i].m_squeezeValue); break;
			case SHIFT_LINK:
				fprintf(f, "linked to %s\t", window.ref[(int)window.ref[i].m_squeezeValue].m_specieName.c_str()); break;
			case SHIFT_LIMIT:
				fprintf(f, "limited to +-%0.3lf\t", window.ref[i].m_squeezeValue);
			default:
				fprintf(f, "free\t"); break;
		}
		fprintf(f, "%s\n", window.ref[i].m_path.c_str());
	}
	fprintf(f, "\n");

	fclose(f);

	return true;
}

bool CReEvaluator::AppendResultToEvaluationLog(const CScanResult *result, const FileHandler::CScanFileHandler *scan){
	CString name;
	CSpectrum skySpec;
	Common common;
	double *evResult = 0;
	FILE *f = fopen(m_evalLog[m_curWindow], "a+");
	if(0 == f)
		return false;

	// simplify the syntax a little bit
	CFitWindow &window = m_window[m_curWindow];

	// Write the scan-information to file
	fprintf(f, "<scaninformation>\n");
	fprintf(f, "\tdate=%02d.%02d.%04d\n",                               scan->m_startTime.day, scan->m_startTime.month, scan->m_startTime.year);
	fprintf(f, "\tstarttime=%02d:%02d:%02d\n",				            scan->m_startTime.hour, scan->m_startTime.minute, scan->m_startTime.second);
	fprintf(f, "\tcompass=%.1lf\n",										scan->GetCompass());
	fprintf(f, "\ttilt=%.1lf\n",											skySpec.m_info.m_pitch);

	const CGPSData &gps = scan->GetGPS();
	fprintf(f, "\tlat=%.6lf\n",												gps.m_latitude);
	fprintf(f, "\tlong=%.6lf\n",											gps.m_longitude);
	fprintf(f, "\talt=%.3lf\n",												gps.m_altitude);

	fprintf(f, "\tserial=%s\n",                                             scan->m_device.c_str());
	fprintf(f, "\tchannel=%d\n",											scan->m_channel);

	scan->GetSky(skySpec);
	fprintf(f, "\tconeangle=%.1lf\n",									skySpec.m_info.m_coneAngle);
	fprintf(f, "\tinterlacesteps=%d\n",								scan->GetInterlaceSteps());
	fprintf(f, "\tstartchannel=%d\n",									scan->GetStartChannel());
	fprintf(f, "\tspectrumlength=%d\n",								scan->GetSpectrumLength());

	fprintf(f, "\tbattery=%.2f\n",										skySpec.m_info.m_batteryVoltage);
	fprintf(f, "\ttemperature=%.2f\n",								skySpec.m_info.m_temperature);

	// The mode
	if(result->IsDirectSunMeasurement())
		fprintf(f, "\tmode=direct_sun\n");
	if(result->IsLunarMeasurement())
		fprintf(f, "\tmode=lunar\n");
	else if(result->IsWindMeasurement())
		fprintf(f, "\tmode=wind\n");
	else if(result->IsStratosphereMeasurement())
		fprintf(f, "\tmode=stratospheric\n");
	else if(result->IsCompositionMeasurement())
		fprintf(f, "\tmode=composition\n");
	else
		fprintf(f, "\tmode=plume\n");

	// Finally, the version of the file and the version of the program
	fprintf(f, "\tsoftwareversion=%d.%d\n", CVersion::majorNumber, CVersion::minorNumber);
	fprintf(f, "\tcompiledate=%s\n",					  __DATE__);

	fprintf(f, "</scaninformation>\n");

	// Write the header
	fprintf(f, "#scanangle\tstarttime\tstoptime\tname\tdelta\tchisquare\texposuretime\tnumspec\tintensity\tfitintensity\tisgoodpoint\toffset\tflag\t");
	for(int i = 0; i < window.nRef; ++i){
		name.Format("%s", window.ref[i].m_specieName.c_str());

		fprintf(f, "column(%s)\tcolumnerror(%s)\t", (LPCTSTR)name, (LPCTSTR)name);
		fprintf(f, "shift(%s)\tshifterror(%s)\t", (LPCTSTR)name, (LPCTSTR)name);
		fprintf(f, "squeeze(%s)\tsqueezeerror(%s)\t", (LPCTSTR)name, (LPCTSTR)name);
	}

	if(window.fitType == FIT_HP_SUB || window.fitType == FIT_POLY){
		name.Format("fraunhoferref");

		fprintf(f, "column(%s)\tcolumnerror(%s)\t", (LPCTSTR)name, (LPCTSTR)name);
		fprintf(f, "shift(%s)\tshifterror(%s)\t", (LPCTSTR)name, (LPCTSTR)name);
		fprintf(f, "squeeze(%s)\tsqueezeerror(%s)", (LPCTSTR)name, (LPCTSTR)name);
	}

	fprintf(f, "\n<spectraldata>\n");

	for(unsigned long i = 0; i < result->GetEvaluatedNum(); ++i){
		const CSpectrumInfo &info = result->GetSpectrumInfo(i);

		// The scan angle
		fprintf(f, "%.0f\t",					info.m_scanAngle);

		// The start time
		fprintf(f, "%02d:%02d:%02d\t", info.m_startTime.hour, info.m_startTime.minute, info.m_startTime.second);

		// The stop time
		fprintf(f, "%02d:%02d:%02d\t", info.m_stopTime.hour, info.m_stopTime.minute, info.m_stopTime.second);

		// The name of the spectrum
        const std::string spectrumName = SimplifyString(info.m_name);
		fprintf(f, "%s\t", spectrumName.c_str());

		// The delta of the fit
		fprintf(f, "%.2e\t", result->GetDelta(i));

		// The chi-square of the fit
		fprintf(f, "%.2e\t", result->GetChiSquare(i));

		// The exposure time of the spectrum
		fprintf(f, "%d\t", info.m_exposureTime);

		// The number of spectra averaged
		fprintf(f, "%d\t", info.m_numSpec);

		// The saved intensity
		fprintf(f, "%.0f\t", info.m_peakIntensity);

		// The intensity in the fit region
		fprintf(f, "%.0f\t", info.m_fitIntensity);

		// The quality of the fit
		fprintf(f, "%d\t", result->IsOk(i));

		// The electronic offset
		fprintf(f, "%.1f\t", info.m_offset);

		// The 'flag' - solenoid or diffusor plate
		fprintf(f, "%d\t", info.m_flag);

		//		fprintf(f, "%.0lf\t", spec.MaxValue(m_window[m_curWindow].fitLow, m_window[m_curWindow].fitHigh));

		// the number of references
		int nRef = m_window[m_curWindow].nRef;
		if(window.fitType == FIT_HP_SUB || window.fitType == FIT_POLY)
			++nRef;

		// The Result
		for(int j = 0; j < nRef; ++j){

			fprintf(f, "%.2e\t", result->GetColumn(i, j));
			fprintf(f, "%.2e\t", result->GetColumnError(i, j));
			fprintf(f, "%.2e\t", result->GetShift(i, j));
			fprintf(f, "%.2e\t", result->GetShiftError(i, j));
			fprintf(f, "%.2e\t", result->GetSqueeze(i, j));
			fprintf(f, "%.2e\t", result->GetSqueezeError(i, j));
		}

		fprintf(f, "\n");
	}
	fprintf(f, "</spectraldata>\n");

	fclose(f);

	return true;
}


bool CReEvaluator::PrepareEvaluation(){
	m_curScanFile = 0;

	for(m_curWindow = 0; m_curWindow < m_windowNum; ++m_curWindow){
		/* create the output directory */
		if(!CreateOutputDirectory())
			return false;

        /** Read the references */
        if (!ReadReferences(m_window[m_curWindow]))
        {
            MessageBox(NULL, "Not all references could be read. Please check settings and start again", "Error in settings", MB_OK);
            return false;
        }

		/* Create the evaluator */
		m_evaluator[m_curWindow].SetFitWindow(m_window[m_curWindow]);

		/* then create the evaluation log and write its header */
		if(!WriteEvaluationLogHeader())
			return false; 
	}

	return true;
}

void CReEvaluator::SortScans(){
	CString	tmp;
	bool change;

    // TODO: Change this. Use std::sort?
	do{
		change = false;
		for(int k = 0; k < m_scanFileNum - 1; ++k){
			CString &str1 = m_scanFile.GetAt(k);
			CString &str2 = m_scanFile.GetAt(k+1);

			if(str1.Compare(str2) > 0){
				tmp					=	m_scanFile.GetAt(k); //copy
				m_scanFile.SetAt(k,  m_scanFile.GetAt(k+1));
				m_scanFile.SetAt(k+1,tmp);
				change = true;
			}else{
				continue;
			}
		}
	}while(change);
}