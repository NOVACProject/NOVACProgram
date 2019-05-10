#pragma once

#include "../Common/Common.h"
#include <SpectralEvaluation/Spectra/SpectrumInfo.h>
#include <SpectralEvaluation/Evaluation/FitParameter.h>
#include <SpectralEvaluation/Evaluation/EvaluationResult.h>
#include <SpectralEvaluation/Evaluation/BasicScanEvaluationResult.h>
#include "FluxResult.h"

namespace Evaluation
{
	/** <b>CScanResult</b> is a class designed to handle the results
	    from evaluating a set of spectra (e.g. a scan). It contains a set of 
	    CEvaluationResult's which describes the result of evaluating each spectrum.
	    The CScanResult also handles information about the whole set
	    of evaluation results such as the offset or the calculated flux of the scan,
	    or a judgement wheather each evaluated spectrum is judged to be an ok 
	    spectrum or not. */

	class CScanResult : public BasicScanEvaluationResult
	{
	public:
		CScanResult();
		CScanResult(const CScanResult& other);

		~CScanResult();

		// ----------------------------------------------------------------------
		// ---------------------- PUBLIC DATA -----------------------------------
		// ----------------------------------------------------------------------

		// ----------------------------------------------------------------------
		// --------------------- PUBLIC METHODS ---------------------------------
		// ----------------------------------------------------------------------

		/** Appends the result to the list of calculated results */
		int AppendResult(const CEvaluationResult &evalRes, const CSpectrumInfo &specInfo);

		/** Removes the spectrum number 'specIndex' from the list of calcualted results */
		int RemoveResult(unsigned int specIndex);

		/** Intializes the memory arrays to have, initially, space for 
		    'specNum' spectra. */
		void InitializeArrays(long specNum);

		/** Retrieves the evaluation result for spectrum number 
		    'specIndex' from the list of calculated results.
				@return false if specIndex is out of bounds... */
		bool GetResult(unsigned int specIndex, CEvaluationResult& result) const;

		/** Adds spectrum number 'specIndex' into the list of spectra in the .pak -file 
				which are corrupted and could not be evaluated */
		void MarkAsCorrupted(unsigned int specIndex);

		/** Retrieves how many spectra are corrupted in the scan */
		int GetCorruptedNum() const;

		/** Stores the information about the sky-spectrum used */
		void SetSkySpecInfo(const CSpectrumInfo &skySpecInfo);

		/** Stores the information about the dark-spectrum used */
		void SetDarkSpecInfo(const CSpectrumInfo &darkSpecInfo);

		/** Stores the information about the offset-spectrum used */
		void SetOffsetSpecInfo(const CSpectrumInfo &offsetSpecInfo);

		/** Stores the information about the dark-current-spectrum used */
		void SetDarkCurrentSpecInfo(const CSpectrumInfo &darkCurSpecInfo);

		/** Check the last spectrum point for goodness of fit.
		    The parameters 'deltaLimit', 'upperLimit' and 'lowerLimit' are for 
		    development purposes only. */
		bool CheckGoodnessOfFit(const CSpectrumInfo& info, float chi2Limit = -1, float upperLimit = -1, float lowerLimit = -1);

		/** Check spectrum number 'index' for goodness of fit.
		    The parameters 'deltaLimit', 'upperLimit' and 'lowerLimit' are for 
		    development purposes only. */
		bool CheckGoodnessOfFit(const CSpectrumInfo& info, int index, float chi2Limit = -1, float upperLimit = -1, float lowerLimit = -1);

		/** Gets the offset of the scan. The offset is calculated as the average of the 
		  three lowest columns values (bad values are skipped). After this function has 
		  been called the actual offset can be retrieved by a call to 'GetOffset'.
		  @param specie - The name of the specie for which the offset should be found.
		  @return 0 on success. @return 1 - if any error occurs. */
		int CalculateOffset(const std::string &specie);

