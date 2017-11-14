// NovacMasterProgram.h : main header file for the NovacMasterProgram application
//
#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols


// CNovacMasterProgramApp:
// See NovacMasterProgram.cpp for the implementation of this class
//

class CNovacMasterProgramApp : public CWinApp
{
public:
	CNovacMasterProgramApp();


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
  virtual BOOL OnIdle(LONG lCount);
};

extern CNovacMasterProgramApp theApp;