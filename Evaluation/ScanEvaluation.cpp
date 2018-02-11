#include "StdAfx.h"
#include "scanevaluation.h"

#include "../Common/Spectra/Spectrum.h"
#include "../Common/Spectra/SpectrumIO.h"
#include "../Common/SpectrumFormat/STDFile.h"
#include "../Common/SpectrumFormat/TXTFile.h"

using namespace Evaluation;

CScanEvaluation::CScanEvaluation(void)
{
	m_result    = nullptr;
	pView       = NULL;
	m_skyOption = SKY_FIRST;
	m_skyIndex  = 0;
	m_pause     = NULL;
	m_sleeping  = NULL;

	// In real-time, nothing should be ignored
	m_ignore_Lower.m_type = IGNORE_NOTHING;
	m_ignore_Upper.m_type = IGNORE_NOTHING;

	// default is that the spectra are summed, not averaged
	m_averagedSpectra = false;
}

CScanEvaluation::~CScanEvaluation(void)
{
	pView = NULL;
}

std::unique_ptr<CScanResult> CScanEvaluation::GetResult()
{
	std::lock_guard<std::mutex> lock{ m_resultMutex };
	std::unique_ptr<CScanResult> copiedResult;

	if(nullptr != m_result.get())
	{
		copiedResult.reset(new CScanResult(*m_result.get()));
	}

	return copiedResult;
}

bool CScanEvaluation::HasResult()
{
	std::lock_guard<std::mutex> lock{ m_resultMutex };

	return (nullptr != m_result.get());
}

