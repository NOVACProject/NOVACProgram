// FileTreeCtrl.cpp : implementation file
//

#include "stdafx.h"
//#include "Dialogs/FileTransferDlg.h"
#include "FileTreeCtrl.h"
#include "NovacMasterProgram.h"
#include "Common/Common.h"


// CFileTreeCtrl

using namespace DlgControls;

IMPLEMENT_DYNAMIC(CFileTreeCtrl, CTreeCtrl)
CFileTreeCtrl::CFileTreeCtrl()
{
	parent = NULL;
}

CFileTreeCtrl::~CFileTreeCtrl()
{
	parent = NULL;
}


BEGIN_MESSAGE_MAP(CFileTreeCtrl, CTreeCtrl)
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()



// CFileTreeCtrl message handlers
void CFileTreeCtrl::SetWnd(CWnd* pWnd)
{
	//m_Wnd = pWnd;
}
//void CFileTreeCtrl::OnItemexpanding(NMHDR* pNMHDR, LRESULT* pResult)
//{
//	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
//	// TODO: Add your control notification handler code here
//
//	// find out what item is getting expanded, and send that to Expand(hItem, TVE_EXPAND)
//	if (pNMTreeView->hdr.code == TVN_ITEMEXPANDING)
//	{
//		HTREEITEM hIT = pNMTreeView->itemNew.hItem;
//		CString cstr, strPath;
//
//		// build up the path to htreeitem
//		strPath = GetItemText(hIT);
//
//		while (hIT != GetRootItem())
//		{
//			hIT = GetParentItem(hIT);
//
//			if (hIT == GetRootItem())
//				cstr.Format(_T("/%s"), (LPCTSTR)strPath);
//			else
//				cstr.Format(_T("%s/%s"), (LPCTSTR)GetItemText(hIT), (LPCTSTR)strPath);
//
//			strPath = cstr;
//		}
//
//		// use that dir to call ExploreDir
////		ExploreDir(strPath, pNMTreeView->itemNew.hItem);
//	}
//	*pResult = 0;
//}

void CFileTreeCtrl::OnLButtonDown(UINT nFlags,CPoint point)
{
	//set selected item
	HTREEITEM hItem = this->HitTest(point, &nFlags);

	if ((hItem != NULL) && (TVHT_ONITEM & nFlags))
	{
		this->SelectItem(hItem);
	}
	CTreeCtrl::OnLButtonDown(nFlags,point);
	// Select the item that is at the point myPoint.
	
}
void CFileTreeCtrl::OnRButtonDown(UINT nFlags,CPoint point)
{
	CString itemText;

	OnLButtonDown(nFlags,point);
	//load popup menu
	CMenu menu;
  VERIFY(menu.LoadMenu(IDR_FILETREE_POP_MENU));
  CMenu* pPopup = menu.GetSubMenu(0);
  ASSERT(pPopup != NULL);

	// TODO: enable or disable the items depending on what we've marked...
	// 1. Get the name of the currently selected item
	itemText		= this->GetItemText(GetSelectedItem());
	int length	= strlen(itemText);

	if(length > 4 && Equals(itemText.Left(4), "disk")){
		// this is a disk
    pPopup->EnableMenuItem(ID_FILETREE_VIEWFILE,			MF_DISABLED | MF_GRAYED);
    pPopup->EnableMenuItem(ID_FILETREE_DELETE,				MF_DISABLED | MF_GRAYED);
		pPopup->ModifyMenu(ID_FILETREE_DOWNLOAD, MF_BYCOMMAND, ID_FILETREE_DOWNLOAD, "Download All Files");
	}else if(length == 4 && Equals(itemText.Left(1), "R")){
		// This is most likely a directory
    pPopup->EnableMenuItem(ID_FILETREE_VIEWFILE,			MF_DISABLED | MF_GRAYED);
    pPopup->EnableMenuItem(ID_FILETREE_DELETE,				MF_DISABLED | MF_GRAYED);
		pPopup->ModifyMenu(ID_FILETREE_DOWNLOAD, MF_BYCOMMAND, ID_FILETREE_DOWNLOAD, "Download Folder");
	}else{
		// this is not a directory...
    pPopup->EnableMenuItem(ID_FILETREE_ENTERFOLDER,		MF_DISABLED | MF_GRAYED);
		pPopup->ModifyMenu(ID_FILETREE_DOWNLOAD, MF_BYCOMMAND, ID_FILETREE_DOWNLOAD, "Download");

		// check if this is a text-file
		if(length > 4 && (Equals(itemText.Right(4), ".txt") || Equals(itemText.Right(4), ".bat") || Equals(itemText.Right(4), ".ini"))){
			// this is a text-file
		}else{
	    pPopup->EnableMenuItem(ID_FILETREE_VIEWFILE,			MF_DISABLED | MF_GRAYED);
		}
	}

	CRect rect;
	GetWindowRect(rect);
	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x + rect.left, point.y + rect.top, parent);
	
}