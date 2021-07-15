// CCalibratePixelToWavelengthDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "CCalibratePixelToWavelengthDialog.h"
#include "afxdialogex.h"
#include "../resource.h"
#include "../Common/Common.h"
#include "../Calibration/WavelengthCalibrationController.h"
#include "OpenInstrumentCalibrationDialog.h"
#include <fstream>
#include <SpectralEvaluation/File/File.h>

// CCalibratePixelToWavelengthDialog dialog

// Pointer to the dialog itself. Used for callbacks
CDialog* wavelengthCalibrationDialog = nullptr;

IMPLEMENT_DYNAMIC(CCalibratePixelToWavelengthDialog, CPropertyPage)

CCalibratePixelToWavelengthDialog::CCalibratePixelToWavelengthDialog(CWnd* pParent /*=nullptr*/)
    : CPropertyPage(IDD_CALIBRATE_WAVELENGTH_DIALOG)
    , m_inputSpectrumFile(_T(""))
    , m_initialCalibrationFileTypeFilter("Novac Instrument Calibration Files\0*.xml\0")
{
    wavelengthCalibrationDialog = this;

    this->m_controller = new WavelengthCalibrationController();
}

CCalibratePixelToWavelengthDialog::~CCalibratePixelToWavelengthDialog()
{
    delete this->m_controller;
}

BOOL CCalibratePixelToWavelengthDialog::OnInitDialog() {
    CPropertyPage::OnInitDialog();

    CRect rect;
    int margin = 2;
    m_graphHolder.GetWindowRect(&rect);
    rect.bottom -= rect.top + margin;
    rect.right -= rect.left + margin;
    rect.top = margin + 7;
    rect.left = margin;
    m_graph.Create(WS_VISIBLE | WS_CHILD, rect, &m_graphHolder);
    m_graph.SetRange(0, 500, 1, -100.0, 100.0, 1);
    m_graph.SetYUnits("Wavelength [nm]");
    m_graph.SetXUnits("Pixel");
    m_graph.SetBackgroundColor(RGB(0, 0, 0));
    m_graph.SetGridColor(RGB(255, 255, 255));
    m_graph.SetPlotColor(RGB(255, 0, 0));
    m_graph.CleanPlot();

    m_graphTypeList.AddString("Polynomial");
    m_graphTypeList.AddString("Spectra & Polynomial");
    m_graphTypeList.AddString("Measured Spectrum");
    m_graphTypeList.AddString("Fraunhofer Spectrum");
    m_graphTypeList.SetCurSel(0);

    LoadDefaultSetup();
    LoadLastSetup();

    return TRUE;  // return TRUE unless you set the focus to a control
}

void CCalibratePixelToWavelengthDialog::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_SPECTRUM, m_inputSpectrumFile);
    DDX_Text(pDX, IDC_EDIT_SOLAR_SPECTRUM, m_setup.m_solarSpectrumFile);
    DDX_Text(pDX, IDC_EDIT_INITIAL_CALIBRATION, m_setup.m_initialCalibrationFile);
    DDX_Control(pDX, IDC_STATIC_GRAPH_HOLDER_PANEL, m_graphHolder);
    DDX_Control(pDX, IDC_BUTTON_RUN, m_runButton);
    DDX_Control(pDX, IDC_BUTTON_SAVE, m_saveButton);
    DDX_Control(pDX, IDC_STATIC_INITIAL_CALIBRATION, m_wavelengthCalibrationLabel);
    DDX_Control(pDX, IDC_LIST_GRAPH_TYPE, m_graphTypeList);
}

