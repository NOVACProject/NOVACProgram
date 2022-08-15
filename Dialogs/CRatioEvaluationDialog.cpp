// CRatioEvaluationDialog.cpp : implementation file
//

#include "StdAfx.h"
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

    ON_LBN_SELCHANGE(IDC_MAJOR_SPECIE_LIST, OnChangeSelectedSpecie)
    ON_LBN_SELCHANGE(IDC_RATIO_SHOW_SELECTION_LIST, &CRatioEvaluationDialog::OnChangeSelectedDoasSpecie)
END_MESSAGE_MAP()

// CRatioEvaluationDialog message handlers

void CRatioEvaluationDialog::OnChangeSelectedSpecie()
{
    const int currentGraph = m_resultTypeList.GetCurSel();
    if (currentGraph == 1) // SO2
    {
        UpdateMajorFitGraph();
    }
    else if (currentGraph == 2) // BrO
    {
        UpdateMinorFitGraph();
    }
}

void CRatioEvaluationDialog::UpdateUserInterfaceWithResult()
{
    UpdateGraph();
    UpdateResultList();
    UpdateListOfReferences();
    UpdateResultList();

    {
        CString progressStr;
        progressStr.Format("Evaluated scan %d out of %d", m_controller->CurrentPakFileIndex(), m_controller->NumberOfPakFilesInSetup());
        SetDlgItemText(IDC_RATIO_PROGRESS_LABEL, progressStr);
    }
}

const novac::DoasResult& CRatioEvaluationDialog::GetMajorWindowResult()
{
    return m_controller->m_results.back().debugInfo.doasResults[0]; // First result is for SO2
}

const novac::DoasResult& CRatioEvaluationDialog::GetMinorWindowResult()
{
    return m_controller->m_results.back().debugInfo.doasResults[1]; // Second result is for BrO
}

void CRatioEvaluationDialog::UpdateGraph()
{
    const int currentGraph = m_resultTypeList.GetCurSel();
    if (currentGraph == 0)
    {
        UpdateScanGraph();
    }
    else if (currentGraph == 1)
    {
        UpdateMajorFitGraph();
    }
    else if (currentGraph == 2)
    {
        UpdateMinorFitGraph();
    }
}

void CRatioEvaluationDialog::UpdateScanGraph()
{
    // Draw the evaluated columns vs scan-angle, highlighting the selected in-plume and out-of-plume spectra.
    m_graph.CleanPlot();
    m_graph.SetXUnits("Angle [deg]");
    m_graph.SetYUnits("Column [molec/cm2]");

    const double columnOffset = m_controller->m_results.back().plumeInScanProperties.offset;

    std::vector<double> scanAngles;
    std::vector<double> columns;
    std::vector<double> columnErrors;
    for (int ii = 0; ii < static_cast<int>(m_controller->m_results.back().initialEvaluation.m_spec.size()); ++ii)
    {
        const novac::CEvaluationResult& result = m_controller->m_results.back().initialEvaluation.m_spec[ii];
        const novac::CSpectrumInfo& spectrumInfo = m_controller->m_results.back().initialEvaluation.m_specInfo[ii];

        scanAngles.push_back(spectrumInfo.m_scanAngle);
        columns.push_back(result.m_referenceResult[0].m_column - columnOffset);
        columnErrors.push_back(result.m_referenceResult[0].m_columnError);
    }

    // All the column-values
    m_graph.SetRange(
        Min(scanAngles),
        Max(scanAngles),
        1,
        Min(columns),
        Max(columns),
        0);
    m_graph.SetPlotColor(RGB(128, 0, 0));
    m_graph.BarChart(scanAngles.data(), columns.data(), columnErrors.data(), static_cast<int>(scanAngles.size()), Graph::CGraphCtrl::PLOT_FIXED_AXIS | Graph::CGraphCtrl::PLOT_BAR_SHOW_X);

    // The selected in-plume column-values
    std::vector<double> inPlumeScanAngles;
    std::vector<double> inPlumeColumns;
    SelectScanAngleAndColumnFromBasicScanEvaluationResult(
        m_controller->m_results.back().initialEvaluation,
        m_controller->m_results.back().debugInfo.plumeSpectra,
        columnOffset,
        inPlumeScanAngles,
        inPlumeColumns);

    m_graph.SetPlotColor(RGB(0, 255, 0));
    m_graph.BarChart(inPlumeScanAngles.data(), inPlumeColumns.data(), static_cast<int>(inPlumeColumns.size()), Graph::CGraphCtrl::PLOT_FIXED_AXIS | Graph::CGraphCtrl::PLOT_BAR_SHOW_X);

    // The selected out-of-plume column-values
    std::vector<double> outOfPlumeScanAngles;
    std::vector<double> outOfPlumeColumns;
    SelectScanAngleAndColumnFromBasicScanEvaluationResult(
        m_controller->m_results.back().initialEvaluation,
        m_controller->m_results.back().debugInfo.outOfPlumeSpectra,
        columnOffset,
        outOfPlumeScanAngles,
        outOfPlumeColumns);

    m_graph.SetPlotColor(RGB(255, 0, 0));
    m_graph.BarChart(outOfPlumeScanAngles.data(), outOfPlumeColumns.data(), static_cast<int>(outOfPlumeScanAngles.size()), Graph::CGraphCtrl::PLOT_FIXED_AXIS | Graph::CGraphCtrl::PLOT_BAR_SHOW_X);
}

