#pragma once

#include "SimplePropertySheet.h"

namespace Dialogs
{
    // CExportDlg

    class CExportDlg : public CSimplePropertySheet
    {
        DECLARE_DYNAMIC(CExportDlg)

    public:
        CExportDlg(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
        CExportDlg();
        CExportDlg(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
        virtual ~CExportDlg();

    protected:
        DECLARE_MESSAGE_MAP()
    };
}
