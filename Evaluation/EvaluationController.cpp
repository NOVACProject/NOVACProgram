#include "StdAfx.h"
#include "evaluationcontroller.h"
#include "ScanEvaluation.h"
#include <SpectralEvaluation/File/SpectrumIO.h>
#include <SpectralEvaluation/File/File.h>
#include "../Common/Version.h"
#include "../NovacProgramLog.h"

#include <sstream>

#ifdef _MSC_VER
#pragma warning (push, 4)
#endif

// we also need the meterological data
#include "../Meteorology/MeteorologicalData.h"

// ... and the settings
#include "../Configuration/Configuration.h"

// ... the judgements for making real-time wind measurements
#include "../WindMeasurement/RealTimeWind.h"

// ... the judgements for making real-time composition measurements
#include "../Common/CompositionMeasurement.h"

// ... the judgements for making instrument calibrations (pixel to wavelength and instrument line shape).
#include "RealTimeCalibration.h"

// ... support for handling the evaluation-log files...
#include "../Common/EvaluationLogFileHandler.h"


using namespace Evaluation;
using namespace FileHandler;
using namespace novac;

extern CMeteorologicalData      g_metData;      // <-- The meteorological data
extern CConfigurationSetting    g_settings;     // <-- The settings
extern CFormView* pView;        // <-- The screen
extern CWinThread* g_windMeas;  // <-- The thread that evaluates the wind-measurements.
extern CWinThread* g_geometry;  // <-- The thread that performes geometrical calculations from the scans

UINT primaryLanguage;
UINT subLanguage;

IMPLEMENT_DYNCREATE(CEvaluationController, CWinThread)

BEGIN_MESSAGE_MAP(CEvaluationController, CWinThread)
    ON_THREAD_MESSAGE(WM_ARRIVED_SPECTRA, OnArrivedSpectra)
    ON_THREAD_MESSAGE(WM_QUIT, OnQuit)
END_MESSAGE_MAP()

CEvaluationController::CEvaluationController(void)
{
    m_fluxSpecie = "SO2";
    m_realTime = false;
    m_date[0] = m_date[1] = m_date[2] = 0;
}

CEvaluationController::~CEvaluationController(void)
{
    for (int i = 0; i < m_spectrometer.GetSize(); ++i)
    {
        if (m_spectrometer[i] != nullptr)
        {
            delete m_spectrometer[i];
            m_spectrometer[i] = nullptr;
        }
    }
}

/** Quits the thread */
void CEvaluationController::OnQuit(WPARAM /*wp*/, LPARAM /*lp*/)
{
    for (int i = 0; i < m_spectrometer.GetSize(); ++i)
    {
        if (m_spectrometer[i] != nullptr)
        {
            delete m_spectrometer[i];
            m_spectrometer[i] = nullptr;
        }
    }
}

void CEvaluationController::OnArrivedSpectra(WPARAM wp, LPARAM /*lp*/)
{
    // The filename of the newly arrived file is the first parameter 
    if ((CString*)wp == nullptr)
    {
        CString errorMessage = "EvaluationController recieved filename with null file name.";
        m_logFileWriter.WriteErrorMessage(errorMessage);
        ShowMessage(errorMessage);
        return;
    }

    const CString fileName = *((CString*)wp); // make a local copy and delete the original
    {
        CString* original = (CString*)wp;
        delete original;
    }

    CString message;
    CString storeFileName_pak, storeFileName_txt;
    CSpectrumIO reader;
    CSpectrum spec;
    bool isFullScan = true;

    // 1. Check if the file exists
    if (!IsExistingFile(fileName))
    {
        CString errorMessage;
        errorMessage.Format("EvaluationController recieved filename with erroneous filePath: %s ", (LPCSTR)fileName);
        m_logFileWriter.WriteErrorMessage(errorMessage);
        ShowMessage(errorMessage);
        return;
    }

    // 2. Find the serial number of the spectrometer and the channel that was used
    const std::string fileNameStr(fileName);
    reader.ReadSpectrum(fileNameStr, 0, spec); // TODO: check for errors!!
    const CString serialNumber(spec.m_info.m_device.c_str());
    const int specPerScan = spec.SpectraPerScan();

    // 3. Find the volcano that the spectrometer with this serial-number monitors
    const int volcanoIndex = Common::GetMonitoredVolcano(serialNumber);

    // 4. Check so that this file contains one full scan
    const novac::MeasurementMode measurementMode = CPakFileHandler::GetMeasurementMode(fileName);

    if (measurementMode == novac::MeasurementMode::Flux)
    {
        const int nSpectra = reader.CountSpectra(fileNameStr);
        isFullScan = (specPerScan == nSpectra); // TODO: will this work if there are repetitions??
    }

    // 5. Evaluate the scan
    EvaluateScan(fileName, volcanoIndex); // TODO: Check for errors

    // 6. Move the file to the archive
    GetArchivingfileName(storeFileName_pak, storeFileName_txt, fileName);
    if (0 == MoveFileEx(fileName, storeFileName_pak, MOVEFILE_REPLACE_EXISTING))
    {// after evaluation, move the file to the archive
        DWORD errorCode = GetLastError();
        message.Format("Could not move file");
        CString str;
        if (Common::FormatErrorCode(errorCode, str))
            message.AppendFormat("Reason - %s", (LPCSTR)str);
        else
            message.AppendFormat("Reason - unknown");
        ShowMessage(message);
        // Try to copy the file instead...
        CopyFile(fileName, storeFileName_pak, TRUE);
    }

    // 7. Upload the file(s) to the data-server
    UploadToNOVACServer(storeFileName_pak, volcanoIndex);
    UploadToNOVACServer(storeFileName_txt, volcanoIndex);

    // 8. If this is a wind-speed measurement, tell the wind-evaluation thread about it
    if (measurementMode == novac::MeasurementMode::Windspeed)
    {
        MakeWindMeasurement(storeFileName_txt, volcanoIndex);
    }
    else if (measurementMode == novac::MeasurementMode::Stratosphere ||
                measurementMode == novac::MeasurementMode::DirectSun ||
                measurementMode == novac::MeasurementMode::Lunar ||
                measurementMode == novac::MeasurementMode::Composition)
    {
        // TODO!!!
    }
    else
    {
        MakeGeometryCalculations(storeFileName_txt, volcanoIndex);
    }

    // 9. Tell the user...
    if (novac::MeasurementMode::Windspeed == measurementMode)
    {
        message.Format("Recieved wind measurement from %s. Scan evaluated and stored as %s.", (LPCSTR)serialNumber, (LPCSTR)storeFileName_pak);
    }
    else if (novac::MeasurementMode::Stratosphere == measurementMode)
    {
        message.Format("Recieved stratospheric measurement from %s. Scan evaluated and stored as %s.", (LPCSTR)serialNumber, (LPCSTR)storeFileName_pak);
    }
    else if (novac::MeasurementMode::DirectSun == measurementMode)
    {
        message.Format("Recieved direct-sun measurement from %s. Scan evaluated and stored as %s.", (LPCSTR)serialNumber, (LPCSTR)storeFileName_pak);
    }
    else if (novac::MeasurementMode::Lunar == measurementMode)
    {
        message.Format("Recieved lunar measurement from %s. Scan evaluated and stored as %s.", (LPCSTR)serialNumber, (LPCSTR)storeFileName_pak);
    }
    else if (novac::MeasurementMode::Composition == measurementMode)
    {
        message.Format("Recieved composition measurement from %s. Scan evaluated and stored as %s.", (LPCSTR)serialNumber, (LPCSTR)storeFileName_pak);
    }
    else
    {
        if (isFullScan)
        {
            message.Format("Received full scan from %s. Scan evaluated and stored as %s.", (LPCSTR)serialNumber, (LPCSTR)storeFileName_pak);
        }
        else
        {
            message.Format("Received incomplete scan from %s. Scan evaluated and stored.", (LPCSTR)serialNumber);
        }
    }
    ShowMessage(message);

    // 10. If the user wants to execute a script, do that!
    if (strlen(g_settings.externalSetting.fullScanScript) > 2)
    {
        ExecuteScript_FullScan(storeFileName_pak, storeFileName_txt);
    }

    // 11. Clean Up
    DeleteFile(fileName);   // If the file still exists, try to delete it.
}