		/** Calculate the flux in this scan, using the supplied compass direction 
		        and coneAngle.
		    The result is saved in the private parameter 'm_flux', whose vale can be 
		    retrieved by a call to 'GetFlux()'
		    @param spec - the spectrometer with which the scan was collected.
		    @return 0 if all is ok. @return 1 if any error occurs. */
		int CalculateFlux(const std::string &specie, const CWindField &wind, double compass, double coneAngle = 90.0, double tilt = 0.0);

		/** Tries to find a plume in the last scan result. If the plume is found
				this function returns true, and the centre of the plume (in scanAngles) 
				is given in 'plumeCentre', the width of the plume (in scanAngles) 
				is given in 'plumeWidth' and the estimated completeness of the plume
				is given in 'plumeCompleteness' (ranging from 0.0 to 1.0)
				*/
		bool CalculatePlumeCentre(const std::string &specie, double &plumeCentre_alpha, double &plumeCentre_phi, double &plumeCompleteness, double &plumeEdge_low, double &plumeEdge_high);

		/** Tries to find a plume in the last scan result. If the plume is found
		    this function returns true. The result of the calculations is stored in
		    the member-variables 'm_plumeCentre' and 'm_plumeCompleteness' */
		bool CalculatePlumeCentre(const std::string &specie);

		/** Tries to calculate the local wind-direction when this scan was collected.
		    If succesful, this function returns 'true' and the result is saved 
		        in 'm_windDirection' 
		    If plumeHeight is specified, this value will be used as plume-height 
		        otherwise the plume height will be taken from the default 
		        wind-field for this volcano.*/
		bool CalculateWindDirection(double plumeCentre, double plumeHeight = -1);

		/** Checks the kind of measurement that we have here and sets the 
		    flag 'm_measurementMode' to the appropriate value... */
		MEASUREMENT_MODE CheckMeasurementMode();

		/** Returns true if this is a flux measurement */
		bool IsFluxMeasurement();

		/** Returns true if this is a fixed angle measurement */
		bool IsFixedAngleMeasurement() const;

		/** Returns true if this is a wind-speed measurement */
		bool IsWindMeasurement() const;
		
		/** Returns true if this is a stratospheric mode measurement */
		bool IsStratosphereMeasurement() const;

		/** Returns true if this is a direct-sun mode measurement */
		bool IsDirectSunMeasurement() const;

		/** Returns true if this is a lunar mode measurement */
		bool IsLunarMeasurement() const;

		/** Returns true if this is a composition mode measurement */
		bool IsCompositionMeasurement() const;

		/** Calculates the maximum good column value in the scan, 
		    corrected for the offset.
		    NB!! The function 'CalculateOffset' must have been called
		    before this function is called. */
		double GetMaxColumn(const std::string &specie) const;

		/** Returns the calculated flux */
		double GetFlux() const {return m_flux.m_flux; }

		/** Returns true if the automatic judgment considers this flux
		    measurement to be a good measurement */
		bool IsFluxOk() const {return m_flux.m_fluxOk; }

		/** Set the flux to the given value. ONLY USED FOR READING EVALUATION-LOGS */
		void SetFlux(double flux) {this->m_flux.m_flux = flux; }

		/** returns the offset of the measurement */
		double GetOffset() const {return m_offset; } 

		/** returns the temperature of the system at the time of the measurement */
		double	GetTemperature() const {return m_skySpecInfo.m_temperature; }

		/** Returns the calculated wind-direction */
		double	GetCalculatedWindDirection() const { return m_windDirection; }

		/** Returns the calculated plume-centre position */
		double	GetCalculatedPlumeCentre(int motor = 0) const {return m_plumeCentre[motor]; }

		/** Returns the calculated plume edges */
		void GetCalculatedPlumeEdges(double &lowEdge, double &highEdge) const;

		/** Returns the calculated plume-completeness */
		double	GetCalculatedPlumeCompleteness() const {return m_plumeCompleteness; }

		/** Sets the offset of the measurement */
		void SetOffset(double offset) {m_offset = offset; } 

		/** returns the number of spectra evaluated */
		unsigned long  GetEvaluatedNum() const {return m_specNum; }

		/** Returns the latitude of the system */
		double	GetLatitude() const;

		/** Returns the longitude of the system */
		double	GetLongitude() const;

