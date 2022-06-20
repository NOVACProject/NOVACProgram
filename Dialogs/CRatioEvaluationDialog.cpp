// CRatioEvaluationDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "afxdialogex.h"
#include "../resource.h"
#include "CRatioEvaluationDialog.h"

#include <SpectralEvaluation/DialogControllers/RatioCalculationController.h>


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

    return TRUE;  // return TRUE unless you set the focus to a control
}

void CRatioEvaluationDialog::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CRatioEvaluationDialog, CPropertyPage)
END_MESSAGE_MAP()


// CRatioEvaluationDialog message handlers
