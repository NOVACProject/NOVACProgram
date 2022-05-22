#pragma once
#include "afxdialogex.h"


// CRatioScanFilesDialog is the first page in the Ratio calculation dialog
// and makes it possible for the user to select a number of scan-files (.pak) to use in the calculations.

class CRatioScanFilesDialog : public CPropertyPage
{
    DECLARE_DYNAMIC(CRatioScanFilesDialog)

public:
    CRatioScanFilesDialog(CWnd* pParent = nullptr);   // standard constructor
    virtual ~CRatioScanFilesDialog();

    /** Initializes the controls and the dialog */
    virtual BOOL OnInitDialog();

    // Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_RATIO_SCANFILES_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()
};
