#include "stdafx.h"
#include "../resource.h"
#include "CRatioEvaluationDialog.h"

#include "../Common/Common.h"

#include <SpectralEvaluation/DialogControllers/RatioCalculationController.h>
#include <SpectralEvaluation/Spectra/Spectrum.h>
#include <SpectralEvaluation/File/File.h>
#include <SpectralEvaluation/VectorUtils.h>

#include <sstream>

#pragma region General helper methods for simplifying the display of data

void SelectScanAngleAndColumnFromBasicScanEvaluationResult(
    const novac::BasicScanEvaluationResult& evaluationResult,
    const std::vector<int>& indicesToSelect,
    double columnOffset,
    std::vector<double>& scanAngles,
    std::vector<double>& columns)
{
    scanAngles.resize(evaluationResult.m_spec.size());
    columns.resize(evaluationResult.m_spec.size());

    for (size_t ii = 0; ii < evaluationResult.m_spec.size(); ++ii)
    {
        const novac::CSpectrumInfo& spectrumInfo = evaluationResult.m_specInfo[ii];
        scanAngles[ii] = spectrumInfo.m_scanAngle;

        if (std::find(begin(indicesToSelect), end(indicesToSelect), static_cast<int>(ii)) != indicesToSelect.end())
        {
            const novac::CEvaluationResult& result = evaluationResult.m_spec[ii];
            columns[ii] = result.m_referenceResult[0].m_column - columnOffset;
        }
        else
        {
            columns[ii] = 0.0;
        }
    }
}


void SelectScanAngleAndColumnFromBasicScanEvaluationResult(
    const novac::BasicScanEvaluationResult& evaluationResult,
    const std::vector<std::pair<int, std::string>>& indicesToSelect,
    double columnOffset,
    std::vector<double>& scanAngles,
    std::vector<double>& columns)
{
    std::vector<int> indices;
    for (const auto& v : indicesToSelect)
    {
        indices.push_back(v.first);
    }

    return SelectScanAngleAndColumnFromBasicScanEvaluationResult(evaluationResult, indices, columnOffset, scanAngles, columns);
}

void UpdateDataInDoasFitGraph(Graph::CDOASFitGraph& doasFitGraph, const novac::DoasResult* doasResult)
{
    if (doasResult == nullptr)
    {
        doasFitGraph.m_nReferences = 0;
        return;
    }

    // The CDoasFitGraph holds its own buffers for drawing the data. Just save the data in these buffers.

    // copy the fitted cross-sections to a local variable
    const int numberOfReferences = static_cast<int>(doasResult->referenceResult.size());
    for (int k = 0; k < numberOfReferences; ++k)
    {
        // Copy out the scaled values for the reference, but pad with zeros to the left such that the length of the vector equals 'fitHigh'
        doasFitGraph.m_fitResult[k].resize(doasResult->fitHigh);
        std::fill_n(
            begin(doasFitGraph.m_fitResult[k]),
            doasResult->fitLow,
            0.0);

        std::copy(
            begin(doasResult->referenceResult[k].scaledValues),
            end(doasResult->referenceResult[k].scaledValues),
            begin(doasFitGraph.m_fitResult[k]) + doasResult->fitLow);

        doasFitGraph.m_specieName[k] = doasResult->referenceResult[k].name;
    }
    doasFitGraph.m_nReferences = numberOfReferences;

    // also copy the residual, in the same way
    doasFitGraph.m_residual.resize(doasResult->fitHigh);
    std::fill_n(
        begin(doasFitGraph.m_residual),
        doasResult->fitLow,
        0.0);

    std::copy(
        begin(doasResult->residual),
        end(doasResult->residual),
        begin(doasFitGraph.m_residual) + doasResult->fitLow);

    doasFitGraph.m_fitLow = doasResult->fitLow;
    doasFitGraph.m_fitHigh = doasResult->fitHigh;
}

