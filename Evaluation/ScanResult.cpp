#include "StdAfx.h"
#include "ScanResult.h"
#include "../VolcanoInfo.h"

// we also need the meterological data
#include "../MeteorologicalData.h"

#include "../Geometry/GeometryCalculator.h"

using namespace Evaluation;

extern CMeteorologicalData g_metData;			// <-- The meteorological data
extern CVolcanoInfo					g_volcanoes;	// <-- A list of all known volcanoes


CScanResult::CScanResult(void)
{
	m_specNum = 0;
	m_offset = 0;
	m_flux.Clear();
	m_geomError					= 30.0;	// best-case guess, 30%
	m_spectroscopyError			= 15.0;	// best-case guess, 15%
	m_scatteringError			= 30.0;	// best-case guess, 30%
	m_windDirection				= -999.0;
	m_plumeCentre[0]			= -999.0;
	m_plumeCentre[1]			= -999.0;
	m_plumeEdge[0]				= -999.0;
	m_plumeEdge[1]				= -999.0;
	m_plumeCompleteness			= -999.0;
	m_measurementMode			= MODE_UNKNOWN;
	m_instrumentType			= INSTR_GOTHENBURG;
	m_corruptedNum				= 0;
}

CScanResult::~CScanResult(void)
{
	m_spec.RemoveAll();
	m_specInfo.RemoveAll();
	m_corruptedSpectra.RemoveAll();
}

/** Intializes the memory arrays to have, initially, space for 
		'specNum' spectra. */
void CScanResult::InitializeArrays(long specNum){
	if(specNum < 0 || specNum > 1024){
		return;
	}

	m_spec.SetSize(specNum);
	m_specInfo.SetSize(specNum);
}

/** Appends the result to the list of calculated results */
int CScanResult::AppendResult(const CEvaluationResult &evalRes, const CSpectrumInfo &specInfo){

	// Append the evaluationresult to the end of the 'm_spec'-vector
	m_spec.SetAtGrow(m_specNum, CEvaluationResult(evalRes));

	// Append the spectral information to the end of the 'm_specInfo'-vector
	m_specInfo.SetAtGrow(m_specNum, CSpectrumInfo(specInfo));

	// Increase the numbers of spectra in this result-set.
	++m_specNum;
	return 0;
}

const CEvaluationResult *CScanResult::GetResult(unsigned int specIndex) const{
	if(specIndex >= m_specNum)
		return NULL; // not a valid index

	return &m_spec.GetAt(specIndex);
}

/** Adds spectrum number 'specIndex' into the list of spectra in the .pak -file 
		which are corrupted and could not be evaluated */
void	CScanResult::MarkAsCorrupted(unsigned int specIndex){
	m_corruptedSpectra.SetAtGrow(m_corruptedNum++, specIndex);
}

/** Retrieves how many spectra are corrupted in the scan */
int CScanResult::GetCorruptedNum() const{
	return m_corruptedNum;
}

/** Removes the spectrum number 'specIndex' from the list of calcualted results */
int CScanResult::RemoveResult(unsigned int specIndex){
	if(specIndex >= m_specNum)
		return 1; // not a valid index

	// Remove the desired value
	m_specInfo.RemoveAt(specIndex, 1);

	// Decrease the number of values in the list
	m_specNum -= 1;

	return 0;
}

/** Stores the information about the sky-spectrum used */
void CScanResult::SetSkySpecInfo(const CSpectrumInfo &skySpecInfo){
	this->m_skySpecInfo = skySpecInfo;
}

/** Stores the information about the dark-spectrum used */
void CScanResult::SetDarkSpecInfo(const CSpectrumInfo &darkSpecInfo){
	this->m_darkSpecInfo = darkSpecInfo;
}

/** Stores the information about the offset-spectrum used */
void CScanResult::SetOffsetSpecInfo(const CSpectrumInfo &offsetSpecInfo){
	this->m_offsetSpecInfo = offsetSpecInfo;
}

/** Stores the information about the dark-current-spectrum used */
void CScanResult::SetDarkCurrentSpecInfo(const CSpectrumInfo &darkCurSpecInfo){
	this->m_darkCurSpecInfo = darkCurSpecInfo;
}

/** Check the last spectrum point for goodness of fit */
bool CScanResult::CheckGoodnessOfFit(const CSpectrumInfo& info, float chi2Limit, float upperLimit, float lowerLimit){
	return CheckGoodnessOfFit(info, m_specNum-1, chi2Limit, upperLimit, lowerLimit);
}

/** Check spectrum number 'index' for goodness of fit */
bool CScanResult::CheckGoodnessOfFit(const CSpectrumInfo& info, int index, float chi2Limit, float upperLimit, float lowerLimit){
	if(index < 0 || (unsigned int)index >= m_specNum)
		return false;

	// remember the electronic offset (NB. this is not same as the scan-offset)
//  m_specInfo[index].m_offset				= (float)offsetLevel;

	return m_spec[index].CheckGoodnessOfFit(info, chi2Limit, upperLimit, lowerLimit);
}

