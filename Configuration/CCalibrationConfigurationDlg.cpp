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
        DDX_Check(pDX, IDC_CHECK_ENABLE_INSTRUMENT_CALIBRATION, m_curSpec->channel[m_channel].autoCalibration.enable);

        DDX_Text(pDX, IDC_EDIT_SOLAR_SPECTRUM_SETTING, m_curSpec->channel[m_channel].autoCalibration.solarSpectrumFile);
        DDX_Text(pDX, IDC_EDIT_INITIAL_CALIBRATION_SETTING, m_curSpec->channel[m_channel].autoCalibration.initialCalibrationFile);

        DDX_Check(pDX, IDC_CHECK_REPLACE_USER_DEFINED_REFERENCES, m_curSpec->channel[m_channel].autoCalibration.generateReferences);
        DDX_Check(pDX, IDC_CHECK_FILTER_REFERENCES, m_curSpec->channel[m_channel].autoCalibration.filterReferences);

        DDX_Text(pDX, IDC_EDIT_INSTRUMENT_LINE_SHAPE_FIT_REGION_LOW, m_curSpec->channel[m_channel].autoCalibration.instrumentLineShapeFitRegion.low);
        DDX_Text(pDX, IDC_EDIT_INSTRUMENT_LINE_SHAPE_FIT_REGION_HIGH, m_curSpec->channel[m_channel].autoCalibration.instrumentLineShapeFitRegion.high);

        DDX_Radio(pDX, IDC_RADIO_INSTRUMENT_LINE_SHAPE_FIT_NOTHING, m_curSpec->channel[m_channel].autoCalibration.instrumentLineShapeFitOption);

        DDX_Text(pDX, IDC_CALIBRATION_INTERVAL_DAYS, m_curSpec->channel[m_channel].autoCalibration.intervalHours);
        DDV_MinMaxInt(pDX, m_curSpec->channel[m_channel].autoCalibration.intervalHours, 0, 10000);

        DDX_Text(pDX, IDC_CALIBRATION_INTERVAL_LOW_HOURS, m_intervalLowHr);
        DDV_MinMaxInt(pDX, m_intervalLowHr, 0, 23);

        DDX_Text(pDX, IDC_CALIBRATION_INTERVAL_LOW_MINUTES, m_intervalLowMin);
        DDV_MinMaxInt(pDX, m_intervalLowMin, 0, 59);

        DDX_Text(pDX, IDC_CALIBRATION_INTERVAL_HIGH_HOURS, m_intervalHighHr);
        DDV_MinMaxInt(pDX, m_intervalHighHr, 0, 23);

        DDX_Text(pDX, IDC_CALIBRATION_INTERVAL_HIGH_MINUTES, m_intervalHighMin);
        DDV_MinMaxInt(pDX, m_intervalHighMin, 0, 59);
    }
}

// Message map

BEGIN_MESSAGE_MAP(CCalibrationConfigurationDlg, CSystemConfigurationPage)
    ON_COMMAND(IDC_CHECK_ENABLE_INSTRUMENT_CALIBRATION, UpdateDialogState)

    ON_BN_CLICKED(IDC_BUTTON_SELECT_INITIAL_CALIBRATION_SETTING, &CCalibrationConfigurationDlg::OnBnClickedButtonSelectInitialCalibrationSetting)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_SOLAR_SPECTRUM_SETTING, &CCalibrationConfigurationDlg::OnBnClickedButtonBrowseSolarSpectrumSetting)

    ON_EN_CHANGE(IDC_CALIBRATION_INTERVAL_DAYS, SaveData)
    ON_EN_CHANGE(IDC_CALIBRATION_INTERVAL_LOW_HOURS, SaveData)
    ON_EN_CHANGE(IDC_CALIBRATION_INTERVAL_LOW_MINUTES, SaveData)
    ON_EN_CHANGE(IDC_CALIBRATION_INTERVAL_HIGH_HOURS, SaveData)
    ON_EN_CHANGE(IDC_CALIBRATION_INTERVAL_HIGH_MINUTES, SaveData)

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
        novac::SplitToHourMinuteSecond(currentSettings.intervalTimeOfDayLow, m_intervalLowHr, m_intervalLowMin, tmp);
        novac::SplitToHourMinuteSecond(currentSettings.intervalTimeOfDayHigh, m_intervalHighHr, m_intervalHighMin, tmp);
    }

    UpdateData(FALSE); // Update the user interface with the new values

    UpdateDialogState();

    return TRUE;  // return TRUE unless you set the focus to a control
}

void CCalibrationConfigurationDlg::UpdateDialogState()
{
    const auto itemsToUpdate = std::vector<int>{
        IDC_BUTTON_SELECT_INITIAL_CALIBRATION_SETTING,
        IDC_BUTTON_BROWSE_SOLAR_SPECTRUM_SETTING,
        IDC_RADIO_INSTRUMENT_LINE_SHAPE_FIT_NOTHING,
        IDC_RADIO_INSTRUMENT_LINE_SHAPE_FIT_SUPER_GAUSS,
        IDC_EDIT_INSTRUMENT_LINE_SHAPE_FIT_REGION_LOW,
        IDC_EDIT_INSTRUMENT_LINE_SHAPE_FIT_REGION_HIGH,
        IDC_CHECK_REPLACE_USER_DEFINED_REFERENCES,
        IDC_CHECK_FILTER_REFERENCES,
        IDC_CALIBRATION_INTERVAL_DAYS,
        IDC_CALIBRATION_INTERVAL_HOURS,
        IDC_CALIBRATION_INTERVAL_MINUTES
    };

    UpdateData(TRUE); // get the values from the user interface

    if (m_curSpec != nullptr && m_channel >= 0)
    {
        auto& currentSettings = m_curSpec->channel[m_channel].autoCalibration;

        for (int elementId : itemsToUpdate)
        {
            auto item = this->GetDlgItem(elementId);
            if (item != nullptr)
            {
                item->EnableWindow(currentSettings.enable);
            }
        }
    }
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

        UpdateData(FALSE); // Update the user interface with the new values
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

    UpdateData(FALSE); // Update the user interface with the new values
}

void ConfigurationDialog::CCalibrationConfigurationDlg::SaveData()
{
    if (m_curSpec == nullptr || m_channel > MAX_CHANNEL_NUM)
    {
        return;
    }

    if (UpdateData(TRUE)) // get the values from the user interface
    {
        m_curSpec->channel[m_channel].autoCalibration.intervalTimeOfDayLow = 3600 * m_intervalLowHr + 60 * m_intervalLowMin;
        m_curSpec->channel[m_channel].autoCalibration.intervalTimeOfDayHigh = 3600 * m_intervalHighHr + 60 * m_intervalHighMin;
    }
}

