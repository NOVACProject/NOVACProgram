#include "StdAfx.h"
#include "ScanResult.h"
#include "../VolcanoInfo.h"
#include <SpectralEvaluation/Utils.h>
#include <SpectralEvaluation/Flux/PlumeInScanProperty.h>

// we also need the meterological data
#include "../MeteorologicalData.h"

#include "../Geometry/GeometryCalculator.h"

#undef min
#undef max

#include <algorithm>

using namespace Evaluation;

extern CMeteorologicalData g_metData;			// <-- The meteorological data
extern CVolcanoInfo					g_volcanoes;	// <-- A list of all known volcanoes


CScanResult::CScanResult(void)
{
    m_specNum = 0;
    m_offset = 0;
    m_flux.Clear();
    m_geomError = 30.0;	// best-case guess, 30%
    m_spectroscopyError = 15.0;	// best-case guess, 15%
    m_scatteringError = 30.0;	// best-case guess, 30%
    m_windDirection = -999.0;
    m_plumeCentre[0] = -999.0;
    m_plumeCentre[1] = -999.0;
    m_plumeEdge[0] = -999.0;
    m_plumeEdge[1] = -999.0;
    m_plumeCompleteness = -999.0;
    m_measurementMode = MODE_UNKNOWN;
}

CScanResult::CScanResult(const CScanResult& other)
{
    // The calculated flux and offset
    this->m_flux = other.m_flux;
    this->m_offset = other.m_offset;

    // The errors
    m_geomError = other.m_geomError;
    m_scatteringError = other.m_scatteringError;
    m_spectroscopyError = other.m_spectroscopyError;

    // The calculated wind-direction and plume-centre
    this->m_windDirection = other.m_windDirection;
    this->m_plumeCentre[0] = other.m_plumeCentre[0];
    this->m_plumeCentre[1] = other.m_plumeCentre[1];
    this->m_plumeEdge[0] = other.m_plumeEdge[0];
    this->m_plumeEdge[1] = other.m_plumeEdge[1];
    this->m_plumeCompleteness = other.m_plumeCompleteness;

    this->m_spec                = std::vector<CEvaluationResult>(begin(other.m_spec), end(other.m_spec));
    this->m_specInfo            = std::vector<CSpectrumInfo>(begin(other.m_specInfo), end(other.m_specInfo));
    this->m_corruptedSpectra    = std::vector<unsigned int>(begin(other.m_corruptedSpectra), end(other.m_corruptedSpectra));
    this->m_specNum             = other.m_specNum;

    this->m_skySpecInfo = other.m_skySpecInfo;
    this->m_darkSpecInfo = other.m_darkSpecInfo;
    this->m_offsetSpecInfo = other.m_offsetSpecInfo;
    this->m_darkCurSpecInfo = other.m_darkCurSpecInfo;

    this->m_measurementMode = other.m_measurementMode;
}

CScanResult::~CScanResult(void)
{
    m_spec.clear();
    m_specInfo.clear();
    m_corruptedSpectra.clear();
}

/** Intializes the memory arrays to have, initially, space for
        'specNum' spectra. */
void CScanResult::InitializeArrays(long specNum) {
    if (specNum < 0 || specNum > 1024) {
        return;
    }

    m_spec.reserve(specNum);
    m_specInfo.reserve(specNum);
}

/** Appends the result to the list of calculated results */
int CScanResult::AppendResult(const CEvaluationResult &evalRes, const CSpectrumInfo &specInfo) {

    // Append the evaluationresult to the end of the 'm_spec'-vector
    m_spec.push_back(CEvaluationResult(evalRes));

    // Append the spectral information to the end of the 'm_specInfo'-vector
    m_specInfo.push_back(CSpectrumInfo(specInfo));

    // Increase the numbers of spectra in this result-set.
    ++m_specNum;
    return 0;
}

bool CScanResult::GetResult(unsigned int specIndex, CEvaluationResult& result) const
{
    if (specIndex >= m_specNum)
        return false; // not a valid index

    result = m_spec[specIndex];
    return true;
}

/** Adds spectrum number 'specIndex' into the list of spectra in the .pak -file
        which are corrupted and could not be evaluated */
void CScanResult::MarkAsCorrupted(unsigned int specIndex)
{
    m_corruptedSpectra.push_back(specIndex);
}

int CScanResult::GetCorruptedNum() const
{
    return (int)m_corruptedSpectra.size();
}