BEGIN_MESSAGE_MAP(CCalibratePixelToWavelengthDialog, CPropertyPage)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_SPECTRUM, &CCalibratePixelToWavelengthDialog::OnClickedButtonBrowseSpectrum)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_SOLAR_SPECTRUM, &CCalibratePixelToWavelengthDialog::OnClickedButtonBrowseSolarSpectrum)
    ON_BN_CLICKED(IDC_BUTTON_RUN, &CCalibratePixelToWavelengthDialog::OnClickedButtonRun)
    ON_BN_CLICKED(IDC_BUTTON_SAVE, &CCalibratePixelToWavelengthDialog::OnClickedButtonSave)

    ON_MESSAGE(WM_DONE, OnCalibrationDone)
    ON_LBN_SELCHANGE(IDC_LIST_GRAPH_TYPE, &CCalibratePixelToWavelengthDialog::OnSelchangeListGraphType)
    ON_BN_CLICKED(IDC_BUTTON_SELECT_INITIAL_CALIBRATION, &CCalibratePixelToWavelengthDialog::OnButtonSelectInitialCalibration)
END_MESSAGE_MAP()

// Persisting the setup to file
std::string CCalibratePixelToWavelengthDialog::SetupFilePath()
{
    Common common;
    common.GetExePath();
    CString path;
    path.Format("%sCalibrateWavelengthDlg.config", common.m_exePath);
    return std::string(path);
}

void CCalibratePixelToWavelengthDialog::SaveSetup()
{
    try
    {
        std::ofstream dst(this->SetupFilePath(), std::ios::out);
        dst << "<CalibrateWavelengthDlg>" << std::endl;
        dst << "\t<SolarSpectrum>" << this->m_setup.m_solarSpectrumFile << "</SolarSpectrum>" << std::endl;
        dst << "\t<InitialCalibrationFile>" << this->m_setup.m_initialCalibrationFile << "</InitialCalibrationFile>" << std::endl;
        dst << "\t<LineShapeFile>" << this->m_setup.m_instrumentLineshapeFile << "</LineShapeFile>" << std::endl;
        dst << "\t<InputFileType>" << (int)this->m_setup.m_calibrationOption << "</InputFileType>" << std::endl;
        dst << "</CalibrateWavelengthDlg>" << std::endl;
    }
    catch (std::exception&)
    {
    }
}

CString ParseXmlString(const char* startTag, const char* stopTag, const std::string& line)
{
    const size_t firstIdx = line.find(startTag);
    const size_t start = firstIdx + strlen(startTag);
    const size_t stop = line.find(stopTag);
    if (stop > start && firstIdx != line.npos && stop != line.npos)
    {
        return CString(line.c_str() + start, static_cast<int>(stop - start));
    }
    return CString(); // parse failure, return empty string.
}

int ParseXmlInteger(const char* startTag, const char* stopTag, const std::string& line)
{
    const size_t firstIdx = line.find(startTag);
    const size_t start = firstIdx + strlen(startTag);
    const size_t stop = line.find(stopTag);
    if (stop > start && firstIdx != line.npos && stop != line.npos)
    {
        std::string valueStr = line.substr(start, stop - start);
        return std::atoi(valueStr.c_str());
    }
    return 0; // parse failure, return zero
}

void CCalibratePixelToWavelengthDialog::LoadDefaultSetup()
{
    Common common;
    common.GetExePath();

    // See if there is any possible references in the current directory already
    const auto solarCrossSection = Common::ListFilesInDirectory(common.m_exePath, "SOLAR*.xs");

    if (solarCrossSection.size() > 0)
    {
        this->m_setup.m_solarSpectrumFile = CString(solarCrossSection.front().c_str());
        UpdateData(FALSE);
    }
}

void CCalibratePixelToWavelengthDialog::LoadLastSetup()
{
    try
    {
        // Super basic xml parsing
        std::ifstream file(this->SetupFilePath(), std::ios::in);
        std::string line;
        while (std::getline(file, line))
        {
            if (line.find("SolarSpectrum") != std::string::npos)
            {
                this->m_setup.m_solarSpectrumFile = ParseXmlString("<SolarSpectrum>", "</SolarSpectrum>", line);
            }
            else if (line.find("InitialCalibrationFile") != std::string::npos)
            {
                this->m_setup.m_initialCalibrationFile = ParseXmlString("<InitialCalibrationFile>", "</InitialCalibrationFile>", line);
            }
            else if (line.find("LineShapeFile") != std::string::npos)
            {
                this->m_setup.m_instrumentLineshapeFile = ParseXmlString("<LineShapeFile>", "</LineShapeFile>", line);
            }
            else if (line.find("InputFileType") != std::string::npos)
            {
                this->m_setup.m_calibrationOption = ParseXmlInteger("<InputFileType>", "</InputFileType>", line);

                // TODO: when to set this?
                // this->m_instrumentCalibrationTypeCombo.SetCurSel(this->m_setup.m_calibrationOption);
            }
        }
    }
    catch (std::exception&)
    {
    }
}