void CRatioEvaluationDialog::UpdateResultList()
{
    m_resultTree.DeleteAllItems();

    CString str;

    const auto& lastResult = m_controller->m_results.back();

    str.Format("Device: %s", lastResult.deviceSerial.c_str());
    m_resultTree.InsertItem(str, TVI_ROOT);

    str.Format("Time: %04d.%02d.%02d %02d:%02d:%02d", lastResult.startTime.year, lastResult.startTime.month, lastResult.startTime.day, lastResult.startTime.hour, lastResult.startTime.minute, lastResult.startTime.second);
    m_resultTree.InsertItem(str, TVI_ROOT);

    str.Format("SO2/BrO Ratio: %.2G ± %.2G", lastResult.ratio.ratio, lastResult.ratio.error);
    m_resultTree.InsertItem(str, TVI_ROOT);

    {
        const novac::DoasResult& so2FitResult = GetMajorWindowResult();
        HTREEITEM hTree = m_resultTree.InsertItem("SO2 Fit:", TVI_ROOT);
        str.Format("Chi²: %.2e", so2FitResult.chiSquare);
        m_resultTree.InsertItem(str, hTree);

        for (const auto& result : so2FitResult.referenceResult)
        {
            str.Format("%s %.2G ± %.2G", result.name.c_str(), result.column, result.columnError);
            m_resultTree.InsertItem(str, hTree);
        }

        m_resultTree.Expand(hTree, TVE_EXPAND);
    }

    {
        const novac::DoasResult& broFitResult = GetMinorWindowResult();
        HTREEITEM hTree = m_resultTree.InsertItem("BrO Fit:", TVI_ROOT);
        str.Format("Chi²: %.2e", broFitResult.chiSquare);
        m_resultTree.InsertItem(str, hTree);

        for (const auto& result : broFitResult.referenceResult)
        {
            str.Format("%s %.2G ± %.2G", result.name.c_str(), result.column, result.columnError);
            m_resultTree.InsertItem(str, hTree);
        }

        m_resultTree.Expand(hTree, TVE_EXPAND);
    }

    m_resultTree.ModifyStyle(0, TVS_LINESATROOT);
}

