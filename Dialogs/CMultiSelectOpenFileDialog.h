#pragma once

namespace Dialogs {

    // Code from: http://www.codeproject.com/dialog/pja_multiselect.asp
    //	made by: PJ Arends
    class CMultiSelectOpenFileDialog : public CFileDialog
    {
        DECLARE_DYNAMIC(CMultiSelectOpenFileDialog)

    private:
        LPTSTR m_pFileBuff = new TCHAR[0];;
        DWORD _CalcRequiredBuffSize();
        void _SetExtBuffer(DWORD dwReqBuffSize);

    public:
        CMultiSelectOpenFileDialog(BOOL bOpenFileDialog, // TRUE for FileOpen, FALSE for FileSaveAs
                LPCTSTR lpszDefExt = NULL,
                LPCTSTR lpszFileName = NULL,
                DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
                LPCTSTR lpszFilter = NULL,
                CWnd* pParentWnd = NULL);

        virtual ~CMultiSelectOpenFileDialog();// { if (Files) { delete[] Files; delete[] Folder; } }

        virtual INT_PTR DoModal() override;
        CString GetNextPathName(POSITION& pos) const;
        POSITION GetStartPosition();

    protected:
        BOOL bParsed;
        TCHAR* Folder;
        TCHAR* Files;
        virtual void OnFileNameChange();
        DECLARE_MESSAGE_MAP()
    };
}