		/** Returns the altitude of the system */
		double	GetAltitude() const;

		/** Returns the compass-direction of the system */
		double	GetCompass() const;

		/** Returns the cone angle of the scanning instrument */
		double	GetConeAngle() const;

		/** Returns the pitch (tilt) of the scanning instrument */
		double	GetPitch() const;

		/** Returns the roll (scan-angle offset) of the scanning instrument */
		double	GetRoll() const;

		/** Returns the battery-voltage of the sky spectrum */ 
		float	GetBatteryVoltage() const;

		/** Returns the name of the requested spectrum */
        std::string GetName(int index) const;

		/** Returns the serial-number of the spectrometer that collected this scan */
        std::string GetSerial() const;

		/** returns the goodness of fit for the fitting of the evaluated 
		        spectrum number 'index'. 
		    This function is the complementary of IsBad(unsigned long index)*/
		int  IsOk(unsigned long index) const {return m_spec[index].IsOK(); }

		/** returns the goodness of fit for the fitting of the evaluated 
		        spectrum number 'index'. 
		    This function is the complementary of IsOK(unsigned long index). */
		int  IsBad(unsigned long index) const {return m_spec[index].IsBad(); }

		/** returns true if the evaluated spectrum number 'index' is marked 
		        as deleted */
		int  IsDeleted(unsigned long index) const {return m_spec[index].IsDeleted(); }

		/** Marks the desired spectrum with the supplied mark_flag.
		    Mark flag must be MARK_BAD_EVALUATION, or MARK_DELETED
		    @return true on success. */
		bool MarkAs(unsigned long index, int MARK_FLAG);

		/** Removes the desired mark from the desired spectrum
		    Mark flag must be MARK_BAD_EVALUATION, or MARK_DELETED
		    @return true on success. */
        bool RemoveMark(unsigned long index, int MARK_FLAG);

		/** Returns a reference to the desired spectrum info-structure */
		const CSpectrumInfo &GetSpectrumInfo(unsigned long index) const;

		/** Returns a reference to the spectrum info-structure of the sky-spectrum used */
		const CSpectrumInfo &GetSkySpectrumInfo() const;

		/** Returns a reference to the spectrum info-structure of the dark-spectrum used */
		const CSpectrumInfo &GetDarkSpectrumInfo() const;

		/** returns the scan angle of evaluated spectrum number 'index'.
		    @param index - the zero based index into the list of  evaluated spectra */
		float GetScanAngle(unsigned long index) const { return (IsValidSpectrumIndex(index)) ? m_specInfo[index].m_scanAngle : 0; }

		/** returns the azimuth angle (the angle of the second motor) of 
		        evaluated spectrum number 'index'.
		    @param index - the zero based index into the list of  evaluated spectra */
		float GetScanAngle2(unsigned long index) const { return (IsValidSpectrumIndex(index)) ? m_specInfo[index].m_scanAngle2 : 0; }

		/** returns the time (UMT) when evaluated spectrum number 'index' was started.
		    @param index - the zero based index into the list of evaluated spectra */
		const CDateTime *GetStartTime(unsigned long index) const {return (IsValidSpectrumIndex(index)) ? &m_specInfo[index].m_startTime : NULL; }

		/** returns the time and date (UMT) when evaluated spectrum number 
		        'index' was started.
		    @param index - the zero based index into the list of evaluated spectra.
		    @return true if the index is valid */
		bool GetStartTime(unsigned long index, CDateTime &time) const;

		/** returns the time and date (UMT) when the sky-spectrum was started. */
		void CScanResult::GetSkyStartTime(CDateTime &t) const;

		/** return the time (UMT) when evaluated spectrum number 'index' was stopped
		    @param index - the zero based index into the list of evaluated spectra */
		const CDateTime *GetStopTime(unsigned long index) const {return (IsValidSpectrumIndex(index)) ? &m_specInfo[index].m_stopTime : NULL; }

		/** returns the time and date (UMT) when evaluated spectrum number 'index' 
		        was stopped.
		    @param index - the zero based index into the list of evaluated spectra.
		    @return true if the index is valid */
		bool GetStopTime(unsigned long index, CDateTime &time) const;

