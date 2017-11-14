
#include "stdafx.h"
#include "../NovacMasterProgram.h"
#include "../Dialogs/QueryStringDialog.h"
#include "../Evaluation/FitWindowFileHandler.h"
#include "FitWindowListBox.h"

using namespace ReEvaluation;

// CFitWindowListBox

IMPLEMENT_DYNAMIC(CFitWindowListBox, CListBox)
CFitWindowListBox::CFitWindowListBox()
{
	m_reeval = NULL;
}

CFitWindowListBox::~CFitWindowListBox()
{
	m_reeval = NULL;
}


BEGIN_MESSAGE_MAP(CFitWindowListBox, CListBox)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID__INSERTFITWINDOW,			OnInsertFitWindow)
	ON_COMMAND(ID__REMOVEWINDOW,				OnRemoveFitWindow)

	ON_COMMAND(ID__LOADWINDOWFROMFILE,	OnLoadFitWindows)
	ON_COMMAND(ID_FILE_LOADFITWINDOWS,	OnLoadFitWindows)

	ON_COMMAND(ID__SAVEWINDOWTOFILE,		OnSaveFitWindows)
	ON_COMMAND(ID_FILE_SAVECURRENTFITWINDOWS, OnSaveFitWindows)

	ON_COMMAND(ID__RENAMEWINDOW,				OnRenameWindow)
END_MESSAGE_MAP()



// CFitWindowListBox message handlers

/** Called to populate the list */
void CFitWindowListBox::PopulateList(){
	if(m_reeval == NULL)
		return;

	this->ResetContent(); // clear the list

	for(int i = 0; i < m_reeval->m_windowNum; ++i){
		this->AddString(m_reeval->m_window[i].name);
	}
}

/** Called when the user presses down the left mouse button */
void CFitWindowListBox::OnLButtonDown(UINT nFlags, CPoint point){
	CListBox::OnLButtonDown(nFlags, point);
}

/** Called to show the context menu */
void CFitWindowListBox::OnContextMenu(CWnd *pWnd, CPoint pos){
	OnLButtonDown(MK_LBUTTON, pos); // make the current menu item marked

	CMenu menu;
	VERIFY(menu.LoadMenu(IDR_FITWINDOWLIST_MENU));
	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT(pPopup != NULL);

	// There has to be at least one fit window defined at all times
	//	if there are to few, don't allow the user to remove any
	if(m_reeval->m_windowNum <= 1){
		pPopup->EnableMenuItem(ID__REMOVEWINDOW, MF_DISABLED | MF_GRAYED);
	}

	// If the list of fit windows is full, don't let the user
	//	add any more fit windows
	if(m_reeval->m_windowNum == CReEvaluator::MAX_N_WINDOWS-1){
		pPopup->EnableMenuItem(ID__INSERTFITWINDOW, MF_DISABLED | MF_GRAYED);
		pPopup->EnableMenuItem(ID__LOADWINDOWFROMFILE, MF_DISABLED | MF_GRAYED);
	}

	// show the popup menu
	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pos.x, pos.y, this);

}

/** Called to insert a fit window into the list */
void CFitWindowListBox::OnInsertFitWindow(){
	CString name;

	// Make sure the list box is initialized ok.
	if(m_reeval == NULL)
		return;

	// Make sure that there's enough space to store one more window 
	if(m_reeval->m_windowNum == CReEvaluator::MAX_N_WINDOWS - 1)
		return;

	// Ask the user for the name of the window
	Dialogs::CQueryStringDialog nameDialog;
	nameDialog.m_windowText.Format("The name of the fit window?");
	nameDialog.m_inputString = &name;
	INT_PTR ret = nameDialog.DoModal();

	if(IDCANCEL == ret)
		return;

	// insert an empty fit window.
	m_reeval->m_window[m_reeval->m_windowNum].name.Format("%s", name);
	m_reeval->m_windowNum += 1;

	// Update the list
	PopulateList();

	// Select the fit window
	SetCurSel(m_reeval->m_windowNum - 1);
}

