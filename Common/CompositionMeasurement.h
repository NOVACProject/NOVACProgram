#pragma once

// forward declarations
namespace Evaluation
{
    class CSpectrometer;
}

namespace Composition
{

    class CCompositionMeasurement
    {
    public:

        /** Runs through the history of the CSpectrometer and judges
                if we should perform a composition measurement now.
                @return true if a composition measurement should be started else return false */
        static bool	IsTimeForCompositionMeasurement(const Evaluation::CSpectrometer* spectrometer);

        /** Starts an automatic composition measurement for the supplied spectrometer */
        static void	StartCompositionMeasurement(const Evaluation::CSpectrometer* spec);
    };

}