/** This function takes a scan-file and evaluates one of the spectra inside it */
RETURN_CODE CEvaluationController::EvaluateSpectrum(const CString& /*fileName*/, int /*spectrumIndex*/, CScanResult* /*result*/)
{

    return FAIL;
}

/** This function takes care of the evaluation of one scan.	*/
RETURN_CODE CEvaluationController::EvaluateScan(const CString& fileName, int volcanoIndex)
{
    clock_t cStart, cFinish;
    CString message;
    CWindField windField;
    CDateTime startTime;

    // The CScanFileHandler is a structure for reading the 
    //		spectral information from the scan-file
    std::unique_ptr<CScanFileHandler> scan = std::make_unique<CScanFileHandler>(m_log);

    // success is true if the evaluation is successful
    bool success = true;

    // Check if the output directories needs to be updated
    UpdateOutputDirectories();

    // clock the time it takes to treat one scan
    cStart = clock();

    /** ------------- The process to evaluate a scan --------------- */

    // 1. Assert that the scan-file exists
    if (!IsExistingFile(fileName))
    {
        m_logFileWriter.WriteErrorMessage(TEXT("Recieved scan with illegal path. Could not evaluate."));
        return FAIL;
    }

    // 2. Read the scan file
    const std::string fileNameStr((LPCSTR)fileName);
    novac::LogContext context(novac::LogContext::FileName, novac::GetFileName(fileNameStr));
    if (!scan->CheckScanFile(context, fileNameStr))
    {
        m_logFileWriter.WriteErrorMessage(TEXT("Could not read recieved scan"));
        return FAIL;
    }

    // 3. Identify which spectrometer has generated this scan
    CSpectrometer* spectrometer = IdentifySpectrometer(*scan);
    if (nullptr == spectrometer)
    {
        Output_SpectrometerNotIdentified();
        return FAIL;
    }
    Output_ArrivedScan(spectrometer);

    // 4. Get information about the spectra, like compass direction, gps, etc...
    GetSpectrumInformation(spectrometer, fileName);

    // 5. Evaluate the scan
    NovacProgramLog log;
    std::unique_ptr<CScanEvaluation> ev = std::make_unique<CScanEvaluation>(log);
    ev->m_pause = nullptr;
    const Configuration::CDarkSettings* darkSettings = &spectrometer->m_settings.channel[0].darkSettings;
    const long numberOfEvaluatedSpectra = ev->EvaluateScan(context, fileNameStr, spectrometer->m_fitWindows[0], nullptr, darkSettings);

    // 6. Get the result from the evaluation
    std::unique_ptr<CScanResult> scanEvaluationResult = ev->GetResult();

    // 7. Check the reasonability of the evaluation
    if (numberOfEvaluatedSpectra == 0 || scanEvaluationResult == nullptr)
    {
        Output_EmptyScan(spectrometer);
        return FAIL;
    }

    // 8. Get the mode of the evaluation
    scanEvaluationResult->m_measurementMode = novac::CheckMeasurementMode(*scanEvaluationResult);

    scanEvaluationResult->GetStartTime(0, startTime);

    // 9. Get the local wind field when the scan was taken
    if (SUCCESS != GetWind(windField, *spectrometer, startTime))
    {
        spectrometer->m_logFileHandler.WriteErrorMessage(m_common.GetString(ERROR_WIND_NOT_FOUND));
    }
    // 10. Calculate the flux. The spectrometer is needed to identify the geometry.
    if (scanEvaluationResult->m_measurementMode != novac::MeasurementMode::Windspeed &&
        scanEvaluationResult->m_measurementMode != novac::MeasurementMode::Windspeed &&
        scanEvaluationResult->m_measurementMode != novac::MeasurementMode::Stratosphere &&
        scanEvaluationResult->m_measurementMode != novac::MeasurementMode::DirectSun &&
        scanEvaluationResult->m_measurementMode != novac::MeasurementMode::Lunar &&
        scanEvaluationResult->m_measurementMode != novac::MeasurementMode::Composition)
    {
        // 10a. Calculate the centre of the plume
        (void)scanEvaluationResult->CalculatePlumeCentre("SO2");

        // 10c. Calculate the flux...
        if (SUCCESS != CalculateFlux(scanEvaluationResult.get(), spectrometer, volcanoIndex, windField))
        {
            Output_FluxFailure(scanEvaluationResult, spectrometer);
            success = false;
        }
    }

    // 11. Append the result to the log file of the corresponding scanningInstrument
    if (SUCCESS != WriteEvaluationResult(scanEvaluationResult.get(), *scan, *spectrometer, windField, volcanoIndex))
    {
        spectrometer->m_logFileHandler.WriteErrorMessage(TEXT("Could not write result to file"));
    }

    // 12. Remember the result from the last scan
    spectrometer->RememberResult(*scanEvaluationResult);

    // 13. Check if we should do a wind-measurement or a composition mode measurement now
    InitiateSpecialModeMeasurement(spectrometer);

    // 14. Run a calibration of the device, if the time is right and the scan seems to be good.
    UpdateInstrumentCalibration(*spectrometer, fileNameStr, scanEvaluationResult->GetStartTime(0));

    // 15. Calculate the time spent in this function
    cFinish = clock();
    Output_TimingOfScanEvaluation(numberOfEvaluatedSpectra, spectrometer->SerialNumber(), ((double)(cFinish - cStart) / (double)CLOCKS_PER_SEC));

    // TODO: Check that this is ok.
    scanEvaluationResult->m_path = std::string((LPCSTR)fileName);

    // 16. Share the results with the rest of the program
    if (success)
    {
        CScanResult* newResult = new CScanResult(*scanEvaluationResult);
        pView->PostMessage(WM_EVAL_SUCCESS, (WPARAM) & (spectrometer->SerialNumber()), (LPARAM)newResult);
    }

    return SUCCESS;
}

CSpectrometer* CEvaluationController::IdentifySpectrometer(const CScanFileHandler& scan)
{
    unsigned char channel = scan.m_channel;
    if (true == CPakFileHandler::CorrectChannelNumber(channel))
    {
        return nullptr; // cannot handle multichannel spectra here!!!
    }

    // find the spectrometer that corresponds to the serial number
    for (long spectrometerNum = 0; spectrometerNum < m_spectrometer.GetSize(); ++spectrometerNum)
    {
        const CString deviceName(scan.m_device.c_str());
        if (Equals(m_spectrometer[spectrometerNum]->SerialNumber(), deviceName))
        {
            if (channel == m_spectrometer[spectrometerNum]->m_channel)
            {
                return m_spectrometer[spectrometerNum];
            }
        }
    }

    const CString deviceName(scan.m_device.c_str());
    return HandleUnIdentifiedSpectrometer(deviceName, scan);
}

