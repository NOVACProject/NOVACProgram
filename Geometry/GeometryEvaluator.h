#pragma once

#include "../CombinerThread.h"

#include "../Common/Common.h"

namespace Geometry{

	/** <b>CGeometryEvaluator</b> takes care of the real-time evaluation of 
			incoming scans with the purpose of calculating plume-height and/or
			wind-directions. */

	class CGeometryEvaluator : public CCombinerThread
	{
	protected:
		/** Default constructor */
		CGeometryEvaluator();

		/** Default destructor */
		virtual ~CGeometryEvaluator();

		DECLARE_DYNCREATE(CGeometryEvaluator)
		DECLARE_MESSAGE_MAP()

	public:
		/** Called when this thread is about to start */
		virtual BOOL InitInstance();

		/** Called when the thread is to be stopped */
		afx_msg void OnQuit(WPARAM wp, LPARAM lp);

		/** Called when the message queue is empty */
		virtual BOOL OnIdle(LONG lCount);

		// ----------------------------------------------------------------------
		// ---------------------- PUBLIC DATA -----------------------------------
		// ----------------------------------------------------------------------

		
		
		// ----------------------------------------------------------------------
		// --------------------- PUBLIC METHODS ---------------------------------
		// ----------------------------------------------------------------------

		/** Called when the CEvaluationController has evaluated a normal scan
				from one connected spectrometer.
				@param wp is a pointer to a CString-object telling the file-name of the 
					evaluation log containing the scan.
				@param lp - not used. 
		*/
		afx_msg void OnEvaluatedScan(WPARAM wp, LPARAM lp);

	protected:
		// ----------------------------------------------------------------------
		// -------------------- PROTECTED DATA ----------------------------------
		// ----------------------------------------------------------------------

		/** The maximum time difference between two scans (in seconds) if
				they are to be used in any geometry calculations. */
		static const int MAX_TIME_DIFFERENCE = 900;

		// ----------------------------------------------------------------------
		// ------------------- PROTECTED METHODS --------------------------------
		// ----------------------------------------------------------------------

		/** Searches the list of evaluation logs and tries to find a log-file
				which matches the given evaluation-log. 
				@return - the number of matching files found, this can be no more than MAX_MATCHING_FILES */
		int	FindMatchingEvalLog(const CString &evalLog, CString match[MAX_MATCHING_FILES], int volcanoIndex);

		/** Calculate the plume-height using the two scans found in the 
				given evaluation-files. */
		RETURN_CODE CalculateGeometry(const CString &evalLog1, const CString &evalLog2, int volcanoIndex);

		/** Takes the filename of an evaluation log and extracts the 
				Serial-number of the spectrometer, the date the scan was performed
				and the start-time of the scan from the filename. */
		bool GetInfoFromFileName(const CString fileName, CDateTime &start, CString &serial);

		///** Writes the results of the geometry calculation to a file.
		//		@param calc - the windspeed calculator which has performed the correlation calculations
		//		@param evallog - one of the evaluation-log files used in the correlation calculations 		*/
		//void WriteGeometryLog(const CWindSpeedCalculator &calc, const CString &evalLog, const Evaluation::CScanResult &scan);
	};
}