int CScanResult::CalculateOffset(const CString &specie){
	if(m_specNum < 0)
		return 1;

	// Get the index for the specie for which we want to calculate the offset
	long specieIndex = GetSpecieIndex(specie);
	if(specieIndex == -1){ // if the specified specie does not exist
		return 1;
	}

	double  *columns = new double[m_specNum];
	bool    *badEval = new bool[m_specNum];

	// We then need to rearrange the column data a little bit. 
	for(unsigned int i = 0; i < m_specNum; ++i){
		columns[i] = m_spec[i].m_ref[specieIndex].m_column;

		// The spectrum is considered as bad if the goodness-of-fit checking
		//	has marked it as bad or the user has marked it as deleted
		if(m_spec[i].IsBad() || m_spec[i].IsDeleted())
			badEval[i] = true;
		else
			badEval[i] = false;
	}

	// Calculate the offset
	this->m_offset = Common::CalculateOffset(columns, badEval, m_specNum);

	delete [] columns;
	delete [] badEval;
	return 0;
}

int CScanResult::GetSpecieIndex(const CString &specie) const
{
	unsigned long i; // iterator

	if(m_specNum <= 0) // <-- if there are no spectra, there can be no species
		return -1;

	// if there's only one specie, assume that this is the correct one
	if(m_spec[0].m_speciesNum == 1)
		return 0;

	// find the index of the interesting specie
	for(i = 0; i < m_spec[0].m_speciesNum; ++i){
		if(Equals(m_spec[0].m_ref[i].m_specieName, specie)){
			return i;
		}
	}

	return -1;
}

int CScanResult::CalculateFlux(const CString &specie, const CWindField &wind, double compass, double coneAngle, double tilt){
	unsigned long i; // iterator

	// If this is a not a flux measurement, then don't calculate any flux
	if(!IsFluxMeasurement())
		return 1;

	// get the specie index
	int specieIndex = GetSpecieIndex(specie);
	if(specieIndex == -1){
		return 1;
	}

	// get the gas factor
	double gasFactor = Common::GetGasFactor(specie);
	if(gasFactor == -1){
		// spec.m_logFileHandler.WriteErrorMessage(TEXT("No GasFactor defined for: " + specie));
		return 1;
	}

	// pull out the good data points out of the measurement and ignore the bad points
	double *scanAngle	= new double[m_specNum];
	double *scanAngle2	= new double[m_specNum];
	double *column		= new double[m_specNum];
	unsigned int nDataPoints = 0;
	for(i = 0; i < m_specNum; ++i){
		if(m_spec[i].IsBad() || m_spec[i].IsDeleted())
			continue; // this is a bad measurement
		if(m_specInfo[i].m_flag >= 64)
			continue; // this is a direct-sun measurement, don't use it to calculate the flux...
		if(Equals(m_specInfo[i].m_name, "direct_sun", 10) || 
			Equals(m_specInfo[i].m_name, "direct_moon", 11) || 
			Equals(m_specInfo[i].m_name, "sun_", 4) || 
			Equals(m_specInfo[i].m_name, "moon_", 5) || 
			Equals(m_specInfo[i].m_name, "sky", 3) ||
			Equals(m_specInfo[i].m_name, "strat", 5) ||
			Equals(m_specInfo[i].m_name, "trop", 4) ||
			Equals(m_specInfo[i].m_name, "maxdoas", 7) ||
			Equals(m_specInfo[i].m_name, "special", 7) ||
			Equals(m_specInfo[i].m_name, "wind", 4) ||
			Equals(m_specInfo[i].m_name, "dark", 4) ||
			Equals(m_specInfo[i].m_name, "dark_cur", 8) ||
			Equals(m_specInfo[i].m_name, "offset", 6)){
			continue; // this is not intended to be used for calculating the flux
		}

		scanAngle[nDataPoints]  = m_specInfo[i].m_scanAngle;
		scanAngle2[nDataPoints] = m_specInfo[i].m_scanAngle2;
		column[nDataPoints]     = m_spec[i].m_ref[specieIndex].m_column;
		++nDataPoints;
	}

	// if there are no good datapoints in the measurement, the flux is assumed to be zero
	if(nDataPoints < 10){
		delete[] scanAngle;
		delete[] scanAngle2;
		delete[] column;
		m_flux.Clear();
		if(nDataPoints == 0)
			ShowMessage("Could not calculate flux, no good datapoints in measurement");
		else
			ShowMessage("Could not calculate flux, too few good datapoints in measurement");
		return 1;
	}

	// Calculate the flux
	m_flux.m_flux = Common::CalculateFlux(scanAngle, scanAngle2, column, m_offset, nDataPoints, wind, compass, gasFactor, m_instrumentType, coneAngle, tilt);
	m_flux.m_windDirection       = wind.GetWindDirection();
	m_flux.m_windDirectionSource = wind.GetWindDirectionSource();
	m_flux.m_windSpeed           = wind.GetWindSpeed();
	m_flux.m_windSpeedSource     = wind.GetWindSpeedSource();
	m_flux.m_plumeHeight         = wind.GetPlumeHeight();
	m_flux.m_plumeHeightSource   = wind.GetPlumeHeightSource();
	m_flux.m_compass             = compass;
	m_flux.m_coneAngle           = coneAngle;
	m_flux.m_tilt                = tilt;
	GetStartTime(0, m_flux.m_startTime);

	delete[] scanAngle;
	delete[] scanAngle2;
	delete[] column;

	return 0;
}

