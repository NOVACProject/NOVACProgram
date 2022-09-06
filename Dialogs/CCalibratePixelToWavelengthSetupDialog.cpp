// CCalibratePixelToWavelengthSetupDialog.cpp : implementation file
//

#include "stdafx.h"
#include "CCalibratePixelToWavelengthSetupDialog.h"
#include "CCalibratePixelToWavelengthDialog.h"
#include "OpenInstrumentCalibrationDialog.h"
#include "../Common/Common.h"
#include "afxdlgs.h"
#include "../resource.h"
#include <SpectralEvaluation/Calibration/StandardCrossSectionSetup.h>

// CCalibratePixelToWavelengthSetupDialog dialog

IMPLEMENT_DYNAMIC(CCalibratePixelToWavelengthSetupDialog, CDialog)

CCalibratePixelToWavelengthSetupDialog::CCalibratePixelToWavelengthSetupDialog(
    CalibratePixelToWavelengthDialogSetup* setup,
    novac::StandardCrossSectionSetup* crossSections,
    CWnd* pParent /*=nullptr*/)
    : m_setup(setup), m_standardCrossSections(crossSections), CDialog(IDD_CALIBRATE_WAVELENGTH_SETTINGS_DIALOG, pParent)
{
}

CCalibratePixelToWavelengthSetupDialog::~CCalibratePixelToWavelengthSetupDialog()
{
    m_setup = nullptr;
    m_standardCrossSections = nullptr;
}

BOOL CCalibratePixelToWavelengthSetupDialog::OnInitDialog() {
    CDialog::OnInitDialog();

    UpdateData(FALSE);

    OnBnClickedRadioInstrumentLineShapeFitOption();

    if (m_standardCrossSections != nullptr)
    {
        auto allCrossSections = m_standardCrossSections->ListReferences();
        for (const std::string& crossSectionFile : allCrossSections)
        {
            CString fileName{ crossSectionFile.c_str() };
            fileName.Append(" [Standard]");
            m_crossSectionsCombo.AddString(fileName);
        }

        if (m_setup->m_fitInstrumentLineShapeOzoneReference.GetLength() > 3)
        {
            m_checkIncludeOzone.SetCheck(1);
        }
        else
        {
            m_checkIncludeOzone.SetCheck(0);
        }

        if (m_crossSectionsCombo.GetCount() > 0)
        {
            int indexToSelect = 0;
            for (int ii = 0; ii < static_cast<int>(m_standardCrossSections->NumberOfReferences()); ++ii)
            {
                if (m_standardCrossSections->ReferenceSpecieName(ii).find("O3") != std::string::npos ||
                    m_standardCrossSections->ReferenceSpecieName(ii).find("o3") != std::string::npos)
                {
                    indexToSelect = ii;
                    break;
                }
            }

            m_crossSectionsCombo.SetCurSel(indexToSelect);
        }
    }

    return TRUE;  // return TRUE unless you set the focus to a control
}

void CCalibratePixelToWavelengthSetupDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_INITIAL_CALIBRATION_SETTING, m_setup->m_initialCalibrationFile);
    DDX_Text(pDX, IDC_EDIT_SOLAR_SPECTRUM_SETTING, m_setup->m_solarSpectrumFile);
    DDX_Control(pDX, IDC_EDIT_INSTRUMENT_LINE_SHAPE_FIT_REGION_LOW, m_fitRegionEditLow);
    DDX_Control(pDX, IDC_EDIT_INSTRUMENT_LINE_SHAPE_FIT_REGION_HIGH, m_fitRegionEditHigh);
    DDX_Control(pDX, IDC_CHECK_INCLUDE_OZONE_ILS_FIT, m_checkIncludeOzone);
    DDX_Text(pDX, IDC_EDIT_INSTRUMENT_LINE_SHAPE_FIT_REGION_LOW, m_setup->m_fitInstrumentLineShapeRegionStart);
    DDX_Text(pDX, IDC_EDIT_INSTRUMENT_LINE_SHAPE_FIT_REGION_HIGH, m_setup->m_fitInstrumentLineShapeRegionStop);
    DDX_Radio(pDX, IDC_RADIO_INSTRUMENT_LINE_SHAPE_FIT_NOTHING, m_setup->m_fitInstrumentLineShapeOption);
    DDX_Control(pDX, IDC_COMBO_STANDARD_REFERENCES, m_crossSectionsCombo);
}