void UpdateNamesOfReferencesInListBox(CListBox& destination, const novac::DoasResult* doasResult)
{
    destination.ResetContent();
    if (doasResult != nullptr)
    {
        const int numberOfReferences = static_cast<int>(doasResult->referenceResult.size());
        for (int k = 0; k < numberOfReferences; ++k)
        {
            CString name = (LPCSTR)doasResult->referenceResult[k].name.c_str();
            destination.AddString(name);
        }
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
    CRect rect;
    int margin = -8;

    // Initialize the graph showing the SO2 columns from the initial evaluation
    m_scanFrame.GetWindowRect(&rect);
    rect.bottom -= rect.top + margin;
    rect.right -= rect.left + margin;
    rect.top = margin + 10;
    rect.left = margin;
    m_graph.Create(WS_VISIBLE | WS_CHILD, rect, &m_scanFrame);
    m_graph.SetRange(-90, 90, 0, 0, 100, 0);
    m_graph.SetXUnits(common.GetString(AXIS_ANGLE));
    m_graph.SetYUnits("Column [ppmm]");
    m_graph.SetBackgroundColor(RGB(0, 0, 0));
    m_graph.SetGridColor(RGB(255, 255, 255));
    m_graph.SetPlotColor(RGB(255, 0, 0));

    // Initialize the list of graphs to show
    m_resultTypeList.AddString("Scan");
    m_resultTypeList.AddString("SO2 Fit");
    m_resultTypeList.AddString("BrO Fit");
    m_resultTypeList.SetCurSel(0);

    return TRUE;  // return TRUE unless you set the focus to a control
}

void CRatioEvaluationDialog::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_FIT_FRAME_SCAN, m_scanFrame);
    DDX_Control(pDX, IDC_MAJOR_SPECIE_LIST, m_so2SpecieList);
    DDX_Control(pDX, IDC_RATIO_SHOW_SELECTION_LIST, m_resultTypeList);
    DDX_Control(pDX, IDC_RATIO_RESULT_TREE, m_resultTree);
    DDX_Control(pDX, IDC_RATIO_MAJOR_LBL_COLUMN, m_referenceColumnLabel);
    DDX_Control(pDX, IDC_RATIO_MAJOR_LBL_SHIFT, m_referenceShiftLabel);
    DDX_Control(pDX, IDC_RATIO_MAJOR_LBL_SQUEEZE, m_referenceSqueezeLabel);
    DDX_Control(pDX, IDC_LBL_SHOWING, m_referenceHeaderLabel);
}

BEGIN_MESSAGE_MAP(CRatioEvaluationDialog, CPropertyPage)
    ON_BN_CLICKED(IDC_RATIO_RUN_EVALUATION_NEXT_SCAN, &CRatioEvaluationDialog::OnBnClickedRunEvaluationOnNextScan)
    ON_BN_CLICKED(IDC_RATIO_RUN_EVALUATION, &CRatioEvaluationDialog::OnBnClickedRunEvaluationOnAllScans)
    ON_BN_CLICKED(IDC_RATIO_SAVE_RESULTS, &CRatioEvaluationDialog::OnBnClickedSaveResults)

    ON_MESSAGE(WM_DONE, OnBackgroundProcessingDone)
    ON_MESSAGE(WM_PROGRESS, OnOneRatioEvaluationDone)

    ON_LBN_SELCHANGE(IDC_MAJOR_SPECIE_LIST, OnChangeSelectedSpecie)
    ON_LBN_SELCHANGE(IDC_RATIO_SHOW_SELECTION_LIST, &CRatioEvaluationDialog::OnChangeSelectedDoasSpecie)
END_MESSAGE_MAP()

// CRatioEvaluationDialog message handlers

void CRatioEvaluationDialog::OnChangeSelectedSpecie()
{
    const int currentGraph = m_resultTypeList.GetCurSel();

    if (m_controller->m_results.empty())
    {
        return;
    }

    auto lastResult = m_controller->m_results.back();

    if (currentGraph == 1) // SO2
    {
        UpdateMajorFitGraph(lastResult);
    }
    else if (currentGraph == 2) // BrO
    {
        UpdateMinorFitGraph(lastResult);
    }
}

LRESULT CRatioEvaluationDialog::OnBackgroundProcessingDone(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    // Re-enable the UI buttons
    GetDlgItem(IDC_RATIO_RUN_EVALUATION_NEXT_SCAN)->EnableWindow(TRUE);
    GetDlgItem(IDC_RATIO_RUN_EVALUATION)->EnableWindow(TRUE);
    GetDlgItem(IDC_RATIO_SAVE_RESULTS)->EnableWindow(TRUE);

    return 0;
}

