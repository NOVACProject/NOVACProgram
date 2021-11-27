// Configuration\EvaluationConfiguration.cpp : implementation file
//

#include "stdafx.h"
#include "../resource.h"
#include "CCalibrationConfigurationDlg.h"
#include "../Dialogs/OpenInstrumentCalibrationDialog.h"
#include <SpectralEvaluation/Calibration/StandardCrossSectionSetup.h>
#include <SpectralEvaluation/DateTime.h>

using namespace ConfigurationDialog;
using namespace novac;


// CCalibrationConfigurationDlg dialog

IMPLEMENT_DYNCREATE(CCalibrationConfigurationDlg, CSystemConfigurationPage)



// CCalibrationConfigurationDlg::CCalibrationConfigurationDlg - Constructor

CCalibrationConfigurationDlg::CCalibrationConfigurationDlg() :
    CSystemConfigurationPage(CCalibrationConfigurationDlg::IDD)
{

}

// CCalibrationConfigurationDlg::DoDataExchange - Moves data between page and properties

void CCalibrationConfigurationDlg::DoDataExchange(CDataExchange* pDX)
{
    CSystemConfigurationPage::DoDataExchange(pDX);

    if (m_curSpec != nullptr)
    {
        DDX_Text(pDX, IDC_EDIT_SOLAR_SPECTRUM_SETTING, m_curSpec->channel[m_channel].autoCalibration.solarSpectrumFile);
        DDX_Text(pDX, IDC_EDIT_INITIAL_CALIBRATION_SETTING, m_curSpec->channel[m_channel].autoCalibration.initialCalibrationFile);

        DDX_Check(pDX, IDC_CHECK_REPLACE_USER_DEFINED_REFERENCES, m_curSpec->channel[m_channel].autoCalibration.generateReferences);
        DDX_Check(pDX, IDC_CHECK_FILTER_REFERENCES, m_curSpec->channel[m_channel].autoCalibration.filterReferences);

        DDX_Text(pDX, IDC_EDIT_INSTRUMENT_LINE_SHAPE_FIT_REGION_LOW, m_curSpec->channel[m_channel].autoCalibration.instrumentLineShapeFitRegion.low);
        DDX_Text(pDX, IDC_EDIT_INSTRUMENT_LINE_SHAPE_FIT_REGION_HIGH, m_curSpec->channel[m_channel].autoCalibration.instrumentLineShapeFitRegion.high);

        DDX_Radio(pDX, IDC_RADIO_INSTRUMENT_LINE_SHAPE_FIT_NOTHING, m_curSpec->channel[m_channel].autoCalibration.instrumentLineShapeFitOption);

        DDX_Text(pDX, IDC_CALIBRATION_INTERVAL_DAYS, m_curSpec->channel[m_channel].autoCalibration.intervalDays);
        DDV_MinMaxInt(pDX, m_curSpec->channel[m_channel].autoCalibration.intervalDays, 0, 10000);

        DDX_Text(pDX, IDC_CALIBRATION_INTERVAL_HOURS, m_intervalHr);
        DDV_MinMaxInt(pDX, m_intervalHr, 0, 23);

        DDX_Text(pDX, IDC_CALIBRATION_INTERVAL_MINUTES, m_intervalMin);
        DDV_MinMaxInt(pDX, m_intervalMin, 0, 59);
    }
}

// Message map

BEGIN_MESSAGE_MAP(CCalibrationConfigurationDlg, CSystemConfigurationPage)
    ON_BN_CLICKED(IDC_BUTTON_SELECT_INITIAL_CALIBRATION_SETTING, &CCalibrationConfigurationDlg::OnBnClickedButtonSelectInitialCalibrationSetting)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_SOLAR_SPECTRUM_SETTING, &CCalibrationConfigurationDlg::OnBnClickedButtonBrowseSolarSpectrumSetting)

    ON_EN_CHANGE(IDC_CALIBRATION_INTERVAL_DAYS, SaveData)
    ON_EN_CHANGE(IDC_CALIBRATION_INTERVAL_HOURS, SaveData)
    ON_EN_CHANGE(IDC_CALIBRATION_INTERVAL_MINUTES, SaveData)

END_MESSAGE_MAP()


BOOL CCalibrationConfigurationDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    if (m_curSpec != nullptr && m_channel >= 0)
    {
        auto& currentSettings = m_curSpec->channel[m_channel].autoCalibration;

        // If there isn't a solar reference file in the settings already, then include the default one.
        if (currentSettings.solarSpectrumFile.GetLength() == 0)
        {
            Common common;
            common.GetExePath();

            // See if there a Fraunhofer reference in the standard cross section setup.
            std::string exePath = common.m_exePath;
            novac::StandardCrossSectionSetup standardCrossSections{ exePath };

            const auto solarCrossSection = standardCrossSections.FraunhoferReferenceFileName();

            if (solarCrossSection.size() > 0)
            {
                currentSettings.solarSpectrumFile = CString(solarCrossSection.c_str());
            }
        }

        // If the wavelength region over which the instrument line shape can be fitted is empty (or reversed) then set a default.
        if (currentSettings.instrumentLineShapeFitRegion.Empty())
        {
            currentSettings.instrumentLineShapeFitRegion = novac::WavelengthRange(330.0, 350.0);
        }

        int tmp;
        novac::SplitToHourMinuteSecond(currentSettings.intervalTimeOfDay, m_intervalHr, m_intervalMin, tmp);
    }

    UpdateData(FALSE);

    return TRUE;  // return TRUE unless you set the focus to a control
}

// CCalibrationConfigurationDlg message handlers


void ConfigurationDialog::CCalibrationConfigurationDlg::OnBnClickedButtonSelectInitialCalibrationSetting()
{
    if (m_curSpec == nullptr || m_channel > MAX_CHANNEL_NUM)
    {
        return;
    }

    auto& currentSettings = m_curSpec->channel[m_channel].autoCalibration;

    OpenInstrumentCalibrationDialog dlg;
    dlg.m_state.initialCalibrationFile = currentSettings.initialCalibrationFile;
    dlg.m_state.instrumentLineshapeFile = currentSettings.instrumentLineshapeFile;
    dlg.m_state.calibrationOption = (InstrumentCalibrationInputOption)currentSettings.initialCalibrationType;

    if (IDOK == dlg.DoModal())
    {
        currentSettings.initialCalibrationFile = dlg.m_state.initialCalibrationFile;
        currentSettings.instrumentLineshapeFile = dlg.m_state.instrumentLineshapeFile;
        currentSettings.initialCalibrationType = (int)dlg.m_state.calibrationOption;

        UpdateData(FALSE);
    }
}

void ConfigurationDialog::CCalibrationConfigurationDlg::OnBnClickedButtonBrowseSolarSpectrumSetting()
{
    if (m_curSpec == nullptr || m_channel > MAX_CHANNEL_NUM)
    {
        return;
    }

    if (!Common::BrowseForFile("Spectrum Files\0*.std;*.txt;*.xs\0", m_curSpec->channel[m_channel].autoCalibration.solarSpectrumFile))
    {
        return;
    }

    UpdateData(FALSE);
}

void ConfigurationDialog::CCalibrationConfigurationDlg::SaveData()
{
    if (m_curSpec == nullptr || m_channel > MAX_CHANNEL_NUM)
    {
        return;
    }

    if (UpdateData(TRUE)) // get the values from the user interface
    {
        m_curSpec->channel[m_channel].autoCalibration.intervalTimeOfDay = 3600 * m_intervalHr + 60 * m_intervalMin;
    }
}

