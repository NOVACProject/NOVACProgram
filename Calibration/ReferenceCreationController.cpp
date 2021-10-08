#include "StdAfx.h"
#include "ReferenceCreationController.h"
#include <SpectralEvaluation/StringUtils.h>
#include <SpectralEvaluation/Evaluation/BasicMath.h>
#include <SpectralEvaluation/Evaluation/CrossSectionData.h>
#include <SpectralEvaluation/Spectra/Spectrum.h>
#include <SpectralEvaluation/File/File.h>
#include <SpectralEvaluation/Calibration/ReferenceSpectrumConvolution.h>
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

void ReferenceCreationController::ConvolveReference()
{
    auto conversion = (m_convertToAir) ? novac::WavelengthConversion::VacuumToAir : novac::WavelengthConversion::None;

    // Read the calibration
    novac::CCrossSectionData instrumentLineShape;
    std::vector<double> pixelToWavelengthMapping;
    if (EqualsIgnoringCase(novac::GetFileExtension(this->m_calibrationFile), ".std"))
    {
        novac::CSpectrum instrumentLineShapeSpectrum;
        if (!novac::ReadInstrumentCalibration(this->m_calibrationFile, instrumentLineShapeSpectrum, pixelToWavelengthMapping))
        {
            throw std::invalid_argument("Failed to read the instrument calibration file");
        }

        instrumentLineShape = novac::CCrossSectionData(instrumentLineShapeSpectrum);
    }
    else
    {
        pixelToWavelengthMapping = novac::GetPixelToWavelengthMappingFromFile(this->m_calibrationFile);
        if (pixelToWavelengthMapping.size() == 0)
        {
            throw std::invalid_argument("Failed to read the wavelength calibration file");
        }

        if (!novac::ReadCrossSectionFile(this->m_instrumentLineshapeFile, instrumentLineShape))
        {
            throw std::invalid_argument("Failed to read the instrument line shape file");
        }
    }

    // Red the reference
    novac::CCrossSectionData highResReference;
    if (!novac::ReadCrossSectionFile(this->m_highResolutionCrossSection, highResReference))
    {
        throw std::exception("Failed to read the reference file");
    }

    // Do the convolution
    std::vector<double> convolutionResult;
    if (!novac::ConvolveReference(pixelToWavelengthMapping, instrumentLineShape, highResReference, convolutionResult, conversion))
    {
        throw std::exception("Failed to convolve the references");
    }

    // Combine the results into the final output cross section data 
    this->m_resultingCrossSection = std::make_unique<novac::CCrossSectionData>();
    this->m_resultingCrossSection->m_crossSection = convolutionResult;
    this->m_resultingCrossSection->m_waveLength = pixelToWavelengthMapping;

    if (this->m_highPassFilter)
    {
        PrepareConvolvedReferenceForHighPassFiltering(m_resultingCrossSection, highResReference.m_waveLength);

        const int length = (int)this->m_resultingCrossSection->m_crossSection.size();

        CBasicMath math;
        math.Mul(this->m_resultingCrossSection->m_crossSection.data(), length, -2.5e15);
        math.Delog(this->m_resultingCrossSection->m_crossSection.data(), length);
        math.HighPassBinomial(this->m_resultingCrossSection->m_crossSection.data(), length, 500);
        math.Log(this->m_resultingCrossSection->m_crossSection.data(), length);
    }

}