		/** returns the date (UMT) when the evaluated spectrum number 'index'
		        was collected
		    @param index - the zero-based index into the list of evaluated spectra. */
		bool GetDate(unsigned long index, unsigned short date[3]) const;

		/** returns the evaluated column for specie number 'specieNum' and 
		        spectrum number 'specNum'
		    @param specieNum - the zero based index into the list of species 
		        to evaluate for
		    @param spectrumNum - the zero based index into the list of evaluated 
		        spectra.*/
		double GetColumn(unsigned long spectrumNum, unsigned long specieNum) const;

		/** returns the error for the evaluated column for specie number 
		        'specieNum' and spectrum number 'specNum'
		    @param specieNum - the zero based index into the list of species 
		        to evaluate for
		    @param spectrumNum - the zero based index into the list of evaluated 
		        spectra.*/
		double GetColumnError(unsigned long spectrumNum, unsigned long specieNum) const;

		/** returns the SHIFT parameter for specie number 'specieNum' and 
		        spectrum number 'specNum'
		    @param specieNum - the zero based index into the list of species 
		        to evaluate for
		    @param spectrumNum - the zero based index into the list of 
		        evaluated spectra.*/
		double GetShift(unsigned long spectrumNum, unsigned long specieNum) const;

		/** returns the error for the SHIFT parameter for specie number 
		        'specieNum' and spectrum number 'specNum'
		    @param specieNum - the zero based index into the list of species 
		        to evaluate for
		    @param spectrumNum - the zero based index into the list of 
		        evaluated spectra.*/
		double GetShiftError(unsigned long spectrumNum, unsigned long specieNum) const;

		/** returns the SQUEEZE parameter for specie number 'specieNum' and 
		        spectrum number 'specNum'
		    @param specieNum - the zero based index into the list of species 
		        to evaluate for
		    @param spectrumNum - the zero based index into the list of 
		        evaluated spectra.*/
		double GetSqueeze(unsigned long spectrumNum, unsigned long specieNum) const;

		/** returns the error for the SQUEEZE parameter for specie number 
		        'specieNum' and spectrum number 'specNum'
		    @param specieNum - the zero based index into the list of species 
		        to evaluate for
		    @param spectrumNum - the zero based index into the list of 
		        evaluated spectra.*/
		double GetSqueezeError(unsigned long spectrumNum, unsigned long specieNum) const;

		/** @return the delta of the fit for spectrum number 'spectrumNum'
		    @param spectrumNum - the spectrum number (zero-based) for 
		        which the delta value is desired */
		double GetDelta(unsigned long spectrumNum) const;

		/** @return the chi-square of the fit for spectrum number 'spectrumNum'
		    @param spectrumNum - the spectrum number (zero-based) for 
		        which the delta value is desired */
		double GetChiSquare(unsigned long spectrumNum) const;

		/** returns the number of spectra averaged to get evaluated 
		        spectrum number 'spectrumNum' 
		    @return the number of spectra averaged. */
		long GetSpecNum(unsigned long spectrumNum) const {return (IsValidSpectrumIndex(spectrumNum)) ? m_specInfo[spectrumNum].m_numSpec: 0; }

		/** returns the expsure time of evaluated spectrum number 'spectrumNum' 
		        in ms  */
		long GetExposureTime(unsigned long spectrumNum) const {return (IsValidSpectrumIndex(spectrumNum)) ? m_specInfo[spectrumNum].m_exposureTime: 0; }

		/** returns the peak intensity of evaluated spectrum number 'spectrumNum'
		    (the maximum intensity of the whole spectrum). */
		float GetPeakIntensity(unsigned long spectrumNum) const {return (IsValidSpectrumIndex(spectrumNum)) ? m_specInfo[spectrumNum].m_peakIntensity: 0; }

		/** returns the fit intensity of evaluated spectrum number 'spectrumNum'
		    (the maximum intensity int the fit region of the spectrum). */
		float GetFitIntensity(unsigned long spectrumNum) const {return (IsValidSpectrumIndex(spectrumNum)) ? m_specInfo[spectrumNum].m_fitIntensity: 0; }

