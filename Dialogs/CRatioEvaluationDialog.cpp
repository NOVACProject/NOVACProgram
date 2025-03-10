﻿#include "stdafx.h"
#include "../resource.h"
#include "CRatioEvaluationDialog.h"

#include "../Common/Common.h"

#include <SpectralEvaluation/DialogControllers/RatioCalculationController.h>
#include <SpectralEvaluation/Spectra/Spectrum.h>
#include <SpectralEvaluation/File/File.h>
#include <SpectralEvaluation/VectorUtils.h>

#include <sstream>

#undef min
#undef max

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

std::string FormatUnit(novac::CrossSectionUnit unit)
{
    if (unit == novac::CrossSectionUnit::ppmm)
    {
        return "ppmm";
    }
    return "molec/cm²";
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

void UpdateNamesOfReferencesInListBox(CNonFlickeringListBoxControl& destination, const novac::DoasResult* doasResult)
{
    destination.DisableRedraw();

    // Make sure to save the name of the last selected refences
    CString previouslySelectedReferenceName;
    if (destination.GetCurSel() >= 0)
    {
        destination.GetText(destination.GetCurSel(), previouslySelectedReferenceName);
    }
    int indexToSelectAfterUpdate = -1;

    // Repopulate the list box
    destination.ResetContent();
    if (doasResult != nullptr)
    {
        const int numberOfReferences = static_cast<int>(doasResult->referenceResult.size());
        for (int k = 0; k < numberOfReferences; ++k)
        {
            CString name = (LPCSTR)doasResult->referenceResult[k].name.c_str();
            destination.AddString(name);

            if (name == previouslySelectedReferenceName)
            {
                indexToSelectAfterUpdate = k;
            }
        }
    }

    // Re-select the previously selected specie, if it still remains in the list
    if (indexToSelectAfterUpdate >= 0)
    {
        destination.SetCurSel(indexToSelectAfterUpdate);
    }

    destination.EnableRedraw();
}

bool ResultShouldBeShown(const RatioCalculationResult* result, int resultFilterOption)
{
    if (resultFilterOption == 1)
    {
        // resultFilterOption == 1 means only showing results where we got a ratio.
        return result->RatioCalculationSuccessful();
    }
    else if (resultFilterOption == 2)
    {
        // resultFilterOption == 2 means only showing results where there is a significant BrO detection.
        return result->SignificantMinorSpecieDetection();
    }

    // resultFilterOption == 0 means show everything.
    return true;
}

#pragma endregion

// CRatioEvaluationDialog dialog

IMPLEMENT_DYNAMIC(CRatioEvaluationDialog, CPropertyPage)

CRatioEvaluationDialog::CRatioEvaluationDialog(RatioCalculationController* controller, CWnd* pParent /*=nullptr*/)
    : CPropertyPage(IDD_RATIO_EVALUATE_DIALOG), m_controller(controller)
{}

CRatioEvaluationDialog::~CRatioEvaluationDialog()
{
    m_controller = nullptr;
}

BOOL CRatioEvaluationDialog::OnInitDialog()
{
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

    m_resultFilterSelector.SetCurSel(0);

    return TRUE;  // return TRUE unless you set the focus to a control
}

BOOL CRatioEvaluationDialog::OnSetActive()
{
    UpdateListOfResults();

    if (m_controller->m_results.size() == 0)
    {
        UpdateUserInterfaceWithResult(nullptr);
    }

    return CPropertyPage::OnSetActive();
}

void CRatioEvaluationDialog::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_FIT_FRAME_SCAN, m_scanFrame);
    DDX_Control(pDX, IDC_MAJOR_SPECIE_LIST, m_doasFitReferencesList);
    DDX_Control(pDX, IDC_RATIO_SHOW_SELECTION_LIST, m_resultTypeList);
    DDX_Control(pDX, IDC_RATIO_RESULT_TREE, m_resultTree);
    DDX_Control(pDX, IDC_RATIO_MAJOR_LBL_COLUMN, m_referenceColumnLabel);
    DDX_Control(pDX, IDC_RATIO_MAJOR_LBL_SHIFT, m_referenceShiftLabel);
    DDX_Control(pDX, IDC_RATIO_MAJOR_LBL_SQUEEZE, m_referenceSqueezeLabel);
    DDX_Control(pDX, IDC_RATIO_EVALUATED_SCANS_LIST, m_resultsList);
    DDX_Control(pDX, IDC_RATIO_EVALUATED_SCANS_SELECTOR_COMBO, m_resultFilterSelector);
}

