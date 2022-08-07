// CRatioEvaluationDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "../resource.h"
#include "CRatioEvaluationDialog.h"

#include "../Common/Common.h"

#include <SpectralEvaluation/DialogControllers/RatioCalculationController.h>
#include <SpectralEvaluation/Spectra/Spectrum.h>
#include <SpectralEvaluation/VectorUtils.h>

#include <sstream>

#pragma region General helper methods for simplifying the display of data

void SelectScanAngleAndColumnFromBasicScanEvaluationResult(
    const novac::BasicScanEvaluationResult& evaluationResult,
    const std::vector<int> indicesToSelect,
    double columnOffset,
    std::vector<double>& scanAngles,
    std::vector<double>& columns)
{
    scanAngles.clear();
    columns.clear();

    for (int ii : indicesToSelect)
    {
        const novac::CEvaluationResult& result = evaluationResult.m_spec[ii];
        const novac::CSpectrumInfo& spectrumInfo = evaluationResult.m_specInfo[ii];

        scanAngles.push_back(spectrumInfo.m_scanAngle);
        columns.push_back(result.m_referenceResult[0].m_column - columnOffset);
    }
}

void UpdateDataInDoasFitGraph(Graph::CDOASFitGraph& doasFitGraph, const novac::DoasResult& doasResult)
{
    // The CDoasFitGraph holds its own buffers for drawing the data...

    // copy the fitted cross-sections to a local variable
    const int numberOfReferences = static_cast<int>(doasResult.referenceResult.size());
    for (int k = 0; k < numberOfReferences; ++k)
    {
        // Copy out the scaled values for the reference, but pad with zeros to the left such that the length of the vector equals 'fitHigh'
        doasFitGraph.m_fitResult[k].resize(doasResult.fitHigh);
        std::fill_n(
            begin(doasFitGraph.m_fitResult[k]),
            doasResult.fitLow,
            0.0);

        std::copy(
            begin(doasResult.referenceResult[k].scaledValues),
            end(doasResult.referenceResult[k].scaledValues),
            begin(doasFitGraph.m_fitResult[k]) + doasResult.fitLow);

        doasFitGraph.m_specieName[k] = doasResult.referenceResult[k].name;
    }
    doasFitGraph.m_nReferences = numberOfReferences;

    // also copy the residual, in the same way
    doasFitGraph.m_residual.resize(doasResult.fitHigh);
    std::fill_n(
        begin(doasFitGraph.m_residual),
        doasResult.fitLow,
        0.0);

    std::copy(
        begin(doasResult.residual),
        end(doasResult.residual),
        begin(doasFitGraph.m_residual) + doasResult.fitLow);

    doasFitGraph.m_fitLow = doasResult.fitLow;
    doasFitGraph.m_fitHigh = doasResult.fitHigh;
}

void UpdateNamesOfReferencesInListBox(CListBox& destination, const novac::DoasResult& doasResult)
{
    destination.ResetContent();
    const int numberOfReferences = static_cast<int>(doasResult.referenceResult.size());
    for (int k = 0; k < numberOfReferences; ++k)
    {
        CString name = (LPCSTR)doasResult.referenceResult[k].name.c_str();
        destination.AddString(name);
    }
}



#pragma endregion

// CRatioEvaluationDialog dialog

IMPLEMENT_DYNAMIC(CRatioEvaluationDialog, CPropertyPage)

CRatioEvaluationDialog::CRatioEvaluationDialog(RatioCalculationController* controller, CWnd* pParent /*=nullptr*/)
    : CPropertyPage(IDD_RATIO_EVALUATE_DIALOG), m_controller(controller)
{
}

CRatioEvaluationDialog::~CRatioEvaluationDialog()
{
    m_controller = nullptr;
}

