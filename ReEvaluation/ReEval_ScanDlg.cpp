#include "stdafx.h"
#include "../resource.h"
#include "ReEval_ScanDlg.h"
#include "ReEvaluator.h"

#include <SpectralEvaluation/File/SpectrumIO.h>
#include <SpectralEvaluation/Spectra/SpectrometerModel.h>

// Include the special multi-choice file-dialog
#include "../Dialogs/CMultiSelectOpenFileDialog.h"

using namespace novac;

namespace ReEvaluation
{

IMPLEMENT_DYNAMIC(CReEval_ScanDlg, CPropertyPage)

CReEval_ScanDlg::CReEval_ScanDlg(CReEvaluator& reeval)
    : CPropertyPage(CReEval_ScanDlg::IDD), m_reeval(reeval), m_scanfileList(reeval, this)
{
}

CReEval_ScanDlg::~CReEval_ScanDlg()
{}

void CReEval_ScanDlg::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_SCANFILE_LIST, m_scanfileList);
    DDX_Control(pDX, IDC_SPECGRAPH_FRAME, m_graphFrame);
    DDX_Control(pDX, IDC_SPEC_SPIN, m_specSpin);
}


BEGIN_MESSAGE_MAP(CReEval_ScanDlg, CPropertyPage)
    ON_BN_CLICKED(IDC_BTN_BROWSESCANFILE, OnBnClickedBtnBrowsescanfile)
    ON_COMMAND(ID_FILE_LOADSCAN, OnBnClickedBtnBrowsescanfile)
    ON_COMMAND(ID__INSERT266, OnBnClickedBtnBrowsescanfile)
    ON_LBN_SELCHANGE(IDC_SCANFILE_LIST, OnLbnSelchangeScanfileList)
    ON_NOTIFY(UDN_DELTAPOS, IDC_SPEC_SPIN, OnChangeSpectrum)
    ON_COMMAND(ID__REMOVESELECTED, OnRemoveSelected)

    ON_MESSAGE(WM_ZOOM, OnZoomGraph)
END_MESSAGE_MAP()


// CReEval_ScanDlg message handlers

void CReEval_ScanDlg::OnBnClickedBtnBrowsescanfile()
{
    TCHAR filter[] = _T("Pak Files|*.pak|All Files|*.*||");

    // save the contents in the dialog
    UpdateData(TRUE);

    // The file-dialog
    Dialogs::CMultiSelectOpenFileDialog fileDialog(TRUE, nullptr, nullptr, OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER, filter);

    if (fileDialog.DoModal() != IDOK)
        return;

    /** Reset the old values */
    m_reeval.m_scanFile.clear();

    /** Go through the filenames */
    POSITION pos = fileDialog.GetStartPosition();

    while (pos)
    {
        CString str = fileDialog.GetNextPathName(pos);
        m_reeval.m_scanFile.push_back(std::string(str));
    }

    // Sort the scans in the file-list
    m_reeval.SortScans();

    // update the list
    m_scanfileList.PopulateList();

    // redraw the screen
    m_scanfileList.SetCurSel(0);
    OnLbnSelchangeScanfileList();
}

