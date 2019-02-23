// SplitPakFilesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "../NovacMasterProgram.h"
#include "SplitPakFilesDlg.h"

#include "../Common/Common.h"
#include "../Common/Spectra/PakFileHandler.h"

// Include the special multi-choice file-dialog
#include "../Dialogs/FECFileDialog.h"

using namespace Dialogs;
// CSplitPakFilesDlg dialog

UINT SplitPakFiles( LPVOID pParam );

IMPLEMENT_DYNAMIC(CSplitPakFilesDlg, CPropertyPage)
CSplitPakFilesDlg::CSplitPakFilesDlg()
	: CPropertyPage(CSplitPakFilesDlg::IDD)
{
	m_pakFileNum = 0;
}

CSplitPakFilesDlg::~CSplitPakFilesDlg()
{
}

void CSplitPakFilesDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_BTN_SPLITPAKFILES, m_btnSplitFiles);
	DDX_Control(pDX, IDC_PROGRESS_BAR,			m_progressBar);
	DDX_Control(pDX, IDC_STATUSBAR,					m_statusBar);
}


BEGIN_MESSAGE_MAP(CSplitPakFilesDlg, CPropertyPage)
	ON_COMMAND(IDC_BTN_BROWSEPAKFILES,	OnBrowsePakFiles)
	ON_COMMAND(IDC_BTN_BROWSEOUTPUTDIR,	OnBrowseOutputDir)
	ON_COMMAND(IDC_BTN_SPLITPAKFILES,		OnSplitFiles)

	ON_EN_CHANGE(IDC_EDIT_OUTPUTDIRECTORY,	OnChangeOutputDir)

	ON_MESSAGE(WM_PROGRESS,							OnProgress)
	ON_MESSAGE(WM_EVAL_FAILURE,					OnFailure)
END_MESSAGE_MAP()


// CSplitPakFilesDlg message handlers
BOOL CSplitPakFilesDlg::OnInitDialog()
{
  CPropertyPage::OnInitDialog();

	//
	m_btnSplitFiles.EnableWindow(FALSE);

	// initialize the progressbar
	m_progressBar.ShowWindow(SW_HIDE);
	m_statusBar.ShowWindow(SW_HIDE);

  return TRUE;  // return TRUE unless you set the focus to a control
  // EXCEPTION: OCX Property Pages should return FALSE
}

/** Called to let the user browse for .pak files to split */
void CSplitPakFilesDlg::OnBrowsePakFiles(){
  TCHAR filter[] = _T("Pak Files|*.pak|All Files|*.*||");
  CString str;

	// The file-dialog
	Dialogs::CFECFileDialog fileDialog(TRUE, NULL, NULL, OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER, filter);

	if(fileDialog.DoModal() != IDOK)
		return;

  /** Reset the old values */
  this->m_pakFileNum = 0;
	this->m_pakFile.RemoveAll();

	/** Go through the filenames */
	POSITION pos = fileDialog.GetStartPosition();

	while(pos){
		str = fileDialog.GetNextPathName(pos);

		m_pakFile.AddTail(str);
		++m_pakFileNum;
	}

	str.Format("%d pak-files selected", m_pakFileNum);
	this->SetDlgItemText(IDC_LABEL_PAKFILES, str);

}

/** Called to let the user browse for an output directory */
void CSplitPakFilesDlg::OnBrowseOutputDir(){
	m_outputDir.Format("");
	Common common;

	// let the user browse for an output directory
	if(!common.BrowseForDirectory(m_outputDir)){
		m_outputDir.Format("");
		return;
	}

	SetDlgItemText(IDC_EDIT_OUTPUTDIRECTORY, m_outputDir);

	if(m_pakFileNum > 0)
		m_btnSplitFiles.EnableWindow(TRUE);

}

/** Called when the user has edited the output-directory edit-box */
void CSplitPakFilesDlg::OnChangeOutputDir(){
	GetDlgItemText(IDC_EDIT_OUTPUTDIRECTORY, m_outputDir);

	if(m_pakFileNum > 0)
		m_btnSplitFiles.EnableWindow(TRUE);
}

LRESULT CSplitPakFilesDlg::OnProgress(WPARAM wp, LPARAM lp){
	CString str;
	int progress = (int)wp;

	// update the progressbar
	m_progressBar.SetPos(progress);

	// update the statusbar
	double fraction = (double)progress / (double)m_pakFileNum;
	str.Format("Splitting pak-files %.0lf %% done", fraction*100.0);
	this->SetDlgItemText(IDC_STATUSBAR, str);

	return 0;
}
LRESULT CSplitPakFilesDlg::OnFailure(WPARAM wp, LPARAM lp){
	CString str;
	
	m_progressBar.SetPos(0);


	str.Format("PakFile error");
	this->SetDlgItemText(IDC_STATUSBAR, str);

	return 0;
}


/** Called when the user pressed the 'split files' button. 
		Does the actual work in this dialog. */
void CSplitPakFilesDlg::OnSplitFiles(){
	// Check the input
	if(m_pakFileNum <= 0){
		MessageBox("No pak files to split");
		return;
	}

	// get the output directory
	GetDlgItemText(IDC_EDIT_OUTPUTDIRECTORY, m_outputDir);

	// initialize the progressbar and the statusbar
	m_progressBar.ShowWindow(SW_SHOW);
	m_progressBar.SetRange(0, (short)m_pakFileNum);
	m_progressBar.SetPos(0);
	m_statusBar.ShowWindow(SW_SHOW);
	m_statusBar.SetWindowText("Splitting pak-files");

	// start a new thread to split the pakfiles
	AfxBeginThread(SplitPakFiles, this, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
}

UINT SplitPakFiles(LPVOID pParam){
	CString errorMsg;

	Dialogs::CSplitPakFilesDlg *dialog = (Dialogs::CSplitPakFilesDlg *)pParam;

	// create a new pakfilehandler
	FileHandler::CPakFileHandler *pakFileHandler = new FileHandler::CPakFileHandler();

	// loop through all .pak files in the list
	POSITION pos = dialog->m_pakFile.GetHeadPosition();
	int pakFileNum = 0;

	while(pos != NULL){
		CString &pakFile = dialog->m_pakFile.GetNext(pos); // <-- get the next .pak file in the list

		// let the pakfilehandler read the pakfile
		if(1 == pakFileHandler->ReadDownloadedFile(pakFile, false, false, &dialog->m_outputDir)){
			errorMsg.Format("Error in reading pakfile: %s", (LPCSTR)pakFile);
			MessageBox(NULL, errorMsg, "Error", MB_OK);
			dialog->PostMessage(WM_EVAL_FAILURE); // <-- tell the dialog about the crash!
			return 1;
		}

		// tell the dialog about the progress
		dialog->PostMessage(WM_PROGRESS, ++pakFileNum);
	}

	delete(pakFileHandler);

	return 0;
}

