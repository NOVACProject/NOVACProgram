#pragma once

#include <vector>
#include <string>

namespace FileHandler
{

class NetCdfFileReader
{
public:
    NetCdfFileReader();

    ~NetCdfFileReader();

    /** Attempts to open the net-cdf file with the provided filename, 
        @throws NetCdfException if this cannot be done. */
    void Open(const std::string& filename);

    void Close();

    /** @returns the index of the variable with the given name.
        @throws NetCdfExcetpion if this cannot be retrieved. */
    int GetIndexOfVariable(const std::string& variableName);

    /** Attempts to read the variable with the provided index
        and return the result as a float array. 
        If the variable is a multi-dimensional array then the array will
        be flattened into a one-dimensional result.
        This will also read the entire variable from file at once, 
            be aware that large files may cause out-of-memory conditions.
        @throws NetCdfException if this cannot be retrieved. */
    std::vector<float> ReadVariableAsFloat(int variableIdx);
    std::vector<float> ReadVariableAsFloat(const std::string& variableName);

    /** Attempts to retrieve the size of the provided variable.
        For a multi-dimensional variable, the result will contain multiple dimensions.
        This will also read the entire variable from file at once,
        be aware that large files may cause out-of-memory conditions.
        @throws NetCdfException if this cannot be retrieved. */
    std::vector<size_t> GetSizeOfVariable(int variableIdx);
    std::vector<size_t> GetSizeOfVariable(const std::string& variableName);

private:
    int m_netCdfFileHandle = 0;

    std::vector<int> GetDimensionIndicesOfVariable(int variableIdx);
};

}
