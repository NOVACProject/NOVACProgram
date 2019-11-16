#include "StdAfx.h"
#include "MeteorologicalData.h"
#include "../File/WindFileReader.h"

/** The global instance of meterological data */
CMeteorologicalData g_metData;

CMeteorologicalData::~CMeteorologicalData()
{
    if (m_wfDatabaseFromFile != nullptr)
    {
        delete m_wfDatabaseFromFile;
        m_wfDatabaseFromFile = nullptr;
    }
}

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
    m_scanner[m_scannerNum].Format("%s", (LPCTSTR)serialNumber);
    m_windFieldAtScanner[m_scannerNum] = windField;
    ++m_scannerNum;

    return 0;
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
    if (m_wfDatabaseFromFile != nullptr)
    {
        delete m_wfDatabaseFromFile;
    }
    m_wfDatabaseFromFile = new CWindFieldDatabase();

    // Start reading the file
    bool fileReadSuccessfully = false;
    if (Equals(fileName.Right(4), ".txt"))
    {
        FileHandler::CWindFileReader fileReader;

        // Set the path to the file
        fileReader.m_windFile = fileName;

        // Read the wind-file
        auto returnCode = fileReader.ReadWindFile(*m_wfDatabaseFromFile);
        fileReadSuccessfully = (returnCode == SUCCESS);
    }
    else if (Equals(fileName.Right(3), ".nc"))
    {
        // NetCdf file

    }

    if (fileReadSuccessfully)
    {
        return 0;
    }
    else
    {
        delete m_wfDatabaseFromFile;
        m_wfDatabaseFromFile = nullptr;
        return 1;
    }
}
