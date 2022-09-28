#pragma once
#include "afxdlgs.h"

struct CalibratePixelToWavelengthDialogSetup;

namespace novac
{
    class StandardCrossSectionSetup;
}

/// <summary>
/// CCalibratePixelToWavelengthSetupDialog dialog. Helps setting up the pixel-to-wavelength calibration
/// and is a helper dialog to CalibratePixelToWavelengthDialog
/// </summary>
class CCalibratePixelToWavelengthSetupDialog : public CDialog
{
    DECLARE_DYNAMIC(CCalibratePixelToWavelengthSetupDialog)

public:
    CCalibratePixelToWavelengthSetupDialog(CalibratePixelToWavelengthDialogSetup* setup, novac::StandardCrossSectionSetup* crossSections, CWnd* pParent = nullptr);
    virtual ~CCalibratePixelToWavelengthSetupDialog();

    /** Initializes the controls and the dialog */
    virtual BOOL OnInitDialog();

    // Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_CALIBRATE_WAVELENGTH_SETTINGS_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()

public:
    CalibratePixelToWavelengthDialogSetup* m_setup;

    novac::StandardCrossSectionSetup* m_standardCrossSections;

    afx_msg void OnBnClickedRadioInstrumentLineShapeFitOption();

    CEdit m_fitRegionEditLow;
    CEdit m_fitRegionEditHigh;

    CButton m_checkIncludeOzone;
    CComboBox m_crossSectionsCombo;
    CComboBox m_spectrometerModelCombo;

    afx_msg void OnBnClickedOk();
    afx_msg void OnClickedButtonBrowseSolarSpectrum();
    afx_msg void OnButtonSelectInitialCalibration();
    afx_msg void OnToggleCheckIncludeOzone();
    afx_msg void OnSelchangeSpectrometerModel();
};
