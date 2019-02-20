#pragma once

#include "../resource.h"
#include <memory>

#include "Spectrometer.h"
#include "ScanResult.h"

#include "../Common/Common.h"
#include "../SpectralEvaluation/File/ScanFileHandler.h"
#include "../Common/LogFileWriter.h"
#include "../Common/Spectra/PakFileHandler.h"

extern CFormView* pView;
extern UINT primaryLanguage;
extern UINT subLanguage;

namespace Evaluation
{
	/** <b>CEvaluationController</b> is the master controller of the evaluation 
		of the spectra. The class is run as a separate thread and
		recives messages on incoming spectra that should be evaluated
		and dispatches the spectra to the correct evaluator. 
		*/

	class CEvaluationController : CWinThread
	{
	public:
		/** Default constructor */
		CEvaluationController(void);

		/** Default destructor */
		~CEvaluationController(void);

		DECLARE_DYNCREATE(CEvaluationController);
		DECLARE_MESSAGE_MAP()

		/** Called when the thread is starting */
		virtual BOOL InitInstance();

		/** Called when the message queue is empty */
		virtual BOOL OnIdle(LONG lCount);

		/** Called when the thread is to be stopped */
		afx_msg void OnQuit(WPARAM wp, LPARAM lp);

		// ----------------------------------------------------------------------
		// ---------------------- PUBLIC DATA -----------------------------------
		// ----------------------------------------------------------------------

		/** A scan-result, for sharing evaluated data with the rest of the
			program. This is updated after every evaluation of a full scan. */
		std::unique_ptr<CScanResult> m_lastResult;

		// ----------------------------------------------------------------------
		// --------------------- PUBLIC METHODS ---------------------------------
		// ----------------------------------------------------------------------

		/** Handling the appearance of a new pak-file one scan which should be evaluated
			@param wp is a pointer to a CString object telling the filename of the pak-file. 
			@param lp - unused. */
		afx_msg void OnArrivedSpectra(WPARAM wp, LPARAM lp);

		/** Used to test the evaluation. 
			@param wp is a pointer to a CString object telling the filename of the pak-file. 
			@param lp - unused. */
		afx_msg void OnTestEvaluation(WPARAM wp, LPARAM lp);

		/** Evaluates a scan. TODO: THIS FUNCTION ASSUMES THAT THE FIRST SPECTRUM
			IN THE SPECTRUM FILE IS THE SKY-REFERENCE, THE SECOND IS A DARK MEASUREMENT
			AND THE FOLLOWING SPECTRA ARE THE ACTUAL MEASUREMENT MAKING UP THE SCAN.
			@param fileName - The name of the pak-file in which the spectra are stored.
			All files in this pak-file will be evaluated as if they are one scan. 
			@param volcanoIndex - the index into the 'g_volcanoes'-list that identifies
			the volcano that the supplied scan comes from.
			@return SUCCESS if the evaluation is sucessful */
		RETURN_CODE EvaluateScan(const CString &fileName, int volcanoIndex);

		/** Evaluates a single spectrum in a scan-file. THIS FUNCTION ASSUMES
			THAT THE FIRST SPECTRUM IN THE FILE IS THE SKY SPECTRUM, THE SECOND IS THE 
			DARK, AND THE INDEX IS A ZERO-BASED INDEX OF MEASURED SPECTRA (NOT INCLUDING SKY AND DARK)
			@param fileName - The name of the pak-file in which the spectra are stored. 
			@param spectrumIndex - The spectrum to evaluate. 
			@return SUCCESS if the evaluation is sucessful */
		RETURN_CODE EvaluateSpectrum(const CString &fileName, int spectrumIndex, CScanResult *result);
		
		/** Checks if this is a good time to initiate a special mode measurement
			for the given spectrometer. 
			Special mode measurements are;
				1 wind-speed measurements
				2 composition measurements */
		void InitiateSpecialModeMeasurement(CSpectrometer *spectrometer, CWindField &windField);
	private:
		// ----------------------------------------------------------------------
		// ---------------------- PRIVATE DATA ----------------------------------
		// ----------------------------------------------------------------------

		/** An array of spectrometers that we know of and that we can evaluate 
			spectra from. This object contains information on which scanning instrument 
			the spectrometer is connected to and where it is situated and what 
			reference files, and settings, that should be used to evaluate spectra 
			from each spectrometer. */
		CArray<CSpectrometer *, CSpectrometer *>m_spectrometer;

		/** A log-file writer to handle the output of the program. 
			This is not used for any output which can be connected to a single 
			spectrometer, that is handled by the logFileHandler in each 
			'm_spectrometer' item. This is used only for describing the 
			state of the program. */
		FileHandler::CLogFileWriter m_logFileWriter;

		/** The specie for which the flux should be calculated. E.g. "SO2" */
		std::string m_fluxSpecie;

		/** Defining which of the result logs is the evaluation log */
		const static int EVALUATION_LOG = 0;

		/** Defining which of the result logs is the flux log */
		const static int FLUX_LOG = 1;

		/** A common-object, for doing common tasks. */
		Common m_common;

		/** Determines if the evaluation should be done in real-time or if we should
			wait until a full scan has arrived. if m_realTime is true then the spectra will
			be evaluated as they arrive, if m_realTime is false then they will not be evaluated
			until a full scan has arrived. THIS IS NOT FULLY IMPLEMENTED YET. */
		bool m_realTime;

