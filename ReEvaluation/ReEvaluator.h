#pragma once

#include <SpectralEvaluation/Configuration/SkySettings.h>
#include <SpectralEvaluation/Evaluation/EvaluationBase.h>
#include "../Evaluation/ScanResult.h"
#include <SpectralEvaluation/Evaluation/ReferenceFile.h>
#include <SpectralEvaluation/Spectra/Spectrum.h>
#include <SpectralEvaluation/File/ScanFileHandler.h>
#include "../Configuration/Configuration.h"

#define MAX_N_SCANFILES 64


namespace ReEvaluation
{
    class CReEvaluator
    {
    public:
        CReEvaluator();

        /** The maximum number of fit windows that can be defined */
        static const long MAX_N_WINDOWS = 10;

        /** The minimum credible intensity, spectra with intensity below this
            value will be treated as dark. */
        static const long MINIMUM_CREDIBLE_INTENSITY = 600;

        /**  this is true if the reevaluation is running, else false */
        bool  fRun;

        /** If this is true then the reevaluator will sleep beteween each spectrum evaluation */
        int   m_pause;

        /** True if the thread is currently sleeping */
        bool  m_sleeping;

        /** The pak-files to reevaluate */
        CArray <CString, CString&> m_scanFile;

        /** The number of pak-files opened */
        long    m_scanFileNum;

        /** The index of the pak-file we're currently working on */
        long    m_curScanFile;

        /** The fit windows */
        Evaluation::CFitWindow  m_window[MAX_N_WINDOWS];

        /** How many windows are defined? */
        long        m_windowNum;

        /** Which is the current fit window? */
        long        m_curWindow;

        /** True if the spectra that we're treating are averaged, not summed */
        bool				m_averagedSpectra;

        /** a string that is updated with information about progres in the calculations.
            every time the string is changed a message is sent to 'pView' */
        CString			m_statusMsg;

        /** The fit results */
        Evaluation::CEvaluationResult m_results;

        /** when doing lengthy calculations the value of this double varies from 0 (in the beginning) to 1 (in the end)
        every now and then a WM_PROGRESS message is sent to the window pView if pView != NULL
        when the process is finished the window is sent the message WM_DONE */
        double  m_progress;
        CWnd    *pView;

        /** The directory in which the output is currently directed */
        CString   m_outputDir;

        /** The file name of the current evaluation log, stored in the m_outputDir directory */
        CString   m_evalLog[MAX_N_WINDOWS];

        /** The options for which spectra to ignore */
        Evaluation::IgnoreOption  m_ignore_Lower;
        Evaluation::IgnoreOption  m_ignore_Upper;

        /** The options for how to get the sky spectrum */
        Configuration::CSkySettings m_skySettings;

        /** The settings for how to handle the dark-measurements */
        Configuration::CDarkSettings m_darkSettings;

    public:

        // ------------------------- METHODS ------------------------------

        /** Halts the current operation */
        bool Stop();

        /** This function takes care of the actual evaluation */
        bool DoEvaluation();

        /** Checks the settings before the evaluation */
        bool MakeInitialSanityCheck();

        ///** Reads the references defined in the FitWindow into the evaluator provided. s*/
        //bool ReadReferences(Evaluation::CEvaluation *evaluator, Evaluation::CFitWindow &window);

        /** Creates the output directory for the current scan file */
        bool CreateOutputDirectory();

        /** Creates, and writes the header in the evaluation log for
            the current scan file and the current fit window. */
        bool  WriteEvaluationLogHeader();

        /** Appends the evaluation result to the evaluation log */
        bool AppendResultToEvaluationLog(const Evaluation::CScanResult *result, const FileHandler::CScanFileHandler *scan);

        /** Sorts the scans in the array 'm_scanFile' in alphabetical order */
        void SortScans();

    private:

        /** Prepares for evaluation */
        bool PrepareEvaluation();

    };
}