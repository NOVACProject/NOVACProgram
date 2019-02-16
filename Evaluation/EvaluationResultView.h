#pragma once

#include <vector>
#include "../SpectralEvaluation/Spectra/Spectrum.h"

namespace Evaluation
{
    /** Small struct used to pass the result of the fit to the view.
        The size of the spectra stored equals the size of the fit-window used. */
    struct CEvaluationResultView
    {
        // The scaled references
        std::vector<CSpectrum> scaledReference;

        CSpectrum measuredSpectrum;
        CSpectrum residual;
        CSpectrum polynomial;
    };

}