RETURN_CODE CEvaluationController::CalculateFlux(CScanResult* result, const CSpectrometer* spectrometer, int volcanoIndex, CWindField& windField)
{
    CString errorMessage;

    // 0. If there's only one specie evaluated for, use it as flux specie. 
    //		No matter what previously said
    if (result->GetSpecieNum(0) == 1)
    {
        m_fluxSpecie = result->GetSpecieName(0, 0);
    }
    else
    {
        m_fluxSpecie = "SO2";
    }

    // 1. Get the offset level of the scan
    if (result->CalculateOffset(m_fluxSpecie))
    {
        if (!result->IsEvaluatedSpecie(m_fluxSpecie))
        {
            spectrometer->m_logFileHandler.WriteErrorMessage(TEXT("Could not calculate scan offset. There is no reference file for: " + CString(m_fluxSpecie.c_str()) + " for this spectrometer!"));
        }
        else
        {
            spectrometer->m_logFileHandler.WriteErrorMessage(TEXT("Could not calculate scan offset. Is there a reference file for: " + CString(m_fluxSpecie.c_str()) + " for this spectrometer ?"));
        }
    }

    // 2. Check the direction of the scanner, if it's a cone and pointing to the
    //		volcano, then change the compass direction to be away from the volcano
#ifdef _DEBUG
    if (spectrometer->m_scanner.compass > 270.5 || spectrometer->m_scanner.compass < 270.3)
        printf("ojdå");
#endif

    // 2. Calculate the flux
    if (result->CalculateFlux(m_fluxSpecie, windField, fmod(spectrometer->m_scanner.compass, 360.0), spectrometer->m_scanner.coneAngle, spectrometer->m_scanner.tilt))
    {
        spectrometer->m_logFileHandler.WriteErrorMessage("Could not calculate flux for scan");
    }

    // 3. Append the result of the flux calculation to the flux-log file
    if (SUCCESS != WriteFluxResult(result, *spectrometer, windField, volcanoIndex))
    {
        spectrometer->m_logFileHandler.WriteErrorMessage(TEXT("Could not write result to file"));
    }

    return SUCCESS;
}

/** Writes the result of the flux - calculation to the flux-log file */
RETURN_CODE CEvaluationController::WriteFluxResult(const CScanResult* result, const CSpectrometer& spectrometer, const CWindField& windField, int volcanoIndex)
{
    CString string, dateStr, dateStr2, serialNumber;
    CString fluxLogFile, directory;
    CString wdSrc, wsSrc, phSrc;
    CString errorMessage;
    CDateTime dateTime;
    double edge1, edge2;

    // 0. Get the sources for the wind-field
    windField.GetWindSpeedSource(wsSrc);
    windField.GetWindDirectionSource(wdSrc);
    windField.GetPlumeHeightSource(phSrc);

    // 1. Output the day and time the scan that generated this measurement started
    result->GetSkyStartTime(dateTime);
    dateStr.Format("%04d-%02d-%02d", dateTime.year, dateTime.month, dateTime.day);
    dateStr2.Format("%04d.%02d.%02d", dateTime.year, dateTime.month, dateTime.day);
    string.Format("%s\t", (LPCSTR)dateStr);
    string.AppendFormat("%02d:%02d:%02d\t", dateTime.hour, dateTime.minute, dateTime.second);

    // 2. Output the time the scan stopped
    result->GetStopTime(result->GetEvaluatedNum() - 1, dateTime);
    string.AppendFormat("%02d:%02d:%02d\t", dateTime.hour, dateTime.minute, dateTime.second);

    // 3. Output the flux result
    string.AppendFormat("%.2lf\t", result->GetFlux());

    // 4. Output the wind speed that was used for calculating this flux
    string.AppendFormat("%.3lf\t", windField.GetWindSpeed());

    // 5. Output the wind direction that was used for calculating this flux
    string.AppendFormat("%.3lf\t", windField.GetWindDirection());

    // 6. Output where we got the wind speed from
    string.AppendFormat("%s\t", (LPCSTR)wsSrc);

    // 7. Output where we got the wind direction from
    string.AppendFormat("%s\t", (LPCSTR)wdSrc);

    // 8. Output the plume height that was used for calculating this flux
    string.AppendFormat("%.1lf\t", windField.GetPlumeHeight());

    // 9. Output where we got the plume height from 
    string.AppendFormat("%s\t", (LPCSTR)phSrc);

    // 10. Output the compass direction
    string.AppendFormat("%.2lf\t", fmod(spectrometer.m_scanner.compass, 360.0));

    // 11. Output where we got the compass direction from
    if (fabs(spectrometer.m_scanner.compass) > 360)
        string.AppendFormat("compassreading\t");
    else
        string.AppendFormat("user\t");

    // 12. Output the plume centre
    string.AppendFormat("%.1lf\t", result->GetCalculatedPlumeCentre());

    // 13. Output the plume edges
    result->GetCalculatedPlumeEdges(edge1, edge2);
    string.AppendFormat("%.1lf\t%.1lf\t", edge1, edge2);

    // 14. Output the plume completeness
    string.AppendFormat("%.2lf\t", result->GetCalculatedPlumeCompleteness());

    // 15. Output the cone-angle of the scanner
    string.AppendFormat("%.1lf\t", spectrometer.m_scanner.coneAngle);

    // 16. Output the tilt of the scanner
    string.AppendFormat("%.1lf\t", spectrometer.m_scanner.tilt);

    // 17. Output whether we think this is a good measurement or not
    if (result->IsFluxOk())
        string.AppendFormat("1\t");
    else
        string.AppendFormat("0\t");

    // 18. Output the instrument temperature
    string.AppendFormat("%.1lf\t", result->GetTemperature());

    // 19. Output the instrument battery voltage
    string.AppendFormat("%.1lf\t", result->GetBatteryVoltage());

    // 20. Output the exposure-time
    string.AppendFormat("%.1ld\t", result->GetSkySpectrumInfo().m_exposureTime);

    // 21. Find the name of the flux-log file to write to

    // 21a. Make the directory
    serialNumber.Format("%s", (LPCSTR)spectrometer.SerialNumber());
    directory.Format("%sOutput\\%s\\%s\\", (LPCSTR)g_settings.outputDirectory, (LPCSTR)dateStr2, (LPCSTR)serialNumber);
    if (CreateDirectoryStructure(directory))
    {
        Common common;
        common.GetExePath();
        directory.Format("%sOutput\\%s", (LPCSTR)common.m_exePath, (LPCSTR)dateStr2);
        if (CreateDirectoryStructure(directory))
        {
            errorMessage.Format("Could not create storage directory for flux-data. Please check settings and restart.");
            ShowMessage(errorMessage);
            MessageBox(nullptr, errorMessage, "Serious Error", MB_OK);
            return FAIL;
        }
    }

    // 21b. Get the file-name
    fluxLogFile.Format("%sFluxLog_%s_%s.txt", (LPCSTR)directory, (LPCSTR)serialNumber, (LPCSTR)dateStr2);

    // 21c. Check if the file exists
    if (!IsExistingFile(fluxLogFile))
    {
        // write the header
        FILE* f = fopen(fluxLogFile, "w");
        if (f != nullptr)
        {
            fprintf(f, "serial=%s\n", (LPCSTR)serialNumber);
            fprintf(f, "volcano=%s\n", (LPCSTR)m_common.SimplifyString(spectrometer.m_scanner.volcano));
            fprintf(f, "site=%s\n", (LPCSTR)m_common.SimplifyString(spectrometer.m_scanner.site));
            fprintf(f, "#scandate\tscanstarttime\tscanstoptime\t");
            fprintf(f, "flux_[kg/s]\t");
            fprintf(f, "windspeed_[m/s]\twinddirection_[deg]\twindspeedsource\twinddirectionsource\t");
            fprintf(f, "plumeheight_[m]\tplumeheightsource\t");
            fprintf(f, "compassdirection_[deg]\tcompasssource\t");
            fprintf(f, "plumecentre_[deg]\tplumeedge1_[deg]\tplumeedge2_[deg]\tplumecompleteness_[%%]\t");
            fprintf(f, "coneangle\ttilt\tokflux\ttemperature\tbatteryvoltage\texposuretime\n");
            fclose(f);
        }
    }

    // 21d. Write the flux-result to the file
    FILE* f = fopen(fluxLogFile, "a+");
    if (f != nullptr)
    {
        fprintf(f, string);
        fprintf(f, "\n");
        fclose(f);
    }

    // 22. Upload the flux-log file to the FTP-Server
    UploadToNOVACServer(fluxLogFile, volcanoIndex);

    return SUCCESS;
}

