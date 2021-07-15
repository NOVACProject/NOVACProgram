// CSpectrometerCalibrationDlg.cpp : implementation file
//
#include "StdAfx.h"
#include "SpectrometerCalibrationDlg.h"
#include "afxdialogex.h"
#include "../resource.h"
#include "CCalibratePixelToWavelengthDialog.h"
#include "CCalibrateReferencesDialog.h"

// CSpectrometerCalibrationDlg dialog

CSpectrometerCalibrationDlg::CSpectrometerCalibrationDlg()
    : CPropertySheet()
{
    m_calibratePixelToWavelength = new CCalibratePixelToWavelengthDialog();
    m_calibratePixelToWavelength->Construct(IDD_CALIBRATE_WAVELENGTH_DIALOG);

    m_calibrateReferences = new CCalibrateReferencesDialog();
    m_calibrateReferences->Construct(IDD_CALIBRATE_REFERENCES);

    AddPage(m_calibratePixelToWavelength);
    AddPage(m_calibrateReferences);
}

CSpectrometerCalibrationDlg::~CSpectrometerCalibrationDlg()
{
    delete m_calibratePixelToWavelength;
    delete m_calibrateReferences;
}

void CSpectrometerCalibrationDlg::DoDataExchange(CDataExchange* pDX)
{
    CPropertySheet::DoDataExchange(pDX);
}

BOOL CSpectrometerCalibrationDlg::OnInitDialog()
{
    BOOL bResult = CPropertySheet::OnInitDialog();

    // ------------ Get the buttons ---------------
    CWnd* pApply = this->GetDlgItem(ID_APPLY_NOW);
    CWnd* pCancel = this->GetDlgItem(IDCANCEL);
    CWnd* pOk = this->GetDlgItem(IDOK);

    // Remove each of the buttons
    if (pApply) {
        pApply->DestroyWindow();
    }
    if (pCancel) {
        pCancel->DestroyWindow();
    }
    if (pOk) {
        pOk->DestroyWindow();
    }

    return bResult;
}

BEGIN_MESSAGE_MAP(CSpectrometerCalibrationDlg, CPropertySheet)
END_MESSAGE_MAP()


// CSpectrometerCalibrationDlg message handlers