/** Called to evaluate one scan */
long CScanEvaluation::EvaluateScan(const CString &scanfile, CEvaluation *eval, bool *fRun, const CConfigurationSetting::DarkSettings *darkSettings){
	
#ifdef _DEBUG
	// this is for searching for memory leaks
	CMemoryState newMem, oldMem, diffMem;
	oldMem.Checkpoint();
#endif

	CString message;	// used for ShowMessage messages
	int	index = 0;		// keeping track of the index of the current spectrum into the .pak-file
	double highestColumn = 0.0;	// the highest column-value in the evaluation
	bool success = true;

	// variables for storing the sky, dark and the measured spectra
	CSpectrum sky, dark, current;

	// Remember the fit-range
	m_fitLow  = eval->m_window.fitLow;
	m_fitHigh = eval->m_window.fitHigh;

	// Check so that the file exists
	if(!IsExistingFile(scanfile)) {
		return 0;
	}

	// The CScanFileHandler is a structure for reading the spectral information 
	//  from the scan-file
	FileHandler::CScanFileHandler scan;  

	// Check the scan file, make sure it's correct and that the file
	//	actually contains spectra
	if(SUCCESS != scan.CheckScanFile(&scanfile)){
		return 0;
	}

	// make a backup of the fit window (this function may make some changes to the
	//  fit window, and we should be able to restore the old values on return).
	CFitWindow backupWindow = eval->m_window;

	// If the user wants to find optimum shift, then the scan shall be evaluated
	//  once with all shifts set to 0 and all squeeze set to 1.
	if(eval->m_window.findOptimalShift){
		for(int k = 0; k < eval->m_window.nRef; ++k){
			eval->m_window.ref[k].m_shiftOption   = SHIFT_FIX;
			eval->m_window.ref[k].m_squeezeOption = SHIFT_FIX;
			eval->m_window.ref[k].m_shiftValue    = 0.0;
			eval->m_window.ref[k].m_squeezeValue  = 1.0;
		}
	}

	// Get the sky and dark spectra and divide them by the number of 
	//     co-added spectra in it
	if(SUCCESS != GetSky(&scan, sky)){
		//if(logFileWriter != NULL)
		//	logFileWriter->WriteErrorMessage("Error in evaluation: Cannot read sky spectrum from file");
		return 0;
	}
	CSpectrum original_sky = sky; // original_sky is the sky-spectrum without dark-spectrum corrections...

	if(m_skyOption != SKY_USER){
		// Get the dark-spectrum and remove it from the sky
		if(SUCCESS != GetDark(&scan, sky, dark, darkSettings)){
			return 0;
		}
		sky.Sub(dark);
	}

	if(sky.NumSpectra() > 0 && !m_averagedSpectra){
		sky.Div(sky.NumSpectra());
		original_sky.Div(original_sky.NumSpectra());
	}

	// Get some important information about the spectra, like
	//	interlace steps, spectrum length and start-channel
	eval->m_window.interlaceStep	= scan.GetInterlaceSteps();
	eval->m_window.specLength		= scan.GetSpectrumLength() * eval->m_window.interlaceStep;
	eval->m_window.startChannel		= scan.GetStartChannel();

	// Adjust the fit-low and fit-high parameters according to the spectra
	m_fitLow  = eval->m_window.fitLow  - eval->m_window.startChannel;
	m_fitHigh = eval->m_window.fitHigh	- eval->m_window.startChannel;

	// If we have a solar-spectrum that we can use to determine the shift
	//	& squeeze then fit that first so that we know the wavelength calibration
	if(eval->m_window.fraunhoferRef.m_path.GetLength() > 4){
		FindOptimumShiftAndSqueeze_Fraunhofer(eval, &scan);
	}

	// if wanted, include the sky spectrum into the fit
	if(eval->m_window.fitType == FIT_HP_SUB || eval->m_window.fitType == FIT_POLY){
		IncludeSkySpecInFit(eval, sky, eval->m_window);
	}

	// the data structure to keep track of the evaluation results
	std::shared_ptr<CScanResult> newResult = std::make_shared<CScanResult>();

	// Check weather we are to find an optimal shift and squeeze
	int nIt = (eval->m_window.findOptimalShift == FALSE) ? 1 : 2;

	// Evaluate the scan (one or two times, depending on the settings)
	for(int iteration = 0; iteration < nIt; ++iteration){

		index = -1; // we're at spectrum number 0 in the .pak-file
		m_indexOfMostAbsorbingSpectrum = -1;	// as far as we know, there's no absorption in any spectrum...

		newResult->SetSkySpecInfo(original_sky.m_info);
		newResult->SetDarkSpecInfo(dark.m_info);

		// Make sure that we'll start with the first spectrum in the scan
		scan.ResetCounter();

		// Evaluate all the spectra in the scan.
		while(1){
			success = true; // assume that we will succeed in evaluating this spectrum

			// If the user wants to exit this thread then do so.
			if(fRun != nullptr && *fRun == false){
				ShowMessage("Scan Evaluation cancelled by user");
				return 0;
			}

			// remember which spectrum we're at
			int	spectrumIndex = current.ScanIndex();

			// a. Read the next spectrum from the file
			int ret = scan.GetNextSpectrum(current);

			if(ret == 0){
				// if something went wrong when reading the spectrum
				if(scan.m_lastError == SpectrumIO::CSpectrumIO::ERROR_SPECTRUM_NOT_FOUND || scan.m_lastError == SpectrumIO::CSpectrumIO::ERROR_EOF){
					// at the end of the file, quit the 'while' loop
					break;
				}else{
					CString errMsg;
					errMsg.Format("Faulty spectrum found in %s", scanfile);
					switch(scan.m_lastError){
						case SpectrumIO::CSpectrumIO::ERROR_CHECKSUM_MISMATCH:
							errMsg.AppendFormat(", Checksum mismatch. Spectrum ignored"); break;
						case SpectrumIO::CSpectrumIO::ERROR_DECOMPRESS:
							errMsg.AppendFormat(", Decompression error. Spectrum ignored"); break;
						default:
							ShowMessage(", Unknown error. Spectrum ignored");
					}
					ShowMessage(errMsg);
					// remember that this spectrum is corrupted
					newResult->MarkAsCorrupted(spectrumIndex);
					continue;
				}
			}

			++index;	// we'have just read the next spectrum in the .pak-file

			// If the read spectrum is the sky or the dark spectrum, 
			//	then don't evaluate it...
			if(current.ScanIndex() == sky.ScanIndex() || current.ScanIndex() == dark.ScanIndex()){
				continue;
			}

			// If the spectrum is read out in an interlaced way then interpolate it back to it's original state
			if(current.m_info.m_interlaceStep > 1)
				current.InterpolateSpectrum();

			// b. Get the dark spectrum for this measured spectrum
			if(SUCCESS != GetDark(&scan, current, dark, darkSettings)){
				return 0;
			}

			// b. Calculate the intensities, before we divide by the number of spectra
			//		and before we subtract the dark
			current.m_info.m_peakIntensity = (float)current.MaxValue(0, current.m_length - 2);
			current.m_info.m_fitIntensity  = (float)current.MaxValue(m_fitLow, m_fitHigh);

			// c. Divide the measured spectrum with the number of co-added spectra
			//     The sky and dark spectra should already be divided before this loop.
			if(current.NumSpectra() > 0 && !m_averagedSpectra)
				current.Div(current.NumSpectra());

			// d. Check if this spectrum is worth evaluating
			if(Ignore(current, eval->m_window)){
				message.Format("Ignoring spectrum %d in scan %s.", current.ScanIndex(), scan.GetFileName());
				ShowMessage(message);
				continue;
			}

			// d2. Now subtract the dark (if we did this earlier, then the 'Ignore' - function would
			//		not function properly)
			if(dark.NumSpectra() > 0 && !m_averagedSpectra)
				dark.Div(dark.NumSpectra());
			current.Sub(dark);

			// e. Evaluate the spectrum
			if(eval->Evaluate(sky, current)){
				CString str;
				str.Format("Failed to evaluate spectrum from spectrometer %s. Failure at spectrum %d in scan containing %d spectra",
					current.m_info.m_device, current.ScanIndex(), current.SpectraPerScan());
				ShowMessage(str);
				success = false;
			}

			// e. Save the evaluation result
			newResult->AppendResult(eval->GetEvaluationResult(), current.m_info);

			// f. Check if this was an ok data point (CScanResult)
			newResult->CheckGoodnessOfFit(current.m_info);

			// g. If it is ok, then check if the value is higher than any of the previous ones
			if(newResult->IsOk(newResult->GetEvaluatedNum()-1) && fabs(newResult->GetColumn(newResult->GetEvaluatedNum()-1, 0)) > highestColumn){
				highestColumn = fabs(newResult->GetColumn(newResult->GetEvaluatedNum()-1, 0));
				m_indexOfMostAbsorbingSpectrum	= index;
			}

			// h. Update the screen (if any)
			if(success && pView != NULL){
				std::lock_guard<std::mutex> lock{ m_resultMutex };
				m_result = newResult;

				ShowResult(current, eval, index, scan.GetSpectrumNumInFile());
			}

			// i. If the user wants us to sleep between each evaluation. Do so...
			if(m_pause != NULL && *m_pause == 1 && m_sleeping != NULL){
				CWinThread *thread = AfxGetThread();
				*m_sleeping = true;
				if(pView != 0)
					pView->PostMessage(WM_GOTO_SLEEP);
				thread->SuspendThread();
				*m_sleeping = false;
			}
		} // end while(1)

		// end of scan...
		if((iteration == 0) && (eval->m_window.findOptimalShift == TRUE)){
			FindOptimumShiftAndSqueeze(eval, &scan, newResult.get());
		}

	}//

	// restore the fit window
	eval->m_window = backupWindow;

#ifdef _DEBUG
	// this is for searching for memory leaks
	newMem.Checkpoint();
	if(diffMem.Difference(oldMem, newMem)){
		diffMem.DumpStatistics(); 
//    diffMem.DumpAllObjectsSince();
	}
#endif

	return m_result->GetEvaluatedNum();
}

