
#include "stdafx.h"
#include "../resource.h"
#include "../Dialogs/QueryStringDialog.h"
#include "../Evaluation/FitWindowFileHandler.h"
#include "FitWindowListBox.h"

namespace ReEvaluation
{

static const char* novacFitWindowFilesFilter = "Novac Fit Window Files\0*.nfw\0";

IMPLEMENT_DYNAMIC(CFitWindowListBox, CListBox)

CFitWindowListBox::CFitWindowListBox(CReEvaluator& reeval)
    : m_reeval(reeval)
{}

CFitWindowListBox::~CFitWindowListBox()
{}

BEGIN_MESSAGE_MAP(CFitWindowListBox, CListBox)
    ON_WM_CONTEXTMENU()
    ON_COMMAND(ID__INSERTFITWINDOW, OnInsertFitWindow)
    ON_COMMAND(ID__REMOVEWINDOW, OnRemoveFitWindow)

    ON_COMMAND(ID__LOADWINDOWFROMFILE, OnLoadFitWindows)
    ON_COMMAND(ID_FILE_LOADFITWINDOWS, OnLoadFitWindows)

    ON_COMMAND(ID__SAVEWINDOWTOFILE, OnSaveFitWindows)
    ON_COMMAND(ID_FILE_SAVECURRENTFITWINDOWS, OnSaveFitWindows)

    ON_COMMAND(ID__RENAMEWINDOW, OnRenameWindow)
END_MESSAGE_MAP()



// CFitWindowListBox message handlers

/** Called to populate the list */
void CFitWindowListBox::PopulateList()
{
    this->ResetContent(); // clear the list

    for (int i = 0; i < m_reeval.m_windowNum; ++i)
    {
        CString name(m_reeval.m_window[i].name.c_str());
        this->AddString(name);
    }
}

/** Called when the user presses down the left mouse button */
void CFitWindowListBox::OnLButtonDown(UINT nFlags, CPoint point)
{
    CListBox::OnLButtonDown(nFlags, point);
}

/** Called to show the context menu */
void CFitWindowListBox::OnContextMenu(CWnd* pWnd, CPoint pos)
{
    OnLButtonDown(MK_LBUTTON, pos); // make the current menu item marked

    CMenu menu;
    VERIFY(menu.LoadMenu(IDR_FITWINDOWLIST_MENU));
    CMenu* pPopup = menu.GetSubMenu(0);
    ASSERT(pPopup != nullptr);

    // There has to be at least one fit window defined at all times
    //	if there are to few, don't allow the user to remove any
    if (m_reeval.m_windowNum <= 1)
    {
        pPopup->EnableMenuItem(ID__REMOVEWINDOW, MF_DISABLED | MF_GRAYED);
    }

    // If the list of fit windows is full, don't let the user
    //	add any more fit windows
    if (m_reeval.m_windowNum == CReEvaluator::MAX_N_WINDOWS - 1)
    {
        pPopup->EnableMenuItem(ID__INSERTFITWINDOW, MF_DISABLED | MF_GRAYED);
        pPopup->EnableMenuItem(ID__LOADWINDOWFROMFILE, MF_DISABLED | MF_GRAYED);
    }

    // show the popup menu
    pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pos.x, pos.y, this);

}

/** Called to insert a fit window into the list */
void CFitWindowListBox::OnInsertFitWindow()
{
    CString name;

    // Make sure that there's enough space to store one more window 
    if (m_reeval.m_windowNum == CReEvaluator::MAX_N_WINDOWS - 1)
        return;

    // Ask the user for the name of the window
    Dialogs::CQueryStringDialog nameDialog;
    nameDialog.m_windowText.Format("The name of the fit window?");
    nameDialog.m_inputString = &name;
    INT_PTR ret = nameDialog.DoModal();

    if (IDCANCEL == ret)
        return;

    // insert an empty fit window.
    m_reeval.m_window[m_reeval.m_windowNum].name = std::string((LPCTSTR)name);
    m_reeval.m_windowNum += 1;

    // Update the list
    PopulateList();

    // Select the fit window
    SetCurSel(m_reeval.m_windowNum - 1);
}

