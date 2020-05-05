#pragma once

#include "Common/Common.h"
#include "Evaluation/ScanResult.h"
#include "WindMeasurement/WindSpeedResult.h"

/** <b>CEvaluatedDataStorage</b> is a class for holding evaluated data for 
    later plotting. */

const enum SPECTROMETER_STATUS {STATUS_GREEN, STATUS_YELLOW, STATUS_RED};

class CEvaluatedDataStorage
{
	// ------------------- class CScanData ----------------------
	// --- takes care of 'remembering' data from entire scans ---
	class CScanData{
	public:
		CScanData();
		~CScanData();
		double	m_flux;				// the calculated flux [kg/s]
		bool		m_fluxOk;		// true if the flux-value is a good measurement
		double	m_time;				// the time-of-day (seconds since midnight) when the scan was started
		int			m_date;			// the date (day of month) when the scan was made
		double	m_battery;			// the battery voltage [V] when the scan was started
		double	m_temp;				// the temperature of the instrument [deg C] when the scan was started
		long		m_expTime;		// the exposure-time of the measurement
		CScanData &operator=(const CScanData &);
	};

	// ------------------- class CSpectrumData --------------------
	// --- takes care of 'remembering' data from full scans -------
	class CSpectrumData{
	public:
		CSpectrumData();
		~CSpectrumData();
		int     m_time;           // the time of day for each spectrum (seconds since midnight)
		double  m_column;         // the evaluated columns
		double  m_columnError;    // the error in the evaluated columns
		double  m_peakSaturation; // the peak saturation-ratios of the spectrum
		double  m_fitSaturation;  // the maximum saturation-ratio in the fit region of the spectrum
		double  m_angle;          // the scan angle used
		bool    m_isBadFit;       // the goodness of fit for the evaluated spectrum
		CSpectrumData &operator=(const CSpectrumData &);
	};

	// ------------------- class CWindMeasData -----------------------
	// --- takes care of 'remembering' data wind-measurements made ---
	class CWindMeasData{
	public:
		CWindMeasData();
		~CWindMeasData();
		int    m_scannerIndex;   // the index of the scanner that generated this measurement
		int    m_time;           // the time of day when the wind-speed measurement was started (seconds since midnight)
		int    m_date;           // the date (day of month) when the measurement was made
		int    m_duration;       // the duration of the measurement (seconds)
		double m_correlation;    // the average correlation coefficient of the measurement (0->1)
		double m_windSpeed;      // the calculated wind-speed
		double m_windSpeedErr;   // the estimated error in the wind-speed
		CWindMeasData &operator=(const CWindMeasData &);
	};

public:

	CEvaluatedDataStorage(void);
	~CEvaluatedDataStorage(void);

	// ----------------------------------------------------------------------
	// ---------------------- PUBLIC DATA -----------------------------------
	// ----------------------------------------------------------------------

	static const int MAX_HISTORY = 1000;

	// ----------------------------------------------------------------------
	// ---------------------- PUBLIC METHODS --------------------------------
	// ----------------------------------------------------------------------

	/** Adds another set of evaluated data to the storage.
	    If 'result' is NULL, then the only serial number will be inserted
	    into the data-set. */
	int AddData(const CString &serial, Evaluation::CScanResult *result);

	/** Adds another set of measured wind-speed data to the storage.
	    If 'result' is NULL, then the only serial number will be inserted
	    into the data-set. */
	int AddWindData(const CString &serial, WindSpeedMeasurement::CWindSpeedResult *result);

	/** Adds flux result */
	void AppendFluxResult(int scannerIndex, const CDateTime &time, double fluxValue, bool fluxOk, double batteryVoltage = -999, double temp = -999, long expTime = -999);

	/** Add spec data to history */
	void AppendSpecDataHistory(int scannerIndex, int time, double column, double columnError,
		double peakSaturation, double fitSaturation, double angle, bool isBadFit);

	/** Returns the smallest and the largest flux in the data bank */
	void GetFluxRange(const CString &serial, double &minFlux, double &maxFlux);

	/** Returns the smallest and the largest columns in the data bank */
	void GetColumnRange(const CString &serial, double &minColumn, double &maxColumn, bool fullDay=false);

	/** Returns the smallest and the largest angle in the data bank */
	void GetAngleRange(const CString &serial, double &minAngle, double &maxAngle);

	/** Get Column data for last scan. 
	    @param serial - the serial number of the spectrometer for which the data should be retrieved.
	    @param dataBuffer - the column data will be copied into this buffer.
	    @param dataErrorBuffer - the column error data will be copied into this buffer.
	    @param bufferSize - the maximum number of data points that the buffer can handle.
		@param fullDay - whether to get for full day (true) or just last scan (false)
	    @return the number of data points copied into the dataBuffer*/
	long GetColumnData(const CString &serial, double *dataBuffer, double *dataErrorBuffer, long bufferSize, bool fullDay = false);
	
