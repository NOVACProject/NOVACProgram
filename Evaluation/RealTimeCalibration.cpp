#include "StdAfx.h"
#include "RealTimeCalibration.h"
#include "../Common/Common.h"
#include "../Evaluation/Spectrometer.h"
#include "../Calibration/WavelengthCalibrationController.h"

#include <SpectralEvaluation/File/File.h>

#include <sstream>

extern CConfigurationSetting g_settings;    // <-- The settings

using namespace Evaluation;

std::string CurrentDateAndTime()
{
    CString timeString;
    timeString.Format(
        "%02d%02d%02d_%02d%02d",
        Common::GetYear() % 1000,
        Common::GetMonth(),
        Common::GetDay(),
        Common::GetHour(),
        Common::GetMinute());

    return std::string((LPCSTR)timeString);
}

std::string GetOutputDirectory()
{
    CString dateStr;
    Common common;
    common.GetDateText(dateStr);

    std::stringstream directory;
    directory << (LPCTSTR)g_settings.outputDirectory;
    directory << "Output\\" << (LPCTSTR)dateStr << "\\";

    return directory.str();
}

std::string GetLogFileName()
{
    std::stringstream debugFile;
    debugFile << GetOutputDirectory() << "Debug_InstrumentCalibration.txt";
    return debugFile.str();
}

std::string GetCalibrationFileName(const novac::CSpectrumInfo& spectrumInformation)
{
    CString dateAndTime;

    dateAndTime.Format(
        "%02d%02d%02d%02d%02d",
        spectrumInformation.m_startTime.year % 1000,
        spectrumInformation.m_startTime.month,
        spectrumInformation.m_startTime.day,
        spectrumInformation.m_startTime.hour,
        spectrumInformation.m_startTime.minute);

    return "Calibration_" + spectrumInformation.m_device + std::string(dateAndTime) + ".std";
}

void CRealTimeCalibration::AppendMessageToLog(const Evaluation::CSpectrometer* spectrometer, const std::string& message)
{
    const std::string logFile = GetLogFileName();

    FILE* f = fopen(logFile.c_str(), "a+");
    if (f != nullptr)
    {
        CString timeStr;
        Common::GetTimeText(timeStr);

        const CString serial = spectrometer->m_scanner.spec[0].serialNumber;

        fprintf(f, "%s\t%s\t", (LPCTSTR)timeStr, (LPCSTR)serial);
        fprintf(f, message.c_str());
        fprintf(f, "\n");

        fclose(f);
        // UploadToNOVACServer(debugFile, thisVolcano, false);
    }
}

bool CRealTimeCalibration::IsTimeForInstrumentCalibration(const Evaluation::CSpectrometer* spectrometer, const std::string& scanFile)
{
    if (spectrometer == nullptr || !novac::IsExistingFile(scanFile))
    {
        return false;
    }

    const std::string logFile = GetLogFileName();

    const auto& autoCalibrationSettings = spectrometer->m_scanner.spec[0].channel[0].autoCalibration;

    if (!autoCalibrationSettings.enable ||
        autoCalibrationSettings.solarSpectrumFile.GetLength() == 0 ||
        autoCalibrationSettings.initialCalibrationFile.GetLength() == 0)
    {
        // The device is not setup. This is expected to be the most common case, hence no debug logging is required.
        return false;
    }

    if (!IsExistingFile(autoCalibrationSettings.solarSpectrumFile))
    {
        std::stringstream message;
        message << "Cannot find provided solar spectrum file: " << autoCalibrationSettings.solarSpectrumFile;
        AppendMessageToLog(spectrometer, message.str());
        return false;
    }
    else if (!IsExistingFile(autoCalibrationSettings.initialCalibrationFile))
    {
        std::stringstream message;
        message << "Cannot find provided initial calibration file: " << autoCalibrationSettings.initialCalibrationFile;
        AppendMessageToLog(spectrometer, message.str());
        return false;
    }


    // no further objections found. 
    return true;
}

bool CRealTimeCalibration::RunInstrumentCalibration(const Evaluation::CSpectrometer* spectrometer, const std::string& scanFile)
{
    try
    {
        const auto& autoCalibrationSettings = spectrometer->m_scanner.spec[0].channel[0].autoCalibration;

        // Reuse the calibration controller, which is also used when the 
        //  user performs the instrument calibrations using the CCalibratePixelToWavelengthDialog.
        // This makes sure we get the same behavior in the dialog and here.
        WavelengthCalibrationController calibrationController;

        calibrationController.m_inputSpectrumFile = scanFile;
        calibrationController.m_solarSpectrumFile = autoCalibrationSettings.solarSpectrumFile;
        calibrationController.m_initialCalibrationFile = autoCalibrationSettings.initialCalibrationFile;
        calibrationController.m_initialLineShapeFile = autoCalibrationSettings.instrumentLineshapeFile;
        calibrationController.m_instrumentLineShapeFitOption = (WavelengthCalibrationController::InstrumentLineShapeFitOption)autoCalibrationSettings.instrumentLineShapeFitOption;
        calibrationController.m_instrumentLineShapeFitRegion = std::make_pair(autoCalibrationSettings.instrumentLineShapeFitRegion.low, autoCalibrationSettings.instrumentLineShapeFitRegion.high);

        // Does the actual calibration. Throws a std::exception if the calibration fails.
        calibrationController.RunCalibration();

        // Now we need to save the output of the calibration as well.
        const std::string directoryName = GetOutputDirectory();

        const std::string calibrationFileName = directoryName + GetCalibrationFileName(calibrationController.m_calibrationDebug.spectrumInfo);

        calibrationController.SaveResultAsStd(calibrationFileName);

        return true;
    }
    catch (std::exception& e)
    {
        AppendMessageToLog(spectrometer, e.what());
    }
    return false;
}
