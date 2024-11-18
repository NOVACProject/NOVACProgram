#pragma once

namespace ReEvaluation
{
class CReEvaluator;

class CPakFileListBox : public CListBox
{
    DECLARE_DYNAMIC(CPakFileListBox)

public:
    CPakFileListBox(CReEvaluator& reeval, CWnd* parent);
    virtual ~CPakFileListBox();

    /** Called to populate the fit window list */
    void PopulateList();

protected:
    DECLARE_MESSAGE_MAP()

private:

    /** A handle to the reevaluator that this object modifies */
    CReEvaluator& m_reeval;

    /** The parent window */
    CWnd* m_parent;

    /** Called when the user presses down the left mouse button */
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);

    /** Called to show the context menu */
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);
};
}