// CCalibratePixelToWavelengthDialog message handlers

void CCalibratePixelToWavelengthDialog::OnClickedButtonBrowseSpectrum()
{
    if (!Common::BrowseForFile("Pak Files\0*.pak\0", this->m_inputSpectrumFile))
    {
        return;
    }
    UpdateData(FALSE);
}

void CCalibratePixelToWavelengthDialog::OnClickedButtonBrowseSolarSpectrum()
{
    if (!Common::BrowseForFile("Spectrum Files\0*.std;*.txt;*.xs\0", this->m_setup.m_solarSpectrumFile))
    {
        return;
    }
    UpdateData(FALSE);
}

void CCalibratePixelToWavelengthDialog::UpdateGraph()
{
    const int currentGraph = this->m_graphTypeList.GetCurSel();
    if (currentGraph == 3)
    {
        DrawFraunhoferSpectrumAndKeypoints();
    }
    else if (currentGraph == 2)
    {
        DrawMeasuredSpectrumAndKeypoints();
    }
    else if (currentGraph == 1)
    {
        DrawSpectraAndInliers();
    }
    else
    {
        DrawPolynomialAndInliers();
    }
}


void CCalibratePixelToWavelengthDialog::OnSelchangeListGraphType()
{
    UpdateGraph();
}

void CCalibratePixelToWavelengthDialog::DrawPolynomialAndInliers()
{
    this->m_graph.CleanPlot();

    m_graph.SetXUnits("Pixel");
    m_graph.SetYUnits("Wavelength [nm]");

    // the old (initial) calibration polynomial
    this->m_graph.SetPlotColor(RGB(128, 0, 0));
    this->m_graph.Plot(
        m_controller->m_calibrationDebug.initialPixelToWavelengthMapping.data(),
        static_cast<int>(m_controller->m_calibrationDebug.initialPixelToWavelengthMapping.size()),
        Graph::CGraphCtrl::PLOT_CONNECTED);

    // the calibration polynomial
    this->m_graph.SetPlotColor(RGB(255, 0, 0));
    this->m_graph.Plot(
        m_controller->m_resultingPixelToWavelengthMapping.data(),
        static_cast<int>(m_controller->m_resultingPixelToWavelengthMapping.size()),
        Graph::CGraphCtrl::PLOT_CONNECTED | Graph::CGraphCtrl::PLOT_FIXED_AXIS);

    // outliers
    this->m_graph.SetCircleColor(RGB(128, 128, 128));
    this->m_graph.DrawCircles(
        m_controller->m_calibrationDebug.outlierCorrespondencePixels.data(),
        m_controller->m_calibrationDebug.outlierCorrespondenceWavelengths.data(),
        static_cast<int>(m_controller->m_calibrationDebug.outlierCorrespondencePixels.size()),
        Graph::CGraphCtrl::PLOT_FIXED_AXIS);

    // inliers
    this->m_graph.SetCircleColor(RGB(255, 255, 255));
    this->m_graph.DrawCircles(
        m_controller->m_calibrationDebug.inlierCorrespondencePixels.data(),
        m_controller->m_calibrationDebug.inlierCorrespondenceWavelengths.data(),
        static_cast<int>(m_controller->m_calibrationDebug.inlierCorrespondencePixels.size()),
        Graph::CGraphCtrl::PLOT_FIXED_AXIS);
}

