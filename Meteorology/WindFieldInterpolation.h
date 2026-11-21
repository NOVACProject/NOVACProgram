#pragma once

#include <vector>

struct InterpolatedWind
{
    std::vector<double> speed;
    std::vector<double> speedError;
    std::vector<double> direction; // [degrees]
    std::vector<double> directionError; // [degrees]
    std::vector<double> cloudCoverage; // fractional cloud coverage in the range [0, 1]
    std::vector<double> relativeHumidity; // [%]
};

/** Performs a linear interpolation to retrieve values from the given four-dimensional
    vector at all points in time for the provided spatial indices.
    This differens from the function 'InterpolateWind' in that no values are calculated,
    only pure interpolation is performed.
    @throws invalid_argument if values is not a four-dimensional matrix.
*/
void InterpolateValue(
    const std::vector<float>& values,
    const std::vector<size_t>& sizes,
    const std::vector<double>& spatialIndices,
    std::vector<double>& result);
