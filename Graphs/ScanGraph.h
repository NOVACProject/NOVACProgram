#pragma once
#include "GraphCtrl.h"

namespace Graph
{

class CScanGraph :
    public CGraphCtrl
{
public:
    CScanGraph(void);
    ~CScanGraph(void);

    /** A parent window, all messages will be directed to this window */
    CWnd* parent;

    DECLARE_MESSAGE_MAP()
    afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);


};
}