LRESULT CRatioEvaluationDialog::OnOneRatioEvaluationDone(WPARAM wParam, LPARAM /*lParam*/)
{
    RatioCalculationResult* newResult = reinterpret_cast<RatioCalculationResult*>(wParam);
    if (newResult == nullptr)
    {
        return 0;
    }

    UpdateUserInterfaceWithResult(*newResult);

    return 0;
}

void CRatioEvaluationDialog::UpdateUserInterfaceWithResult(RatioCalculationResult& result)
{
    if (!result.debugInfo.errorMessage.empty())
    {
        // If there was an error in the evaluation, then don't attemp to show all the references.
        m_resultTypeList.ResetContent();
        m_resultTypeList.AddString("Scan");
        m_resultTypeList.SetCurSel(0);

        GetDlgItem(IDC_RATIO_ERROR_MESSAGE_LABEL)->ShowWindow(SW_SHOW);
        SetDlgItemText(IDC_RATIO_ERROR_MESSAGE_LABEL, result.debugInfo.errorMessage.c_str());
    }
    else
    {
        if (m_resultTypeList.GetCount() != 3)
        {
            m_resultTypeList.ResetContent();
            m_resultTypeList.AddString("Scan");
            m_resultTypeList.AddString("SO2 Fit");
            m_resultTypeList.AddString("BrO Fit");
            m_resultTypeList.SetCurSel(0);
        }
        GetDlgItem(IDC_RATIO_ERROR_MESSAGE_LABEL)->ShowWindow(SW_HIDE);
    }

    UpdateGraph(result);
    UpdateResultList(result);
    UpdateListOfReferences(result);
    UpdateResultList(result);

    {
        CString progressStr;
        progressStr.Format("Evaluated scan %d out of %d", m_controller->CurrentPakFileIndex(), m_controller->NumberOfPakFilesInSetup());
        SetDlgItemText(IDC_RATIO_PROGRESS_LABEL, progressStr);
    }
}

const novac::DoasResult* CRatioEvaluationDialog::GetMajorWindowResult(const RatioCalculationResult& result)
{
    if (result.debugInfo.doasResults.empty())
    {
        return nullptr;
    }
    return &result.debugInfo.doasResults[0]; // First result is for SO2
}

const novac::DoasResult* CRatioEvaluationDialog::GetMinorWindowResult(const RatioCalculationResult& result)
{
    if (result.debugInfo.doasResults.size() < 2)
    {
        return nullptr;
    }
    return &result.debugInfo.doasResults[1]; // Second result is for BrO
}

void CRatioEvaluationDialog::UpdateGraph(RatioCalculationResult& result)
{
    const int currentGraph = m_resultTypeList.GetCurSel();
    if (currentGraph == 0)
    {
        UpdateScanGraph(result);
    }
    else if (currentGraph == 1)
    {
        UpdateMajorFitGraph(result);
    }
    else if (currentGraph == 2)
    {
        UpdateMinorFitGraph(result);
    }
}