/** Includes the sky spectrum into the fit */
bool  CScanEvaluation::IncludeSkySpecInFit(CEvaluation *eval, const CSpectrum &skySpectrum, CFitWindow &window){
	double sky[MAX_SPECTRUM_LENGTH];

	// first make a local copy of the sky spectrum
	CSpectrum tmpSpec = skySpectrum;

	int specLen = tmpSpec.m_length;

	// Remove any remaining offset of the sky-spectrum
	eval->RemoveOffset(tmpSpec.m_data, specLen, window.UV);

	// High-pass filter the sky-spectrum
	if(window.fitType == FIT_HP_SUB)
		eval->HighPassBinomial(tmpSpec.m_data, specLen, 500);

	// Logaritmate the sky-spectrum
	eval->Log(tmpSpec.m_data, specLen);

	memcpy(sky, tmpSpec.m_data, specLen*sizeof(double));

	// Include the spectrum into the fit
	eval->IncludeAsReference(sky, specLen, window.nRef);

	// set the shift and squeeze
	window.ref[window.nRef].m_columnOption		= SHIFT_FIX;
	window.ref[window.nRef].m_columnValue			= (window.fitType == FIT_POLY) ? -1.0 : 1.0;
	if(window.shiftSky == TRUE){
		window.ref[window.nRef].m_shiftOption   = SHIFT_LIMIT;
		window.ref[window.nRef].m_shiftMaxValue = 3.0;
		window.ref[window.nRef].m_shiftValue		= -3.0;
		window.ref[window.nRef].m_squeezeOption = SHIFT_LIMIT;
		window.ref[window.nRef].m_squeezeMaxValue= 1.05;
		window.ref[window.nRef].m_squeezeValue	= 0.95;
	}else{
		window.ref[window.nRef].m_shiftOption   = SHIFT_FIX;
		window.ref[window.nRef].m_shiftValue		= 0.0;
		window.ref[window.nRef].m_squeezeOption = SHIFT_FIX;
		window.ref[window.nRef].m_squeezeValue	= 1.0;
	}

	// set the name of the reference
	window.ref[window.nRef].m_specieName.Format("FraunhoferRef");

	++window.nRef;
	return true;
}

