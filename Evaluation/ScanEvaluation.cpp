#include "StdAfx.h"
#include "ScanEvaluation.h"
#include "EvaluationResultView.h"
#include "../SpectralEvaluation/Evaluation/EvaluationBase.h"
#include "../SpectralEvaluation/File/ScanFileHandler.h"
#include "../SpectralEvaluation/File/SpectrumIO.h"
#include "../SpectralEvaluation/File/STDFile.h"
#include "../SpectralEvaluation/File/TXTFile.h"
#include "../SpectralEvaluation/Utils.h"

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
	pView = nullptr;
}

int CScanEvaluation::NumberOfSpectraInLastResult()
{
	std::lock_guard<std::mutex> lock{ m_resultMutex };

	if (nullptr != m_result)
	{
		return m_result->GetEvaluatedNum();
	}
	else
	{
		return 0;
	}
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
long CScanEvaluation::EvaluateScan(const CString &scanfile, const CFitWindow& window, bool *fRun, const Configuration::CDarkSettings *darkSettings){
	
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
	m_fitLow  = window.fitLow;
	m_fitHigh = window.fitHigh;

	// Check so that the file exists
	if(!IsExistingFile(scanfile)) {
		return 0;
	}

	// The CScanFileHandler is a structure for reading the spectral information 
	//  from the scan-file
	FileHandler::CScanFileHandler scan;  

	// Check the scan file, make sure it's correct and that the file
	//	actually contains spectra
    const std::string scanFileName((LPCSTR)scanfile);
	if(SUCCESS != scan.CheckScanFile(scanFileName)) {
		return 0;
	}

	// make a backup of the fit window (this function may make some changes to the
	//  fit window, and we should be able to restore the old values on return).
	CFitWindow copyOfWindow = window;

	// If the user wants to find optimum shift, then the scan shall be evaluated
	//  once with all shifts set to 0 and all squeeze set to 1.
	if(copyOfWindow.findOptimalShift)
    {
		for(int k = 0; k < copyOfWindow.nRef; ++k)
        {
            copyOfWindow.ref[k].m_shiftOption   = SHIFT_FIX;
            copyOfWindow.ref[k].m_squeezeOption = SHIFT_FIX;
            copyOfWindow.ref[k].m_shiftValue    = 0.0;
            copyOfWindow.ref[k].m_squeezeValue  = 1.0;
		}
	}

	// Get the sky and dark spectra and divide them by the number of 
	//     co-added spectra in it
	if(SUCCESS != GetSky(&scan, sky))
    {
		return 0;
	}
	CSpectrum original_sky = sky; // original_sky is the sky-spectrum without dark-spectrum corrections...

	if(m_skyOption != SKY_USER) {
		if(SUCCESS != GetDark(&scan, sky, dark, darkSettings)) {
			return 0;
		}
		sky.Sub(dark);
	}

	if(sky.NumSpectra() > 0 && !m_averagedSpectra) {
		sky.Div(sky.NumSpectra());
		original_sky.Div(original_sky.NumSpectra());
	}

    // Get some important information about the spectra, like
    //	interlace steps, spectrum length and start-channel
    copyOfWindow.interlaceStep = scan.GetInterlaceSteps();
    copyOfWindow.specLength = scan.GetSpectrumLength() * copyOfWindow.interlaceStep;
    copyOfWindow.startChannel = scan.GetStartChannel();

    // Create our evaluator
    std::unique_ptr<CEvaluationBase> eval = std::make_unique<CEvaluationBase>(copyOfWindow);

    // tell the evaluator which sky-spectrum to use
    eval->SetSkySpectrum(sky);

	// Adjust the fit-low and fit-high parameters according to the spectra
	m_fitLow  = copyOfWindow.fitLow  - copyOfWindow.startChannel;
	m_fitHigh = copyOfWindow.fitHigh - copyOfWindow.startChannel;

	// If we have a solar-spectrum that we can use to determine the shift
	//	& squeeze then fit that first so that we know the wavelength calibration
	if(copyOfWindow.fraunhoferRef.m_path.size() > 4)
    {
        // TODO: Implement this properly...
        CFitWindow* newWindow = FindOptimumShiftAndSqueeze_Fraunhofer(eval.get(), &scan);

        if (nullptr != newWindow)
        {
            copyOfWindow = *newWindow; // copy the good window here.
            eval.reset(new CEvaluationBase{copyOfWindow});
            delete newWindow;
        }
	}

	// the data structure to keep track of the evaluation results
	std::shared_ptr<CScanResult> newResult = std::make_shared<CScanResult>();

	// Check weather we are to find an optimal shift and squeeze
	int nIt = (copyOfWindow.findOptimalShift == FALSE) ? 1 : 2;

	// Evaluate the scan (one or two times, depending on the settings)
	for(int iteration = 0; iteration < nIt; ++iteration)
    {
		index = -1; // we're at spectrum number 0 in the .pak-file
		m_indexOfMostAbsorbingSpectrum = -1;	// as far as we know, there's no absorption in any spectrum...

		newResult->SetSkySpecInfo(original_sky.m_info);
		newResult->SetDarkSpecInfo(dark.m_info);

		// Make sure that we'll start with the first spectrum in the scan
		scan.ResetCounter();

		// Evaluate all the spectra in the scan.
		while(1)
        {
			success = true; // assume that we will succeed in evaluating this spectrum

			// If the user wants to exit this thread then do so.
			if(fRun != nullptr && *fRun == false) {
				ShowMessage("Scan Evaluation cancelled by user");
				return 0;
			}

			// remember which spectrum we're at
			int	spectrumIndex = current.ScanIndex();

			// a. Read the next spectrum from the file
			int ret = scan.GetNextSpectrum(current);

			if(ret == 0) {
				// if something went wrong when reading the spectrum
				if(scan.m_lastError == SpectrumIO::CSpectrumIO::ERROR_SPECTRUM_NOT_FOUND || scan.m_lastError == SpectrumIO::CSpectrumIO::ERROR_EOF){
					// at the end of the file, quit the 'while' loop
					break;
				}else{
					CString errMsg;
					errMsg.Format("Faulty spectrum found in %s", (LPCSTR)scanfile);
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
			if(current.ScanIndex() == sky.ScanIndex() || current.ScanIndex() == dark.ScanIndex()) {
				continue;
			}

			// If the spectrum is read out in an interlaced way then interpolate it back to it's original state
			if(current.m_info.m_interlaceStep > 1) {
				current.InterpolateSpectrum();
			}

			// b. Get the dark spectrum for this measured spectrum
			if(SUCCESS != GetDark(&scan, current, dark, darkSettings)) {
				return 0;
			}

			// b. Calculate the intensities, before we divide by the number of spectra
			//		and before we subtract the dark
			current.m_info.m_peakIntensity = (float)current.MaxValue(0, current.m_length - 2);
			current.m_info.m_fitIntensity  = (float)current.MaxValue(m_fitLow, m_fitHigh);

			// c. Divide the measured spectrum with the number of co-added spectra
			//     The sky and dark spectra should already be divided before this loop.
			if(current.NumSpectra() > 0 && !m_averagedSpectra) {
				current.Div(current.NumSpectra());
			}

			// d. Check if this spectrum is worth evaluating
			if(Ignore(current, copyOfWindow))
            {
				message.Format("Ignoring spectrum %d in scan %s.", current.ScanIndex(), scan.GetFileName().c_str());
				ShowMessage(message);
				continue;
			}

			// d2. Now subtract the dark (if we did this earlier, then the 'Ignore' - function would
			//		not function properly)
			if(dark.NumSpectra() > 0 && !m_averagedSpectra) {
				dark.Div(dark.NumSpectra());
			}

			current.Sub(dark);

			// e. Evaluate the spectrum
			if(eval->Evaluate(current))
            {
				CString str;
				str.Format("Failed to evaluate spectrum from spectrometer %s. Failure at spectrum %d in scan containing %d spectra. Message: '%s'",
					current.m_info.m_device.c_str(), current.ScanIndex(), current.SpectraPerScan()), eval->m_lastError.c_str();
				ShowMessage(str);
				success = false;
			}

			// e. Save the evaluation result
			newResult->AppendResult(eval->GetEvaluationResult(), current.m_info);

			// f. Check if this was an ok data point (CScanResult)
			newResult->CheckGoodnessOfFit(current.m_info);

			// g. If it is ok, then check if the value is higher than any of the previous ones
			if(newResult->IsOk(newResult->GetEvaluatedNum()-1) && fabs(newResult->GetColumn(newResult->GetEvaluatedNum()-1, 0)) > highestColumn) {
				highestColumn = fabs(newResult->GetColumn(newResult->GetEvaluatedNum()-1, 0));
				m_indexOfMostAbsorbingSpectrum	= index;
			}

			// h. Update the screen (if any)
			if (success)
            {
				UpdateResult(newResult);

                if(pView != nullptr)
                {
				    ShowResult(current, eval.get(), index, scan.GetSpectrumNumInFile());
			    }
            }

			// i. If the user wants us to sleep between each evaluation. Do so...
			if(m_pause != nullptr && *m_pause == 1 && m_sleeping != nullptr){
				CWinThread *thread = AfxGetThread();
				*m_sleeping = true;
				if(pView != 0) {
					pView->PostMessage(WM_GOTO_SLEEP);
				}
				thread->SuspendThread();
				*m_sleeping = false;
			}
			else {
				Sleep(20);
			}
		} // end while(1)

		// end of scan...
		if((iteration == 0) && (copyOfWindow.findOptimalShift == TRUE))
        {
            if (m_indexOfMostAbsorbingSpectrum < 0)
            {
                ShowMessage("Could not determine optimal shift & squeeze. No good spectra in scan.");
                break;
            }
            else
            {
                CEvaluationResult result = FindOptimumShiftAndSqueeze(eval.get(), &scan, newResult.get());

                // Get a new fit-window to use for the second iteration...
                CFitWindow newWindow = copyOfWindow; // create a new copy

                                                     // 5. Set the shift for all references to this value
                for (int k = 0; k < newWindow.nRef; ++k) {
                    if (newWindow.ref[k].m_specieName.compare("FraunhoferRef") == 0)
                        continue;

                    newWindow.ref[k].m_shiftOption   = SHIFT_FIX;
                    newWindow.ref[k].m_squeezeOption = SHIFT_FIX;
                    newWindow.ref[k].m_shiftValue    = result.m_referenceResult[0].m_shift;
                    newWindow.ref[k].m_squeezeValue  = result.m_referenceResult[0].m_squeeze;
                }

                eval.reset(new CEvaluationBase{newWindow});
		    }
        }
	}

	return NumberOfSpectraInLastResult();
}

void CScanEvaluation::UpdateResult(std::shared_ptr<CScanResult> newResult)
{
	std::lock_guard<std::mutex> lock{ m_resultMutex };
	m_result = newResult;
}

void CScanEvaluation::ShowResult(const CSpectrum &spec, const CEvaluationBase *eval, long curSpecIndex, long specNum){
	if(pView == nullptr) {
		return;
	}

	int fitLow	= eval->FitWindow().fitLow  - spec.m_info.m_startChannel;
	int fitHigh = eval->FitWindow().fitHigh - spec.m_info.m_startChannel;

	/** copy the spectra. These spectra will be filled in after each evaluation
		and the address of the first spectrum in the array will be sent
		with the 'WM_EVAL_SUCCESS' message to the pView window.
		The first spectrum in the array defines the last read spectrum.
		The second spectrum is the residual of the fit,
		The third spectrum is the fitted polynomial, and the following
		MAX_N_REFERENCES + 1 spectra are the scaled reference spectra used in the fit. */
    // TODO: Create a struct to hold this information instead. This is messy and prone to errors...
    CEvaluationResultView* resultView = new CEvaluationResultView();
    resultView->scaledReference.resize(eval->NumberOfReferencesFitted());

    resultView->measuredSpectrum = spec;

	// copy the residual and the polynomial
	for(int i = fitLow; i < fitHigh; ++i)
    {
        resultView->residual.m_data[i] = eval->m_residual.GetAt(i - fitLow);
        resultView->polynomial.m_data[i] = eval->m_fitResult[0].GetAt(i);
	}

	// copy the scaled referencefiles
	for(size_t tmpRefIndex = 0; tmpRefIndex < eval->NumberOfReferencesFitted(); ++tmpRefIndex)
    {
		for(int i = fitLow; i < fitHigh; ++i)
        {
            resultView->scaledReference[tmpRefIndex].m_data[i] = eval->m_fitResult[tmpRefIndex+1].GetAt(i);
		}
	}

	{
		std::lock_guard<std::mutex> lock{ m_resultMutex };
		CScanResult* copiedResult = new CScanResult(*m_result.get());

		// post the message to the view to update. This will also transfer the ownership of the two pointers to the view
		pView->PostMessage(WM_EVAL_SUCCESS, (WPARAM)resultView, (LPARAM)copiedResult);
	}

	m_prog_SpecCur = curSpecIndex;
	m_prog_SpecNum = specNum;
	pView->PostMessage(WM_PROGRESS2, (WPARAM)m_prog_SpecCur, (LPARAM)m_prog_SpecNum);
}

RETURN_CODE CScanEvaluation::GetDark(FileHandler::CScanFileHandler *scan, const CSpectrum &spec, CSpectrum &dark, const Configuration::CDarkSettings *darkSettings)
{
    m_lastErrorMessage = "";
    const bool successs = ScanEvaluationBase::GetDark(*scan, spec, dark, darkSettings);

    if (m_lastErrorMessage.size() > 0)
    {
        CString message;
        message.Format("%s", m_lastErrorMessage.c_str());
        ShowMessage(message);
    }

    if (successs)
        return SUCCESS;
    else
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
            const std::string userSkyFileName((LPCSTR)m_userSkySpectrum);
			if(reader.ReadSpectrum(userSkyFileName, 0, sky))
                return SUCCESS;
            else
                return FAIL;
		}else if(Equals(m_userSkySpectrum.Right(4), ".std", 4)){
			// If the spectrum is in .std format
            const std::string fileName((LPCSTR)m_userSkySpectrum);
			if(SpectrumIO::CSTDFile::ReadSpectrum(sky, fileName))
                return SUCCESS;
            else
                return FAIL;
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
		m_userSkySpectrum.Format("%s", (LPCSTR)*skySpecPath);

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


CEvaluationResult CScanEvaluation::FindOptimumShiftAndSqueeze(const CEvaluationBase *originalEvaluation, FileHandler::CScanFileHandler *scan, CScanResult *result)
{
	int k;
	CSpectrum spec, sky, dark;
	int specieNum = 0;
	CString message;

	// 1. Find the spectrum with the highest column value

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
    CFitWindow newFitWindow = originalEvaluation->FitWindow(); // Create a local copy which we can modify
    newFitWindow.ref[0].m_shiftOption   = SHIFT_FREE;
    newFitWindow.ref[0].m_squeezeOption = SHIFT_FIX;
    newFitWindow.ref[0].m_squeezeValue  = 1.0;
	for(k = 1; k < newFitWindow.nRef; ++k){
		if(EqualsIgnoringCase(newFitWindow.ref[k].m_specieName, "FraunhoferRef"))
			continue;

		newFitWindow.ref[k].m_shiftOption   = SHIFT_LINK;
		newFitWindow.ref[k].m_squeezeOption = SHIFT_LINK;
		newFitWindow.ref[k].m_shiftValue    = 0.0;
		newFitWindow.ref[k].m_squeezeValue  = 0.0;
	}
	// Get the sky-spectrum
	GetSky(scan, sky);
	if(sky.NumSpectra() > 0  && !m_averagedSpectra)
    {
		sky.Div(sky.NumSpectra());
    }

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
    CEvaluationBase newEval{ newFitWindow };
    newEval.SetSkySpectrum(sky);
    newEval.Evaluate(spec, 5000);

	// 4. See what the optimum value for the shift turned out to be.
	CEvaluationResult newResult = newEval.GetEvaluationResult();
	double optimumShift		    = newResult.m_referenceResult[0].m_shift;
	double optimumSqueeze       = newResult.m_referenceResult[0].m_squeeze;

	// 5. We're done!
	message.Format("Optimum shift set to : %.2lf. Optimum squeeze set to: %.2lf ", optimumShift, optimumSqueeze);
	ShowMessage(message);
	
    return newResult;
}

CFitWindow* CScanEvaluation::FindOptimumShiftAndSqueeze_Fraunhofer(const CEvaluationBase *originalEvaluation, FileHandler::CScanFileHandler *scan)
{
    CFitWindow newFitWindow = originalEvaluation->FitWindow(); // Create a local copy which we can modify
	double shift, shiftError, squeeze, squeezeError;
	long fitLow = newFitWindow.fitLow;
	long fitHigh= newFitWindow.fitHigh;
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
	maxInt				= CSpectrometerModel::GetMaxIntensity(spectrum.m_info.m_specModel);
	if(spectrum.NumSpectra() > 0) {
		fitSaturation	= fitIntensity / (spectrum.NumSpectra() * maxInt);
	}
	else {
		int numSpec		= (int)floor(spectrum.MaxValue() / maxInt); // a guess for the number of co-adds
		fitSaturation	= fitIntensity / (maxInt * spectrum.NumSpectra());
	}

	if(fitSaturation < 0.9 && fitSaturation > 0.1) {
		indexOfMostSuitableSpectrum = INDEX_OF_SKYSPECTRUM;
		bestSaturation				= fitSaturation;
	}

	scan->ResetCounter(); // start from the beginning
	
	while(scan->GetNextSpectrum(spectrum)) {
		fitIntensity		= spectrum.MaxValue(fitLow, fitHigh);
		maxInt				= CSpectrometerModel::GetMaxIntensity(spectrum.m_info.m_specModel);

		// Get the saturation-ratio for this spectrum
		if(spectrum.NumSpectra() > 0){
			fitSaturation	= fitIntensity / (spectrum.NumSpectra() * maxInt);
		}else{
			int numSpec		= (int)floor(spectrum.MaxValue() / maxInt); // a guess for the number of co-adds
			fitSaturation	= fitIntensity / (maxInt * spectrum.NumSpectra());
		}

		// Check if this spectrum is good...
		if(fitSaturation < 0.9 && fitSaturation > 0.1) {
			if(fitSaturation > bestSaturation) {
				indexOfMostSuitableSpectrum = curIndex;
				bestSaturation							= fitSaturation;
			}
		}

		// Go to the next spectrum
		++curIndex;
	}

	// 2. Get the spectrum we should evaluate...
	if(indexOfMostSuitableSpectrum == NO_SPECTRUM_INDEX) {
		return nullptr; // we could not find any good spectrum to use...
	} else if(indexOfMostSuitableSpectrum == INDEX_OF_SKYSPECTRUM) {
		scan->GetSky(spectrum);
		message.Format("Determining shift and squeeze from sky-spectrum");
	} else {
		scan->GetSpectrum(spectrum, indexOfMostSuitableSpectrum);
		message.Format("Determining shift and squeeze from spectrum %d", indexOfMostSuitableSpectrum);
	}

	if(spectrum.NumSpectra() > 0 && !m_averagedSpectra) {
		spectrum.Div(spectrum.NumSpectra());
	}

	if(SUCCESS != GetDark(scan, spectrum, dark)) {
		return nullptr; // fail
	}

	if(dark.NumSpectra() > 0 && !m_averagedSpectra) {
		dark.Div(dark.NumSpectra());
	}

	spectrum.Sub(dark);

	ShowMessage(message);

	// 3. Do the evaluation.
    CEvaluationBase shiftEvaluator{newFitWindow};

	if(shiftEvaluator.EvaluateShift(spectrum, shift, shiftError, squeeze, squeezeError)) {
		// We failed to make the fit, what shall we do now??
		ShowMessage("Failed to determine shift and squeeze. Will proceed with default parameters.");
        return nullptr;
	}else{
		if(fabs(shiftError) < 1 && fabs(squeezeError) < 0.01)
        {
            CFitWindow* bestFitWindow = new CFitWindow{ originalEvaluation->FitWindow() };
			// The fit is good enough to use the values
			for(int it = 0; it < originalEvaluation->FitWindow().nRef; ++it){
                bestFitWindow->ref[it].m_shiftOption		= SHIFT_FIX;
                bestFitWindow->ref[it].m_squeezeOption		= SHIFT_FIX;
                bestFitWindow->ref[it].m_shiftValue			= shift;
                bestFitWindow->ref[it].m_squeezeValue		= squeeze;
			}
			message.Format("Shift: %.2lf ± %.2lf; Squeeze: %.2lf ± %.2lf", shift, shiftError, squeeze, squeezeError);
			ShowMessage(message);

            return bestFitWindow;
		}else{
			ShowMessage("Fit not good enough. Will proceed with default parameters.");

            return nullptr;
		}
	}
}