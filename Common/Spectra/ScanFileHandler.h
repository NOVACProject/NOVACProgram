#pragma once

#include <afxtempl.h>

#include "../Common.h"
#include "../../SpectralEvaluation/Spectra/DateTime.h"
#include "SpectrumIO.h"

#include "../../SpectralEvaluation/Evaluation/EvaluationResult.h"


#include "../WindField.h"

namespace FileHandler
{
	/** <b>CScanFileHandler</b> is a class to read in information from the scan-files (all the spectra 
	    from one scan are supposted to be packed together in one file in Manne's 'pak'-format.
	     Each instance of 'CScanFileHandler' is capable of reading data from one .pak-file.
	*/
	class CScanFileHandler
	{
	public:
		/** Default constructor */
		CScanFileHandler(void);

		/** Constructs a new CScanFileHandler object and redirects the output to the supplied 
			log-file writer */
		CScanFileHandler(FileHandler::CLogFileWriter &log);

		/** Default destructor */
		~CScanFileHandler(void);

		// ----------------------------------------------------------------------
		// ---------------------- PUBLIC DATA -----------------------------------
		// ----------------------------------------------------------------------

		/** The serial number of the spectrometer which has collected the spectra 
			in this scan */
		CString m_device;

		/** The channel of the spectrometer which was used for collecting this scan.
			(if a SD2000 with multiple channels is used, one spectrometer should be 
			configured for each channel). */
		unsigned char m_channel;

		/** The time (UMT) when the measurement started */
        CDateTime m_startTime;

		/** The time (UMT) when the measurement was finished */
        CDateTime m_stopTime;

		/** If any error occurs in the reading of the file, this int is set to
			any of the errors defined int 'SpectrumIO.h. */
		int m_lastError;

		// ----------------------------------------------------------------------
		// --------------------- PUBLIC METHODS ---------------------------------
		// ----------------------------------------------------------------------

		/** Checks the scan saved in the given filename
			If any file-error occurs the parameter 'm_lastError' will be set.
			@param fileName - the name of the file in which the spectra of the scan are saved.
			@return SUCCESS on success. @return FAIL if any error occurs */
		RETURN_CODE CheckScanFile(const CString *fileName);

		/** Gets the next spectrum in the scan. 
			If any file-error occurs the parameter 'm_lastError' will be set.
			@param spec - will on successful return be filled with the newly read spectrum.
			@return the number of spectra read (0 if failure and 1 on success).*/
		int GetNextSpectrum(CSpectrum &spec);

		/** Returns the desired spectrum in the scan. 
			If any file-error occurs the parameter 'm_lastError' will be set.
			@param spec - will on successful return be filled with the newly read spectrum.
			@param specNo - The zero-based index into the scan-file.
			@return the number of spectra read.*/
		int GetSpectrum(CSpectrum &spec, long specNo);

		/** Gets the dark spectrum of the scan */
		int GetDark(CSpectrum &spec) const;

		/** Gets the sky spectrum of the scan */
		int GetSky(CSpectrum &spec) const;

		/** Gets the offset spectrum of the scan - if any */
		int GetOffset(CSpectrum &spec) const;

		/** Gets the dark-current spectrum of the scan - if any */
		int GetDarkCurrent(CSpectrum &spec) const;

		/** Returns the interlace steps for the spectra in this scan-file.
				 @return the interlace steps for the spectra in this scan.
				 @return -1 if the function 'CheckScanFile' has not been called */
		int	GetInterlaceSteps() const;
		
		/** Returns the length of the spectra in this scan-file.
				 @return the spectrum-length for the spectra in this scan.
				 @return -1 if the function 'CheckScanFile' has not been called */
		int	GetSpectrumLength() const;

		/** Returns the start-channel for the spectra in this scan-file.
				This is the pixel on the detector for which corresponds to the first
					datapoint in the spectra (normally 0). 
				 @return the start-channel for the spectra in this scan.
				 @return -1 if the function 'CheckScanFile' has not been called */
		int	GetStartChannel() const;

		/** Retrieves GPS-information from the spectrum files */
		const CGPSData &GetGPS() const;

		/** Retrieves compass-information from the spectrum files */
		double GetCompass() const;

		/** Retrieves the name of the file that this object is working on */
		const CString &GetFileName() const {return m_fileName; }

		/** Resets the m_specReadSoFarNum to start reading from the first spectrum again */
		void  ResetCounter();

		/** Retrieves the total number of spectra in the .pak-file (including sky and dark) */
		int GetSpectrumNumInFile();

	private:
		// ----------------------------------------------------------------------
		// ---------------------- PRIVATE DATA ----------------------------------
		// ----------------------------------------------------------------------

		/** The dark spectrum */
		CSpectrum m_dark;

		/** The sky spectrum */
		CSpectrum m_sky;

		/** The offset spectrum - if any */
		CSpectrum m_offset;

		/** The dark-current spectrum - if any */
		CSpectrum m_darkCurrent;

		/** Remember how many spectra we have read from the scan */
		unsigned int m_specReadSoFarNum;

		/** True if the function 'CheckScanFile' has been called, and the scan-file handler
				has been initialized */
		bool m_initialized;

		/** The filename of the spectrum file */
		CString m_fileName;

		/** A pointer to a log file writer, if this is not null the error output
			from e.g. reading spectra will be directed to this log file writer */
		FileHandler::CLogFileWriter *m_logFileWriter;

		/** The total number of spectra in the current .pak-file */
		int m_specNum;

		/** An array containing the spectra in the current spectrum file.
			These are read in when 'CheckScanFile' is called and retrieved
			by GetSpectrum(...)
			The buffer is introduced to save some read/writes from hard-disk */
		CArray <CSpectrum, CSpectrum&> m_spectrumBuffer;
		
		/** The number of spectra read in to the m_spectrumBuffer 
			This might not be the same as 'm_specNum' */
		int m_spectrumBufferNum;
		// ----------------------------------------------------------------------
		// --------------------- PRIVATE METHODS --------------------------------
		// ----------------------------------------------------------------------



	};
}