void CScanEvaluation::ShowResult(const CSpectrum &spec, const CEvaluation *eval, long curSpecIndex, long specNum){
	if(pView == NULL)
		return;

	int fitLow	= eval->m_window.fitLow  - spec.m_info.m_startChannel;
	int fitHigh = eval->m_window.fitHigh - spec.m_info.m_startChannel;

	// copy the spectrum
	m_spec[0] = spec;

	// copy the residual and the polynomial
	for(int i = fitLow; i < fitHigh; ++i){
		m_spec[1].m_data[i] = eval->m_residual.GetAt(i - fitLow);	// m_spec[1] is the residual
		m_spec[2].m_data[i] = eval->m_fitResult[0].GetAt(i);		// m_spec[2] is the polynomial
	}


	// copy the scaled referencefiles
	for(int tmpRefIndex = 0; tmpRefIndex < eval->m_window.nRef; ++tmpRefIndex){
		for(int i = fitLow; i < fitHigh; ++i){
			m_spec[tmpRefIndex + 3].m_data[i] = eval->m_fitResult[tmpRefIndex+1].GetAt(i);
		}
	}

	{
		CScanResult* copiedResult = new CScanResult(*m_result.get());
		pView->PostMessage(WM_EVAL_SUCCESS, (WPARAM)&m_spec[0], (LPARAM)copiedResult);
	}

	m_prog_SpecCur = curSpecIndex;
	m_prog_SpecNum = specNum;
	pView->PostMessage(WM_PROGRESS2, (WPARAM)&m_prog_SpecCur, (LPARAM)&m_prog_SpecNum);
}

