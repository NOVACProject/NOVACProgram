#include "StdAfx.h"
#include "MeteorologicalData.h"
#include "../File/WindFileReader.h"
#include "../File/NetCdfWindFileReader.h"

/** The global instance of meterological data */
CMeteorologicalData g_metData;

int CMeteorologicalData::SetWindField(const CString& serialNumber, const CWindField& windField)
{
    for (int i = 0; i < m_scannerNum; ++i)
    {
        if (Equals(serialNumber, m_scanner[i]))
        {
            m_windFieldAtScanner[i] = windField;
            return 0;
        }
    }

    // scanner not found
    if (m_scannerNum == MAX_NUMBER_OF_SCANNING_INSTRUMENTS)
    {
        // array full
        return 1;
    }

    // insert a scanner
    m_scanner[m_scannerNum] = serialNumber;
    m_windFieldAtScanner[m_scannerNum] = windField;
    ++m_scannerNum;

    return 0;
}

void CMeteorologicalData::SetVolcanoes(const std::vector<CNamedLocation>& volcanoes)
{
    this->m_volcanoes = volcanoes;
}

int CMeteorologicalData::GetWindField(const CString& serialNumber, const CDateTime& dt, CWindField& windField)
{
    int scannerIndex = -1;

    // Find the index of the scanner
    for (scannerIndex = 0; scannerIndex < m_scannerNum; ++scannerIndex)
    {
        if (Equals(serialNumber, m_scanner[scannerIndex]))
        {
            break;
        }
    }

    // 1. Try to read the wind from the wind-field file reader
    if (m_wfDatabaseFromFile != nullptr)
    {
        std::lock_guard<std::mutex> lock(m_wfDatabaseMutex);
        if (SUCCESS == m_wfDatabaseFromFile->InterpolateWindField(dt, windField))
        {
            // Check if the file contains the wind-speed, the wind-direction and/or the plume height
            if (!m_wfDatabaseFromFile->m_containsWindDirection && scannerIndex >= 0)
            {
                windField.SetWindDirection(m_windFieldAtScanner[scannerIndex].GetWindDirection(), MET_USER);
            }

            if (!m_wfDatabaseFromFile->m_containsWindSpeed)
            {
                windField.SetWindSpeed(m_windFieldAtScanner[scannerIndex].GetWindSpeed(), MET_USER);
            }

            if (!m_wfDatabaseFromFile->m_containsPlumeHeight)
            {
                windField.SetPlumeHeight(m_windFieldAtScanner[scannerIndex].GetPlumeHeight(), MET_USER);
            }
            return 0; // wind-field found...
        }
    }

    // 2. No wind field found in the reader, use the user-supplied or default...
    if (scannerIndex >= 0)
    {
        windField = m_windFieldAtScanner[scannerIndex];
        return 0;
    }

    // scanner not found
    return 1;
}

int CMeteorologicalData::ReadWindFieldFromFile(const CString& fileName)
{
    std::lock_guard<std::mutex> lock(m_wfDatabaseMutex);

    // Completely reset the data in the existing file-reader
    m_wfDatabaseFromFile.reset(new CWindFieldDatabase());

    // Start reading the file
    bool fileReadSuccessfully = false;
    if (Equals(fileName.Right(4), ".txt"))
    {
        fileReadSuccessfully = ReadWindFieldFromTextFile(fileName);
    }
    else if (Equals(fileName.Right(3), ".nc"))
    {
        fileReadSuccessfully = ReadWindFieldFromNetCdfFile(fileName);
    }

    if (fileReadSuccessfully)
    {
        return 0;
    }
    else
    {
        m_wfDatabaseFromFile.release();
        return 1;
    }
}

bool CMeteorologicalData::ReadWindFieldFromTextFile(const CString& fileName)
{
    FileHandler::CWindFileReader fileReader;

    fileReader.m_windFile = fileName;

    auto returnCode = fileReader.ReadWindFile(*m_wfDatabaseFromFile);

    return returnCode == SUCCESS;
}

bool CMeteorologicalData::ReadWindFieldFromNetCdfFile(const CString& fileName)
{
    FileHandler::CNetCdfWindFileReader fileReader;

    fileReader.m_windFile = fileName;

    // Read the wind-file. If we have multiple volcanoes listed, then this wind-field
    //  file may be for one volcano only. Test with each volcano until we found the correct one.
    //  NOTICE: This will not work for a wind-field file with multiple locations...
    if (this->m_volcanoes.size() > 0)
    {
        for (const CNamedLocation& volcano : this->m_volcanoes)
        {
            auto returnCode = fileReader.ReadWindFile(volcano, *m_wfDatabaseFromFile);
            if (returnCode == SUCCESS) // we found the correct volcano.
            {
                return true;
            }
        }

        return false; // failed to read the file.
    }
    else
    {
        CGPSData volcanoPosition; // TODO: Fill in this!
        auto returnCode = fileReader.ReadWindFile(volcanoPosition, *m_wfDatabaseFromFile);

        return returnCode == SUCCESS;
    }
}