void CRatioEvaluationDialog::UpdateListOfReferences()
{
    const int currentGraph = m_resultTypeList.GetCurSel();
    if (currentGraph == 1) // SO2
    {
        m_so2SpecieList.ShowWindow(SW_SHOW);
        m_referenceHeaderLabel.ShowWindow(SW_SHOW);
        m_referenceColumnLabel.ShowWindow(SW_SHOW);
        m_referenceShiftLabel.ShowWindow(SW_SHOW);
        m_referenceSqueezeLabel.ShowWindow(SW_SHOW);
        UpdateNamesOfReferencesInListBox(m_so2SpecieList, GetMajorWindowResult());
    }
    else if (currentGraph == 2) // BrO
    {
        m_so2SpecieList.ShowWindow(SW_SHOW);
        m_referenceHeaderLabel.ShowWindow(SW_SHOW);
        m_referenceColumnLabel.ShowWindow(SW_SHOW);
        m_referenceShiftLabel.ShowWindow(SW_SHOW);
        m_referenceSqueezeLabel.ShowWindow(SW_SHOW);
        UpdateNamesOfReferencesInListBox(m_so2SpecieList, GetMinorWindowResult());
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

void CRatioEvaluationDialog::UpdateReferenceResultLabels(const novac::DoasResult& doasResult, int indexOfSelectedReference)
{
    double column = doasResult.referenceResult[indexOfSelectedReference].column;
    double columnError = doasResult.referenceResult[indexOfSelectedReference].columnError;
    double shift = doasResult.referenceResult[indexOfSelectedReference].shift;
    double shiftError = doasResult.referenceResult[indexOfSelectedReference].shiftError;
    double squeeze = doasResult.referenceResult[indexOfSelectedReference].squeeze;
    double squeezeError = doasResult.referenceResult[indexOfSelectedReference].squeezeError;

    CString columnStr, shiftStr, squeezeStr;
    columnStr.Format("Column %.2G ± %.2G", column, columnError);
    shiftStr.Format("Shift: %.2lf ± %.2lf", shift, shiftError);
    squeezeStr.Format("Squeeze: %.2lf ± %.2lf", squeeze, squeezeError);

    SetDlgItemText(IDC_RATIO_MAJOR_LBL_COLUMN, columnStr);
    SetDlgItemText(IDC_RATIO_MAJOR_LBL_SHIFT, shiftStr);
    SetDlgItemText(IDC_RATIO_MAJOR_LBL_SQUEEZE, squeezeStr);
}

void CRatioEvaluationDialog::UpdateMajorFitGraph()
{
    int refIndex = m_so2SpecieList.GetCurSel();
    if (refIndex < 0)
    {
        m_so2SpecieList.SetCurSel(0);
        refIndex = 0;
    }

    m_graph.SetXUnits("Pixel");
    m_graph.SetYUnits("Intensity [-]");

    UpdateDataInDoasFitGraph(m_graph, GetMajorWindowResult());

    m_graph.DrawFit(refIndex);

    const novac::DoasResult& doasResult = GetMajorWindowResult();
    UpdateReferenceResultLabels(doasResult, refIndex);
}

void CRatioEvaluationDialog::UpdateMinorFitGraph()
{
    int refIndex = m_so2SpecieList.GetCurSel();
    if (refIndex < 0)
    {
        m_so2SpecieList.SetCurSel(0);
        refIndex = 0;
    }

    UpdateDataInDoasFitGraph(m_graph, GetMinorWindowResult());

    m_graph.DrawFit(refIndex);

    const novac::DoasResult& doasResult = GetMinorWindowResult();
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

        UpdateUserInterfaceWithResult();

        m_controller->SaveSetup(SetupFilePath());
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
    try
    {
        const auto windows = m_controller->SetupFitWindows();

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

        while (m_controller->HasMoreScansToEvaluate())
        {
            // TODO: Make background operation
            m_controller->EvaluateNextScan(windows);

            UpdateUserInterfaceWithResult();
        }

        m_controller->SaveSetup(SetupFilePath());
    }
    catch (std::exception& ex)
    {
        std::stringstream message;
        message << "Ratio calculation failed. Error: " << ex.what();
        MessageBox(message.str().c_str());
    }
}

void CRatioEvaluationDialog::OnChangeSelectedDoasSpecie()
{
    UpdateGraph();
    UpdateListOfReferences();
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