RETURN_CODE CScanEvaluation::GetDark(FileHandler::CScanFileHandler *scan, const CSpectrum &spec, CSpectrum &dark, const CConfigurationSetting::DarkSettings *darkSettings){
	CString message;
	CSpectrum offset, darkCurrent, offset_dc;
	bool offsetCorrectDC = true; // this is true if the dark current spectrum should be offset corrected

	// 1. The user wants to take the dark spectrum directly from the measurement
	//		as the second spectrum in the scan.
	if(darkSettings == NULL || darkSettings->m_darkSpecOption == MEASURE || darkSettings->m_darkSpecOption == MODEL_SOMETIMES){
		if(0 != scan->GetDark(dark)){
			message.Format("Could not read dark-spectrum from scan %s", scan->GetFileName());
			ShowMessage(message);
			return FAIL;
		}

		// if there is no dark spectrum but one offset and one dark-current spectrum,
		//	then read those instead and model the dark spectrum
		if(dark.m_length == 0){
			scan->GetOffset(offset);
			scan->GetDarkCurrent(darkCurrent);
			if(offset.m_length == darkCurrent.m_length && offset.m_length > 0){
				// 3c-1 Scale the offset spectrum to the measured
				offset.Mult(spec.NumSpectra() / (double)offset.NumSpectra());
				offset.m_info.m_numSpec = spec.NumSpectra();

				// 3c-2 Remove offset from the dark-current spectrum
				if(offsetCorrectDC){
					offset_dc.Mult(darkCurrent.NumSpectra() / (double)offset_dc.NumSpectra());
					darkCurrent.Sub(offset_dc);
				}

				// 3c-3 Scale the dark-current spectrum to the measured
				darkCurrent.Mult((spec.NumSpectra() * spec.ExposureTime()) / (double)(darkCurrent.NumSpectra() * darkCurrent.ExposureTime()));
				darkCurrent.m_info.m_numSpec = spec.NumSpectra();

				// 3d. Make the dark-spectrum
				dark.Clear();
				dark.m_length = offset.m_length;
				dark.Add(offset);
				dark.Add(darkCurrent);

				ShowMessage("Warning: Incorrect settings: check settings for dark current correction");
				return SUCCESS;
			}else{
				ShowMessage("WARNING: NO DARK SPECTRUM FOUND IN SCAN. INCORRECT DARK CURRENT CORRECTION");
				return SUCCESS;
			}
		}

		// If the dark-spectrum is read out in an interlaced way then interpolate it back to it's original state
		if(dark.m_info.m_interlaceStep > 1){
			dark.InterpolateSpectrum();
		}

		// Check so that the exposure-time of the dark-spectrum is same as the 
		//	exposure time of the measured spectrum
		if(dark.ExposureTime() != spec.ExposureTime()){
			ShowMessage("WARNING: EXPOSURE-TIME OF DARK-SPECTRUM IS NOT SAME AS FOR MEASURED SPECTRUM. INCORRECT DARK-CORRECTION!!");
		}

		// Make sure that there are the same number of exposures in the
		//	dark-spectrum as in the measured spectrum
		if(dark.NumSpectra() != spec.NumSpectra()){
			dark.Mult(spec.NumSpectra() / (double)dark.NumSpectra());
		}

		return SUCCESS;
	}

	// 3. The user wants to model the dark spectrum
	if(darkSettings->m_darkSpecOption == MODEL_ALWAYS){
		// 3a. Get the offset spectrum
		if(darkSettings->m_offsetOption == USER_SUPPLIED){
			if(strlen(darkSettings->m_offsetSpec) < 3)
				return FAIL;
			if(FAIL == SpectrumIO::CSTDFile::ReadSpectrum(offset, darkSettings->m_offsetSpec)){
				if(FAIL == SpectrumIO::CTXTFile::ReadSpectrum(offset, darkSettings->m_offsetSpec))
					return FAIL;
			}
		}else{
			scan->GetOffset(offset);
		}
		offset_dc = offset;

		// 3b. Get the dark-current spectrum
		if(darkSettings->m_darkCurrentOption == USER_SUPPLIED){
			if(strlen(darkSettings->m_darkCurrentSpec) < 3)
				return FAIL;
			if(FAIL == SpectrumIO::CSTDFile::ReadSpectrum(darkCurrent, darkSettings->m_darkCurrentSpec)){
				if(FAIL == SpectrumIO::CTXTFile::ReadSpectrum(darkCurrent, darkSettings->m_darkCurrentSpec))
					return FAIL;
			}
			offsetCorrectDC = false;
		}else{
			scan->GetDarkCurrent(darkCurrent);
		}

		// 3c-1 Scale the offset spectrum to the measured
		offset.Mult(spec.NumSpectra() / (double)offset.NumSpectra());
		offset.m_info.m_numSpec = spec.NumSpectra();

		// 3c-2 Remove offset from the dark-current spectrum
		if(offsetCorrectDC){
			offset_dc.Mult(darkCurrent.NumSpectra() / (double)offset_dc.NumSpectra());
			darkCurrent.Sub(offset_dc);
		}

		// 3c-3 Scale the dark-current spectrum to the measured
		darkCurrent.Mult((spec.NumSpectra() * spec.ExposureTime()) / (double)(darkCurrent.NumSpectra() * darkCurrent.ExposureTime()));
		darkCurrent.m_info.m_numSpec = spec.NumSpectra();

		// 3d. Make the dark-spectrum
		dark.Clear();
		dark.m_length								= offset.m_length;
		dark.m_info.m_interlaceStep = offset.m_info.m_interlaceStep;
		dark.m_info.m_channel				= offset.m_info.m_channel;
		dark.Add(offset);
		dark.Add(darkCurrent);

		// If the dark-spectrum is read out in an interlaced way then interpolate it back to it's original state
		if(dark.m_info.m_interlaceStep > 1){
			dark.InterpolateSpectrum();
		}

		return SUCCESS;
	}

	// 4. The user has his own favourite dark-spectrum that he wants to use
	if(darkSettings->m_darkSpecOption == DARK_USER_SUPPLIED){
		// Try to read the spectrum
		if(strlen(darkSettings->m_offsetSpec) < 3)
			return FAIL;
		if(FAIL == SpectrumIO::CSTDFile::ReadSpectrum(dark, darkSettings->m_offsetSpec)){
			if(FAIL == SpectrumIO::CTXTFile::ReadSpectrum(dark, darkSettings->m_offsetSpec))
				return FAIL;
		}

		// If the dark-spectrum is read out in an interlaced way then interpolate it back to it's original state
		if(dark.m_info.m_interlaceStep > 1){
			dark.InterpolateSpectrum();
		}

		return SUCCESS;
	}

	// something is not implemented
	return FAIL;
}

