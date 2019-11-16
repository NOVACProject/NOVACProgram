#include "NetCdfFileReader.h"
#include "NetCdfException.h"
#include "../External/NetCdf_4_7_2/include/netcdf.h"
#include <sstream>

// TODO: move
template<class T>
T ProductOfElements(std::vector<T> values)
{
    T product = 1;

    for (T dim : values)
    {
        product *= dim;
    }

    return product;
}

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

bool NetCdfFileReader::ContainsVariable(const std::string& variableName)
{
    int index = 0;
    int status = nc_inq_varid(m_netCdfFileHandle, variableName.c_str(), &index);

    if (status == NC_NOERR)
    {
        return true;
    }
    else if (status == NC_ENOTVAR)
    {
        return false;
    }
    else
    {
        std::stringstream msg;
        msg << "Failed to retrieve variable '" << variableName << "'. Error code returned was: " << status;
        throw NetCdfException(msg.str().c_str(), status);
    }
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


NetCdfTensor NetCdfFileReader::ReadVariable(const std::string& variableName)
{
    NetCdfTensor result;

    // retrieves the variable index, this throws an exception if the variable cannot be found.
    int variableIndex = GetIndexOfVariable(variableName);

    result.size = this->GetSizeOfVariable(variableIndex);

    {
        std::vector<char> name;
        name.resize(NC_MAX_NAME + 1);
        auto dimensionIndices = this->GetDimensionIndicesOfVariable(variableIndex);
        result.dimensions.resize(dimensionIndices.size());
        for (size_t ii = 0; ii < dimensionIndices.size(); ++ii)
        {
            result.dimensions[ii].index = dimensionIndices[ii];

            if (NC_NOERR == nc_inq_dimname(this->m_netCdfFileHandle, dimensionIndices[ii], name.data()))
            {
                result.dimensions[ii].name = std::string(name.data());
            }
        }
    }

    LinearScaling variableScaling;
    if (GetLinearScalingForVariable(variableIndex, variableScaling))
    {
        result.values = this->ReadVariableAsFloat(variableIndex, variableScaling);
    }
    else
    {
        result.values = this->ReadVariableAsFloat(variableIndex);
    }

    result.name = variableName;

    return result;
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

int NetCdfFileReader::GetNumberOfAttributesForVariable(int variableIdx)
{
    int numberOfAttributes = 0;
    int error = nc_inq_natts(this->m_netCdfFileHandle, &numberOfAttributes);

    if (error == NC_NOERR)
    {
        return numberOfAttributes;
    }
    return -1;
}

bool NetCdfFileReader::GetLinearScalingForVariable(int variableIdx, NetCdfFileReader::LinearScaling& scaling)
{
    int numberOfAttributes = GetNumberOfAttributesForVariable(variableIdx);
    if (numberOfAttributes < 0)
    {
        return false;
    }

    bool scalingFoundInFile = false;
    double scaleFactorValue = 1.0;
    if (NC_NOERR == nc_get_att_double(this->m_netCdfFileHandle, variableIdx, "scale_factor", &scaleFactorValue))
    {
        scaling.scaleFactor = scaleFactorValue;
        scalingFoundInFile = true;
    }

    double additionalOffsetValue = 1.0;
    if (NC_NOERR == nc_get_att_double(this->m_netCdfFileHandle, variableIdx, "add_offset", &additionalOffsetValue))
    {
        scaling.offset = additionalOffsetValue;
        scalingFoundInFile = true;
    }

    return scalingFoundInFile;
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

std::vector<float> NetCdfFileReader::ReadVariableAsFloat(int variableIdx, const LinearScaling& scaling)
{
    std::vector<float> values = ReadVariableAsFloat(variableIdx);

    for (size_t ii = 0; ii < values.size(); ++ii)
    {
        values[ii] = values[ii] * scaling.scaleFactor + scaling.offset;
    }

    return values;
}

}  // namespace FileHandler

