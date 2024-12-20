#include "StdAfx.h"
#include "../NovacMasterProgram.h"
#include "ReEvaluator.h"

#include "../Common/Version.h"
#include "../Evaluation/ScanEvaluation.h"
#include "../Dialogs/QueryStringDialog.h"
#include "../NovacProgramLog.h"
#include <SpectralEvaluation/StringUtils.h>
#include <SpectralEvaluation/File/File.h>
#include <SpectralEvaluation/Spectra/SpectrometerModel.h>

#include <sstream>

using namespace Evaluation;
using namespace novac;

namespace ReEvaluation
{

CReEvaluator::CReEvaluator()
{
    // initialize the scan file list
    m_scanFile.reserve(64); // <-- The initial guess of the number of pak-files

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
}

/** Halts the current operation */
bool CReEvaluator::Stop()
{
    fRun = false;
    return true;
}

bool CReEvaluator::DoEvaluation()
{
    MakeInitialSanityCheck();

    /** Prepare everything for evaluating */
    if (!PrepareEvaluation())
    {
        return false;
    }

    /* evaluate the spectra */
    m_progress = 0;

    NovacProgramLog log;

    // The CScanEvaluation-object handles the evaluation of one single scan.
    CScanEvaluation ev(log);
    ev.pView = this->pView;
    ev.m_pause = &m_pause;
    ev.m_sleeping = &m_sleeping;

    // Set the options for the CScanEvaluation object
    ev.SetOption_Sky(m_skySettings);
    ev.SetOption_Ignore(m_ignore_Lower, m_ignore_Upper);
    ev.SetOption_AveragedSpectra(m_averagedSpectra);

    // loop through all the scan files
    for (size_t curScanFile = 0; curScanFile < m_scanFile.size(); ++curScanFile)
    {
        m_progress = curScanFile / (double)m_scanFile.size();
        m_progress *= 1000.0;
        if (pView != nullptr)
        {
            pView->PostMessage(WM_PROGRESS, (WPARAM)m_progress);
        }

        // The CScanFileHandler is a structure for reading the spectral information from the scan-file
        CScanFileHandler scan(m_log);

        // Check the scan file
        const std::string scanFileName(m_scanFile[curScanFile]);
        novac::LogContext context(novac::LogContext::FileName, novac::GetFileName(scanFileName));
        if (SUCCESS != scan.CheckScanFile(context, scanFileName))
        {
            CString errStr;
            errStr.Format("Could not read scan-file %s", m_scanFile[curScanFile]);
            MessageBox(nullptr, errStr, "Error", MB_OK);
            continue;
        }

        // update the status window
        m_statusMsg.Format("Evaluating scan number %d", curScanFile);
        m_log.Information(context, std::string(m_statusMsg));

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
                    if (IDNO == MessageBox(nullptr, message, "Saturated sky spectrum?", MB_YESNO))
                    {
                        break; // continue with the next scan-file
                    }
                }
            }

            // Evaluate the scan-file
            int success = ev.EvaluateScan(context, m_scanFile[curScanFile], m_window[m_curWindow], &fRun, &m_darkSettings);

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

void CReEvaluator::MakeInitialSanityCheck() const
{
    if (this->m_windowNum <= 0)
    {
        throw std::invalid_argument("Cannot run re-evaluation, no fit windows have been defined.");
    }

    if (m_windowNum > MAX_N_WINDOWS)
    {
        throw std::invalid_argument("Cannot run re-evaluation, too many fit windows have been defined.");
    }

    if (this->m_scanFile.size() == 0)
    {
        throw std::invalid_argument("Cannot run re-evaluation, no scan files have been added.");
    }
}

bool CReEvaluator::CreateOutputDirectory()
{
    struct tm* tim;
    time_t t;
    time(&t);
    tim = localtime(&t);

    CString cDateTime;
    cDateTime.Format("%04d%02d%02d_%02d%02d", tim->tm_year + 1900, tim->tm_mon + 1, tim->tm_mday, tim->tm_hour, tim->tm_min);

    // Get the directory name
    CString fileName(m_scanFile[0].c_str());
    Common::GetFileName(fileName);
    CString path(m_scanFile[0].c_str());
    Common::GetDirectory(path); // gets the directory of the scan-file
    m_outputDir.Format("%s\\ReEvaluation_%s_%s", (LPCTSTR)path, (LPCTSTR)fileName, (LPCTSTR)cDateTime);

    // Create the directory
    if (0 == CreateDirectory(m_outputDir, nullptr))
    {
        DWORD errorCode = GetLastError();
        if (errorCode != ERROR_ALREADY_EXISTS)
        {
            /* We shouldn't quit just because the directory that we want to create already exists. */
            CString tmpStr;
            tmpStr.Format("Could not create output directory. Error code returned %ld. Do you want to create an output directory elsewhere?", errorCode);
            int ret = MessageBox(nullptr, tmpStr, "Could not create output directory", MB_YESNO);
            if (ret == IDNO)
            {
                return false;
            }
            else
            {
                // Create the output-directory somewhere else
                Dialogs::CQueryStringDialog pathDialog;
                pathDialog.m_windowText.Format("The path to the output directory?");
                pathDialog.m_inputString = &path;
                INT_PTR ret = pathDialog.DoModal();
                if (IDCANCEL == ret)
                {
                    return false;
                }
                m_outputDir.Format("%s\\ReEvaluation_%s_%s", (LPCTSTR)path, (LPCTSTR)fileName, (LPCTSTR)cDateTime);
                if (0 == CreateDirectory(m_outputDir, nullptr))
                {
                    tmpStr.Format("Could not create output directory. ReEvaluation aborted.");
                    MessageBox(nullptr, tmpStr, "ERROR", MB_OK);
                    return false;
                }
            }
        }
    }
    return true;
}

void CReEvaluator::WriteEvaluationLogHeader(int fitWindowIndex)
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
        throw std::invalid_argument("Could not create evaluation-log file, evaluation aborted");
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
    switch (m_skySettings.skyOption)
    {
    case Configuration::SKY_OPTION::MEASURED_IN_SCAN:                   fprintf(f, "#Sky: First spectrum in scanFile\n");	break;
    case Configuration::SKY_OPTION::USER_SUPPLIED:                      fprintf(f, "#Sky: %s\n", m_skySettings.skySpectrumFile.c_str()); break;
    case Configuration::SKY_OPTION::SPECTRUM_INDEX_IN_SCAN:             fprintf(f, "#Sky: Spectrum number %d", m_skySettings.indexInScan); break;
    case Configuration::SKY_OPTION::AVERAGE_OF_GOOD_SPECTRA_IN_SCAN:    fprintf(f, "#Sky: Average of all good spectra in scanFile\n");
    }

