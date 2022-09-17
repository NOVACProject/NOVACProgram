#include "stdafx.h"
#include "..\NovacMasterProgram.h"
#include "MergeEvalLogDlg.h"
#include "..\File\FileMerger.h"

#include "../Common/Common.h"

// Include the special multi-choice file-dialog
#include "../Dialogs/FECFileDialog.h"

using namespace Dialogs;

// CMergeEvalLogDlg dialog

IMPLEMENT_DYNAMIC(CMergeEvalLogDlg, CDialog)
CMergeEvalLogDlg::CMergeEvalLogDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CMergeEvalLogDlg::IDD, pParent)
{
}

CMergeEvalLogDlg::~CMergeEvalLogDlg()
{
}

void CMergeEvalLogDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LABEL_EVALLOGS, m_inputListLabel);
    DDX_Control(pDX, IDC_EDIT_OUTPUTFILE_EVALLOG, m_outputFileEdit);
}


BEGIN_MESSAGE_MAP(CMergeEvalLogDlg, CDialog)
    ON_BN_CLICKED(IDC_BTN_BROWSEEVALLOGS, OnBrowseEvallogs)
    ON_BN_CLICKED(IDC_BTN_BROWSEOUTPUTFILE_EVALLOG, OnBrowseOutputFileEvallog)
    ON_BN_CLICKED(IDOK, OnBnClickedMerge)
    ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
END_MESSAGE_MAP()


// CMergeEvalLogDlg message handlers

void CMergeEvalLogDlg::OnBrowseEvallogs()
{
    TCHAR filter[512] = "Evaluation logs (.txt)|*.txt|All files|*.*||";
    CString userMessage;

    // The file-dialog
    Dialogs::CMultiSelectOpenFileDialog fileDialog(TRUE, NULL, NULL, OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER, filter);

    if (fileDialog.DoModal() != IDOK)
        return;

    /** Reset the old values */
    this->m_evalLogFile.RemoveAll();

    /** Go through the filenames */
    POSITION pos = fileDialog.GetStartPosition();

    while (pos)
    {
        CString str = fileDialog.GetNextPathName(pos);
        m_evalLogFile.AddTail(str);
    }

    // Update the dialog
    userMessage.Format("%d EvalLog-files selected", m_evalLogFile.GetCount());
    this->SetDlgItemText(IDC_LABEL_EVALLOGS, userMessage);
}

void CMergeEvalLogDlg::OnBrowseOutputFileEvallog()
{
    TCHAR filter[512];
    CString fileName;
    Common common;

    // Make a filter for files to look for
    int n = _stprintf(filter, "Evaluation logs\0");
    n += _stprintf(filter + n + 1, "*.txt;\0");
    filter[n + 2] = 0;

    // let the user browse for an output directory
    if (!common.BrowseForFile_SaveAs(filter, fileName)) {
        return;
    }
    SetDlgItemText(IDC_EDIT_OUTPUTFILE_EVALLOG, fileName);
}

void CMergeEvalLogDlg::OnBnClickedMerge()
{
    FileMerger* merger = new FileMerger();

    // Get the output file
    CString outputFile;
    GetDlgItemText(IDC_EDIT_OUTPUTFILE_EVALLOG, outputFile);
    merger->SetOutputFile(outputFile);

    // Specify the input files
    merger->SetFilesToMerge(this->m_evalLogFile);

    int ret = merger->MergeFiles();
    if (ret != 0)
    {
        if (ret == 1)
        {
            MessageBox("No files selected for merging.", "Error", MB_OK);
        }
        else if (ret == 2)
        {
            MessageBox("Invalid output file.", "Error", MB_OK);
        }
        else if (ret == 3)
        {
            MessageBox("Cannot open output file for writing.", "Error", MB_OK);
        }

        delete merger;

        return;
    }
    else {
        MessageBox("Done!", "Info", MB_OK);
    }

    delete merger;


    OnOK();
}

void CMergeEvalLogDlg::OnBnClickedCancel()
{
    OnCancel();
}
