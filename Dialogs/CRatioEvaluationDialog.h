#pragma once
#include "afxdialogex.h"

#include "../Graphs/SpectrumGraph.h"
#include "../Graphs/DOASFitGraph.h"

class RatioCalculationController;

namespace novac
{
    struct DoasResult;
}

// CRatioEvaluationDialog is page page in the Ratio calculation dialog where the 
// user finally runs the configured evaluation and sees the result.

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

    afx_msg void OnBnClickedRunEvaluationOnNextScan();
    afx_msg void OnBnClickedRunEvaluationOnAllScans();

    afx_msg void OnChangeMajorSpecie();
    afx_msg void OnChangeMinorSpecie();

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()

private:

    // The controller which this dialog helps to setup. Notice that this dialog does not own the pointer and will not delete it.
    RatioCalculationController* m_controller;

    // The graph of the SO2 fit
    Graph::CDOASFitGraph m_so2FitGraph;
    CStatic m_fitFrameSO2;
    CListBox m_so2SpecieList;

    // The graph of the BrO fit
    Graph::CDOASFitGraph m_broFitGraph;
    CStatic m_fitFrameBrO;
    CListBox m_broSpecieList;

    // m_scanGraph is the graph of which spectra have been selected in the scan.
    Graph::CGraphCtrl m_scanGraph;
    CStatic m_scanFrame;

    void UpdateUserInterfaceWithResult();
    void UpdateScanGraph();
    void UpdateMajorFitGraph();
    void UpdateMinorFitGraph();
    void UpdateListOfReferences();

    void UpdateFinalRatioLabel();

    void UpdateReferenceResultMajorFit(int indexOfSelectedReference);
    void UpdateReferenceResultMinorFit(int indexOfSelectedReference);

    const novac::DoasResult& GetMajorWindowResult();
    const novac::DoasResult& GetMinorWindowResult();
};