RETURN_CODE CEvaluationController::WriteEvaluationResult(const CScanResult* result, const CScanFileHandler& scan, const CSpectrometer& spectrometer, CWindField& windField, int volcanoIndex)
{
    CString string, string1, string2, string3, string4;
    const CConfigurationSetting::SpectrometerSetting& settings = spectrometer.m_settings;
    CString pakFile, txtFile, evalLogFile;
    CString wsSrc, wdSrc, phSrc;
    CDateTime dateTime;

    const CString fName(scan.GetFileName().c_str());
    GetArchivingfileName(pakFile, txtFile, fName);

    // 1. Get the name of the evaluation-log file to write to...
    //		The path is the top-directory of the text-file
    evalLogFile.Format(txtFile);
    m_common.GetDirectory(evalLogFile); // the directory where the pak/txt-files are
    evalLogFile = evalLogFile.Left((int)strlen(evalLogFile) - 1);	// remove the last backslash
    m_common.GetDirectory(evalLogFile); // the parent-directory to the pak/txt-files

    // The date of the measurement & the serial-number of the spectrometer
    result->GetSkyStartTime(dateTime);

    evalLogFile.AppendFormat("EvaluationLog_%s_%04d.%02d.%02d.txt", (LPCSTR)spectrometer.SerialNumber(), dateTime.year, dateTime.month, dateTime.day);

    // 0. Create the additional scan-information
    string.Format("<scaninformation>\n");
    string.AppendFormat("\tdate=%02d.%02d.%04d\n", scan.m_startTime.day, scan.m_startTime.month, scan.m_startTime.year);
    string.AppendFormat("\tstarttime=%02d:%02d:%02d\n", scan.m_startTime.hour, scan.m_startTime.minute, scan.m_startTime.second);
    string.AppendFormat("\tcompass=%.1lf\n", spectrometer.m_scanner.compass);
    string.AppendFormat("\ttilt=%.1lf\n", spectrometer.m_scanner.tilt);
    string.AppendFormat("\tlat=%.6lf\n", spectrometer.m_scanner.gps.m_latitude);
    string.AppendFormat("\tlong=%.6lf\n", spectrometer.m_scanner.gps.m_longitude);
    string.AppendFormat("\talt=%.3lf\n", spectrometer.m_scanner.gps.m_altitude);

    string.AppendFormat("\tvolcano=%s\n", (LPCSTR)m_common.SimplifyString(spectrometer.m_scanner.volcano));
    string.AppendFormat("\tsite=%s\n", (LPCSTR)m_common.SimplifyString(spectrometer.m_scanner.site));
    string.AppendFormat("\tobservatory=%s\n", (LPCSTR)m_common.SimplifyString(spectrometer.m_scanner.observatory));

    string.AppendFormat("\tserial=%s\n", (LPCSTR)settings.serialNumber);
    string.AppendFormat("\tspectrometer=%s\n", spectrometer.m_settings.modelName.c_str());
    string.AppendFormat("\tchannel=%d\n", spectrometer.m_channel);
    string.AppendFormat("\tconeangle=%.1lf\n", spectrometer.m_scanner.coneAngle);
    string.AppendFormat("\tinterlacesteps=%d\n", scan.GetInterlaceSteps());
    string.AppendFormat("\tstartchannel=%d\n", scan.GetStartChannel());
    string.AppendFormat("\tspectrumlength=%d\n", scan.GetSpectrumLength());
    string.AppendFormat("\tflux=%.2lf\n", result->GetFlux());
    string.AppendFormat("\tbattery=%.2f\n", result->GetBatteryVoltage());
    string.AppendFormat("\ttemperature=%.2f\n", result->GetTemperature());

    // The mode
    string.AppendFormat("\tmode=%s\n", novac::ToString(result->m_measurementMode).c_str());

    const double maxIntensity = std::max(spectrometer.GetMaxIntensity(), 1.0);

    // Finally, the version of the file and the version of the program
    string.AppendFormat("\tsoftwareversion=%d.%d\n", CVersion::majorNumber, CVersion::minorNumber);
    string.AppendFormat("\tcompiledate=%s\n", __DATE__);

    string.AppendFormat("</scaninformation>\n");

    // 0.1	Create an flux-information part and write it to the same file
    windField.GetWindSpeedSource(wsSrc);
    windField.GetWindDirectionSource(wdSrc);
    windField.GetPlumeHeightSource(phSrc);

    double plumeEdge1, plumeEdge2;
    double plumeCompleteness = result->GetCalculatedPlumeCompleteness();
    double plumeCentre1 = result->GetCalculatedPlumeCentre(0);
    result->GetCalculatedPlumeEdges(plumeEdge1, plumeEdge2);

    string.AppendFormat("<fluxinfo>\n");
    string.AppendFormat("\tflux=%.4lf\n", result->GetFlux()); // ton/day
    string.AppendFormat("\twindspeed=%.4lf\n", windField.GetWindSpeed());
    string.AppendFormat("\twinddirection=%.4lf\n", windField.GetWindDirection());
    string.AppendFormat("\tplumeheight=%.2lf\n", windField.GetPlumeHeight());
    string.AppendFormat("\twindspeedsource=%s\n", (LPCSTR)wsSrc);
    string.AppendFormat("\twinddirectionsource=%s\n", (LPCSTR)wdSrc);
    string.AppendFormat("\tplumeheightsource=%s\n", (LPCSTR)phSrc);
    if (fabs(spectrometer.m_scanner.compass) > 360.0)
        string.AppendFormat("\tcompasssource=compassreading\n");
    else
        string.AppendFormat("\tcompasssource=user\n");
    string.AppendFormat("\tplumecompleteness=%.2lf\n", plumeCompleteness);
    string.AppendFormat("\tplumecentre=%.2lf\n", plumeCentre1);
    string.AppendFormat("\tplumeedge1=%.2lf\n", plumeEdge1);
    string.AppendFormat("\tplumeedge2=%.2lf\n", plumeEdge2);
    string.AppendFormat("</fluxinfo>\n");

    // 0.2	Write spectrometer settings to file

    string.AppendFormat("<spectrometer>\n");
    string.AppendFormat("\t<serialNumber>%s</serialNumber>\n", (LPCSTR)settings.serialNumber);
    string.AppendFormat("\t<model>%s</model>\n", spectrometer.m_settings.modelName.c_str());
    for (int i = 0; i < settings.channelNum; i++)
    {
        string.AppendFormat("\t<channel number='%d'>\n", i);
        const novac::CFitWindow& fitWindow = settings.channel[i].fitWindow;
        for (const novac::CReferenceFile& ref : fitWindow.reference)
        {
            string.AppendFormat("\t\t<Reference>\n");
            string.AppendFormat("\t\t\t<name>%s</name>\n", ref.m_specieName.c_str());
            string.AppendFormat("\t\t\t<path>%s</path>\n", ref.m_path.c_str());

            CString shiftString;
            if (ref.m_shiftOption == novac::SHIFT_TYPE::SHIFT_FIX)
                shiftString.Format("fix to %.2lf", ref.m_shiftValue);
            else if (ref.m_shiftOption == novac::SHIFT_TYPE::SHIFT_FREE)
                shiftString.Format("free");
            else if (ref.m_shiftOption == novac::SHIFT_TYPE::SHIFT_LIMIT)
                shiftString.Format("limit from %.2lf to %.2lf", ref.m_shiftValue, ref.m_shiftMaxValue);
            else if (ref.m_shiftOption == novac::SHIFT_TYPE::SHIFT_LINK)
                shiftString.Format("link to %.0lf", ref.m_shiftValue);
            string.AppendFormat("\t\t\t<shift>%s</shift>\n", (LPCSTR)shiftString);

            CString squeezeString;
            if (ref.m_squeezeOption == novac::SHIFT_TYPE::SHIFT_FIX)
                squeezeString.Format("fix to %.2lf", ref.m_squeezeValue);
            else if (ref.m_squeezeOption == novac::SHIFT_TYPE::SHIFT_FREE)
                squeezeString.Format("free");
            else if (ref.m_squeezeOption == novac::SHIFT_TYPE::SHIFT_LIMIT)
                squeezeString.Format("limit from %.2lf to %.2lf", ref.m_squeezeValue, ref.m_squeezeMaxValue);
            else if (ref.m_squeezeOption == novac::SHIFT_TYPE::SHIFT_LINK)
                squeezeString.Format("link to %.0lf", ref.m_squeezeValue);
            string.AppendFormat("\t\t\t<squeeze>%s</squeeze>\n", (LPCSTR)squeezeString);

            string.AppendFormat("\t\t</Reference>\n");
        }
        string.AppendFormat("\t\t<fitLow>%d</fitLow>\n", fitWindow.fitLow);
        string.AppendFormat("\t\t<fitHigh>%d</fitHigh>\n", fitWindow.fitHigh);
        string.AppendFormat("\t\t<Reference>\n");

        string.AppendFormat("\t</channel>\n");
    }
    string.AppendFormat("</spectrometer>\n");

    // 1. write the header
    const novac::CFitWindow& window = settings.channel[0].fitWindow;
    string.AppendFormat("#scanangle\tstarttime\tstoptime\tname\tspecsaturation\tfitsaturation\tcounts_ms\tdelta\tchisquare\texposuretime\tnumspec\t");

    for (const novac::CReferenceFile& ref : spectrometer.m_fitWindows[0].reference)
    {
        string.AppendFormat("column(%s)\tcolumnerror(%s)\t", ref.m_specieName.c_str(), ref.m_specieName.c_str());
        string.AppendFormat("shift(%s)\tshifterror(%s)\t", ref.m_specieName.c_str(), ref.m_specieName.c_str());
        string.AppendFormat("squeeze(%s)\tsqueezeerror(%s)\t", ref.m_specieName.c_str(), ref.m_specieName.c_str());
    }
    string.AppendFormat("isgoodpoint\toffset\tflag");
    string.AppendFormat("\n<spectraldata>\n");

    // ----------------------------------------------------------------------------------------------
    // 2. ----------------- Write the parameters for the sky and the dark-spectra -------------------
    // ----------------------------------------------------------------------------------------------
    CSpectrum sky, dark, darkCurrent, offset;
    string1.Format(""); string2.Format(""); string3.Format(""); string4.Format("");
    scan.GetSky(sky);
    if (sky.m_info.m_interlaceStep > 1)
        sky.InterpolateSpectrum();
    if (sky.m_length > 0)
    {
        sky.m_info.m_fitIntensity = (float)(sky.MaxValue(window.fitLow, window.fitHigh));
        if (sky.NumSpectra() > 0)
            sky.Div(sky.NumSpectra());
        CEvaluationLogFileHandler::FormatEvaluationResult(&sky.m_info, nullptr, maxIntensity * sky.NumSpectra(), spectrometer.m_fitWindows[0].NumberOfReferences(), string1);
    }
    scan.GetDark(dark);
    if (dark.m_info.m_interlaceStep > 1)
        dark.InterpolateSpectrum();
    if (dark.m_length > 0)
    {
        dark.m_info.m_fitIntensity = (float)(dark.MaxValue(window.fitLow, window.fitHigh));
        if (dark.NumSpectra() > 0)
            dark.Div(dark.NumSpectra());
        CEvaluationLogFileHandler::FormatEvaluationResult(&dark.m_info, nullptr, maxIntensity * dark.NumSpectra(), spectrometer.m_fitWindows[0].NumberOfReferences(), string2);
    }
    scan.GetOffset(offset);
    if (offset.m_info.m_interlaceStep > 1)
        offset.InterpolateSpectrum();
    if (offset.m_length > 0)
    {
        offset.m_info.m_fitIntensity = (float)(offset.MaxValue(window.fitLow, window.fitHigh));
        offset.Div(offset.NumSpectra());
        CEvaluationLogFileHandler::FormatEvaluationResult(&offset.m_info, nullptr, maxIntensity * offset.NumSpectra(), spectrometer.m_fitWindows[0].NumberOfReferences(), string3);
    }
    scan.GetDarkCurrent(darkCurrent);
    if (darkCurrent.m_info.m_interlaceStep > 1)
        darkCurrent.InterpolateSpectrum();
    if (darkCurrent.m_length > 0)
    {
        darkCurrent.m_info.m_fitIntensity = (float)(darkCurrent.MaxValue(window.fitLow, window.fitHigh));
        darkCurrent.Div(darkCurrent.NumSpectra());
        CEvaluationLogFileHandler::FormatEvaluationResult(&darkCurrent.m_info, nullptr, maxIntensity * darkCurrent.NumSpectra(), spectrometer.m_fitWindows[0].NumberOfReferences(), string4);
    }

    string.AppendFormat("%s", (LPCSTR)string1);
    string.AppendFormat("%s", (LPCSTR)string2);
    string.AppendFormat("%s", (LPCSTR)string3);
    string.AppendFormat("%s", (LPCSTR)string4);

    // ----------------------------------------------------------------------------------------------
    // 3. ------------------- Then write the parameters for each spectrum ---------------------------
    // ----------------------------------------------------------------------------------------------
    for (unsigned long itSpectrum = 0; itSpectrum < result->GetEvaluatedNum(); ++itSpectrum)
    {
        int nSpectra = result->GetSpectrumInfo(itSpectrum).m_numSpec;

        // 3a. Pretty print the result and the spectral info into a string
        CEvaluationResult evResult;
        result->GetResult(itSpectrum, evResult);

        CEvaluationLogFileHandler::FormatEvaluationResult(&result->GetSpectrumInfo(itSpectrum), &evResult, maxIntensity * nSpectra, spectrometer.m_fitWindows[0].NumberOfReferences(), string1);

        string.AppendFormat("%s", (LPCSTR)string1);
    }
    string.AppendFormat("</spectraldata>\n");

    // 3b. Write it all to the main evaluation log file
    FILE* eF = fopen(evalLogFile, "a");
    if (eF != nullptr)
    {
        fprintf(eF, "\n");
        fprintf(eF, string);
        fclose(eF);
    }

    // 3c. Write it all to the additional evaluation log file
    FILE* f = fopen(txtFile, "w");
    if (f != nullptr)
    {
        fprintf(f, string);
        fclose(f);
    }


    // Upload the flux-log file to the FTP-Server
    UploadToNOVACServer(evalLogFile, volcanoIndex);

    return SUCCESS;
}