BEGIN_MESSAGE_MAP(CCalibratePixelToWavelengthSetupDialog, CDialog)
    ON_BN_CLICKED(IDC_RADIO_INSTRUMENT_LINE_SHAPE_FIT_NOTHING, &CCalibratePixelToWavelengthSetupDialog::OnBnClickedRadioInstrumentLineShapeFitOption)
    ON_BN_CLICKED(IDC_RADIO_INSTRUMENT_LINE_SHAPE_FIT_SUPER_GAUSS, &CCalibratePixelToWavelengthSetupDialog::OnBnClickedRadioInstrumentLineShapeFitOption)

    ON_BN_CLICKED(IDC_BUTTON_BROWSE_SOLAR_SPECTRUM_SETTING, &CCalibratePixelToWavelengthSetupDialog::OnClickedButtonBrowseSolarSpectrum)
    ON_BN_CLICKED(IDC_BUTTON_SELECT_INITIAL_CALIBRATION_SETTING, &CCalibratePixelToWavelengthSetupDialog::OnButtonSelectInitialCalibration)

    ON_BN_CLICKED(IDOK, &CCalibratePixelToWavelengthSetupDialog::OnBnClickedOk)
    ON_BN_CLICKED(IDC_CHECK_INCLUDE_OZONE_ILS_FIT, &CCalibratePixelToWavelengthSetupDialog::OnToggleCheckIncludeOzone)
END_MESSAGE_MAP()

// CCalibratePixelToWavelengthSetupDialog message handlers

void CCalibratePixelToWavelengthSetupDialog::OnBnClickedRadioInstrumentLineShapeFitOption()
{
    UpdateData(TRUE); // get the selections from the user interface

    BOOL enableInstrumentLineShapeFit = m_setup->m_fitInstrumentLineShapeOption == 1;
    m_fitRegionEditLow.EnableWindow(enableInstrumentLineShapeFit);
    m_fitRegionEditHigh.EnableWindow(enableInstrumentLineShapeFit);
    m_checkIncludeOzone.EnableWindow(enableInstrumentLineShapeFit);
    m_crossSectionsCombo.EnableWindow(enableInstrumentLineShapeFit);
}

void CCalibratePixelToWavelengthSetupDialog::OnButtonSelectInitialCalibration()
{
    OpenInstrumentCalibrationDialog dlg;
    dlg.m_state.initialCalibrationFile = m_setup->m_initialCalibrationFile;
    dlg.m_state.instrumentLineshapeFile = m_setup->m_instrumentLineshapeFile;
    dlg.m_state.calibrationOption = (InstrumentCalibrationInputOption)m_setup->m_calibrationOption;

    if (IDOK == dlg.DoModal())
    {
        m_setup->m_initialCalibrationFile = dlg.m_state.initialCalibrationFile;
        m_setup->m_instrumentLineshapeFile = dlg.m_state.instrumentLineshapeFile;
        m_setup->m_calibrationOption = (int)dlg.m_state.calibrationOption;

        UpdateData(FALSE);
    }
}

void CCalibratePixelToWavelengthSetupDialog::OnClickedButtonBrowseSolarSpectrum()
{
    if (!Common::BrowseForFile("Spectrum Files\0*.std;*.txt;*.xs\0", m_setup->m_solarSpectrumFile))
    {
        return;
    }
    UpdateData(FALSE);
}


void CCalibratePixelToWavelengthSetupDialog::OnBnClickedOk()
{
    // Validate the input data..
    if (!IsExistingFile(m_setup->m_solarSpectrumFile))
    {
        MessageBox("Please select a high resolved solar spectrum to use in the calibration", "Missing input", MB_OK);
        return;
    }
    if (!IsExistingFile(m_setup->m_initialCalibrationFile))
    {
        MessageBox("Please select a file which contains an initial guess for the wavelength calibration of the spectrometer", "Missing input", MB_OK);
        return;
    }
    if (m_setup->m_fitInstrumentLineShapeOption == 1)
    {
        // If we are to fit an instrument line shape, check the region.
        double fitRegionStart = std::atof(m_setup->m_fitInstrumentLineShapeRegionStart);
        double fitRegionStop = std::atof(m_setup->m_fitInstrumentLineShapeRegionStart);

        if (std::abs(fitRegionStart) < std::numeric_limits<float>::epsilon())
        {
            MessageBox("Failed to interpret the fit from as a numeric value. Please check the value and try again.", "Cannot parse input", MB_OK);
            return;
        }
        if (std::abs(fitRegionStop) < std::numeric_limits<float>::epsilon())
        {
            MessageBox("Failed to interpret the fit to as a numeric value. Please check the value and try again.", "Cannot parse input", MB_OK);
            return;
        }
        if (fitRegionStart > fitRegionStop)
        {
            MessageBox("The 'fit from' is larger than 'fit to'. Please make sure the values are correct and try again.", "Cannot parse input", MB_OK);
            return;
        }
    }

    CDialog::OnOK();
}

void CCalibratePixelToWavelengthSetupDialog::OnToggleCheckIncludeOzone()
{
    UpdateData(TRUE); // get the selections from the user interface

    bool includeOzone = m_checkIncludeOzone.GetCheck() == 1;
    int selectedReferenceIdx = m_crossSectionsCombo.GetCurSel();

    if (includeOzone && m_standardCrossSections != nullptr && selectedReferenceIdx >= 0)
    {
        const std::string path = m_standardCrossSections->ReferenceFileName(selectedReferenceIdx);
        m_setup->m_fitInstrumentLineShapeOzoneReference.Format("%s", path.c_str());
    }
    else
    {
        m_setup->m_fitInstrumentLineShapeOzoneReference = "";
    }
}
