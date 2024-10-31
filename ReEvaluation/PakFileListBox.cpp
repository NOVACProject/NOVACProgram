
#include "stdafx.h"
#include "../NovacMasterProgram.h"
#include "../Dialogs/QueryStringDialog.h"
#include "../Evaluation/FitWindowFileHandler.h"
#include "PakFileListBox.h"

using namespace ReEvaluation;

// CPakFileListBox

IMPLEMENT_DYNAMIC(CPakFileListBox, CListBox)
CPakFileListBox::CPakFileListBox()
{
    m_reeval = nullptr;
    m_parent = nullptr;
}

CPakFileListBox::~CPakFileListBox()
{
    m_reeval = nullptr;
    m_parent = nullptr;
}


BEGIN_MESSAGE_MAP(CPakFileListBox, CListBox)
    ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()



// CPakFileListBox message handlers

void CPakFileListBox::PopulateList()
{
    // first remove everything that is already there
    this->ResetContent();

    // then add one string for every scan file selected
    for (size_t i = 0; i < m_reeval->m_scanFile.size(); ++i)
    {
        this->AddString(m_reeval->m_scanFile[i].c_str());
    }

    // Find the longest string in the list box.
    CString      str;
    CSize      sz;
    int      dx = 0;
    TEXTMETRIC   tm;
    CDC* pDC = this->GetDC();
    CFont* pFont = this->GetFont();

    // Select the listbox font, save the old font
    CFont* pOldFont = pDC->SelectObject(pFont);
    // Get the text metrics for avg char width
    pDC->GetTextMetrics(&tm);

    for (int i = 0; i < this->GetCount(); i++)
    {
        this->GetText(i, str);
        sz = pDC->GetTextExtent(str);

        // Add the avg width to prevent clipping
        sz.cx += tm.tmAveCharWidth;

        if (sz.cx > dx)
            dx = sz.cx;
    }
    // Select the old font back into the DC
    pDC->SelectObject(pOldFont);
    this->ReleaseDC(pDC);

    // Set the horizontal extent so every character of all strings can be scrolled to.
    this->SetHorizontalExtent(dx);
}

/** Called when the user presses down the left mouse button */
void CPakFileListBox::OnLButtonDown(UINT nFlags, CPoint point)
{
    CListBox::OnLButtonDown(nFlags, point);
}

/** Called to show the context menu */
void CPakFileListBox::OnContextMenu(CWnd* pWnd, CPoint pos)
{
    if (m_parent == nullptr)
        return;

    OnLButtonDown(MK_LBUTTON, pos); // make the current menu item marked

    CMenu menu;
    VERIFY(menu.LoadMenu(IDR_REEVAL_SCAN_CONTEXTMENU));
    CMenu* pPopup = menu.GetSubMenu(0);
    ASSERT(pPopup != nullptr);

    // If there are no files opened, then none can be removed...
    if (m_reeval->m_scanFile.size() == 0)
    {
        pPopup->EnableMenuItem(ID__REMOVESELECTED, MF_DISABLED | MF_GRAYED);
    }

    // show the popup menu
    pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pos.x, pos.y, m_parent);

}