    // Write the region used:
    fprintf(f, "#OffsetRemovedFrom: %zd\n", window.offsetRemovalRange.from);
    fprintf(f, "#OffsetRemovedTo: %zd\n", window.offsetRemovalRange.to);

    // If the spectra are averaged or not
    if (m_averagedSpectra)
        fprintf(f, "#Spectra are averaged, not summed\n");

    // The reference-files
    fprintf(f, "#nSpecies=%d\n", window.NumberOfReferences());
    fprintf(f, "#Specie\tShift\tSqueeze\tReferenceFile\n");
    for (const auto& reference : window.reference)
    {
        fprintf(f, "#%s\t", reference.m_specieName.c_str());
        switch (reference.m_shiftOption)
        {
        case SHIFT_TYPE::SHIFT_FIX:
            fprintf(f, "%0.3lf\t", reference.m_shiftValue); break;
        case SHIFT_TYPE::SHIFT_LINK:
            fprintf(f, "linked to %s\t", window.reference[static_cast<size_t>(reference.m_shiftValue)].m_specieName.c_str()); break;
        case SHIFT_TYPE::SHIFT_LIMIT:
            fprintf(f, "limited to +-%0.3lf\t", reference.m_shiftValue);
        default:
            fprintf(f, "free\t"); break;
        }

        switch (reference.m_squeezeOption)
        {
        case SHIFT_TYPE::SHIFT_FIX:
            fprintf(f, "%0.3lf\t", reference.m_squeezeValue); break;
        case SHIFT_TYPE::SHIFT_LINK:
            fprintf(f, "linked to %s\t", window.reference[static_cast<size_t>(reference.m_squeezeValue)].m_specieName.c_str()); break;
        case SHIFT_TYPE::SHIFT_LIMIT:
            fprintf(f, "limited to +-%0.3lf\t", reference.m_squeezeValue);
        default:
            fprintf(f, "free\t"); break;
        }
        fprintf(f, "%s\n", reference.m_path.c_str());
    }
    fprintf(f, "\n");

    fclose(f);
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
    const novac::MeasurementMode mode = novac::CheckMeasurementMode(result);
    fprintf(f, "\tmode=%s\n", novac::ToString(mode).c_str());

    // Finally, the version of the file and the version of the program
    fprintf(f, "\tsoftwareversion=%d.%d\n", CVersion::majorNumber, CVersion::minorNumber);
    fprintf(f, "\tcompiledate=%s\n", __DATE__);

    fprintf(f, "</scaninformation>\n");

    // Write the header
    fprintf(f, "#scanangle\tstarttime\tstoptime\tname\tdelta\tchisquare\texposuretime\tnumspec\tintensity\tfitintensity\tisgoodpoint\toffset\tflag\t");
    for (const auto& reference : window.reference)
    {
        fprintf(f, "column(%s)\tcolumnerror(%s)\t", reference.m_specieName.c_str(), reference.m_specieName.c_str());
        fprintf(f, "shift(%s)\tshifterror(%s)\t", reference.m_specieName.c_str(), reference.m_specieName.c_str());
        fprintf(f, "squeeze(%s)\tsqueezeerror(%s)\t", reference.m_specieName.c_str(), reference.m_specieName.c_str());
    }

    if (window.fitType == novac::FIT_TYPE::FIT_HP_SUB || window.fitType == novac::FIT_TYPE::FIT_POLY)
    {
        const char* name = "fraunhoferref";

        fprintf(f, "column(%s)\tcolumnerror(%s)\t", name, name);
        fprintf(f, "shift(%s)\tshifterror(%s)\t", name, name);
        fprintf(f, "squeeze(%s)\tsqueezeerror(%s)", name, name);
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
        int nRef = window.NumberOfReferences();
        if (window.fitType == novac::FIT_TYPE::FIT_HP_SUB || window.fitType == novac::FIT_TYPE::FIT_POLY)
        {
            ++nRef;
        }

        // The Result
        for (int j = 0; j < nRef; ++j)
        {

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
    try
    {
        for (int curWindow = 0; curWindow < m_windowNum; ++curWindow)
        {
            if (!CreateOutputDirectory())
            {
                return false;
            }

            ReadReferences(m_window[curWindow]);

            WriteEvaluationLogHeader(curWindow);
        }
    }
    catch (novac::InvalidReferenceException& ex)
    {
        std::stringstream msg;
        msg << ex.what() << ". Please check settings and start again.";
        throw std::invalid_argument(msg.str());
    }

    return true;
}

void CReEvaluator::SortScans()
{
    std::sort(begin(m_scanFile), end(m_scanFile));
}

}  // namespace ReEvaluation
