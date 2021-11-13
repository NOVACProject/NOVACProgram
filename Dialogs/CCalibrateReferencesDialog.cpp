// CCalibrateReferenes.cpp : implementation file
//

#include "StdAfx.h"
#include "CCalibrateReferencesDialog.h"
#include "afxdialogex.h"
#include "../resource.h"
#include <fstream>
#include "../Common/Common.h"
#include "../Calibration/ReferenceCreationController.h"
#include "OpenInstrumentCalibrationDialog.h"
#include "CCreateStandardReferencesDialog.h"
#include <SpectralEvaluation/Evaluation/CrossSectionData.h>
#include <SpectralEvaluation/File/File.h>
#include <SpectralEvaluation/File/XmlUtil.h>
#include <SpectralEvaluation/Calibration/StandardCrossSectionSetup.h>

// CCalibrateReferenes dialog

IMPLEMENT_DYNAMIC(CCalibrateReferencesDialog, CPropertyPage)

CCalibrateReferencesDialog::CCalibrateReferencesDialog(CWnd* pParent /*=nullptr*/)
    : CPropertyPage(IDD_CALIBRATE_REFERENCES)
    , m_highPassFilterReference(TRUE)
    , m_calibrationFile(_T(""))
    , m_inputInVacuum(TRUE)
    , m_standardCrossSections(nullptr)
{
    m_controller = new ReferenceCreationController();
}

CCalibrateReferencesDialog::~CCalibrateReferencesDialog()
{
    delete m_controller;
    delete m_standardCrossSections;
}

BOOL CCalibrateReferencesDialog::OnInitDialog() {
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
    m_graph.SetYUnits("");
    m_graph.SetXUnits("Wavelength");
    m_graph.SetBackgroundColor(RGB(0, 0, 0));
    m_graph.SetGridColor(RGB(255, 255, 255));
    m_graph.SetPlotColor(RGB(255, 0, 0));
    m_graph.CleanPlot();

    // First load the default setup.
    LoadDefaultSetup();

    // Then load any references the user may have added.
    LoadLastSetup();

    return TRUE;  // return TRUE unless you set the focus to a control
}

void CCalibrateReferencesDialog::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_HIGH_RES_CROSSSECTION, m_crossSectionsCombo);
    DDX_Check(pDX, IDC_CHECK_HIGH_PASS_FILTER, m_highPassFilterReference);
    DDX_Text(pDX, IDC_EDIT_CALIBRATION, m_calibrationFile);
    DDX_Control(pDX, IDC_STATIC_GRAPH_HOLDER2, m_graphHolder);
    DDX_Check(pDX, IDC_CHECK_INPUT_IN_VACUUM, m_inputInVacuum);
    DDX_Control(pDX, IDC_BUTTON_SAVE, m_saveButton);
    DDX_Control(pDX, IDC_BUTTON_CREATE_STANDARD_REFERENCES, m_createStandardReferencesButton);
}

BEGIN_MESSAGE_MAP(CCalibrateReferencesDialog, CPropertyPage)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_SOLAR_SPECTRUM, &CCalibrateReferencesDialog::OnClickedBrowseCrossSection)
    ON_CBN_SELCHANGE(IDC_COMBO_HIGH_RES_CROSSSECTION, &CCalibrateReferencesDialog::OnConvolutionOptionChanged)
    ON_BN_CLICKED(IDC_CHECK_HIGH_PASS_FILTER, &CCalibrateReferencesDialog::OnConvolutionOptionChanged)
    ON_BN_CLICKED(IDC_CHECK_INPUT_IN_VACUUM, &CCalibrateReferencesDialog::OnConvolutionOptionChanged)
    ON_BN_CLICKED(IDC_BUTTON_RUN_CREATE_REFERENCE, &CCalibrateReferencesDialog::OnClickedButtonRunCreateReference)
    ON_BN_CLICKED(IDC_BUTTON_SAVE, &CCalibrateReferencesDialog::OnClickedButtonSave)
    ON_BN_CLICKED(IDC_BUTTON_SELECT_CALIBRATION, &CCalibrateReferencesDialog::OnClickedButtonSelectCalibration)
    ON_BN_CLICKED(IDC_BUTTON_CREATE_STANDARD_REFERENCES, &CCalibrateReferencesDialog::OnClickedButtonCreateStandardReferences)
END_MESSAGE_MAP()

