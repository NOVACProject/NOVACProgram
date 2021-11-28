#include "StdAfx.h"
#include "../NovacMasterProgram.h"
#include "ReEvaluator.h"

#include "../Common/Version.h"
#include "../Evaluation/ScanEvaluation.h"
#include "../Dialogs/QueryStringDialog.h"
#include <SpectralEvaluation/StringUtils.h>
#include <SpectralEvaluation/Spectra/SpectrometerModel.h>

using namespace ReEvaluation;
using namespace Evaluation;
using namespace novac;

CReEvaluator::CReEvaluator(void)
{
    fRun = false;
    m_pause = 0;
    m_sleeping = false;

    // initialize the scan file list
    m_scanFileNum = 0;
    m_scanFile.SetSize(64); // <-- The initial guess of the number of pak-files

    // initialize the list of references 
    for (int i = 0; i < MAX_N_WINDOWS; ++i) {
        for (int j = 1; j < MAX_N_REFERENCES; ++j) {
            m_window[i].ref[j].SetShift(SHIFT_FIX, 0.0);
            m_window[i].ref[j].SetSqueeze(SHIFT_FIX, 1.0);
        }
    }
    m_windowNum = 1;
    m_curWindow = 0;

    pView = nullptr;
    m_progress = 0;

    m_statusMsg.Format("");

    // ignoring
    m_ignore_Lower.m_channel = 1144;
    m_ignore_Lower.m_intensity = 1000.0;
    m_ignore_Lower.m_type = IGNORE_DARK;

    m_ignore_Upper.m_channel = 1144;
    m_ignore_Upper.m_intensity = 4000.0;
    m_ignore_Upper.m_type = IGNORE_NOTHING;

    // The sky spectrum 
    m_skySettings.skyOption = Configuration::SKY_OPTION::MEASURED_IN_SCAN;
    m_skySettings.indexInScan = 0;

    // The default is that the spectra are summed, not averaged
    m_averagedSpectra = false;
}

/** Halts the current operation */
bool CReEvaluator::Stop() {
    fRun = false;
    return true;
}

