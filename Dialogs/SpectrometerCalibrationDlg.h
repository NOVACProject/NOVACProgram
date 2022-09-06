#pragma once

#include "afxdlgs.h"

// This is the spectrometer calibration dialog
//  where we can both fit an instrument line shape _and_ calibrate the wavelength
class CSpectrometerCalibrationDlg : public CPropertySheet
{
public:
    CSpectrometerCalibrationDlg();   // standard constructor
    virtual ~CSpectrometerCalibrationDlg();

    virtual BOOL OnInitDialog();

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()

private:
    CPropertyPage* m_calibratePixelToWavelength;
    CPropertyPage* m_calibrateReferences;

};
