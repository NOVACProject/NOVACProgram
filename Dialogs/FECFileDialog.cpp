#include "stdafx.h"
#include "FECFileDialog.h"

#ifndef _INC_CDERR
#include <cderr.h>     // for FNERR_BUFFERTOSMALL
#endif // _INC_CDERR

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace Dialogs;
/////////////////////////////////////////////////////////////////////////////
// CFECFileDialog

IMPLEMENT_DYNAMIC(CFECFileDialog, CFileDialog)
/////////////////////////////////////////////////////////////////////////////
//
//  CFECFileDialog constructor  (public member function)
//    Initializes the class variables
//
//  Parameters :
//    See CFileDialog::CFileDialog for parameter information
//
//  Returns :
//    Nothing
//
/////////////////////////////////////////////////////////////////////////////


CFECFileDialog::CFECFileDialog(BOOL bOpenFileDialog, LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
                               DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd) :
    CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd)
{
    Files = NULL;
    Folder = NULL;
    bParsed = FALSE;
}
/////////////////////////////////////////////////////////////////////////////
//
//  CFECFileDialog destructor  (public member function)
//    Cleans up the class variables
//
//  Parameters :
//    None
//
//  Returns :
//    Nothing
//
/////////////////////////////////////////////////////////////////////////////


CFECFileDialog::~CFECFileDialog()
{
    if (Files)
    {
        delete[] Files;
        delete[] Folder;
    }
}

BEGIN_MESSAGE_MAP(CFECFileDialog, CFileDialog)
    //{{AFX_MSG_MAP(CFECFileDialog)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
//
//  CFECFileDialog::DoModal  (public member function)
//    Starts the CFileDialog and properly handles the return code
//
//  Parameters :
//    None
//
//  Returns :
//    IDOK if the user selected the OK button
//    IDCANCEL if the user selected the Cancel button or an error occured
//
/////////////////////////////////////////////////////////////////////////////

INT_PTR CFECFileDialog::DoModal()
{
    if (Files)
    {
        delete[] Files;
        Files = NULL;
        delete[] Folder;
        Folder = NULL;
    }

    auto ret = CFileDialog::DoModal();

    if (ret == IDCANCEL)
    {
        DWORD err = CommDlgExtendedError();
        if (err == FNERR_BUFFERTOOSMALL/*0x3003*/ && Files)
            ret = IDOK;
    }
    return ret;
}

/////////////////////////////////////////////////////////////////////////////
//
//  CFECFileDialog::GetNextPathName  (public member function)
//    Call this function to retrieve the next file name from the group
//    selected in the dialog box.
//
//  Parameters :
//    pos - A reference to a POSITION value returned from a previous GetNextPathName
//          or GetStartPosition function call. Will be set to NULL when the end of
//          the list has been reached.
//
//  Returns :
//    The full path of the file
//
/////////////////////////////////////////////////////////////////////////////

CString CFECFileDialog::GetNextPathName(POSITION& pos) const
{
    if (!Files)
        return CFileDialog::GetNextPathName(pos);

    ASSERT(pos);
    TCHAR* ptr = (TCHAR*)pos;

    CString ret = Folder;
    ret += _T("\\");
    ret += ptr;

    ptr += _tcslen(ptr) + 1;
    if (*ptr)
        pos = (POSITION)ptr;
    else
        pos = NULL;

    return ret;
}

/////////////////////////////////////////////////////////////////////////////
//
//  CFECFileDialog::GetStartPosition  (public member function)
//    Call this member function to retrieve the POSITION of the first file
//    in a list of multiple selected files.
//
//  Parameters :
//    None
//
//  Returns :
//    A POSITION value that is used by the GetNextPathName function
//
/////////////////////////////////////////////////////////////////////////////

POSITION CFECFileDialog::GetStartPosition()
{
    if (!Files)
        return CFileDialog::GetStartPosition();

    if (!bParsed)
    {
        CString temp = Files;
        temp.Replace(_T("\" \""), _T("\""));
        temp.Delete(0, 1);                      // remove leading quote mark
        temp.Delete(temp.GetLength() - 1, 1);   // remove trailing space

        _tcscpy(Files, temp);

        TCHAR* ptr = Files;
        while (*ptr)
        {
            if ('\"' == *ptr)
                *ptr = '\0';
            ++ptr;
        }
        bParsed = TRUE;
    }

    return (POSITION)Files;
}

/////////////////////////////////////////////////////////////////////////////
//
//  CFECFileDialog::OnFileNameChange  (protected member function)
//    If the lpstrFile and nMaxFile variables in the OPENFILENAME structure
//    are not large enough to handle the users selection, we handle the
//    selection ourselves.
//
//  Parameters :
//    None
//
//  Returns :
//    Nothing
//
//  Note :
//    Works only if the CFileDialog is created with the OFN_EXPLORER flag
//
/////////////////////////////////////////////////////////////////////////////

void CFECFileDialog::OnFileNameChange()
{
    DWORD dwReqBuffSize = _CalcRequiredBuffSize();
    if (dwReqBuffSize > GetOFN().nMaxFile)
    {
        _SetExtBuffer(dwReqBuffSize);
    }

    CFileDialog::OnFileNameChange();
}

void CFECFileDialog::_SetExtBuffer(DWORD dwReqBuffSize)
{
    try
    {
        GetOFN().nMaxFile = dwReqBuffSize;
        delete[]m_pFileBuff;
        m_pFileBuff = new TCHAR[dwReqBuffSize];
        GetOFN().lpstrFile = m_pFileBuff;
    }
    catch (CException* e)
    {
        e->ReportError();
        e->Delete();
    }
}

DWORD CFECFileDialog::_CalcRequiredBuffSize()
{
    DWORD dwRet = 0;
    IFileOpenDialog* pFileOpen = GetIFileOpenDialog();
    ASSERT(NULL != pFileOpen);

    CComPtr<IShellItemArray> pIItemArray;
    HRESULT hr = pFileOpen->GetSelectedItems(&pIItemArray);
    DWORD dwItemCount = 0;
    if (SUCCEEDED(hr))
    {
        hr = pIItemArray->GetCount(&dwItemCount);
        if (SUCCEEDED(hr))
        {
            for (DWORD dwItem = 0; dwItem < dwItemCount; dwItem++)
            {
                CComPtr<IShellItem> pItem;
                hr = pIItemArray->GetItemAt(dwItem, &pItem);
                if (SUCCEEDED(hr))
                {
                    LPWSTR pszName = NULL;
                    if (dwItem == 0)
                    {
                        // get full path and file name
                        hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszName);
                    }
                    else
                    {
                        // get file name
                        hr = pItem->GetDisplayName(SIGDN_NORMALDISPLAY, &pszName);
                    }
                    if (SUCCEEDED(hr))
                    {
                        dwRet += wcslen(pszName) + 1;
                        ::CoTaskMemFree(pszName);
                    }
                }
            }
        }
    }
    dwRet += 1; // add an extra character
    // release the IFileOpenDialog pointer
    pFileOpen->Release();
    return dwRet;
}