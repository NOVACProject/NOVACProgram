// CRatioScanFilesDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "afxdialogex.h"
#include "../resource.h"
#include "CRatioScanFilesDialog.h"

#include <SpectralEvaluation/DialogControllers/RatioCalculationController.h>

#include <SpectralEvaluation/File/SpectrumIO.h>
#include <SpectralEvaluation/Spectra/Spectrum.h>
#include <SpectralEvaluation/Spectra/SpectrometerModel.h>

#include <algorithm>

// Include the special multi-choice file-dialog
#include "../Dialogs/FECFileDialog.h"

#include "../Common/Common.h" // TODO: try to avoid this

#undef min
#undef max

// CRatioScanFilesDialog dialog

IMPLEMENT_DYNAMIC(CRatioScanFilesDialog, CPropertyPage)

CRatioScanFilesDialog::CRatioScanFilesDialog(RatioCalculationController* controller, CWnd* pParent /*=nullptr*/)
    : CPropertyPage(IDD_RATIO_SCANFILES_DIALOG), m_controller(controller)
{
}

CRatioScanFilesDialog::~CRatioScanFilesDialog()
{
    m_controller = nullptr;
}

BOOL CRatioScanFilesDialog::OnInitDialog() {
    CPropertyPage::OnInitDialog();

    Common common;

    // Initialize the spectrum graph.
    CRect rect;
    int margin = -8;
    m_graphFrame.GetWindowRect(&rect);
    rect.bottom -= rect.top + margin;
    rect.right -= rect.left + margin;
    rect.top = margin + 10;
    rect.left = margin;
    m_specGraph.Create(WS_VISIBLE | WS_CHILD, rect, &m_graphFrame);
    m_specGraph.SetRange(0, MAX_SPECTRUM_LENGTH, 1, 0.0, 4095.0, 1);
    m_specGraph.SetYUnits(common.GetString(AXIS_INTENSITY));
    m_specGraph.SetXUnits(common.GetString(AXIS_CHANNEL));
    m_specGraph.SetBackgroundColor(RGB(0, 0, 0));
    m_specGraph.SetGridColor(RGB(255, 255, 255));//(192, 192, 255)) ;
    m_specGraph.SetPlotColor(RGB(0, 255, 0));
    m_specGraph.parentWnd = this;

    return TRUE;  // return TRUE unless you set the focus to a control
}

void CRatioScanFilesDialog::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_SCANFILE_LIST, m_pakFileListBox);
    DDX_Control(pDX, IDC_SPECGRAPH_FRAME, m_graphFrame);
    DDX_Control(pDX, IDC_SPEC_SPIN, m_specSpin);
}


BEGIN_MESSAGE_MAP(CRatioScanFilesDialog, CPropertyPage)
    ON_BN_CLICKED(IDC_BTN_BROWSESCANFILE, &CRatioScanFilesDialog::OnBnClickedBtnBrowsescanfile)
    ON_NOTIFY(UDN_DELTAPOS, IDC_SPEC_SPIN, &CRatioScanFilesDialog::OnChangeSpectrumInFile)
    ON_LBN_SELCHANGE(IDC_SCANFILE_LIST, &CRatioScanFilesDialog::OnChangeSelectedSpectrumFile)
END_MESSAGE_MAP()


// CRatioScanFilesDialog message handlers


void CRatioScanFilesDialog::OnBnClickedBtnBrowsescanfile()
{
    TCHAR filter[] = _T("Pak Files|*.pak|All Files|*.*||");

    // Reset all data
    m_controller->m_pakfiles.clear();
    m_currentlyDisplayedSpectrumIdx = 0;

    // Let the user select the file.
    Dialogs::CFECFileDialog fileDialog(TRUE, NULL, NULL, OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER, filter);
    if (fileDialog.DoModal() != IDOK)
    {
        return;
    }

    // Go through the names of the selected files.
    POSITION pos = fileDialog.GetStartPosition();

    std::vector<std::string> pakfileList;
    while (pos) {
        std::string str = std::string((LPCSTR)fileDialog.GetNextPathName(pos));
        pakfileList.push_back(str);
    }

    // Sort the scans in the file-list
    std::sort(begin(pakfileList), end(pakfileList));

    // update the list
    m_controller->m_pakfiles = pakfileList;

    m_pakFileListBox.ResetContent();
    for (const auto& file : m_controller->m_pakfiles)
    {
        CString fileName{ file.c_str() };
        m_pakFileListBox.AddString(fileName);
    }

    // reset the counters
    m_pakFileListBox.SetCurSel(0);
    m_specSpin.SetPos((short)0);

    // Update the UI
    OnChangeSelectedSpectrumFile();
}

void CRatioScanFilesDialog::OnChangeSpectrumInFile(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);

    if (m_controller->m_pakfiles.size() == 0)
    {
        return;
    }

    // Reset the scale of the graph
    m_specGraph.ResetZoomRect();

    UpdateUserInterfaceWithSelectedSpectrum();

    *pResult = 0;
}


