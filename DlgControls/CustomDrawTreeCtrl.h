#pragma once

#include "afxdlgs.h"
#include "NonFlickeringTreeControl.h"
#include <unordered_map>

// The CCustomDrawTreeCtrl makes it possible to customize how each element in the tree is displayed
// by using a custom draw method.
// Strongly inspired by https://www.codeproject.com/articles/2340/ctreectrlex-setting-color-and-font-attribute-for-i
class CCustomDrawTreeCtrl : public CNonFlickeringTreeControl
{
    DECLARE_DYNAMIC(CCustomDrawTreeCtrl)

public:
    CCustomDrawTreeCtrl() { }
    virtual ~CCustomDrawTreeCtrl() {}

    // Set the color of a single HTREEITEM
    void SetItemColor(HTREEITEM, COLORREF);

    void DeleteAllItems();

protected:

    afx_msg void OnPaint();

    DECLARE_MESSAGE_MAP()

private:

    struct FontWithColor {
        COLORREF color;
        LOGFONT  logfont;
    };

    // listing which elements in the tree have a custom font and color.
    std::unordered_map< HTREEITEM, FontWithColor> m_customElements;
};