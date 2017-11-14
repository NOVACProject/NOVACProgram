#include "StdAfx.h"
#include "meteorologicaldata.h"

/** The global instance of meterological data */
CMeteorologicalData g_metData;

CMeteorologicalData::CMeteorologicalData(void)
{
	m_scannerNum = 0;
	m_wfReader = NULL;
}

CMeteorologicalData::~CMeteorologicalData(void)
{
	if(m_wfReader != NULL){
		delete m_wfReader;
		m_wfReader = NULL;
	}
}

/** Set the wind field for a scanner */
int CMeteorologicalData::SetWindField(const CString &serialNumber, const CWindField &windField){
	int i;
	for(i = 0; i < m_scannerNum; ++i){
		if(Equals(serialNumber, m_scanner[i])){
			m_windFieldAtScanner[i] = windField;
			return 0;
		}
	}

	// scanner not found
	if(m_scannerNum == MAX_NUMBER_OF_SCANNING_INSTRUMENTS){
		// array full
		return 1;
	}

	// insert a scanner
	m_scanner[m_scannerNum].Format("%s", serialNumber);
	m_windFieldAtScanner[m_scannerNum] = windField;
	++m_scannerNum;

	return 0;
}

/** Get the wind field for a scanner */
int CMeteorologicalData::GetWindField(const CString &serialNumber, const CDateTime &dt, CWindField &windField){
	int scannerIndex = -1;

	// Find the index of the scanner
	for(scannerIndex = 0; scannerIndex < m_scannerNum; ++scannerIndex){
		if(Equals(serialNumber, m_scanner[scannerIndex])){
			break;
		}
	}

	// 1. Try to read the wind from the wind-field file reader
	if(m_wfReader != NULL){
		if(SUCCESS == m_wfReader->InterpolateWindField(dt, windField)){
			// Check if the file contains the wind-speed, the wind-direction and/or the plume height
			if(!m_wfReader->m_containsWindDirection && scannerIndex >= 0){
				windField.SetWindDirection(m_windFieldAtScanner[scannerIndex].GetWindDirection(), MET_USER);
			}

			if(!m_wfReader->m_containsWindSpeed){
				windField.SetWindSpeed(m_windFieldAtScanner[scannerIndex].GetWindSpeed(), MET_USER);
			}

			if(!m_wfReader->m_containsPlumeHeight){
				windField.SetPlumeHeight(m_windFieldAtScanner[scannerIndex].GetPlumeHeight(), MET_USER);
			}
			return 0; // wind-field found...
		}
	}

	// 2. No wind field found in the reader, use the user-supplied or default...
	if(scannerIndex >= 0){
		windField = m_windFieldAtScanner[scannerIndex];
		return 0;
	}

	// scanner not found
	return 1;
}

/** Tries to read in a wind-field from a file. If this is successfull
			then all wind-data returned will be first searched for in the wind-field
			file and secondly from the user given or default values. 
			@return 0 on success */
int CMeteorologicalData::ReadWindFieldFromFile(const CString &fileName){
	
	// Completely reset the data in the existing file-reader
	if(m_wfReader != NULL){
		delete m_wfReader;
	}
	m_wfReader = new FileHandler::CWindFileReader();

	// Set the path to the file
	m_wfReader->m_windFile.Format(fileName);

	// Read the wind-file
	if(SUCCESS != m_wfReader->ReadWindFile()){
		delete m_wfReader;
		m_wfReader = NULL;
		return 1;
	}else{
		return 0;
	}
}
