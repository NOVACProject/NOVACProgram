#include "stdafx.h"
#include "../resource.h"       // main symbols
#include "SimplePropertySheet.h"

using namespace Dialogs;
// CSimplePropertySheet

IMPLEMENT_DYNAMIC(CSimplePropertySheet, CPropertySheet)
CSimplePropertySheet::CSimplePropertySheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
    :CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
}

CSimplePropertySheet::CSimplePropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
    : CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
}

CSimplePropertySheet::CSimplePropertySheet()
    : CPropertySheet()
{
}

CSimplePropertySheet::~CSimplePropertySheet()
{
}


BEGIN_MESSAGE_MAP(CSimplePropertySheet, CPropertySheet)
END_MESSAGE_MAP()


// CSimplePropertySheet message handlers

BOOL Dialogs::CSimplePropertySheet::OnInitDialog()
{
    BOOL bResult = CPropertySheet::OnInitDialog();

    CRect rect;

    // Get the buttons...
    CWnd* pApply = this->GetDlgItem(ID_APPLY_NOW);
    CWnd* pCancel = this->GetDlgItem(IDCANCEL);
    CWnd* pOk = this->GetDlgItem(IDOK);

    // Get the position of the 'Apply'-button, and then remove it
    if (pApply) {
        pApply->GetWindowRect(rect);
        ScreenToClient(rect);
        pApply->DestroyWindow();
    }

    // remove the 'OK'-button
    if (pOk)
        pOk->DestroyWindow();

    // Change the 'Cancel'-button to a 'Close'-button, move it to where the 
    //	'Apply'-button was
    if (pCancel) {
        pCancel->SetWindowText("Close");
        pCancel->MoveWindow(rect);
    }

    return bResult;
}