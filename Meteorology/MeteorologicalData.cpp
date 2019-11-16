#include "StdAfx.h"
#include "MeteorologicalData.h"

/** The global instance of meterological data */
CMeteorologicalData g_metData;

CMeteorologicalData::CMeteorologicalData()
 : m_scannerNum(0),
   m_wfReader(nullptr)
{
}

CMeteorologicalData::~CMeteorologicalData()
{
    if (m_wfReader != nullptr)
    {
        delete m_wfReader;
        m_wfReader = nullptr;
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
    if (m_wfReader != nullptr)
    {
        if (SUCCESS == m_wfReader->InterpolateWindField(dt, windField))
        {
            // Check if the file contains the wind-speed, the wind-direction and/or the plume height
            if (!m_wfReader->m_containsWindDirection && scannerIndex >= 0)
            {
                windField.SetWindDirection(m_windFieldAtScanner[scannerIndex].GetWindDirection(), MET_USER);
            }

            if (!m_wfReader->m_containsWindSpeed)
            {
                windField.SetWindSpeed(m_windFieldAtScanner[scannerIndex].GetWindSpeed(), MET_USER);
            }

            if (!m_wfReader->m_containsPlumeHeight)
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
    // Completely reset the data in the existing file-reader
    if (m_wfReader != nullptr)
    {
        delete m_wfReader;
    }
    m_wfReader = new FileHandler::CWindFileReader();

    // Set the path to the file
    m_wfReader->m_windFile.Format(fileName);

    // Read the wind-file
    if (SUCCESS != m_wfReader->ReadWindFile())
    {
        delete m_wfReader;
        m_wfReader = nullptr;
        return 1;
    }
    else
    {
        return 0;
    }
}
