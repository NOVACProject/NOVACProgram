// CRatioEvaluationDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "afxdialogex.h"
#include "../resource.h"
#include "CRatioEvaluationDialog.h"


// CRatioEvaluationDialog dialog

IMPLEMENT_DYNAMIC(CRatioEvaluationDialog, CPropertyPage)

CRatioEvaluationDialog::CRatioEvaluationDialog(CWnd* pParent /*=nullptr*/)
    : CPropertyPage(IDD_RATIO_EVALUATE_DIALOG)
{

}

CRatioEvaluationDialog::~CRatioEvaluationDialog()
{
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
