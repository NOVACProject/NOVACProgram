#pragma once

#include "Common/Common.h"

/** The class CWindFileController is responsible for the downloading
		of wind-field files from the FTP-server and occasional re-reading. */
class CWindFileController : public CWinThread
{
public:
	CWindFileController(void);
	~CWindFileController(void);
	DECLARE_DYNCREATE(CWindFileController);
	DECLARE_MESSAGE_MAP()

	// ----------------------------------------------------------------------
	// --------------------- PUBLIC METHODS ---------------------------------
	// ----------------------------------------------------------------------

	/** Called when the thread is to be stopped */
	afx_msg void OnQuit(WPARAM wp, LPARAM lp);

	/** */
	afx_msg void OnTimer(UINT nIDEvent, LPARAM lp);

	/** Called when the thread is starting */
	virtual BOOL InitInstance();

	/** Called when the thread is stopping */
	virtual int ExitInstance();

	/** Called when there's nothing else to do. */
	virtual BOOL OnIdle(LONG lCount);

	// ----------------------------------------------------------------------
	// --------------------- PUBLIC VARIABLES --------------------------------
	// ----------------------------------------------------------------------

	/** Timer */
	UINT_PTR m_nTimerID;

protected:
	// ----------------------------------------------------------------------
	// --------------------- PROTECTED METHODS ------------------------------
	// ----------------------------------------------------------------------

	/** Tries to download the file by connecting to an FTP-server */
	void DownloadFileByFTP();
};
