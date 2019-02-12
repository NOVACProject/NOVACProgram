#pragma once

#include "../Common/LogFileWriter.h"
#include "../SpectralEvaluation/Evaluation/ReferenceFile.h"
#include "../Configuration/Configuration.h"

#include "SpectrometerHistory.h"
#include "ScanResult.h"

#include "Evaluation.h"

namespace Evaluation
{
	/** <b>CSpectrometer</b> is a data structure which is used by the 
			EvaluationController to keep track of what spectrometers there are, 
			where they are, and how to evaluate spectra from them. */
	class CSpectrometer
	{
	public:
		/** Default constructor */
		CSpectrometer(void);

		/** Default desctructor */
		~CSpectrometer(void);

		/** The settings for this spectrometer */
		CConfigurationSetting::SpectrometerSetting m_settings;

		/** The scanning instrument to which the spectrometer is connected */
		CConfigurationSetting::ScanningInstrumentSetting m_scanner;

		/** The local history at this spectrometer. For multichannel-
				spectrometers, this is a shared object between the channels. */
		CSpectrometerHistory *m_history;

		/** A set of evaluators which can evaluate spectra from this spectrometer.
				Each evaluator evaluates for one fit window. */
        std::vector<CEvaluation> m_evaluator;
		// CEvaluation *m_evaluator[MAX_FIT_WINDOWS];

		/** The number of fit windows defined */
		unsigned short	m_fitWindowNum;
		
		/** The channel that this spectrometer is configured as. For OceanOptics
				S2000 spectrometers, there can be several channels contained in the
				same box. They will then have the same serialnumber, but different channels.
				If such a spectrometer is used, one CSpectrometer should be configured
				for each channel. */
		unsigned char m_channel;

		/** The number of GPS-readings that have been averaged to get the 
				current position of this scanner */
		double m_gpsReadingsNum;

		/** A handler for writing log-file, to handle the output of the evaluation. */
		FileHandler::CLogFileWriter m_logFileHandler;

		/** Getting the serial number of the spectrometer */
		const CString &SerialNumber() const;

		/** Getting the maximum intensity of one single spectrum 
			(equals to the range of the AD-Converter).
			Returns 0.0 if unknown. */
		double	GetMaxIntensity() const;

		/** Adds the result from the supplied evaluation to the history
			of evaluations. */
		void RememberResult(CScanResult &lastResult);

		/** assignment operator */
		CSpectrometer &operator=(const CSpectrometer &spec2);
	};
}