void CRatioEvaluationDialog::UpdateScanGraph(RatioCalculationResult& result)
{
    // Draw the evaluated columns vs scan-angle, highlighting the selected in-plume and out-of-plume spectra.
    m_graph.CleanPlot();
    m_graph.SetXUnits("Angle [deg]");
    if (m_controller->m_crossSectionUnit == novac::CrossSectionUnit::ppmm)
    {
        m_graph.SetYUnits("Column [ppmm]");
    }
    else
    {
        m_graph.SetYUnits("Column [molec/cm2]");
    }

    if (m_controller->m_results.empty())
    {
        return;
    }

    const double columnOffset = result.plumeInScanProperties.offset;

    std::vector<double> scanAngles;
    std::vector<double> columns;
    std::vector<double> columnErrors;
    std::vector<double> peakSaturation;
    for (int ii = 0; ii < static_cast<int>(result.initialEvaluation.m_spec.size()); ++ii)
    {
        const novac::CEvaluationResult& initialEvaluation = result.initialEvaluation.m_spec[ii];
        const novac::CSpectrumInfo& spectrumInfo = result.initialEvaluation.m_specInfo[ii];

        scanAngles.push_back(spectrumInfo.m_scanAngle);
        columns.push_back(initialEvaluation.m_referenceResult[0].m_column - columnOffset);
        columnErrors.push_back(initialEvaluation.m_referenceResult[0].m_columnError);
        peakSaturation.push_back(100.0 * spectrumInfo.m_peakIntensity / (double)(spectrumInfo.m_numSpec * result.debugInfo.spectrometerFullDynamicRange));
    }

    // All the column-values
    m_graph.SetRange(
        Min(scanAngles),
        Max(scanAngles),
        1,
        Min(columns),
        Max(columns),
        0);
    m_graph.SetPlotColor(RGB(255, 0, 0)); // red
    m_graph.BarChart(scanAngles.data(), columns.data(), columnErrors.data(), static_cast<int>(scanAngles.size()), Graph::CGraphCtrl::PLOT_FIXED_AXIS | Graph::CGraphCtrl::PLOT_BAR_SHOW_X);

    // The selected in-plume column-values
    std::vector<double> inPlumeScanAngles;
    std::vector<double> inPlumeColumns;
    SelectScanAngleAndColumnFromBasicScanEvaluationResult(
        result.initialEvaluation,
        result.debugInfo.plumeSpectrumIndices,
        columnOffset,
        inPlumeScanAngles,
        inPlumeColumns);

    m_graph.SetPlotColor(RGB(0, 255, 0)); // green
    m_graph.BarChart(inPlumeScanAngles.data(), inPlumeColumns.data(), static_cast<int>(inPlumeColumns.size()), Graph::CGraphCtrl::PLOT_FIXED_AXIS | Graph::CGraphCtrl::PLOT_BAR_SHOW_X);

    // The selected out-of-plume column-values
    std::vector<double> outOfPlumeScanAngles;
    std::vector<double> outOfPlumeColumns;
    SelectScanAngleAndColumnFromBasicScanEvaluationResult(
        result.initialEvaluation,
        result.debugInfo.outOfPlumeSpectrumIndices,
        columnOffset,
        outOfPlumeScanAngles,
        outOfPlumeColumns);

    m_graph.SetPlotColor(RGB(0, 128, 0)); // dark greens
    m_graph.BarChart(outOfPlumeScanAngles.data(), outOfPlumeColumns.data(), static_cast<int>(outOfPlumeScanAngles.size()), Graph::CGraphCtrl::PLOT_FIXED_AXIS | Graph::CGraphCtrl::PLOT_BAR_SHOW_X);

    // The rejected spectra
    std::vector<double> rejectedScanAngles;
    std::vector<double> rejectedPlumeColumns;
    SelectScanAngleAndColumnFromBasicScanEvaluationResult(
        result.initialEvaluation,
        result.debugInfo.rejectedSpectrumIndices,
        columnOffset,
        rejectedScanAngles,
        rejectedPlumeColumns);

    m_graph.SetPlotColor(RGB(128, 0, 0)); // dark red
    m_graph.BarChart(rejectedScanAngles.data(), rejectedPlumeColumns.data(), static_cast<int>(rejectedPlumeColumns.size()), Graph::CGraphCtrl::PLOT_FIXED_AXIS | Graph::CGraphCtrl::PLOT_BAR_SHOW_X);

    // draw the peak saturation
    m_graph.SetSecondRangeY(0, 100, 0, false);
    m_graph.SetSecondYUnit("Saturation");
    m_graph.SetCircleColor(RGB(255, 255, 255));
    m_graph.DrawCircles(scanAngles.data(), peakSaturation.data(), static_cast<int>(scanAngles.size()), Graph::CGraphCtrl::PLOT_SECOND_AXIS);


    // Draw the plume-centre position. TODO: Improve on this such that the user can
    // visualize which region is considered to be plume and hence which spectra are eligible for being in-plume spectrum...
    if (result.plumeInScanProperties.plumeCenter > -200) {
        m_graph.DrawLine(Graph::VERTICAL, result.plumeInScanProperties.plumeCenter, RGB(255, 255, 0), Graph::STYLE_DASHED);
        m_graph.DrawLine(Graph::VERTICAL, result.plumeInScanProperties.plumeHalfLow, RGB(128, 128, 0), Graph::STYLE_DASHED);
        m_graph.DrawLine(Graph::VERTICAL, result.plumeInScanProperties.plumeHalfHigh, RGB(128, 128, 0), Graph::STYLE_DASHED);
    }
}