BEGIN_MESSAGE_MAP(CRatioEvaluationDialog, CPropertyPage)
    ON_BN_CLICKED(IDC_RATIO_RUN_EVALUATION_NEXT_SCAN, &CRatioEvaluationDialog::OnBnClickedRunEvaluationOnNextScan)
    ON_BN_CLICKED(IDC_RATIO_RUN_EVALUATION, &CRatioEvaluationDialog::OnBnClickedRunEvaluationOnAllScans)
    ON_BN_CLICKED(IDC_RATIO_SAVE_RESULTS, &CRatioEvaluationDialog::OnBnClickedSaveResults)

    ON_MESSAGE(WM_DONE, OnBackgroundProcessingDone)
    ON_MESSAGE(WM_PROGRESS, OnOneRatioEvaluationDone)

    ON_COMMAND(ID_SAVE_SAVERESULTCSV, SaveSelectedSpectrumResultToCsv)
    ON_COMMAND(ID_SAVE_SAVESPECTRA_PAK, SaveSelectedSpectraToPak)
    ON_COMMAND(ID_SAVE_SAVESPECTRA_STD, SaveSelectedSpectraToStd)

    ON_WM_CONTEXTMENU()

    ON_LBN_SELCHANGE(IDC_MAJOR_SPECIE_LIST, OnChangeSelectedSpecie)
    ON_LBN_SELCHANGE(IDC_RATIO_SHOW_SELECTION_LIST, &CRatioEvaluationDialog::OnChangeSelectedDoasSpecie)
    ON_LBN_SELCHANGE(IDC_RATIO_EVALUATED_SCANS_LIST, &CRatioEvaluationDialog::OnChangeEvaluatedScan)
    ON_BN_CLICKED(IDC_RATIO_CLEAR_RESULTS, &CRatioEvaluationDialog::OnBnClickedClearResults)
    ON_CBN_SELCHANGE(IDC_RATIO_EVALUATED_SCANS_SELECTOR_COMBO, &CRatioEvaluationDialog::OnSelchangeEvaluatedScansSelectorCombo)
END_MESSAGE_MAP()

// CRatioEvaluationDialog message handlers

RatioCalculationResult GetResultToDisplay(const RatioCalculationController* controller, CListBox& resultSelector, CComboBox& resultFilterSelector)
{
    const int index = resultSelector.GetCurSel();

    if (static_cast<size_t>(index) >= controller->m_results.size())
    {
        // invalid index, take the last result
        return controller->m_results.back();
    }

    if (resultFilterSelector.GetCurSel() == 1)
    {
        // Since we now may be filtering the list of results, we can't just use an index and must revert to looping throught he results
        int successfulResultIndex = 0;
        for (int ii = 0; ii < static_cast<int>(controller->m_results.size()); ++ii)
        {
            if (controller->m_results[ii].RatioCalculationSuccessful())
            {
                if (successfulResultIndex == index)
                {
                    return controller->m_results[ii];
                }
                ++successfulResultIndex;
            }
        }
    }
    else if (resultFilterSelector.GetCurSel() == 2)
    {
        // Since we now may be filtering the list of results, we can't just use an index and must revert to looping throught he results
        int successfulResultIndex = 0;
        for (int ii = 0; ii < static_cast<int>(controller->m_results.size()); ++ii)
        {
            if (controller->m_results[ii].SignificantMinorSpecieDetection())
            {
                if (successfulResultIndex == index)
                {
                    return controller->m_results[ii];
                }
                ++successfulResultIndex;
            }
        }
    }

    // Default option, show all the results. This means we can just just indexing.
    return controller->m_results[index];
}

void CRatioEvaluationDialog::OnChangeSelectedSpecie()
{
    const int currentGraph = m_resultTypeList.GetCurSel();

    if (m_controller->m_results.empty())
    {
        return;
    }

    RatioCalculationResult lastResult = GetResultToDisplay(m_controller, m_resultsList, m_resultFilterSelector);

    if (currentGraph == 1) // SO2
    {
        UpdateMajorFitGraph(&lastResult);
    }
    else if (currentGraph == 2) // BrO
    {
        UpdateMinorFitGraph(&lastResult);
    }
}

