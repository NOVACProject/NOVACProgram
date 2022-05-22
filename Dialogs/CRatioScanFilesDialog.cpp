// CRatioScanFilesDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "afxdialogex.h"
#include "../resource.h"
#include "CRatioScanFilesDialog.h"


// CRatioScanFilesDialog dialog

IMPLEMENT_DYNAMIC(CRatioScanFilesDialog, CPropertyPage)

CRatioScanFilesDialog::CRatioScanFilesDialog(CWnd* pParent /*=nullptr*/)
    : CPropertyPage(IDD_RATIO_SCANFILES_DIALOG)
{
}

CRatioScanFilesDialog::~CRatioScanFilesDialog()
{
}

BOOL CRatioScanFilesDialog::OnInitDialog() {
    CPropertyPage::OnInitDialog();

    return TRUE;  // return TRUE unless you set the focus to a control
}

void CRatioScanFilesDialog::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CRatioScanFilesDialog, CPropertyPage)
END_MESSAGE_MAP()


// CRatioScanFilesDialog message handlers
