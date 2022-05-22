#pragma once
#include "afxdialogex.h"


// CRatioScanFilesDialog is page page in the Ratio calculation dialog where the 
// user can run the evaluation and see the result.

class CRatioEvaluationDialog : public CPropertyPage
{
    DECLARE_DYNAMIC(CRatioEvaluationDialog)

public:
    CRatioEvaluationDialog(CWnd* pParent = nullptr);   // standard constructor
    virtual ~CRatioEvaluationDialog();

    /** Initializes the controls and the dialog */
    virtual BOOL OnInitDialog();

    // Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_RATIO_EVALUATE_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()
};
