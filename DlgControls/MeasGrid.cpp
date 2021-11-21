#include "stdafx.h"
#include "measgrid.h"
#include "../resource.h"

using namespace DlgControls;

CMeasGrid::CMeasGrid(void)
{
}

CMeasGrid::~CMeasGrid(void)
{
}
BEGIN_MESSAGE_MAP(CMeasGrid, CGridCtrl)
    ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

void CMeasGrid::OnContextMenu(CWnd* pWnd, CPoint point) {
    CMenu menu;
    VERIFY(menu.LoadMenu(IDR_MEASUREMENT_MENU));
    CMenu* pPopup = menu.GetSubMenu(0);
    ASSERT(pPopup != NULL);

    CCellRange cellRange = GetSelectedCellRange();
    int minRow = cellRange.GetMinRow() - 1;
    int nRows = cellRange.GetRowSpan();

    if (nRows <= 0) { /* nothing selected*/
        pPopup->EnableMenuItem(ID__INSERTROW, MF_DISABLED | MF_GRAYED);
        pPopup->EnableMenuItem(ID__DELETEROW, MF_DISABLED | MF_GRAYED);
    }

    pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, parent);

}

/* Called when the user has edited one cell */
void CMeasGrid::OnEndEditCell(int nRow, int nCol, CString str) {
    CGridCtrl::OnEndEditCell(nRow, nCol, str);

    return;
}