void CRatioEvaluationDialog::UpdateResultList(const RatioCalculationResult& lastResult)
{
    m_resultTree.DeleteAllItems();
    if (m_controller->m_results.empty())
    {
        return;
    }

    CString str;

    str.Format("Device: %s", lastResult.deviceSerial.c_str());
    m_resultTree.InsertItem(str, TVI_ROOT);

    str.Format("Time: %04d.%02d.%02d %02d:%02d:%02d", lastResult.startTime.year, lastResult.startTime.month, lastResult.startTime.day, lastResult.startTime.hour, lastResult.startTime.minute, lastResult.startTime.second);
    m_resultTree.InsertItem(str, TVI_ROOT);

    str.Format("Plume completeness: %.1lf", lastResult.plumeInScanProperties.completeness);
    m_resultTree.InsertItem(str, TVI_ROOT);

    if (!lastResult.debugInfo.errorMessage.empty())
    {
        str.Format("%s", lastResult.debugInfo.errorMessage.c_str());
        m_resultTree.InsertItem(str, TVI_ROOT);
        return;
    }

    str.Format("Plume center: %.1lf", lastResult.plumeInScanProperties.plumeCenter);
    m_resultTree.InsertItem(str, TVI_ROOT);

    str.Format("Plume range: %.1lf to %.1lf", lastResult.plumeInScanProperties.plumeHalfLow, lastResult.plumeInScanProperties.plumeHalfHigh);
    m_resultTree.InsertItem(str, TVI_ROOT);

    str.Format("SO2/BrO Ratio: %.2G � %.2G", lastResult.ratio.ratio, lastResult.ratio.error);
    m_resultTree.InsertItem(str, TVI_ROOT);

    {
        const novac::DoasResult* so2FitResult = GetMajorWindowResult(lastResult);
        if (so2FitResult != nullptr)
        {
            HTREEITEM hTree = m_resultTree.InsertItem("SO2 Fit:", TVI_ROOT);
            str.Format("Chi�: %.2e", so2FitResult->chiSquare);
            m_resultTree.InsertItem(str, hTree);

            for (const auto& result : so2FitResult->referenceResult)
            {
                str.Format("%s %.2G � %.2G", result.name.c_str(), result.column, result.columnError);
                m_resultTree.InsertItem(str, hTree);
            }

            m_resultTree.Expand(hTree, TVE_EXPAND);
        }
    }

    {
        const novac::DoasResult* broFitResult = GetMinorWindowResult(lastResult);
        if (broFitResult != nullptr)
        {
            HTREEITEM hTree = m_resultTree.InsertItem("BrO Fit:", TVI_ROOT);
            str.Format("Chi�: %.2e", broFitResult->chiSquare);
            m_resultTree.InsertItem(str, hTree);

            for (const auto& result : broFitResult->referenceResult)
            {
                str.Format("%s %.2G � %.2G", result.name.c_str(), result.column, result.columnError);
                m_resultTree.InsertItem(str, hTree);
            }

            m_resultTree.Expand(hTree, TVE_EXPAND);
        }
    }
}

void CRatioEvaluationDialog::UpdateListOfReferences(const RatioCalculationResult& lastResult)
{
    const int currentGraph = m_resultTypeList.GetCurSel();
    if (currentGraph == 1 && !m_controller->m_results.empty()) // SO2
    {
        m_so2SpecieList.ShowWindow(SW_SHOW);
        m_referenceHeaderLabel.ShowWindow(SW_SHOW);
        m_referenceColumnLabel.ShowWindow(SW_SHOW);
        m_referenceShiftLabel.ShowWindow(SW_SHOW);
        m_referenceSqueezeLabel.ShowWindow(SW_SHOW);
        UpdateNamesOfReferencesInListBox(m_so2SpecieList, GetMajorWindowResult(lastResult));
    }
    else if (currentGraph == 2 && !m_controller->m_results.empty()) // BrO
    {
        m_so2SpecieList.ShowWindow(SW_SHOW);
        m_referenceHeaderLabel.ShowWindow(SW_SHOW);
        m_referenceColumnLabel.ShowWindow(SW_SHOW);
        m_referenceShiftLabel.ShowWindow(SW_SHOW);
        m_referenceSqueezeLabel.ShowWindow(SW_SHOW);
        UpdateNamesOfReferencesInListBox(m_so2SpecieList, GetMinorWindowResult(lastResult));
    }
    else
    {
        m_so2SpecieList.ShowWindow(SW_HIDE);
        m_referenceHeaderLabel.ShowWindow(SW_HIDE);
        m_referenceColumnLabel.ShowWindow(SW_HIDE);
        m_referenceShiftLabel.ShowWindow(SW_HIDE);
        m_referenceSqueezeLabel.ShowWindow(SW_HIDE);
    }
}