bool CReEvaluator::DoEvaluation()
{
    /* Check the settings before we start */
    if (!MakeInitialSanityCheck())
    {
        return false;
    }

    /** Prepare everything for evaluating */
    if (!PrepareEvaluation())
    {
        return false;
    }

    /* evaluate the spectra */
    m_progress = 0;

    // The CScanEvaluation-object handles the evaluation of one single scan.
    CScanEvaluation ev;
    ev.pView = this->pView;
    ev.m_pause = &m_pause;
    ev.m_sleeping = &m_sleeping;

    // Set the options for the CScanEvaluation object
    ev.SetOption_Sky(m_skySettings);
    ev.SetOption_Ignore(m_ignore_Lower, m_ignore_Upper);
    ev.SetOption_AveragedSpectra(m_averagedSpectra);

    // loop through all the scan files
    for (int curScanFile = 0; curScanFile < m_scanFileNum; ++curScanFile)
    {
        m_progress = curScanFile / (double)m_scanFileNum;
        m_progress *= 1000.0;
        if (pView != nullptr)
        {
            pView->PostMessage(WM_PROGRESS, (WPARAM)m_progress);
        }

        // The CScanFileHandler is a structure for reading the spectral information from the scan-file
        CScanFileHandler scan;

        // Check the scan file
        const std::string scanFileName((LPCSTR)m_scanFile[curScanFile]);
        if (SUCCESS != scan.CheckScanFile(scanFileName))
        {
            CString errStr;
            errStr.Format("Could not read scan-file %s", (LPCTSTR)m_scanFile[curScanFile]);
            MessageBox(NULL, errStr, "Error", MB_OK);
            continue;
        }

        // update the status window
        m_statusMsg.Format("Evaluating scan number %d", curScanFile);
        ShowMessage(m_statusMsg);

        // For each scanfile: loop through the fit windows
        for (m_curWindow = 0; m_curWindow < m_windowNum; ++m_curWindow)
        {
            CFitWindow& thisWindow = m_window[m_curWindow];

            // Check the interlace steps
            CSpectrum skySpec;
            scan.GetSky(skySpec);

            if (skySpec.Channel() > MAX_CHANNEL_NUM)
            {
                // We should use an interlaced window instead
                if (-1 == Common::GetInterlaceSteps(skySpec.Channel(), skySpec.m_info.m_interlaceStep))
                {
                    return 0; // WRONG!!
                }

                thisWindow.interlaceStep = skySpec.m_info.m_interlaceStep;
                thisWindow.specLength = skySpec.m_length * skySpec.m_info.m_interlaceStep;
            }

            if (skySpec.m_info.m_startChannel > 0 || skySpec.m_length != thisWindow.specLength / thisWindow.interlaceStep)
            {
                // If the spectra are too short or the start channel is not zero
                //	then they are read out as partial spectra. Lets adapt the evaluator to that
                thisWindow.specLength = skySpec.m_length;
                thisWindow.startChannel = skySpec.m_info.m_startChannel;
            }

            if (skySpec.m_info.m_interlaceStep > 1)
            {
                skySpec.InterpolateSpectrum();
            }

            // check the quality of the sky-spectrum
            auto model = novac::CSpectrometerDatabase::GetInstance().GuessModelFromSerial(skySpec.m_info.m_device);
            const double maximumSaturationRatio = novac::GetMaximumSaturationRatioOfSpectrum(skySpec, model, thisWindow.fitLow, thisWindow.fitHigh);

            if (maximumSaturationRatio >= 0.95)
            {
                if (skySpec.NumSpectra() > 0)
                {
                    CString message;
                    message.Format("It seems like the sky-spectrum is saturated in the fit-region. Continue?");
                    if (IDNO == MessageBox(NULL, message, "Saturated sky spectrum?", MB_YESNO))
                    {
                        break; // continue with the next scan-file
                    }
                }
            }

            // Evaluate the scan-file
            int success = ev.EvaluateScan(m_scanFile[curScanFile], m_window[m_curWindow], &fRun, &m_darkSettings);

            // Check if the user wants to stop
            if (!fRun)
            {
                return true;
            }

            // get the result of the evaluation and write them to file
            if (success)
            {
                std::unique_ptr<CScanResult> res = ev.GetResult();
                AppendResultToEvaluationLog(*res.get(), scan, m_curWindow);
            }

        }//end for m_curWindow...
        m_curWindow = 0;

    } // end for(curScanFile...

    if (pView != nullptr)
    {
        pView->PostMessage(WM_DONE);
    }

    fRun = false;

    return true;
}

/* Check the settings before we start */
bool CReEvaluator::MakeInitialSanityCheck() {
    // 1. Check so that there are any fit windows defined
    if (this->m_windowNum <= 0)
        return false;

    // 2. Check so that there are not too many fit windows defined
    if (m_windowNum > MAX_N_WINDOWS)
        return false;

    // 3. Check so that there are any spectrum files to evaluate
    if (this->m_scanFileNum <= 0)
        return false;

    return true;
}

bool CReEvaluator::CreateOutputDirectory()
{
    CString cDateTime, path, fileName;
    struct tm* tim;
    time_t t;

    time(&t);
    tim = localtime(&t);
    cDateTime.Format("%04d%02d%02d_%02d%02d", tim->tm_year + 1900, tim->tm_mon + 1, tim->tm_mday, tim->tm_hour, tim->tm_min);

    // Get the directory name
    path.Format("%s", (LPCTSTR)m_scanFile[0]);
    fileName.Format("%s", (LPCTSTR)m_scanFile[0]);
    Common::GetDirectory(path); // gets the directory of the scan-file
    Common::GetFileName(fileName);
    m_outputDir.Format("%s\\ReEvaluation_%s_%s", (LPCTSTR)path, (LPCTSTR)fileName, (LPCTSTR)cDateTime);

    // Create the directory
    if (0 == CreateDirectory(m_outputDir, NULL)) {
        DWORD errorCode = GetLastError();
        if (errorCode != ERROR_ALREADY_EXISTS) { /* We shouldn't quit just because the directory that we want to create already exists. */
            CString tmpStr;
            tmpStr.Format("Could not create output directory. Error code returned %ld. Do you want to create an output directory elsewhere?", errorCode);
            int ret = MessageBox(NULL, tmpStr, "Could not create output directory", MB_YESNO);
            if (ret == IDNO)
                return false;
            else {
                // Create the output-directory somewhere else
                Dialogs::CQueryStringDialog pathDialog;
                pathDialog.m_windowText.Format("The path to the output directory?");
                pathDialog.m_inputString = &path;
                INT_PTR ret = pathDialog.DoModal();
                if (IDCANCEL == ret)
                    return false;
                m_outputDir.Format("%s\\ReEvaluation_%s_%s", (LPCTSTR)path, (LPCTSTR)fileName, (LPCTSTR)cDateTime);
                if (0 == CreateDirectory(m_outputDir, NULL)) {
                    tmpStr.Format("Could not create output directory. ReEvaluation aborted.");
                    MessageBox(NULL, tmpStr, "ERROR", MB_OK);
                    return false;
                }
            }
        }
    }
    return true;
}

