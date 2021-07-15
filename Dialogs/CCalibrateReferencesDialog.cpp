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
#include <SpectralEvaluation/Evaluation/CrossSectionData.h>
#include <SpectralEvaluation/File/File.h>

// CCalibrateReferenes dialog

IMPLEMENT_DYNAMIC(CCalibrateReferencesDialog, CPropertyPage)

CCalibrateReferencesDialog::CCalibrateReferencesDialog(CWnd* pParent /*=nullptr*/)
    : CPropertyPage(IDD_CALIBRATE_REFERENCES)
    , m_highPassFilterReference(TRUE)
    , m_calibrationFile(_T(""))
    , m_inputInVacuum(TRUE)
{
    this->m_controller = new ReferenceCreationController();
}

CCalibrateReferencesDialog::~CCalibrateReferencesDialog()
{
    delete this->m_controller;
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
}

BEGIN_MESSAGE_MAP(CCalibrateReferencesDialog, CPropertyPage)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_SOLAR_SPECTRUM, &CCalibrateReferencesDialog::OnBnClickedBrowseCrossSection)
    ON_CBN_SELCHANGE(IDC_COMBO_HIGH_RES_CROSSSECTION, &CCalibrateReferencesDialog::OnConvolutionOptionChanged)
    ON_BN_CLICKED(IDC_CHECK_HIGH_PASS_FILTER, &CCalibrateReferencesDialog::OnConvolutionOptionChanged)
    ON_BN_CLICKED(IDC_CHECK_INPUT_IN_VACUUM, &CCalibrateReferencesDialog::OnConvolutionOptionChanged)
    ON_BN_CLICKED(IDC_BUTTON_RUN_CREATE_REFERENCE, &CCalibrateReferencesDialog::OnBnClickedButtonRunCreateReference)
    ON_BN_CLICKED(IDC_BUTTON_SAVE, &CCalibrateReferencesDialog::OnClickedButtonSave)
    ON_BN_CLICKED(IDC_BUTTON_SELECT_CALIBRATION, &CCalibrateReferencesDialog::OnButtonSelectCalibration)
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
        std::ofstream dst(this->SetupFilePath(), std::ios::out);
        dst << "<CalibrateReferencesDlg>" << std::endl;

        for (int ii = 0; ii < this->m_crossSectionsCombo.GetCount(); ++ii)
        {
            CString filePath;
            this->m_crossSectionsCombo.GetLBText(ii, filePath);
            dst << "\t<CrossSection>" << filePath << "</CrossSection>" << std::endl;
        }

        dst << "</CalibrateReferencesDlg>" << std::endl;
    }
    catch (std::exception&)
    {
    }
}

CString ParseXmlString(const char* startTag, const char* stopTag, const std::string& line);

void CCalibrateReferencesDialog::LoadLastSetup()
{
    try
    {
        // Super basic xml parsing
        std::ifstream file(this->SetupFilePath(), std::ios::in);
        std::string line;
        while (std::getline(file, line))
        {
            if (line.find("CrossSection") != std::string::npos)
            {
                CString crossSectionFile = ParseXmlString("<CrossSection>", "</CrossSection>", line);

                if (IsExistingFile(crossSectionFile))
                {
                    this->m_crossSectionsCombo.AddString(crossSectionFile);
                }
            }
        }

        if (this->m_crossSectionsCombo.GetCount() > 0)
        {
            this->m_crossSectionsCombo.SetCurSel(0);
        }
    }
    catch (std::exception&)
    {
    }

    // If this is the first time we used this dialog, then just use the default
    if (this->m_crossSectionsCombo.GetCount() == 0)
    {
        this->LoadDefaultSetup();
    }
}

void CCalibrateReferencesDialog::LoadDefaultSetup()
{
    Common common;
    common.GetExePath();

    // See if there is any possible references in the current directory already
    const auto allCrossSections = Common::ListFilesInDirectory(common.m_exePath, "*.xs");

    for (const std::string& crossSectionFile : allCrossSections)
    {
        CString fileName{ crossSectionFile.c_str() };
        this->m_crossSectionsCombo.AddString(fileName);
    }

    if (this->m_crossSectionsCombo.GetCount() > 0)
    {
        this->m_crossSectionsCombo.SetCurSel(0);
    }
}

