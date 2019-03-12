#pragma once

#include "Common.h"
#include "../Evaluation/ScanResult.h"
#include <SpectralEvaluation/Evaluation/EvaluationResult.h>

namespace FileHandler
{

//#define MAX_N_SCANS 800

	class CEvaluationLogFileHandler
	{
		static const int ORIGINAL_ARRAY_LENGTH = 50;
	public:
		CEvaluationLogFileHandler(void);
		~CEvaluationLogFileHandler(void);

		/** The evaluation log */
		CString m_evaluationLog;

		// ------------------- PUBLIC METHODS -------------------------

		/** Reads the evaluation log */
		RETURN_CODE ReadEvaluationLog();

		/** Writes the contents of the array 'm_scan' to a new evaluation-log file */
		RETURN_CODE WriteEvaluationLog(const CString fileName);

		/** Returns true if the scan number 'scanNo' in the most recently read 
				evaluation log file is a wind speed measurement. */
		bool	IsWindSpeedMeasurement(int scanNo);
		
		/** Appends the evaluation result of one spectrum to the given string. 
				@param info - the information about the spectrum
				@param result - the evaluation result, can be NULL
				@param string - will on return be filled with the output line to be written to the evaluation-log.
				@return SUCCESS - always */
		static RETURN_CODE FormatEvaluationResult(const CSpectrumInfo *info, const Evaluation::CEvaluationResult *result, double maxIntensity, int nSpecies, CString &string);

		// ------------------- PUBLIC DATA -------------------------

		/** Information from the evaluated scans */
		CArray<Evaluation::CScanResult, Evaluation::CScanResult&> m_scan;

		/** Information of the wind field used to calculate the flux of each scan */
		CArray<CWindField, CWindField &> m_windField;

		/** How many scans that have been read from the evaluation log */
		long  m_scanNum;

		/** The species that were found in this evaluation log */
		CString m_specie[20];

		/** The number of species found in the evaluation log */
		long    m_specieNum;

		/** The currently selected specie */
		long    m_curSpecie;;

		/** The additional spectrum information of one spectrum. */
		CSpectrumInfo m_specInfo;
	protected:

		typedef struct LogColumns{
			int column[MAX_N_REFERENCES];
			int columnError[MAX_N_REFERENCES];
			int shift[MAX_N_REFERENCES];
			int shiftError[MAX_N_REFERENCES];
			int squeeze[MAX_N_REFERENCES];
			int squeezeError[MAX_N_REFERENCES];
			int intensity;
			int fitIntensity;
			int peakSaturation;
			int fitSaturation;
			int offset;
			int delta;
			int chiSquare;
			int nSpec;
			int expTime;
			int position;
			int position2;
			int nSpecies;
			int starttime;
			int stoptime;
			int name;
		}LogColumns;

		/** Data structure to remember what column corresponds to which value in the evaluation log */
		LogColumns m_col;

		/** The result from the evaluation of one spectrum. */
		Evaluation::CEvaluationResult m_evResult;

		/** Reads the header line for the scan information and retrieves which 
			column represents which value. */
		void ParseScanHeader(const char szLine[8192]);

		/** Reads and parses the XML-shaped 'scanInfo' header before the scan */
		void ParseScanInformation(CSpectrumInfo &scanInfo, double &flux, FILE *f);

		/** Reads and parses the XML-shaped 'fluxInfo' header before the scan */
		void ParseFluxInformation(CWindField &windField, double &flux, FILE *f);

		/** Resets the information about which column data is stored in */
		void ResetColumns();

		/** Resets the old scan information */
		void ResetScanInformation();

		/** Makes a quick scan through the evaluation-log 
			to count the number of scans in it */
		long CountScansInFile();

		/** Makes a quick scan through the evaluation-log 
			to get the start-times of each scan.
			@return the number of scans in the file */
		long GetScanStartTimes(CArray<CDateTime*, CDateTime*> &array);

		/** Sorts the scans in order of collection */
		void SortScans();

		/** Returns true if the scans are already ordered */
		bool IsSorted();

		/** Sorts the CDateTime-objects in the given array.
				Algorithm based on bubble sort (~O(NlogN)) */
		void SortScanStartTimes(CArray<CDateTime*, CDateTime*> &array, CArray<unsigned int, unsigned int&> &sortOrder);

		/** Sorts the CScanResult-objects in the given array.
				Algorithm based on MergeSort (~O(NlogN)) */
		static void SortScans(CArray<Evaluation::CScanResult, Evaluation::CScanResult&> &array, bool ascending = true);

		/** Sorts the CScanResult-objects in the given array.
				Algorithm based on BubbleSort (~O(N2))
				Quite efficient for small arrays since the elements does not have to be copied
					and thus uses very little memory */
		static void BubbleSortScans(CArray<Evaluation::CScanResult, Evaluation::CScanResult&> &array, bool ascending = true);

		/** Merges the two arrays in a sorted way and stores the
				result in the output-array 'result' */
		static void MergeArrays(CArray<Evaluation::CScanResult, Evaluation::CScanResult&> &array1, CArray<Evaluation::CScanResult, Evaluation::CScanResult&> &array2, CArray<Evaluation::CScanResult, Evaluation::CScanResult&> &result, bool ascending = true);
	};
}