void CCalibratePixelToWavelengthDialog::DrawMeasuredSpectrumAndKeypoints()
{
    this->m_graph.CleanPlot();

    m_graph.SetXUnits("Pixel");
    m_graph.SetYUnits("");

    // the measured spectrum
    this->m_graph.SetPlotColor(RGB(255, 0, 0));
    this->m_graph.Plot(
        m_controller->m_calibrationDebug.measuredSpectrum.data(),
        static_cast<int>(m_controller->m_calibrationDebug.measuredSpectrum.size()),
        Graph::CGraphCtrl::PLOT_CONNECTED);

    // all the keypoints found
    this->m_graph.SetCircleColor(RGB(128, 128, 128));
    this->m_graph.DrawCircles(
        m_controller->m_calibrationDebug.measuredSpectrumKeypointPixels.data(),
        m_controller->m_calibrationDebug.measuredSpectrumKeypointIntensities.data(),
        static_cast<int>(m_controller->m_calibrationDebug.measuredSpectrumKeypointIntensities.size()),
        Graph::CGraphCtrl::PLOT_FIXED_AXIS);

    // all the keypoints used
    this->m_graph.SetCircleColor(RGB(255, 255, 255));
    this->m_graph.DrawCircles(
        m_controller->m_calibrationDebug.measuredSpectrumInlierKeypointPixels.data(),
        m_controller->m_calibrationDebug.measuredSpectrumInlierKeypointIntensities.data(),
        static_cast<int>(m_controller->m_calibrationDebug.measuredSpectrumInlierKeypointIntensities.size()),
        Graph::CGraphCtrl::PLOT_FIXED_AXIS);
}

void CCalibratePixelToWavelengthDialog::DrawFraunhoferSpectrumAndKeypoints()
{
    this->m_graph.CleanPlot();

    m_graph.SetXUnits("Wavelength");
    m_graph.SetYUnits("");

    // the Fraunhofer spectrum
    this->m_graph.SetPlotColor(RGB(0, 255, 0));
    this->m_graph.XYPlot(
        m_controller->m_resultingPixelToWavelengthMapping.data(),
        m_controller->m_calibrationDebug.fraunhoferSpectrum.data(),
        static_cast<int>(m_controller->m_calibrationDebug.fraunhoferSpectrum.size()),
        Graph::CGraphCtrl::PLOT_CONNECTED);

    // all the keypoints found
    this->m_graph.SetCircleColor(RGB(128, 128, 128));
    this->m_graph.DrawCircles(
        m_controller->m_calibrationDebug.fraunhoferSpectrumKeypointWavelength.data(),
        m_controller->m_calibrationDebug.fraunhoferSpectrumKeypointIntensities.data(),
        static_cast<int>(m_controller->m_calibrationDebug.fraunhoferSpectrumKeypointIntensities.size()),
        Graph::CGraphCtrl::PLOT_FIXED_AXIS);

    // all the keypoints used
    this->m_graph.SetCircleColor(RGB(255, 255, 255));
    this->m_graph.DrawCircles(
        m_controller->m_calibrationDebug.fraunhoferSpectrumInlierKeypointWavelength.data(),
        m_controller->m_calibrationDebug.fraunhoferSpectrumInlierKeypointIntensities.data(),
        static_cast<int>(m_controller->m_calibrationDebug.fraunhoferSpectrumInlierKeypointIntensities.size()),
        Graph::CGraphCtrl::PLOT_FIXED_AXIS);
}