CSpectrometer* CEvaluationController::HandleUnIdentifiedSpectrometer(const CString& serialNumber, const CScanFileHandler& scan)
{
    CSpectrum tmpSpec;
    CString message;


    // ------------- Configure a new spectrometer ------------------
    CSpectrometer* newSpectrometer = new CSpectrometer();

    // 1. Set the serial number
    newSpectrometer->m_settings.serialNumber.Format("%s", (LPCSTR)serialNumber);

    // 2. Configure what we know about the scanning instrument
    newSpectrometer->m_scanner.observatory.Format("unknown");
    newSpectrometer->m_scanner.volcano.Format("unknown");
    scan.GetSky(tmpSpec);
    newSpectrometer->m_scanner.gps = tmpSpec.GPS();

    // 3. Configure the log file handlers
    CString dateStr, path, filePath;
    m_common.GetDateText(dateStr);
    path.Format("%sOutput\\%s", (LPCSTR)g_settings.outputDirectory, (LPCSTR)dateStr);
    filePath.Format("%s\\%s", (LPCSTR)path, (LPCSTR)serialNumber);

    newSpectrometer->m_logFileHandler.SetErrorLogFile(filePath, "ErrorLog.txt");

    // 4. Configure the fit window (here we don't know anything, just guess what could be an ok setting)
    CSpectrometer* spec0 = m_spectrometer[0];
    for (CFitWindow window : spec0->m_fitWindows)
    {
        newSpectrometer->m_fitWindows.push_back(window);
    }
    newSpectrometer->m_history = std::make_shared<CSpectrometerHistory>();

    // 5. Insert the new spectrometer
    long spectrometerNum = (long)m_spectrometer.GetSize();
    m_spectrometer.SetAtGrow(spectrometerNum, newSpectrometer);

    // 6. Tell the user what just happened...
    message.Format("RECEIVED SCAN FROM A NOT CONFIGURED SPECTROMETER: %s. SPECTRA WILL NOT BE CORRECTLY EVALUATED. PLEASE CHECK SETTINGS AND RESTART!", (LPCSTR)serialNumber);
    ShowMessage(message);
    m_logFileWriter.WriteErrorMessage(message);

    // 7. finally return a pointer to the new spectrometer
    return m_spectrometer[spectrometerNum];
}

