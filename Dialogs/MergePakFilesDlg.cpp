#include "stdafx.h"
#include "../NovacMasterProgram.h"
#include "MergePakFilesDlg.h"

#include "../Common/Common.h"
#include "../Common/Spectra/PakFileHandler.h"

// Include the special multi-choice file-dialog
#include "../Dialogs/FECFileDialog.h"

using namespace Dialogs;
// CMergePakFilesDlg dialog

UINT MergePakFiles_Concatenate( LPVOID pParam );
UINT MergePakFiles_Combine( LPVOID pParam );

IMPLEMENT_DYNAMIC(CMergePakFilesDlg, CPropertyPage)
CMergePakFilesDlg::CMergePakFilesDlg()
	: CPropertyPage(CMergePakFilesDlg::IDD)
{
	m_pakFileNum	= 0;
	m_sortOrder		= 0;
	m_mergeOption	= 0;
	m_mergeNum		= 0;
}

CMergePakFilesDlg::~CMergePakFilesDlg()
{
}

void CMergePakFilesDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_BTN_MERGEPAKFILES,		m_btnMergeFiles);
	DDX_Control(pDX, IDC_PROGRESS_BAR,				m_progressBar);
	DDX_Control(pDX, IDC_STATUSBAR,						m_statusBar);

	DDX_Radio(pDX, IDC_RADIO_SORTBYNAME_ASC,	m_sortOrder);

	DDX_Radio(pDX, IDC_RADIO_CONCATENATE,			m_mergeOption);

	DDX_Text(pDX,		IDC_EDIT_MERGENUM,				m_mergeNum);

}


BEGIN_MESSAGE_MAP(CMergePakFilesDlg, CPropertyPage)
	ON_COMMAND(IDC_BTN_BROWSEPAKFILES,		OnBrowsePakFiles)
	ON_COMMAND(IDC_BTN_BROWSEOUTPUTFILE,	OnBrowseOutputFile)
	ON_COMMAND(IDC_BTN_MERGEPAKFILES,			OnMergeFiles)

	ON_EN_CHANGE(IDC_EDIT_OUTPUTFILE,			OnChangeOutputFile)

	ON_MESSAGE(WM_PROGRESS,							OnProgress)
	ON_MESSAGE(WM_EVAL_FAILURE,					OnFailure)
END_MESSAGE_MAP()


// CMergePakFilesDlg message handlers
BOOL CMergePakFilesDlg::OnInitDialog()
{
  CPropertyPage::OnInitDialog();

	//
	m_btnMergeFiles.EnableWindow(FALSE);

	// initialize the progressbar
	m_progressBar.ShowWindow(SW_HIDE);
	m_statusBar.ShowWindow(SW_HIDE);

  return TRUE;  // return TRUE unless you set the focus to a control
  // EXCEPTION: OCX Property Pages should return FALSE
}

/** Called to let the user browse for .pak files to merge */
void CMergePakFilesDlg::OnBrowsePakFiles(){
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

		m_pakFile.AddTail(CString(str));
		++m_pakFileNum;
	}

	// Update the dialog
	str.Format("%d pak-files selected", m_pakFileNum);
	this->SetDlgItemText(IDC_LABEL_PAKFILES, str);
}

/** Called to let the user browse for an output file */
void CMergePakFilesDlg::OnBrowseOutputFile(){
  TCHAR filter[512];
	CString fileName;
	Common common;

	// Make a filter for files to look for
  int n = _stprintf(filter, "Pak Files\0");
  n += _stprintf(filter + n + 1, "*.pak;\0");
  filter[n + 2] = 0;


	// let the user browse for an output directory
	if(!common.BrowseForFile_SaveAs(filter, fileName)){
		return;
	}
	m_outputFile.Format("%s", (LPCSTR)fileName);

	SetDlgItemText(IDC_EDIT_OUTPUTFILE, m_outputFile);

	if(m_pakFileNum > 0)
		m_btnMergeFiles.EnableWindow(TRUE);
}

/** Called when the user has edited the output-file edit-box */
void CMergePakFilesDlg::OnChangeOutputFile(){
	GetDlgItemText(IDC_EDIT_OUTPUTFILE, m_outputFile);

	if(m_pakFileNum > 0)
		m_btnMergeFiles.EnableWindow(TRUE);
}

