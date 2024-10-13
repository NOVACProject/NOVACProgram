#pragma once

#include "ScanResult.h"
#include <memory>
#include <mutex>

#include <SpectralEvaluation/Evaluation/ScanEvaluationBase.h>
#include <SpectralEvaluation/Evaluation/FitParameter.h>
#include <SpectralEvaluation/Configuration/SkySettings.h>
#include "../Common/Common.h"
#include "../Configuration/Configuration.h"

namespace novac
{
class CSpectrum;
class CEvaluationBase;
class CScanFileHandler;
}

namespace Evaluation
{
/**
    An object of the <b>CScanEvaluation</b>-class handles the evaluation of one
    scan.
*/
class CScanEvaluation : public novac::ScanEvaluationBase
{

public:
    CScanEvaluation(novac::ILogger& log);
    virtual ~CScanEvaluation();

    /** If the contents of m_pause is true then the current thread will
        be suspended between each iteration. Useful for reevaluation as it
        lets the user have a look at each fitted spectrum before continuing. */
    int* m_pause = nullptr;

    /** The state of the evaluation. If m_sleeping is true then the
        thread is sleeping and needs to be woken up. */
    bool* m_sleeping = nullptr;

    /** if pView != NULL then after the evaluation of a spectrum, a 'WM_EVAL_SUCCESS'
        message will be sent to pView. */
    CWnd* pView = nullptr;

    /** Called to evaluate one scan.
            @return the number of spectra evaluated. */
    long EvaluateScan(const CString& scanfile, const novac::CFitWindow& window, bool* fRun = NULL, const Configuration::CDarkSettings* darkSettings = NULL);

    /** Setting the option for how to get the sky spectrum. */
    void SetOption_Sky(const Configuration::CSkySettings& settings);

    /** Setting the option for which spectra to ignore */
    void SetOption_Ignore(IgnoreOption lowerLimit, IgnoreOption upperLimit);

    /** Setting the option for wheather the spectra are averaged or not. */
    void SetOption_AveragedSpectra(bool averaged);

    /** @return a copy of the scan result */
    std::unique_ptr<CScanResult> GetResult();

    /** @return the number of spectra in the last scan evaluated */
    int NumberOfSpectraInLastResult();

private:

    /** The evaluation results from the last scan evaluated */
    std::shared_ptr<CScanResult> m_result;

    /** A mutex to protect the scan result from bein updated/deleted/altered from two threads simultaneously */
    std::mutex m_resultMutex;

    // ----------------------- PRIVATE METHODS ---------------------------

    /** This returns the sky spectrum that is to be used in the fitting. */
    RETURN_CODE GetSky(novac::CScanFileHandler* scan, novac::CSpectrum& sky);

    /** This returns the dark spectrum that is to be used in the fitting.
        @param scan - the scan-file handler from which to get the dark spectrum
        @param spec - the spectrum for which the dark spectrum should be retrieved
        @param dark - will on return be filled with the dark spectrum
        @param darkSettings - the settings for how to get the dark spectrum from this spectrometer */
    RETURN_CODE GetDark(novac::CScanFileHandler* scan, const novac::CSpectrum& spec, novac::CSpectrum& dark, const Configuration::CDarkSettings* darkSettings = NULL);

    /** checks the spectrum to the settings and returns 'true' if the spectrum should not be evaluated */
    bool Ignore(const novac::CSpectrum& spec, const novac::CFitWindow window);

    /** This function updates the 'm_residual' and 'm_fitResult' spectra
        and sends the 'WM_EVAL_SUCCESS' message to the pView-window. */
    void ShowResult(const novac::CSpectrum& spec, const novac::CEvaluationBase* eval, long curSpecIndex, long specNum);

    /** Updates the m_result in a thread safe manner (locking the m_resultMutex) */
    void UpdateResult(std::shared_ptr<CScanResult> newResult);

    /** Finds the optimum shift and squeeze for an evaluated scan
                by looking at the spectrum with the highest absorption of the evaluated specie
                and evaluate it with shift and squeeze free
         @return the fit-result for the evaluated specie. */
    novac::CEvaluationResult FindOptimumShiftAndSqueeze(const novac::CEvaluationBase* originalEvaluation, novac::CScanFileHandler* scan, CScanResult* result);

    /** Finds the optimum shift and squeeze for an scan by evaluating
        with a solar-reference spectrum and studying the shift of the
        Fraunhofer-lines.
        @param eval - the evaluator to use for the evaluation. On successful determination
            of the shift and squeeze then the shift and squeeze of the reference-files
            in the CEvaluationBase-objects CFitWindow will be fixed to the optimum value found
        @param scan - a handle to the spectrum file.
        @return a new CFitWindow to use with the references shift and squeeze fixed to the found optimal value.
        @return nullptr if an optimal shift and squeeze could not be found. */
    novac::CFitWindow* FindOptimumShiftAndSqueeze_Fraunhofer(const novac::CEvaluationBase* originalEvaluation, novac::CScanFileHandler* scan);

    // ------------------------ THE PARAMETERS FOR THE EVALUATION ------------------
    /** This is the options for the sky spectrum */
    Configuration::CSkySettings m_skySettings;

    /** The options for which spectra to ignore */
    IgnoreOption m_ignore_Lower;
    IgnoreOption m_ignore_Upper;

    /** This is the fit region */
    long m_fitLow;
    long m_fitHigh;

    /** True if the spectra are averaged, not summed */
    bool m_averagedSpectra;

    /** Remember the index of the spectrum with the highest absorption, to be able to
        adjust the shift and squeeze with it later */
    int m_indexOfMostAbsorbingSpectrum;

    /** how many spectra there are in the current scan-file (for showing the progress) */
    long m_prog_SpecNum;

    /** which spectrum we are on in the current scan-file (for showing the progress) */
    long m_prog_SpecCur;
};
}