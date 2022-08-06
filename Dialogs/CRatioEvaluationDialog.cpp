// CRatioEvaluationDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "afxdialogex.h"
#include "../resource.h"
#include "CRatioEvaluationDialog.h"

#include "../Common/Common.h"

#include <SpectralEvaluation/DialogControllers/RatioCalculationController.h>
#include <SpectralEvaluation/Spectra/Spectrum.h>
#include <SpectralEvaluation/VectorUtils.h>

#include <sstream>

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
    m_so2FitGraph.parentWnd = this;

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
    m_broFitGraph.parentWnd = this;

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
}


BEGIN_MESSAGE_MAP(CRatioEvaluationDialog, CPropertyPage)
    ON_BN_CLICKED(IDC_RATIO_RUN_EVALUATION_NEXT_SCAN, &CRatioEvaluationDialog::OnBnClickedRunEvaluationOnNextScan)
    ON_BN_CLICKED(IDC_RATIO_RUN_EVALUATION_NEXT_SCAN, &CRatioEvaluationDialog::OnBnClickedRunEvaluationOnAllScans)
END_MESSAGE_MAP()

// CRatioEvaluationDialog message handlers

void CRatioEvaluationDialog::UpdateGraphs()
{
    UpdateScanGraph();
    UpdateMajorFitGraph();
    UpdateMinorFitGraph();
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
    m_scanGraph.SetPlotColor(RGB(255, 0, 0));
    m_scanGraph.BarChart(scanAngles.data(), columns.data(), columnErrors.data(), static_cast<int>(scanAngles.size()), Graph::CGraphCtrl::PLOT_FIXED_AXIS | Graph::CGraphCtrl::PLOT_BAR_SHOW_X);

    // The selected in-plume column-values
    std::vector<double> inPlumeScanAngles;
    std::vector<double> inPlumeColumns;
    for (int ii : m_controller->m_lastResult.debugInfo.plumeSpectra)
    {
        const novac::CEvaluationResult& result = m_controller->m_lastResult.initialEvaluation.m_spec[ii];
        const novac::CSpectrumInfo& spectrumInfo = m_controller->m_lastResult.initialEvaluation.m_specInfo[ii];

        inPlumeScanAngles.push_back(spectrumInfo.m_scanAngle);
        inPlumeColumns.push_back(result.m_referenceResult[0].m_column - columnOffset);
    }

    m_scanGraph.SetPlotColor(RGB(0, 255, 0));
    m_scanGraph.BarChart(inPlumeScanAngles.data(), inPlumeColumns.data(), static_cast<int>(inPlumeColumns.size()), Graph::CGraphCtrl::PLOT_FIXED_AXIS | Graph::CGraphCtrl::PLOT_BAR_SHOW_X);

    // The selected out-of-plume column-values
    std::vector<double> outOfPlumeScanAngles;
    std::vector<double> outOfPlumeColumns;
    for (int ii : m_controller->m_lastResult.debugInfo.outOfPlumeSpectra)
    {
        const novac::CEvaluationResult& result = m_controller->m_lastResult.initialEvaluation.m_spec[ii];
        const novac::CSpectrumInfo& spectrumInfo = m_controller->m_lastResult.initialEvaluation.m_specInfo[ii];

        outOfPlumeScanAngles.push_back(spectrumInfo.m_scanAngle);
        outOfPlumeColumns.push_back(result.m_referenceResult[0].m_column - columnOffset);
    }

    m_scanGraph.SetPlotColor(RGB(128, 0, 0));
    m_scanGraph.BarChart(outOfPlumeScanAngles.data(), outOfPlumeColumns.data(), static_cast<int>(outOfPlumeScanAngles.size()), Graph::CGraphCtrl::PLOT_FIXED_AXIS | Graph::CGraphCtrl::PLOT_BAR_SHOW_X);
}

void CRatioEvaluationDialog::UpdateMajorFitGraph()
{

}

void CRatioEvaluationDialog::UpdateMinorFitGraph()
{

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

            UpdateGraphs();
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
