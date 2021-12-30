// CustomSpectrometerModelDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CustomSpectrometerModelDlg.h"
#include "afxdialogex.h"
#include "resource.h"


// CustomSpectrometerModelDlg dialog

IMPLEMENT_DYNAMIC(CustomSpectrometerModelDlg, CDialog)

CustomSpectrometerModelDlg::CustomSpectrometerModelDlg(CWnd* pParent /*=NULL*/)
    : CDialog(IDD_CONFIGURE_SPECTROMETERMODEL, pParent)
{

}

CustomSpectrometerModelDlg::~CustomSpectrometerModelDlg()
{
}

void CustomSpectrometerModelDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_MODELNAME, m_modelName);
    DDX_Text(pDX, IDC_EDIT_MAXIMUMINTENSITY, m_maxIntensity);
}


BEGIN_MESSAGE_MAP(CustomSpectrometerModelDlg, CDialog)
    ON_BN_CLICKED(IDOK, &CustomSpectrometerModelDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CustomSpectrometerModelDlg message handlers


void CustomSpectrometerModelDlg::OnBnClickedOk()
{
    // Validate the data
    UpdateData(true);

    if (m_modelName.GetLength() == 0)
    {
        MessageBox("The model name must not be empty.");
        return;
    }


    double configuredMaxIntensity = atof(m_maxIntensity);
    if (configuredMaxIntensity <= 0.0)
    {
        MessageBox("Invalid maximum intensity.");
        return;
    }

    m_configuredSpectrometer.modelName = m_modelName;
    m_configuredSpectrometer.maximumIntensityForSingleReadout = configuredMaxIntensity;

    CDialog::OnOK();
}
