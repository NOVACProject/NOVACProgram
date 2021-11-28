#include "StdAfx.h"
#include "RealTimeCalibration.h"
#include "../Common/Common.h"
#include "../Evaluation/Spectrometer.h"
#include "../Calibration/WavelengthCalibrationController.h"
#include "../Calibration/ReferenceCreationController.h"

#include <SpectralEvaluation/File/File.h>
#include <SpectralEvaluation/Calibration/StandardCrossSectionSetup.h>

#include <sstream>

extern CConfigurationSetting g_settings;    // <-- The settings

using namespace Evaluation;

// ------------- Free functions used to help out with the calibration -------------
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

std::string FormatDateAndTimeOfSpectrum(const novac::CSpectrumInfo& spectrumInformation)
{
    CString dateAndTime;

    dateAndTime.Format(
        "%02d%02d%02d_%02d%02d",
        spectrumInformation.m_startTime.year % 1000,
        spectrumInformation.m_startTime.month,
        spectrumInformation.m_startTime.day,
        spectrumInformation.m_startTime.hour,
        spectrumInformation.m_startTime.minute);

    return std::string(dateAndTime);
}

std::string GetCalibrationFileName(const novac::CSpectrumInfo& spectrumInformation)
{
    return "Calibration_" + spectrumInformation.m_device + "_" + FormatDateAndTimeOfSpectrum(spectrumInformation) + ".std";
}

void RunCalibration(WavelengthCalibrationController& calibrationController, const std::string& scanFile, const CConfigurationSetting::AutomaticCalibrationSetting& autoCalibrationSettings)
{
    calibrationController.m_inputSpectrumFile = scanFile;
    calibrationController.m_solarSpectrumFile = autoCalibrationSettings.solarSpectrumFile;
    calibrationController.m_initialCalibrationFile = autoCalibrationSettings.initialCalibrationFile;
    calibrationController.m_initialLineShapeFile = autoCalibrationSettings.instrumentLineshapeFile;
    calibrationController.m_instrumentLineShapeFitOption = (WavelengthCalibrationController::InstrumentLineShapeFitOption)autoCalibrationSettings.instrumentLineShapeFitOption;
    calibrationController.m_instrumentLineShapeFitRegion = std::make_pair(autoCalibrationSettings.instrumentLineShapeFitRegion.low, autoCalibrationSettings.instrumentLineShapeFitRegion.high);

    // Does the actual calibration. Throws a std::exception if the calibration fails.
    calibrationController.RunCalibration();
}

std::vector<novac::CReferenceFile> CreateStandardReferences(const novac::CSpectrumInfo& spectrumInformation, const std::unique_ptr<novac::InstrumentCalibration>& calibration, const CConfigurationSetting::AutomaticCalibrationSetting& autoCalibrationSettings, const std::string& directoryName)
{
    Common common;
    common.GetExePath();
    std::string exePath = common.m_exePath;
    novac::StandardCrossSectionSetup standardCrossSections{ exePath };

    ReferenceCreationController referenceController;
    std::vector<novac::CReferenceFile> referencesCreated;

    for (size_t ii = 0; ii < standardCrossSections.NumberOfReferences(); ++ii)
    {
        referenceController.m_highPassFilter = autoCalibrationSettings.filterReferences;
        referenceController.m_convertToAir = standardCrossSections.IsReferenceInVacuum(ii);
        referenceController.m_highResolutionCrossSection = standardCrossSections.ReferenceFileName(ii);
        referenceController.ConvolveReference(*calibration);

        // Save the result
        const std::string filteringStr = (autoCalibrationSettings.filterReferences) ?
            "_HP500_PPMM" :
            "";
        const std::string dstFileName =
            directoryName +
            spectrumInformation.m_device + 
            "_" +
            standardCrossSections.ReferenceSpecieName(ii) +
            filteringStr +
            "_" +
            FormatDateAndTimeOfSpectrum(spectrumInformation) +
            ".txt";
        novac::SaveCrossSectionFile(dstFileName, *(referenceController.m_resultingCrossSection));

        novac::CReferenceFile newReference;
        newReference.m_specieName = standardCrossSections.ReferenceSpecieName(ii);
        newReference.m_path = dstFileName;
        newReference.m_isFiltered = autoCalibrationSettings.filterReferences;
        referencesCreated.push_back(newReference);
    }

    return referencesCreated;
}

void ReplaceReferences(std::vector<novac::CReferenceFile>& newReferences, Evaluation::CSpectrometer* spectrometer)
{
    // First make sure that all the cross sections could be read in before attempting to replace anything
    for (size_t idx = 0; idx < newReferences.size(); ++idx)
    {
        if (newReferences[idx].ReadCrossSectionDataFromFile())
        {
            std::stringstream message;
            message << "Failed to read cross section data from file: " << newReferences[idx].m_path << std::endl;
            throw std::invalid_argument(message.str());
        }

        if (newReferences[idx].m_data == nullptr ||
            newReferences[idx].m_data->m_crossSection.size() == 0)
        {
            std::stringstream message;
            message << "The cross section was read from file but contained no data. File: " << newReferences[idx].m_path << std::endl;
            throw std::invalid_argument(message.str());
        }
    }

    // Now replace the references
    auto& fitWindow = spectrometer->m_scanner.spec[0].channel[0].fitWindow;
    fitWindow.nRef = static_cast<int>(newReferences.size());
    for (size_t idx = 0; idx < newReferences.size(); ++idx)
    {
        fitWindow.ref[idx] = newReferences[idx];
    }

}

// ----------- CRealTimeCalibration class -----------
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

bool CRealTimeCalibration::RunInstrumentCalibration(Evaluation::CSpectrometer* spectrometer, const std::string& scanFile)
{
    try
    {
        const auto& autoCalibrationSettings = spectrometer->m_scanner.spec[0].channel[0].autoCalibration;

        // Use the WavelengthCalibrationController, which is also used when the 
        //  user performs the instrument calibrations using the CCalibratePixelToWavelengthDialog.
        // This makes sure we get the same behavior in the dialog and here.
        WavelengthCalibrationController calibrationController;
        RunCalibration(calibrationController, scanFile, autoCalibrationSettings);

        // Save new instrument calibration.
        const std::string directoryName = GetOutputDirectory();
        const std::string calibrationFileName = directoryName + GetCalibrationFileName(calibrationController.m_calibrationDebug.spectrumInfo);
        calibrationController.SaveResultAsStd(calibrationFileName);

        // Create the standard references.
        const auto finalCalibration = calibrationController.GetFinalCalibration();
        auto referencesCreated = CreateStandardReferences(calibrationController.m_calibrationDebug.spectrumInfo, finalCalibration, autoCalibrationSettings, directoryName);

        // All references have successfully been created, replace the references used by the evaluation with the new references.
        if (autoCalibrationSettings.generateReferences && referencesCreated.size() > 0)
        {
            ReplaceReferences(referencesCreated, spectrometer);
        }

        return true;
    }
    catch (std::exception& e)
    {
        AppendMessageToLog(spectrometer, e.what());
    }
    return false;
}