/** Called to load a set of fit window from a file */
void CFitWindowListBox::OnLoadFitWindows(){
	FileHandler::CFitWindowFileHandler fitWindowReader;
	if(NULL == m_reeval)
		return;

	if(m_reeval->m_windowNum == m_reeval->MAX_N_WINDOWS - 1)
		return;

	// Ask for a file to read from
	CString fileName;
	fileName.Format("");
	TCHAR filter[512];
	int n = _stprintf(filter, "Novac Fit Window Files\0");
	n += _stprintf(filter + n + 1, "*.nfw;\0");
	filter[n + 2] = 0;
	Common common;

	// let the user browse for an evaluation log file and if one is selected, read it
	if(common.BrowseForFile(filter, fileName)){
		int index = 0;							// <-- the fit window index in the file
		m_reeval->m_windowNum = 0;	// <-- empty the list of fit-windows

		while(m_reeval->m_windowNum < m_reeval->MAX_N_WINDOWS){
			if(SUCCESS == fitWindowReader.ReadFitWindow(m_reeval->m_window[m_reeval->m_windowNum], fileName, index)){
				++index;
				++m_reeval->m_windowNum;
			}else{
				break;
			}
		}
	}

	// Update the window
	this->PopulateList();
	this->SetCurSel(0);

	// Need to tell the parent window to update
	CWnd *pWnd = GetParent();
	if(pWnd)
		pWnd->SendMessage(WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), LBN_SELCHANGE), (LPARAM)m_hWnd);
}

/** Called to save a set of fit windows to file */
void CFitWindowListBox::OnSaveFitWindows(){
	FileHandler::CFitWindowFileHandler fitWindowWriter;
	if(NULL == m_reeval)
		return;

	if(0 == m_reeval->m_windowNum)
		return;

	// Ask for a file to save the data to
	CString fileName;
	fileName.Format("");
	TCHAR filter[512];
	int n = _stprintf(filter, "Novac Fit Window Files\0");
	n += _stprintf(filter + n + 1, "*.nfw;\0");
	filter[n + 2] = 0;
	Common common;

	// let the user browse for an evaluation log file and if one is selected, read it
	if(common.BrowseForFile_SaveAs(filter, fileName)){
		// if there's not a .nfw-ending on the file, append it!
		if(!Equals(".nfw", fileName.Right(4))){
			fileName.AppendFormat(".nfw");
		}

		bool overWrite = true;	// the first window to be saved overwrites the file, the others appends the file

		for(int i = 0; i < m_reeval->m_windowNum; ++i){
			fitWindowWriter.WriteFitWindow(m_reeval->m_window[i], fileName, overWrite);
			overWrite = false;
		}
	}
}

/** Called to rename a fit window */
void CFitWindowListBox::OnRenameWindow(){
	CString name;

	if(m_reeval == NULL)
		return;

	int curSel = GetCurSel();
	if(curSel < 0 || curSel > m_reeval->MAX_N_WINDOWS)
		return;

	// Let the initial guess for the name be the old name of the window
	name.Format(m_reeval->m_window[curSel].name);

	// Ask the user for the name of the window
	Dialogs::CQueryStringDialog nameDialog;
	nameDialog.m_windowText.Format("The new name of the fit window?");
	nameDialog.m_inputString = &name;
	INT_PTR ret = nameDialog.DoModal();

	if(IDCANCEL == ret)
		return;

	// Change the name 
	m_reeval->m_window[curSel].name.Format("%s", name);

	// Update hte list
	PopulateList();
}

/** Called to remove a fit window from the list */
void CFitWindowListBox::OnRemoveFitWindow(){
	if(NULL == m_reeval)
		return;

	// make sure that there's always at least one window defined
	if(m_reeval->m_windowNum <= 1)
		return;

	int curSel = this->GetCurSel();
	if(curSel < 0 || curSel > m_reeval->MAX_N_WINDOWS)
		return;

	// Are you sure?
	int ret = MessageBox("Are you sure you want to remove this fit window?", "Remove fit window", MB_YESNO);
	if(IDNO == ret)
		return;

	// Remove the window

	// Shift down all the other windows.
	int i;
	for(i = curSel; i < m_reeval->MAX_N_WINDOWS-2; ++i){
		m_reeval->m_window[i] = m_reeval->m_window[i+1];		
	}
	m_reeval->m_window[i].Clear();
	m_reeval->m_windowNum -= 1;

	// Update the window and the list
	PopulateList();
}