// CCalibrateReferenes message handlers

void CCalibrateReferencesDialog::OnBnClickedBrowseCrossSection()
{
    CString crossSectionFile;
    if (!Common::BrowseForFile("Spectrum Files\0*.txt;*.xs\0", crossSectionFile))
    {
        return;
    }

    int elementIdx = this->m_crossSectionsCombo.AddString(crossSectionFile);
    this->m_crossSectionsCombo.SetCurSel(elementIdx);

    UpdateData(FALSE);
    UpdateReference();
}

void CCalibrateReferencesDialog::OnConvolutionOptionChanged()
{
    UpdateReference();
}

void CCalibrateReferencesDialog::OnBnClickedButtonRunCreateReference()
{
    UpdateReference();
}

void CCalibrateReferencesDialog::OnClickedButtonSave()
{
    try
    {
        CString destinationFileName = "";
        if (Common::BrowseForFile_SaveAs("Reference Files\0*.xs\0", destinationFileName))
        {
            std::string dstFileName = novac::EnsureFilenameHasSuffix(std::string(destinationFileName), "xs");
            novac::SaveCrossSectionFile(dstFileName, *(this->m_controller->m_resultingCrossSection));
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

        if (this->m_calibrationFile.IsEmpty() ||
            this->m_crossSectionsCombo.GetCurSel() < 0)
        {
            return;
        }
        if (!IsExistingFile(this->m_calibrationFile))
        {
            MessageBox("Please select an existing wavelength calibration file", "Missing input", MB_OK);
            return;
        }

        int selectedReferenceIdx = this->m_crossSectionsCombo.GetCurSel();
        CString crossSectionFilePath;
        this->m_crossSectionsCombo.GetLBText(selectedReferenceIdx, crossSectionFilePath);

        if (!IsExistingFile(crossSectionFilePath))
        {
            MessageBox("Please select an existing high resolved cross section file", "Missing input", MB_OK);
            return;
        }

        this->m_controller->m_calibrationFile = this->m_calibrationFile;
        this->m_controller->m_instrumentLineshapeFile = this->m_instrumentLineshapeFile;
        this->m_controller->m_highPassFilter = this->m_highPassFilterReference;
        this->m_controller->m_convertToAir = this->m_inputInVacuum;
        this->m_controller->m_highResolutionCrossSection = crossSectionFilePath;
        this->m_controller->ConvolveReference();

        this->UpdateGraph();

        this->SaveSetup();

        this->m_saveButton.EnableWindow(TRUE);
    }
    catch (std::exception& e)
    {
        MessageBox(e.what(), "Failed to convolve reference.", MB_OK);

        this->UpdateGraph();

        this->m_saveButton.EnableWindow(FALSE);
    }
}

void CCalibrateReferencesDialog::UpdateGraph()
{
    this->m_graph.CleanPlot();

    // the reference
    if (this->m_controller->m_resultingCrossSection != nullptr && m_controller->m_resultingCrossSection->GetSize() > 0)
    {
        this->m_graph.SetPlotColor(RGB(255, 0, 0));
        this->m_graph.XYPlot(
            m_controller->m_resultingCrossSection->m_waveLength.data(),
            m_controller->m_resultingCrossSection->m_crossSection.data(),
            static_cast<int>(m_controller->m_resultingCrossSection->GetSize()),
            Graph::CGraphCtrl::PLOT_CONNECTED);
    }
}

void CCalibrateReferencesDialog::OnButtonSelectCalibration()
{
    OpenInstrumentCalibrationDialog dlg;
    dlg.m_state.initialCalibrationFile = this->m_calibrationFile;
    dlg.m_state.instrumentLineshapeFile = this->m_instrumentLineshapeFile;

    if (IDOK == dlg.DoModal())
    {
        this->m_calibrationFile = dlg.m_state.initialCalibrationFile;
        this->m_instrumentLineshapeFile = dlg.m_state.instrumentLineshapeFile;

        UpdateData(FALSE);
        UpdateReference();
    }
}