/** Tries to find a plume in the last scan result. If the plume is found
		this function returns true. The result of the calculations is stored in
		the member-variables 'm_plumeCentre', 'm_plumeCompleteness' and m_plumeEdge[0] and m_plumeEdge[1] */
bool CScanResult::CalculatePlumeCentre(const CString &specie){
	double plumeCentre_alpha, plumeCentre_phi, plumeCompleteness;
	return CalculatePlumeCentre(specie, plumeCentre_alpha, plumeCentre_phi, plumeCompleteness, m_plumeEdge[0], m_plumeEdge[1]);
}

/** Tries to find a plume in the last scan result. If the plume is found
		this function returns true, and the centre of the plume (in scanAngles) 
		is given in 'plumeCentre' */
bool CScanResult::CalculatePlumeCentre(const CString &specie, double &plumeCentre_alpha, double &plumeCentre_phi, double &plumeCompleteness, double &plumeEdge_low, double &plumeEdge_high){
	unsigned long i; // iterator
	m_plumeCentre[0]		= -999.0; // notify that the plume-centre position is unknown
	m_plumeCentre[1]		= -999.0; // notify that the plume-centre position is unknown
	m_plumeCompleteness		= -999.0;
	m_plumeEdge[0]			= -999.0;
	m_plumeEdge[1]			= -999.0;

	// if this is a wind-speed measurement, then there's no use to try to 
	//		calculate the plume-centre
	if(this->IsWindMeasurement())
		return false;

	// get the specie index
	int specieIndex = GetSpecieIndex(specie);
	if(specieIndex == -1){
		return false;
	}

	// pull out the good data points out of the measurement and ignore the bad points
	double *scanAngle		= new double[m_specNum];
	double *phi				= new double[m_specNum];
	double *column			= new double[m_specNum];
	double *columnError		= new double[m_specNum];
	bool	 *badEval		= new bool[m_specNum];
	for(i = 0; i < m_specNum; ++i){
		if(m_spec[i].IsBad() || m_spec[i].IsDeleted()){
			badEval[i] = true;
		}else{
			badEval[i]	  = false;
			scanAngle[i]  = m_specInfo[i].m_scanAngle;
			phi[i]		  = m_specInfo[i].m_scanAngle2;
			column[i]     = m_spec[i].m_ref[specieIndex].m_column;
			columnError[i]= m_spec[i].m_ref[specieIndex].m_columnError;
		}
	}

	// Calculate the centre of the plume
	bool ret = Common::FindPlume(scanAngle, phi, column, columnError, badEval, m_specNum, plumeCentre_alpha, plumeCentre_phi, plumeEdge_low, plumeEdge_high);

	if(ret){
		// Remember the calculated value of the plume centre
		m_plumeCentre[0]	= plumeCentre_alpha;
		m_plumeCentre[1]	= plumeCentre_phi;

		m_plumeEdge[0]		= plumeEdge_low;
		m_plumeEdge[1]		= plumeEdge_high;

		double offset = Common::CalculateOffset(column, badEval, m_specNum);

		// Estimate the completeness of the plume
		Common::CalculatePlumeCompleteness(scanAngle, phi, column, columnError, badEval, offset, m_specNum, plumeCompleteness);
		m_plumeCompleteness	= plumeCompleteness;

		// Also calculate the wind-direction
		CalculateWindDirection(plumeCentre_alpha);

		// The flux is probably ok
		m_flux.m_fluxOk = true;
	}else{
		// If there's no plume, then the flux is probably not very good
		m_flux.m_fluxOk = false;
	}

	delete[] scanAngle;
	delete[] phi;
	delete[] column;
	delete[] columnError;
	delete[] badEval;

	return ret;
}

