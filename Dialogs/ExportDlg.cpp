#include "stdafx.h"
#include "../NovacMasterProgram.h"
#include "ExportDlg.h"

using namespace Dialogs;

// CExportDlg

IMPLEMENT_DYNAMIC(CExportDlg, CPropertySheet)

CExportDlg::CExportDlg()
{
}

CExportDlg::CExportDlg(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
    :CSimplePropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
}

CExportDlg::CExportDlg(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
    : CSimplePropertySheet(pszCaption, pParentWnd, iSelectPage)
{
}

CExportDlg::~CExportDlg()
{
}


BEGIN_MESSAGE_MAP(CExportDlg, CSimplePropertySheet)
END_MESSAGE_MAP()


// CExportDlg message handlers