/** Removes the spectrum number 'specIndex' from the list of calcualted results */
int CScanResult::RemoveResult(unsigned int specIndex)
{
    if (specIndex >= m_specNum)
        return 1; // not a valid index

    // Remove the desired value
    m_specInfo.erase(begin(m_specInfo) + specIndex);

    // Decrease the number of values in the list
    m_specNum -= 1;

    return 0;
}

/** Stores the information about the sky-spectrum used */
void CScanResult::SetSkySpecInfo(const CSpectrumInfo &skySpecInfo) {
    this->m_skySpecInfo = skySpecInfo;
}

/** Stores the information about the dark-spectrum used */
void CScanResult::SetDarkSpecInfo(const CSpectrumInfo &darkSpecInfo) {
    this->m_darkSpecInfo = darkSpecInfo;
}

/** Stores the information about the offset-spectrum used */
void CScanResult::SetOffsetSpecInfo(const CSpectrumInfo &offsetSpecInfo) {
    this->m_offsetSpecInfo = offsetSpecInfo;
}

/** Stores the information about the dark-current-spectrum used */
void CScanResult::SetDarkCurrentSpecInfo(const CSpectrumInfo &darkCurSpecInfo) {
    this->m_darkCurSpecInfo = darkCurSpecInfo;
}

/** Check the last spectrum point for goodness of fit */
bool CScanResult::CheckGoodnessOfFit(const CSpectrumInfo& info, float chi2Limit, float upperLimit, float lowerLimit) {
    return CheckGoodnessOfFit(info, m_specNum - 1, chi2Limit, upperLimit, lowerLimit);
}

/** Check spectrum number 'index' for goodness of fit */
bool CScanResult::CheckGoodnessOfFit(const CSpectrumInfo& info, int index, float chi2Limit, float upperLimit, float lowerLimit) {
    if (index < 0 || (unsigned int)index >= m_specNum)
        return false;

    // remember the electronic offset (NB. this is not same as the scan-offset)
//  m_specInfo[index].m_offset				= (float)offsetLevel;

    return m_spec[index].CheckGoodnessOfFit(info, chi2Limit, upperLimit, lowerLimit);
}

int CScanResult::CalculateOffset(const std::string &specie)
{
    if (m_specNum < 0)
        return 1;

    // Get the index for the specie for which we want to calculate the offset
    long specieIndex = GetSpecieIndex(specie);
    if (specieIndex == -1) { // if the specified specie does not exist
        return 1;
    }

    std::vector<double> columns(m_specNum);
    std::vector<bool> badEval(m_specNum);

    // We then need to rearrange the column data a little bit. 
    for (unsigned long i = 0; i < m_specNum; ++i) {
        columns[i] = m_spec[i].m_referenceResult[specieIndex].m_column;

        // The spectrum is considered as bad if the goodness-of-fit checking
        //	has marked it as bad or the user has marked it as deleted
        if (m_spec[i].IsBad() || m_spec[i].IsDeleted()) {
            badEval[i] = true;
        }
        else {
            badEval[i] = false;
        }
    }

    // Calculate the offset
    this->m_offset = CalculatePlumeOffset(columns, badEval, m_specNum);

    return 0;
}

int CScanResult::GetSpecieIndex(const std::string &specie) const
{
    if (m_specNum <= 0) // <-- if there are no spectra, there can be no species
        return -1;

    // if there's only one specie, assume that this is the correct one
    if (m_spec[0].m_referenceResult.size() == 1)
    {
        return 0;
    }

    // find the index of the interesting specie
    for (size_t i = 0; i < m_spec[0].m_referenceResult.size(); ++i)
    {
        if (EqualsIgnoringCase(m_spec[0].m_referenceResult[i].m_specieName, specie))
        {
            return i;
        }
    }

    return -1;
}

