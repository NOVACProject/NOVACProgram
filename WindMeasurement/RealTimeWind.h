#pragma once

// forward declarations
namespace Evaluation
{
class CSpectrometer;
}
class CWindField;

namespace WindSpeedMeasurement
{
class CRealTimeWind
{
public:
    /** Runs through the history of the CSpectrometer and checks the settings
            for automatic wind-speed measurements and judges if we should perform
            an automatic wind-speed measurement now.
            @return true if a wind-speed measurement should be started else return false */
    static bool	IsTimeForWindMeasurement(const Evaluation::CSpectrometer* spectrometer);

    /** Starts an automatic wind-speed measurement for the supplied spectrometer */
    static void	StartWindSpeedMeasurement(const Evaluation::CSpectrometer* spec);

};
}