		/** The date when the output directories were changed last 
			(initialized when 'InitializeOutput' is called). 
			This is compared to the current date every time 'EvaluateScan' is 
			called, and if the dates are different, the output directories are initialized again. 
			(m_date[0] is the year, m_date[1] is the month (1-12), and m_date[2] is the day (1-31) */
		unsigned short m_date[3];

		// ----------------------------------------------------------------------
		// --------------------- PRIVATE METHODS --------------------------------
		// ----------------------------------------------------------------------

		/** Indentifies the scanning instrument from which this scan was generated. 
			@param scan a reference to a scan that should be identified. 
			@return a pointer to the spectrometer. @return NULL if no spectrometer found */
		CSpectrometer *IdentifySpectrometer(const FileHandler::CScanFileHandler *scan);

		/** Calculates the flux from the provided scan using the information given in 'scan' 
			@param scan - The scan, including evaluated results of the scan, this will be updated with information on the flux. 
			@param spectrometer - reference to a spectrometer giving information on geometry. 
			@param volcanoIndex - the index into the global array 'g_volcanoes' which defines which volcano this flux originates from
			@return SUCCESS if operation completed sucessfully.
			if operation is not successful - evalResult will be undefined. */
		RETURN_CODE CalculateFlux(CScanResult *result, const CSpectrometer *spectrometer, int volcanoIndex, CWindField &windField);

		/** Appends the evaluated flux to the appropriate log file.
			@param scan - the scan itself, also containing information about the evaluation and the flux.
			@param spectrometer - information about the scanning instrument that generated the scan. 
			@param volcanoIndex - identifies the volcano that this flux comes from
			@return SUCCESS if operation completed sucessfully. */
		RETURN_CODE WriteFluxResult(const CScanResult *result, const CSpectrometer &spectrometer, const CWindField &windField, int volcanoIndex);

		/** Appends the evaluation result to the appropriate log file.
			@param result - a CScanResult holding information about the result
			@param scan - the scan itself, also containing information about the evaluation and the flux.
			@param scanningInstrument - information about the scanning instrument that generated the scan. 
			@return SUCCESS if operation completed sucessfully. */
		RETURN_CODE WriteEvaluationResult(const CScanResult *result, const FileHandler::CScanFileHandler *scan, const CSpectrometer &spectrometer, CWindField &windField);

		/** Handles the connection of a un-identified spectrometer to the network.
			@param serialNumber - the serial number of the newly connected spectrometer. 
			@return a pointer to the newly connected spectrometer. 
			@return NULL upon failure. */
		CSpectrometer *HandleUnIdentifiedSpectrometer(const CString &serialNumber, const FileHandler::CScanFileHandler *scan);

		/** Initializes the output logs that are generated. One error-log, one status-log, and one
			result-log for every spectrometer, and a similar set for the evaluationController as a whole.
			@return SUCCESS if everything works. */
		RETURN_CODE InitializeOutput();

		/** Initialize the spectrometers according to the settings i 'g_settings'.
			This is needed when the program starts and every time the configuration file is reloaded.
			@return SUCCESS if everything works. */
		RETURN_CODE InitializeSpectrometers();

		/** Tries to retrieve the local wind field when the scan was collected. 
			@param wind - a wind field structure that will be filled with the wind field data.
			@param scan - the scan for which we want to know the wind field.
			@param spectrometer - the spectrometer which collected the scan.
			@param dt - the time and day for which we want to get the wind-field
			@return SUCESS if all is ok. @return FAIL if any error occurs.*/
		RETURN_CODE GetWind(CWindField &wind, const CSpectrometer &spectrometer, const CDateTime &dt);

		/** Gets the filename under which the scan-file should be stored.
			@return SUCCESS if a filename is found. */
		RETURN_CODE GetArchivingfileName(CString &pakFile, CString &txtFile, const CString &temporaryScanFile);

		/** Sends a command to the WindEvaluator thread to use the supplied
			evaluation-log file for correlation. */
		RETURN_CODE MakeWindMeasurement(const CString &fileName, int volcanoIndex);

		/** Sends a command to the GeometryEvaluator thread to use the supplied
			evaluation-log file for geometry calculations. */
		RETURN_CODE MakeGeometryCalculations(const CString &fileName, int volcanoIndex);

		/** Makes calculations of the geometrical setup using the given
			Heidelberg (V-II) instrument and the last evaluation result */
		RETURN_CODE MakeGeometryCalculations_Heidelberg(CSpectrometer *spectrometer);

		/** Retrieves information from the spectrum-file and saves it */
		void GetSpectrumInformation(CSpectrometer *spectrometer, const CString &fileName);

		/** Executes a shell-command with the defined parameters */
		void ExecuteScript_FullScan(const CString &param1, const CString &param2);

		/** Checks today's date and if necessary updates the output directories */
		void UpdateOutputDirectories();

		/** Shows information that we have recieved a new scan */
		void Output_ArrivedScan(const CSpectrometer *spec);

		/** Shows information about errors in identifying the spectrometer */
		void Output_SpectrometerNotIdentified();

		/** Shows information about a fit failure */
		void Output_FitFailure(const CSpectrum &spec);

		/** Shows the information about a failure in the flux calculation */
		void Output_FluxFailure(const CScanResult *result, const CSpectrometer *spec);

		/** Shows the timing information from evaluating a scan */
		void Output_TimingOfScanEvaluation(int spectrumNum, const CString &serial, double timeElapsed);

		/** Shows information about an arrival of a scan without any spectra in it */
		void Output_EmptyScan(const CSpectrometer *spectrometer); 
};

}