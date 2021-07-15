#include "StdAfx.h"

#undef max
#undef min
#include <filesystem>
#include "WavelengthCalibrationController.h"
#include <SpectralEvaluation/Spectra/Spectrum.h>
#include <SpectralEvaluation/File/File.h>
#include <SpectralEvaluation/File/STDFile.h>
#include <SpectralEvaluation/Calibration/WavelengthCalibration.h>
#include <SpectralEvaluation/Calibration/InstrumentLineShapeEstimation.h>
#include <SpectralEvaluation/Calibration/FraunhoferSpectrumGeneration.h>
#include <SpectralEvaluation/Interpolation.h>

// TODO: It should be possible to remove this header...
#include <SpectralEvaluation/Calibration/WavelengthCalibrationByRansac.h>

WavelengthCalibrationController::WavelengthCalibrationController()
    : m_calibrationDebug(0U)
{
}

WavelengthCalibrationController::~WavelengthCalibrationController()
{
    if (m_measuredInstrumentLineShapeSpectrum != nullptr)
    {
        delete m_measuredInstrumentLineShapeSpectrum;
    }
}

/// <summary>
/// Reads and parses m_initialLineShapeFile and saves the result to the provided settings
/// </summary>
void ReadInstrumentLineShape(const std::string& initialLineShapeFile, novac::WavelengthCalibrationSettings& settings)
{
    novac::CCrossSectionData measuredInstrumentLineShape;
    if (!novac::ReadCrossSectionFile(initialLineShapeFile, measuredInstrumentLineShape))
    {
        throw std::invalid_argument("Cannot read the provided instrument lineshape file");
    }
    settings.initialInstrumentLineShape = measuredInstrumentLineShape;
    settings.estimateInstrumentLineShape = novac::InstrumentLineshapeEstimationOption::None;
}

/// <summary>
/// Guesses for an instrment line shape from the measured spectrum. Useful if no measured instrument line shape exists
/// </summary>
void CreateGuessForInstrumentLineShape(const std::string& solarSpectrumFile, const novac::CSpectrum& measuredSpectrum, novac::WavelengthCalibrationSettings& settings)
{
    // The user has not supplied an instrument-line-shape, create a guess for one.
    std::vector<std::pair<std::string, double>> noCrossSections;
    novac::FraunhoferSpectrumGeneration fraunhoferSpectrumGen{ solarSpectrumFile, noCrossSections };

    novac::InstrumentLineShapeEstimation ilsEstimation{ settings.initialPixelToWavelengthMapping };

    double resultInstrumentFwhm;
    ilsEstimation.EstimateInstrumentLineShape(fraunhoferSpectrumGen, measuredSpectrum, settings.initialInstrumentLineShape, resultInstrumentFwhm);

    // no need to estimate further (or is it?)
    settings.estimateInstrumentLineShape = novac::InstrumentLineshapeEstimationOption::None;
}