	/** Get Time data. 
	    @param serial - the serial number of the spectrometer for which the data should be retrieved.
	    @param dataBuffer - the time data will be copied into this buffer.
	    @param bufferSize - the maximum number of data points that the buffer can handle.
		@param fullDay - 0 if for last scan; 1 if for full day
	    @return the number of data points copied into the dataBuffer*/
	long GetTimeData(const CString &serial, double *dataBuffer, long bufferSize, bool fullDay=false);

	/** Get the Column data for the measurements which are considered bad.
	    @param serial - the serial number of the spectrometer for which the data should be retrieved.
	    @param dataBuffer - the column data of the bad measurements will be copied into this buffer.
	    @param bufferSize - the maximum number of data points that the buffer can handle.
		@param fullDay - whether to get for full day (true) or just last scan (false)
	    @return the number of column data points (good and bad) */
	long GetBadColumnData(const CString &serial, double *dataBuffer, long bufferSize, bool fullDay=false);

	/** Get the Column data for the measurements which are considered good.
	@param serial - the serial number of the spectrometer for which the data should be retrieved.
	@param dataBuffer - the column data of the bad measurements will be copied into this buffer.
	@param bufferSize - the maximum number of data points that the buffer can handle.
	@param fullDay - whether to get for full day (true) or just last scan (false)
	@return the number of column data points (good and bad) */
	long GetGoodColumnData(const CString &serial, double *dataBuffer, long bufferSize, bool fullDay = false);

	/** Get Angle data. 
	    @param serial - the serial number of the spectrometer for which the data should be retrieved.
	    @param dataBuffer - the column data will be copied into this buffer.
	    @param bufferSize - the maximum number of data points that the buffer can handle.
	    @return the number of data points copied into the dataBuffer*/
	long GetAngleData(const CString &serial, double *dataBuffer, long bufferSize);

	/** Get Intensity data. 
	    @param scannerIndex - the scanner for which the data should be retrieved.
	    @param peakSat - the peak saturation data will be copied into this buffer.
	    @param fitSat - the fit saturation data will be copied into this buffer.
	    @param bufferSize - the maximum number of data points that the buffer can handle.
		@param fullDay - whether to get for full day (true) or just last scan (false)
	    @return the number of data points copied into the dataBuffer*/
	long GetIntensityData(const CString &serial, double *peakSat, double *fitSat, long bufferSize, bool fullDay=true);

	/** Get Flux data. 
	    @param serial - the serial number of the spectrometer for which the data should be retrieved.
	    @param timeBuffer - the time-data will be copied into this buffer. The time format is the number of seconds since midnight
	    @param dataBuffer - the flux data will be copied into this buffer.
	    @param qualityBuffer - the quality of the data will be copied into this buffer
	    @param bufferSize - the maximum number of data points that the buffer can handle.
	    @return the number of data points copied into the dataBuffer*/
	long GetFluxData(const CString &serial, double *timeBuffer, double *dataBuffer, int *qualityBuffer, long bufferSize);

	/** Get flux statistics
	    @param serial - the serial number of the spectrometer for which the data should be retrieved.
	    @param average - the average flux-value will be copied to this parameter
	    @param std - the standard deviation of the fluxes will be copied to this parameter
	    @return the number of data points used */
	long GetFluxStat(const CString &serial, double &average, double &std);

	/** Get wind-measurement data
	    @param serial - the serial number of the spectrometer for which we want to retrieve todays wind-measurements
	    @param timeBuffer - the time-data will be copied into this buffer. Times are in seconds since midnight.
	    @param wsBuffer - the calculated wind-speeds will be copied into this buffer. Unit is m/s
	    @param wseBuffer - the estimated errors in the calculated wind-speed will be copied into this buffer. 
	    @param bufferSize - the maximum number of data points that the buffers can handle.
	    @return the number of datapoints copied into the buffers. */
	long GetWindMeasurementData(const CString &serial, double *timeBuffer, double *wsBuffer, double *wseBuffer, long bufferSize);

	/** Get the offset of the last scan */
	double GetOffset(const CString &serial);

	/** Get the scan-angle for the calculated plume-centre 
	    position of the last scan. */
	double GetPlumeCentre(const CString &serial);

	/** Set the status of the spectrometer 
	    @param serial - the serial number of the spectrometer which status should be updated.
	    @return 0 on success. return 1 on failure */
	int SetStatus(const CString &serial, SPECTROMETER_STATUS status);

	/** Get the status of the spectrometer 
	    @param serial - the serial number of the spectrometer which status should be updated
	    @return 0 on sucess. @return 1 on failure. */
	int GetStatus(const CString &serial, SPECTROMETER_STATUS &status);

	/** Gets the dynamic range for the spectrometer. Returns 0 if unknown */
	double GetDynamicRange(const CString &serial);

	/** Gets the temperature of the last scan */
	double  GetTemperature(const CString &serial);