void CRatioEvaluationDialog::UpdateReferenceResultLabels(const novac::DoasResult* doasResult, int indexOfSelectedReference)
{
    if (doasResult == nullptr)
    {
        SetDlgItemText(IDC_RATIO_MAJOR_LBL_COLUMN, "");
        SetDlgItemText(IDC_RATIO_MAJOR_LBL_SHIFT, "");
        SetDlgItemText(IDC_RATIO_MAJOR_LBL_SQUEEZE, "");
        return;
    }

    double column = doasResult->referenceResult[indexOfSelectedReference].column;
    double columnError = doasResult->referenceResult[indexOfSelectedReference].columnError;
    double shift = doasResult->referenceResult[indexOfSelectedReference].shift;
    double shiftError = doasResult->referenceResult[indexOfSelectedReference].shiftError;
    double squeeze = doasResult->referenceResult[indexOfSelectedReference].squeeze;
    double squeezeError = doasResult->referenceResult[indexOfSelectedReference].squeezeError;

    CString columnStr, shiftStr, squeezeStr;
    columnStr.Format("Column %.2G � %.2G", column, columnError);
    shiftStr.Format("Shift: %.2lf � %.2lf", shift, shiftError);
    squeezeStr.Format("Squeeze: %.2lf � %.2lf", squeeze, squeezeError);

    SetDlgItemText(IDC_RATIO_MAJOR_LBL_COLUMN, columnStr);
    SetDlgItemText(IDC_RATIO_MAJOR_LBL_SHIFT, shiftStr);
    SetDlgItemText(IDC_RATIO_MAJOR_LBL_SQUEEZE, squeezeStr);
}

void CRatioEvaluationDialog::UpdateMajorFitGraph(RatioCalculationResult& lastResult)
{
    int refIndex = m_so2SpecieList.GetCurSel();
    if (refIndex < 0)
    {
        m_so2SpecieList.SetCurSel(0);
        refIndex = 0;
    }

    m_graph.SetXUnits("Pixel");
    m_graph.SetYUnits("Intensity [-]");

    if (m_controller->m_results.empty())
    {
        return;
    }

    UpdateDataInDoasFitGraph(m_graph, GetMajorWindowResult(lastResult));

    m_graph.DrawFit(refIndex);

    const novac::DoasResult* doasResult = GetMajorWindowResult(lastResult);
    UpdateReferenceResultLabels(doasResult, refIndex);
}

void CRatioEvaluationDialog::UpdateMinorFitGraph(RatioCalculationResult& lastResult)
{
    int refIndex = m_so2SpecieList.GetCurSel();
    if (refIndex < 0)
    {
        m_so2SpecieList.SetCurSel(0);
        refIndex = 0;
    }

    if (m_controller->m_results.empty())
    {
        return;
    }

    UpdateDataInDoasFitGraph(m_graph, GetMinorWindowResult(lastResult));

    m_graph.DrawFit(refIndex);

    const novac::DoasResult* doasResult = GetMinorWindowResult(lastResult);
    UpdateReferenceResultLabels(doasResult, refIndex);
}

std::string SetupFilePath()
{
    Common common;
    common.GetExePath();
    CString path;
    path.Format("%sRatioCalculationDlg.config", common.m_exePath);
    return std::string(path);
}

void CRatioEvaluationDialog::OnBnClickedRunEvaluationOnNextScan()
{
    try
    {
        if (!m_controller->HasMoreScansToEvaluate())
        {
            int answer = MessageBox("All scans evaluated. Start over from beginning?", "All scans evaluated", MB_YESNO);
            if (IDNO == answer)
            {
                return;
            }

            // Reset the list of .pak files and results.
            m_controller->SetupPakFileList(m_controller->ListPakFiles());
            m_controller->m_results.clear();
        }

        // TODO: SetupFitWindows shouldn't have to be done for each scan
        const auto windows = m_controller->SetupFitWindows();

        // TODO: Make background operation
        m_controller->EvaluateNextScan(windows);

        auto& result = m_controller->m_results.back();

        UpdateUserInterfaceWithResult(result);

        m_controller->SaveSetup(SetupFilePath());
    }
    catch (std::exception& ex)
    {
        std::stringstream message;
        message << "Ratio calculation failed. Error: " << ex.what();
        MessageBox(message.str().c_str());
    }
}

