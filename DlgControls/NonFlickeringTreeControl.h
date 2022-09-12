#pragma once

#include "afxdlgs.h"

// The CNonFlickeringTreeControl attempts to resolve one problem with the MFC CTreeControl,
// the tree will flicker (alot) when adding/remvoing objects from the tree as each change 
// causes a redraw of the entire control.
// Idea and code taken from https://stackoverflow.com/questions/17448981/pause-rendering-drawing-ctreectrl-mfc
class CNonFlickeringTreeControl : public CTreeCtrl
{
public:
    void DisableRedraw()
    {
        SetRedraw(FALSE);
        ModifyStyle(NULL, TVS_NOSCROLL);
    }

    void EnableRedraw()
    {
        ModifyStyle(TVS_NOSCROLL, NULL);
        SetRedraw(TRUE);
        RedrawWindow(NULL, NULL, RDW_NOCHILDREN | RDW_UPDATENOW | RDW_INVALIDATE);
    }
};