#pragma once

#include "GridCtrl\GridCtrl.h"

namespace novac
{
class CFitWindow;
}

namespace DlgControls
{

class CReferenceFileControl : public CGridCtrl
{
public:
    CReferenceFileControl(CWnd* parentWindow);
    ~CReferenceFileControl();

    DECLARE_MESSAGE_MAP()

public:

    /** Pointer to the fit window that this grid controls */
    novac::CFitWindow* m_window = nullptr;

private:

    /** Handle to the parent */
    CWnd* parent;

    /** Called when the user has finished editing one cell */
    void OnEndEditCell(int nRow, int nCol, CString str);

    /** Called to show a context menu */
    afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);

};
}