struct BackgroundEvaluationInterface
{
    bool* evaluationIsRunning = nullptr;

    HWND window = nullptr;

    RatioCalculationController* controller;
};

// Background thread for doing the evaluations
UINT EvaluateAllScans(void* pParam) {
    BackgroundEvaluationInterface* evaluationInterface = (BackgroundEvaluationInterface*)pParam;

    // Setup the evaluation
    const auto windows = evaluationInterface->controller->SetupFitWindows();

    while (evaluationInterface->controller->HasMoreScansToEvaluate() && *(evaluationInterface->evaluationIsRunning) == true)
    {
        // Evaluate one scan
        evaluationInterface->controller->EvaluateNextScan(windows);

        // Get the last result and send this to the UI
        auto newResult = &(evaluationInterface->controller->m_results.back());
        PostMessage(evaluationInterface->window, WM_PROGRESS, reinterpret_cast<WPARAM>(newResult), 0);
    }

    *(evaluationInterface->evaluationIsRunning) = false;

    // Tell the user interface that we are done processing
    PostMessage(evaluationInterface->window, WM_DONE, 0, 0);

    // Clean up..
    evaluationInterface->evaluationIsRunning = nullptr;
    evaluationInterface->controller = nullptr;
    evaluationInterface->window = nullptr;
    delete evaluationInterface;

    return 0;
}

void CRatioEvaluationDialog::OnBnClickedRunEvaluationOnAllScans()
{
    try
    {
        if (!m_controller->HasMoreScansToEvaluate())
        {
            int answer = MessageBox("All scans evaluated. Start over from beginning?", "All scans evaluated", MB_YESNO);
            if (IDNO == answer)
            {
                return;
            }

            // Reset the list of .pak files and results.
            m_controller->SetupPakFileList(m_controller->ListPakFiles());
            m_controller->m_results.clear();
        }

        m_controller->SaveSetup(SetupFilePath());

        // Disable the UI buttons
        GetDlgItem(IDC_RATIO_RUN_EVALUATION_NEXT_SCAN)->EnableWindow(FALSE);
        GetDlgItem(IDC_RATIO_RUN_EVALUATION)->EnableWindow(FALSE);
        GetDlgItem(IDC_RATIO_SAVE_RESULTS)->EnableWindow(FALSE);

        // Start the background thread which does the processing
        this->m_backgroundProcessingIsRunning = true;

        BackgroundEvaluationInterface* processingSync = new BackgroundEvaluationInterface();
        processingSync->window = this->m_hWnd;
        processingSync->controller = this->m_controller;
        processingSync->evaluationIsRunning = &(this->m_backgroundProcessingIsRunning);
        AfxBeginThread(EvaluateAllScans, processingSync, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
    }
    catch (std::exception& ex)
    {
        std::stringstream message;
        message << "Ratio calculation failed. Error: " << ex.what();
        MessageBox(message.str().c_str());

        // Re-enable the UI buttons
        GetDlgItem(IDC_RATIO_RUN_EVALUATION_NEXT_SCAN)->EnableWindow(TRUE);
        GetDlgItem(IDC_RATIO_RUN_EVALUATION)->EnableWindow(TRUE);
        GetDlgItem(IDC_RATIO_SAVE_RESULTS)->EnableWindow(TRUE);
    }
}

void CRatioEvaluationDialog::OnChangeSelectedDoasSpecie()
{
    if (m_controller->m_results.empty())
    {
        return;
    }

    auto lastResult = m_controller->m_results.back();

    UpdateGraph(lastResult);
    UpdateListOfReferences(lastResult);
}

void CRatioEvaluationDialog::OnBnClickedSaveResults()
{
    CString destinationFileName = "";
    if (Common::BrowseForFile_SaveAs("CSV Data Files\0*.csv\0", destinationFileName))
    {
        std::string dstFileName = novac::EnsureFilenameHasSuffix(std::string(destinationFileName), "csv");
        const std::string columnSeparatorChar = ";"; // TODO: Let the user choose this
        m_controller->SaveResultsToCsvFile(dstFileName, columnSeparatorChar);
    }
}