void CCalibratePixelToWavelengthDialog::DrawSpectraAndInliers()
{
    this->m_graph.CleanPlot();

    m_graph.SetXUnits("Pixel");
    m_graph.SetYUnits("Wavelength [nm]");
    m_graph.SetSecondRangeY(0, 1, 1, false);

    // the calibration polynomial
    this->m_graph.SetPlotColor(RGB(255, 0, 0));
    this->m_graph.Plot(
        m_controller->m_resultingPixelToWavelengthMapping.data(),
        static_cast<int>(m_controller->m_resultingPixelToWavelengthMapping.size()),
        Graph::CGraphCtrl::PLOT_CONNECTED);

    // inliers
    this->m_graph.SetCircleColor(RGB(255, 255, 255));
    this->m_graph.DrawCircles(
        m_controller->m_calibrationDebug.inlierCorrespondencePixels.data(),
        m_controller->m_calibrationDebug.inlierCorrespondenceWavelengths.data(),
        static_cast<int>(m_controller->m_calibrationDebug.inlierCorrespondencePixels.size()),
        Graph::CGraphCtrl::PLOT_FIXED_AXIS);

    // the measured spectrum (on the secondary axis)
    this->m_graph.SetPlotColor(RGB(255, 0, 0));
    this->m_graph.Plot(
        m_controller->m_calibrationDebug.measuredSpectrum.data(),
        static_cast<int>(m_controller->m_calibrationDebug.measuredSpectrum.size()),
        Graph::CGraphCtrl::PLOT_CONNECTED | Graph::CGraphCtrl::PLOT_SECOND_AXIS);

    // the Fraunhofer spectrum
    this->m_graph.SetPlotColor(RGB(0, 255, 0));
    this->m_graph.Plot(
        m_controller->m_calibrationDebug.fraunhoferSpectrum.data(),
        static_cast<int>(m_controller->m_calibrationDebug.fraunhoferSpectrum.size()),
        Graph::CGraphCtrl::PLOT_SECOND_AXIS | Graph::CGraphCtrl::PLOT_CONNECTED);

    // The inlier correspondences
    for (size_t ii = 0; ii < m_controller->m_calibrationDebug.inlierCorrespondencePixels.size(); ++ii)
    {
        this->m_graph.DrawLine(
            m_controller->m_calibrationDebug.inlierCorrespondencePixels[ii],
            m_controller->m_calibrationDebug.inlierCorrespondencePixels[ii],
            m_controller->m_calibrationDebug.inlierCorrespondenceMeasuredIntensity[ii],
            m_controller->m_calibrationDebug.inlierCorrespondenceFraunhoferIntensity[ii],
            RGB(255, 255, 255),
            Graph::STYLE_SOLID, Graph::CGraphCtrl::PLOT_SECOND_AXIS);
    }
}

/// <summary>
/// This is the calibration background thread.
/// The calibration takes so much time that we don't want to have it running on the foreground thread - which would block the UI
/// </summary>
/// <param name="pParam">and instance of the WavelengthCalibrationController to run</param>
/// <return>Zero on successful calibration</return>
UINT RunCalibration(void* pParam)
{
    WavelengthCalibrationController* controller = static_cast<WavelengthCalibrationController*>(pParam);
    int returnValue = 0;

    try
    {
        controller->m_errorMessage = "";

        controller->RunCalibration();
    }
    catch (std::exception& e)
    {
        controller->m_errorMessage = e.what();

        returnValue = 1;
    }

    // Inform the user interface that the calibration is done.
    if (wavelengthCalibrationDialog != nullptr)
    {
        wavelengthCalibrationDialog->PostMessage(WM_DONE);
    }

    return returnValue;
}

CString runButtonOriginalText;

void CCalibratePixelToWavelengthDialog::OnClickedButtonRun()
{
    if (!IsExistingFile(m_inputSpectrumFile))
    {
        MessageBox("Please select a spectrum to calibrate", "Missing input", MB_OK);
        return;
    }
    if (!IsExistingFile(m_setup.m_solarSpectrumFile))
    {
        MessageBox("Please select a high resolved solar spectrum to use in the calibration", "Missing input", MB_OK);
        return;
    }
    if (!IsExistingFile(m_setup.m_initialCalibrationFile))
    {
        MessageBox("Please select a file which contains an initial guess for the wavelength calibration of the spectrometer", "Missing input", MB_OK);
        return;
    }

    this->m_controller->m_inputSpectrumFile = this->m_inputSpectrumFile;
    this->m_controller->m_solarSpectrumFile = this->m_setup.m_solarSpectrumFile;
    this->m_controller->m_initialCalibrationFile = this->m_setup.m_initialCalibrationFile;
    this->m_controller->m_initialLineShapeFile = this->m_setup.m_instrumentLineshapeFile;

    this->m_runButton.GetWindowTextA(runButtonOriginalText);

    try
    {
        this->m_runButton.SetWindowTextA("Calibrating...");
        this->m_runButton.EnableWindow(FALSE);
        this->m_saveButton.EnableWindow(FALSE);

        CCmdTarget::BeginWaitCursor();

        // Run the calibration in a background thread and wait for the calibration to finish (continues in OnCalibrationDone below)
        auto pSpecThread = AfxBeginThread(RunCalibration, (LPVOID)(this->m_controller), THREAD_PRIORITY_NORMAL, 0, 0, NULL);
    }
    catch (std::exception& e)
    {
        HandleCalibrationFailure(e.what());
        this->m_runButton.SetWindowTextA(runButtonOriginalText);
    }
}