int CScanResult::CalculateFlux(const std::string &specie, const CWindField &wind, double compass, double coneAngle, double tilt) {

    // If this is a not a flux measurement, then don't calculate any flux
    if (!IsFluxMeasurement())
        return 1;

    // get the specie index
    int specieIndex = GetSpecieIndex(specie);
    if (specieIndex == -1) {
        return 1;
    }

    // get the gas factor
    double gasFactor = Common::GetGasFactor(CString(specie.c_str()));
    if (gasFactor == -1) {
        // spec.m_logFileHandler.WriteErrorMessage(TEXT("No GasFactor defined for: " + specie));
        return 1;
    }

    // pull out the good data points out of the measurement and ignore the bad points
    double *scanAngle = new double[m_specNum];
    double *scanAngle2 = new double[m_specNum];
    double *column = new double[m_specNum];
    unsigned int nDataPoints = 0;
    for (unsigned long i = 0; i < m_specNum; ++i) {
        if (m_spec[i].IsBad() || m_spec[i].IsDeleted())
            continue; // this is a bad measurement
        if (m_specInfo[i].m_flag >= 64)
            continue; // this is a direct-sun measurement, don't use it to calculate the flux...
        if (EqualsIgnoringCase(m_specInfo[i].m_name, "direct_sun", 10) ||
            EqualsIgnoringCase(m_specInfo[i].m_name, "direct_moon", 11) ||
            EqualsIgnoringCase(m_specInfo[i].m_name, "sun_", 4) ||
            EqualsIgnoringCase(m_specInfo[i].m_name, "moon_", 5) ||
            EqualsIgnoringCase(m_specInfo[i].m_name, "sky", 3) ||
            EqualsIgnoringCase(m_specInfo[i].m_name, "strat", 5) ||
            EqualsIgnoringCase(m_specInfo[i].m_name, "trop", 4) ||
            EqualsIgnoringCase(m_specInfo[i].m_name, "maxdoas", 7) ||
            EqualsIgnoringCase(m_specInfo[i].m_name, "special", 7) ||
            EqualsIgnoringCase(m_specInfo[i].m_name, "wind", 4) ||
            EqualsIgnoringCase(m_specInfo[i].m_name, "dark", 4) ||
            EqualsIgnoringCase(m_specInfo[i].m_name, "dark_cur", 8) ||
            EqualsIgnoringCase(m_specInfo[i].m_name, "offset", 6)) {
            continue; // this is not intended to be used for calculating the flux
        }

        scanAngle[nDataPoints]  = m_specInfo[i].m_scanAngle;
        scanAngle2[nDataPoints] = m_specInfo[i].m_scanAngle2;
        column[nDataPoints]     = m_spec[i].m_referenceResult[specieIndex].m_column;
        ++nDataPoints;
    }

    // if there are no good datapoints in the measurement, the flux is assumed to be zero
    if (nDataPoints < 10) {
        delete[] scanAngle;
        delete[] scanAngle2;
        delete[] column;
        m_flux.Clear();
        if (nDataPoints == 0)
            ShowMessage("Could not calculate flux, no good datapoints in measurement");
        else
            ShowMessage("Could not calculate flux, too few good datapoints in measurement");
        return 1;
    }

    // Calculate the flux
    m_flux.m_flux = Common::CalculateFlux(scanAngle, scanAngle2, column, m_offset, nDataPoints, wind, compass, gasFactor, coneAngle, tilt);
    m_flux.m_windDirection = wind.GetWindDirection();
    m_flux.m_windDirectionSource = wind.GetWindDirectionSource();
    m_flux.m_windSpeed = wind.GetWindSpeed();
    m_flux.m_windSpeedSource = wind.GetWindSpeedSource();
    m_flux.m_plumeHeight = wind.GetPlumeHeight();
    m_flux.m_plumeHeightSource = wind.GetPlumeHeightSource();
    m_flux.m_compass = compass;
    m_flux.m_coneAngle = coneAngle;
    m_flux.m_tilt = tilt;
    GetStartTime(0, m_flux.m_startTime);

    delete[] scanAngle;
    delete[] scanAngle2;
    delete[] column;

    return 0;
}

/** Tries to find a plume in the last scan result. If the plume is found
        this function returns true. The result of the calculations is stored in
        the member-variables 'm_plumeCentre', 'm_plumeCompleteness' and m_plumeEdge[0] and m_plumeEdge[1] */
bool CScanResult::CalculatePlumeCentre(const std::string &specie) {
    double plumeCentre_alpha, plumeCentre_phi, plumeCompleteness;
    return CalculatePlumeCentre(specie, plumeCentre_alpha, plumeCentre_phi, plumeCompleteness, m_plumeEdge[0], m_plumeEdge[1]);
}

/** Tries to find a plume in the last scan result. If the plume is found
        this function returns true, and the centre of the plume (in scanAngles)
        is given in 'plumeCentre' */
