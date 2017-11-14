#include "StdAfx.h"
#include "communicationdatastorage.h"

// ----------------- CLinkInfo - class ------------------------
CCommunicationDataStorage::CLinkInfo::CLinkInfo(){
	m_downloadSpeed = 0.0;
}
CCommunicationDataStorage::CLinkInfo::~CLinkInfo(){
}
CCommunicationDataStorage::CLinkInfo &CCommunicationDataStorage::CLinkInfo::operator=(const CCommunicationDataStorage::CLinkInfo &info){
	this->m_downloadSpeed = info.m_downloadSpeed;
	this->m_time					= info.m_time;

	return *this;
}

// --------- CCommunicationDataStorage - class ----------------

CCommunicationDataStorage::CCommunicationDataStorage(void)
{
	for(int i = 0; i < MAX_NUMBER_OF_SCANNING_INSTRUMENTS; ++i)
    this->m_serials[i].Format("");
	memset(m_communicationStatus,-1,sizeof(int)*MAX_NUMBER_OF_SCANNING_INSTRUMENTS);
	memset(m_nDataLinkInformation, 0, sizeof(int)*MAX_NUMBER_OF_SCANNING_INSTRUMENTS);
	
	m_nFTPServerLinkInformation = 0;
	m_serialNum = 0;
}

CCommunicationDataStorage::~CCommunicationDataStorage(void)
{
}
int CCommunicationDataStorage::AddData(const CString &serial)
{
  // get the scanner index
  int scannerIndex = GetScannerIndex(serial);

  if((scannerIndex < 0))
	{
    if(m_serialNum < MAX_NUMBER_OF_SCANNING_INSTRUMENTS)
		{
      // if the scanner is not in the list then insert it
      scannerIndex = m_serialNum;
      m_serials[m_serialNum].Format("%s", serial);
      ++m_serialNum;
    }
		else
		{
      // could not insert the serial number
      return -1;
    }
  }
	return 1;
}
/** Set the status of the communication 
    @param serialID - the serial number of the spectrometer which communication status should be updated */
void CCommunicationDataStorage::SetStatus(const CString &serialID, COMMUNICATION_STATUS status)
{
	AddData(serialID);
	int scannerIndex = GetScannerIndex(serialID);
	if(scannerIndex > -1)
		this->m_communicationStatus[scannerIndex] = status;	
}
/** Get the status of the communication 
    @param serialID - the serial number of the spectrometer which communication status should be updated */
int CCommunicationDataStorage::GetStatus(const CString &serialID)
{
	int status = -1;
	int scannerIndex = GetScannerIndex(serialID);
	if(scannerIndex > -1)
		status = this->m_communicationStatus[scannerIndex];
	return status;
}
/** Returns the spectrometer index given a serial number */
int  CCommunicationDataStorage::GetScannerIndex(const CString &serial){
  static CString lastSerial = CString();
  static int lastIndex = 0;

  unsigned int i;

  // Check if this is the same serial number as last time this function was called
  if(Equals(serial, lastSerial)){
    return lastIndex;
  }

  // If the serial number is different from last time, search for it
  for(i = 0; i < m_serialNum; ++i){
    if(Equals(serial, m_serials[i])){
      lastSerial.Format("%s", serial);
      lastIndex = i;
      return (int)i;
    }
  }

  // not found
  return -1;
}