std::string CCalibrateReferencesDialog::SetupFilePath()
{
    Common common;
    common.GetExePath();
    CString path;
    path.Format("%sCalibrateReferencesDlg.config", common.m_exePath);
    return std::string(path);
}

void CCalibrateReferencesDialog::SaveSetup()
{
    try
    {
        std::ofstream dst(SetupFilePath(), std::ios::out);
        dst << "<CalibrateReferencesDlg>" << std::endl;

        for (int ii = 0; ii < m_crossSectionsCombo.GetCount(); ++ii)
        {
            CString filePath;
            m_crossSectionsCombo.GetLBText(ii, filePath);

            if (filePath.Find(" [Standard]") < 0)
            {
                dst << "\t<CrossSection>" << filePath << "</CrossSection>" << std::endl;
            }
        }

        dst << "</CalibrateReferencesDlg>" << std::endl;
    }
    catch (std::exception&)
    {
    }
}

void CCalibrateReferencesDialog::LoadLastSetup()
{
    try
    {
        // Super basic xml parsing
        std::ifstream file(SetupFilePath(), std::ios::in);
        std::string line;
        while (std::getline(file, line))
        {
            if (line.find("CrossSection") != std::string::npos)
            {
                auto str = novac::ParseXmlString("CrossSection", line);
                CString crossSectionFile(str.c_str());

                if (IsExistingFile(crossSectionFile))
                {
                    m_crossSectionsCombo.AddString(crossSectionFile);
                }
            }
        }

        if (m_crossSectionsCombo.GetCount() > 0)
        {
            m_crossSectionsCombo.SetCurSel(0);
        }
    }
    catch (std::exception&)
    {
    }
}

void CCalibrateReferencesDialog::LoadDefaultSetup()
{
    Common common;
    common.GetExePath();

    m_crossSectionsCombo.Clear();

    std::string exePath = common.m_exePath;
    m_standardCrossSections = new novac::StandardCrossSectionSetup{ exePath };

    auto allCrossSections = m_standardCrossSections->ListReferences();
    for (const std::string& crossSectionFile : allCrossSections)
    {
        CString fileName{ crossSectionFile.c_str() };
        fileName.Append(" [Standard]");
        m_crossSectionsCombo.AddString(fileName);
    }

    if (m_crossSectionsCombo.GetCount() > 0)
    {
        m_crossSectionsCombo.SetCurSel(0);
    }
}

// CCalibrateReferenes message handlers

void CCalibrateReferencesDialog::OnClickedBrowseCrossSection()
{
    CString crossSectionFile;
    if (!Common::BrowseForFile("Spectrum Files\0*.txt;*.xs\0", crossSectionFile))
    {
        return;
    }

    int elementIdx = m_crossSectionsCombo.AddString(crossSectionFile);
    m_crossSectionsCombo.SetCurSel(elementIdx);

    UpdateData(FALSE); // Update the UI
    UpdateReference();
}

void CCalibrateReferencesDialog::OnConvolutionOptionChanged()
{
    UpdateReference();
}

void CCalibrateReferencesDialog::OnClickedButtonRunCreateReference()
{
    UpdateReference();
}

void CCalibrateReferencesDialog::OnClickedButtonSave()
{
    try
    {
        CString destinationFileName = "";
        if (Common::BrowseForFile_SaveAs("Reference Files\0*.txt\0", destinationFileName))
        {
            std::string dstFileName = novac::EnsureFilenameHasSuffix(std::string(destinationFileName), "txt");
            novac::SaveCrossSectionFile(dstFileName, *(m_controller->m_resultingCrossSection));
        }
    }
    catch (std::exception& e)
    {
        MessageBox(e.what(), "Failed to save cross section file", MB_OK);
    }
}