void CRatioScanFilesDialog::OnChangeSelectedSpectrumFile()
{
    const std::string pakFile = GetCurrentlySelectedPakFile();
    if (pakFile.empty())
    {
        return;
    }

    // Check the scan file so that it is ok.
    novac::CSpectrumIO reader;
    const int numberOfSpectraInFile = reader.CountSpectra(pakFile);

    // if (numberOfSpectraInFile == 0) {
    //     status.Format("Scan file is corrupt");
    //     SetDlgItemText(IDC_NUMSPEC_LABEL, status);
    //     return;
    // }

    // Update the spin control
    m_specSpin.SetRange(0, (short)(numberOfSpectraInFile - 1));
    m_specSpin.SetPos((short)0);

    UpdateUserInterfaceWithSelectedSpectrum();
}

std::string CRatioScanFilesDialog::GetCurrentlySelectedPakFile() const
{
    if (m_controller->m_pakfiles.size() == 0)
    {
        return "";
    }

    int idx = m_pakFileListBox.GetCurSel();
    if (idx < 0 || idx >= static_cast<int>(m_controller->m_pakfiles.size()))
    {
        return "";
    }

    return m_controller->m_pakfiles[idx];
}

int CRatioScanFilesDialog::GetcurrentlySelectedSpectrumIndexInFile() const
{
    return m_specSpin.GetPos32();
}

bool CRatioScanFilesDialog::TryReadSpectrum(const std::string& pakfileName, int spectrumIndex, novac::CSpectrum& spectrum)
{
    novac::CSpectrumIO reader;
    return reader.ReadSpectrum(pakfileName, spectrumIndex, spectrum);
}

void CRatioScanFilesDialog::UpdateUserInterfaceWithSelectedSpectrum()
{
    const std::string filename = GetCurrentlySelectedPakFile();
    const int spectrumIndex = GetcurrentlySelectedSpectrumIndexInFile();

    novac::CSpectrum currentSpectrum;
    if (!TryReadSpectrum(filename, spectrumIndex, currentSpectrum))
    {
        return;
    }

    DrawSpectrum(currentSpectrum);
    UpdateSpectrumInfo(currentSpectrum, spectrumIndex, filename);
}

void CRatioScanFilesDialog::DrawSpectrum(novac::CSpectrum& spectrum)
{
    Graph::CSpectrumGraph::plotRange range;
    range.minIntens = 0.0;
    range.maxIntens = FullDynamicRangeForSpectrum(spectrum.m_info);
    range.minLambda = 0.0;
    range.maxLambda = spectrum.m_length;

    m_specGraph.CleanPlot();
    m_specGraph.SetRange(range.minLambda, range.maxLambda, 0, range.minIntens, range.maxIntens, 0);

    m_specGraph.XYPlot(nullptr, spectrum.m_data, spectrum.m_length, Graph::CGraphCtrl::PLOT_FIXED_AXIS | Graph::CGraphCtrl::PLOT_CONNECTED);
}

void CRatioScanFilesDialog::UpdateSpectrumInfo(const novac::CSpectrum& spectrum, int spectrumIndex, const std::string& fullFilePath)
{
    CString status, dateStr, startStr, stopStr, expTimeStr;
    CString deviceStr, numSpecStr, motorStr, geomStr;
    const novac::CSpectrumInfo& info = spectrum.m_info;

    std::string fileName = fullFilePath;
    Common::GetFileName(fileName);

    // show the status bar
    status.Format("Showing spectrum %d in %s", spectrumIndex + 1, fileName.c_str());
    SetDlgItemText(IDC_NUMSPEC_LABEL, status);

    // Show the name of the spectrometer
    deviceStr.Format("Device: %s", info.m_device.c_str());
    SetDlgItemText(IDC_LBL_DEVICE, deviceStr);

    // Show the date the spectrum was collected
    dateStr.Format("Date: %04d.%02d.%02d [yyyy.mm.dd]", info.m_startTime.year, info.m_startTime.month, info.m_startTime.day);
    SetDlgItemText(IDC_LBL_DATE, dateStr);

    // Show the time when the spectrum collection began
    startStr.Format("Start Time: %02d:%02d:%02d", info.m_startTime.hour, info.m_startTime.minute, info.m_startTime.second);
    SetDlgItemText(IDC_LBL_STARTTIME, startStr);

    // Show the time when the spectrum collection stopped
    stopStr.Format("Stop Time: %02d:%02d:%02d", info.m_stopTime.hour, info.m_stopTime.minute, info.m_stopTime.second);
    SetDlgItemText(IDC_LBL_STOPTIME, stopStr);

    // Show the exposure time of the spectrum
    expTimeStr.Format("Exposure Time: %d [ms]", info.m_exposureTime);
    SetDlgItemText(IDC_LBL_EXPTIME, expTimeStr);

    // Show the number of spectra coadded to get this spectrum
    numSpecStr.Format("#Exposures: %d", info.m_numSpec);
    SetDlgItemText(IDC_LBL_NUMSPEC, numSpecStr);

    // Show the position of the motor when the spectrum was collected
    motorStr.Format("Scan Angle: %0.0f [deg]", info.m_scanAngle);
    SetDlgItemText(IDC_LBL_MOTORPOS, motorStr);

    // Show if this is a cone-scanner or a flat scanner
    if (fabs(info.m_coneAngle - 90.0) < 1e-2)
        geomStr.Format("Geometry: flat");
    else
        geomStr.Format("Geometry: Cone, %.1lf [deg]", info.m_coneAngle);
    SetDlgItemText(IDC_LBL_CONEORFLAT, geomStr);

}
