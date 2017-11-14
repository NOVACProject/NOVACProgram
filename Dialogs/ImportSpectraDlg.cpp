#include "stdafx.h"
#include "../NovacMasterProgram.h"
#include "ImportSpectraDlg.h"
#include "../Common/Common.h"
#include "../Common/Spectrumformat/StdFile.h"
#include "../Common/Spectra/SpectrumIO.h"

#include <CDerr.h>

// Include the special multi-choice file-dialog
#include "../Dialogs/FECFileDialog.h"

// CImportSpectraDlg dialog

using namespace Dialogs;
using namespace SpectrumIO;

int nSpectraDone;

UINT ImportSpectraToOneFile( LPVOID pParam );
UINT ImportScanDOASSpectra( LPVOID pParam );
int	 ImportScanDOASSpectraInDirectory(const CArray<CString *, CString *>	&spectrumFiles, int nSpecFiles, Dialogs::CImportSpectraDlg *wnd);

int	 compareCString(const void *str1, const void *str2);
// finding the indices of the first spectra
bool FindFirst(const CArray<CString *, CString *>	&spectrumFiles, int nSpecFiles, unsigned int &darkIndex, unsigned int &skyIndex, unsigned int &specIndex);

// reading spectra
bool ReadDarkSpectrum(unsigned int darkIndex, CSpectrum &spec, const CArray<CString *, CString *>	&spectrumFiles, int nSpecFiles);
bool ReadSkySpectrum(unsigned int skyIndex, CSpectrum &spec, const CArray<CString *, CString *>	&spectrumFiles, int nSpecFiles);
bool ReadSpectrum(unsigned int index, CSpectrum &spec, const CArray<CString *, CString *>	&spectrumFiles, int nSpecFiles);
int ReadSpectrum(unsigned int index, const CString &prefix, CSpectrum &spec, const CArray<CString *, CString *>	&spectrumFiles, int nSpecFiles, int startAt = 0);


IMPLEMENT_DYNAMIC(CImportSpectraDlg, CPropertyPage)
CImportSpectraDlg::CImportSpectraDlg()
	: CPropertyPage(CImportSpectraDlg::IDD)
{
	m_nSpecFiles = 0;
	m_multiFile = 1;
	m_specPerScan = 51;
	m_outputDir.Format("");
	m_nScanAngles = 0;

	m_interlacedSpectra = 0;
	m_channel = 0;
	m_averagedSpectra		= 0;

	m_importThread = NULL;
	m_running			 = false;

	m_doChangeNumExposures = 0;
	m_userGivenNumExposures = 15;

	m_useUserScanAngles = 1;
	m_scanAngleFormat = 1;
}

CImportSpectraDlg::~CImportSpectraDlg()
{
	for(int i = 0; i < m_nSpecFiles; ++i){
		CString *str = m_spectrumFiles.GetAt(i);
		delete str;
	}
}

void CImportSpectraDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	DDX_Radio(pDX,		IDC_RADIO1,							m_multiFile);
	DDX_Radio(pDX,		IDC_RADIO3,							m_scanAngleFormat);
	DDX_Text(pDX,			IDC_EDIT_SPECPERSCAN,		m_specPerScan);
	DDX_Control(pDX,	IDC_IMPORT_SPECTRA_BTN, m_importBtn);
  DDX_Control(pDX,	IDC_IMPORT_PROGRESSBAR, m_progressBar);
	DDX_Control(pDX,	IDC_STATUSLABEL,				m_statusLabel);
	DDX_Check(pDX,		IDC_CHECK_SCANANGLES,		m_useUserScanAngles);

	// If the spectra are interlaced or not
	DDX_Check(pDX,		IDC_CHECK_INTERLACED_SPECTRA,	m_interlacedSpectra);
	DDX_Text(pDX,			IDC_EDIT_CHANNEL,				m_channel);

	// If we should change the serial-number
	DDX_Text(pDX,			IDC_EDIT_SERIAL,				m_userGivenSerial);

	// If we should change the number of co-added exposures
	DDX_Text(pDX,			IDC_EDIT_EXPOSURENUM,		m_userGivenNumExposures);
	DDX_Check(pDX,		IDC_CHANGE_EXPOSURENUM,	m_doChangeNumExposures);

	// If the spectra are averaged or not
	DDX_Check(pDX,		IDC_AVERAGED_SPECTRA,		m_averagedSpectra);
}


