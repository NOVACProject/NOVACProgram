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

    afx_msg void OnChangeSelectedSpecie();

    afx_msg void OnSelchangeRatioShowSelectionList();

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()

private:

    // The controller which this dialog helps to setup. Notice that this dialog does not own the pointer and will not delete it.
    RatioCalculationController* m_controller;

    // Graph type selector
    CListBox m_resultTypeList;

    // The list of species, for the current FitWindow
    CListBox m_so2SpecieList;

    // Displaying the results
    CListBox m_resultList;

    // m_scanGraph is the main graph in the window
    Graph::CDOASFitGraph m_scanGraph;
    CStatic m_scanFrame;

    void UpdateUserInterfaceWithResult();
    void UpdateGraph();
    void UpdateScanGraph();
    void UpdateMajorFitGraph();
    void UpdateMinorFitGraph();
    void UpdateResultList();
    void UpdateListOfReferences();

    void UpdateReferenceResultMajorFit(int indexOfSelectedReference);
    void UpdateReferenceResultMinorFit(int indexOfSelectedReference);

    const novac::DoasResult& GetMajorWindowResult();
    const novac::DoasResult& GetMinorWindowResult();
};
