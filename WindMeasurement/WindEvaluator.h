#pragma once

#include "../Evaluation/ScanResult.h"

#include "../CombinerThread.h"

// The wind-speed calculators
#include "WindSpeedCalculator.h"


namespace WindSpeedMeasurement{

	/** <b>CWindEvaluator</b> takes care of the real-time evaluation of 
			incoming wind-speed measurements performed with the dual-beam 
			technique. */
	class CWindEvaluator : public CCombinerThread
	{
	protected:
		/** Default constructor */
		CWindEvaluator();

		/** Default destructor */
		virtual ~CWindEvaluator();

		DECLARE_DYNCREATE(CWindEvaluator)
		DECLARE_MESSAGE_MAP()
	public:

		/** Called when this thread is about to start */
		virtual BOOL InitInstance();

    /** Called when the message queue is empty */
    virtual BOOL OnIdle(LONG lCount);

    /** Called when the thread is to be stopped */
    afx_msg void OnQuit(WPARAM wp, LPARAM lp);

    // ----------------------------------------------------------------------
    // ---------------------- PUBLIC DATA -----------------------------------
    // ----------------------------------------------------------------------

		/** The settings for how to perform the correlation calculations */
		CWindSpeedMeasSettings	m_settings;
		

    // ----------------------------------------------------------------------
    // --------------------- PUBLIC METHODS ---------------------------------
    // ----------------------------------------------------------------------

		/** Called when the CEvaluationController has evaluated a measured
				time-series from a wind-speed measurement from one spectrometer channel
				@param wp is a pointer to a CString-object telling the file-name of the 
					evaluation log containing the time-series.
				@param lp - not used. 
		*/
		afx_msg void OnEvaluatedWindMeasurement(WPARAM wp, LPARAM lp);

	protected:
    // ----------------------------------------------------------------------
    // -------------------- PROTECTED DATA ----------------------------------
    // ----------------------------------------------------------------------

    // ----------------------------------------------------------------------
    // ------------------- PROTECTED METHODS --------------------------------
    // ----------------------------------------------------------------------

		/** Searches the list of evaluation logs and tries to find a log-file
				which matches the given evaluation-log. 
				@return - the number of matching files found, this can be no more than MAX_MATCHING_FILES */
		int	FindMatchingEvalLog(const CString &evalLog, CString match[MAX_MATCHING_FILES], int volcanoIndex);

		/** Calculate the correlation between the two time-series found in the 
				given evaluation-files. */
		RETURN_CODE CalculateCorrelation(const CString &evalLog1, const CString &evalLog2, int volcanoIndex);

		/** Calculate the correlation between the two time-series found in the 
				given evaluation-file. */
		RETURN_CODE CalculateCorrelation_Heidelberg(const CString &evalLog, int volcanoIndex);

		/** Writes the results of the windspeed measurement to a file.
				@param calc - the windspeed calculator which has performed the correlation calculations
				@param evallog - one of the evaluation-log files used in the correlation calculations 		*/
		void WriteWindMeasurementLog(const CWindSpeedCalculator &calc, const CString &evalLog, const Evaluation::CScanResult &scan, int volcanoIndex);

		/** Tells the rest of the program about the result of the correlation - calculation.
				Either if the calculation was successful or not... */
		void PostWindMeasurementResult(double avgDelay, double avgCorr, double distance, const novac::CDateTime startTime, const novac::CDateTime stopTime, const CString &serial);
	};
}