void WavelengthCalibrationController::RunCalibration()
{
    novac::WavelengthCalibrationSettings settings;
    settings.highResSolarAtlas = this->m_solarSpectrumFile;

    novac::CSpectrum measuredSpectrum;
    if (!novac::ReadSpectrum(this->m_inputSpectrumFile, measuredSpectrum))
    {
        throw std::invalid_argument("Cannot read the provided input spectrum file");
    }

    // subtract the dark-spectrum (if this has been provided)
    novac::CSpectrum darkSpectrum;
    if (novac::ReadSpectrum(this->m_darkSpectrumFile, darkSpectrum))
    {
        measuredSpectrum.Sub(darkSpectrum);
    }

    if (novac::GetFileExtension(this->m_initialCalibrationFile).compare(".std") == 0)
    {
        if (m_measuredInstrumentLineShapeSpectrum != nullptr)
        {
            delete m_measuredInstrumentLineShapeSpectrum;
        }

        this->m_measuredInstrumentLineShapeSpectrum = new novac::CSpectrum();
        if (!novac::ReadInstrumentCalibration(this->m_initialCalibrationFile, *m_measuredInstrumentLineShapeSpectrum, settings.initialPixelToWavelengthMapping))
        {
            throw std::invalid_argument("Failed to read the instrument calibration file");
        }

        // Convert CSpectrum to CCrossSectionData
        novac::CCrossSectionData measuredInstrumentLineShape(*m_measuredInstrumentLineShapeSpectrum);

        settings.initialInstrumentLineShape = measuredInstrumentLineShape;
        settings.estimateInstrumentLineShape = novac::InstrumentLineshapeEstimationOption::None;
    }
    else
    {
        settings.initialPixelToWavelengthMapping = novac::GetPixelToWavelengthMappingFromFile(this->m_initialCalibrationFile);

        if (this->m_initialLineShapeFile.size() > 0)
        {
            ReadInstrumentLineShape(m_initialLineShapeFile, settings);
        }
        else
        {
            CreateGuessForInstrumentLineShape(this->m_solarSpectrumFile, measuredSpectrum, settings);
        }
    }

    // So far no cross sections provided...

    novac::WavelengthCalibrationSetup setup{ settings };
    auto result = setup.DoWavelengthCalibration(measuredSpectrum);

    // Copy out the result
    this->m_resultingPixelToWavelengthMapping = result.pixelToWavelengthMapping;
    this->m_resultingPixelToWavelengthMappingCoefficients = result.pixelToWavelengthMappingCoefficients;

    // Also copy out some debug information, which makes it possible for the user to inspect the calibration
    {
        const auto& calibrationDebug = setup.GetLastCalibrationSetup();
        this->m_calibrationDebug = WavelengthCalibrationDebugState(calibrationDebug.allCorrespondences.size());
        this->m_calibrationDebug.initialPixelToWavelengthMapping = settings.initialPixelToWavelengthMapping;

        for (size_t correspondenceIdx = 0; correspondenceIdx < calibrationDebug.allCorrespondences.size(); ++correspondenceIdx)
        {
            const auto& corr = calibrationDebug.allCorrespondences[correspondenceIdx]; // easier syntax.

            if (calibrationDebug.correspondenceIsInlier[correspondenceIdx])
            {
                this->m_calibrationDebug.inlierCorrespondencePixels.push_back(corr.measuredValue);
                this->m_calibrationDebug.inlierCorrespondenceWavelengths.push_back(corr.theoreticalValue);
                this->m_calibrationDebug.inlierCorrespondenceMeasuredIntensity.push_back(calibrationDebug.measuredKeypoints[corr.measuredIdx].intensity);
                this->m_calibrationDebug.inlierCorrespondenceFraunhoferIntensity.push_back(calibrationDebug.fraunhoferKeypoints[corr.theoreticalIdx].intensity);

                this->m_calibrationDebug.measuredSpectrumInlierKeypointPixels.push_back(calibrationDebug.measuredKeypoints[corr.measuredIdx].pixel);
                this->m_calibrationDebug.measuredSpectrumInlierKeypointIntensities.push_back(calibrationDebug.measuredKeypoints[corr.measuredIdx].intensity);

                this->m_calibrationDebug.fraunhoferSpectrumInlierKeypointWavelength.push_back(calibrationDebug.fraunhoferKeypoints[corr.theoreticalIdx].wavelength);
                this->m_calibrationDebug.fraunhoferSpectrumInlierKeypointIntensities.push_back(calibrationDebug.fraunhoferKeypoints[corr.theoreticalIdx].intensity);
            }
            else
            {
                this->m_calibrationDebug.outlierCorrespondencePixels.push_back(corr.measuredValue);
                this->m_calibrationDebug.outlierCorrespondenceWavelengths.push_back(corr.theoreticalValue);
            }
        }

        this->m_calibrationDebug.measuredSpectrum = std::vector<double>(calibrationDebug.measuredSpectrum->m_data, calibrationDebug.measuredSpectrum->m_data + calibrationDebug.measuredSpectrum->m_length);
        this->m_calibrationDebug.fraunhoferSpectrum = std::vector<double>(calibrationDebug.fraunhoferSpectrum->m_data, calibrationDebug.fraunhoferSpectrum->m_data + calibrationDebug.fraunhoferSpectrum->m_length);

        for (const auto& pt : calibrationDebug.measuredKeypoints)
        {
            this->m_calibrationDebug.measuredSpectrumKeypointPixels.push_back(pt.pixel);
            this->m_calibrationDebug.measuredSpectrumKeypointIntensities.push_back(pt.intensity);
        }

        for (const auto& pt : calibrationDebug.fraunhoferKeypoints)
        {
            this->m_calibrationDebug.fraunhoferSpectrumKeypointWavelength.push_back(pt.wavelength);
            this->m_calibrationDebug.fraunhoferSpectrumKeypointIntensities.push_back(pt.intensity);
        }
    }
}


std::pair<std::string, std::string> FormatProperty(const char* name, double value);

void WavelengthCalibrationController::SaveResultAsStd(const std::string& filename)
{
    // Get the peak itself
    size_t startIdx = 0; // TODO: Set this value to something more reasonable

    // Additional information about the spectrum
    novac::CSTDFile::ExtendedFormatInformation extendedFileInfo;
    extendedFileInfo.MinChannel = static_cast<int>(startIdx);
    extendedFileInfo.MaxChannel = static_cast<int>(startIdx + m_measuredInstrumentLineShapeSpectrum->m_length);
    extendedFileInfo.MathLow = extendedFileInfo.MinChannel;
    extendedFileInfo.MathHigh = extendedFileInfo.MaxChannel;
    novac::LinearInterpolation(m_resultingPixelToWavelengthMapping, 0.5 * m_measuredInstrumentLineShapeSpectrum->m_length, extendedFileInfo.Marker);
    extendedFileInfo.calibrationPolynomial = this->m_resultingPixelToWavelengthMappingCoefficients;

    // Create the spectrum to save from the instrument line-shape + the 
    std::unique_ptr<novac::CSpectrum> spectrumToSave;
    {
        // Extend the measured spectrum line shape into a full spectrum
        std::vector<double> extendedPeak(m_resultingPixelToWavelengthMapping.size(), 0.0);
        std::copy(m_measuredInstrumentLineShapeSpectrum->m_data, m_measuredInstrumentLineShapeSpectrum->m_data + m_measuredInstrumentLineShapeSpectrum->m_length, extendedPeak.begin() + startIdx);

        spectrumToSave = std::make_unique<novac::CSpectrum>(m_resultingPixelToWavelengthMapping, extendedPeak);
    }

    // spectrumToSave->m_info = this->m_inputspectrumInformation;

    novac::CSTDFile::WriteSpectrum(*spectrumToSave, filename, extendedFileInfo);
}

void WavelengthCalibrationController::SaveResultAsClb(const std::string& filename)
{
    novac::SaveDataToFile(filename, this->m_resultingPixelToWavelengthMapping);
}

void WavelengthCalibrationController::SaveResultAsSlf(const std::string& filename)
{
    novac::SaveCrossSectionFile(filename, *m_measuredInstrumentLineShapeSpectrum);
}
