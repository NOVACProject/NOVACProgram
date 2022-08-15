// CCalibratePixelToWavelengthDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "CCalibratePixelToWavelengthDialog.h"
#include "afxdialogex.h"
#include "../resource.h"
#include "../Common/Common.h"
#include <SpectralEvaluation/DialogControllers/NovacProgramWavelengthCalibrationController.h>
#include <SpectralEvaluation/DialogControllers/ReferenceCreationController.h>
#include "OpenInstrumentCalibrationDialog.h"
#include "CCalibratePixelToWavelengthSetupDialog.h"
#include "CCreateStandardReferencesDialog.h"
#include "CLogDialog.h"
#include <fstream>
#include <SpectralEvaluation/File/File.h>
#include <SpectralEvaluation/Evaluation/CrossSectionData.h>
#include <SpectralEvaluation/Calibration/InstrumentCalibration.h>
#include <SpectralEvaluation/Calibration/StandardCrossSectionSetup.h>
#include <SpectralEvaluation/File/XmlUtil.h>

// CCalibratePixelToWavelengthDialog dialog

// Pointer to the dialog itself. Used for callbacks
CDialog* wavelengthCalibrationDialog = nullptr;

IMPLEMENT_DYNAMIC(CCalibratePixelToWavelengthDialog, CPropertyPage)

CCalibratePixelToWavelengthDialog::CCalibratePixelToWavelengthDialog(CWnd* pParent /*=nullptr*/)
    : CPropertyPage(IDD_CALIBRATE_WAVELENGTH_DIALOG)
    , m_inputSpectrumFile(_T(""))
    , m_standardCrossSections(nullptr)
    , m_initialCalibrationFileTypeFilter("Novac Instrument Calibration Files\0*.xml\0")
{
    wavelengthCalibrationDialog = this;

    m_controller = new NovacProgramWavelengthCalibrationController();
}

CCalibratePixelToWavelengthDialog::~CCalibratePixelToWavelengthDialog()
{
    delete m_controller;
    delete m_standardCrossSections;
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
    m_graphTypeList.AddString("Instrument Line Shape");
    m_graphTypeList.SetCurSel(0);

    UpdateGreenLegend(false);
    UpdateRedLegend(false);

    LoadDefaultSetup();
    LoadLastSetup();
    UpdateData(FALSE);

    return TRUE;  // return TRUE unless you set the focus to a control
}

void CCalibratePixelToWavelengthDialog::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_SPECTRUM, m_inputSpectrumFile);
    DDX_Control(pDX, IDC_STATIC_GRAPH_HOLDER_PANEL, m_graphHolder);
    DDX_Control(pDX, IDC_BUTTON_RUN, m_runButton);
    DDX_Control(pDX, IDC_BUTTON_SAVE, m_saveCalibrationButton);
    DDX_Control(pDX, IDC_BUTTON_SAVE_REFERENCES, m_saveReferencesButton);
    DDX_Control(pDX, IDC_LIST_GRAPH_TYPE, m_graphTypeList);
    DDX_Control(pDX, IDC_WAVELENGTH_CALIBRATION_DETAILS_LIST, m_detailedResultList);
    DDX_Control(pDX, IDC_BUTTON_VIEW_LOG, m_viewLogButton);
    DDX_Control(pDX, IDC_STATIC_LEGEND_GREEN, m_greenLegendLabel);
    DDX_Control(pDX, IDC_STATIC_LEGEND_RED, m_redLegendLabel);
    DDX_Control(pDX, IDC_STATIC_RED, m_redLegendIcon);
    DDX_Control(pDX, IDC_STATIC_GREEN, m_greenLegendIcon);
}

