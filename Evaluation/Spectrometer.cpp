#include "StdAfx.h"
#include "spectrometer.h"
#include <SpectralEvaluation/Spectra/SpectrometerModel.h>

using namespace Evaluation;
using namespace novac;

CSpectrometer::CSpectrometer()
    : m_history(nullptr), m_channel(0), m_gpsReadingsNum(0)
{}

CSpectrometer::~CSpectrometer(void)
{
    m_history = nullptr;
}

CSpectrometer& CSpectrometer::operator=(const CSpectrometer& spec2)
{
    this->m_fitWindows.resize(spec2.m_fitWindows.size());
    for (size_t i = 0; i < spec2.m_fitWindows.size(); ++i)
    {
        this->m_fitWindows[i] = spec2.m_fitWindows[i];
    }

    this->m_settings = spec2.m_settings;
    this->m_scanner = spec2.m_scanner;
    this->m_logFileHandler = spec2.m_logFileHandler;
    this->m_channel = spec2.m_channel;
    this->m_gpsReadingsNum = spec2.m_gpsReadingsNum;
    return *this;
}

CSpectrometer::CSpectrometer(const CSpectrometer& spec2)
{
    this->m_fitWindows.resize(spec2.m_fitWindows.size());
    for (size_t i = 0; i < spec2.m_fitWindows.size(); ++i)
    {
        this->m_fitWindows[i] = spec2.m_fitWindows[i];
    }

    this->m_settings = spec2.m_settings;
    this->m_scanner = spec2.m_scanner;
    this->m_logFileHandler = spec2.m_logFileHandler;
    this->m_channel = spec2.m_channel;
    this->m_gpsReadingsNum = spec2.m_gpsReadingsNum;
}

const CString& CSpectrometer::SerialNumber() const
{
    return this->m_settings.serialNumber;
}

double CSpectrometer::GetMaxIntensity() const
{
    return CSpectrometerDatabase::GetInstance().GetModel(m_settings.modelName).maximumIntensityForSingleReadout;
}

void CSpectrometer::RememberResult(CScanResult& lastResult)
{
    static std::string specie = "SO2";

    // 0. Some checking of reasonability
    if (lastResult.GetEvaluatedNum() <= 1)
    {
        return;
    }

    // 1. Check the mode of the measurement
    const novac::MeasurementMode lastMode = novac::CheckMeasurementMode(lastResult);
    lastResult.m_measurementMode = lastMode;

    // 2. Get the position of the plume centre, if there is any plume
    double centre1, centre2, plumeCompleteness, plumeEdge_low, plumeEdge_high;
    bool inPlume = lastResult.CalculatePlumeCentre(specie, centre1, centre2, plumeCompleteness, plumeEdge_low, plumeEdge_high);

    // 3. Get date and time when the scan started
    unsigned short date[3];
    lastResult.GetDate(0, date);
    const CDateTime* starttid = lastResult.GetStartTime(0);
    CDateTime tid = CDateTime(date[0], date[1], date[2], starttid->hour, starttid->minute, starttid->second);

    // 4. Get the maximum column value
    double maxColumn = lastResult.GetMaxColumn(specie);

    // 5. Append the results to the history
    if (m_history != nullptr)
    {
        if (novac::MeasurementMode::Flux == lastMode)
        {
            m_history->AppendScanResult(lastResult, specie);
        }
        else if (novac::MeasurementMode::Composition == lastMode)
        {
            m_history->AppendCompMeasurement(tid.year, tid.month, tid.day, tid.hour, tid.minute, tid.second);
        }
        else if (novac::MeasurementMode::Windspeed == lastMode)
        {
            m_history->AppendWindMeasurement(tid.year, tid.month, tid.day, tid.hour, tid.minute, tid.second);
        }
    }
}