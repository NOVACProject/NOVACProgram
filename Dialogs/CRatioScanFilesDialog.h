#pragma once
#include "afxdialogex.h"

#include "../Graphs/SpectrumGraph.h"

// CRatioScanFilesDialog is the first page in the Ratio calculation dialog
// and makes it possible for the user to select a number of scan-files (.pak) to use in the calculations.

class RatioCalculationController;

namespace novac
{
    class CSpectrum;
}

class CRatioScanFilesDialog : public CPropertyPage
{
    DECLARE_DYNAMIC(CRatioScanFilesDialog)

public:
    CRatioScanFilesDialog(RatioCalculationController* controller, CWnd* pParent = nullptr);   // standard constructor
    virtual ~CRatioScanFilesDialog();

    /** Initializes the controls and the dialog */
    virtual BOOL OnInitDialog();

    // Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_RATIO_SCANFILES_DIALOG };
#endif

    /** When the user presses the 'browse' button. Opens a dialog and lets the user
        select one or more spectrum files that will be loaded into the program. */
    afx_msg void OnBnClickedBtnBrowsescanfile();

    /** Called when the user selects a new spectrum file to look at. */
    afx_msg void OnChangeSelectedSpectrumFile();
    
    /** Called when the user has pressed the spin control and wants
        to see the next or the previous spectrum in the current file */
    afx_msg void OnChangeSpectrumInFile(NMHDR* pNMHDR, LRESULT* pResult);

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()

private:

    // The controller which this dialog helps to setup. Notice that this dialog does not own the pointer and will not delete it.
    RatioCalculationController* m_controller;

    CListBox m_pakFileListBox;

    CStatic m_graphFrame; // The frame for the spectrum graph
    Graph::CSpectrumGraph m_specGraph; // The spectrum graph
    CSpinButtonCtrl m_specSpin;// The spin button, controlls which spectrum to show

    int m_currentlyDisplayedSpectrumIdx; // For the display of the spectrum. this is the index of the spectrum in the .pak file currently displayed.

    /** Retrieves the full file name and path of the currently selected .pak file
        Returns empty string if no file is selected. */
    std::string GetCurrentlySelectedPakFile() const;

    /** Retrieves the currently selected spectrum in the current .pak file.
        Notice that this may return a value larger than the number of spectra in the file */
    int GetcurrentlySelectedSpectrumIndexInFile() const;

    bool TryReadSpectrum(const std::string& pakfileName, int spectrumIndex, novac::CSpectrum& spectrum);

    /** Updates the graph and the labels with the currently selected spectrum */
    void UpdateUserInterfaceWithSelectedSpectrum();

    /** Draws the currently selected spectrum from the currently selected spectrum-file into the spectrum graph. */
    void DrawSpectrum(novac::CSpectrum& spectrum);

    /** Update the information labels */
    void UpdateSpectrumInfo(const novac::CSpectrum& spectrum, int spectrumIndex, const std::string& fullFilePath);

};