bool CReEvaluator::WriteEvaluationLogHeader(int fitWindowIndex)
{
    CString time, date, name;
    Common::GetDateText(date);
    Common::GetTimeText(time);

    // simplify the syntax a little bit
    const CFitWindow& window = m_window[fitWindowIndex];

    // Get the name of the evaluation log file
    m_evalLog[fitWindowIndex] = m_outputDir + "\\ReEvaluationLog_" + CString(window.name.c_str()) + ".txt";

    // Try to open the log file
    FILE* f = fopen(m_evalLog[fitWindowIndex], "w");
    if (f == nullptr)
    {
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
    if (m_ignore_Lower.m_type == IGNORE_LIMIT)
        fprintf(f, "#Ignore Spectra with intensity below=%.1lf @ channel %d\n", m_ignore_Lower.m_intensity, m_ignore_Lower.m_channel);
    else
        fprintf(f, "#Ignore Dark Spectra\n");

    // Ignoring saturated spectra?
    if (m_ignore_Upper.m_type == IGNORE_LIMIT)
        fprintf(f, "#Ignore Spectra with intensity above=%.1lf @ channel %d\n", m_ignore_Upper.m_intensity, m_ignore_Upper.m_channel);
    else
        fprintf(f, "#Ignore Saturated Spectra\n");

    // The sky-spectrum used
    switch (m_skySettings.skyOption) {
    case Configuration::SKY_OPTION::MEASURED_IN_SCAN:                   fprintf(f, "#Sky: First spectrum in scanFile\n");	break;
    case Configuration::SKY_OPTION::USER_SUPPLIED:                      fprintf(f, "#Sky: %s\n", m_skySettings.skySpectrumFile.c_str()); break;
    case Configuration::SKY_OPTION::SPECTRUM_INDEX_IN_SCAN:             fprintf(f, "#Sky: Spectrum number %d", m_skySettings.indexInScan); break;
    case Configuration::SKY_OPTION::AVERAGE_OF_GOOD_SPECTRA_IN_SCAN:    fprintf(f, "#Sky: Average of all good spectra in scanFile\n");
    }

    // Write the region used:
    if (window.UV)
        fprintf(f, "#Region: UV\n");
    else
        fprintf(f, "#Region: Visible\n");

    // If the spectra are averaged or not
    if (m_averagedSpectra)
        fprintf(f, "#Spectra are averaged, not summed\n");

    // The reference-files
    fprintf(f, "#nSpecies=%d\n", window.nRef);
    fprintf(f, "#Specie\tShift\tSqueeze\tReferenceFile\n");
    for (int i = 0; i < window.nRef; ++i) {
        fprintf(f, "#%s\t", window.ref[i].m_specieName.c_str());
        switch (window.ref[i].m_shiftOption) {
        case SHIFT_FIX:
            fprintf(f, "%0.3lf\t", window.ref[i].m_shiftValue); break;
        case SHIFT_LINK:
            fprintf(f, "linked to %s\t", window.ref[(int)window.ref[i].m_shiftValue].m_specieName.c_str()); break;
        case SHIFT_LIMIT:
            fprintf(f, "limited to +-%0.3lf\t", window.ref[i].m_shiftValue);
        default:
            fprintf(f, "free\t"); break;
        }

        switch (window.ref[i].m_squeezeOption) {
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

bool CReEvaluator::AppendResultToEvaluationLog(const Evaluation::CScanResult& result, const CScanFileHandler& scan, int fitWindowIndex)
{
    CSpectrum skySpec;

    FILE* f = fopen(m_evalLog[fitWindowIndex], "a+");
    if (nullptr == f)
    {
        return false;
    }

    // simplify the syntax a little bit
    const CFitWindow& window = m_window[fitWindowIndex];

    // Write the scan-information to file
    fprintf(f, "<scaninformation>\n");
    fprintf(f, "\tdate=%02d.%02d.%04d\n", scan.m_startTime.day, scan.m_startTime.month, scan.m_startTime.year);
    fprintf(f, "\tstarttime=%02d:%02d:%02d\n", scan.m_startTime.hour, scan.m_startTime.minute, scan.m_startTime.second);
    fprintf(f, "\tcompass=%.1lf\n", scan.GetCompass());
    fprintf(f, "\ttilt=%.1lf\n", skySpec.m_info.m_pitch);

    const CGPSData& gps = scan.GetGPS();
    fprintf(f, "\tlat=%.6lf\n", gps.m_latitude);
    fprintf(f, "\tlong=%.6lf\n", gps.m_longitude);
    fprintf(f, "\talt=%.3lf\n", gps.m_altitude);

    fprintf(f, "\tserial=%s\n", scan.m_device.c_str());
    fprintf(f, "\tchannel=%d\n", scan.m_channel);

    scan.GetSky(skySpec);
    fprintf(f, "\tconeangle=%.1lf\n", skySpec.m_info.m_coneAngle);
    fprintf(f, "\tinterlacesteps=%d\n", scan.GetInterlaceSteps());
    fprintf(f, "\tstartchannel=%d\n", scan.GetStartChannel());
    fprintf(f, "\tspectrumlength=%d\n", scan.GetSpectrumLength());

    fprintf(f, "\tbattery=%.2f\n", skySpec.m_info.m_batteryVoltage);
    fprintf(f, "\ttemperature=%.2f\n", skySpec.m_info.m_temperature);

    // The mode
    if (result.IsDirectSunMeasurement())
        fprintf(f, "\tmode=direct_sun\n");
    if (result.IsLunarMeasurement())
        fprintf(f, "\tmode=lunar\n");
    else if (result.IsWindMeasurement())
        fprintf(f, "\tmode=wind\n");
    else if (result.IsStratosphereMeasurement())
        fprintf(f, "\tmode=stratospheric\n");
    else if (result.IsCompositionMeasurement())
        fprintf(f, "\tmode=composition\n");
    else
        fprintf(f, "\tmode=plume\n");

    // Finally, the version of the file and the version of the program
    fprintf(f, "\tsoftwareversion=%d.%d\n", CVersion::majorNumber, CVersion::minorNumber);
    fprintf(f, "\tcompiledate=%s\n", __DATE__);

    fprintf(f, "</scaninformation>\n");

    // Write the header
    fprintf(f, "#scanangle\tstarttime\tstoptime\tname\tdelta\tchisquare\texposuretime\tnumspec\tintensity\tfitintensity\tisgoodpoint\toffset\tflag\t");
    for (int i = 0; i < window.nRef; ++i)
    {
        CString name;
        name.Format("%s", window.ref[i].m_specieName.c_str());

        fprintf(f, "column(%s)\tcolumnerror(%s)\t", (LPCTSTR)name, (LPCTSTR)name);
        fprintf(f, "shift(%s)\tshifterror(%s)\t", (LPCTSTR)name, (LPCTSTR)name);
        fprintf(f, "squeeze(%s)\tsqueezeerror(%s)\t", (LPCTSTR)name, (LPCTSTR)name);
    }

    if (window.fitType == novac::FIT_TYPE::FIT_HP_SUB || window.fitType == novac::FIT_TYPE::FIT_POLY)
    {
        CString name = "fraunhoferref";

        fprintf(f, "column(%s)\tcolumnerror(%s)\t", (LPCTSTR)name, (LPCTSTR)name);
        fprintf(f, "shift(%s)\tshifterror(%s)\t", (LPCTSTR)name, (LPCTSTR)name);
        fprintf(f, "squeeze(%s)\tsqueezeerror(%s)", (LPCTSTR)name, (LPCTSTR)name);
    }

    fprintf(f, "\n<spectraldata>\n");

    for (unsigned long i = 0; i < result.GetEvaluatedNum(); ++i)
    {
        const CSpectrumInfo& info = result.GetSpectrumInfo(i);

        // The scan angle
        fprintf(f, "%.0f\t", info.m_scanAngle);

        // The start time
        fprintf(f, "%02d:%02d:%02d\t", info.m_startTime.hour, info.m_startTime.minute, info.m_startTime.second);

        // The stop time
        fprintf(f, "%02d:%02d:%02d\t", info.m_stopTime.hour, info.m_stopTime.minute, info.m_stopTime.second);

        // The name of the spectrum
        const std::string spectrumName = SimplifyString(info.m_name);
        fprintf(f, "%s\t", spectrumName.c_str());

        // The delta of the fit
        fprintf(f, "%.2e\t", result.GetDelta(i));

        // The chi-square of the fit
        fprintf(f, "%.2e\t", result.GetChiSquare(i));

        // The exposure time of the spectrum
        fprintf(f, "%d\t", info.m_exposureTime);

        // The number of spectra averaged
        fprintf(f, "%d\t", info.m_numSpec);

        // The saved intensity
        fprintf(f, "%.0f\t", info.m_peakIntensity);

        // The intensity in the fit region
        fprintf(f, "%.0f\t", info.m_fitIntensity);

        // The quality of the fit
        fprintf(f, "%d\t", result.IsOk(i));

        // The electronic offset
        fprintf(f, "%.1f\t", info.m_offset);

        // The 'flag' - solenoid or diffusor plate
        fprintf(f, "%d\t", info.m_flag);

        //		fprintf(f, "%.0lf\t", spec.MaxValue(m_window[m_curWindow].fitLow, m_window[m_curWindow].fitHigh));

        // the number of references
        int nRef = window.nRef;
        if (window.fitType == novac::FIT_TYPE::FIT_HP_SUB || window.fitType == novac::FIT_TYPE::FIT_POLY)
        {
            ++nRef;
        }

        // The Result
        for (int j = 0; j < nRef; ++j) {

            fprintf(f, "%.2e\t", result.GetColumn(i, j));
            fprintf(f, "%.2e\t", result.GetColumnError(i, j));
            fprintf(f, "%.2e\t", result.GetShift(i, j));
            fprintf(f, "%.2e\t", result.GetShiftError(i, j));
            fprintf(f, "%.2e\t", result.GetSqueeze(i, j));
            fprintf(f, "%.2e\t", result.GetSqueezeError(i, j));
        }

        fprintf(f, "\n");
    }
    fprintf(f, "</spectraldata>\n");

    fclose(f);

    return true;
}


bool CReEvaluator::PrepareEvaluation()
{
    for (int curWindow = 0; curWindow < m_windowNum; ++curWindow)
    {
        if (!CreateOutputDirectory())
        {
            return false;
        }

        if (!ReadReferences(m_window[curWindow]))
        {
            MessageBox(NULL, "Not all references could be read. Please check settings and start again", "Error in settings", MB_OK);
            return false;
        }

        /* then create the evaluation log and write its header */
        if (!WriteEvaluationLogHeader(curWindow))
        {
            return false;
        }
    }

    return true;
}

void CReEvaluator::SortScans() {
    CString	tmp;
    bool change;

    // TODO: Change this. Use std::sort?
    do {
        change = false;
        for (int k = 0; k < m_scanFileNum - 1; ++k) {
            CString& str1 = m_scanFile.GetAt(k);
            CString& str2 = m_scanFile.GetAt(k + 1);

            if (str1.Compare(str2) > 0) {
                tmp = m_scanFile.GetAt(k); //copy
                m_scanFile.SetAt(k, m_scanFile.GetAt(k + 1));
                m_scanFile.SetAt(k + 1, tmp);
                change = true;
            }
            else {
                continue;
            }
        }
    } while (change);
}