/** Called when the thread is first started */
BOOL Evaluation::CEvaluationController::InitInstance()
{
    CWinThread::InitInstance();

    // 1. Set the language of the thread 
    ::SetThreadLocale(MAKELCID(MAKELANGID(primaryLanguage, subLanguage), SORT_DEFAULT));

    // 2. Initialize the spectrometers
    if (FAIL == InitializeSpectrometers())
    {
        TerminateProcess(GetCurrentProcess(), 0); // Kill the application
        return 0;
    }

    // 3. Initialize the output files
    InitializeOutput();

    return 1;
}

RETURN_CODE CEvaluationController::InitializeSpectrometers()
{
    long spectrometerNum = 0;
    CString message;

    /** reset */
    m_spectrometer.SetSize(g_settings.scannerNum); // make the initial assumption that there's only one spectrometer / scanner (reasonable!!!)

    try
    {

        /** fill in the content of each spectrometer */
        for (unsigned long i = 0; i < g_settings.scannerNum; ++i)
        {
            for (unsigned long j = 0; j < g_settings.scanner[i].specNum; ++j)
            {

                auto history = std::make_shared<CSpectrometerHistory>();

                for (unsigned char k = 0; k < g_settings.scanner[i].spec[j].channelNum; ++k)
                {

                    CConfigurationSetting::ScanningInstrumentSetting& scanner = g_settings.scanner[i];
                    CConfigurationSetting::SpectrometerSetting& spec = g_settings.scanner[i].spec[j];

                    CSpectrometer* curSpec = new CSpectrometer();

                    // the settings
                    curSpec->m_settings = spec;
                    curSpec->m_scanner = scanner;
                    curSpec->m_channel = k;
                    curSpec->m_history = history;

                    // the fit window
                    novac::CFitWindow& window = spec.channel[k].fitWindow;

                    ReadReferences(window);

                    // set the fit window
                    curSpec->m_fitWindows.push_back(window);

                    // Insert the new spectrometer
                    m_spectrometer.SetAtGrow(spectrometerNum, curSpec);

                    // iterate to the next spectrometer
                    ++spectrometerNum;
                }
            }
        }
    }
    catch (novac::InvalidReferenceException& ex)
    {
        std::stringstream msg;
        msg << "Cannot read all references.\n" << ex.what() << "\n";
        msg << "The program will now terminate.\nCheck the settings and restart.";
        ShowMessage(msg.str());
        MessageBox(nullptr, msg.str().c_str(), "Serious Error - Cannot read References", MB_OK);
        Sleep(500);
        return FAIL;
    }

    return SUCCESS;
}

/** Called all messages have been handled */
BOOL Evaluation::CEvaluationController::OnIdle(LONG lCount)
{
    return CWinThread::OnIdle(lCount);
}

// finds the local wind field for a specific scan and scanning instrument
RETURN_CODE CEvaluationController::GetWind(CWindField& wind, const CSpectrometer& spectrometer, const CDateTime& dt)
{

    if (g_metData.GetWindField(spectrometer.m_settings.serialNumber, dt, wind))
    {
        // if the wind field could not be found, use the standard values
        wind = g_metData.defaultWindField;
        return SUCCESS;
    }

    return SUCCESS;
}

/** Initializes the output logs that are generated. One error-log, one status-log, and one
        result-log for every spectrometer, and a similar set for the evaluationController as a whole */
