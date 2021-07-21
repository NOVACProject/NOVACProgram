#include "StdAfx.h"
#include "ScanGraph.h"
#include "../resource.h"

using namespace Graph;

CScanGraph::CScanGraph(void)
{
    parent = NULL;
}

CScanGraph::~CScanGraph(void)
{
    parent = NULL;
}

BEGIN_MESSAGE_MAP(CScanGraph, CGraphCtrl)
    ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()


void CScanGraph::OnContextMenu(CWnd* pWnd, CPoint point) {
    CMenu menu;
    VERIFY(menu.LoadMenu(IDR_MENU_SHOWFIT_CONTEXT));
    CMenu* pPopup = menu.GetSubMenu(0);
    ASSERT(pPopup != NULL);

    //		pPopup->EnableMenuItem(ID__SAVEASBITMAP, MF_DISABLED | MF_GRAYED);

    if (parent != NULL) {
        pPopup->EnableMenuItem(ID__SAVEASASCII, MF_DISABLED | MF_GRAYED);

        pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, parent);
    }

}