void CRatioEvaluationDialog::OnChangeEvaluatedScan()
{
    if (m_backgroundProcessingIsRunning)
    {
        return;
    }

    RatioCalculationResult lastResult = GetResultToDisplay(m_controller, m_resultsList, m_resultFilterSelector);

    UpdateUserInterfaceWithResult(&lastResult);
}


void CRatioEvaluationDialog::OnChangeSelectedDoasSpecie()
{
    if (m_controller->m_results.empty())
    {
        return;
    }

    RatioCalculationResult lastResult = GetResultToDisplay(m_controller, m_resultsList, m_resultFilterSelector);

    UpdateListOfReferences(&lastResult);
    UpdateGraph(&lastResult);
}

void CRatioEvaluationDialog::UpdateStateWhileBackgroundProcessingIsRunning()
{
    const BOOL enableUserInteraction = (m_backgroundProcessingIsRunning) ? FALSE : TRUE;

    GetDlgItem(IDC_RATIO_RUN_EVALUATION_NEXT_SCAN)->EnableWindow(enableUserInteraction);
    GetDlgItem(IDC_RATIO_RUN_EVALUATION)->EnableWindow(enableUserInteraction);
    GetDlgItem(IDC_RATIO_SAVE_RESULTS)->EnableWindow(enableUserInteraction);
    GetDlgItem(IDC_RATIO_EVALUATED_SCANS_LIST)->EnableWindow(enableUserInteraction);
    GetDlgItem(IDC_RATIO_CLEAR_RESULTS)->EnableWindow(enableUserInteraction);
    GetDlgItem(IDC_RATIO_EVALUATED_SCANS_SELECTOR_COMBO)->EnableWindow(enableUserInteraction);
}

LRESULT CRatioEvaluationDialog::OnBackgroundProcessingDone(WPARAM wParam, LPARAM /*lParam*/)
{
    m_backgroundProcessingThread = nullptr;

    // Re-enable the UI buttons
    UpdateStateWhileBackgroundProcessingIsRunning();

    // If there was an error message, then display it to the user
    if (wParam != 0)
    {
        std::string* errorMessage = reinterpret_cast<std::string*>(wParam);

        MessageBox(errorMessage->c_str());

        delete errorMessage;
    }

    return 0;
}

LRESULT CRatioEvaluationDialog::OnOneRatioEvaluationDone(WPARAM wParam, LPARAM /*lParam*/)
{
    RatioCalculationResult* newResult = reinterpret_cast<RatioCalculationResult*>(wParam);
    if (newResult == nullptr)
    {
        return 0;
    }

    AddToListOfResults(newResult);

    UpdateUserInterfaceWithResult(newResult);

    delete newResult;

    return 0;
}