LRESULT CMergePakFilesDlg::OnProgress(WPARAM wp, LPARAM lp){
	CString str;
	int progress = (int)wp;

	// update the progressbar
	m_progressBar.SetPos(progress);

	// update the statusbar
	double fraction = (double)progress / (double)m_pakFileNum;
	str.Format("Merging pak-files %.0lf %% done", fraction*100.0);
	this->SetDlgItemText(IDC_STATUSBAR, str);

	return 0;
}
LRESULT CMergePakFilesDlg::OnFailure(WPARAM wp, LPARAM lp){
	CString str;
	
	m_progressBar.SetPos(0);

	str.Format("PakFile error");
	this->SetDlgItemText(IDC_STATUSBAR, str);

	return 0;
}


/** Called when the user pressed the 'merge files' button. 
		Does the actual work in this dialog. */
void CMergePakFilesDlg::OnMergeFiles(){
	CString message;

	// Update the contents of the dialog
	UpdateData(TRUE);

	// Check the input
	if(m_pakFileNum <= 0){
		MessageBox("No pak files to merge");
		return;
	}

	// Check for errors in the input
	if(m_mergeOption == 1 && m_mergeNum > MAX_SCANS_COMBINATIONS){
		message.Format("Cannot merge more than %d scans together. Please check settings and try again", MAX_SCANS_COMBINATIONS);
		MessageBox(message, "Error in settings", MB_OK);
		return;
	}

	// get the output file-name
	GetDlgItemText(IDC_EDIT_OUTPUTFILE, m_outputFile);

	// initialize the progressbar and the statusbar
	m_progressBar.ShowWindow(SW_SHOW);
	m_progressBar.SetRange(0, (short)m_pakFileNum);
	m_progressBar.SetPos(0);
	m_statusBar.ShowWindow(SW_SHOW);
	m_statusBar.SetWindowText("Sorting pak-files");

	// Sort the files
	if(m_sortOrder == 0)
		Common::Sort(m_pakFile, true, true);
	else
		Common::Sort(m_pakFile, true, false);


	m_statusBar.ShowWindow(SW_SHOW);
	m_statusBar.SetWindowText("Merging pak-files");

	// start a thread to merge the pakfiles
	if(m_mergeOption == 0)
		AfxBeginThread(MergePakFiles_Concatenate, this, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
	else
		AfxBeginThread(MergePakFiles_Combine, this, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
}

UINT MergePakFiles_Concatenate(LPVOID pParam){
	CString errorMsg;
	CSpectrum curSpec;
	static const int BUFFER_SIZE = 16384;
	char dataBuffer[BUFFER_SIZE];

	// Reading of the spectrum header
	const int HEADER_BUF_SIZE = 16384;
	char *spectrumHeader = (char*)calloc(HEADER_BUF_SIZE, sizeof(char)); // <-- the spectrum header, in binary format
	int specHeaderSize = 0;

	Dialogs::CMergePakFilesDlg *dialog = (Dialogs::CMergePakFilesDlg *)pParam;

	// Get the file-name to write to
	CString outputFile = CString(dialog->m_outputFile);

	// create a spectrum reader/writer
	SpectrumIO::CSpectrumIO *specHandler = new SpectrumIO::CSpectrumIO();

	// Try to open the output-file
	FILE *uf	= fopen(outputFile, "wb");
	if(uf == NULL){
		MessageBox(NULL, "Could not open output-file for writing. Please check file-name and try again", "Error", MB_OK);
		return 1;
	}

	// loop through all .pak files in the list
	POSITION pos = dialog->m_pakFile.GetHeadPosition();
	int pakFileNum = 0;

	while(pos != NULL){
		CString &pakFile = dialog->m_pakFile.GetNext(pos); // <-- get the next .pak file in the list

		int spectrumNum = 0; // <-- remember the spectrum we're at

		// Loop through all spectra in the file
		FILE *f		= fopen(pakFile, "rb");

		if(f != NULL){
			size_t ret = 1;
			while(ret > 0){
				ret = fread(dataBuffer, sizeof(char), BUFFER_SIZE, f);
				fwrite(dataBuffer, sizeof(char), ret, uf);


				//// Read the next spectrum in the file
				//RETURN_CODE ret = specHandler->ReadNextSpectrum(f, curSpec, specHeaderSize, spectrumHeader, HEADER_BUF_SIZE);
				//if(ret == FAIL){
				//	// 1. Check if this is the end of the file
				//	if(specHandler->m_lastError == SpectrumIO::CSpectrumIO::ERROR_EOF || specHandler->m_lastError == SpectrumIO::CSpectrumIO::ERROR_COULD_NOT_OPEN_FILE || specHandler->m_lastError == SpectrumIO::CSpectrumIO::ERROR_SPECTRUM_NOT_FOUND){
				//		break;
				//	}

				//	// 2. Ignore this spectrum and try to find the next one in the file
				//	++spectrumNum;
				//	if(SUCCESS != specHandler->FindSpectrumNumber(f, spectrumNum))
				//		break;
				//}

				//// Save the spectrum to file
				//if(specHeaderSize > 0){
				//	specHandler->AddSpectrumToFile(outputFile, curSpec, spectrumHeader, specHeaderSize);
				//}else{
				//	specHandler->AddSpectrumToFile(outputFile, curSpec);
				//}
			}
			fclose(f);
		}

		// tell the dialog about the progress
		dialog->PostMessage(WM_PROGRESS, ++pakFileNum);
	}

	fclose(uf);

	delete(specHandler);

	return 0;
}

UINT MergePakFiles_Combine(LPVOID pParam){
	CString errorMsg;
	CSpectrum curSpec, tmpSpec;
	static const int BUFFER_SIZE = 16384;
	FILE *f[CMergePakFilesDlg::MAX_SCANS_COMBINATIONS];
	int		isFinished[CMergePakFilesDlg::MAX_SCANS_COMBINATIONS]; // <-- this equals 1 if we have read all spectra in the current file
	int		nFinished = 0;

	int fileIndex; // <-- iterator for the files

	Dialogs::CMergePakFilesDlg *dialog = (Dialogs::CMergePakFilesDlg *)pParam;

	// Get the file-name to write to
	CString outputFile = CString(dialog->m_outputFile);

	// create a spectrum reader/writer
	SpectrumIO::CSpectrumIO *specHandler = new SpectrumIO::CSpectrumIO();

	// Try to open the output-file
	FILE *uf	= fopen(outputFile, "wb");
	if(uf == NULL){
		MessageBox(NULL, "Could not open output-file for writing. Please check file-name and try again", "Error", MB_OK);
		return 1;
	}

	// loop through all .pak files in the list, m_mergeNum at a time...
	POSITION pos = dialog->m_pakFile.GetHeadPosition();
	int pakFileNum = 0;

	while(pos != NULL){

		// ----- First open all the .pak-files we want to read from -----
		for(fileIndex = 0; fileIndex < dialog->m_mergeNum; ++fileIndex){
			if(pos == NULL){
				--dialog->m_mergeNum; // <-- we're running out of files to merge
				--fileIndex;
				continue;
			}

			CString &pakFile	= dialog->m_pakFile.GetNext(pos); // <-- get the next .pak file in the list

			// Open the current .pak-file
			f[fileIndex]			= fopen(pakFile, "rb");

			// Check
			if(f[fileIndex] == NULL){
				MessageBox(NULL, "Could not open .pak-file for writing", "Error", MB_OK);
				fclose(uf);
				return 1;
			}
		}

		// ----- Now loop through all the spectra in the .pak-files -----
		memset(isFinished, 0, CMergePakFilesDlg::MAX_SCANS_COMBINATIONS * sizeof(int));
		nFinished = 0;
		while(nFinished < dialog->m_mergeNum){
			curSpec.Clear();
			for(fileIndex = 0; fileIndex < dialog->m_mergeNum; ++fileIndex){
				// check if we have read all the spectra in this file
				if(isFinished[fileIndex]){
					continue;
				}

				// Read the next spectrum in each file
				RETURN_CODE ret = specHandler->ReadNextSpectrum(f[fileIndex], tmpSpec);
				if(ret == FAIL){
					// Ignore this spectrum and try to find the spectrum in the next file
					isFinished[fileIndex] = 1;
					++nFinished;
					continue;
				}

				// Add the spectra
				if(fileIndex == 0)
					curSpec = tmpSpec;
				else
					curSpec.Add(tmpSpec);
			}

			// -------- Save the spectrum to file ----------
			if(curSpec.m_length > 0)
				specHandler->AddSpectrumToFile(outputFile, curSpec);
		}

		// ---- Close all the files ----
		for(fileIndex = 0; fileIndex < dialog->m_mergeNum; ++fileIndex){
			fclose(f[fileIndex]);
		}

		// tell the dialog about the progress
		pakFileNum += dialog->m_mergeNum;
		dialog->PostMessage(WM_PROGRESS, pakFileNum);
	}

	fclose(uf);

	delete(specHandler);

	return 0;
}