/** Called to load a set of fit window from a file */
void CFitWindowListBox::OnLoadFitWindows()
{
    if (m_reeval.m_windowNum == m_reeval.MAX_N_WINDOWS - 1)
    {
        return;
    }

    // Ask for a file to read from
    CString fileName = "";

    // let the user browse for an evaluation log file and if one is selected, read it
    if (Common::BrowseForFile(novacFitWindowFilesFilter, fileName))
    {
        FileHandler::CFitWindowFileHandler fitWindowReader;
        std::vector<novac::CFitWindow> windowsInFile = fitWindowReader.ReadFitWindowFile(fileName);

        // Set the windows of the evaluator
        m_reeval.m_windowNum = windowsInFile.size();
        for (size_t ii = 0; ii < windowsInFile.size(); ++ii)
        {
            m_reeval.m_window[ii] = windowsInFile[ii];
        }
    }

    // Update the window
    this->PopulateList();
    this->SetCurSel(0);

    // Need to tell the parent window to update
    CWnd* pWnd = GetParent();
    if (nullptr != pWnd)
    {
        pWnd->SendMessage(WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), LBN_SELCHANGE), (LPARAM)m_hWnd);
    }
}

/** Called to save a set of fit windows to file */
void CFitWindowListBox::OnSaveFitWindows()
{
    FileHandler::CFitWindowFileHandler fitWindowWriter;

    if (0 == m_reeval.m_windowNum)
        return;

    // Ask for a file to save the data to
    CString fileName = "";

    // let the user browse for an evaluation log file and if one is selected, read it
    if (Common::BrowseForFile_SaveAs(novacFitWindowFilesFilter, fileName))
    {
        // if there's not a .nfw-ending on the file, append it!
        if (!Equals(".nfw", fileName.Right(4)))
        {
            fileName.AppendFormat(".nfw");
        }

        bool overWrite = true;	// the first window to be saved overwrites the file, the others appends the file

        for (int i = 0; i < m_reeval.m_windowNum; ++i)
        {
            fitWindowWriter.WriteFitWindow(m_reeval.m_window[i], fileName, overWrite);
            overWrite = false;
        }
    }
}

/** Called to rename a fit window */
void CFitWindowListBox::OnRenameWindow()
{
    CString name;

    int curSel = GetCurSel();
    if (curSel < 0 || curSel > m_reeval.MAX_N_WINDOWS)
        return;

    // Let the initial guess for the name be the old name of the window
    name = CString(m_reeval.m_window[curSel].name.c_str());

    // Ask the user for the name of the window
    Dialogs::CQueryStringDialog nameDialog;
    nameDialog.m_windowText.Format("The new name of the fit window?");
    nameDialog.m_inputString = &name;
    INT_PTR ret = nameDialog.DoModal();

    if (IDCANCEL == ret)
        return;

    // Change the name 
    m_reeval.m_window[curSel].name = std::string((LPCTSTR)name);

    // Update hte list
    PopulateList();
}

/** Called to remove a fit window from the list */
void CFitWindowListBox::OnRemoveFitWindow()
{
    // make sure that there's always at least one window defined
    if (m_reeval.m_windowNum <= 1)
        return;

    int curSel = this->GetCurSel();
    if (curSel < 0 || curSel > m_reeval.MAX_N_WINDOWS)
        return;

    // Are you sure?
    int ret = MessageBox("Are you sure you want to remove this fit window?", "Remove fit window", MB_YESNO);
    if (IDNO == ret)
        return;

    // Remove the window

    // Shift down all the other windows.
    int i;
    for (i = curSel; i < m_reeval.MAX_N_WINDOWS - 2; ++i)
    {
        m_reeval.m_window[i] = m_reeval.m_window[i + 1];
    }
    m_reeval.m_window[i].Clear();
    m_reeval.m_windowNum -= 1;

    // Update the window and the list
    PopulateList();
}

} // namespace ReEvaluation
