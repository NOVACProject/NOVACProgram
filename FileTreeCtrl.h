#pragma once


// CFileTreeCtrl

namespace DlgControls {
    /** The <b>CFileTreeCtrl</b> is a specialized class to show the
            file-structure on the electronics box in the FileTransferDialog. */
    class CFileTreeCtrl : public CTreeCtrl
    {
        DECLARE_DYNAMIC(CFileTreeCtrl)

    public:
        CFileTreeCtrl();
        virtual ~CFileTreeCtrl();

    protected:
        DECLARE_MESSAGE_MAP()
    public:
        //void OnItemexpanding(NMHDR* pNMHDR, LRESULT* pResult);
        afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
        afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
        void SetWnd(CWnd* pWnd);
    public:
        //variables
        CWnd* parent;
    };
}