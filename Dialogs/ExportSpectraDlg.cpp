#include "stdafx.h"
#include "../NovacMasterProgram.h"

#include "../Common/Common.h"
#include "../Common/Spectra/SpectrumIO.h"
#include "../Common/Spectra/PakFileHandler.h"
#include "../Common/SpectrumFormat/STDFile.h"

#include "ExportSpectraDlg.h"

// Include the special multi-choice file-dialog
#include "../Dialogs/FECFileDialog.h"

using namespace SpectrumIO;
using namespace Dialogs;

// CExportSpectraDlg dialog

IMPLEMENT_DYNAMIC(CExportSpectraDlg, CPropertyPage)
CExportSpectraDlg::CExportSpectraDlg()
	: CPropertyPage(CExportSpectraDlg::IDD)
{
  m_exportFormat = 0;
}

CExportSpectraDlg::~CExportSpectraDlg()
{
}

void CExportSpectraDlg::DoDataExchange(CDataExchange* pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_EXPORT_SPECTRA_BTN, m_exportBtn);
  DDX_Radio(pDX, IDC_RADIO_EXPORTFORMAT_STD, m_exportFormat);
}


BEGIN_MESSAGE_MAP(CExportSpectraDlg, CPropertyPage)
  ON_BN_CLICKED(IDC_BROWSE_PAKFILE_BTN, OnBrowsePakFile)
  ON_BN_CLICKED(IDC_BROWSE_EXPORTDIR,		OnBrowseExportDir)
  ON_BN_CLICKED(IDC_EXPORT_SPECTRA_BTN, OnExportSpectra)
	ON_EN_CHANGE(IDC_EDIT_EXPORTDIR,			OnChangeExportDir)
END_MESSAGE_MAP()


// CExportSpectraDlg message handlers

void CExportSpectraDlg::OnBrowsePakFile()
{
	CString str, longString;
    TCHAR filter[] = _T("Pak Files|*.pak|All Files|*.*||");
	int nPakFiles = 0;

	// The file-dialog
	Dialogs::CFECFileDialog fileDialog(TRUE, NULL, NULL, OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER, filter);

	if(fileDialog.DoModal() != IDOK)
		return;

  /** Reset the list */
  m_pakFile.RemoveAll();

	/** Go through the filenames */
	POSITION pos = fileDialog.GetStartPosition();

	while(pos){
		str = fileDialog.GetNextPathName(pos);

		m_pakFile.AddTail(CString(str));
		++nPakFiles;
	}

  // Update the screen
	longString.Format("%d files selected", nPakFiles);
  SetDlgItemText(IDC_STATIC_PAKFILES,longString);

  CString folder;
  GetDlgItemText(IDC_EDIT_EXPORTDIR, folder);
  if(strlen(folder) > 0)
    m_exportBtn.EnableWindow(TRUE);
}

void CExportSpectraDlg::OnBrowseExportDir()
{
  CString folderName;
  Common common;

  if(!common.BrowseForDirectory(folderName))
    return;
  else
    SetDlgItemText(IDC_EDIT_EXPORTDIR, folderName);

  if(m_pakFile.GetSize() > 0)
    m_exportBtn.EnableWindow(TRUE);
}

