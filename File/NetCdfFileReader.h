#pragma once

#include <vector>
#include <string>

namespace FileHandler
{

struct NetCdfDimension
{
    int index;
    std::string name;
};

struct NetCdfTensor
{
    // Defines the number of dimensions of this variable
    //  and the size in each dimension.
    std::vector<size_t> size;

    // The dimensions of this variable
    std::vector<NetCdfDimension> dimensions;

    // Stores the values of this variable.
    //  Multi-dimensional variables are stored as flattened arrays.
    std::vector<float> values;

    // The name of the variable.
    std::string name;
};

class NetCdfFileReader
{
public:
    NetCdfFileReader();

    ~NetCdfFileReader();

    /** Attempts to open the net-cdf file with the provided filename, 
        @throws NetCdfException if this cannot be done. */
    void Open(const std::string& filename);

    void Close();

    /** @returns true if this file contains a variable with the provided name.
        @throws NetCdfException if the file cannot be read. */
    bool ContainsVariable(const std::string& variableName);

    /** @returns the index of the variable with the given name.
        @throws NetCdfException if this cannot be retrieved. */
    int GetIndexOfVariable(const std::string& variableName);

    /** Reads one variable from this netcdf file and returns the result.
        @throws NetCdfException if the variable cannot be found or the file cannot be read. */
    NetCdfTensor ReadVariable(const std::string& variableName);

private:
    int m_netCdfFileHandle = 0;

    struct LinearScaling
    {
        double offset = 0.0;
        double scaleFactor = 1.0;
    };

    /** retrieves the LinearScaling which is to be applied to the variable with the provided index.
        @return true if either scale_factor OR add_offset is set. */
    bool GetLinearScalingForVariable(int variableIdx, LinearScaling& scaling);

    /** @return the number of attributes associated with one variable.
        @return -1 if the reading failed for some reason */
    int GetNumberOfAttributesForVariable(int variableIdx);

    /** Attempts to retrieve the size of the provided variable.
        For a multi-dimensional variable, the result will contain multiple dimensions.
        This will also read the entire variable from file at once,
            be aware that large files may cause out-of-memory conditions.
        @throws NetCdfException if this cannot be retrieved. */
    std::vector<size_t> GetSizeOfVariable(int variableIdx);

    std::vector<int> GetDimensionIndicesOfVariable(int variableIdx);

    /** Attempts to read the variable with the provided index
        and return the result as a float array.
        If the variable is a multi-dimensional array then the array will
         be flattened into a one-dimensional result.
        This will also read the entire variable from file at once,
            be aware that large files may cause out-of-memory conditions.
        This will apply the provided linear scaling factor.
        @throws NetCdfException if this cannot be retrieved. */
    std::vector<float> ReadVariableAsFloat(int variableIdx, const LinearScaling& scaling);
    std::vector<float> ReadVariableAsFloat(int variableIdx);


};

}