RETURN_CODE CEvaluationController::InitializeOutput()
{
    long i;
    Common common;
    common.GetExePath();
    CString dateStr, path, tmpDir;
    common.GetDateText(dateStr);

    // Create the output directory
    path.Format("%sOutput\\%s", (LPCSTR)g_settings.outputDirectory, (LPCSTR)dateStr);

    // test so that the output directory can be created...
    if (CreateDirectoryStructure(path))
    {
        path.Format("%sOutput\\%s", (LPCSTR)common.m_exePath, (LPCSTR)dateStr);
        CString message;
        if (CreateDirectoryStructure(path))
        {
            message.Format("Could not create any output directory, please check settings and restart!");
            ShowMessage(message);
            MessageBox(nullptr, message, "Error with output directory", MB_OK);
            return FAIL;
        }
        else
        {
            message.Format("Could not create output directory, output will be directed to: %s", (LPCSTR)path);
            ShowMessage(message);
            MessageBox(nullptr, message, "Error with output directory", MB_OK);
            g_settings.outputDirectory.Format("%s", (LPCSTR)common.m_exePath);
        }
    }

    // the log files for the spectrometers 
    CString filePath;
    for (i = 0; i < m_spectrometer.GetSize(); ++i)
    {
        filePath.Format("%s\\%s", (LPCSTR)path, (LPCSTR)m_spectrometer[i]->SerialNumber());
        m_spectrometer[i]->m_logFileHandler.SetErrorLogFile(filePath, "ErrorLog.txt");
    }

    // store the date when the directories were last created
    m_date[0] = (unsigned short)m_common.GetYear();
    m_date[1] = (unsigned short)m_common.GetMonth();
    m_date[2] = (unsigned short)m_common.GetDay();

    return SUCCESS;
}


/** Handles the output when a new scan has arrived but we could not identify the spectrometer */
void CEvaluationController::Output_SpectrometerNotIdentified()
{
    m_logFileWriter.WriteErrorMessage(TEXT("Could not identify spectrometer, scan not evaluated."));
    ShowMessage("Unrecognized spectrometer, scan not evaluated");
}

/** Handles the output when a new scan has arrived and we have identified the spectrometer */
void CEvaluationController::Output_ArrivedScan(const CSpectrometer* spec)
{
    CString message;
    message.Format("%s %s", (LPCSTR)m_common.GetString(RECIEVED_NEW_SCAN_FROM), (LPCSTR)spec->m_settings.serialNumber);
    ShowMessage(message);
}

/** Handles the output when a fit failure has occured */
void CEvaluationController::Output_FitFailure(const CSpectrum& spec)
{
    CString message;
    message.Format("Failed to evaluate spectrum from spectrometer: %s", spec.m_info.m_device.c_str());
    m_logFileWriter.WriteErrorMessage(message);
    pView->PostMessage(WM_EVAL_FAILURE, (WPARAM) & (spec.m_info.m_device));
    ShowMessage(message);
}

void CEvaluationController::Output_FluxFailure(const std::unique_ptr<CScanResult>& result, const CSpectrometer* spec)
{
    spec->m_logFileHandler.WriteErrorMessage(TEXT("Could not calculate the flux"));

    CScanResult* copiedResult = (nullptr != result) ? new CScanResult(*result) : nullptr;

    pView->PostMessage(WM_EVAL_FAILURE, (WPARAM) & (spec->m_settings.serialNumber), (LPARAM)copiedResult);
}

/** Shows the timing information from evaluating a scan */
void CEvaluationController::Output_TimingOfScanEvaluation(int numberOfEvaluatedSpectra, const CString& serial, double timeElapsed)
{
    CString timingMessage;
    timingMessage.Format("Evaluated one scan of %d spectra from %s in %lf seconds (%lf seconds/spectrum)", numberOfEvaluatedSpectra, (LPCSTR)serial, timeElapsed, timeElapsed / (double)numberOfEvaluatedSpectra);
    ShowMessage(timingMessage);
}

void CEvaluationController::Output_EmptyScan(const CSpectrometer* spectrometer)
{
    CString message;
    message.Format("Recieved empty scan from %s", (LPCSTR)spectrometer->m_settings.serialNumber);
    ShowMessage(message);
}


RETURN_CODE CEvaluationController::GetArchivingfileName(CString& pakFile, CString& txtFile, const CString& temporaryScanFile)
{
    CSpectrumIO reader;

    CSpectrum tmpSpec;
    CString serialNumber, dateStr, timeStr, dateStr2;

    // 0. Make an initial assumption of the file-names
    int i = 0;
    while (1)
    {
        pakFile.Format("%s\\Output\\UnknownScans\\%d.pak", (LPCSTR)g_settings.outputDirectory, ++i);
        if (!IsExistingFile(pakFile))
            break;
    }
    txtFile.Format("%s\\Output\\UnknownScans\\%d.txt", (LPCSTR)g_settings.outputDirectory, i);

    // 1. Read the first spectrum in the scan
    const std::string tempScanFileName((LPCSTR)temporaryScanFile);
    if (SUCCESS != reader.ReadSpectrum(tempScanFileName, 0, tmpSpec))
        return FAIL;
    CSpectrumInfo& info = tmpSpec.m_info;
    int channel = info.m_channel;

    // 1a. If the GPS had no connection with the satelites when collecting the sky-spectrum,
    //			then try to find a spectrum in the file for which it had connection...
    i = 1;
    while (info.m_startTime.year == 2004 && info.m_startTime.month == 3 && info.m_startTime.day == 22)
    {
        if (SUCCESS != reader.ReadSpectrum(tempScanFileName, i++, tmpSpec))
            break;
        info = tmpSpec.m_info;
    }

    // 2. Get the serialNumber of the spectrometer
    serialNumber.Format("%s", info.m_device.c_str());

    // 3. Get the time and date when the scan started
    dateStr.Format("%02d%02d%02d", info.m_startTime.year % 1000, info.m_startTime.month, info.m_startTime.day);
    dateStr2.Format("%04d.%02d.%02d", info.m_startTime.year, info.m_startTime.month, info.m_startTime.day);
    timeStr.Format("%02d%02d", info.m_startTime.hour, info.m_startTime.minute);


    // 4. Write the archiving name of the spectrum file

    // 4a. Write the folder name
    pakFile.Format("%sOutput\\%s\\%s\\Scans\\", (LPCSTR)g_settings.outputDirectory, (LPCSTR)dateStr2, (LPCSTR)serialNumber);
    txtFile.Format("%s", (LPCSTR)pakFile);

    // 4b. Make sure that the folder exists
    (void)CreateDirectoryStructure(pakFile);

    // 4c. Write the name of the archiving file itself
    if (channel < 128 && channel > MAX_CHANNEL_NUM)
        channel = channel % 16;
    pakFile.AppendFormat("%s_%s_%s_%1d.pak", (LPCSTR)serialNumber, (LPCSTR)dateStr, (LPCSTR)timeStr, channel);
    txtFile.AppendFormat("%s_%s_%s_%1d.txt", (LPCSTR)serialNumber, (LPCSTR)dateStr, (LPCSTR)timeStr, channel);

    if (strlen(pakFile) > MAX_PATH)
        return FAIL;

    return SUCCESS;
}

void CEvaluationController::UpdateOutputDirectories()
{
    // Check the current date. If the current date is different from the 
    //	date when the output directories were last initialized, then
    //	initialize them again.
    if ((m_date[0] != m_common.GetYear()) || (m_date[1] != m_common.GetMonth()) || (m_date[2] != m_common.GetDay()))
    {
        InitializeOutput();
    }
}