BEGIN_MESSAGE_MAP(CImportSpectraDlg, CPropertyPage)
	ON_BN_CLICKED(IDC_BROWSE_SPECTRA_BTN,			BrowseForSpectra)
	ON_BN_CLICKED(IDC_BROWSE_DIRECTORIES_BTN,	BrowseForSpectra_Directories)
	ON_BN_CLICKED(IDC_BROWSE_OUTPUTDIR,				BrowseForOutputDirectory)
	ON_BN_CLICKED(IDC_IMPORT_SPECTRA_BTN,			Import)
	ON_BN_CLICKED(IDC_BUTTON_TEST,						TestFn)

	ON_EN_KILLFOCUS(IDC_EDIT_OUTPUTDIR,				SaveData)
	ON_EN_CHANGE(IDC_EDIT_OUTPUTDIR,					SaveData)

	ON_EN_CHANGE(IDC_EDIT_SCANANGLES,					SaveScanAngles)

	ON_MESSAGE(WM_PROGRESS,										OnProgress)
	ON_MESSAGE(WM_DONE,												OnDone)
END_MESSAGE_MAP()


// CImportSpectraDlg message handlers


/** Browse for individual spectrum files to import */
void CImportSpectraDlg::BrowseForSpectra(){
	CString str, message;
  TCHAR filter[] = "Spectrum files | *.std";

	// The file-dialog
	Dialogs::CFECFileDialog fileDialog(TRUE, NULL, NULL, OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER, filter);

	if(fileDialog.DoModal() != IDOK)
		return;

  /** Reset the old values */
	m_spectrumFiles.RemoveAll();
	m_spectrumFiles.SetSize(max(m_spectrumFiles.GetCount(), 1000));
	m_nSpecFiles = 0;

	/** Go through the filenames */
	POSITION pos = fileDialog.GetStartPosition();

	while(pos){
		str = fileDialog.GetNextPathName(pos);

		m_spectrumFiles.SetAtGrow(m_nSpecFiles++, new CString(str));
	}

	message.Format("%d Files selected.", m_nSpecFiles);
	SetDlgItemText(IDC_STATIC_SPECTRA, message);

	if(strlen(m_outputDir) > 0){
		m_importBtn.EnableWindow(TRUE);
	}
}

/** Browse for entire directories containing files to import */
void CImportSpectraDlg::BrowseForSpectra_Directories(){
	Common common;
	CString folderName, message;
	bool includeSubDir = false;

	// Let the user select a directory
	if(!common.BrowseForDirectory(folderName)){
		return;
	}

	// Ask if we should go into sub-directories also
	if(IDYES == MessageBox("Include spectrum files in sub-directories also?", "Recurse?", MB_YESNO | MB_ICONQUESTION)){
		includeSubDir = true;
	}

	// --------------- BROWSE FOR AND INCLUDE ALL SPECTRUM FILES --------------
  /** Reset the old values */
	m_spectrumFiles.RemoveAll();
	m_spectrumFiles.SetSize(max(m_spectrumFiles.GetCount(), 1000));
	m_nSpecFiles = 0;

	// Tell the user what we are doing
	SetDlgItemText(IDC_STATIC_SPECTRA, "Counting files. Please wait...");

	// Find all files
	SearchForSpectrumFiles(folderName, includeSubDir);

	message.Format("%d Files selected.", m_nSpecFiles);
	SetDlgItemText(IDC_STATIC_SPECTRA, message);

	if(strlen(m_outputDir) > 0){
		m_importBtn.EnableWindow(TRUE);
	}
}

/** Recursively looks through the given directory for .pak-files
		Will go into sub-directories (recursively) only if 'includeSubDir' is true.
		The found spectra will be inserted into the list 'm_spectrumFiles' */