LRESULT CCalibratePixelToWavelengthDialog::OnCalibrationDone(WPARAM wParam, LPARAM lParam)
{
    try
    {
        CCmdTarget::EndWaitCursor();

        if (this->m_controller->m_errorMessage.size() > 0)
        {
            HandleCalibrationFailure(m_controller->m_errorMessage.c_str());
            this->m_runButton.SetWindowTextA(runButtonOriginalText);
            return 0;
        }

        SaveSetup();

        UpdateGraph();

        this->m_saveButton.EnableWindow(TRUE);
        this->m_runButton.EnableWindow(TRUE);
        this->m_runButton.SetWindowTextA(runButtonOriginalText);
    }
    catch (std::exception& e)
    {
        HandleCalibrationFailure(e.what());
        this->m_runButton.SetWindowTextA(runButtonOriginalText);
    }

    return 0;
}

void CCalibratePixelToWavelengthDialog::HandleCalibrationFailure(const char* errorMessage)
{
    MessageBox(errorMessage, "Failed to calibrate", MB_OK);

    this->m_saveButton.EnableWindow(FALSE);
    this->m_runButton.EnableWindow(TRUE);
}

void CCalibratePixelToWavelengthDialog::OnClickedButtonSave()
{
    try
    {
        if (m_controller->m_measuredInstrumentLineShapeSpectrum == nullptr)
        {
            throw std::exception("Cannot find the instrument line shape");
        }

        // Save the instrument line shape and the pixel-to-wavelength calibration to file
        CString destinationFileName = "";
        int selectedType = 1;
        if (Common::BrowseForFile_SaveAs("Extended Standard Files\0*.std\0QDOAS Calibrations\0*.clb;*.slf", destinationFileName, &selectedType))
        {
            if (selectedType == 2)
            {
                std::string dstFileName = novac::EnsureFilenameHasSuffix(std::string(destinationFileName), "clb");
                m_controller->SaveResultAsClb(dstFileName);

                dstFileName = novac::EnsureFilenameHasSuffix(std::string(destinationFileName), "slf");
                m_controller->SaveResultAsSlf(dstFileName);
            }
            else
            {
                std::string dstFileName = novac::EnsureFilenameHasSuffix(std::string(destinationFileName), "std");
                m_controller->SaveResultAsStd(dstFileName);
            }
        }
    }
    catch (std::exception& e)
    {
        MessageBox(e.what(), "Failed to save pixel to wavelength mapping to file", MB_OK);
    }
}

void CCalibratePixelToWavelengthDialog::OnButtonSelectInitialCalibration()
{
    OpenInstrumentCalibrationDialog dlg;
    dlg.m_state.initialCalibrationFile = this->m_setup.m_initialCalibrationFile;
    dlg.m_state.instrumentLineshapeFile = this->m_setup.m_instrumentLineshapeFile;
    dlg.m_state.calibrationOption = (InstrumentCalibrationInputOption)this->m_setup.m_calibrationOption;

    if (IDOK == dlg.DoModal())
    {
        this->m_setup.m_initialCalibrationFile = dlg.m_state.initialCalibrationFile;
        this->m_setup.m_instrumentLineshapeFile = dlg.m_state.instrumentLineshapeFile;

        UpdateData(FALSE);
    }
}