		/** returns the electronic offset in spectrum number 'spectrumNum' */
		float GetElectronicOffset(unsigned long spectrumNum) const {return (IsValidSpectrumIndex(spectrumNum)) ? m_specInfo[spectrumNum].m_offset: 0; }

		/** returns true if the spectra have been evaluated for the supplied specie.
		    @param specie - a string containing the name of the specie to 
		        search for, e.g. "SO2" (case insensitive)*/
		bool IsEvaluatedSpecie(const std::string &specie) const {return (-1 != GetSpecieIndex(specie)); }

		/** returns the number of species that were used in the evaluation of a 
		    given spectrum */
		int GetSpecieNum(unsigned long spectrumNum) const {return (IsValidSpectrumIndex(spectrumNum)) ? (int)m_spec[spectrumNum].m_referenceResult.size() : 0; }

		/** returns the specie name */
		const std::string GetSpecieName(unsigned long spectrumNum, unsigned long specieNum) const;

		/** Assignment operator */
		CScanResult &operator=(const CScanResult &s2);

		/** Getting the estimated geometrical error */
		double	GetGeometricalError() const;

		/** Getting the scattering Error */
		double	GetScatteringError() const;

		/** Getting the spectroscopical error */
		double	GetSpectroscopicalError() const;

		/** Getting the estimated geometrical error */
		void	SetGeometricalError(double err);

		/** Getting the scattering Error */
		void	SetScatteringError(double err);

		/** Getting the spectroscopical error */
		void	SetSpectroscopicalError(double err);
	private:

		// ----------------------------------------------------------------------
		// --------------------- PRIVATE DATA -----------------------------------
		// ----------------------------------------------------------------------

		/** The offset in the measurement */
		double m_offset;

		/** The calculated flux and the parameters used to 
		     calculate the flux */
		CFluxResult m_flux;

		/** The estimated error (in percent) in the geometrical setup for
		    this flux-calculation. */
		double	m_geomError;

		/** The estimated error (in percent) due to scattering inside or below the
		    plume for this flux-calculation */
		double	m_scatteringError;

		/** The estimated error (in percent) of the spectral results due
		    to incertainties in cross-sections, slit-functions, stray-light etc. */
		double	m_spectroscopyError;

		/** The calculated wind-direction. This is set to -999 if unknown */
		double	m_windDirection;

		/** The scan-angle for the calculated plume-centre. Set to -999 if unknown */
		double	m_plumeCentre[2];

		/** The calculated edges of the plume. Set to -999 if unknown */
		double	m_plumeEdge[2];

		/** The calculated completness of the plume. Set to -999 if unknown */
		double	m_plumeCompleteness;

		/** The number of evaluations */
		unsigned long m_specNum = 0;
	
		/** Flag to signal if this is a wind measurement, a scan, or something else. */
		MEASUREMENT_MODE m_measurementMode;

		// ----------------------------------------------------------------------
		// -------------------- PRIVATE METHODS ---------------------------------
		// ----------------------------------------------------------------------

		/** Returns the index, i, into 'm_evaluated[x].m_ref[i]' that corresponds to the gas 'specie'
		    @param specie - the gas to search for e.g. SO2 (case insensitive).
		    @return the zero-based index of the specie.
		    @return -1 if specie not found.*/
		int GetSpecieIndex(const std::string &specie) const;

		/** makes a sanity check of the parameters and returns fit parameter number 'index'.
		    @param specIndex - the zero based into the list of evaluated spectra.
		    @param specieIndex - the zero based into the list of species to evaluate for.
		    @param fitParameter - a parameter to return.
		    @return NaN if any parameter is wrong */
		double GetFitParameter(unsigned long spectrumNum, unsigned long specieIndex, FIT_PARAMETER parameter) const; 

		/** returns true if the given index is a valid spectrum index */
		inline bool IsValidSpectrumIndex(unsigned long spectrumNum) const { return (spectrumNum >= 0 && spectrumNum < m_specNum) ? true : false; }
	};
}