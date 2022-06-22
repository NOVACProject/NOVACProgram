// CRatioEvaluationDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "afxdialogex.h"
#include "../resource.h"
#include "CRatioEvaluationDialog.h"

#include "../Common/Common.h"

#include <SpectralEvaluation/DialogControllers/RatioCalculationController.h>
#include <SpectralEvaluation/Spectra/Spectrum.h>


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

    return TRUE;  // return TRUE unless you set the focus to a control
}

void CRatioEvaluationDialog::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_FIT_FRAME_SO2, m_fitFrameSO2);
    DDX_Control(pDX, IDC_FIT_FRAME_BRO, m_fitFrameBrO);
}


BEGIN_MESSAGE_MAP(CRatioEvaluationDialog, CPropertyPage)
END_MESSAGE_MAP()


// CRatioEvaluationDialog message handlers
