#pragma once
#include <afxdlgs.h>

#include "../Graphs/SpectrumGraph.h"
#include "../Graphs/DOASFitGraph.h"
#include "../DlgControls/NonFlickeringTreeControl.h"
#include "../DlgControls/NonFlickeringListBoxControl.h"

class RatioCalculationController;
struct RatioCalculationResult;

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

    /** Called when this is selected and becomes active */
    virtual BOOL OnSetActive();

    // Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_RATIO_EVALUATE_DIALOG };
#endif

    afx_msg void OnBnClickedRunEvaluationOnNextScan();
    afx_msg void OnBnClickedRunEvaluationOnAllScans();
    afx_msg void OnBnClickedClearResults();
    afx_msg void OnBnClickedSaveResults();

    afx_msg void OnChangeSelectedSpecie();
    afx_msg void OnChangeEvaluatedScan();

    afx_msg void OnChangeSelectedDoasSpecie();

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()

private:

    // The controller which this dialog helps to setup. Notice that this dialog does not own the pointer and will not delete it.
    RatioCalculationController* m_controller;

    // Boolean flag which is set to true while the processing of the scans is running in the background
    bool m_backgroundProcessingIsRunning = false;

    CWinThread* m_backgroundProcessingThread = nullptr;

    // Graph type selector
    CListBox m_resultTypeList;

    // The list of species, for the current FitWindow
    CNonFlickeringListBoxControl m_doasFitReferencesList;

    // The list of results of the evaluation
    CListBox m_resultsList;

    // A filtering selector for what results to show in m_resultsList
    CComboBox m_resultFilterSelector;

    // The labels showing the column, shift and squeeze for the current specie.
    CStatic m_referenceColumnLabel;
    CStatic m_referenceShiftLabel;
    CStatic m_referenceSqueezeLabel;

    // Displaying the results
    CNonFlickeringTreeControl m_resultTree;

    // m_graph is the main graph in the window
    Graph::CDOASFitGraph m_graph;
    CStatic m_scanFrame;

    LRESULT OnBackgroundProcessingDone(WPARAM wParam, LPARAM lParam);
    LRESULT OnOneRatioEvaluationDone(WPARAM wParam, LPARAM lParam);

    void UpdateUserInterfaceWithResult(RatioCalculationResult* result);
    void UpdateGraph(RatioCalculationResult* result);
    void UpdateScanGraph(RatioCalculationResult* result);
    void UpdateMajorFitGraph(RatioCalculationResult* result);
    void UpdateMinorFitGraph(RatioCalculationResult* result);
    void UpdateCurrentResultTree(const RatioCalculationResult* result);
    void UpdateListOfReferences(const RatioCalculationResult* result);

    void UpdateReferenceResultLabels(const novac::DoasResult* doasResult, int indexOfSelectedReference);

    void UpdateStateWhileBackgroundProcessingIsRunning();

    // returns the last result for the major window. returns nullptr if none exists.
    const novac::DoasResult* GetMajorWindowResult(const RatioCalculationResult* result);

    // returns the last result for the minor window. returns nullptr if none exists.
    const novac::DoasResult* GetMinorWindowResult(const RatioCalculationResult* result);

    // Update the listing of the results.
    void UpdateListOfResults();

    // Adds one element to the list of results, does not clear previously added results.
    void AddToListOfResults(const RatioCalculationResult* result);
public:
    afx_msg void OnSelchangeEvaluatedScansSelectorCombo();
};