void CCalibrateReferencesDialog::UpdateReference()
{
    try
    {
        UpdateData(TRUE); // get the selections from the user interface

        if (m_calibrationFile.IsEmpty() ||
            m_crossSectionsCombo.GetCurSel() < 0)
        {
            return;
        }
        if (!IsExistingFile(m_calibrationFile))
        {
            MessageBox("Please select an existing wavelength calibration file", "Missing input", MB_OK);
            return;
        }

        int selectedReferenceIdx = m_crossSectionsCombo.GetCurSel();

        CString crossSectionFilePath;
        if (m_standardCrossSections != nullptr && selectedReferenceIdx < m_standardCrossSections->NumberOfReferences())
        {
            const std::string path = m_standardCrossSections->ReferenceFileName(selectedReferenceIdx);
            crossSectionFilePath.Format("%s", path.c_str());
        }
        else
        {
            m_crossSectionsCombo.GetLBText(selectedReferenceIdx, crossSectionFilePath);
        }

        if (!IsExistingFile(crossSectionFilePath))
        {
            MessageBox("Please select an existing high resolved cross section file", "Missing input", MB_OK);
            return;
        }

        m_controller->m_calibrationFile = m_calibrationFile;
        m_controller->m_instrumentLineshapeFile = m_instrumentLineshapeFile;
        m_controller->m_highPassFilter = m_highPassFilterReference;
        m_controller->m_convertToAir = m_inputInVacuum;
        m_controller->m_highResolutionCrossSection = crossSectionFilePath;
        m_controller->ConvolveReference();

        UpdateGraph();

        SaveSetup();

        m_saveButton.EnableWindow(TRUE);
    }
    catch (std::exception& e)
    {
        MessageBox(e.what(), "Failed to convolve reference.", MB_OK);

        UpdateGraph();

        m_saveButton.EnableWindow(FALSE);
        m_createStandardReferencesButton.EnableWindow(FALSE);
    }
}

void CCalibrateReferencesDialog::UpdateGraph()
{
    m_graph.CleanPlot();

    // the reference
    if (m_controller->m_resultingCrossSection != nullptr && m_controller->m_resultingCrossSection->GetSize() > 0)
    {
        m_graph.SetPlotColor(RGB(255, 0, 0));
        m_graph.XYPlot(
            m_controller->m_resultingCrossSection->m_waveLength.data(),
            m_controller->m_resultingCrossSection->m_crossSection.data(),
            static_cast<int>(m_controller->m_resultingCrossSection->GetSize()),
            Graph::CGraphCtrl::PLOT_CONNECTED);
    }
}

void CCalibrateReferencesDialog::OnClickedButtonSelectCalibration()
{
    OpenInstrumentCalibrationDialog dlg;
    dlg.m_state.initialCalibrationFile = m_calibrationFile;
    dlg.m_state.instrumentLineshapeFile = m_instrumentLineshapeFile;

    if (IDOK == dlg.DoModal())
    {
        m_calibrationFile = dlg.m_state.initialCalibrationFile;
        m_instrumentLineshapeFile = dlg.m_state.instrumentLineshapeFile;

        UpdateData(FALSE);
        UpdateReference();

        if (m_standardCrossSections != nullptr &&
            m_standardCrossSections->NumberOfReferences() > 0)
        {
            m_createStandardReferencesButton.EnableWindow(TRUE);
        }
    }
}

void CCalibrateReferencesDialog::OnClickedButtonCreateStandardReferences()
{
    if (m_standardCrossSections == nullptr || m_standardCrossSections->NumberOfReferences() == 0)
    {
        MessageBox("Failed to load standard references", "Missing data", MB_OK);
        m_createStandardReferencesButton.EnableWindow(FALSE);
        return;
    }

    CCreateStandardReferencesDialog userInputDialog;
    userInputDialog.m_standardCrossSections = m_standardCrossSections;
    userInputDialog.m_fileNameFilteringInfix = (m_highPassFilterReference) ? "_HP500" : "";
    if (IDOK != userInputDialog.DoModal())
    {
        return;
    }

    for (size_t ii = 0; ii < m_standardCrossSections->NumberOfReferences(); ++ii)
    {
        try
        {
            // Options
            m_inputInVacuum = m_standardCrossSections->IsReferenceInVacuum(ii);
            UpdateData(FALSE);

            // Do the convolution
            m_controller->m_calibrationFile = m_calibrationFile;
            m_controller->m_instrumentLineshapeFile = m_instrumentLineshapeFile;
            m_controller->m_highPassFilter = m_highPassFilterReference;
            m_controller->m_convertToAir = m_standardCrossSections->IsReferenceInVacuum(ii);
            m_controller->m_highResolutionCrossSection = m_standardCrossSections->ReferenceFileName(ii);
            m_controller->ConvolveReference();

            UpdateGraph();

            // Save the result
            std::string dstFileName = userInputDialog.ReferenceName(ii);
            novac::SaveCrossSectionFile(dstFileName, *(m_controller->m_resultingCrossSection));
        }
        catch (std::exception& e)
        {
            MessageBox(e.what(), "Failed to convolve reference.", MB_OK);

            UpdateGraph();

            m_saveButton.EnableWindow(FALSE);
            m_createStandardReferencesButton.EnableWindow(FALSE);
        }
    }
}