void CRatioEvaluationDialog::UpdateUserInterfaceWithResult(RatioCalculationResult* result)
{
    if (result != nullptr && !result->debugInfo.errorMessage.empty())
    {
        // If there was an error in the evaluation, then don't attemp to show all the references.
        m_resultTypeList.ResetContent();
        m_resultTypeList.AddString("Scan");
        m_resultTypeList.SetCurSel(0);

        GetDlgItem(IDC_RATIO_ERROR_MESSAGE_LABEL)->ShowWindow(SW_SHOW);
        SetDlgItemText(IDC_RATIO_ERROR_MESSAGE_LABEL, result->debugInfo.errorMessage.c_str());
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
    UpdateResultTree(result);
    UpdateListOfReferences(result);

    {
        CString progressStr;
        if (m_controller->CurrentPakFileIndex() == m_controller->NumberOfPakFilesInSetup())
        {
            progressStr.Format("All %d scans evaluated", m_controller->NumberOfPakFilesInSetup());
        }
        else
        {
            progressStr.Format("Evaluated scan %d out of %d", m_controller->CurrentPakFileIndex(), m_controller->NumberOfPakFilesInSetup());
        }
        SetDlgItemText(IDC_RATIO_PROGRESS_LABEL, progressStr);
    }
}

const novac::DoasResult* CRatioEvaluationDialog::GetMajorWindowResult(const RatioCalculationResult* result)
{
    if (result->debugInfo.doasResults.empty())
    {
        return nullptr;
    }
    return &result->debugInfo.doasResults[0]; // First result is for SO2
}

const novac::DoasResult* CRatioEvaluationDialog::GetMinorWindowResult(const RatioCalculationResult* result)
{
    if (result->debugInfo.doasResults.size() < 2)
    {
        return nullptr;
    }
    return &result->debugInfo.doasResults[1]; // Second result is for BrO
}

void CRatioEvaluationDialog::UpdateGraph(RatioCalculationResult* result)
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

void CRatioEvaluationDialog::UpdateScanGraph(RatioCalculationResult* result)
{
    // Draw the evaluated columns vs scan-angle, highlighting the selected in-plume and out-of-plume spectra.
    m_graph.CleanPlot();
    m_graph.SetXUnits("Angle [deg]");
    {
        CString columnStr;
        columnStr.Format("Column [%s]", FormatUnit(m_controller->m_crossSectionUnit).c_str());
        m_graph.SetYUnits(columnStr);
    }

    if (result == nullptr)
    {
        return;
    }

    const double columnOffset = result->plumeInScanProperties.offset.ValueOrDefault(0.0);

    std::vector<double> scanAngles;
    std::vector<double> columns;
    std::vector<double> columnErrors;
    std::vector<double> peakSaturation;
    int numberOfGoodColumnValue = 0;
    double minimumGoodColumnValue = std::numeric_limits<double>::max();
    double maximumGoodColumnValue = std::numeric_limits<double>::lowest();
    double minimumOfAllColumnValues = std::numeric_limits<double>::max();
    double maximumOfAllColumnValues = std::numeric_limits<double>::lowest();
    for (int ii = 0; ii < static_cast<int>(result->initialEvaluation.m_spec.size()); ++ii)
    {
        const novac::CEvaluationResult& initialEvaluation = result->initialEvaluation.m_spec[ii];
        const novac::CSpectrumInfo& spectrumInfo = result->initialEvaluation.m_specInfo[ii];

        const double columnValue = initialEvaluation.m_referenceResult[0].m_column - columnOffset;
        if (!initialEvaluation.IsBad())
        {
            minimumGoodColumnValue = std::min(minimumGoodColumnValue, columnValue);
            maximumGoodColumnValue = std::max(maximumGoodColumnValue, columnValue);
            ++numberOfGoodColumnValue;
        }
        minimumOfAllColumnValues = std::min(minimumOfAllColumnValues, columnValue);
        maximumOfAllColumnValues = std::max(maximumOfAllColumnValues, columnValue);

        scanAngles.push_back(spectrumInfo.m_scanAngle);
        columns.push_back(columnValue);
        columnErrors.push_back(initialEvaluation.m_referenceResult[0].m_columnError);

        const double saturation = std::max(0.0, std::min(100.0, 100.0 * spectrumInfo.m_peakIntensity / (double)(spectrumInfo.m_numSpec * result->debugInfo.spectrometerFullDynamicRange)));
        peakSaturation.push_back(saturation);
    }

    // Set up the plot
    const double plotMinValue = (numberOfGoodColumnValue > 0) ? minimumGoodColumnValue : minimumOfAllColumnValues;
    const double plotMaxValue = (numberOfGoodColumnValue > 0) ? maximumGoodColumnValue : maximumOfAllColumnValues;
    m_graph.SetRange(
        -90.0,
        90.0,
        1,
        plotMinValue,
        plotMaxValue,
        0);
    m_graph.SetPlotColor(RGB(255, 0, 0)); // red
    m_graph.BarChart(scanAngles.data(), columns.data(), columnErrors.data(), static_cast<int>(scanAngles.size()), Graph::CGraphCtrl::PLOT_FIXED_AXIS | Graph::CGraphCtrl::PLOT_BAR_SHOW_X);

    // The selected in-plume column-values
    std::vector<double> inPlumeScanAngles;
    std::vector<double> inPlumeColumns;
    SelectScanAngleAndColumnFromBasicScanEvaluationResult(
        result->initialEvaluation,
        result->debugInfo.plumeSpectrumIndices,
        columnOffset,
        inPlumeScanAngles,
        inPlumeColumns);

    m_graph.SetPlotColor(RGB(0, 255, 0)); // green
    m_graph.BarChart(inPlumeScanAngles.data(), inPlumeColumns.data(), static_cast<int>(inPlumeColumns.size()), Graph::CGraphCtrl::PLOT_FIXED_AXIS | Graph::CGraphCtrl::PLOT_BAR_SHOW_X);

    // The selected out-of-plume column-values
    std::vector<double> outOfPlumeScanAngles;
    std::vector<double> outOfPlumeColumns;
    SelectScanAngleAndColumnFromBasicScanEvaluationResult(
        result->initialEvaluation,
        result->debugInfo.outOfPlumeSpectrumIndices,
        columnOffset,
        outOfPlumeScanAngles,
        outOfPlumeColumns);

    m_graph.SetPlotColor(RGB(0, 128, 0)); // dark greens
    m_graph.BarChart(outOfPlumeScanAngles.data(), outOfPlumeColumns.data(), static_cast<int>(outOfPlumeScanAngles.size()), Graph::CGraphCtrl::PLOT_FIXED_AXIS | Graph::CGraphCtrl::PLOT_BAR_SHOW_X);

    // The rejected spectra
    std::vector<double> rejectedScanAngles;
    std::vector<double> rejectedPlumeColumns;
    SelectScanAngleAndColumnFromBasicScanEvaluationResult(
        result->initialEvaluation,
        result->debugInfo.rejectedSpectrumIndices,
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
    if (result->plumeInScanProperties.plumeCenter.HasValue())
    {
        m_graph.DrawLine(Graph::VERTICAL, result->plumeInScanProperties.plumeCenter.Value(), RGB(255, 255, 0), Graph::STYLE_DASHED);
        m_graph.DrawLine(Graph::VERTICAL, result->plumeInScanProperties.plumeHalfLow.ValueOrDefault(-90.0), RGB(128, 128, 0), Graph::STYLE_DASHED);
        m_graph.DrawLine(Graph::VERTICAL, result->plumeInScanProperties.plumeHalfHigh.ValueOrDefault(+90.0), RGB(128, 128, 0), Graph::STYLE_DASHED);
    }
}

// @return RED if the error is larger than the absolute of the value, otherwise BLACK
COLORREF GetColorForValue(double value, double error)
{
    if (std::abs(error) > std::abs(value))
    {
        return RGB(255, 0, 0);
    }
    return RGB(0, 0, 0);
}

void CRatioEvaluationDialog::UpdateResultTree(const RatioCalculationResult* result)
{
    m_resultTree.DisableRedraw();

    m_resultTree.DeleteAllItems();
    if (result == nullptr)
    {
        m_resultTree.EnableRedraw();
        return;
    }

    const std::string unit = FormatUnit(m_controller->m_crossSectionUnit);

    CString str;

    str.Format("Device: %s", result->deviceSerial.c_str());
    auto firstItem = m_resultTree.InsertItem(str, TVI_ROOT);

    str.Format("Time: %04d.%02d.%02d %02d:%02d:%02d UTC", result->startTime.year, result->startTime.month, result->startTime.day, result->startTime.hour, result->startTime.minute, result->startTime.second);
    m_resultTree.InsertItem(str, TVI_ROOT);

    if (!result->plumeInScanProperties.completeness.HasValue())
    {
        str.Format("Plume completeness: N/A");
        m_resultTree.InsertItem(str, TVI_ROOT);
    }
    else
    {
        str.Format("Plume completeness: %.1lf", result->plumeInScanProperties.completeness);
        m_resultTree.InsertItem(str, TVI_ROOT);

        str.Format("Plume center: %.1lf [deg]", result->plumeInScanProperties.plumeCenter);
        m_resultTree.InsertItem(str, TVI_ROOT);

        // Format the plume ranges. Undetermined values are formatted at either -90 or +90 depending on the edge.
        const double plumeHalfLow = result->plumeInScanProperties.plumeHalfLow.ValueOrDefault(-90.0);
        const double plumeHalfHigh = result->plumeInScanProperties.plumeHalfHigh.ValueOrDefault(+90.0);
        str.Format("Plume range: %.1lf to %.1lf [deg]", plumeHalfLow, plumeHalfHigh);
        m_resultTree.InsertItem(str, TVI_ROOT);
    }

    if (!result->RatioCalculationSuccessful())
    {
        str.Format("%s", result->debugInfo.errorMessage.c_str());
        m_resultTree.InsertItem(str, TVI_ROOT);

        m_resultTree.SelectItem(firstItem);

        m_resultTree.EnableRedraw();
        return;
    }

    str.Format("BrO/SO2 Ratio: %.2E ± %.2E", result->ratio.ratio, result->ratio.error);
    auto item = m_resultTree.InsertItem(str, TVI_ROOT);
    m_resultTree.SetItemColor(item, GetColorForValue(result->ratio.ratio, result->ratio.error));

    {
        const novac::DoasResult* so2FitResult = GetMajorWindowResult(result);
        if (so2FitResult != nullptr)
        {
            HTREEITEM hTree = m_resultTree.InsertItem("SO2 Fit:", TVI_ROOT);
            str.Format("Chi²: %.2E", so2FitResult->chiSquare);
            m_resultTree.InsertItem(str, hTree);

            for (const auto& result : so2FitResult->referenceResult)
            {
                str.Format("%s %.2E ± %.2E", result.name.c_str(), result.column, result.columnError);
                item = m_resultTree.InsertItem(str, hTree);
                m_resultTree.SetItemColor(item, GetColorForValue(result.column, result.columnError));
            }

            m_resultTree.Expand(hTree, TVE_EXPAND);
        }
    }

    {
        const novac::DoasResult* broFitResult = GetMinorWindowResult(result);
        if (broFitResult != nullptr)
        {
            HTREEITEM hTree = m_resultTree.InsertItem("BrO Fit:", TVI_ROOT);
            str.Format("Chi²: %.2E", broFitResult->chiSquare);
            m_resultTree.InsertItem(str, hTree);

            for (const auto& result : broFitResult->referenceResult)
            {
                str.Format("%s %.2E ± %.2E", result.name.c_str(), result.column, result.columnError);
                m_resultTree.InsertItem(str, hTree);
            }

            m_resultTree.Expand(hTree, TVE_EXPAND);
        }
    }

    m_resultTree.SelectItem(firstItem);

    m_resultTree.EnableRedraw();
}

void CRatioEvaluationDialog::UpdateListOfReferences(const RatioCalculationResult* result)
{
    const int currentGraph = m_resultTypeList.GetCurSel();
    if (currentGraph == 1 && result != nullptr) // SO2
    {
        m_doasFitReferencesList.ShowWindow(SW_SHOW);
        m_referenceColumnLabel.ShowWindow(SW_SHOW);
        m_referenceShiftLabel.ShowWindow(SW_SHOW);
        m_referenceSqueezeLabel.ShowWindow(SW_SHOW);
        UpdateNamesOfReferencesInListBox(m_doasFitReferencesList, GetMajorWindowResult(result));
    }
    else if (currentGraph == 2 && result != nullptr) // BrO
    {
        m_doasFitReferencesList.ShowWindow(SW_SHOW);
        m_referenceColumnLabel.ShowWindow(SW_SHOW);
        m_referenceShiftLabel.ShowWindow(SW_SHOW);
        m_referenceSqueezeLabel.ShowWindow(SW_SHOW);
        UpdateNamesOfReferencesInListBox(m_doasFitReferencesList, GetMinorWindowResult(result));
    }
    else
    {
        m_doasFitReferencesList.ShowWindow(SW_HIDE);
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
    columnStr.Format("Column %.2G ± %.2G", column, columnError);
    shiftStr.Format("Shift: %.2lf ± %.2lf", shift, shiftError);
    squeezeStr.Format("Squeeze: %.2lf ± %.2lf", squeeze, squeezeError);

    SetDlgItemText(IDC_RATIO_MAJOR_LBL_COLUMN, columnStr);
    SetDlgItemText(IDC_RATIO_MAJOR_LBL_SHIFT, shiftStr);
    SetDlgItemText(IDC_RATIO_MAJOR_LBL_SQUEEZE, squeezeStr);
}

void CRatioEvaluationDialog::UpdateMajorFitGraph(RatioCalculationResult* result)
{
    int refIndex = m_doasFitReferencesList.GetCurSel();
    if (refIndex < 0)
    {
        m_doasFitReferencesList.SetCurSel(0);
        refIndex = 0;
    }

    m_graph.SetXUnits("Pixel");
    m_graph.SetYUnits("Intensity [-]");

    if (result == nullptr)
    {
        return;
    }

    UpdateDataInDoasFitGraph(m_graph, GetMajorWindowResult(result));

    m_graph.DrawFit(refIndex);

    const novac::DoasResult* doasResult = GetMajorWindowResult(result);
    UpdateReferenceResultLabels(doasResult, refIndex);
}

void CRatioEvaluationDialog::UpdateMinorFitGraph(RatioCalculationResult* result)
{
    int refIndex = m_doasFitReferencesList.GetCurSel();
    if (refIndex < 0)
    {
        m_doasFitReferencesList.SetCurSel(0);
        refIndex = 0;
    }

    if (result == nullptr)
    {
        return;
    }

    UpdateDataInDoasFitGraph(m_graph, GetMinorWindowResult(result));

    m_graph.DrawFit(refIndex);

    const novac::DoasResult* doasResult = GetMinorWindowResult(result);
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

        const auto windows = m_controller->SetupFitWindows();

        m_controller->EvaluateNextScan(windows);

        auto& result = m_controller->m_results.back();

        UpdateListOfResults();

        UpdateUserInterfaceWithResult(&result);

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
UINT EvaluateAllScans(void* pParam)
{
    BackgroundEvaluationInterface* evaluationInterface = (BackgroundEvaluationInterface*)pParam;

    try
    {
        // Setup the evaluation
        const auto windows = evaluationInterface->controller->SetupFitWindows();

        while (evaluationInterface->controller->HasMoreScansToEvaluate() && *(evaluationInterface->evaluationIsRunning) == true)
        {
            // Evaluate one scan
            evaluationInterface->controller->EvaluateNextScan(windows);

            // Copy out the last result and send this to the UI (copy since we are passing data from one thread to another)
            RatioCalculationResult* newResult = new RatioCalculationResult(evaluationInterface->controller->m_results.back());
            PostMessage(evaluationInterface->window, WM_PROGRESS, reinterpret_cast<WPARAM>(newResult), 0);
        }

        *(evaluationInterface->evaluationIsRunning) = false;

        // Tell the user interface that we are done processing
        PostMessage(evaluationInterface->window, WM_DONE, 0, 0);
    }
    catch (std::exception& ex)
    {
        std::stringstream message;
        message << "Ratio calculation failed. Error: " << ex.what();
        std::string* errorMessage = new std::string(message.str());

        *(evaluationInterface->evaluationIsRunning) = false;

        // Tell the user interface that we are done processing
        PostMessage(evaluationInterface->window, WM_DONE, reinterpret_cast<WPARAM>(errorMessage), 0);
    }

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

        // Start the background thread which does the processing
        this->m_backgroundProcessingIsRunning = true;

        // Disable the UI buttons
        UpdateStateWhileBackgroundProcessingIsRunning();

        BackgroundEvaluationInterface* processingSync = new BackgroundEvaluationInterface();
        processingSync->window = this->m_hWnd;
        processingSync->controller = this->m_controller;
        processingSync->evaluationIsRunning = &(this->m_backgroundProcessingIsRunning);
        m_backgroundProcessingThread = AfxBeginThread(EvaluateAllScans, processingSync, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
    }
    catch (std::exception& ex)
    {
        std::stringstream message;
        message << "Ratio calculation failed. Error: " << ex.what();
        MessageBox(message.str().c_str());

        // Re-enable the UI buttons
        UpdateStateWhileBackgroundProcessingIsRunning();
    }
}

void CRatioEvaluationDialog::OnBnClickedSaveResults()
{
    if (m_controller->m_results.empty())
    {
        MessageBox("No results to save. Please run the evaluations first", "No result", MB_OK);
        return;
    }

    CString destinationFileName = "";
    if (Common::BrowseForFile_SaveAs("CSV Data Files\0*.csv\0", destinationFileName))
    {
        std::string dstFileName = novac::EnsureFilenameHasSuffix(std::string(destinationFileName), "csv");
        const std::string columnSeparatorChar = ";"; // TODO: Let the user choose this
        m_controller->SaveResultsToCsvFile(dstFileName, m_controller->m_results, true, columnSeparatorChar);
    }
}

void CRatioEvaluationDialog::SaveSelectedSpectrumResultToCsv()
{
    if (m_controller->m_results.empty())
    {
        return;
    }

    std::vector<RatioCalculationResult> results;
    results.push_back(GetResultToDisplay(m_controller, m_resultsList, m_resultFilterSelector));

    CString destinationFileName = "";
    if (Common::BrowseForFile_SaveAs("CSV Data Files\0*.csv\0", destinationFileName))
    {
        std::string dstFileName = novac::EnsureFilenameHasSuffix(std::string(destinationFileName), "csv");
        const std::string columnSeparatorChar = ";"; // TODO: Let the user choose this
        m_controller->SaveResultsToCsvFile(dstFileName, results, false, columnSeparatorChar);
    }
}

void CRatioEvaluationDialog::SaveSelectedSpectraToPak()
{
    if (m_controller->m_results.empty())
    {
        return;
    }

    auto result = GetResultToDisplay(m_controller, m_resultsList, m_resultFilterSelector);

    CString destinationDirectory = "";
    if (Common::BrowseForDirectory(destinationDirectory))
    {
        std::string dstFileName = std::string(destinationDirectory);
        m_controller->SaveSpectraToPakFile(dstFileName, result);
    }
}

void CRatioEvaluationDialog::SaveSelectedSpectraToStd()
{
    if (m_controller->m_results.empty())
    {
        return;
    }

    auto result = GetResultToDisplay(m_controller, m_resultsList, m_resultFilterSelector);

    CString destinationDirectory = "";
    if (Common::BrowseForDirectory(destinationDirectory))
    {
        std::string dstFileName = std::string(destinationDirectory);
        m_controller->SaveSpectraToStdFile(dstFileName, result);
    }
}

void CRatioEvaluationDialog::AddToListOfResults(const RatioCalculationResult* result)
{
    if (result == nullptr)
    {
        return;
    }
    if (!ResultShouldBeShown(result, m_resultFilterSelector.GetCurSel()))
    {
        return;
    }

    std::string filename = novac::GetFileName(result->filename);
    if (!result->RatioCalculationSuccessful())
    {
        filename = filename + " (no ratio)";
    }

    m_resultsList.AddString(filename.c_str());
    m_resultsList.SetCurSel(m_resultsList.GetCount() - 1); // select the new item
}

void CRatioEvaluationDialog::UpdateListOfResults()
{
    m_resultsList.ResetContent();

    for (const auto& result : m_controller->m_results)
    {
        if (!ResultShouldBeShown(&result, m_resultFilterSelector.GetCurSel()))
        {
            continue;
        }
        std::string filename = novac::GetFileName(result.filename);
        if (!result.debugInfo.errorMessage.empty())
        {
            filename = filename + " (no ratio)";
        }

        m_resultsList.AddString(filename.c_str());
    }

    // Set the focus on the last element in the list.
    if (m_controller->m_results.size() > 0)
    {
        m_resultsList.SetCurSel(static_cast<int>(m_controller->m_results.size() - 1));
    }
}

void CRatioEvaluationDialog::OnBnClickedClearResults()
{
    const int answer = MessageBox("This will remove all results evaluated so far. Are you sure you want to continue?", NULL, MB_YESNO);
    if (IDNO == answer)
    {
        return;
    }

    m_controller->m_results.clear();

    UpdateListOfResults();
    UpdateUserInterfaceWithResult(nullptr);
}

void CRatioEvaluationDialog::OnSelchangeEvaluatedScansSelectorCombo()
{
    UpdateListOfResults();
}

void CRatioEvaluationDialog::OnContextMenu(CWnd* pWnd, CPoint point)
{

    CMenu menu;
    VERIFY(menu.LoadMenu(IDR_RATIO_RESULT_CONTEXTMENU));
    CMenu* pPopup = menu.GetSubMenu(0);
    ASSERT(pPopup != NULL);

    pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
}
