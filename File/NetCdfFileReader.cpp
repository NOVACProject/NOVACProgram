#include "NetCdfFileReader.h"
#include "NetCdfException.h"
#include "../External/NetCdf_4_7_2/include/netcdf.h"
#include <sstream>

namespace FileHandler
{

NetCdfFileReader::NetCdfFileReader()
{
    m_netCdfFileHandle = 0;
}

NetCdfFileReader::~NetCdfFileReader()
{
    Close();
}

void NetCdfFileReader::Open(const std::string& filename)
{
    if (m_netCdfFileHandle != 0)
    {
        Close();
    }

    int status = nc_open(filename.c_str(), NC_NOWRITE, &m_netCdfFileHandle);

    if (status != NC_NOERR)
    {
        std::stringstream msg;
        msg << "Failed to open net-cdf file with path: '" << filename << "' Error code returned was: " << status;
        throw NetCdfException(msg.str().c_str(), status);
    }
}

void NetCdfFileReader::Close()
{
    if (m_netCdfFileHandle != 0)
    {
        nc_close(m_netCdfFileHandle);
        m_netCdfFileHandle = 0;
    }
}

std::vector<int> NetCdfFileReader::GetDimensionIndicesOfVariable(int variableIdx)
{
    int nofDimensions = 0;
    int status = nc_inq_varndims(m_netCdfFileHandle, variableIdx, &nofDimensions);
    if (status != NC_NOERR)
    {
        std::stringstream msg;
        msg << "Failed to retrieve the number of dimensions of variable '" << variableIdx << "'. Error code returned was: " << status;
        throw NetCdfException(msg.str().c_str(), status);
    }

    std::vector<int> dimensions(nofDimensions);
    status = nc_inq_vardimid(m_netCdfFileHandle, variableIdx, dimensions.data());
    if (status != NC_NOERR)
    {
        std::stringstream msg;
        msg << "Failed to retrieve the dimension indices of variable '" << variableIdx << "'. Error code returned was: " << status;
        throw NetCdfException(msg.str().c_str(), status);
    }

    return dimensions;
}

int NetCdfFileReader::GetIndexOfVariable(const std::string& variableName)
{
    int index = 0;
    int status = nc_inq_varid(m_netCdfFileHandle, variableName.c_str(), &index);

    if (status != NC_NOERR)
    {
        std::stringstream msg;
        msg << "Failed to retrieve the index of variable '" << variableName << "'. Error code returned was: " << status;
        throw NetCdfException(msg.str().c_str(), status);
    }

    return index;
}

std::vector<size_t> NetCdfFileReader::GetSizeOfVariable(int variableIdx)
{
    auto dimensionIndices = GetDimensionIndicesOfVariable(variableIdx);

    std::vector<size_t> sizes(dimensionIndices.size());

    for (size_t ii = 0; ii < dimensionIndices.size(); ++ii)
    {
        size_t value = 0;
        int status = nc_inq_dimlen(m_netCdfFileHandle, dimensionIndices[ii], &value);

        if (status != NC_NOERR)
        {
            std::stringstream msg;
            msg << "Failed to retrieve the dimensions of variable '" << variableIdx << "'. Error code returned was: " << status;
            throw NetCdfException(msg.str().c_str(), status);
        }

        sizes[ii] = value;
    }

    return sizes;
}

std::vector<size_t> NetCdfFileReader::GetSizeOfVariable(const std::string& variableName)
{
    int index = GetIndexOfVariable(variableName);

    return GetSizeOfVariable(index);
}

// helper function to multiply all the values in a vector
size_t ProductOfElements(std::vector<size_t> sizes)
{
    size_t product = 1;

    for (size_t dim : sizes)
    {
        product *= dim;
    }

    return product;
}

std::vector<float> NetCdfFileReader::ReadVariableAsFloat(int variableIdx)
{
    std::vector<size_t> variableSize = GetSizeOfVariable(variableIdx);

    size_t totalNumberOfElements = ProductOfElements(variableSize);

    std::vector<float> values(totalNumberOfElements);

    int status = nc_get_var_float(m_netCdfFileHandle, variableIdx, values.data());
    if (status != NC_NOERR)
    {
        std::stringstream msg;
        msg << "Failed to retrieve the values of variable '" << variableIdx << "'. Error code returned was: " << status;
        throw NetCdfException(msg.str().c_str(), status);
    }

    return values;
}

std::vector<float> NetCdfFileReader::ReadVariableAsFloat(const std::string& variableName)
{
    int index = GetIndexOfVariable(variableName);

    return ReadVariableAsFloat(index);
}

}

