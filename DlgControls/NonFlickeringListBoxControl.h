#pragma once

#include "afxdlgs.h"

// The CNonFlickeringTreeControl attempts to resolve one problem with the MFC CTreeControl,
// the tree will flicker (alot) when adding/remvoing objects from the tree as each change 
// causes a redraw of the entire control.
// Idea and code taken from https://stackoverflow.com/questions/17448981/pause-rendering-drawing-ctreectrl-mfc
class CNonFlickeringListBoxControl : public CListBox
{
public:
    void DisableRedraw()
    {
        SetRedraw(FALSE);
    }

    void EnableRedraw()
    {
        SetRedraw(TRUE);
    }
};