void CExportSpectraDlg::OnExportSpectra()
{
  // Save all data in the dialog
  UpdateData(TRUE);

  // Get the export directory
  CString exportDir;
  GetDlgItemText(IDC_EDIT_EXPORTDIR, exportDir);

  // Check so that everything is ok
  if(m_pakFile.GetSize() <= 0)
    return;
  if(strlen(exportDir) == 0)
    return;

  // Get the export directory

  // Export all spectra in the spectrum files. 
  SpectrumIO::CSpectrumIO reader;
  Common common;
  CSpectrum spec;
  CString message, path, pakFileName;

  int curSpec = 0;
  int ret = 0;
  POSITION pos = m_pakFile.GetHeadPosition();
  while(pos != NULL){
    CString pakFile = m_pakFile.GetNext(pos);
    curSpec = 0;

    // reset the counter of how many spectra we've converted
    m_specIndex = 0;

    // Create the directory where to save the spectra
    pakFileName.Format(pakFile);
    common.GetFileName(pakFileName);
    pakFileName.Format("%s", (LPCSTR)pakFileName.Left((int)strlen(pakFileName) - 4));
    path.Format("%s\\%s", (LPCSTR)exportDir, (LPCSTR)pakFileName);
    if(CreateDirectoryStructure(path))
      continue; // could not create output directory.

    // Loop through all the spectra in the pak-file
		FILE *sFile = fopen(pakFile, "rb");
		if(sFile != NULL){

			int spectrumNumber = 0; // <-- the number of spectra in the pak-file

			while(1){

//			while(SUCCESS == reader.ReadNextSpectrum(sFile, spec)){
				RETURN_CODE ret = reader.ReadNextSpectrum(sFile, spec);
				if(ret == FAIL){
					if(reader.m_lastError == SpectrumIO::CSpectrumIO::ERROR_EOF || reader.m_lastError == SpectrumIO::CSpectrumIO::ERROR_COULD_NOT_OPEN_FILE || reader.m_lastError == SpectrumIO::CSpectrumIO::ERROR_SPECTRUM_NOT_FOUND)
						break;
					switch(reader.m_lastError){
						case SpectrumIO::CSpectrumIO::ERROR_CHECKSUM_MISMATCH:	
							ShowMessage("Pak file contains a corrupt spectrum, checksum mismatch"); break;
						case SpectrumIO::CSpectrumIO::ERROR_DECOMPRESS:
							ShowMessage("Pak file contains a corrupt spectrum, could not decompress spectrum"); break;
					}
					//else
					++spectrumNumber;
					if(SUCCESS != reader.FindSpectrumNumber(sFile, spectrumNumber))
						break;
				}else
					++spectrumNumber;

				// Save the spectrum in a correct filename and correct fileformat
				SaveSpectrum(spec, path);

				// Increase the number of exported spectra
				++m_specIndex;

				// Increase our conter for the number of spectra generated
				++curSpec;
			} // finished reading one pak-file
			fclose(sFile);
		}

    if(ret != 0 && reader.m_lastError == CSpectrumIO::ERROR_SPECTRUM_NOT_FOUND){
      continue; // end of file - continue with the next pak-file
    }

    if(ret != 0){
      switch(reader.m_lastError){
        case CSpectrumIO::ERROR_COULD_NOT_OPEN_FILE:   message.Format("Could not open spectrum file %s",(LPCSTR) pakFileName); break;
        case CSpectrumIO::ERROR_CHECKSUM_MISMATCH:     message.Format("Spectrum number %d in %s is corrupt - Checksum mismatch",  curSpec, (LPCSTR)pakFileName); break;
        case CSpectrumIO::ERROR_SPECTRUM_TOO_LARGE:    message.Format("Spectrum number %d in %s is corrupt - Spectrum too large", curSpec, (LPCSTR)pakFileName); break;
        case CSpectrumIO::ERROR_DECOMPRESS:            message.Format("Spectrum number %d in %s is corrupt - Decompression error",curSpec, (LPCSTR)pakFileName); break;
      }
      MessageBox(message, "Error", MB_OK);
      return;
    }
  } // end of while(pos != NULL)

  if(m_specIndex > 0){
    message.Format("Successfully exported %d Spectrum files", m_specIndex);
    MessageBox(message);
  }
  return;
}

BOOL CExportSpectraDlg::OnInitDialog()
{
  CPropertyPage::OnInitDialog();
  
  m_exportBtn.EnableWindow(FALSE);

  return TRUE;  // return TRUE unless you set the focus to a control
  // EXCEPTION: OCX Property Pages should return FALSE
}

void CExportSpectraDlg::OnChangeExportDir(){
	CString exportDir;
	GetDlgItemText(IDC_EDIT_EXPORTDIR	, exportDir);

	if(strlen(exportDir) > 2)
		m_exportBtn.EnableWindow(TRUE);
	else
		m_exportBtn.EnableWindow(FALSE);
}

void CExportSpectraDlg::SaveSpectrum(const CSpectrum &spec, const CString &path){
	CString filename;

	// Get the channel number of the spectrum
	unsigned char channel = spec.Channel();
	bool isMultiChannelSpec = FileHandler::CPakFileHandler::CorrectChannelNumber(channel);

	if(!isMultiChannelSpec){
		// Create a filename that describes the spectrum properly
		filename.Format("%s\\%05d_%d.STD", (LPCSTR)path, m_specIndex, channel);

		// Write the spectrum to file
		CSTDFile::WriteSpectrum(spec, filename, 1);

		return;
	}

	// --------------- MULTICHANNEL SPECTRA ------------------
	// Split the interlaced spectrum into it's components
	CSpectrum *mSpec[MAX_CHANNEL_NUM];
	for(int k = 0; k < MAX_CHANNEL_NUM; ++k){
		mSpec[k] = new CSpectrum();
	}

	int nChn = spec.Split(mSpec);

	for(int k = 0; k < nChn; ++k){
		filename.Format("%s\\%05d_%d.STD", (LPCSTR)path, m_specIndex, k);

		// Write the spectrum to file
		CSTDFile::WriteSpectrum(mSpec[k], filename, 1);
	}

	for(int k = 0; k < MAX_CHANNEL_NUM; ++k){
		delete(mSpec[k]);
	}
}