BOOL CRatioEvaluationDialog::OnInitDialog() {
    CPropertyPage::OnInitDialog();

    Common common;

    // Initialize the SO2 fit graph.
    CRect rect;
    int margin = -8;
    m_fitFrameSO2.GetWindowRect(&rect);
    rect.bottom -= rect.top + margin;
    rect.right -= rect.left + margin;
    rect.top = margin + 10;
    rect.left = margin;
    m_so2FitGraph.Create(WS_VISIBLE | WS_CHILD, rect, &m_fitFrameSO2);
    m_so2FitGraph.SetRange(0, MAX_SPECTRUM_LENGTH, 1, 0.0, 4095.0, 1);
    m_so2FitGraph.SetYUnits(common.GetString(AXIS_INTENSITY));
    m_so2FitGraph.SetXUnits(common.GetString(AXIS_CHANNEL));
    m_so2FitGraph.SetBackgroundColor(RGB(0, 0, 0));
    m_so2FitGraph.SetGridColor(RGB(255, 255, 255));
    m_so2FitGraph.SetPlotColor(RGB(0, 255, 0));
    // m_so2FitGraph.parentWnd = this;

    // Initialize the BrO fit graph
    m_fitFrameBrO.GetWindowRect(&rect);
    rect.bottom -= rect.top + margin;
    rect.right -= rect.left + margin;
    rect.top = margin + 10;
    rect.left = margin;
    m_broFitGraph.Create(WS_VISIBLE | WS_CHILD, rect, &m_fitFrameBrO);
    m_broFitGraph.SetRange(0, MAX_SPECTRUM_LENGTH, 1, 0.0, 4095.0, 1);
    m_broFitGraph.SetYUnits(common.GetString(AXIS_INTENSITY));
    m_broFitGraph.SetXUnits(common.GetString(AXIS_CHANNEL));
    m_broFitGraph.SetBackgroundColor(RGB(0, 0, 0));
    m_broFitGraph.SetGridColor(RGB(255, 255, 255));
    m_broFitGraph.SetPlotColor(RGB(0, 255, 0));
    // m_broFitGraph.parentWnd = this;

    // Initialize the graph showing the SO2 columns from the initial evaluation
    m_scanFrame.GetWindowRect(&rect);
    rect.bottom -= rect.top + margin;
    rect.right -= rect.left + margin;
    rect.top = margin + 10;
    rect.left = margin;
    m_scanGraph.Create(WS_VISIBLE | WS_CHILD, rect, &m_scanFrame);
    m_scanGraph.SetRange(-90, 90, 0, 0, 100, 0);
    m_scanGraph.SetXUnits(common.GetString(AXIS_ANGLE));
    m_scanGraph.SetYUnits("Column [ppmm]");
    m_scanGraph.SetBackgroundColor(RGB(0, 0, 0));
    m_scanGraph.SetGridColor(RGB(255, 255, 255));
    m_scanGraph.SetPlotColor(RGB(255, 0, 0));
    // m_scanGraph.parentWnd = this;

    return TRUE;  // return TRUE unless you set the focus to a control
}

void CRatioEvaluationDialog::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_FIT_FRAME_SO2, m_fitFrameSO2);
    DDX_Control(pDX, IDC_FIT_FRAME_BRO, m_fitFrameBrO);
    DDX_Control(pDX, IDC_FIT_FRAME_SCAN, m_scanFrame);
    DDX_Control(pDX, IDC_MAJOR_SPECIE_LIST, m_so2SpecieList);
    DDX_Control(pDX, IDC_MINOR_SPECIE_LIST, m_broSpecieList);
}


BEGIN_MESSAGE_MAP(CRatioEvaluationDialog, CPropertyPage)
    ON_BN_CLICKED(IDC_RATIO_RUN_EVALUATION_NEXT_SCAN, &CRatioEvaluationDialog::OnBnClickedRunEvaluationOnNextScan)
    ON_BN_CLICKED(IDC_RATIO_RUN_EVALUATION_NEXT_SCAN, &CRatioEvaluationDialog::OnBnClickedRunEvaluationOnAllScans)

    ON_LBN_SELCHANGE(IDC_MAJOR_SPECIE_LIST, OnChangeMajorSpecie)
    ON_LBN_SELCHANGE(IDC_MINOR_SPECIE_LIST, OnChangeMinorSpecie)

END_MESSAGE_MAP()

// CRatioEvaluationDialog message handlers

void CRatioEvaluationDialog::OnChangeMajorSpecie()
{
    UpdateMajorFitGraph();
}

void CRatioEvaluationDialog::OnChangeMinorSpecie()
{
    UpdateMinorFitGraph();
}

void CRatioEvaluationDialog::UpdateUserInterfaceWithResult()
{
    UpdateScanGraph();
    UpdateListOfReferences();
    UpdateMajorFitGraph();
    UpdateMinorFitGraph();
    UpdateFinalRatioLabel();
}

const novac::DoasResult& CRatioEvaluationDialog::GetMajorWindowResult()
{
    return m_controller->m_lastResult.debugInfo.doasResults[0]; // First result is for SO2
}

const novac::DoasResult& CRatioEvaluationDialog::GetMinorWindowResult()
{
    return m_controller->m_lastResult.debugInfo.doasResults[1]; // Second result is for BrO
}

