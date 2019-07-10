#pragma once
#include "afxwin.h"
#include <SpectralEvaluation/Spectra/SpectrometerModel.h>

// CustomSpectrometerModelDlg dialog

class CustomSpectrometerModelDlg : public CDialog
{
    DECLARE_DYNAMIC(CustomSpectrometerModelDlg)

public:
    CustomSpectrometerModelDlg(CWnd* pParent = NULL);   // standard constructor
    virtual ~CustomSpectrometerModelDlg();

    // Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_CONFIGURE_SPECTROMETERMODEL };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedOk();

    SpectrometerModel m_configuredSpectrometer;

private:
    CString m_modelName;
    CString m_maxIntensity;
};
