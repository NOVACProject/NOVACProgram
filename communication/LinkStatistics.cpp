#include "StdAfx.h"
#include "linkstatistics.h"
#include <SpectralEvaluation/DateTime.h>

using namespace Communication;
using namespace novac;


CLinkStatistics::CTransferInfo::CTransferInfo(){
	this->day		= 0;
	this->hour		= 0;
	this->minute	= 0;
	this->second	= 0;
	this->speed		= 0.0;
	this->success	= true;
}
CLinkStatistics::CTransferInfo::~CTransferInfo(){
}

CLinkStatistics::CLinkStatistics(void)
{
	Clear();
}

CLinkStatistics::~CLinkStatistics(void)
{
	Clear();
}

void CLinkStatistics::Clear(){
	this->m_downloads.RemoveAll();
	this->m_uploads.RemoveAll();
}

// -------------------- Retrieving data -----------------------
/** Getting the successrate for the number of downloads.
		@return the portion of number of attempts to download files
			that have succeeded (0 -> 1) */
double CLinkStatistics::GetDownloadSuccessRate() const{
	return GetSuccessRate(m_downloads);
}

/** Getting the average download speed for this link [kb/s] */
double CLinkStatistics::GetAveragedDownloadSpeed() const{
	return GetAverageSpeed(m_downloads);
}

/** Returns the number of successful downloads today on this link */
long CLinkStatistics::GetDownloadNum() const{
	return GetSuccessfulNum(m_downloads);
}

/** Getting the successrate for the number of uploads
		@return the portion of number of attempts to upload files
			that have succeeded (0 -> 1) */
double CLinkStatistics::GetUploadSuccessRate() const{
	return GetSuccessRate(m_uploads);
}

/** Getting the average upload speed for this link [kb/s] */
double CLinkStatistics::GetAveragedUploadSpeed() const{
	return GetAverageSpeed(m_uploads);
}

/** Returns the number of successful uploads today on this link */
long CLinkStatistics::GetUploadNum() const{
	return GetSuccessfulNum(m_uploads);
}

// ----------------------- Adding data -------------------------
/** Append one download-speed to the history,
		this will also append one successfull download to the statistics */
void CLinkStatistics::AppendDownloadSpeed(double speed){
	AppendSuccessfulTransfer(speed, m_downloads);
}

/** Append one failed download to the history */
void CLinkStatistics::AppendFailedDownload(){
	AppendFailedTransfer(m_downloads);
}

/** Append one upload-speed to the history
		this will also append one successfull upload to the statistics */
void CLinkStatistics::AppendUploadSpeed(double speed){
	AppendSuccessfulTransfer(speed, m_uploads);
}

/** Append one failed upload to the history */
void CLinkStatistics::AppendFailedUpload(){
	AppendFailedTransfer(m_uploads);
}

// ------------- Protected methods ---------------

/** Trim the list of transfer-data so that they only contain data from
		today... */
void	CLinkStatistics::TrimLists(){
	// Get the current time and date
	CDateTime now;
	now.SetToNow();

	// the data is sorted in order of appendance, thus wee only need to check the
	//	first items in the list. If we find one element which is from today then
	//	all elements after it will also be from today...

	// 1. The download list
	POSITION dPos = m_downloads.GetHeadPosition();
	while(dPos != NULL){
		CTransferInfo &info = m_downloads.GetNext(dPos);

		if(info.day == now.day)
			break;

		m_downloads.RemoveHead();
		dPos = m_downloads.GetHeadPosition();
	}

	// 2. The upload list
	POSITION uPos = m_uploads.GetHeadPosition();
	while(uPos != NULL){
		CTransferInfo &info = m_uploads.GetNext(uPos);

		if(info.day == now.day)
			break;

		m_uploads.RemoveHead();
		uPos = m_uploads.GetHeadPosition();
	}
}

/** Retrieves the success-rate from the given list */
double CLinkStatistics::GetSuccessRate(const CList <CTransferInfo, CTransferInfo&>	&list) const{
	long nSuccess = 0;
	long nFailure	= 0;

	POSITION pos = list.GetHeadPosition();
	while(pos != NULL){
		const CTransferInfo &info = list.GetNext(pos);

		if(info.success)
			++nSuccess;
		else
			++nFailure;
	}

	// If there were no transfers, then return 0
	if(nSuccess == 0 && nFailure == 0)
		return 0.0;

	return nSuccess / (double)(nSuccess + nFailure);
}

/** Appends the given data-speed to the given list */
void	CLinkStatistics::AppendSuccessfulTransfer(double speed, CList <CTransferInfo, CTransferInfo&>	&list){
	// First of all, trim the list so that we don't have too many strange data here...
	TrimLists();

	// Get the current time and date
	CDateTime now;
	now.SetToNow();

	// Make a new transfer-info-object
	CTransferInfo info;
	info.speed		= speed;
	info.day		= now.day;
	info.hour		= now.hour;
	info.minute		= now.minute;
	info.second		= now.second;
	info.success	= true;

	// Append the item to the list
	list.AddTail(info);

	// We're done!
	return;
}

/** Appends a failure to the given list */
void CLinkStatistics::AppendFailedTransfer(CList <CTransferInfo, CTransferInfo&>	&list){
	// First of all, trim the list so that we don't have too many strange data here...
	TrimLists();

	// Get the current time and date
	CDateTime now;
	now.SetToNow();

	// Make a new transfer-info-object
	CTransferInfo info;
	info.speed		= 0.0;
	info.day		= now.day;
	info.hour		= now.hour;
	info.minute		= now.minute;
	info.second		= now.second;
	info.success	= false;

	// Append the item to the list
	list.AddTail(info);

	// We're done!
	return;
}

/** Gets the average speed from the given list */
double	CLinkStatistics::GetAverageSpeed(const CList <CTransferInfo, CTransferInfo&>	&list) const{
	double avgSpeed		= 0.0;
	long   nTransfers	= 0;

	POSITION pos = list.GetHeadPosition();
	while(pos != NULL){
		const CTransferInfo &info = list.GetNext(pos);

		if(info.success && info.speed > 0){
			++nTransfers;
			avgSpeed += info.speed;
		}
	}

	return (avgSpeed / (double)nTransfers);
}

/** Returns the number of successful transfers today on this link */
long CLinkStatistics::GetSuccessfulNum(const CList <CTransferInfo, CTransferInfo&>	&list) const{
	long nSuccess = 0;

	POSITION pos = list.GetHeadPosition();
	while(pos != NULL){
		const CTransferInfo &info = list.GetNext(pos);

		if(info.success)
			++nSuccess;
	}

	return nSuccess;
}