/** This returns the sky spectrum that is to be used in the fitting. */
RETURN_CODE CScanEvaluation::GetSky(FileHandler::CScanFileHandler *scan, CSpectrum &sky){
	CString errorMsg;

	// If the sky spectrum is the first spectrum in the scan
	if(m_skyOption == SKY_FIRST){
		scan->GetSky(sky);

		if(sky.m_info.m_interlaceStep > 1)
			sky.InterpolateSpectrum();

		return SUCCESS;
	}

	// If the sky spectrum is the average of all credible spectra
	if(m_skyOption == SKY_AVERAGE_OF_GOOD){
		int interlaceSteps = scan->GetInterlaceSteps();
		int startChannel	 = scan->GetStartChannel();
		int fitLow	= m_fitLow	/ interlaceSteps - startChannel;
		int fitHigh = m_fitHigh / interlaceSteps - startChannel;

		CSpectrum tmp;
		scan->GetSky(tmp);
		scan->ResetCounter();
		SpecData intens = tmp.MaxValue(fitLow, fitHigh);
		if(intens < 4095*tmp.NumSpectra() && !tmp.IsDark())
			sky = tmp;
		else
			sky.Clear();
		while(scan->GetNextSpectrum(tmp)){
			intens = tmp.MaxValue(fitLow, fitHigh);
		if(intens < 4095*tmp.NumSpectra() && !tmp.IsDark())
				sky.Add(tmp);
		}
		scan->ResetCounter();

		if(sky.m_info.m_interlaceStep > 1)
			sky.InterpolateSpectrum();
		
		return SUCCESS;
	}

	// If the user wants to use another spectrum than 'sky' as reference-spectrum...
	if(m_skyOption == SKY_INDEX){
		if(0 == scan->GetSpectrum(sky, m_skyIndex))
			return FAIL;

		if(sky.m_info.m_interlaceStep > 1)
			sky.InterpolateSpectrum();
		
		return SUCCESS;
	}

	// If the user has supplied a special sky-spectrum to use
	if(m_skyOption == SKY_USER){
		if(Equals(m_userSkySpectrum.Right(4), ".pak", 4)){
			// If the spectrum is in .pak format
			SpectrumIO::CSpectrumIO reader;
			return reader.ReadSpectrum(m_userSkySpectrum, 0, sky);
		}else if(Equals(m_userSkySpectrum.Right(4), ".std", 4)){
			// If the spectrum is in .std format
			return SpectrumIO::CSTDFile::ReadSpectrum(sky, m_userSkySpectrum);
		}else{
			// If we don't recognize the sky-spectrum format
			errorMsg.Format("Unknown format for sky spectrum. Please use .pak or .std");
			ShowMessage(errorMsg);
			MessageBox(NULL, errorMsg, "Error", MB_OK);
			return FAIL;
		}
	}

	return FAIL;
}

/** Setting the option for how to get the sky spectrum */
void  CScanEvaluation::SetOption_Sky(SKY_OPTION skyOption, long skyIndex, const CString *skySpecPath){
	if(skySpecPath != NULL)
		m_userSkySpectrum.Format("%s", *skySpecPath);

	if(skyOption == SKY_USER && skySpecPath == NULL)
		m_skyOption = SKY_FIRST;
	else
		m_skyOption = skyOption;

	m_skyIndex = skyIndex;
}

void  CScanEvaluation::SetOption_Ignore(IgnoreOption lowerLimit, IgnoreOption upperLimit){
	this->m_ignore_Lower = lowerLimit;
	this->m_ignore_Upper = upperLimit;
}

/** Setting the option for wheather the spectra are averaged or not. */
void	CScanEvaluation::SetOption_AveragedSpectra(bool averaged){
	this->m_averagedSpectra = averaged;
}

/** Returns true if the spectrum should be ignored */
bool CScanEvaluation::Ignore(const CSpectrum &spec, const CFitWindow window){
	bool ret = false;

	// Dark spectra
	if(m_ignore_Lower.m_type == IGNORE_DARK){
		ret = spec.IsDark();
	}

	if(m_ignore_Lower.m_type == IGNORE_LIMIT){
		ret = (spec.AverageValue((m_ignore_Lower.m_channel- 10) , (m_ignore_Lower.m_channel + 10) ) < m_ignore_Lower.m_intensity);
	}

	// Saturated spectra
	if(m_ignore_Upper.m_type == IGNORE_DARK){
		ret |= (spec.MaxValue(window.fitLow, window.fitHigh) >= 4000);
	}
	if(m_ignore_Upper.m_type == IGNORE_LIMIT){
		ret |= (spec.AverageValue((m_ignore_Upper.m_channel - 10) , (m_ignore_Upper.m_channel + 10) ) > m_ignore_Upper.m_intensity);
	}

	return ret;
}


