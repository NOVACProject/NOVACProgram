// CRatioSetupDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "afxdialogex.h"
#include "../resource.h"
#include "CRatioSetupDialog.h"


// CRatioSetupDialog dialog

IMPLEMENT_DYNAMIC(CRatioSetupDialog, CPropertyPage)

CRatioSetupDialog::CRatioSetupDialog(CWnd* pParent /*=nullptr*/)
    : CPropertyPage(IDD_RATIO_WINDOW_SETUP_DIALOG)
{

}

CRatioSetupDialog::~CRatioSetupDialog()
{
}

BOOL CRatioSetupDialog::OnInitDialog() {
    CPropertyPage::OnInitDialog();

    return TRUE;  // return TRUE unless you set the focus to a control
}

void CRatioSetupDialog::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CRatioSetupDialog, CPropertyPage)
END_MESSAGE_MAP()


// CRatioSetupDialog message handlers
