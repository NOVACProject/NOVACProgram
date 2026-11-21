#include "stdafx.h"
#include "WindFieldInterpolation.h"
#include <SpectralEvaluation/Interpolation.h>
#include <stdexcept>
#include <cmath>

using namespace novac;

void SelectCubeValues(const std::vector<float>& tensor, const std::vector<size_t>& tensorDimensions, const std::vector<size_t>& floorIdx, std::vector<double>& corners)
{
    const size_t timeDim = 0;
    const size_t lvlDim = 1;
    const size_t latDim = 2;
    const size_t lonDim = 3;

    // ----------- Pick out the neighoring u- and v- values at this point in time -----------
    // -----------      this is a small cube with 2x2x2 values     -----------
    for (size_t lvlIdx = floorIdx[lvlDim]; lvlIdx <= floorIdx[lvlDim] + 1; ++lvlIdx)
    {
        for (size_t latIdx = floorIdx[latDim]; latIdx <= floorIdx[latDim] + 1; ++latIdx)
        {
            for (size_t lonIdx = floorIdx[lonDim]; lonIdx <= floorIdx[lonDim] + 1; ++lonIdx)
            {
                size_t index = ((floorIdx[timeDim] * tensorDimensions[lvlDim] + lvlIdx) * tensorDimensions[latDim] + latIdx) * tensorDimensions[lonDim] + lonIdx;

                size_t minorIndex = ((lvlIdx - floorIdx[lvlDim]) * 2 + (latIdx - floorIdx[latDim])) * 2 + (lonIdx - floorIdx[lonDim]);

                corners[minorIndex] = tensor[index];
            }
        }
    }
}

void InterpolateValue(
    const std::vector<float>& values,
    const std::vector<size_t>& sizes,
    const std::vector<double>& spatialIndices,
    std::vector<double>& result)
{
    if (sizes.size() != 4)
    {
        throw new std::invalid_argument("Invalid data to InterpolateValue, the data must be four-dimensional.");
    }
    if (spatialIndices.size() != 3)
    {
        throw new std::invalid_argument("Invalid data to InterpolateValue, there must be three spatial dimensions.");
    }

    // defining the dimensions
    const size_t timeDim = 0;

    const size_t lvlFloor = (size_t)std::floor(spatialIndices[0]);
    const size_t latFloor = (size_t)std::floor(spatialIndices[1]);
    const size_t lonFloor = (size_t)std::floor(spatialIndices[2]);

    std::vector<size_t> floorIdx = { 0, lvlFloor, latFloor, lonFloor };

    result.resize(sizes[timeDim]);

    // temporary variable in the loop below.
    std::vector<double> unitCubeValues(8);
    std::vector<double> indices(3);

    // Dimensions are [time, level, latitude, longitude]
    for (size_t timeIdx = 0; timeIdx < sizes[timeDim]; ++timeIdx)
    {
        floorIdx[timeDim] = timeIdx;

        // ----------- Pick out the neighoring u- and v- values at this point in time -----------
        // -----------      this is a small cube with 2x2x2 values     -----------
        SelectCubeValues(values, sizes, floorIdx, unitCubeValues);

        // Now perform a tri-linear interpolation inside this cube to calculate the interpolated value
        double interpolatedValue = 0;
        indices[0] = spatialIndices[0] - lvlFloor;
        indices[1] = spatialIndices[1] - latFloor;
        indices[2] = spatialIndices[2] - lonFloor;

        if (TriLinearInterpolation(unitCubeValues, indices, interpolatedValue))
        {
            result[timeIdx] = interpolatedValue;
        }
    }
}