/** Tries to calculate the local wind-direction when this scan was collected */
bool CScanResult::CalculateWindDirection(double plumeCentre_alpha, double plumeHeight){
	// The wind-direction is set to -999 if unknown
	m_windDirection = -999;

	// Get the GPS-position of the system
	double latitude		= GetLatitude();
	double longitude	= GetLongitude();

	// If the gps is unknown, we cannot calculate any wind-direction
	if(fabs(latitude) < 1e-2 && fabs(longitude) < 1e-2)
		return false;

	// Get the compass direction of the system
	double compass		= GetCompass();

	// Get the plume-height above the system
	CWindField wind;
	CDateTime startTime;
	this->GetStartTime(0, startTime);
	if(plumeHeight < 0){
		if(g_metData.GetWindField(GetSerial(), startTime, wind))
			wind = g_metData.defaultWindField; // <-- if the actual wind-field at the scanner is unknown, use the default values
	}else{
		wind.SetPlumeHeight(plumeHeight, MET_GEOMETRY_CALCULATION);
	}

	// the distance from the system to the intersection-point
	double intersectionDistance = wind.GetPlumeHeight() * tan(DEGREETORAD * plumeCentre_alpha);

	// the direction from the system to the intersection-point
	double angle = (compass - 90);

	// the intersection-point
	double lat2, lon2;
	Common common;
	common.CalculateDestination(latitude, longitude, intersectionDistance, angle, lat2, lon2);

	// the bearing from the volcano to the intersection-point

	// Get the nearest volcano
	int volcanoIndex = Geometry::CGeometryCalculator::GetNearestVolcano(latitude, longitude);
	if(volcanoIndex == -1)
		return false; // <-- no volcano could be found

	// get the volcanoes latitude & longitude
	double vLat = g_volcanoes.m_peakLatitude[volcanoIndex];
	double vLon = g_volcanoes.m_peakLongitude[volcanoIndex];

	// the wind-direction
	this->m_windDirection = common.GPSBearing(vLat, vLon, lat2, lon2);

	return true;
}

/** Calculates the maximum good column value in the scan, corrected for the offset */
double CScanResult::GetMaxColumn(const CString &specie) const{
	unsigned long i; // iterator
	double maxColumn = 0.0;

	// get the specie index
	int specieIndex = GetSpecieIndex(specie);
	if(specieIndex == -1){
		return 0.0;
	}

	// Go through the column values and pick out the highest
	for(i = 0; i < m_specNum; ++i){
		if(m_spec[i].IsBad() || m_spec[i].IsDeleted()){
			continue;
		}
		maxColumn = max(maxColumn, m_spec[i].m_ref[specieIndex].m_column - m_offset);
	}

	return maxColumn;
}

/** Returns the calculated plume edges */
void CScanResult::GetCalculatedPlumeEdges(double &lowEdge, double &highEdge) const{
	lowEdge		= m_plumeEdge[0];
	highEdge	= m_plumeEdge[1];
}

double CScanResult::GetColumn(unsigned long spectrumNum, unsigned long specieNum) const{
	return this->GetFitParameter(spectrumNum, specieNum, COLUMN);
}

double CScanResult::GetColumnError(unsigned long spectrumNum, unsigned long specieNum) const{
	return this->GetFitParameter(spectrumNum, specieNum, COLUMN_ERROR);
}

double CScanResult::GetShift(unsigned long spectrumNum, unsigned long specieNum) const{
	return this->GetFitParameter(spectrumNum, specieNum, SHIFT);
}

double CScanResult::GetShiftError(unsigned long spectrumNum, unsigned long specieNum) const{
	return this->GetFitParameter(spectrumNum, specieNum, SHIFT_ERROR);
}

double CScanResult::GetSqueeze(unsigned long spectrumNum, unsigned long specieNum) const{
	return this->GetFitParameter(spectrumNum, specieNum, SQUEEZE);
}

double CScanResult::GetSqueezeError(unsigned long spectrumNum, unsigned long specieNum) const{
	return this->GetFitParameter(spectrumNum, specieNum, SQUEEZE_ERROR);
}

/** @return the delta of the fit for spectrum number 'spectrumNum'
	  @param spectrumNum - the spectrum number (zero-based) for which the delta value is desired         */
double CScanResult::GetDelta(unsigned long spectrumNum) const{
	return this->m_spec[spectrumNum].m_delta;
}

/** @return the chi-square of the fit for spectrum number 'spectrumNum'
	  @param spectrumNum - the spectrum number (zero-based) for which the delta value is desired         */
double CScanResult::GetChiSquare(unsigned long spectrumNum) const{
	 return this->m_spec[spectrumNum].m_chiSquare;
}