bool CScanResult::CalculatePlumeCentre(const std::string &specie, double &plumeCentre_alpha, double &plumeCentre_phi, double &plumeCompleteness, double &plumeEdge_low, double &plumeEdge_high) {
    m_plumeCentre[0] = -999.0; // notify that the plume-centre position is unknown
    m_plumeCentre[1] = -999.0; // notify that the plume-centre position is unknown
    m_plumeCompleteness = -999.0;
    m_plumeEdge[0] = -999.0;
    m_plumeEdge[1] = -999.0;

    // if this is a wind-speed measurement, then there's no use to try to 
    //		calculate the plume-centre
    if (this->IsWindMeasurement())
        return false;

    // get the specie index
    int specieIndex = GetSpecieIndex(specie);
    if (specieIndex == -1) {
        return false;
    }

    // pull out the good data points out of the measurement and ignore the bad points
    std::vector<double> scanAngle(m_specNum);
    std::vector<double> phi(m_specNum);
    std::vector<double> column(m_specNum);
    std::vector<double> columnError(m_specNum);
    std::vector<bool> badEval(m_specNum);

    for (unsigned long i = 0; i < m_specNum; ++i) {
        if (m_spec[i].IsBad() || m_spec[i].IsDeleted()) {
            badEval[i] = true;
        }
        else {
            badEval[i] = false;
            scanAngle[i] = m_specInfo[i].m_scanAngle;
            phi[i] = m_specInfo[i].m_scanAngle2;
            column[i] = m_spec[i].m_referenceResult[specieIndex].m_column;
            columnError[i] = m_spec[i].m_referenceResult[specieIndex].m_columnError;
        }
    }

    // Calculate the centre of the plume
    CPlumeInScanProperty plumeProperties;
    bool ret = FindPlume(scanAngle, phi, column, columnError, badEval, m_specNum, plumeProperties);

    if (ret) {
        // Remember the calculated value of the plume centre
        m_plumeCentre[0] = plumeProperties.plumeCenter;
        m_plumeCentre[1] = plumeProperties.plumeCenter2;
        plumeCentre_alpha = plumeProperties.plumeCenter;
        plumeCentre_phi   = plumeProperties.plumeCenter2;

        m_plumeEdge[0] = plumeProperties.plumeEdgeLow;
        m_plumeEdge[1] = plumeProperties.plumeEdgeHigh;
        plumeEdge_low = plumeProperties.plumeEdgeLow;
        plumeEdge_high = plumeProperties.plumeEdgeHigh;

        double offset = CalculatePlumeOffset(column, badEval, m_specNum);

        // Estimate the completeness of the plume
        CalculatePlumeCompleteness(scanAngle, phi, column, columnError, badEval, offset, m_specNum, plumeProperties);
        m_plumeCompleteness = plumeProperties.completeness;
        plumeCompleteness = plumeProperties.completeness;

        // Also calculate the wind-direction
        CalculateWindDirection(plumeCentre_alpha);

        // The flux is probably ok
        m_flux.m_fluxOk = true;
    }
    else {
        // If there's no plume, then the flux is probably not very good
        m_flux.m_fluxOk = false;
    }

    return ret;
}