void CEvaluationController::GetSpectrumInformation(CSpectrometer* spectrometer, const CString& fileName)
{
    CSpectrum skySpec, darkSpec;
    double lat, lon, alt;

    // 1. Open the file for reading
    CScanFileHandler scan(m_log);

    const std::string fileNameStr((LPCSTR)fileName);
    novac::LogContext context(novac::LogContext::FileName, fileNameStr);
    scan.CheckScanFile(context, fileNameStr);
    scan.GetDark(darkSpec);
    scan.GetSky(skySpec);

    if (std::abs(skySpec.Latitude()) > 0.01 && std::abs(skySpec.Longitude()) > 0.01)
    {
        double	N = spectrometer->m_gpsReadingsNum;
        lat = (spectrometer->m_scanner.gps.m_latitude * N + skySpec.GPS().m_latitude) / (N + 1);
        lon = (spectrometer->m_scanner.gps.m_longitude * N + skySpec.GPS().m_longitude) / (N + 1);
        alt = (spectrometer->m_scanner.gps.m_altitude * N + skySpec.GPS().m_altitude) / (N + 1);

        spectrometer->m_scanner.gps = CGPSData(lat, lon, alt);
        spectrometer->m_gpsReadingsNum += 1;
    }
    spectrometer->m_scanner.coneAngle = skySpec.m_info.m_coneAngle;
    spectrometer->m_scanner.tilt = skySpec.m_info.m_pitch;

    // Check the compass-value...
    if (spectrometer->m_scanner.compass != skySpec.Compass())
    {
        if (skySpec.Compass() == darkSpec.Compass())
        {
            if ((skySpec.Compass() > 0 && skySpec.Compass() < 360) || (skySpec.Compass() > 360 && skySpec.Compass() < 720)) // only update if the compass-value is reasonable.
            {
                spectrometer->m_scanner.compass = skySpec.Compass();
            }
        }
    }

    // Change the global variable according to the new information
    if (g_settings.scannerNum == 1)
    {
        g_settings.scanner[0].gps = spectrometer->m_scanner.gps;
        g_settings.scanner[0].compass = spectrometer->m_scanner.compass;
        g_settings.scanner[0].coneAngle = skySpec.m_info.m_coneAngle;
        g_settings.scanner[0].tilt = skySpec.m_info.m_pitch;
    }
    else
    {
        for (unsigned int i = 0; i < g_settings.scannerNum; ++i)
        {
            if (Equals(g_settings.scanner[i].spec[0].serialNumber, spectrometer->SerialNumber()))
            {
                g_settings.scanner[i].gps = spectrometer->m_scanner.gps;
                g_settings.scanner[i].compass = spectrometer->m_scanner.compass;
                g_settings.scanner[i].coneAngle = skySpec.m_info.m_coneAngle;
                g_settings.scanner[i].tilt = skySpec.m_info.m_pitch;
                break;
            }
        }
    }

    // Update the configuration.xml file with the new values (?)
    if (spectrometer->m_gpsReadingsNum > 0 && (fmod(spectrometer->m_gpsReadingsNum, 10.0) == 0))
    {
        pView->PostMessage(WM_REWRITE_CONFIGURATION, NULL, NULL);
    }
}

/** Sends a command to the windspeed thread to consider the given wind-speed measurement evaluation log */
RETURN_CODE CEvaluationController::MakeWindMeasurement(const CString& fileName, int volcanoIndex)
{
    CString* fn = nullptr;

    if (g_windMeas == nullptr)
        return FAIL;

    fn = new CString();
    fn->Format(fileName);

    if (g_windMeas->PostThreadMessage(WM_NEW_WIND_EVALLOG, (WPARAM)fn, (LPARAM)volcanoIndex))
    {
        return SUCCESS;
    }
    else
    {
        return FAIL;
    }
}

/** Sends a command to the geometry thread to consider the given scan-evaluation log */
RETURN_CODE CEvaluationController::MakeGeometryCalculations(const CString& fileName, int volcanoIndex)
{
    CString* fn = nullptr;

    if (g_geometry == nullptr)
        return FAIL;

    fn = new CString();
    fn->Format(fileName);

    if (g_geometry->PostThreadMessage(WM_NEW_SCAN_EVALLOG, (WPARAM)fn, volcanoIndex))
    {
        return SUCCESS;
    }
    else
    {
        return FAIL;
    }
}

void CEvaluationController::ExecuteScript_FullScan(const CString& param1, const CString& param2)
{
    CString command, filePath, directory;

    // The file to execute
    filePath.Format("%s", (LPCSTR)g_settings.externalSetting.fullScanScript);

    // The directory of the executable file
    directory.Format(filePath);
    Common::GetDirectory(directory);

    // The parameters to the script
    command.Format("%s %s", (LPCSTR)param1, (LPCSTR)param2);

    // Call the script
    int result = (int)ShellExecute(nullptr, "open", filePath, command, directory, SW_SHOW);

    // Check the return-code if there was any error...
    if (result > 32)
        return;
    switch (result)
    {
    case 0:	break;// out of memory
    case ERROR_FILE_NOT_FOUND:	MessageBox(nullptr, "Specified script-file not found", "Error in script", MB_OK);	break;	// file not found
    case ERROR_PATH_NOT_FOUND:	MessageBox(nullptr, "Specified script-file not found", "Error in script", MB_OK);	break;	// path not found
    case ERROR_BAD_FORMAT:		break;	// 
    case SE_ERR_ACCESSDENIED:break;
    case SE_ERR_ASSOCINCOMPLETE:break;
    case SE_ERR_DDEBUSY:break;
    case SE_ERR_DDEFAIL:break;
    case SE_ERR_DDETIMEOUT:break;
    case SE_ERR_DLLNOTFOUND:break;
    case SE_ERR_NOASSOC:break;
    case SE_ERR_OOM:break;
    case SE_ERR_SHARE:break;
    }

    // don't try again to access the file
    g_settings.externalSetting.fullScanScript.Format("");
}

void CEvaluationController::InitiateSpecialModeMeasurement(const CSpectrometer* spectrometer)
{
    static time_t cTimeOfLastWindMeasurement = 0;
    static time_t cTimeOfLastCompMeasurement = 0;
    time_t now;

    // Get the current time
    time(&now);

    // If we have done a special-mode measurement recently, then don't
    //  even check if we should do it again.
    if ((cTimeOfLastWindMeasurement != 0) && (difftime(now, cTimeOfLastWindMeasurement) < 3600))
        return;
    if ((cTimeOfLastCompMeasurement != 0) && (difftime(now, cTimeOfLastCompMeasurement) < 3600))
        return;

    // Check if we should do a wind-measurement
    if (WindSpeedMeasurement::CRealTimeWind::IsTimeForWindMeasurement(spectrometer))
    {
        WindSpeedMeasurement::CRealTimeWind::StartWindSpeedMeasurement(spectrometer);
        time(&cTimeOfLastWindMeasurement); // <-- remember the time of the last wind-measurement
    }
    else
    {
        // We shouldn't do a wind-measurement, check if we should do a composition measurement instead
        if (Composition::CCompositionMeasurement::IsTimeForCompositionMeasurement(spectrometer))
        {
            Composition::CCompositionMeasurement::StartCompositionMeasurement(spectrometer);
            time(&cTimeOfLastCompMeasurement); // <-- remember the time of the last composition-measurement
        }
    }

    return;
}

void CEvaluationController::UpdateInstrumentCalibration(CSpectrometer& spectrometer, const std::string& lastEvaluatedScan, const novac::CDateTime* startTimeOfScan)
{
    if (CRealTimeCalibration::IsTimeForInstrumentCalibration(spectrometer, lastEvaluatedScan, startTimeOfScan))
    {
        CRealTimeCalibration::RunInstrumentCalibration(spectrometer, lastEvaluatedScan, g_settings);
    }
}

#ifdef _MSC_VER
#pragma warning (pop)
#endif