/** Returns the desired fit parameter */
double CScanResult::GetFitParameter(unsigned long specIndex, unsigned long specieIndex, FIT_PARAMETER parameter) const{
	if(specIndex < 0 || specIndex > m_specNum)
		return 0.0f;

	if(specieIndex < 0 || specieIndex > this->m_spec[specIndex].m_speciesNum)
		return 0.0f;

	switch(parameter){
		case COLUMN:        return this->m_spec[specIndex].m_ref[specieIndex].m_column;
		case COLUMN_ERROR:  return this->m_spec[specIndex].m_ref[specieIndex].m_columnError;
		case SHIFT:         return this->m_spec[specIndex].m_ref[specieIndex].m_shift;
		case SHIFT_ERROR:   return this->m_spec[specIndex].m_ref[specieIndex].m_shiftError;
		case SQUEEZE:       return this->m_spec[specIndex].m_ref[specieIndex].m_squeeze;
		case SQUEEZE_ERROR: return this->m_spec[specIndex].m_ref[specieIndex].m_squeezeError;
		case DELTA:         return this->m_spec[specIndex].m_delta;
		default:            return 0.0f;
	}
}

const CSpectrumInfo &CScanResult::GetSpectrumInfo(unsigned long index) const {
	return m_specInfo[index];
}

/** Returns a reference to the spectrum info-structure of the sky-spectrum used */
const CSpectrumInfo &CScanResult::GetSkySpectrumInfo() const{
	return m_skySpecInfo;
}

/** Returns a reference to the spectrum info-structure of the dark-spectrum used */
const CSpectrumInfo &CScanResult::GetDarkSpectrumInfo() const{
	return m_darkSpecInfo;
}


/** Assignment operator */
CScanResult &CScanResult::operator=(const CScanResult &s2){
	// The calculated flux and offset
	this->m_flux    = s2.m_flux;
	this->m_offset  = s2.m_offset;

	// The errors
	m_geomError         = s2.m_geomError;
	m_scatteringError   = s2.m_scatteringError;
	m_spectroscopyError = s2.m_spectroscopyError;

	// The calculated wind-direction and plume-centre
	this->m_windDirection     = s2.m_windDirection;
	this->m_plumeCentre[0]    = s2.m_plumeCentre[0];
	this->m_plumeCentre[1]    = s2.m_plumeCentre[1];
	this->m_plumeEdge[0]      = s2.m_plumeEdge[0];
	this->m_plumeEdge[1]      = s2.m_plumeEdge[1];
	this->m_plumeCompleteness = s2.m_plumeCompleteness;

	this->m_spec.Copy(s2.m_spec);
	this->m_specInfo.Copy(s2.m_specInfo);
	this->m_corruptedSpectra.Copy(s2.m_corruptedSpectra);
	this->m_corruptedNum = s2.m_corruptedNum;
	this->m_specNum      = s2.m_specNum;

	this->m_skySpecInfo   = s2.m_skySpecInfo;
	this->m_darkSpecInfo  = s2.m_darkSpecInfo;

	this->m_measurementMode = s2.m_measurementMode;
	this->m_instrumentType  = s2.m_instrumentType;

	return *this;
}

/** Marks the desired spectrum with the supplied mark_flag.
    Mark flag must be MARK_BAD_EVALUATION, or MARK_DELETED
    @return SUCCESS on success. */
RETURN_CODE  CScanResult::MarkAs(unsigned long index, int MARK_FLAG){
	if(!IsValidSpectrumIndex(index))
		return FAIL;

	return m_spec[index].MarkAs(MARK_FLAG);
}

/** Removes the desired mark from the desired spectrum
    Mark flag must be MARK_BAD_EVALUATION, or MARK_DELETED
    @return SUCCESS on success. */
RETURN_CODE  CScanResult::RemoveMark(unsigned long index, int MARK_FLAG){
	if(!IsValidSpectrumIndex(index))
		return FAIL;

	return m_spec[index].RemoveMark(MARK_FLAG);
}

/** returns the date (UMT) when the evaluated spectrum number 'index'	was collected
	    @param index - the zero-based index into the list of evaluated spectra. */
RETURN_CODE CScanResult::GetDate(unsigned long index, unsigned short date[3]) const{
	if(!IsValidSpectrumIndex(index))
		return FAIL;

	date[0] = m_specInfo[index].m_date[0];
	date[1] = m_specInfo[index].m_date[1];
	date[2] = m_specInfo[index].m_date[2];

	return SUCCESS;
}

/** Returns the latitude of the system */
double	CScanResult::GetLatitude() const{
	for(unsigned int k = 0; k < m_specNum; ++k){
		const CSpectrumInfo &info = m_specInfo.GetAt(k);
		if(fabs(info.m_gps.m_latitude) > 1e-2)
			return info.m_gps.m_latitude;
	}
	return 0.0;
}

