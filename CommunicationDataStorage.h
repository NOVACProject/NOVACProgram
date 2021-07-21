#pragma once

#include "Common/Common.h"
//GREEN - running, YELLOW - sleeping , RED - not connected
const enum COMMUNICATION_STATUS {COMM_STATUS_GREEN, COMM_STATUS_YELLOW, COMM_STATUS_RED};

class CCommunicationDataStorage
{

	// ------------------- class CLinkInfo -----------------------
	// --- takes care of 'remembering' the status of each link ---
	class CLinkInfo{
	public:
		CLinkInfo();
		~CLinkInfo();
		novac::CDateTime		m_time;
		double			m_downloadSpeed;
		CLinkInfo &operator=(const CLinkInfo &);
	};

public:
	CCommunicationDataStorage(void);
	~CCommunicationDataStorage(void);
	
	// ----------------------------------------------------------------------
	// ---------------------- PUBLIC DATA -----------------------------------
	// ----------------------------------------------------------------------

	static const int MAX_HISTORY = 1000;

	// ----------------------------------------------------------------------
	// ---------------------- PUBLIC METHODS --------------------------------
	// ----------------------------------------------------------------------

	/** Sets the status of the instrument with the given serial-ID number to
			the given status value */
	void SetStatus(const CString &serialID, COMMUNICATION_STATUS status);

	/** Returns the status value of the instrument with the given serial-ID */
	int GetStatus(const CString &serialID);

	/** Returns the spectrometer index given a serial number */
	int  GetScannerIndex(const CString &serial);

	/** Get link-speed data. 
		@param serial - the serial number of the spectrometer for which the data should be retrieved.
		@param timeBuffer - the time-data will be copied into this buffer. The time format is the number of seconds since midnight
		@param dataBuffer - the link-speed data will be copied into this buffer.
		@param bufferSize - the maximum number of data points that the buffer can handle.
		@return the number of data points copied into the dataBuffer*/
	long GetLinkSpeedData(const CString &serial, double *timeBuffer, double *dataBuffer, long bufferSize);

	/** Adds one bit of communication information to the list.
			If the serial equals "FTP" then the information is 
				tought to be from the ftp-uploading link */
	void	AddDownloadData(const CString &serial, double linkSpeed, const novac::CDateTime *timeOfDownload = NULL);

	/**Add one serial number into the m_serials[]array*/
	int AddData(const CString &serial);
private:

	// ----------------------------------------------------------------------
	// ---------------------- PRIVATE DATA ----------------------------------
	// ----------------------------------------------------------------------

	/** How many serial numbers have been inserted */
	unsigned int m_serialNum;

	int m_communicationStatus[MAX_NUMBER_OF_SCANNING_INSTRUMENTS];

	/** The serial numbers */
	CString m_serials[MAX_NUMBER_OF_SCANNING_INSTRUMENTS];

	/** Holds a list of the information we have on the data-links */
	CLinkInfo	m_dataLinkInformation[MAX_NUMBER_OF_SCANNING_INSTRUMENTS][MAX_HISTORY];
	int				m_dataLinkIndex[MAX_NUMBER_OF_SCANNING_INSTRUMENTS];

	/** Holds the information about the FTP-uploading data link */
	CLinkInfo m_ftpServerLinkInformation[MAX_HISTORY];
	int				m_nFTPServerLinkInformation;

	// ----------------------------------------------------------------------
	// -------------------- PRIVATE METHODS ---------------------------------
	// ----------------------------------------------------------------------

	/** Clear out old data from the 'm_dataLinkInformation' buffers */
	void RemoveOldLinkInformation();

};
