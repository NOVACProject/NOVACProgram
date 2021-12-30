#pragma once


namespace Dialogs {
    // CSimplePropertySheet

    class CSimplePropertySheet : public CPropertySheet
    {
        DECLARE_DYNAMIC(CSimplePropertySheet)

    public:
        CSimplePropertySheet();
        CSimplePropertySheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
        CSimplePropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
        virtual ~CSimplePropertySheet();

        /** Initializing the dialog */
        virtual BOOL OnInitDialog();

    protected:
        DECLARE_MESSAGE_MAP()
    };
}