/** Returns the longitude of the system */
double	CScanResult::GetLongitude() const{
	for(unsigned int k = 0; k < m_specNum; ++k){
		const CSpectrumInfo &info = m_specInfo.GetAt(k);
		if(fabs(info.m_gps.m_longitude) > 1e-2)
			return info.m_gps.m_longitude;
	}
	return 0.0;
}
/** Returns the altitude of the system */
double	CScanResult::GetAltitude() const{
	for(unsigned int k = 0; k < m_specNum; ++k){
		const CSpectrumInfo &info = m_specInfo.GetAt(k);
		if(fabs(info.m_gps.m_altitude) > 1e-2)
			return info.m_gps.m_altitude;
	}
	return 0.0;
}

/** Returns the compass-direction of the system */
double	CScanResult::GetCompass() const{
	if(m_specNum == 0)
		return 0.0;

	const CSpectrumInfo &info = m_specInfo.GetAt(0);
	return info.m_compass;
}

/** Returns the battery-voltage of the sky spectrum */
float	CScanResult::GetBatteryVoltage() const{
	if(m_specNum == 0)
		return 0.0;

	const CSpectrumInfo &info = m_specInfo.GetAt(0);
	return info.m_batteryVoltage;
}

/** Returns the cone angle of the scanning instrument */
double	CScanResult::GetConeAngle() const{
	if(m_specNum == 0)
		return 90.0;

	const CSpectrumInfo &info = m_specInfo.GetAt(0);
	return info.m_coneAngle;
}

/** Returns the pitch of the scanning instrument */
double	CScanResult::GetPitch() const{
	if(m_specNum == 0)
		return 90.0;

	const CSpectrumInfo &info = m_specInfo.GetAt(0);
	return info.m_pitch; 
}

/** Returns the roll of the scanning instrument */
double	CScanResult::GetRoll() const{
	if(m_specNum == 0)
		return 90.0;

	const CSpectrumInfo &info = m_specInfo.GetAt(0);
	return info.m_roll; 
}

/** Returns the name of the requested spectrum */
CString CScanResult::GetName(int index) const{
	if(!IsValidSpectrumIndex(index))
		return CString("");

	const CSpectrumInfo &info = m_specInfo.GetAt(index);
	return info.m_name;
}

/** Returns the serial-number of the spectrometer that collected this scan */
CString CScanResult::GetSerial() const{
	for(unsigned int k = 0; k < m_specNum; ++k){
		const CSpectrumInfo &info = m_specInfo.GetAt(k);
		if(strlen(info.m_device) > 0)
			return info.m_device;
	}
	return CString("");	
}

/** Checks the kind of measurement that we have here and sets the flag 'm_measurementMode'
		to the appropriate value... */
MEASUREMENT_MODE CScanResult::CheckMeasurementMode(){
	if(IsStratosphereMeasurement()){
		m_measurementMode = MODE_STRATOSPHERE;
	}else if(IsWindMeasurement()){
		m_measurementMode = MODE_WINDSPEED;
	}else if(this->IsDirectSunMeasurement()){
		m_measurementMode = MODE_DIRECT_SUN;
	}else if(this->IsLunarMeasurement()){
		m_measurementMode = MODE_LUNAR;
	}else if(this->IsCompositionMeasurement()){
		m_measurementMode = MODE_COMPOSITION;
	}else{
		m_measurementMode = MODE_FLUX;
	}

	return m_measurementMode;
}

/** Returns true if this is a stratospheric mode measurement */
bool CScanResult::IsStratosphereMeasurement() const{
	double SZA, SAZ;
	CDateTime startTime;

	// Check so that the measurement is long enough, but not too long
	if(m_specNum < 3 || m_specNum > 50)
		return false;

	// Check if we've already checked the mode
	if(m_measurementMode == MODE_STRATOSPHERE)
		return true;

	// If the measurement started at a time when the Solar Zenith Angle 
	//	was larger than 75 degrees then it is not a wind-speed measurement
	this->GetStartTime(0, startTime);
	if(SUCCESS != Common::GetSunPosition(startTime, GetLatitude(), GetLongitude(), SZA, SAZ))
		return false; // error
	if(fabs(SZA) < 75.0)
		return false;

	// It is here assumed that the measurement is a stratospheric measurment
	//	if there are more than 3 repetitions in the zenith positon
	int nRepetitions	= 0; // <-- the number of repetitions in one position
	for(unsigned int k = 3; k < m_specNum; ++k){
		float pos = GetScanAngle(k);
		if(fabs(pos) < 1e-2)
			++nRepetitions;
		else{
			nRepetitions = 0;
		}

		if(nRepetitions > 3){
			return true;
		}
	}

	return false;
}

bool CScanResult::IsFluxMeasurement(){

	// Check if we've already checked the mode
	if(m_measurementMode == MODE_FLUX)
		return true;

	// Then check the measurement mode
	this->CheckMeasurementMode();

	if(m_measurementMode == MODE_FLUX){
		return true;
	}else{
		return false;
	}
}

