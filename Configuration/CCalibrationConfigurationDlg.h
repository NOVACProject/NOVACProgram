#pragma once

#include "../Common/Common.h"
#include "SystemConfigurationPage.h"

// CCalibrationConfigurationDlg : Property page dialog

namespace ConfigurationDialog
{
    class CCalibrationConfigurationDlg : public CSystemConfigurationPage
    {
        DECLARE_DYNCREATE(CCalibrationConfigurationDlg)

        // Constructors
    public:
        CCalibrationConfigurationDlg();

        // Dialog Data
        enum { IDD = IDD_CONFIGURE_CALIBRATION };

        /** Initializing the dialog */
        virtual BOOL OnInitDialog();

        /** The spectrometer channel that this object is modifying */
        unsigned int m_channel = 0;

        // Implementation
    protected:
        virtual void DoDataExchange(CDataExchange* pDX);        // DDX/DDV support

        DECLARE_MESSAGE_MAP()

    private:

        /** The time when to start performing the calibrations */
        int m_intervalLowHr;
        int m_intervalLowMin;

        /** The time when to stop performing the calibrations */
        int m_intervalHighHr;
        int m_intervalHighMin;

        afx_msg void OnBnClickedButtonSelectInitialCalibrationSetting();
        afx_msg void OnBnClickedButtonBrowseSolarSpectrumSetting();
        afx_msg void SaveData();

        /**  Enable or disable the controls based on if the auto calibration is enabled or nor */
        afx_msg void UpdateDialogState();
    };
}