#pragma once

#include <string>
#include <vector>

namespace novac
{
    class InstrumentCalibration;
    class CSpectrum;
}

class WavelengthCalibrationController
{
public:
    WavelengthCalibrationController();
    ~WavelengthCalibrationController();

    enum class InstrumentLineShapeFitOption
    {
        None = 0,
        SuperGaussian = 1
    };

    /// <summary>
    /// The full path to the spectrum to calibrate (should be a .pak file)
    /// </summary>
    std::string m_inputSpectrumFile;

    /// <summary>
    /// The full path to the high resolved solar spectrum
    /// </summary>
    std::string m_solarSpectrumFile;

    /// <summary>
    /// The full path to a file which contains an initial calibration
    /// This can be either a full calibration file, as saved from another novac program,
    /// or just the pixel-to-wavelength mapping file.
    /// </summary>
    std::string m_initialCalibrationFile;

    /// <summary>
    /// The full path to a file which contains an initial measured line shape.
    /// This may be left out if m_initialCalibrationFile does contain an instrument line shape.
    /// </summary>
    std::string m_initialLineShapeFile;

    /// <summary>
    /// The option for if an instrument line shape should be fitted as well during
    /// the retrieval of the pixel-to-wavelength calibration.
    /// </summary>
    InstrumentLineShapeFitOption m_instrumentLineShapeFitOption;

    /// <summary>
    /// The wavelength region in which the instrument line shape should be fitted (in nm).
    /// </summary>
    std::pair<double, double> m_instrumentLineShapeFitRegion;

    /// <summary>
    /// One additional cross section to be included when performing the instrument line shape fitting.
    /// Typically this is the high resolution cross section for ozone.
    /// </summary>
    std::string m_crossSectionsForInstrumentLineShapeFitting;

    /// <summary>
    /// This is the initial calibration, used as a starting point for the calibration routine.
    /// Must be set before calling RunCalibration().
    /// </summary>
    std::unique_ptr<novac::InstrumentCalibration> m_initialCalibration;

    /// <summary>
    /// This is the result of the wavelength calibration.
    /// Can only be set after calling RunCalibration().
    /// </summary>
    std::unique_ptr<novac::InstrumentCalibration> m_resultingCalibration;

    /// <summary>
    /// User friendly description of the fitted parameters for the instrument line shape function.
    /// </summary>
    std::vector<std::pair<std::string, std::string>> m_instrumentLineShapeParameterDescriptions;

    /// <summary>
    /// If the calibration fails, for some reason, then this message should be set to indicate why.
    /// </summary>
    std::string m_errorMessage;

    /// <summary>
    /// An elementary log, will contain debugging information from running the calibration.
    /// </summary>
    std::vector<std::string> m_log;

    struct WavelengthCalibrationDebugState
    {
        WavelengthCalibrationDebugState(size_t estimatedSize)
        {
            inlierCorrespondencePixels.reserve(estimatedSize);
            inlierCorrespondenceWavelengths.reserve(estimatedSize);
            outlierCorrespondencePixels.reserve(estimatedSize);
            outlierCorrespondenceWavelengths.reserve(estimatedSize);
        }

        std::vector<double> initialPixelToWavelengthMapping;

        std::vector<double> inlierCorrespondencePixels;
        std::vector<double> inlierCorrespondenceWavelengths;
        std::vector<double> inlierCorrespondenceMeasuredIntensity; //< the intensity of the measured spectrum
        std::vector<double> inlierCorrespondenceFraunhoferIntensity; //< the intensity of the Fraunhofer spectrum

        std::vector<double> outlierCorrespondencePixels;
        std::vector<double> outlierCorrespondenceWavelengths;

        std::vector<double> measuredSpectrum;

        // All the keypoints from the measured spectrum
        std::vector<double> measuredSpectrumKeypointPixels;
        std::vector<double> measuredSpectrumKeypointIntensities;

        // The inlier keypoints from the measured spectrum
        std::vector<double> measuredSpectrumInlierKeypointPixels;
        std::vector<double> measuredSpectrumInlierKeypointIntensities;

        std::vector<double> fraunhoferSpectrum;

        // All the keypoints from the Fraunhofer spectrum
        std::vector<double> fraunhoferSpectrumKeypointWavelength;
        std::vector<double> fraunhoferSpectrumKeypointIntensities;

        // The inlier keypoints from the Fraunhofer spectrum
        std::vector<double> fraunhoferSpectrumInlierKeypointWavelength;
        std::vector<double> fraunhoferSpectrumInlierKeypointIntensities;
    };

    WavelengthCalibrationDebugState m_calibrationDebug;

    /// <summary>
    /// Performs the actual wavelength calibration
    /// </summary>
    void RunCalibration();

    /// <summary>
    /// Saves the resulting pixel-to-wavelength and instrument-line-shape information as a .std file.
    /// </summary>
    void SaveResultAsStd(const std::string& filename);

    /// <summary>
    /// Saves the resulting pixel-to-wavelength mapping information as a .clb file.
    /// </summary>
    void SaveResultAsClb(const std::string& filename);

    /// <summary>
    /// Saves the resulting instrument line shape information as a .slf file.
    /// </summary>
    void SaveResultAsSlf(const std::string& filename);

    /// <summary>
    /// Empties the previous result (if any).
    /// </summary>
    void ClearResult();

private:

    /// <summary>
    /// Guesses for an instrment line shape from the measured spectrum. Useful if no measured instrument line shape exists
    /// </summary>
    void CreateGuessForInstrumentLineShape(const std::string& solarSpectrumFile, const novac::CSpectrum& measuredSpectrum, novac::InstrumentCalibration& calibration);

    /// <summary>
    /// Checks the provided (dark corrected) spectrum and makes sure that it is good enough for the calibration to succeed.
    /// Throws an std::invalid_argument exception if the spectrum isn't good enough.
    /// </summary>
    void CheckSpectrumQuality(const novac::CSpectrum& spectrum) const;

    void Log(const std::string& message);
    void Log(const std::string& message, double value);
    void Log(const std::string& message, const std::string& messagePart2);

};