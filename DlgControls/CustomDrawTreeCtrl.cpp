#include "StdAfx.h"
#include "CustomDrawTreeCtrl.h"


IMPLEMENT_DYNAMIC(CCustomDrawTreeCtrl, CTreeCtrl)

BEGIN_MESSAGE_MAP(CCustomDrawTreeCtrl, CNonFlickeringTreeControl)
    ON_WM_PAINT()
END_MESSAGE_MAP()

void CCustomDrawTreeCtrl::DeleteAllItems()
{
    m_customElements.clear();
    CTreeCtrl::DeleteAllItems();
}

void CCustomDrawTreeCtrl::SetItemColor(HTREEITEM item, COLORREF color)
{
    const auto current = m_customElements.find(item);
    if (current != m_customElements.end())
    {
        FontWithColor fontWithColor = current->second;
        fontWithColor.color = color;
        m_customElements[item] = fontWithColor;
    }
    else
    {
        FontWithColor fontWithColor{};
        fontWithColor.color = color;
        fontWithColor.logfont.lfFaceName[0] = '\0';
        m_customElements[item] = fontWithColor;
    }
}

void CCustomDrawTreeCtrl::OnPaint()
{
    CPaintDC dc(this);

    // Create a memory DC compatible with the paint DC
    CDC memDC;
    memDC.CreateCompatibleDC(&dc);

    CRect rcClip, rcClient;
    dc.GetClipBox(&rcClip);
    GetClientRect(&rcClient);

    // Select a compatible bitmap into the memory DC
    CBitmap bitmap;
    bitmap.CreateCompatibleBitmap(&dc, rcClient.Width(), rcClient.Height());
    memDC.SelectObject(&bitmap);

    // Set clip region to be same as that in paint DC
    CRgn rgn;
    rgn.CreateRectRgnIndirect(&rcClip);
    memDC.SelectClipRgn(&rgn);
    rgn.DeleteObject();

    // First let the control do its default drawing.
    CWnd::DefWindowProc(WM_PAINT, (WPARAM)memDC.m_hDC, 0);

    HTREEITEM hItem = GetFirstVisibleItem();

    int iItemCount = GetVisibleCount() + 1;
    while (hItem && iItemCount--)
    {
        CRect rect;

        // Do not meddle with selected items or drop highlighted items
        UINT selflag = TVIS_DROPHILITED | TVIS_SELECTED;

        //if ( !(GetTreeCtrl().GetItemState( hItem, selflag ) & selflag ) 
        //	&& m_mapColorFont.Lookup( hItem, cf ))

        if ((GetItemState(hItem, selflag) & selflag) && ::GetFocus() == m_hWnd)
        {

        }
        else
        {
            const auto it = m_customElements.find(hItem);
            if (it != m_customElements.end())
            {
                FontWithColor cf = it->second;

                CFont* pFontDC;
                CFont fontDC;
                LOGFONT logfont;

                if (cf.logfont.lfFaceName[0] != '\0')
                    logfont = cf.logfont;
                else {
                    // No font specified, so use window font
                    CFont* pFont = GetFont();
                    pFont->GetLogFont(&logfont);
                }

                fontDC.CreateFontIndirect(&logfont);
                pFontDC = memDC.SelectObject(&fontDC);

                if (cf.color != (COLORREF)-1)
                    memDC.SetTextColor(cf.color);
                else
                    memDC.SetTextColor(GetSysColor(COLOR_WINDOWTEXT));


                CString sItem = GetItemText(hItem);

                GetItemRect(hItem, &rect, TRUE);
                memDC.SetBkColor(GetSysColor(COLOR_WINDOW));
                memDC.TextOut(rect.left + 2, rect.top + 1, sItem);

                memDC.SelectObject(pFontDC);
            }
            hItem = GetNextVisibleItem(hItem);
        }
    }


    dc.BitBlt(rcClip.left, rcClip.top, rcClip.Width(), rcClip.Height(), &memDC,
                rcClip.left, rcClip.top, SRCCOPY);

    memDC.DeleteDC();
}