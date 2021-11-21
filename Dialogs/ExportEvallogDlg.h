#pragma once
#include "afxwin.h"

#include "../Common/EvaluationLogFileHandler.h"

namespace Dialogs
{

    // CExportEvallogDlg dialog

    class CExportEvallogDlg : public CPropertyPage, FileHandler::CEvaluationLogFileHandler
    {
        DECLARE_DYNAMIC(CExportEvallogDlg)

    public:
        CExportEvallogDlg();
        virtual ~CExportEvallogDlg();

        // Dialog Data
        enum { IDD = IDD_EXPORT_EVALLOG };

    protected:
        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

        DECLARE_MESSAGE_MAP()
    public:
        afx_msg void OnBrowseEvalLog();
        afx_msg void OnBrowseExportFile();
        CButton m_exportBtn;

        int m_exportFormat;
        afx_msg void OnExportLog();
    };
}