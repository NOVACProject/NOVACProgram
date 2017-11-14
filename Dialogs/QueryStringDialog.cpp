// QueryStringDialog.cpp : implementation file
//

#include "stdafx.h"
#include "../NovacMasterProgram.h"
#include "QueryStringDialog.h"

using namespace Dialogs;

// CQueryStringDialog dialog

IMPLEMENT_DYNAMIC(CQueryStringDialog, CDialog)
CQueryStringDialog::CQueryStringDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CQueryStringDialog::IDD, pParent)
{
  m_windowText.Format("Query Dialog");
  m_inputString = NULL;
}

CQueryStringDialog::~CQueryStringDialog()
{
}

void CQueryStringDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_STRING_EDIT, m_editBox);
}


BEGIN_MESSAGE_MAP(CQueryStringDialog, CDialog)
END_MESSAGE_MAP()


// CQueryStringDialog message handlers

BOOL CQueryStringDialog::OnInitDialog()
{
  CDialog::OnInitDialog();

	// Set the title-bar of the window
  SetWindowText(m_windowText);

	// Make an initial guess for the string to input
	SetDlgItemText(IDC_STRING_EDIT, *m_inputString);

	// Set the focus to the input-box
	m_editBox.SetFocus();

  return FALSE;  // return TRUE unless you set the focus to a control
  // EXCEPTION: OCX Property Pages should return FALSE
}

void CQueryStringDialog::OnOK()
{
  if(this->m_inputString != NULL){
    CString tmpStr;
    this->GetDlgItemText(IDC_STRING_EDIT, tmpStr);
    m_inputString->Format("%s", tmpStr);
  }

  CDialog::OnOK();
}