/** Finds the optimum shift and squeeze for an evaluated scan */
void CScanEvaluation::FindOptimumShiftAndSqueeze(CEvaluation *eval, FileHandler::CScanFileHandler *scan, CScanResult *result){
	int k;
	CSpectrum spec, sky, dark;
	int specieNum = 0;
	CString message;

	// 1. Find the spectrum with the highest column value
	if(m_indexOfMostAbsorbingSpectrum < 0){
		ShowMessage("Could not determine optimal shift & squeeze. No good spectra in scan.");
		return; // <-- no good data-point found. Quit it!
	}

	// 2. Make sure that this spectrum was ok and that the column-value is high enough
	//double columnError		= result->GetColumnError(m_indexOfMostAbsorbingSpectrum, specieNum); // <-- the column error that corresponds to the highest column-value
	//double highestColumn	= result->GetColumn(m_indexOfMostAbsorbingSpectrum, specieNum);
	//if(highestColumn < 2 * columnError){
	//	ShowMessage("Could not determine optimal shift & squeeze. Maximum column is too low.");
	//	return;
	//}

	// 2. Tell the user
	message.Format("ReEvaluating spectrum number %d to determine optimum shift and squeeze", m_indexOfMostAbsorbingSpectrum);
	ShowMessage(message);

	// 3. Evaluate this spectrum again with free (and linked) shift
	eval->m_window.ref[0].m_shiftOption	 = SHIFT_FREE;
	eval->m_window.ref[0].m_squeezeOption = SHIFT_FIX;
	eval->m_window.ref[0].m_squeezeValue  = 1.0;
	for(k = 1; k < eval->m_window.nRef; ++k){
		if(Equals(eval->m_window.ref[k].m_specieName, "FraunhoferRef"))
			continue;

		eval->m_window.ref[k].m_shiftOption   = SHIFT_LINK;
		eval->m_window.ref[k].m_squeezeOption = SHIFT_LINK;
		eval->m_window.ref[k].m_shiftValue    = 0.0;
		eval->m_window.ref[k].m_squeezeValue  = 0.0;
	}
	// Get the sky-spectrum
	GetSky(scan, sky);
	if(sky.NumSpectra() > 0  && !m_averagedSpectra)
		sky.Div(sky.NumSpectra());

	// Get the dark-spectrum
	GetDark(scan, sky, dark);
	if(dark.NumSpectra() > 0  && !m_averagedSpectra)
		dark.Div(dark.NumSpectra());

	// Subtract the dark...
	sky.Sub(dark);

	// Get the measured spectrum
	scan->GetSpectrum(spec, 2 + m_indexOfMostAbsorbingSpectrum); // The two comes from the sky and the dark spectra in the beginning
	if(spec.m_info.m_interlaceStep > 1)
		spec.InterpolateSpectrum();
	if(spec.NumSpectra() > 0  && !m_averagedSpectra)
		spec.Div(spec.NumSpectra());

	// Get the dark-spectrum and remove it
	GetDark(scan, spec, dark);
	spec.Sub(dark);

	// Evaluate
	eval->Evaluate(sky, spec, 5000);

	// 4. See what the optimum value for the shift turned out to be.
	CEvaluationResult newResult = eval->GetEvaluationResult();
	double optimumShift		= newResult.m_ref[0].m_shift;
	double optimumSqueeze = newResult.m_ref[0].m_squeeze;


	// 5. Set the shift for all references to this value
	for(k = 0; k < eval->m_window.nRef; ++k){
		if(Equals(eval->m_window.ref[k].m_specieName, "FraunhoferRef"))
			continue;

		eval->m_window.ref[k].m_shiftOption     = SHIFT_FIX;
		eval->m_window.ref[k].m_squeezeOption   = SHIFT_FIX;
		eval->m_window.ref[k].m_shiftValue      = optimumShift;
		eval->m_window.ref[k].m_squeezeValue    = optimumSqueeze;
	}

	// 6. We're done!
	message.Format("Optimum shift set to : %.2lf. Optimum squeeze set to: %.2lf ", optimumShift, optimumSqueeze);
	ShowMessage(message);
	return;
}