/** Adds one bit of communication information to the list */
void	CCommunicationDataStorage::AddDownloadData(const CString &serial, double linkSpeed, const CDateTime *timeOfDownload){
	RemoveOldLinkInformation(); // First of all, clear out all old data

	if(Equals(serial, "FTP")){
		// The data comes from the FTP-uploading link

		// First check if the data-buffer is full
		if(m_nFTPServerLinkInformation == MAX_HISTORY){
			// shift all data-bits down one step
			for(int i = 0; i < MAX_HISTORY; ++i){
				m_ftpServerLinkInformation[i] = m_ftpServerLinkInformation[i+1];
			}
			--m_nFTPServerLinkInformation;
		}

		// replace the information
		CLinkInfo &thisInfo				= m_ftpServerLinkInformation[m_nFTPServerLinkInformation];
		thisInfo.m_downloadSpeed	= linkSpeed;
		if(timeOfDownload == NULL)
			thisInfo.m_time.SetToNow();
		else
			thisInfo.m_time = *timeOfDownload;

		++m_nFTPServerLinkInformation;
	}else{
		// The data comes from one of the scanners
		AddData(serial);
		int scannerIndex = GetScannerIndex(serial);
		if(scannerIndex <= -1)
			return;

		// First check if the data-buffer is full
		if(m_nDataLinkInformation[scannerIndex] == MAX_HISTORY){
			// shift all data-bits down one step
			for(int i = 0; i < MAX_HISTORY; ++i){
				m_dataLinkInformation[scannerIndex][i] = m_dataLinkInformation[scannerIndex][i+1];
			}
			--m_nDataLinkInformation[scannerIndex];
		}

		// replace the information
		CLinkInfo &thisInfo				= m_dataLinkInformation[scannerIndex][m_nDataLinkInformation[scannerIndex]];
		thisInfo.m_downloadSpeed	= linkSpeed;
		if(timeOfDownload == NULL)
			thisInfo.m_time.SetToNow();
		else
			thisInfo.m_time = *timeOfDownload;

		++m_nDataLinkInformation[scannerIndex];
	}
}

/** Get link-speed data. 
    @param serial - the serial number of the spectrometer for which the data should be retrieved.
		@param timeBuffer - the time-data will be copied into this buffer. The time format is the number of seconds since midnight
    @param dataBuffer - the link-speed data will be copied into this buffer.
    @param bufferSize - the maximum number of data points that the buffer can handle.
    @return the number of data points copied into the dataBuffer*/
long CCommunicationDataStorage::GetLinkSpeedData(const CString &serial, double *timeBuffer, double *dataBuffer, long bufferSize){
	int nCopy;

	if(Equals(serial, "FTP")){
		// Copy the link-speed data
		nCopy = min(bufferSize, m_nFTPServerLinkInformation);
		for(int i = 0; i < nCopy; ++i){
			CLinkInfo &info		= m_ftpServerLinkInformation[i];
			dataBuffer[i]			= info.m_downloadSpeed;
			timeBuffer[i]			= info.m_time.hour * 3600.0 + info.m_time.minute * 60.0 + info.m_time.second;
		}
	}else{
		// get the scanner index
		int scannerIndex = GetScannerIndex(serial);

		if((scannerIndex < 0) || (scannerIndex > MAX_NUMBER_OF_SCANNING_INSTRUMENTS))
			return 0;

		// Copy the link-speed data
		nCopy = min(bufferSize, m_nDataLinkInformation[scannerIndex]);
		for(int i = 0; i < nCopy; ++i){
			CLinkInfo &info		= m_dataLinkInformation[scannerIndex][i];
			dataBuffer[i]			= info.m_downloadSpeed;
			timeBuffer[i]			= info.m_time.hour * 3600.0 + info.m_time.minute * 60.0 + info.m_time.second;
		}
	}

  return nCopy;
}

/** Clear out old data from the 'm_dataLinkInformation' buffers */
void CCommunicationDataStorage::RemoveOldLinkInformation(){
	static int lastDate; // the day of month when this function was last called
	int k;	// iterator

	// todays date
	int today = Common::GetDay();

	// don't check this several times every day
	if(lastDate == today)
		return;

	// Clear old pieces of link-information
	for(unsigned int scannerIndex = 0; scannerIndex < MAX_NUMBER_OF_SCANNING_INSTRUMENTS; ++scannerIndex){
		k = 0;

		while(k < m_nDataLinkInformation[scannerIndex]){
			if(m_dataLinkInformation[scannerIndex][k].m_time.day == today){
				++k;
			}else{
				// if this measurement is not from today, then remove it
				for(int j = k; j < m_nDataLinkInformation[scannerIndex]-1; ++j){
					m_dataLinkInformation[scannerIndex][j]	= m_dataLinkInformation[scannerIndex][j+1];
				}
				--m_nDataLinkInformation[scannerIndex];
			}
		}
	}

	lastDate = today;
}
