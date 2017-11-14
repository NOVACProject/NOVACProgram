#pragma once

#include "../EvaluatedDataStorage.h"
#include "../CommunicationDataStorage.h"

// CReportWriter

namespace FileHandler{
	// The <b>CReportWriter</b> class takes care of writing a report at the end
	//	of each day containing the results of the measurements for the past day.
	//	The writing should take place ~2 hours after the last instrument has gone
	//	to sleep for the night.

	class CReportWriter : public CWinThread
	{

	protected:
		CReportWriter();           // protected constructor used by dynamic creation
		virtual ~CReportWriter();
		DECLARE_DYNCREATE(CReportWriter)
		DECLARE_MESSAGE_MAP()

	public:
    // ----------------------------------------------------------------------
    // --------------------- PUBLIC METHODS ---------------------------------
    // ----------------------------------------------------------------------

		/** Called when the thread is starting */
		virtual BOOL InitInstance();

		/** Called when the thread is stopping */
		virtual int ExitInstance();

		/** Called to set the timer */
		void SetTimer();

		/** */
		afx_msg void OnTimer(UINT nIDEvent, LPARAM lp);

		/** Called to write the report */
		static void		WriteReport(CEvaluatedDataStorage *evalDataStorage, CCommunicationDataStorage *commDataStorage);

		// ----------------------------------------------------------------------
    // --------------------- PUBLIC VARIABLES --------------------------------
    // ----------------------------------------------------------------------

		/** Timer */
		UINT_PTR m_nTimerID;

	private:

    // ----------------------------------------------------------------------
    // --------------------- PRIVATE METHODS --------------------------------
    // ----------------------------------------------------------------------

		/** Generate the header of the Report */
		static void WriteReportHeader(FILE *f);

		/** Generate the footer of the Report */
		static void WriteReportFooter(FILE *f);

		/** Generate a histogram of the data and stores it to a graphics file */
		static int WriteDataHistogram(double *data, long nDataPoints, long nBins, const CString &filePath, const CString &fileName);

		/** Removes the bad data points from the 'timeBuffer' and 'dataBuffer'.
				The flag whether the data point is good or not is found in 'qualityBuffer' */
		static long RemoveBadFluxData(double *timeBuffer, double *dataBuffer, int *qualityBuffer, long numDataPoints);
	};
}