BEGIN_MESSAGE_MAP(CCalibratePixelToWavelengthDialog, CPropertyPage)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_SPECTRUM, &CCalibratePixelToWavelengthDialog::OnClickedButtonBrowseSpectrum)
    ON_BN_CLICKED(IDC_BUTTON_RUN, &CCalibratePixelToWavelengthDialog::OnClickedButtonRun)
    ON_BN_CLICKED(IDC_BUTTON_SAVE, &CCalibratePixelToWavelengthDialog::OnClickedButtonSaveCalibration)
    ON_BN_CLICKED(IDC_BUTTON_SAVE_REFERENCES, &CCalibratePixelToWavelengthDialog::OnClickedButtonSaveReferences)

    ON_MESSAGE(WM_DONE, OnCalibrationDone)
    ON_LBN_SELCHANGE(IDC_LIST_GRAPH_TYPE, &CCalibratePixelToWavelengthDialog::OnSelchangeListGraphType)
    ON_BN_CLICKED(IDC_BUTTON_SETUP_WAVELENGTH_CALIBRATION, &CCalibratePixelToWavelengthDialog::OnBnClickedSetupWavelengthCalibration)
    ON_BN_CLICKED(IDC_BUTTON_VIEW_LOG, &CCalibratePixelToWavelengthDialog::OnBnClickedButtonViewLog)
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
        std::ofstream dst(SetupFilePath(), std::ios::out);
        dst << "<CalibrateWavelengthDlg>" << std::endl;
        dst << "\t<SolarSpectrum>" << m_setup.m_solarSpectrumFile << "</SolarSpectrum>" << std::endl;
        dst << "\t<InitialCalibrationFile>" << m_setup.m_initialCalibrationFile << "</InitialCalibrationFile>" << std::endl;
        dst << "\t<InputFileType>" << (int)m_setup.m_calibrationOption << "</InputFileType>" << std::endl;
        dst << "\t<InstrumentLineShapeFitType>" << (int)m_setup.m_fitInstrumentLineShapeOption << "</InstrumentLineShapeFitType>" << std::endl;
        dst << "\t<InstrumentLineShapeFitFrom>" << m_setup.m_fitInstrumentLineShapeRegionStart << "</InstrumentLineShapeFitFrom>" << std::endl;
        dst << "\t<InstrumentLineShapeFitTo>" << m_setup.m_fitInstrumentLineShapeRegionStop << "</InstrumentLineShapeFitTo>" << std::endl;
        dst << "</CalibrateWavelengthDlg>" << std::endl;
    }
    catch (std::exception&)
    {
    }
}

void CCalibratePixelToWavelengthDialog::LoadDefaultSetup()
{
    Common common;
    common.GetExePath();

    // See if there a Fraunhofer reference in the standard cross section setup.
    std::string exePath = common.m_exePath;
    m_standardCrossSections = new novac::StandardCrossSectionSetup{ exePath };

    const auto solarCrossSection = m_standardCrossSections->FraunhoferReferenceFileName();

    if (solarCrossSection.size() > 0)
    {
        m_setup.m_solarSpectrumFile = CString(solarCrossSection.c_str());
    }
}