bool CScanResult::IsWindMeasurement() const{
	if(this->IsWindMeasurement_Gothenburg())
		return true;
	if(this->IsWindMeasurement_Heidelberg())
		return true;

	return false;
}

bool CScanResult::IsWindMeasurement_Gothenburg() const{
	double SZA, SAZ;
	CDateTime startTime;

	// Check so that the measurement is long enough
	if(m_specNum < 52)
		return false;

	// Check if we've already checked the mode
	if(m_measurementMode == MODE_WINDSPEED)
		return true;

	// If the measurement started at a time when the Solar Zenith Angle 
	//	was larger than 85 degrees then it is not a wind-speed measurement
	this->GetStartTime(0, startTime);
	if(SUCCESS != Common::GetSunPosition(startTime, GetLatitude(), GetLongitude(), SZA, SAZ))
		return false; // error
	if(fabs(SZA) >= 85.0)
		return false;

	// Check if this is a wind-measurement in the Gothenburg method...
	int nRepetitions	= 0; // <-- the number of repetitions in one position
	float lastPos			= GetScanAngle(3);
	float lastPos2		= GetScanAngle2(3);

	// It is here assumed that the measurement is a wind speed measurment
	//	if there are more then 50 repetitions in one measurement positon
	for(unsigned int k = 4; k < m_specNum; ++k){
		float pos  = GetScanAngle(k);
		float pos2 = GetScanAngle2(k);
		if((fabs(pos - lastPos) < 1e-2) && (fabs(pos2 - lastPos2) < 1e-2))
			++nRepetitions;
		else{
			nRepetitions = 0;
			lastPos  = pos;
			lastPos2 = pos2;
		}

		if(nRepetitions > 50){
			return true;
		}
	}

	return false;
}

bool CScanResult::IsWindMeasurement_Heidelberg() const{
	double SAZ, SZA;
	CDateTime startTime;

	// Check so that the measurement is long enough
	if(m_specNum < 52)
		return false;

	// Check if we've already checked the mode
	if(m_measurementMode == MODE_WINDSPEED)
		return true;

	// Check if the channel-number is equal to 0
	if(m_specInfo[0].m_channel > 0)
		return false;

	// If the measurement started at a time when the Solar Zenith Angle 
	//	was larger than 75 degrees then it is not a wind-speed measurement
	this->GetStartTime(0, startTime);
	if(SUCCESS != Common::GetSunPosition(startTime, GetLatitude(), GetLongitude(), SZA, SAZ))
		return false; // error
	if(fabs(SZA) >= 75.0)
		return false;

	// Check if this is a wind-measurement in the Heidelberg method...
	int nRepetitions		= 0; // <-- the number of repetitions in one position
	float scanAngle[2]	= {GetScanAngle(3), GetScanAngle(4)};
	float scanAngle2[2]	= {GetScanAngle2(3),GetScanAngle2(4)};
	int		scanIndex			= 0;

	// It is here assumed that the measurement is a wind speed measurement
	//	if there are more then 50 repetitions in one measurement positon
	for(unsigned int k = 5; k < m_specNum; ++k){
		float pos = GetScanAngle(k);
		float pos2= GetScanAngle2(k);

		if((fabs(pos - scanAngle[scanIndex]) < 1e-2) && (fabs(pos2 - scanAngle2[scanIndex]) < 1e-2)){
			++nRepetitions;
			scanIndex = (scanIndex + 1) % 2;
		}else{
			return false;
		}

		if(nRepetitions > 50){
			return true;
		}
	}
	
	return false;
}

/** Returns true if this is a direct-sun mode measurement */
bool CScanResult::IsDirectSunMeasurement() const{
	int	nFound = 0;

	// It is here assumed that the measurement is a direct-sun measurment
	//	if there is at least 5 spectra with the name 'direct_sun'
	for(unsigned int k = 5; k < m_specNum; ++k){
		CString &name = GetName(k);
		if(Equals(name, "direct_sun")){
			++nFound;
			if(nFound == 5)
				return true;
		}
	}
	
	return false;
}

/** Returns true if this is a lunar mode measurement */
bool CScanResult::IsLunarMeasurement() const{
	int	nFound = 0;

	// It is here assumed that the measurement is a lunar measurment
	//	if there is at least 1 spectrum with the name 'lunar'
	for(unsigned int k = 5; k < m_specNum; ++k){
		CString &name = GetName(k);
		if(Equals(name, "lunar"))
			++nFound;
			if(nFound == 5)
				return true;
	}
	
	return false;
}

/** Returns true if this is a composition mode measurement */
bool CScanResult::IsCompositionMeasurement() const{
	// It is here assumed that the measurement is a composition measurment
	//	if there is 
	//		* at least 1 spectrum with the name 'comp'
	//		* no more than 15 spectra in total
	if(m_specNum > 15)
		return false;

	for(unsigned int k = 0; k < m_specNum; ++k){
		CString &name = GetName(k);
		if(Equals(name, "comp")){
			return true;
		}
	}

	return false;
}

