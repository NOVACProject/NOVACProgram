#include "stdafx.h"
#include "NetCdfWindFileReader.h"
#include "NetCdfFileReader.h"

namespace FileHandler
{

RETURN_CODE CNetCdfWindFileReader::ReadWindFile(CWindFieldDatabase& result)
{
    NetCdfFileReader fileReader;
    fileReader.Open((LPCSTR)m_windFile);

    // First the mandatory variables
    NetCdfTensor longitude = fileReader.ReadVariable("longitude");

    NetCdfTensor latitude = fileReader.ReadVariable("latitude");

    NetCdfTensor level = fileReader.ReadVariable("level");

    NetCdfTensor time = fileReader.ReadVariable("time");

    NetCdfTensor u = fileReader.ReadVariable("u");

    NetCdfTensor v = fileReader.ReadVariable("v");


    return FAIL;
}

}