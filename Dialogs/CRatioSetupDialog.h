#pragma once
#include "afxdialogex.h"


// CRatioScanFilesDialog is the second page in the Ratio calculation dialog
// and makes it possible for the user to setup the properties for the DOAS fits.

class CRatioSetupDialog : public CPropertyPage
{
    DECLARE_DYNAMIC(CRatioSetupDialog)

public:
    CRatioSetupDialog(CWnd* pParent = nullptr);   // standard constructor
    virtual ~CRatioSetupDialog();

    /** Initializes the controls and the dialog */
    virtual BOOL OnInitDialog();

    // Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_RATIO_WINDOW_SETUP_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()
};