	/** Gets the temperature of saved scans.
	    @param serial - the serial number of the spectrometer for which the data should be retrieved.
	    @param timeBuffer - the time-data will be copied into this buffer. The time format is the number of seconds since midnight
	    @param dataBuffer - the flux data will be copied into this buffer.
	    @param bufferSize - the maximum number of data points that the buffer can handle.
	    @return the number of data points copied into the dataBuffer*/
	long  GetTemperatureData(const CString &serial, double *timeBuffer, double *dataBuffer, long bufferSize);

	/** Gets the battery voltage of the last scan */
	double GetBatteryVoltage(const CString &serial);

	/** Gets the battery-voltage of saved scans.
	    @param serial - the serial number of the spectrometer for which the data should be retrieved.
	    @param timeBuffer - the time-data will be copied into this buffer. The time format is the number of seconds since midnight
	    @param dataBuffer - the flux data will be copied into this buffer.
	    @param bufferSize - the maximum number of data points that the buffer can handle.
	    @return the number of data points copied into the dataBuffer*/
	long GetBatteryVoltageData(const CString &serial, double *timeBuffer, double *dataBuffer, long bufferSize);

	/** Gets the exposure-times of saved scans.
	    @param serial - the serial number of the spectrometer for which the data should be retrieved.
	    @param timeBuffer - the time-data will be copied into this buffer. The time format is the number of seconds since midnight
	    @param dataBuffer - the flux data will be copied into this buffer.
	    @param bufferSize - the maximum number of data points that the buffer can handle.
	    @return the number of data points copied into the dataBuffer*/
	long GetExposureTimeData(const CString &serial, double *timeBuffer, double *dataBuffer, long bufferSize);

	/** Gets the lowest recorded temperature for the given spectrometer */
	double GetMinTemperature(const CString &serial);
		
	/** Gets the highest recorded temperature for the given spectrometer */
	double GetMaxTemperature(const CString &serial);

	/** Gets the lowest recorded battery voltage for the given spectrometer */
	double GetMinBatteryVoltage(const CString &serial);

	/** Gets the highest recorded battery voltage for the given spectrometer */
	double GetMaxBatteryVoltage(const CString &serial);
	
private:
	// ----------------------------------------------------------------------
	// --------------------- PRIVATE DATA -----------------------------------
	// ----------------------------------------------------------------------

	/** The status of the spectrometer */
	SPECTROMETER_STATUS m_spectrometerStatus[MAX_NUMBER_OF_SCANNING_INSTRUMENTS];

	/** The information about the spectra of the last scan */
	CSpectrumData m_specData[MAX_NUMBER_OF_SCANNING_INSTRUMENTS][MAX_SPEC_PER_SCAN];

	/** The number of positions in each scan */
	int m_positionsNum[MAX_NUMBER_OF_SCANNING_INSTRUMENTS];

	/** The information about the spectra for the day */
	CSpectrumData m_specDataDay[MAX_NUMBER_OF_SCANNING_INSTRUMENTS][MAX_SPEC_PER_SCAN*30];

	/** The number of positions in each m_specDataDay */
	int m_specIndex[MAX_NUMBER_OF_SCANNING_INSTRUMENTS];

	/** The offset in the measurement */
	double m_offset[MAX_NUMBER_OF_SCANNING_INSTRUMENTS];

	/** The calculated plume-centre positions */
	double m_plumeCentre[MAX_NUMBER_OF_SCANNING_INSTRUMENTS];

	/** The maximum [1] and mimimum [0] temperature measured for each scanning instrument */
	double m_temperatureRange[2][MAX_NUMBER_OF_SCANNING_INSTRUMENTS];

	/** The 'scan-memory' */
	CScanData m_data[MAX_NUMBER_OF_SCANNING_INSTRUMENTS][MAX_HISTORY];
	int m_fluxIndex[MAX_NUMBER_OF_SCANNING_INSTRUMENTS];

	/** The list of wind-speed measurements made.
	    This is impplemented as a list for simplicity, the number of wind-speed
	    measurements in one day will in all cases be very low. */
	CList <CWindMeasData, CWindMeasData&> m_windData;

	/** The serial numbers */
	CString m_serials[MAX_NUMBER_OF_SCANNING_INSTRUMENTS];

	/** The spectrometer-model that corresponds to each serial-number */
	std::string m_models[MAX_NUMBER_OF_SCANNING_INSTRUMENTS];

	/** The time-difference between GMT and the position of each scanner */
	double	m_hoursToGMT[MAX_NUMBER_OF_SCANNING_INSTRUMENTS];

	/** How many serial numbers have been inserted */
	unsigned int m_serialNum;

	/** Scan end time from evaluation result. */
	CDateTime m_scanTime[MAX_NUMBER_OF_SCANNING_INSTRUMENTS];

	// ----------------------------------------------------------------------
	// -------------------- PRIVATE METHODS ---------------------------------
	// ----------------------------------------------------------------------

	/** Removes old flux results */
	void RemoveOldFluxResults();

	/** Reset spec index */
	void  RemoveOldSpec(int scannerIndex);

	/** Returns the spectrometer index given a serial number */
	int GetScannerIndex(const CString &serial);
};