/** Tries to calculate the local wind-direction when this scan was collected */
bool CScanResult::CalculateWindDirection(double plumeCentre_alpha, double plumeHeight) {
    // The wind-direction is set to -999 if unknown
    m_windDirection = -999;

    // Get the GPS-position of the system
    double latitude = GetLatitude();
    double longitude = GetLongitude();

    // If the gps is unknown, we cannot calculate any wind-direction
    if (fabs(latitude) < 1e-2 && fabs(longitude) < 1e-2)
        return false;

    // Get the compass direction of the system
    double compass = GetCompass();

    // Get the plume-height above the system
    CWindField wind;
    CDateTime startTime;
    this->GetStartTime(0, startTime);
    if (plumeHeight < 0) {
        if (g_metData.GetWindField(CString(GetSerial().c_str()), startTime, wind))
            wind = g_metData.defaultWindField; // <-- if the actual wind-field at the scanner is unknown, use the default values
    }
    else {
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
    if (volcanoIndex == -1)
        return false; // <-- no volcano could be found

    // get the volcanoes latitude & longitude
    double vLat = g_volcanoes.m_peakLatitude[volcanoIndex];
    double vLon = g_volcanoes.m_peakLongitude[volcanoIndex];

    // the wind-direction
    this->m_windDirection = common.GPSBearing(vLat, vLon, lat2, lon2);

    return true;
}

/** Calculates the maximum good column value in the scan, corrected for the offset */
double CScanResult::GetMaxColumn(const std::string &specie) const {
    double maxColumn = 0.0;

    // get the specie index
    int specieIndex = GetSpecieIndex(specie);
    if (specieIndex == -1) {
        return 0.0;
    }

    // Go through the column values and pick out the highest
    for (unsigned long i = 0; i < m_specNum; ++i) {
        if (m_spec[i].IsBad() || m_spec[i].IsDeleted()) {
            continue;
        }
        maxColumn = std::max(maxColumn, m_spec[i].m_referenceResult[specieIndex].m_column - m_offset);
    }

    return maxColumn;
}

/** Returns the calculated plume edges */
void CScanResult::GetCalculatedPlumeEdges(double &lowEdge, double &highEdge) const {
    lowEdge = m_plumeEdge[0];
    highEdge = m_plumeEdge[1];
}

double CScanResult::GetColumn(unsigned long spectrumNum, unsigned long specieNum) const {
    return this->GetFitParameter(spectrumNum, specieNum, COLUMN);
}

double CScanResult::GetColumnError(unsigned long spectrumNum, unsigned long specieNum) const {
    return this->GetFitParameter(spectrumNum, specieNum, COLUMN_ERROR);
}

double CScanResult::GetShift(unsigned long spectrumNum, unsigned long specieNum) const {
    return this->GetFitParameter(spectrumNum, specieNum, SHIFT);
}

double CScanResult::GetShiftError(unsigned long spectrumNum, unsigned long specieNum) const {
    return this->GetFitParameter(spectrumNum, specieNum, SHIFT_ERROR);
}

double CScanResult::GetSqueeze(unsigned long spectrumNum, unsigned long specieNum) const {
    return this->GetFitParameter(spectrumNum, specieNum, SQUEEZE);
}

double CScanResult::GetSqueezeError(unsigned long spectrumNum, unsigned long specieNum) const {
    return this->GetFitParameter(spectrumNum, specieNum, SQUEEZE_ERROR);
}

/** @return the delta of the fit for spectrum number 'spectrumNum'
      @param spectrumNum - the spectrum number (zero-based) for which the delta value is desired         */
double CScanResult::GetDelta(unsigned long spectrumNum) const {
    return this->m_spec[spectrumNum].m_delta;
}

/** @return the chi-square of the fit for spectrum number 'spectrumNum'
      @param spectrumNum - the spectrum number (zero-based) for which the delta value is desired         */
double CScanResult::GetChiSquare(unsigned long spectrumNum) const {
    return this->m_spec[spectrumNum].m_chiSquare;
}

/** Returns the desired fit parameter */
double CScanResult::GetFitParameter(unsigned long specIndex, unsigned long specieIndex, FIT_PARAMETER parameter) const {
    if (specIndex < 0 || specIndex > m_specNum)
        return 0.0;

    if (specieIndex < 0 || specieIndex > this->m_spec[specIndex].m_referenceResult.size())
        return 0.0;

    switch (parameter) {
        case COLUMN:        return this->m_spec[specIndex].m_referenceResult[specieIndex].m_column;
        case COLUMN_ERROR:  return this->m_spec[specIndex].m_referenceResult[specieIndex].m_columnError;
        case SHIFT:         return this->m_spec[specIndex].m_referenceResult[specieIndex].m_shift;
        case SHIFT_ERROR:   return this->m_spec[specIndex].m_referenceResult[specieIndex].m_shiftError;
        case SQUEEZE:       return this->m_spec[specIndex].m_referenceResult[specieIndex].m_squeeze;
        case SQUEEZE_ERROR: return this->m_spec[specIndex].m_referenceResult[specieIndex].m_squeezeError;
        case DELTA:         return this->m_spec[specIndex].m_delta;
        default:            return 0.0;
    }
}

const CSpectrumInfo &CScanResult::GetSpectrumInfo(unsigned long index) const {
    return m_specInfo[index];
}

/** Returns a reference to the spectrum info-structure of the sky-spectrum used */
const CSpectrumInfo &CScanResult::GetSkySpectrumInfo() const {
    return m_skySpecInfo;
}

/** Returns a reference to the spectrum info-structure of the dark-spectrum used */
const CSpectrumInfo &CScanResult::GetDarkSpectrumInfo() const {
    return m_darkSpecInfo;
}


/** Assignment operator */
CScanResult &CScanResult::operator=(const CScanResult &s2) {
    // The calculated flux and offset
    this->m_flux = s2.m_flux;
    this->m_offset = s2.m_offset;

    // The errors
    m_geomError = s2.m_geomError;
    m_scatteringError = s2.m_scatteringError;
    m_spectroscopyError = s2.m_spectroscopyError;

    // The calculated wind-direction and plume-centre
    this->m_windDirection = s2.m_windDirection;
    this->m_plumeCentre[0] = s2.m_plumeCentre[0];
    this->m_plumeCentre[1] = s2.m_plumeCentre[1];
    this->m_plumeEdge[0] = s2.m_plumeEdge[0];
    this->m_plumeEdge[1] = s2.m_plumeEdge[1];
    this->m_plumeCompleteness = s2.m_plumeCompleteness;

    this->m_spec                = std::vector<CEvaluationResult>(begin(s2.m_spec), end(s2.m_spec));
    this->m_specInfo            = std::vector<CSpectrumInfo>(begin(s2.m_specInfo), end(s2.m_specInfo));
    this->m_corruptedSpectra    = std::vector<unsigned int>(begin(s2.m_corruptedSpectra), end(s2.m_corruptedSpectra));
    this->m_specNum             = s2.m_specNum;

    this->m_skySpecInfo = s2.m_skySpecInfo;
    this->m_darkSpecInfo = s2.m_darkSpecInfo;
    this->m_offsetSpecInfo = s2.m_offsetSpecInfo;
    this->m_darkCurSpecInfo = s2.m_darkCurSpecInfo;

    this->m_measurementMode = s2.m_measurementMode;

    return *this;
}

bool CScanResult::MarkAs(unsigned long index, int MARK_FLAG) {
    if (!IsValidSpectrumIndex(index))
        return false;

    return m_spec[index].MarkAs(MARK_FLAG);
}

bool  CScanResult::RemoveMark(unsigned long index, int MARK_FLAG) {
    if (!IsValidSpectrumIndex(index))
        return false;

    return m_spec[index].RemoveMark(MARK_FLAG);
}

/** returns the date (UMT) when the evaluated spectrum number 'index'	was collected
        @param index - the zero-based index into the list of evaluated spectra. */
bool CScanResult::GetDate(unsigned long index, unsigned short date[3]) const {
    if (!IsValidSpectrumIndex(index))
        return false;

    date[0] = m_specInfo[index].m_startTime.year;
    date[1] = m_specInfo[index].m_startTime.month;
    date[2] = m_specInfo[index].m_startTime.day;

    return true;
}

/** Returns the latitude of the system */
double	CScanResult::GetLatitude() const {
    for (unsigned long k = 0; k < m_specNum; ++k) {
        const CSpectrumInfo &info = m_specInfo.at(k);
        if (fabs(info.m_gps.m_latitude) > 1e-2)
            return info.m_gps.m_latitude;
    }
    return 0.0;
}

/** Returns the longitude of the system */
double	CScanResult::GetLongitude() const {
    for (unsigned long k = 0; k < m_specNum; ++k) {
        const CSpectrumInfo &info = m_specInfo.at(k);
        if (fabs(info.m_gps.m_longitude) > 1e-2)
            return info.m_gps.m_longitude;
    }
    return 0.0;
}
/** Returns the altitude of the system */
double	CScanResult::GetAltitude() const {
    for (unsigned long k = 0; k < m_specNum; ++k) {
        const CSpectrumInfo &info = m_specInfo.at(k);
        if (fabs(info.m_gps.m_altitude) > 1e-2)
            return info.m_gps.m_altitude;
    }
    return 0.0;
}

/** Returns the compass-direction of the system */
double	CScanResult::GetCompass() const {
    if (m_specNum == 0)
        return 0.0;

    const CSpectrumInfo &info = m_specInfo.at(0);
    return info.m_compass;
}

/** Returns the battery-voltage of the sky spectrum */
float	CScanResult::GetBatteryVoltage() const {
    if (m_specNum == 0)
        return 0.0;

    const CSpectrumInfo &info = m_specInfo.at(0);
    return info.m_batteryVoltage;
}

/** Returns the cone angle of the scanning instrument */
double	CScanResult::GetConeAngle() const {
    if (m_specNum == 0)
        return 90.0;

    const CSpectrumInfo &info = m_specInfo.at(0);
    return info.m_coneAngle;
}

/** Returns the pitch of the scanning instrument */
double	CScanResult::GetPitch() const {
    if (m_specNum == 0)
        return 90.0;

    const CSpectrumInfo &info = m_specInfo.at(0);
    return info.m_pitch;
}

/** Returns the roll of the scanning instrument */
double	CScanResult::GetRoll() const {
    if (m_specNum == 0)
        return 90.0;

    const CSpectrumInfo &info = m_specInfo.at(0);
    return info.m_roll;
}

/** Returns the name of the requested spectrum */
std::string CScanResult::GetName(int index) const {
    if (!IsValidSpectrumIndex(index))
        return "";

    const CSpectrumInfo &info = m_specInfo.at(index);
    return info.m_name;
}

/** Returns the serial-number of the spectrometer that collected this scan */
std::string CScanResult::GetSerial() const {
    for (unsigned long k = 0; k < m_specNum; ++k) {
        const CSpectrumInfo &info = m_specInfo.at(k);
        if (info.m_device.size() > 0)
            return info.m_device;
    }
    return "";
}

/** Checks the kind of measurement that we have here and sets the flag 'm_measurementMode'
        to the appropriate value... */
MEASUREMENT_MODE CScanResult::CheckMeasurementMode() {
    if (IsStratosphereMeasurement()) {
        m_measurementMode = MODE_STRATOSPHERE;
    }
    else if (IsWindMeasurement()) {
        m_measurementMode = MODE_WINDSPEED;
    }
    else if (this->IsDirectSunMeasurement()) {
        m_measurementMode = MODE_DIRECT_SUN;
    }
    else if (this->IsLunarMeasurement()) {
        m_measurementMode = MODE_LUNAR;
    }
    else if (this->IsCompositionMeasurement()) {
        m_measurementMode = MODE_COMPOSITION;
    }
    else {
        m_measurementMode = MODE_FLUX;
    }

    return m_measurementMode;
}

/** Returns true if this is a stratospheric mode measurement */
bool CScanResult::IsStratosphereMeasurement() const {
    double SZA, SAZ;
    CDateTime startTime;

    // Check so that the measurement is long enough, but not too long
    if (m_specNum < 3 || m_specNum > 50)
        return false;

    // Check if we've already checked the mode
    if (m_measurementMode == MODE_STRATOSPHERE)
        return true;

    // If the measurement started at a time when the Solar Zenith Angle 
    //	was larger than 75 degrees then it is not a wind-speed measurement
    this->GetStartTime(0, startTime);
    if (SUCCESS != Common::GetSunPosition(startTime, GetLatitude(), GetLongitude(), SZA, SAZ))
        return false; // error
    if (fabs(SZA) < 75.0)
        return false;

    // It is here assumed that the measurement is a stratospheric measurment
    //	if there are more than 3 repetitions in the zenith positon
    int nRepetitions = 0; // <-- the number of repetitions in one position
    for (unsigned long k = 3; k < m_specNum; ++k) {
        float pos = GetScanAngle(k);
        if (fabs(pos) < 1e-2)
            ++nRepetitions;
        else {
            nRepetitions = 0;
        }

        if (nRepetitions > 3) {
            return true;
        }
    }

    return false;
}

bool CScanResult::IsFluxMeasurement() {

    // Check if we've already checked the mode
    if (m_measurementMode == MODE_FLUX)
        return true;

    // Then check the measurement mode
    this->CheckMeasurementMode();

    if (m_measurementMode == MODE_FLUX) {
        return true;
    }
    else {
        return false;
    }
}

bool CScanResult::IsWindMeasurement() const {
    double SZA, SAZ;
    CDateTime startTime;

    // Check so that the measurement is long enough
    if (m_specNum < 52)
        return false;

    // Check if we've already checked the mode
    if (m_measurementMode == MODE_WINDSPEED)
        return true;

    // If the measurement started at a time when the Solar Zenith Angle 
    //	was larger than 85 degrees then it is not a wind-speed measurement
    this->GetStartTime(0, startTime);
    if (SUCCESS != Common::GetSunPosition(startTime, GetLatitude(), GetLongitude(), SZA, SAZ))
        return false; // error
    if (fabs(SZA) >= 85.0)
        return false;

    // Check if this is a wind-measurement in the Gothenburg method...
    int nRepetitions = 0; // <-- the number of repetitions in one position
    float lastPos = GetScanAngle(3);
    float lastPos2 = GetScanAngle2(3);

    // It is here assumed that the measurement is a wind speed measurment
    //	if there are more then 50 repetitions in one measurement positon
    for (unsigned long k = 4; k < m_specNum; ++k) {
        float pos = GetScanAngle(k);
        float pos2 = GetScanAngle2(k);
        if ((fabs(pos - lastPos) < 1e-2) && (fabs(pos2 - lastPos2) < 1e-2))
            ++nRepetitions;
        else {
            nRepetitions = 0;
            lastPos = pos;
            lastPos2 = pos2;
        }

        if (nRepetitions > 50) {
            return true;
        }
    }

    return false;
}

/** Returns true if this is a direct-sun mode measurement */
bool CScanResult::IsDirectSunMeasurement() const {
    int	nFound = 0;

    // It is here assumed that the measurement is a direct-sun measurment
    //	if there is at least 5 spectra with the name 'direct_sun'
    for (unsigned long k = 5; k < m_specNum; ++k) {
        const std::string name = GetName(k);
        if (EqualsIgnoringCase(name, "direct_sun")) {
            ++nFound;
            if (nFound == 5)
                return true;
        }
    }

    return false;
}

/** Returns true if this is a lunar mode measurement */
bool CScanResult::IsLunarMeasurement() const {
    int	nFound = 0;

    // It is here assumed that the measurement is a lunar measurment
    //	if there is at least 1 spectrum with the name 'lunar'
    for (unsigned long k = 5; k < m_specNum; ++k) {
        const std::string name = GetName(k);
        if (EqualsIgnoringCase(name, "lunar"))
            ++nFound;
        if (nFound == 5)
            return true;
    }

    return false;
}

/** Returns true if this is a composition mode measurement */
bool CScanResult::IsCompositionMeasurement() const {
    // It is here assumed that the measurement is a composition measurment
    //	if there is 
    //		* at least 1 spectrum with the name 'comp'
    //		* no more than 15 spectra in total
    if (m_specNum > 15)
        return false;

    for (unsigned long k = 0; k < m_specNum; ++k) {
        const std::string name = GetName(k);
        if (EqualsIgnoringCase(name, "comp")) {
            return true;
        }
    }

    return false;
}

bool CScanResult::GetStartTime(unsigned long index, CDateTime &t) const {
    if (!IsValidSpectrumIndex(index))
        return false;

    // The start-time
    t.hour = (unsigned char)m_specInfo[index].m_startTime.hour;
    t.minute = (unsigned char)m_specInfo[index].m_startTime.minute;
    t.second = (unsigned char)m_specInfo[index].m_startTime.second;

    // The date
    t.year = m_specInfo[index].m_startTime.year;
    t.month = (unsigned char)m_specInfo[index].m_startTime.month;
    t.day = (unsigned char)m_specInfo[index].m_startTime.day;

    return true;
}

void CScanResult::GetSkyStartTime(CDateTime &t) const {
    // The date
    t.year = m_skySpecInfo.m_startTime.year;
    t.month = (unsigned char)m_skySpecInfo.m_startTime.month;
    t.day = (unsigned char)m_skySpecInfo.m_startTime.day;

    // The start-time
    t.hour = (unsigned char)m_skySpecInfo.m_startTime.hour;
    t.minute = (unsigned char)m_skySpecInfo.m_startTime.minute;
    t.second = (unsigned char)m_skySpecInfo.m_startTime.second;
}

bool CScanResult::GetStopTime(unsigned long index, CDateTime &t) const {
    if (!IsValidSpectrumIndex(index))
        return false;

    // The start-time
    t.hour = (unsigned char)m_specInfo[index].m_stopTime.hour;
    t.minute = (unsigned char)m_specInfo[index].m_stopTime.minute;
    t.second = (unsigned char)m_specInfo[index].m_stopTime.second;

    // The date
    t.year = m_specInfo[index].m_stopTime.year;
    t.month = (unsigned char)m_specInfo[index].m_stopTime.month;
    t.day = (unsigned char)m_specInfo[index].m_stopTime.day;

    return true;
}

/** Getting the estimated geometrical error */
double	CScanResult::GetGeometricalError() const {
    return m_geomError;
}

/** Getting the scattering Error */
double	CScanResult::GetScatteringError() const {
    return m_scatteringError;
}

/** Getting the spectroscopical error */
double	CScanResult::GetSpectroscopicalError() const {
    return m_spectroscopyError;
}

/** Getting the estimated geometrical error */
void	CScanResult::SetGeometricalError(double err) {
    m_geomError = err;
}

/** Getting the scattering Error */
void	CScanResult::SetScatteringError(double err) {
    m_scatteringError = err;
}

/** Getting the spectroscopical error */
void	CScanResult::SetSpectroscopicalError(double err) {
    m_spectroscopyError = err;
}

const std::string CScanResult::GetSpecieName(unsigned long spectrumNum, unsigned long specieNum) const
{
    if (IsValidSpectrumIndex(spectrumNum) && m_spec[spectrumNum].m_referenceResult.size() > specieNum)
    {
        return m_spec[spectrumNum].m_referenceResult[specieNum].m_specieName;
    }
    else
    {
        return "";
    }
}