void	CImportSpectraDlg::SearchForSpectrumFiles(const CString &dir, bool includeSubDir){
  WIN32_FIND_DATA FindFileData;
  char fileToFind[MAX_PATH];

	/** Go through the filenames */
	if(includeSubDir)
		sprintf(fileToFind, "%s\\*", dir);
	else
		sprintf(fileToFind, "%s\\*.std", dir);

	// Search for files
  HANDLE hFile = FindFirstFile(fileToFind, &FindFileData);

	if(hFile == INVALID_HANDLE_VALUE){
		return; // no files found
	}

	do{
    CString fileName, fullFileName;
    fullFileName.Format("%s\\%s", dir, FindFileData.cFileName);
		fileName.Format("%s", FindFileData.cFileName);

		// don't include the current and the parent directories
		if(fileName.GetLength() == 2 && Equals(fileName, ".."))
			continue;
		if(fileName.GetLength() == 1 && Equals(fileName, "."))
			continue;

		// 1. Check what we found
		if(Equals(fileName.Right(4), ".std")){
			// This is a .std-file. Insert it into the list of files...
			m_spectrumFiles.SetAtGrow(m_nSpecFiles++, new CString(fullFileName));
    }else if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
			// This is a directory. If we are to search sub-directories then go into the directory
			if(includeSubDir){
				SearchForSpectrumFiles(fullFileName, includeSubDir);	
			}
		}

	}while(0 != FindNextFile(hFile, &FindFileData));

  FindClose(hFile);
}

/** Browse for the pak-file to save as */
void CImportSpectraDlg::BrowseForOutputDirectory(){
	Common common;
	CString folderName;

	if(!common.BrowseForDirectory(folderName)){
		return;
	}
	m_outputDir.Format(folderName);
	SetDlgItemText(IDC_EDIT_OUTPUTDIR, folderName);

	if(m_nSpecFiles > 0){
		m_importBtn.EnableWindow(TRUE);
	}
}

