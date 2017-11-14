#include "stdafx.h"
#include "../NovacMasterProgram.h"
#include "FluxErrorDlg.h"

using namespace Dialogs;

// CFluxErrorDlg dialog

IMPLEMENT_DYNAMIC(CFluxErrorDlg, CDialog)
CFluxErrorDlg::CFluxErrorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFluxErrorDlg::IDD, pParent)
{
}

CFluxErrorDlg::~CFluxErrorDlg()
{
}

void CFluxErrorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	// The data in the edit-boxes
	DDX_Text(pDX, IDC_EDIT_GEOMERROR,			m_geomError);
	DDX_Text(pDX, IDC_EDIT_SPECERROR,			m_specError);
	DDX_Text(pDX, IDC_EDIT_SCATTERINGERROR,		m_scatteringError);
	DDX_Text(pDX, IDC_EDIT_WINDERROR,			m_windError);
}


BEGIN_MESSAGE_MAP(CFluxErrorDlg, CDialog)
	ON_WM_KILLFOCUS()

	// Changing any of the values
	ON_EN_CHANGE(IDC_EDIT_GEOMERROR,			SaveData)
	ON_EN_CHANGE(IDC_EDIT_SPECERROR,			SaveData)
	ON_EN_CHANGE(IDC_EDIT_SCATTERINGERROR,		SaveData)
	ON_EN_CHANGE(IDC_EDIT_WINDERROR,			SaveData)
END_MESSAGE_MAP()


// CFluxErrorDlg message handlers

BOOL Dialogs::CFluxErrorDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Position the window in the lower left corner of the screen
	CRect windowRect, screenRect;
	GetWindowRect(windowRect);
	int windowheight	= windowRect.Height();
	int screenHeight	= GetSystemMetrics(SM_CYSCREEN);
	windowRect.right	= windowRect.right - windowRect.left;
	windowRect.left		= 0; 
	windowRect.bottom	= screenHeight - 50;	 
	windowRect.top		= windowRect.bottom - windowheight;
	MoveWindow(windowRect);

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CFluxErrorDlg::SaveData(){
	this->UpdateData(TRUE);
}
void Dialogs::CFluxErrorDlg::OnKillFocus(CWnd* pNewWnd)
{
	CDialog::OnKillFocus(pNewWnd);

	SaveData();
}
