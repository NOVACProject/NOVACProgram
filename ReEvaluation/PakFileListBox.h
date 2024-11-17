#pragma once

#include "ReEvaluator.h"

// CPakFileListBox

namespace ReEvaluation
{

class CPakFileListBox : public CListBox
{
    DECLARE_DYNAMIC(CPakFileListBox)

public:
    CPakFileListBox(CReEvaluator& reeval);
    virtual ~CPakFileListBox();

    /** Called to populate the fit window list */
    void PopulateList();

    /** The parent window */
    CWnd* m_parent;

protected:
    DECLARE_MESSAGE_MAP()

private:

    /** A handle to the reevaluator that this object modifies */
    CReEvaluator& m_reeval;

    /** Called when the user presses down the left mouse button */
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);

    /** Called to show the context menu */
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);
};
}