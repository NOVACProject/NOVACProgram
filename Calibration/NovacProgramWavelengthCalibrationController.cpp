#include "StdAfx.h"
#include "NovacProgramWavelengthCalibrationController.h"
#include <SpectralEvaluation/File/ScanFileHandler.h>
#include <sstream>

void NovacProgramWavelengthCalibrationController::ReadInput(novac::CSpectrum& measuredSpectrum)
{
    novac::CScanFileHandler pakFileHandler;
    if (!pakFileHandler.CheckScanFile(m_inputSpectrumFile))
    {
        std::stringstream msg;
        msg << "Cannot read the provided input spectrum file. Error:  " << pakFileHandler.m_lastError;
        throw std::invalid_argument(msg.str());
    }

    if (pakFileHandler.GetSky(measuredSpectrum))
    {
        throw std::invalid_argument("Cannot read the provided input spectrum file");
    }
    Log("Read measured spectrum: ", m_inputSpectrumFile);

    // subtract the dark-spectrum (if this exists)
    novac::CSpectrum darkSpectrum;
    if (!pakFileHandler.GetDark(darkSpectrum))
    {
        measuredSpectrum.Sub(darkSpectrum);
        Log("Read and subtracted dark spectrum");
    }

}