/** returns the time and date (UMT) when evaluated spectrum number 'index' was started.
    @param index - the zero based index into the list of evaluated spectra */
RETURN_CODE CScanResult::GetStartTime(unsigned long index, CDateTime &t) const{
	if(!IsValidSpectrumIndex(index))
		return FAIL;

	// The start-time
	t.hour		= (unsigned char)m_specInfo[index].m_startTime.hr;
	t.minute	= (unsigned char)m_specInfo[index].m_startTime.m;
	t.second	= (unsigned char)m_specInfo[index].m_startTime.sec;

	// The date
	t.year		= m_specInfo[index].m_date[0];
	t.month		= (unsigned char)m_specInfo[index].m_date[1];
	t.day		= (unsigned char)m_specInfo[index].m_date[2];

	return SUCCESS;
}

void CScanResult::GetSkyStartTime(CDateTime &t) const{
	// The date
	t.year		= m_skySpecInfo.m_date[0];
	t.month		= (unsigned char)m_skySpecInfo.m_date[1];
	t.day		= (unsigned char)m_skySpecInfo.m_date[2];

	// The start-time
	t.hour		= (unsigned char)m_skySpecInfo.m_startTime.hr;
	t.minute	= (unsigned char)m_skySpecInfo.m_startTime.m;
	t.second	= (unsigned char)m_skySpecInfo.m_startTime.sec;
}

/** returns the time and date (UMT) when evaluated spectrum number 'index' was stopped.
    @param index - the zero based index into the list of evaluated spectra.
		@return SUCCESS if the index is valid */
RETURN_CODE CScanResult::GetStopTime(unsigned long index, CDateTime &t) const{
	if(!IsValidSpectrumIndex(index))
		return FAIL;

	// The start-time
	t.hour		= (unsigned char)m_specInfo[index].m_stopTime.hr;
	t.minute	= (unsigned char)m_specInfo[index].m_stopTime.m;
	t.second	= (unsigned char)m_specInfo[index].m_stopTime.sec;

	// The date
	t.year		= m_specInfo[index].m_date[0];
	t.month		= (unsigned char)m_specInfo[index].m_date[1];
	t.day		= (unsigned char)m_specInfo[index].m_date[2];

	return SUCCESS;
}

/** Sets the type of the instrument used */
void CScanResult::SetInstrumentType(INSTRUMENT_TYPE type){
	this->m_instrumentType = type;
}

/** Sets the type of the instrument used */
INSTRUMENT_TYPE CScanResult::GetInstrumentType() const{
	return this->m_instrumentType;
}

/** Getting the estimated geometrical error */
double	CScanResult::GetGeometricalError() const{
	return m_geomError;
}

/** Getting the scattering Error */
double	CScanResult::GetScatteringError() const{
	return m_scatteringError;
}

/** Getting the spectroscopical error */
double	CScanResult::GetSpectroscopicalError() const{
	return m_spectroscopyError;
}

/** Getting the estimated geometrical error */
void	CScanResult::SetGeometricalError(double err){
	m_geomError = err;
}

/** Getting the scattering Error */
void	CScanResult::SetScatteringError(double err){
	m_scatteringError = err;
}

/** Getting the spectroscopical error */
void	CScanResult::SetSpectroscopicalError(double err){
	m_spectroscopyError = err;
}

/** Applies the given correction to all or some evaluated references.
		@param corretionToApply - the correction to apply...
		@param parameters - The parameters for the corrections
		@param nParameters - the number of parameters (i.e. the length of the array 'parameters'
		@param specie - The name of the specie for which to apply the correction, if NULL then correction will be applied to all species.
		@return zero if all is ok, otherwise a non-zero value
*/
int CScanResult::ApplyCorrection(CORRECTION correctionToApply, double *parameters, long nParameters, CString *specie){
	unsigned int i, k;
	double f; // the correction factor

	for(i = 0; i < m_specNum; ++i){
		CEvaluationResult &evalRes = m_spec.GetAt(i); // a handle to the result.

		// Loop through all evaluated species
		for(k = 0; k < evalRes.m_speciesNum; ++k){
			// check if we should apply any correction to this specie
			if(specie != NULL){
				if(!Equals(evalRes.m_ref[k].m_specieName, *specie)){
					continue;
				}

				// Correct this column...
				f = CColumnCorrection::GetCorrectionFactor(correctionToApply, parameters, nParameters);
				evalRes.m_ref[k].m_column *= f;

				// Write the column-correction to the end of the list
				CColumnCorrection &corrList = evalRes.m_correction.GetAt(k);
				corrList.m_corrections.AddTail(correctionToApply);
				corrList.m_correctionValues.AddTail(f);
			}

		}
	}

	return 0;
}
