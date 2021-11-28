#pragma once

#include <memory>
#include <string>

#include <SpectralEvaluation/Calibration/InstrumentCalibration.h>

namespace novac
{
    class CCrossSectionData;
}

class ReferenceCreationController
{
public:
    ReferenceCreationController();

    /** Input: the full file path to the high resolved cross section file to use. */
    std::string m_highResolutionCrossSection;

    /** Setting: should the result be high-pass filtered (default is true as this is the default for novac references) */
    bool m_highPassFilter = true;

    /** Setting: set to true if the high resolution reference is measured in vacuum. */
    bool m_convertToAir = false;

    /** Output: The resulting convolved cross section. */
    std::unique_ptr<novac::CCrossSectionData> m_resultingCrossSection;

    /* Reads the provided calibration from file. The result can then later be passed into ConvolveReference. */
    static novac::InstrumentCalibration ReadCalibration(std::string calibrationFile, std::string instrumentLineshapeFile);

    /** Performs the convolution of the reference using the provided calibration.
         This will update m_resultingCrossSection*/
    void ConvolveReference(const novac::InstrumentCalibration& calibration);
};

