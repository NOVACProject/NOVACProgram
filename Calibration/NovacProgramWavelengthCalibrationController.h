#pragma once

#include <SpectralEvaluation/DialogControllers/WavelengthCalibrationController.h>

class NovacProgramWavelengthCalibrationController : public WavelengthCalibrationController
{
public:
    NovacProgramWavelengthCalibrationController()
        : WavelengthCalibrationController() { }

    /** The full path to the spectrum to calibrate (should be a .pak file) */
    std::string m_inputSpectrumFile;

protected:

    virtual void ReadInput(novac::CSpectrum& measurement) override;

};