void CScanEvaluation::FindOptimumShiftAndSqueeze_Fraunhofer(CEvaluation *eval, FileHandler::CScanFileHandler *scan){
	CFitWindow backupWindow = eval->m_window;
	double shift, shiftError, squeeze, squeezeError;
	long fitLow = backupWindow.fitLow;
	long fitHigh= backupWindow.fitHigh;
	CSpectrum spectrum, dark;
	CString message;
	double fitIntensity, fitSaturation, maxInt;
	double bestSaturation = -1.0;
	int curIndex = 0;
	const int INDEX_OF_SKYSPECTRUM	= -1;
	const int NO_SPECTRUM_INDEX			= -2;

	// 1. Find the spectrum for which we should determine shift & squeeze
	//			This spectrum should have high enough intensity in the fit-region
	//			without being saturated.
	int indexOfMostSuitableSpectrum = NO_SPECTRUM_INDEX;
	scan->GetSky(spectrum);
	fitIntensity		= spectrum.MaxValue(fitLow, fitHigh);
	maxInt					= CSpectrometerModel::GetMaxIntensity(spectrum.m_info.m_specModel);
	if(spectrum.NumSpectra() > 0){
		fitSaturation	= fitIntensity / (spectrum.NumSpectra() * maxInt);
	}else{
		int numSpec		= floor(spectrum.MaxValue() / maxInt); // a guess for the number of co-adds
		fitSaturation = fitIntensity / (maxInt * spectrum.NumSpectra());
	}
	if(fitSaturation < 0.9 && fitSaturation > 0.1){
		indexOfMostSuitableSpectrum = INDEX_OF_SKYSPECTRUM; // sky-spectrum
		bestSaturation							= fitSaturation;
	}
	scan->ResetCounter(); // start from the beginning
	while(scan->GetNextSpectrum(spectrum)){
		fitIntensity		= spectrum.MaxValue(fitLow, fitHigh);
		maxInt					= CSpectrometerModel::GetMaxIntensity(spectrum.m_info.m_specModel);

		// Get the saturation-ratio for this spectrum
		if(spectrum.NumSpectra() > 0){
			fitSaturation	= fitIntensity / (spectrum.NumSpectra() * maxInt);
		}else{
			int numSpec		= floor(spectrum.MaxValue() / maxInt); // a guess for the number of co-adds
			fitSaturation = fitIntensity / (maxInt * spectrum.NumSpectra());
		}

		// Check if this spectrum is good...
		if(fitSaturation < 0.9 && fitSaturation > 0.1){
			if(fitSaturation > bestSaturation){
				indexOfMostSuitableSpectrum = curIndex;
				bestSaturation							= fitSaturation;
			}
		}

		// Go to the next spectrum
		++curIndex;
	}

	// 2. Get the spectrum we should evaluate...
	if(indexOfMostSuitableSpectrum == NO_SPECTRUM_INDEX){
		return; // we could not find any good spectrum to use...
	}else if(indexOfMostSuitableSpectrum == INDEX_OF_SKYSPECTRUM){
		scan->GetSky(spectrum);
		message.Format("Determining shift and squeeze from sky-spectrum");
	}else{
		scan->GetSpectrum(spectrum, indexOfMostSuitableSpectrum);
		message.Format("Determining shift and squeeze from spectrum %d", indexOfMostSuitableSpectrum);
	}
	if(spectrum.NumSpectra() > 0 && !m_averagedSpectra)
		spectrum.Div(spectrum.NumSpectra());
	if(SUCCESS != GetDark(scan, spectrum, dark)){
		return; // fail
	}
	if(dark.NumSpectra() > 0 && !m_averagedSpectra)
		dark.Div(dark.NumSpectra());
	spectrum.Sub(dark);

	ShowMessage(message);

	// 3. Do the evaluation.
	if(eval->EvaluateShift(spectrum, backupWindow, shift, shiftError, squeeze, squeezeError)){
		// We failed to make the fit, what shall we do now??
		ShowMessage("Failed to determine shift and squeeze. Will proceed with default parameters.");
	}else{
		if(fabs(shiftError) < 1 && fabs(squeezeError) < 0.01){
			// The fit is good enough to use the values
			for(int it = 0; it < eval->m_window.nRef; ++it){
				eval->m_window.ref[it].m_shiftOption		= SHIFT_FIX;
				eval->m_window.ref[it].m_squeezeOption		= SHIFT_FIX;
				eval->m_window.ref[it].m_shiftValue			= shift;
				eval->m_window.ref[it].m_squeezeValue		= squeeze;
			}
			message.Format("Shift: %.2lf ± %.2lf; Squeeze: %.2lf ± %.2lf", shift, shiftError, squeeze, squeezeError);
			ShowMessage(message);
		}else{
			ShowMessage("Fit not good enough. Will proceed with default parameters.");
		}
	}
}