void CRatioEvaluationDialog::UpdateScanGraph()
{
    // Draw the evaluated columns vs scan-angle, highlighting the selected in-plume and out-of-plume spectra.
    m_scanGraph.CleanPlot();

    const double columnOffset = m_controller->m_lastResult.plumeInScanProperties.offset;

    std::vector<double> scanAngles;
    std::vector<double> columns;
    std::vector<double> columnErrors;
    for (int ii = 0; ii < static_cast<int>(m_controller->m_lastResult.initialEvaluation.m_spec.size()); ++ii)
    {
        const novac::CEvaluationResult& result = m_controller->m_lastResult.initialEvaluation.m_spec[ii];
        const novac::CSpectrumInfo& spectrumInfo = m_controller->m_lastResult.initialEvaluation.m_specInfo[ii];

        scanAngles.push_back(spectrumInfo.m_scanAngle);
        columns.push_back(result.m_referenceResult[0].m_column - columnOffset);
        columnErrors.push_back(result.m_referenceResult[0].m_columnError);
    }

    // All the column-values
    m_scanGraph.SetRange(
        Min(scanAngles),
        Max(scanAngles),
        1,
        Min(columns),
        Max(columns),
        0);
    m_scanGraph.SetPlotColor(RGB(128, 0, 0));
    m_scanGraph.BarChart(scanAngles.data(), columns.data(), columnErrors.data(), static_cast<int>(scanAngles.size()), Graph::CGraphCtrl::PLOT_FIXED_AXIS | Graph::CGraphCtrl::PLOT_BAR_SHOW_X);

    // The selected in-plume column-values
    std::vector<double> inPlumeScanAngles;
    std::vector<double> inPlumeColumns;
    SelectScanAngleAndColumnFromBasicScanEvaluationResult(
        m_controller->m_lastResult.initialEvaluation,
        m_controller->m_lastResult.debugInfo.plumeSpectra,
        columnOffset,
        inPlumeScanAngles,
        inPlumeColumns);

    m_scanGraph.SetPlotColor(RGB(0, 255, 0));
    m_scanGraph.BarChart(inPlumeScanAngles.data(), inPlumeColumns.data(), static_cast<int>(inPlumeColumns.size()), Graph::CGraphCtrl::PLOT_FIXED_AXIS | Graph::CGraphCtrl::PLOT_BAR_SHOW_X);

    // The selected out-of-plume column-values
    std::vector<double> outOfPlumeScanAngles;
    std::vector<double> outOfPlumeColumns;
    SelectScanAngleAndColumnFromBasicScanEvaluationResult(
        m_controller->m_lastResult.initialEvaluation,
        m_controller->m_lastResult.debugInfo.outOfPlumeSpectra,
        columnOffset,
        outOfPlumeScanAngles,
        outOfPlumeColumns);

    m_scanGraph.SetPlotColor(RGB(255, 0, 0));
    m_scanGraph.BarChart(outOfPlumeScanAngles.data(), outOfPlumeColumns.data(), static_cast<int>(outOfPlumeScanAngles.size()), Graph::CGraphCtrl::PLOT_FIXED_AXIS | Graph::CGraphCtrl::PLOT_BAR_SHOW_X);
}

void CRatioEvaluationDialog::UpdateListOfReferences()
{
    UpdateNamesOfReferencesInListBox(m_so2SpecieList, GetMajorWindowResult());
    UpdateNamesOfReferencesInListBox(m_broSpecieList, GetMinorWindowResult());
}

void CRatioEvaluationDialog::UpdateFinalRatioLabel()
{
    CString ratioStr;

    ratioStr.Format("Calculated SO2/BrO Ratio: %.2G ± %.2G", m_controller->m_lastResult.ratio.ratio, m_controller->m_lastResult.ratio.error);

    SetDlgItemText(IDC_FINAL_RATIO_LBL_COLUMN, ratioStr);
}

