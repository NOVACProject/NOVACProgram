// SelectionDialog.cpp : implementation file
//

#include "stdafx.h"
#include "../NovacMasterProgram.h"
#include "SelectionDialog.h"

using namespace Dialogs;

// CSelectionDialog dialog

IMPLEMENT_DYNAMIC(CSelectionDialog, CDialog)
CSelectionDialog::CSelectionDialog(CWnd* pParent /*=NULL*/)
    : CDialog(CSelectionDialog::IDD, pParent)
{
    m_currentSelection = NULL;
}

CSelectionDialog::~CSelectionDialog()
{
}

void CSelectionDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_SELECTION_COMBO, m_comboBox);
}


BEGIN_MESSAGE_MAP(CSelectionDialog, CDialog)
END_MESSAGE_MAP()


// CSelectionDialog message handlers

BOOL CSelectionDialog::OnInitDialog()
{
    CDialog::OnInitDialog();

    /** Add the strings */
    for (int i = 0; i < MAX_OPTIONS; ++i) {
        if (strlen(m_option[i]) > 0)
            m_comboBox.AddString(m_option[i]);
    }
    m_comboBox.SetCurSel(0);

    /** Set the window text */
    this->SetWindowText(this->m_windowText);

    m_comboBox.SetFocus();

    return FALSE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CSelectionDialog::OnOK()
{
    if ((this->m_currentSelection != NULL) && (m_comboBox.GetCurSel() >= 0)) {
        UpdateData(TRUE);
        m_currentSelection->Format(m_option[m_comboBox.GetCurSel()]);
    }

    CDialog::OnOK();
}
