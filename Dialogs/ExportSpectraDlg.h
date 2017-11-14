#pragma once

#include <Afxtempl.h>
#include "afxwin.h"

// CExportSpectraDlg dialog

namespace Dialogs
{
  class CExportSpectraDlg : public CPropertyPage
  {
	  DECLARE_DYNAMIC(CExportSpectraDlg)

  public:
	  CExportSpectraDlg();
	  virtual ~CExportSpectraDlg();

  // Dialog Data
	  enum { IDD = IDD_EXPORT_SPECTRA };

  protected:
	  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	  DECLARE_MESSAGE_MAP()
  public:
		/** Called when the user wants too look for pak-file(s) to export */
    afx_msg void OnBrowsePakFile();

		/** Called when the user want to look for an output directory*/
    afx_msg void OnBrowseExportDir();

		/** Called when the user changes the output directory */
		afx_msg void OnChangeExportDir();

		/** Does the actual exporting */
    afx_msg void OnExportSpectra();

		/** Initializes the controls in the dialog */
    virtual BOOL OnInitDialog();

		/** The 'export' - button */
    CButton m_exportBtn;

		/** The list of pak-files to export */
    CList<CString, CString &> m_pakFile;

    /** The format in which to export the spectra */
    int m_exportFormat;

	private:
		/** Saves a spectrum to a correct filename */
		void SaveSpectrum(const CSpectrum &spec, const CString &path);

		int m_skyIndex, m_darkIndex, m_specIndex;

  };
}