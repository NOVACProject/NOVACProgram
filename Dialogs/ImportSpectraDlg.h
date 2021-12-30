#pragma once
#include <afxtempl.h>

// CImportSpectraDlg dialog
namespace Dialogs
{

    class CImportSpectraDlg : public CPropertyPage
    {
        DECLARE_DYNAMIC(CImportSpectraDlg)

    public:
        CImportSpectraDlg();
        virtual ~CImportSpectraDlg();

        // Dialog Data
        enum { IDD = IDD_IMPORT_SPECTRA };

        virtual BOOL OnInitDialog();
    protected:
        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

        DECLARE_MESSAGE_MAP()

    public:
        /** The spectrum files to import */
        CArray<CString*, CString*>	m_spectrumFiles;

        /** The scan angles, only used for spectra collected with ScanDOAS. */
        CArray<float, float> m_scanAngles;

        /** The number of scan angles in the list */
        int	m_nScanAngles;

        /** if 'm_useUserScanAngles' is 1 then the scan-angles provided by the
                user shall be written into the spectrum files. */
        int	m_useUserScanAngles;

        /** The format of the user supplied scan angles */
        int m_scanAngleFormat;

        /** The number of spectrum files to import */
        int m_nSpecFiles;

        /** The output directory where the data will be saved */
        CString	m_outputDir;

        /** The option, wheather to save the spectra as one file or as one file per scan.
                if m_multiFile is 0 the spectra are saved in one file
                if m_multiFile is 1 the spectra are saved, one scan per file */
        int			m_multiFile;

        /** The number of spectra in each scan */
        int			m_specPerScan;

        /** If the spectra are interlaced or not. If interlaced then value is non-zero */
        int			m_interlacedSpectra;

        /** The channel the spectra comes from */
        int			m_channel;

        /** The serial-number of the spectrometer given by the user */
        CString	m_userGivenSerial;

        /** If the spectra are averaged or not.
                If averaged then the value is non-zero. */
        int					m_averagedSpectra;

        /** The number of exposures, if the user wants to change it */
        int					m_userGivenNumExposures;

        /** Controls if we should change the number of exposures or not.
                Non-zero if the number should be changed. */
        int					m_doChangeNumExposures;

        /** True if the importing thread is running */
        bool				m_running;

    private:
        /** Testing */
        afx_msg void TestFn();

        /** The progress control */
        CProgressCtrl	m_progressBar;

        /** The status label, next to the progress bar */
        CEdit	m_statusLabel;

        /** The import button */
        CButton m_importBtn;

        /** The importing thread */
        CWinThread* m_importThread;

        /** Browse for individual spectrum files to import */
        afx_msg void BrowseForSpectra();

        /** Browse for entire directories containing files to import */
        afx_msg void BrowseForSpectra_Directories();

        /** Browse for the pak-file to save as */
        afx_msg void BrowseForOutputDirectory();

        /** Called to start the importin of data */
        afx_msg void Import();

        /** Saves the data in the dialog */
        afx_msg void SaveData();

        /** Saves the data in the 'elevation angles' edit-box */
        afx_msg void SaveScanAngles();

        /** Updates the progress bar */
        afx_msg LRESULT OnProgress(WPARAM wp, LPARAM lp);
        afx_msg LRESULT OnDone(WPARAM wp, LPARAM lp);

        /** Checks the wheather the spectrum files could be generated
                by ScanDOAS. */
        bool IsScanDOASFiles();

        /** Sorts the spectrum files */
        bool SortSpectrumFiles();

        /** Recursively looks through the given directory for .pak-files
                Will go into sub-directories (recursively) only if 'includeSubDir' is true.
                The found spectra will be inserted into the list 'm_spectrumFiles' */
        void	SearchForSpectrumFiles(const CString& dir, bool includeSubDir);



    };
}