BOOL CReEval_ScanDlg::OnInitDialog()
{
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
    m_specGraph.SetGridColor(RGB(255, 255, 255));
    m_specGraph.SetPlotColor(RGB(0, 255, 0));
    m_specGraph.parentWnd = this;

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CReEval_ScanDlg::OnLbnSelchangeScanfileList()
{

    m_curSpecFile = m_scanfileList.GetCurSel();
    m_curSpec = 0;

    if (CheckScanFile())
    {
        SetDlgItemText(IDC_NUMSPEC_LABEL, "Scan file is corrupt");
        return;
    }

    // update the on screen information
    UpdateInfo();

    // Update the spin control
    m_specSpin.SetRange(0, (short)(m_specNum - 1));
    m_specSpin.SetPos((short)m_curSpec);

    // draw the spectrum
    DrawSpectrum();

    // update the labels
    UpdateInfo();
}

void CReEval_ScanDlg::DrawSpectrum()
{
    if (TryReadSpectrum())
        return;

    Graph::CSpectrumGraph::plotRange range;
    GetPlotRange(range);

    m_specGraph.CleanPlot();
    m_specGraph.SetPlotColor(RGB(0, 255, 0));
    m_specGraph.SetRange(range.minLambda, range.maxLambda, 0, range.minIntens, range.maxIntens, 0);

    m_specGraph.XYPlot(nullptr, m_spectrum.m_data, m_spectrum.m_length, Graph::CGraphCtrl::PLOT_FIXED_AXIS | Graph::CGraphCtrl::PLOT_CONNECTED);
}

int CReEval_ScanDlg::TryReadSpectrum()
{
    CSpectrumIO reader;

    // Read the spectrum
    const std::string scanFileName(m_reeval.m_scanFile[m_curSpecFile]);
    const bool success = reader.ReadSpectrum(scanFileName, m_curSpec, m_spectrum);

    if (!success)
    {
        CString message;
        message.Format("Error reading spectrum %d. Error: %s", m_curSpec, novac::ToString(reader.m_lastError).c_str());
        return 1;
    }
    else
    {
        //CSpectrumInfo &info = m_spectrum.m_info;
        //CString dateStr, startTimeStr, stopTimeStr, expTimeStr, numSpecStr;
        //dateStr.Format("Date: %04d.%02d.%02d", info.m_date[0], info.m_date[1], info.m_date[2]);
        //startTimeStr.Format("Start Time: %02d:%02d:%02d", info.m_startTime.hr, info.m_startTime.m, info.m_startTime.sec);
        //stopTimeStr.Format("Stop Time:  %02d:%02d:%02d", info.m_stopTime.hr, info.m_stopTime.m, info.m_stopTime.sec);
        //expTimeStr.Format("Exposure Time: %d [ms]", info.m_exposureTime);
        //numSpecStr.Format("# Spectra: %d", info.m_numSpec);

        //SetDlgItemText(IDC_LBL_DATE, dateStr);
        //SetDlgItemText(IDC_LBL_STARTTIME, startTimeStr);
        //SetDlgItemText(IDC_LBL_STOPTIME, stopTimeStr);
        //SetDlgItemText(IDC_LBL_EXPTIME, expTimeStr);
        //SetDlgItemText(IDC_LBL_SPECNUM, numSpecStr);

        return 0;
    }
}

int CReEval_ScanDlg::CheckScanFile()
{
    m_specNum = 0;

    // Read one spectrum to check the channel numbers
    const std::string scanFileName(m_reeval.m_scanFile[m_curSpecFile]);

    CSpectrumIO reader;
    CSpectrum spec;
    reader.ReadSpectrum(scanFileName, 0, spec);

    unsigned char chn = spec.Channel();
    int steps = 0;
    if (-1 == Common::GetInterlaceSteps(chn, steps))
    {
        MessageBox("This scan-file contains interlaced spectra. The ReEvaluation can only evaluate one channel at a time. Please split this file into one pak-file for each channel using the 'Split/Merge Pak-file(s)'-function under the 'File' menu", "Cannot evaluate interlaced spectra", MB_OK);
        return 1;
    }

    // Read the spectrum
    m_specNum = reader.CountSpectra(scanFileName);

    // check if the file was ok. 
    if (reader.m_lastError == novac::FileError::SpectrumNotFound || reader.m_lastError == novac::FileError::EndOfFile)
    {
        return 0;
    }

    // something is wrong with the file
    return 1;
}

void CReEval_ScanDlg::OnChangeSpectrum(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);

    if (m_specNum <= 0)
        return;

    // get the current spectrum index
    m_curSpec = m_specSpin.GetPos32();
    if (pNMUpDown->iDelta > 0)
        m_curSpec += 1;
    else
        m_curSpec -= 1;

    // set the limits for the selected spectrum
    m_curSpec = std::max(m_curSpec, 0L);
    m_curSpec = std::min(m_curSpec, m_specNum - 1);

    // Reset the scale of the graph
    m_specGraph.ResetZoomRect();

    // redraw the graph
    DrawSpectrum();

    // update the spectrum information
    UpdateInfo();

    *pResult = 0;
}

void CReEval_ScanDlg::UpdateInfo()
{
    CString status, dateStr, startStr, stopStr, expTimeStr;
    CString deviceStr, numSpecStr, motorStr, geomStr;
    CSpectrumInfo& info = m_spectrum.m_info;

    // show the status bar
    status.Format("Showing spectrum %d out of %d", m_curSpec + 1, m_specNum);
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

/** Called when the user wants to remove the currently selected scan */
void CReEval_ScanDlg::OnRemoveSelected()
{
    if (m_specNum <= 0)
        return;

    // The currently selected scan-file
    m_curSpecFile = m_scanfileList.GetCurSel();
    if (m_curSpecFile == -1)
        return;

    // Remove the file
    m_reeval.m_scanFile.erase(begin(m_reeval.m_scanFile) + m_curSpecFile);

    m_curSpecFile = std::max(0L, m_curSpecFile - 1);

    // Update the list
    m_scanfileList.PopulateList();

    m_scanfileList.SetCurSel(m_curSpecFile);

    // redraw the graph
    DrawSpectrum();

    // update the spectrum information
    UpdateInfo();

}

void CReEval_ScanDlg::GetPlotRange(Graph::CSpectrumGraph::plotRange& range)
{
    // See if the user has determined any range...
    Graph::CSpectrumGraph::plotRange userDeterminedRange;
    m_specGraph.GetZoomRect(userDeterminedRange);

    if (fabs(userDeterminedRange.maxLambda) > 0.1)
    {
        range = userDeterminedRange;
        return;
    }
    else
    {
        range.minIntens = 0.0;
        range.maxIntens = FullDynamicRangeForSpectrum(m_spectrum.m_info);
        range.minLambda = 0.0;
        range.maxLambda = m_spectrum.m_length;

        return;
    }
}

LRESULT CReEval_ScanDlg::OnZoomGraph(WPARAM wParam, LPARAM lParam)
{
    this->DrawSpectrum();
    return 0;
}

}  // namespace ReEvaluation