void CCalibratePixelToWavelengthDialog::LoadLastSetup()
{
    try
    {
        // Super basic xml parsing
        std::ifstream file(SetupFilePath(), std::ios::in);
        std::string line;
        while (std::getline(file, line))
        {
            if (line.find("SolarSpectrum") != std::string::npos)
            {
                auto str = novac::ParseXmlString("SolarSpectrum", line);
                m_setup.m_solarSpectrumFile = CString(str.c_str());
            }
            else if (line.find("LineShapeFile") != std::string::npos)
            {
                auto str = novac::ParseXmlString("LineShapeFile", line);
                m_setup.m_instrumentLineshapeFile = CString(str.c_str());
            }
            else if (line.find("InputFileType") != std::string::npos)
            {
                m_setup.m_calibrationOption = novac::ParseXmlInteger("InputFileType", line, 0);
            }
            else if (line.find("InstrumentLineShapeFitType") != std::string::npos)
            {
                m_setup.m_fitInstrumentLineShapeOption = novac::ParseXmlInteger("InstrumentLineShapeFitType", line, 0);
            }
            else if (line.find("InstrumentLineShapeFitFrom") != std::string::npos)
            {
                auto str = novac::ParseXmlString("InstrumentLineShapeFitFrom", line);
                m_setup.m_fitInstrumentLineShapeRegionStart = CString(str.c_str());
            }
            else if (line.find("InstrumentLineShapeFitTo") != std::string::npos)
            {
                auto str = novac::ParseXmlString("InstrumentLineShapeFitTo", line);
                m_setup.m_fitInstrumentLineShapeRegionStop = CString(str.c_str());
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
    if (!Common::BrowseForFile("Pak Files\0*.pak\0", m_inputSpectrumFile))
    {
        return;
    }

    m_controller->ClearResult();
    UpdateData(FALSE);
    UpdateGraph();
}

void CCalibratePixelToWavelengthDialog::UpdateGraph()
{
    const int currentGraph = m_graphTypeList.GetCurSel();
    if (currentGraph == 4)
    {
        DrawFittedInstrumentLineShape();
    }
    else if (currentGraph == 3)
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

void CCalibratePixelToWavelengthDialog::UpdateResultList()
{
    m_detailedResultList.ResetContent(); // clears the list
    if (m_controller->m_resultingCalibration == nullptr)
    {
        return;
    }

    CString text;

    // The polynomial coefficients.
    text.Format("Wavelength polynomial order: %d", m_controller->m_resultingCalibration->pixelToWavelengthPolynomial.size() - 1);
    m_detailedResultList.AddString(text);

    for (size_t coefficientIdx = 0; coefficientIdx < m_controller->m_resultingCalibration->pixelToWavelengthPolynomial.size(); ++coefficientIdx)
    {
        text.Format("c%d: %.4g", coefficientIdx, m_controller->m_resultingCalibration->pixelToWavelengthPolynomial[coefficientIdx]);
        m_detailedResultList.AddString(text);
    }

    // The Instrument Line Shape:
    for (const auto& parameter : m_controller->m_instrumentLineShapeParameterDescriptions)
    {
        text.Format("%s: %s", parameter.first.c_str(), parameter.second.c_str());
        m_detailedResultList.AddString(text);
    }
}

void CCalibratePixelToWavelengthDialog::OnSelchangeListGraphType()
{
    UpdateGraph();
}

void CCalibratePixelToWavelengthDialog::DrawPolynomialAndInliers()
{
    m_graph.CleanPlot();

    m_graph.SetXUnits("Pixel");
    m_graph.SetYUnits("Wavelength [nm]");

    // the old (initial) calibration polynomial
    m_graph.SetPlotColor(RGB(0, 255, 0));
    m_graph.Plot(
        m_controller->m_calibrationDebug.initialPixelToWavelengthMapping.data(),
        static_cast<int>(m_controller->m_calibrationDebug.initialPixelToWavelengthMapping.size()),
        Graph::CGraphCtrl::PLOT_CONNECTED);

    UpdateGreenLegend(m_controller->m_calibrationDebug.initialPixelToWavelengthMapping.size() > 0,
        "Initial wavelength calibration");

    // the calibration polynomial
    if (m_controller->m_resultingCalibration != nullptr)
    {
        m_graph.SetPlotColor(RGB(255, 0, 0));
        m_graph.Plot(
            m_controller->m_resultingCalibration->pixelToWavelengthMapping.data(),
            static_cast<int>(m_controller->m_resultingCalibration->pixelToWavelengthMapping.size()),
            Graph::CGraphCtrl::PLOT_CONNECTED | Graph::CGraphCtrl::PLOT_FIXED_AXIS);
    }
    UpdateRedLegend(m_controller->m_resultingCalibration != nullptr && m_controller->m_resultingCalibration->pixelToWavelengthMapping.size() > 0,
        "Resulting wavelength calibration");

    // outliers
    m_graph.SetCircleColor(RGB(128, 128, 128));
    m_graph.DrawCircles(
        m_controller->m_calibrationDebug.outlierCorrespondencePixels.data(),
        m_controller->m_calibrationDebug.outlierCorrespondenceWavelengths.data(),
        static_cast<int>(m_controller->m_calibrationDebug.outlierCorrespondencePixels.size()),
        Graph::CGraphCtrl::PLOT_FIXED_AXIS);

    // inliers
    m_graph.SetCircleColor(RGB(255, 255, 255));
    m_graph.DrawCircles(
        m_controller->m_calibrationDebug.inlierCorrespondencePixels.data(),
        m_controller->m_calibrationDebug.inlierCorrespondenceWavelengths.data(),
        static_cast<int>(m_controller->m_calibrationDebug.inlierCorrespondencePixels.size()),
        Graph::CGraphCtrl::PLOT_FIXED_AXIS);
}

void CCalibratePixelToWavelengthDialog::DrawMeasuredSpectrumAndKeypoints()
{
    m_graph.CleanPlot();

    m_graph.SetXUnits("Pixel");
    m_graph.SetYUnits("");

    // the measured spectrum
    m_graph.SetPlotColor(RGB(255, 0, 0));
    m_graph.Plot(
        m_controller->m_calibrationDebug.measuredSpectrum.data(),
        static_cast<int>(m_controller->m_calibrationDebug.measuredSpectrum.size()),
        Graph::CGraphCtrl::PLOT_CONNECTED);

    // all the keypoints found
    m_graph.SetCircleColor(RGB(128, 128, 128));
    m_graph.DrawCircles(
        m_controller->m_calibrationDebug.measuredSpectrumKeypointPixels.data(),
        m_controller->m_calibrationDebug.measuredSpectrumKeypointIntensities.data(),
        static_cast<int>(m_controller->m_calibrationDebug.measuredSpectrumKeypointIntensities.size()),
        Graph::CGraphCtrl::PLOT_FIXED_AXIS);

    // all the keypoints used
    m_graph.SetCircleColor(RGB(255, 255, 255));
    m_graph.DrawCircles(
        m_controller->m_calibrationDebug.measuredSpectrumInlierKeypointPixels.data(),
        m_controller->m_calibrationDebug.measuredSpectrumInlierKeypointIntensities.data(),
        static_cast<int>(m_controller->m_calibrationDebug.measuredSpectrumInlierKeypointIntensities.size()),
        Graph::CGraphCtrl::PLOT_FIXED_AXIS);

    UpdateRedLegend(m_controller->m_calibrationDebug.measuredSpectrum.size() > 0, "Measured spectrum");
    UpdateGreenLegend(false);
}

void CCalibratePixelToWavelengthDialog::DrawFittedInstrumentLineShape()
{
    m_graph.CleanPlot();

    m_graph.SetXUnits("Delta Wavelength");
    m_graph.SetYUnits("");

    // The initial instrument line shape
    if (m_controller->m_initialCalibration != nullptr)
    {
        m_graph.SetPlotColor(RGB(255, 0, 0));
        m_graph.XYPlot(
            m_controller->m_initialCalibration->instrumentLineShapeGrid.data(),
            m_controller->m_initialCalibration->instrumentLineShape.data(),
            static_cast<int>(m_controller->m_initialCalibration->instrumentLineShape.size()),
            Graph::CGraphCtrl::PLOT_CONNECTED);
    }
    UpdateRedLegend(m_controller->m_initialCalibration != nullptr && m_controller->m_initialCalibration->instrumentLineShape.size() > 0,
        "Initial instrument line shape");

    // The fitted instrument line shape
    if (m_controller->m_resultingCalibration != nullptr)
    {
        m_graph.SetPlotColor(RGB(0, 255, 0));
        m_graph.XYPlot(
            m_controller->m_resultingCalibration->instrumentLineShapeGrid.data(),
            m_controller->m_resultingCalibration->instrumentLineShape.data(),
            static_cast<int>(m_controller->m_resultingCalibration->instrumentLineShape.size()),
            Graph::CGraphCtrl::PLOT_CONNECTED | Graph::CGraphCtrl::PLOT_FIXED_AXIS);
    }
    UpdateGreenLegend(m_controller->m_resultingCalibration != nullptr && m_controller->m_resultingCalibration->instrumentLineShape.size() > 0,
        "Resulting instrument line shape");
}

void CCalibratePixelToWavelengthDialog::DrawFraunhoferSpectrumAndKeypoints()
{
    m_graph.CleanPlot();

    m_graph.SetXUnits("Wavelength");
    m_graph.SetYUnits("");

    // the Fraunhofer spectrum
    if (m_controller->m_resultingCalibration != nullptr)
    {
        m_graph.SetPlotColor(RGB(0, 255, 0));
        m_graph.XYPlot(
            m_controller->m_resultingCalibration->pixelToWavelengthMapping.data(),
            m_controller->m_calibrationDebug.fraunhoferSpectrum.data(),
            static_cast<int>(m_controller->m_calibrationDebug.fraunhoferSpectrum.size()),
            Graph::CGraphCtrl::PLOT_CONNECTED);
    }
    UpdateGreenLegend(m_controller->m_resultingCalibration != nullptr && m_controller->m_calibrationDebug.fraunhoferSpectrum.size() > 0,
        "Fraunhofer spectrum");

    // all the keypoints found
    m_graph.SetCircleColor(RGB(128, 128, 128));
    m_graph.DrawCircles(
        m_controller->m_calibrationDebug.fraunhoferSpectrumKeypointWavelength.data(),
        m_controller->m_calibrationDebug.fraunhoferSpectrumKeypointIntensities.data(),
        static_cast<int>(m_controller->m_calibrationDebug.fraunhoferSpectrumKeypointIntensities.size()),
        Graph::CGraphCtrl::PLOT_FIXED_AXIS);

    // all the keypoints used
    m_graph.SetCircleColor(RGB(255, 255, 255));
    m_graph.DrawCircles(
        m_controller->m_calibrationDebug.fraunhoferSpectrumInlierKeypointWavelength.data(),
        m_controller->m_calibrationDebug.fraunhoferSpectrumInlierKeypointIntensities.data(),
        static_cast<int>(m_controller->m_calibrationDebug.fraunhoferSpectrumInlierKeypointIntensities.size()),
        Graph::CGraphCtrl::PLOT_FIXED_AXIS);

    UpdateRedLegend(false);
}

void CCalibratePixelToWavelengthDialog::DrawSpectraAndInliers()
{
    m_graph.CleanPlot();

    m_graph.SetXUnits("Pixel");
    m_graph.SetYUnits("Wavelength [nm]");
    m_graph.SetSecondRangeY(0, 1, 1, false);

    // the calibration polynomial
    if (m_controller->m_resultingCalibration)
    {
        m_graph.SetPlotColor(RGB(255, 0, 0));
        m_graph.Plot(
            m_controller->m_resultingCalibration->pixelToWavelengthMapping.data(),
            static_cast<int>(m_controller->m_resultingCalibration->pixelToWavelengthMapping.size()),
            Graph::CGraphCtrl::PLOT_CONNECTED);
    }

    // inliers
    m_graph.SetCircleColor(RGB(255, 255, 255));
    m_graph.DrawCircles(
        m_controller->m_calibrationDebug.inlierCorrespondencePixels.data(),
        m_controller->m_calibrationDebug.inlierCorrespondenceWavelengths.data(),
        static_cast<int>(m_controller->m_calibrationDebug.inlierCorrespondencePixels.size()),
        Graph::CGraphCtrl::PLOT_FIXED_AXIS);

    // the measured spectrum (on the secondary axis)
    m_graph.SetPlotColor(RGB(255, 0, 0));
    m_graph.Plot(
        m_controller->m_calibrationDebug.measuredSpectrum.data(),
        static_cast<int>(m_controller->m_calibrationDebug.measuredSpectrum.size()),
        Graph::CGraphCtrl::PLOT_CONNECTED | Graph::CGraphCtrl::PLOT_SECOND_AXIS);

    UpdateRedLegend(m_controller->m_calibrationDebug.measuredSpectrum.size() > 0,
        "Measured spectrum");

    // the Fraunhofer spectrum
    m_graph.SetPlotColor(RGB(0, 255, 0));
    m_graph.Plot(
        m_controller->m_calibrationDebug.fraunhoferSpectrum.data(),
        static_cast<int>(m_controller->m_calibrationDebug.fraunhoferSpectrum.size()),
        Graph::CGraphCtrl::PLOT_SECOND_AXIS | Graph::CGraphCtrl::PLOT_CONNECTED);

    UpdateGreenLegend(m_controller->m_calibrationDebug.fraunhoferSpectrum.size() > 0,
        "Fraunhofer spectrum");

    // The inlier correspondences
    for (size_t ii = 0; ii < m_controller->m_calibrationDebug.inlierCorrespondencePixels.size(); ++ii)
    {
        m_graph.DrawLine(
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
    if (m_setup.m_fitInstrumentLineShapeOzoneReference.GetLength() > 3 &&
        !IsExistingFile(m_setup.m_fitInstrumentLineShapeOzoneReference))
    {
        MessageBox("Failed to find the provided ozone reference file to be used when deriving the instrument line shape.", "Missing input", MB_OK);
        return;
    }

    m_controller->m_inputSpectrumFile = m_inputSpectrumFile;
    m_controller->m_solarSpectrumFile = m_setup.m_solarSpectrumFile;
    m_controller->m_initialCalibrationFile = m_setup.m_initialCalibrationFile;
    m_controller->m_initialLineShapeFile = m_setup.m_instrumentLineshapeFile;
    m_controller->m_instrumentLineShapeFitOption = (WavelengthCalibrationController::InstrumentLineShapeFitOption)m_setup.m_fitInstrumentLineShapeOption;
    m_controller->m_instrumentLineShapeFitRegion = novac::WavelengthRange(
        std::atof(m_setup.m_fitInstrumentLineShapeRegionStart),
        std::atof(m_setup.m_fitInstrumentLineShapeRegionStop));
    m_controller->m_crossSectionsForInstrumentLineShapeFitting = m_setup.m_fitInstrumentLineShapeOzoneReference;

    m_runButton.GetWindowTextA(runButtonOriginalText);

    try
    {
        m_runButton.SetWindowTextA("Calibrating...");
        m_runButton.EnableWindow(FALSE);
        m_saveCalibrationButton.EnableWindow(FALSE);
        m_saveReferencesButton.EnableWindow(FALSE);

        CCmdTarget::BeginWaitCursor();

        // Run the calibration in a background thread and wait for the calibration to finish (continues in OnCalibrationDone below)
        auto pSpecThread = AfxBeginThread(RunCalibration, (LPVOID)(m_controller), THREAD_PRIORITY_NORMAL, 0, 0, NULL);
    }
    catch (std::exception& e)
    {
        HandleCalibrationFailure(e.what());
        m_runButton.SetWindowTextA(runButtonOriginalText);
    }
}

LRESULT CCalibratePixelToWavelengthDialog::OnCalibrationDone(WPARAM wParam, LPARAM lParam)
{
    try
    {
        CCmdTarget::EndWaitCursor();

        if (m_controller->m_errorMessage.size() > 0)
        {
            HandleCalibrationFailure(m_controller->m_errorMessage.c_str());
            m_runButton.SetWindowTextA(runButtonOriginalText);
            return 0;
        }

        SaveSetup();

        UpdateGraph();

        UpdateResultList();

        m_saveCalibrationButton.EnableWindow(TRUE);
        m_saveReferencesButton.EnableWindow(TRUE);
        m_runButton.EnableWindow(TRUE);
        m_runButton.SetWindowTextA(runButtonOriginalText);
    }
    catch (std::exception& e)
    {
        HandleCalibrationFailure(e.what());
        m_runButton.SetWindowTextA(runButtonOriginalText);
    }

    m_viewLogButton.EnableWindow(m_controller->m_log.size() > 0);

    return 0;
}

void CCalibratePixelToWavelengthDialog::HandleCalibrationFailure(const char* errorMessage)
{
    MessageBox(errorMessage, "Failed to calibrate", MB_OK);

    m_saveCalibrationButton.EnableWindow(FALSE);
    m_saveReferencesButton.EnableWindow(FALSE);
    m_runButton.EnableWindow(TRUE);
    m_viewLogButton.EnableWindow(m_controller->m_log.size() > 0);

    SaveSetup();
    UpdateGraph();
}

void CCalibratePixelToWavelengthDialog::OnClickedButtonSaveReferences()
{
    if (m_standardCrossSections == nullptr || m_standardCrossSections->NumberOfReferences() == 0)
    {
        MessageBox("Failed to load standard references", "Missing data", MB_OK);
        m_saveReferencesButton.EnableWindow(FALSE);
        return;
    }

    const bool highPassFilterReference = true;

    CCreateStandardReferencesDialog userInputDialog;
    userInputDialog.m_standardCrossSections = m_standardCrossSections;
    userInputDialog.m_fileNameFilteringInfix = "_HP500_PPMM";

    if (IDOK != userInputDialog.DoModal())
    {
        return;
    }

    try
    {
        ReferenceCreationController referenceController;
        referenceController.m_highPassFilter = highPassFilterReference;
        referenceController.m_unitSelection = 0; // default to ppmm

        const auto calibration = m_controller->GetFinalCalibration();

        // First the ordinary references
        for (size_t ii = 0; ii < m_standardCrossSections->NumberOfReferences(); ++ii)
        {
            // Do the convolution
            referenceController.m_convertToAir = m_standardCrossSections->IsReferenceInVacuum(ii);
            referenceController.m_highResolutionCrossSection = m_standardCrossSections->ReferenceFileName(ii);
            referenceController.m_isPseudoAbsorber = m_standardCrossSections->IsAdditionalAbsorber(ii);
            referenceController.ConvolveReference(*calibration);

            // Save the result
            std::string dstFileName = userInputDialog.ReferenceName(ii, m_standardCrossSections->IsAdditionalAbsorber(ii));
            novac::SaveCrossSectionFile(dstFileName, *(referenceController.m_resultingCrossSection));
        }

        // Save the Fraunhofer reference as well
        {
            // Do the convolution
            referenceController.m_highPassFilter = false;
            referenceController.m_convertToAir = false;
            referenceController.m_highResolutionCrossSection = m_standardCrossSections->FraunhoferReferenceFileName();
            referenceController.m_isPseudoAbsorber = true;
            referenceController.ConvolveReference(*calibration);

            // Save the result
            std::string dstFileName = userInputDialog.FraunhoferReferenceName();
            novac::SaveCrossSectionFile(dstFileName, *(referenceController.m_resultingCrossSection));
        }
    }
    catch (std::exception& e)
    {
        MessageBox(e.what(), "Failed to convolve reference.", MB_OK);

        UpdateGraph();

        m_saveReferencesButton.EnableWindow(FALSE);
    }
}

void CCalibratePixelToWavelengthDialog::OnClickedButtonSaveCalibration()
{
    try
    {
        if (m_controller->m_resultingCalibration == nullptr)
        {
            throw std::exception("Cannot find the fitted result");
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

void CCalibratePixelToWavelengthDialog::OnBnClickedSetupWavelengthCalibration()
{
    CCalibratePixelToWavelengthSetupDialog setupDlg{ &m_setup, m_standardCrossSections };
    setupDlg.DoModal();
}

void CCalibratePixelToWavelengthDialog::OnBnClickedButtonViewLog()
{
    CLogDialog logDialog{ m_controller->m_log };
    logDialog.DoModal();
}

void CCalibratePixelToWavelengthDialog::UpdateGreenLegend(bool show, const char* message)
{
    if (show && message != nullptr)
    {
        m_greenLegendIcon.ShowWindow(SW_SHOW);
        m_greenLegendLabel.ShowWindow(SW_SHOW);
        m_greenLegendLabel.SetWindowTextA(message);
    }
    else
    {
        m_greenLegendIcon.ShowWindow(SW_HIDE);
        m_greenLegendLabel.ShowWindow(SW_HIDE);
    }
}

void CCalibratePixelToWavelengthDialog::UpdateRedLegend(bool show, const char* message)
{
    if (show && message != nullptr)
    {
        m_redLegendIcon.ShowWindow(SW_SHOW);
        m_redLegendLabel.ShowWindow(SW_SHOW);
        m_redLegendLabel.SetWindowTextA(message);
    }
    else
    {
        m_redLegendIcon.ShowWindow(SW_HIDE);
        m_redLegendLabel.ShowWindow(SW_HIDE);
    }
}

