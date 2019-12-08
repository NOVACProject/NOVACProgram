#include "stdafx.h"
#include "NetCdfWindFileReader.h"
#include "NetCdfFileReader.h"
#include "../Meteorology/WindFieldInterpolation.h"
#include <SpectralEvaluation/Interpolation.h>

namespace FileHandler
{

RETURN_CODE CNetCdfWindFileReader::ReadWindFile(const CGPSData& position, CWindFieldDatabase& result)
{
    {
        CString message;
        message.Format("Reading wind field from NetCdf data file %s", (LPCSTR)m_windFile);
        ShowMessage(message);
    }

    NetCdfFileReader fileReader;
    fileReader.Open((LPCSTR)m_windFile);

    // This is the fixed translation between levels and altitudes
    const std::vector<float> levels
    {
        225, 250, 300, 350, 400, 450,
        500, 550, 600, 650, 700, 750,
        775, 800, 825, 850, 875, 900,
        925, 950, 975, 1000
    };
    const std::vector<float> altitudes_km
    {
        10.42F,9.59F, 8.81F, 7.38F, 6.71F, 5.50F,
        4.94F, 4.42F, 3.48F, 3.06F, 2.67F, 2.31F,
        1.98F, 1.68F, 1.41F, 1.17F, 0.95F, 0.76F,
        0.60F, 0.46F, 0.24F, 0.10F
    };

    // First the mandatory variables
    NetCdfTensor longitude = fileReader.ReadVariable("longitude");
    NetCdfTensor latitude = fileReader.ReadVariable("latitude");
    NetCdfTensor level = fileReader.ReadVariable("level");
    NetCdfTensor time = fileReader.ReadVariable("time");
    NetCdfTensor u = fileReader.ReadVariable("u");
    NetCdfTensor v = fileReader.ReadVariable("v");

    const double latitudeIdx = GetFractionalIndex(latitude.values, position.m_latitude);
    double longitudeIdx = GetFractionalIndex(longitude.values, position.m_longitude);
    if (std::isnan(longitudeIdx))
    {
        longitudeIdx = GetFractionalIndex(longitude.values, 360.0 + position.m_longitude);
    }
    const double levelIdx = GetFractionalIndex(altitudes_km, position.m_altitude * 0.001);

    InterpolatedWind interpolationResult;
    InterpolateWind(
        u.values,
        v.values,
        u.size,
        { levelIdx, latitudeIdx, longitudeIdx },
        interpolationResult);

    if (interpolationResult.speed.size() == 0)
    {
        ShowMessage("Failed to read NetCdf data file, no wind speeds could be found.");
        return FAIL;
    }
    else if (interpolationResult.speed.size() != interpolationResult.direction.size())
    {
        ShowMessage("Failed to read NetCdf data file, number of estimated wind speeds and wind directions does not agree.");
        return FAIL;
    }
    else if (interpolationResult.speed.size() != time.values.size())
    {
        ShowMessage("Failed to read NetCdf data file, number of estimated wind speeds and number of time stamps does not agree.");
        return FAIL;
    }

    // TODO: Get the source...
    MET_SOURCE source = MET_ECMWF_FORECAST;

    // Convert to a CWindFiledDatabase
    result.Clear();
    for (size_t ii = 0; ii < interpolationResult.speed.size(); ++ii)
    {
        CWindField field;

        // time is hours since 1900-01-01 00:00:0.0
        time_t rawtimeSinceEpoch = (time_t)(time.values[ii] * 3600.0) - 2208988800L;
        struct tm ts;
        gmtime_s(&ts, &rawtimeSinceEpoch); // get the individual fields.
        field.SetDate(ts.tm_year + 1900, ts.tm_mon + 1, ts.tm_mday);
        field.SetTime(ts.tm_hour, ts.tm_min, ts.tm_sec);

        // Set the interpolated wind speed and direction
        field.SetWindSpeed(interpolationResult.speed[ii], source, interpolationResult.speedError[ii]);
        field.SetWindDirection(interpolationResult.direction[ii], source, interpolationResult.directionError[ii]);

        result.InsertWindField(field);
    }

    return SUCCESS;
}

}