#pragma once

#include <string>

namespace Evaluation
{
    class CSpectrometer;
}
namespace novac
{
    class CDateTime;
}

namespace Evaluation
{
    class CRealTimeCalibration
    {
    public:
        /** Runs through the history of the CSpectrometer and the provided measurement,
            checking the settings for automatic instrument calibrations and the provided measurement
            and judges if we should perform an automatic instrument calibration now.
            @return true if an instrument calibration should be started else return false */
        static bool IsTimeForInstrumentCalibration(const Evaluation::CSpectrometer* spectrometer, const std::string& scanFile, const novac::CDateTime* startTimeOfLastScan);

        /** Performs an automatic instrument calibration for the supplied spectrometer using the provided
            .pak file for measurement data.
            If the AutomaticCalibrationSetting for the spectrometer says that the references should be created and replace existing
                references in the real-time evaluation then the references will also be updated.
            @return true if the calibration succeeded.*/
        static bool RunInstrumentCalibration(Evaluation::CSpectrometer* spec, const std::string& scanFile);

    private:

        static void AppendMessageToLog(const Evaluation::CSpectrometer* spec, const std::string& message);

    };
}