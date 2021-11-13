#pragma once

struct CalibratePixelToWavelengthDialogSetup;

/// <summary>
/// CCalibratePixelToWavelengthSetupDialog dialog. Helps setting up the pixel-to-wavelength calibration
/// and is a helper dialog to CalibratePixelToWavelengthDialog
/// </summary>
class CCalibratePixelToWavelengthSetupDialog : public CDialog
{
    DECLARE_DYNAMIC(CCalibratePixelToWavelengthSetupDialog)

public:
    CCalibratePixelToWavelengthSetupDialog(CalibratePixelToWavelengthDialogSetup* setup, CWnd* pParent = nullptr);
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

    afx_msg void OnBnClickedRadioInstrumentLineShapeFitOption();

    CEdit m_fitRegionEditLow;
    CEdit m_fitRegionEditHigh;

    afx_msg void OnBnClickedOk();
    afx_msg void OnClickedButtonBrowseSolarSpectrum();
    afx_msg void OnButtonSelectInitialCalibration();

};
