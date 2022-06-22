#pragma once
#include "afxdialogex.h"

#include "../Graphs/SpectrumGraph.h"

class RatioCalculationController;

// CRatioScanFilesDialog is page page in the Ratio calculation dialog where the 
// user can run the evaluation and see the result.

class CRatioEvaluationDialog : public CPropertyPage
{
    DECLARE_DYNAMIC(CRatioEvaluationDialog)

public:
    CRatioEvaluationDialog(RatioCalculationController* controller, CWnd* pParent = nullptr);   // standard constructor
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

private:

    // The controller which this dialog helps to setup. Notice that this dialog does not own the pointer and will not delete it.
    RatioCalculationController* m_controller;

    // The graph of the SO2 fit
    Graph::CSpectrumGraph m_so2FitGraph;
    CStatic m_fitFrameSO2;

    // The graph of the BrO fit
    Graph::CSpectrumGraph m_broFitGraph;
    CStatic m_fitFrameBrO;
};
