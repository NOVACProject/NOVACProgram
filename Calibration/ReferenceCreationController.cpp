#include "StdAfx.h"
#include "ReferenceCreationController.h"
#include <SpectralEvaluation/StringUtils.h>
#include <SpectralEvaluation/Evaluation/BasicMath.h>
#include <SpectralEvaluation/Evaluation/CrossSectionData.h>
#include <SpectralEvaluation/Spectra/Spectrum.h>
#include <SpectralEvaluation/File/File.h>
#include <SpectralEvaluation/Calibration/InstrumentCalibration.h>
#include <SpectralEvaluation/Calibration/ReferenceSpectrumConvolution.h>
#include <SpectralEvaluation/Calibration/WavelengthCalibration.h>
#include <algorithm>

#undef min
#undef max

namespace novac
{
    std::vector<double> GetPixelToWavelengthMappingFromFile(const std::string& clbFile);
}

// Question here: should maybe these free function be moved to SpectralEvaluation ?

void DistributeValue(std::vector<double>& data, double value, size_t firstIndex, size_t lastIndex)
{
    lastIndex = std::min(lastIndex, data.size());

    for (size_t ii = firstIndex; ii < lastIndex; ++ii)
    {
        data[ii] = value;
    }
}

void PrepareConvolvedReferenceForHighPassFiltering(const std::unique_ptr<novac::CCrossSectionData>& reference, const std::vector<double>& validWavelengthRange)
{
    // If we are to filter the spectrum, then make sure that the first/last valid value in the high-res cross section 
    //  is copied to the end of the spectrum otherwise we'll have an abrupt step and hence a large spike in the filtered result.
    if (reference->m_waveLength.front() < validWavelengthRange.front())
    {
        size_t indexOfFirstValidValue = 0;
        while (reference->m_waveLength[indexOfFirstValidValue] < validWavelengthRange.front())
        {
            ++indexOfFirstValidValue;
        }

        DistributeValue(reference->m_crossSection, reference->m_crossSection[indexOfFirstValidValue + 1], 0, indexOfFirstValidValue + 1);
    }

    if (reference->m_waveLength.back() > validWavelengthRange.back())
    {
        size_t indexOfLastValidValue = reference->m_waveLength.size() - 1;
        while (reference->m_waveLength[indexOfLastValidValue] > validWavelengthRange.back() && indexOfLastValidValue > 0)
        {
            --indexOfLastValidValue;
        }

        DistributeValue(reference->m_crossSection, reference->m_crossSection[indexOfLastValidValue - 1], indexOfLastValidValue, reference->m_waveLength.size());
    }
}

ReferenceCreationController::ReferenceCreationController()
{
}

novac::InstrumentCalibration ReferenceCreationController::ReadCalibration(std::string calibrationFile, std::string instrumentLineshapeFile)
{
    // Read the calibration
    novac::InstrumentCalibration initialCalibration;
    if (EqualsIgnoringCase(novac::GetFileExtension(calibrationFile), ".std"))
    {
        // This is a file which may contain either just the wavelength calibration _or_ both a wavelength calibration and an instrument line shape.
        if (!novac::ReadInstrumentCalibration(calibrationFile, initialCalibration))
        {
            throw std::invalid_argument("Failed to read the instrument calibration file");
        }
        if (initialCalibration.instrumentLineShapeGrid.size() == 0)
        {
            throw std::invalid_argument("Provided instrument calibration file does not contain an instrument line shape.");
        }
    }
    else
    {
        initialCalibration.pixelToWavelengthMapping = novac::GetPixelToWavelengthMappingFromFile(calibrationFile);
        if (!novac::IsPossiblePixelToWavelengthCalibration(initialCalibration.pixelToWavelengthMapping))
        {
            throw std::invalid_argument("Failed to read the wavelength calibration file");
        }

        novac::CCrossSectionData instrumentLineShape;
        if (!novac::ReadCrossSectionFile(instrumentLineshapeFile, instrumentLineShape))
        {
            throw std::invalid_argument("Failed to read the instrument line shape file");
        }

        initialCalibration.instrumentLineShapeGrid = instrumentLineShape.m_waveLength;
        initialCalibration.instrumentLineShape = instrumentLineShape.m_crossSection;
    }

    return initialCalibration;
}

void ReferenceCreationController::ConvolveReference(const novac::InstrumentCalibration& initialCalibration)
{
    auto conversion = (m_convertToAir) ? novac::WavelengthConversion::VacuumToAir : novac::WavelengthConversion::None;

    // Extract the line shape (needed for the convolution below)
    novac::CCrossSectionData instrumentLineShape;
    instrumentLineShape.m_waveLength = initialCalibration.instrumentLineShapeGrid;
    instrumentLineShape.m_crossSection = initialCalibration.instrumentLineShape;

    // Read the reference
    novac::CCrossSectionData highResReference;
    if (!novac::ReadCrossSectionFile(m_highResolutionCrossSection, highResReference))
    {
        throw std::exception("Failed to read the reference file");
    }

    // Do the convolution
    std::vector<double> convolutionResult;
    novac::ConvolveReference(
        initialCalibration.pixelToWavelengthMapping,
        instrumentLineShape,
        highResReference,
        convolutionResult,
        conversion);

    // Combine the results into the final output cross section data 
    m_resultingCrossSection = std::make_unique<novac::CCrossSectionData>();
    m_resultingCrossSection->m_crossSection = convolutionResult;
    m_resultingCrossSection->m_waveLength = initialCalibration.pixelToWavelengthMapping;

    if (m_highPassFilter)
    {
        PrepareConvolvedReferenceForHighPassFiltering(m_resultingCrossSection, highResReference.m_waveLength);

        const int length = (int)m_resultingCrossSection->m_crossSection.size();

        CBasicMath math;
        math.Mul(m_resultingCrossSection->m_crossSection.data(), length, -2.5e15);
        math.Delog(m_resultingCrossSection->m_crossSection.data(), length);
        math.HighPassBinomial(m_resultingCrossSection->m_crossSection.data(), length, 500);
        math.Log(m_resultingCrossSection->m_crossSection.data(), length);
    }

}