/** Starts the importing of files */
void CImportSpectraDlg::Import(){
	UpdateData(TRUE); // <-- Save the data in the dialog
	CString *str = NULL;

	// 1. Check if the user wants to cancel the currently running importing...
	if(m_running){
		m_running = false;
		Sleep(1000);
		SetDlgItemText(IDC_IMPORT_SPECTRA_BTN, "Import");
		return;
	}

	if(m_spectrumFiles.GetCount() <= 0 || m_nSpecFiles <= 0){
		MessageBox("No spectrum files to import", "Error in importing", MB_OK);
		return;
	}

	// Show the status label
	m_statusLabel.ShowWindow(SW_SHOW);
	m_statusLabel.SetWindowText("");

	/** If the user says that the files are generated by ScanDOAS, 
			check if this is reasonable. */
	if((1 == m_multiFile) && !IsScanDOASFiles()){
		int ret = MessageBox("It seems like the spectrum files are not generated by ScanDOAS. Proceed and save spectra in one file instead?", "Error", MB_YESNO);
		if(IDNO == ret)
			return;
		m_multiFile = 0;
	}

	if((1 == m_multiFile) && m_useUserScanAngles){
		if(m_nScanAngles > m_specPerScan){
			MessageBox("There are more scan angles defined than there are spectra in one scan. Please check settings and try again");
			return;
		}
		if(m_nScanAngles < m_specPerScan){
			MessageBox("There are more spectra in one scan than there are scan angles defined. Please check settings and try again");
			return;
		}
	}

	// Initialize the progresbar
	m_progressBar.ShowWindow(SW_SHOW);
	m_progressBar.SetRange(0, m_nSpecFiles);
	m_progressBar.SetPos(0);

	// Start by sorting the files...
	m_statusLabel.SetWindowText("Sorting filenames...");
	if(SortSpectrumFiles()){
		MessageBox("Sorting failed");
		return;
	}
	m_statusLabel.SetWindowText("Done sorting filenames");

	// Start the conversion thread
	if(m_multiFile == 0){
		m_statusLabel.SetWindowText("Importing spectra");
		m_importThread = AfxBeginThread(ImportSpectraToOneFile, this, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
		this->SetDlgItemText(IDC_IMPORT_SPECTRA_BTN, "Cancel");
		m_running = true;
	}
	if(m_multiFile == 1){
		m_statusLabel.SetWindowText("Importing ScanDOAS spectra");
		m_importThread = AfxBeginThread(ImportScanDOASSpectra, this, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
		this->SetDlgItemText(IDC_IMPORT_SPECTRA_BTN, "Cancel");
		m_running = true;
	}
}

BOOL Dialogs::CImportSpectraDlg::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	m_importBtn.EnableWindow(FALSE);

	m_progressBar.ShowWindow(SW_HIDE);

	m_statusLabel.ShowWindow(SW_HIDE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CImportSpectraDlg::SaveData(){
	// Get the output directory
	GetDlgItemText(IDC_EDIT_OUTPUTDIR, m_outputDir);

	if((strlen(m_outputDir) > 0) && (m_nSpecFiles > 0)){
		m_importBtn.EnableWindow(TRUE);
	}
}

void CImportSpectraDlg::SaveScanAngles(){
	CString str;
  char buffer[16384];
	float tmpFlt;

	// Get the text in the edit-box
	GetDlgItemText(IDC_EDIT_SCANANGLES, str);
	sprintf(buffer, "%s", str);
	
  // reset the selection list
	m_nScanAngles = 0;

	char* szToken = (char*)(LPCSTR)buffer;
  while(szToken = strtok(szToken, ",; ")){
    if(strlen(szToken) == 0)
      continue;

    if(!sscanf(szToken, "%f", &tmpFlt))
      break;

		m_scanAngles.SetAtGrow(m_nScanAngles++, tmpFlt);

    szToken = NULL;
  }

}

bool CImportSpectraDlg::SortSpectrumFiles(){
	if(m_spectrumFiles.GetCount() <= 1 || m_nSpecFiles <= 1){
		return true;
	}

	CString **array = (CString **)m_spectrumFiles.GetData();
	qsort(array, m_nSpecFiles, sizeof(CString *), compareCString);

	return false;
}

bool CImportSpectraDlg::IsScanDOASFiles(){
	

	return true;
}

/** Updates the progress bar */
LRESULT CImportSpectraDlg::OnProgress(WPARAM wp, LPARAM lp){
	m_progressBar.SetPos((int)wp);

	return 0;
}

LRESULT CImportSpectraDlg::OnDone(WPARAM wp, LPARAM lp){
	m_progressBar.SetPos((int)wp);


	SetDlgItemText(IDC_IMPORT_SPECTRA_BTN, "&Import");
	m_statusLabel.ShowWindow(SW_SHOW);
	m_statusLabel.SetWindowText("Done!");
	return 0;
}

void CImportSpectraDlg::TestFn(){
	CSpectrum spec;
	CSTDFile::ReadSpectrum(spec, "C:\\Novac\\TestData\\STD-Files\\00000_0.STD");

	return;
}

UINT ImportSpectraToOneFile( LPVOID pParam ){
	Dialogs::CImportSpectraDlg *wnd = (Dialogs::CImportSpectraDlg *)pParam;
	
	if(wnd->m_nSpecFiles <= 0)
		return 1;

	unsigned int nSpecFiles		= wnd->m_nSpecFiles;
	SpectrumIO::CSpectrumIO writer;
	CString serialNumber, scanDate, scanTime, pakFile;

	// Make sure that the output directory exists
	CreateDirectoryStructure(wnd->m_outputDir);	



	return 0;
}

/** This imports spectra generated using ScanDOAS. The spectra are sorted
	with all spectra from one scan into one pak-file. */
UINT ImportScanDOASSpectra( LPVOID pParam ){
	Dialogs::CImportSpectraDlg *wnd = (Dialogs::CImportSpectraDlg *)pParam;
	CString directory, lastDirectory;
	CList <CString, CString&> directories;
	int k;
	int nDirectories = 0;
	
	// 1 Find all unique directories in the list of files
	for(k = 0; k < wnd->m_nSpecFiles; ++k){
		// Extract the directory for the current file
		directory.Format("%s", *wnd->m_spectrumFiles.GetAt(k));
		Common::GetDirectory(directory);

		// Check if we have this directory already in the list
		if(directories.GetCount() == 0){
			// 1. If the list is empty. just insert the current directory
			directories.AddTail(CString(directory));
			++nDirectories;
			lastDirectory.Format(directory);
			continue;
		}else if(Equals(directory, lastDirectory)){
			continue;  // compare with the last inserted directory to make things a little bit faster...
		}else{
			// 2. Compare with all the other directories in the list, if the current directory
			//		is not in the list, then insert it.
			bool found = false;
			POSITION pos = directories.GetHeadPosition();
			while(pos != NULL){
				if(Equals(directory, directories.GetNext(pos))){
					found = true;
					break;
				}
			}
			if(!found){
				directories.AddTail(CString(directory));
				++nDirectories;
				lastDirectory.Format(directory);
			}
		}
	}

	// 1b. Error check
	if(nDirectories == 0){
		return 1;
	}

	// 2. We should now have a list of which directories we are going to look through.
	//		If there are more than one, then call 'ImportScanDOASSpectraInDirectory'
	//		once for each sub-directory.
	if(nDirectories == 1){
		nSpectraDone = 0;
		ImportScanDOASSpectraInDirectory(wnd->m_spectrumFiles, wnd->m_nSpecFiles, wnd);
		wnd->SendMessage(WM_DONE, wnd->m_nSpecFiles);
		return 0;
	}else{
		nSpectraDone = 0;
		POSITION pos = directories.GetHeadPosition();
		while(pos != NULL){
			CString curDirectory;
			curDirectory.Format(directories.GetNext(pos));

			// Make a new array with all spectra in the current directory
			CArray<CString *, CString *>	spectrumFiles;
			spectrumFiles.SetSize(wnd->m_nSpecFiles / nDirectories); // Initial guess of the number of spectrum files
			int nSpectrumFiles = 0;
			for(k = 0; k < wnd->m_nSpecFiles; ++k){
				directory.Format("%s", *wnd->m_spectrumFiles.GetAt(k));
				Common::GetDirectory(directory);
				if(Equals(directory, curDirectory)){
					spectrumFiles.SetAtGrow(nSpectrumFiles++, wnd->m_spectrumFiles.GetAt(k));
				}
			}

			ImportScanDOASSpectraInDirectory(spectrumFiles, nSpectrumFiles, wnd);
		}
	}

	wnd->SendMessage(WM_DONE, wnd->m_nSpecFiles);
	return 0;
}

int	 ImportScanDOASSpectraInDirectory(const CArray<CString *, CString *>	&m_spectrumFiles, int nSpectrumFiles, Dialogs::CImportSpectraDlg *wnd){
	if(nSpectrumFiles <= 0)
		return 1;

	unsigned int skyIndex		= 0;
	unsigned int darkIndex	= 0;
	unsigned int specIndex	= 0;
	unsigned int nSpecPerScan = wnd->m_specPerScan;
	unsigned int nSpecFiles		= nSpectrumFiles;
	bool fillInScanAngles			= (wnd->m_useUserScanAngles == 1);
	int	m_channel							= (wnd->m_channel);
	int	m_interlacedSpectra		= (wnd->m_interlacedSpectra);
	int	m_averagedSpectra			= (wnd->m_averagedSpectra);
	bool changeSerial					= (wnd->m_userGivenSerial.GetLength() > 0);
	bool changeExposureNum		= (wnd->m_doChangeNumExposures > 0);
	int	exposureNum						= (wnd->m_userGivenNumExposures);
	CSpectrum sky, dark, spec;
	CString serialNumber, scanDate, scanTime, pakFile;
	SpectrumIO::CSpectrumIO writer;
	int tmpInt;
	int currentYear					= Common::GetYear();

	if(changeSerial){
		serialNumber.Format("%s", wnd->m_userGivenSerial);
		serialNumber.MakeUpper();
	}

	// Make sure that the output directory exists
	CreateDirectoryStructure(wnd->m_outputDir);

	// Start by finding the first sky, dark and spectrum indeces
	FindFirst(m_spectrumFiles, nSpecFiles, darkIndex, skyIndex, specIndex);

	unsigned int startIndex = darkIndex + skyIndex + specIndex;

	// Loop through all spectra and 
	while(nSpecFiles > (specIndex + skyIndex + darkIndex - startIndex)){
		// first read the dark and the sky spectrum
		if(!ReadDarkSpectrum(darkIndex++, dark, m_spectrumFiles, nSpecFiles)){
			return 1;
		}
		if(!ReadSkySpectrum(skyIndex++, sky, m_spectrumFiles, nSpecFiles)){
			return 1;
		}

		// Set the spec-index
		sky.m_info.m_scanSpecNum	= nSpecPerScan + 2;
		sky.m_info.m_scanIndex		= 0;
		if(fillInScanAngles)
			sky.m_info.m_scanAngle = 0.0;
		dark.m_info.m_scanSpecNum = nSpecPerScan + 2;
		dark.m_info.m_scanIndex		= 1;
		if(fillInScanAngles)
			dark.m_info.m_scanAngle = 180.0;

		// Set the names of the dark and the sky-spectra
		sky.m_info.m_name.Format("sky");
		dark.m_info.m_name.Format("dark");

		// Set the serial-number information
		sky.m_info.m_device.Format("%s", serialNumber);
		dark.m_info.m_device.Format("%s", serialNumber);
	
		// Correct the date
		if(sky.m_info.m_date[0] > currentYear){
			tmpInt							 = sky.m_info.m_date[0] - 2000;
			sky.m_info.m_date[0] = sky.m_info.m_date[2] + 2000;
			sky.m_info.m_date[2] = tmpInt;
			tmpInt							 = dark.m_info.m_date[0] - 2000;
			dark.m_info.m_date[0]= dark.m_info.m_date[2] + 2000;
			dark.m_info.m_date[2]= tmpInt;
		}
	
		// If necessary, correct the start-time of the sky-spectrum
		if(sky.m_info.m_startTime.hr == 0 && sky.m_info.m_startTime.m == 0){
			sky.m_info.m_startTime = sky.m_info.m_stopTime;		
		}

		// If wanted, set the interlace and channel status of the spectra
		if(m_interlacedSpectra){
			sky.m_info.m_channel	= (unsigned char)(m_channel + 16 * (2048 / sky.m_length) - 16);
			dark.m_info.m_channel = (unsigned char)(m_channel + 16 * (2048 / dark.m_length)- 16);
		}

		// If the user wants to change the number of co-added spectra then do so now.
		if(changeExposureNum){
			sky.m_info.m_numSpec	= exposureNum;
			dark.m_info.m_numSpec	= exposureNum;
		}

		// If the spectra are averaged, then multiply then with the number of spectra co-added
		if(m_averagedSpectra){
			if(sky.NumSpectra() != 0)
				sky.Mult(sky.NumSpectra());

			if(dark.NumSpectra() != 0)
				dark.Mult(dark.NumSpectra());			
		}

		// Get the file name for the .pak-file to save
		CSpectrumInfo &info = sky.m_info;
		if(!changeSerial){
			serialNumber.Format("%s", info.m_device);
			if(0 == serialNumber.Compare(".........."))
				serialNumber.Format("%s", info.m_name);
		}
		scanDate.Format("%02d%02d%02d", info.m_date[0], info.m_date[1], info.m_date[2]);
		scanTime.Format("%02d%02d", info.m_startTime.hr, info.m_startTime.m);
		pakFile.Format("%s\\%s_%s_%s.pak", wnd->m_outputDir, serialNumber, scanDate, scanTime);
		int it = 1;
		while(IsExistingFile(pakFile)){
			pakFile.Format("%s\\%s_%s_%s_v%d.pak", wnd->m_outputDir, serialNumber, scanDate, scanTime, it++);
		}

		// Add them to the output file
		writer.AddSpectrumToFile(pakFile, sky);
		writer.AddSpectrumToFile(pakFile, dark);

		// Then read all the spectra in the scan...
		for(unsigned int i = 0; i < nSpecPerScan; ++i){
			// Check if we should continue running
			if(wnd->m_running == false){
//				wnd->SendMessage(WM_DONE, specIndex + skyIndex + darkIndex - 2 - startIndex);
				return 0;
			}

			// read the spectrum
			if(!ReadSpectrum(specIndex++, spec, m_spectrumFiles, nSpecFiles)){
				return 1;
			}

			// Set the spec-index
			spec.m_info.m_scanSpecNum = nSpecPerScan + 2;
			spec.m_info.m_scanIndex		= i + 2;

			// Correct the date
			if(spec.m_info.m_date[0] > currentYear){
				tmpInt							 = spec.m_info.m_date[0] - 2000;
				spec.m_info.m_date[0]= spec.m_info.m_date[2] + 2000;
				spec.m_info.m_date[2]= tmpInt;
			}

			// Set the scan angle (if wanted)
			if(fillInScanAngles){
				spec.m_info.m_scanAngle = wnd->m_scanAngles.GetAt(i);
				if(wnd->m_scanAngleFormat == 1)
					spec.m_info.m_scanAngle *= 360.0f / 200.0f;
			}

			// Set the serial-number...
			spec.m_info.m_device.Format("%s", serialNumber);

			// Set the name of the measurement
			spec.m_info.m_name.Format("scan");

			// If wanted, se the interlace and channel status of the spectra
			if(m_interlacedSpectra){
				spec.m_info.m_channel	= (unsigned char)(m_channel + 16 * (2048 / spec.m_length) - 16);
			}

			// If the user wants to change the number of co-added spectra then do so now.
			if(changeExposureNum){
				spec.m_info.m_numSpec	= exposureNum;
			}

			// If the spectra are averaged, then multiply then with the number of spectra co-added
			if(m_averagedSpectra){
				if(spec.NumSpectra() != 0)
					spec.Mult(spec.NumSpectra());
			}

			// add it to the pak-file
			writer.AddSpectrumToFile(pakFile, spec);

			wnd->SendMessage(WM_PROGRESS, ++nSpectraDone);
		}
	}

	return 0;
}

// finds the first dark, sky and spectrum indeces
bool FindFirst(const CArray<CString *, CString *>	&spectrumFiles, int nSpecFiles, unsigned int &darkIndex, unsigned int &skyIndex, unsigned int &specIndex){
	CString *str = NULL;
	unsigned int tmpInt1, tmpInt2;
	char buffer[1024];

	// reset all values
	darkIndex = skyIndex = specIndex = UINT_MAX;
	
	for(int i = 0; i < nSpecFiles; ++i){
		str = spectrumFiles.GetAt(i);
		int lastBackslash = str->ReverseFind('\\');
		sprintf(buffer, "%s", str->Right((int)strlen(*str) - lastBackslash - 1));

		// this is a dark spectrum
		if(strstr(buffer, "dark")){
			// get the number
			sscanf(buffer, "dark%ud_%ud.STD", &tmpInt1, &tmpInt2);
			if(tmpInt1 < darkIndex)
				darkIndex = tmpInt1;

			continue;
		}

		// this is a sky spectrum
		if(strstr(buffer, "sky")){
			// get the number
			sscanf(buffer, "sky%ud_%ud.STD", &tmpInt1, &tmpInt2);
			if(tmpInt1 < skyIndex)
				skyIndex = tmpInt1;

			continue;
		}

		// this is a common spectrum
		sscanf(buffer, "%ud_%ud.STD", &tmpInt1, &tmpInt2);
		if(tmpInt1 < specIndex)
			specIndex = tmpInt1;
	}

	return true;
}

bool ReadDarkSpectrum(unsigned int darkIndex, CSpectrum &spec, const CArray<CString *, CString *>	&spectrumFiles, int nSpecFiles){
	static int lastIndex = 0;
	int i = ReadSpectrum(darkIndex, "dark", spec, spectrumFiles, nSpecFiles, lastIndex);
	if(i < 0)
		return false;
	else
		lastIndex = i;
	return true;
}

bool ReadSkySpectrum(unsigned int skyIndex, CSpectrum &spec, const CArray<CString *, CString *>	&spectrumFiles, int nSpecFiles){
	static int lastIndex = 0;
	int i = ReadSpectrum(skyIndex, "sky", spec, spectrumFiles, nSpecFiles, lastIndex);
	if(i < 0)
		return false;
	else
		lastIndex = i;
	return true;
}

bool ReadSpectrum(unsigned int index, CSpectrum &spec, const CArray<CString *, CString *>	&spectrumFiles, int nSpecFiles){
	static int lastIndex = 0;
	int i = ReadSpectrum(index, "", spec, spectrumFiles, nSpecFiles, lastIndex);
	if(i < 0)
		return false;
	else
		lastIndex = i;
	return true;
}

int ReadSpectrum(unsigned int index, const CString &prefix, CSpectrum &spec, const CArray<CString *, CString *>	&spectrumFiles, int nSpecFiles, int startAt){
	CString *str;
	CString number;
	number.Format("%05d", index);

	for(int i = startAt; i < nSpecFiles; ++i){
		str = spectrumFiles.GetAt(i);
		// search for 'dark' spectra...
		if(-1 == str->Find(prefix))
			continue;

		// ... among the dark spectra, find the spectrum with the correct index
		if(-1 == str->Find(number))
			continue;

		// Found it!
		// Read the spectrum
		CSTDFile::ReadSpectrum(spec, *str);
		return i;
	}	

	for(i = 0; i < startAt; ++i){
		str = spectrumFiles.GetAt(i);
		// search for 'dark' spectra...
		if(-1 == str->Find(prefix))
			continue;

		// ... among the dark spectra, find the spectrum with the correct index
		if(-1 == str->Find(number))
			continue;

		// Found it!
		// Read the spectrum
		CSTDFile::ReadSpectrum(spec, *str);
		return i;
	}

	return -1;
}

int	 compareCString(const void *str1, const void *str2){
	CString **s1 = (CString **)str1;
	CString **s2 = (CString **)str2;
	return (*s1)->Compare(**s2);
}