void CRatioEvaluationDialog::UpdateReferenceResultMajorFit(int indexOfSelectedReference)
{
    const novac::DoasResult& doasResult = GetMajorWindowResult();

    double column = doasResult.referenceResult[indexOfSelectedReference].column;
    double columnError = doasResult.referenceResult[indexOfSelectedReference].columnError;
    double shift = doasResult.referenceResult[indexOfSelectedReference].shift;
    double shiftError = doasResult.referenceResult[indexOfSelectedReference].shiftError;
    double squeeze = doasResult.referenceResult[indexOfSelectedReference].squeeze;
    double squeezeError = doasResult.referenceResult[indexOfSelectedReference].squeezeError;

    CString columnStr, shiftStr, squeezeStr, deltaStr, chi2Str;
    columnStr.Format("Column %.2G ± %.2G", column, columnError);
    shiftStr.Format("Shift: %.2lf ± %.2lf", shift, shiftError);
    squeezeStr.Format("Squeeze: %.2lf ± %.2lf", squeeze, squeezeError);
    deltaStr.Format("Delta: %.2e", doasResult.delta);
    chi2Str.Format("Chi²: %.2e", doasResult.chiSquare);

    SetDlgItemText(IDC_RATIO_MAJOR_LBL_COLUMN, columnStr);
    SetDlgItemText(IDC_RATIO_MAJOR_LBL_SHIFT, shiftStr);
    SetDlgItemText(IDC_RATIO_MAJOR_LBL_SQUEEZE, squeezeStr);
    SetDlgItemText(IDC_RATIO_MAJOR_LBL_DELTA, deltaStr);
    SetDlgItemText(IDC_RATIO_MAJOR_LBL_CHISQUARE, chi2Str);
}

void CRatioEvaluationDialog::UpdateReferenceResultMinorFit(int indexOfSelectedReference)
{
    const novac::DoasResult& doasResult = GetMinorWindowResult();

    double column = doasResult.referenceResult[indexOfSelectedReference].column;
    double columnError = doasResult.referenceResult[indexOfSelectedReference].columnError;
    double shift = doasResult.referenceResult[indexOfSelectedReference].shift;
    double shiftError = doasResult.referenceResult[indexOfSelectedReference].shiftError;
    double squeeze = doasResult.referenceResult[indexOfSelectedReference].squeeze;
    double squeezeError = doasResult.referenceResult[indexOfSelectedReference].squeezeError;

    CString columnStr, shiftStr, squeezeStr, deltaStr, chi2Str;
    columnStr.Format("Column %.2G ± %.2G", column, columnError);
    shiftStr.Format("Shift: %.2lf ± %.2lf", shift, shiftError);
    squeezeStr.Format("Squeeze: %.2lf ± %.2lf", squeeze, squeezeError);
    deltaStr.Format("Delta: %.2e", doasResult.delta);
    chi2Str.Format("Chi²: %.2e", doasResult.chiSquare);

    SetDlgItemText(IDC_RATIO_MINOR_LBL_COLUMN, columnStr);
    SetDlgItemText(IDC_RATIO_MINOR_LBL_SHIFT, shiftStr);
    SetDlgItemText(IDC_RATIO_MINOR_LBL_SQUEEZE, squeezeStr);
    SetDlgItemText(IDC_RATIO_MINOR_LBL_DELTA, deltaStr);
    SetDlgItemText(IDC_RATIO_MINOR_LBL_CHISQUARE, chi2Str);
}

void CRatioEvaluationDialog::UpdateMajorFitGraph()
{
    // The reference that we shall draw
    int refIndex = m_so2SpecieList.GetCurSel();
    if (refIndex < 0)
    {
        m_so2SpecieList.SetCurSel(0);
        refIndex = 0;
    }

    UpdateDataInDoasFitGraph(m_so2FitGraph, GetMajorWindowResult());

    m_so2FitGraph.DrawFit(refIndex);

    UpdateReferenceResultMajorFit(refIndex);
}

void CRatioEvaluationDialog::UpdateMinorFitGraph()
{
    // The reference that we shall draw
    int refIndex = m_broSpecieList.GetCurSel();
    if (refIndex < 0)
    {
        m_broSpecieList.SetCurSel(0);
        refIndex = 0;
    }

    UpdateDataInDoasFitGraph(m_broFitGraph, GetMinorWindowResult());

    m_broFitGraph.DrawFit(refIndex);

    UpdateReferenceResultMinorFit(refIndex);
}

void CRatioEvaluationDialog::OnBnClickedRunEvaluationOnNextScan()
{
    try
    {
        if (m_controller->HasMoreScansToEvaluate())
        {
            // TODO: SetupFitWindows shouldn't have to be done for each scan
            const auto windows = m_controller->SetupFitWindows();

            m_controller->EvaluateNextScan(windows);

            UpdateUserInterfaceWithResult();
        }
    }
    catch (std::exception& ex)
    {
        std::stringstream message;
        message << "Ratio calculation failed. Error: " << ex.what();
        MessageBox(message.str().c_str());
    }
}

void CRatioEvaluationDialog::OnBnClickedRunEvaluationOnAllScans()
{
    // TODO: Add your control notification handler code here
}
