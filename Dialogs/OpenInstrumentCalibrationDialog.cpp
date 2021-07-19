// OpenInstrumentCalibrationDialog.cpp : implementation file
//

#include "stdafx.h"
#include "OpenInstrumentCalibrationDialog.h"
#include "afxdialogex.h"
#include "../Common/Common.h"
#include "../resource.h"

// OpenInstrumentCalibrationDialog dialog

IMPLEMENT_DYNAMIC(OpenInstrumentCalibrationDialog, CDialog)

OpenInstrumentCalibrationDialog::OpenInstrumentCalibrationDialog(CWnd* pParent /*=nullptr*/)
    : CDialog(IDD_CALIBRATE_OPEN_CALIBRATION, pParent)
    , m_initialCalibrationFileTypeFilter("Extended Standard Files\0*.std\0\0")
{
}

OpenInstrumentCalibrationDialog::~OpenInstrumentCalibrationDialog()
{
}

BOOL OpenInstrumentCalibrationDialog::OnInitDialog()
{
    CDialog::OnInitDialog();
    this->m_comboDataType.SetCurSel((int)this->m_state.calibrationOption);

    return TRUE;  // return TRUE unless you set the focus to a control
}

void OpenInstrumentCalibrationDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_INITIAL_DATA_TYPE, m_comboDataType);
    DDX_Text(pDX, IDC_EDIT_INITIAL_CALIBRATION, m_state.initialCalibrationFile);
    DDX_Text(pDX, IDC_EDIT_INITIAL_CALIBRATION2, m_state.instrumentLineshapeFile);
    DDX_Control(pDX, IDC_BUTTON_BROWSE_LINE_SHAPE, m_buttonBrowseInstrumentLineShape);
    DDX_Control(pDX, IDC_STATIC_LINEHSHAPE, m_labelInstrumentLineShape);
    DDX_Control(pDX, IDC_STATIC_INITIAL_CALIBRATION, m_labelCalibration);
}

BEGIN_MESSAGE_MAP(OpenInstrumentCalibrationDialog, CDialog)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_LINE_SHAPE, &OpenInstrumentCalibrationDialog::OnButtonBrowseLineShape)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_CALIBRATION, &OpenInstrumentCalibrationDialog::OnButtonBrowseCalibration)
    ON_CBN_SELCHANGE(IDC_COMBO_INITIAL_DATA_TYPE, &OpenInstrumentCalibrationDialog::OnSelchangeDataType)
END_MESSAGE_MAP()


// OpenInstrumentCalibrationDialog message handlers


void OpenInstrumentCalibrationDialog::OnButtonBrowseLineShape()
{
    if (!Common::BrowseForFile("Instrument Line Shape Files\0*.slf\0Spectrum Files\0*.txt;*.xs\0", this->m_state.instrumentLineshapeFile))
    {
        return;
    }

    UpdateData(FALSE);
}

void OpenInstrumentCalibrationDialog::OnButtonBrowseCalibration()
{
    if (!Common::BrowseForFile(m_initialCalibrationFileTypeFilter, this->m_state.initialCalibrationFile))
    {
        return;
    }

    UpdateData(FALSE);
}

void OpenInstrumentCalibrationDialog::OnSelchangeDataType()
{
    this->m_state.calibrationOption = (InstrumentCalibrationInputOption)this->m_comboDataType.GetCurSel();

    if (this->m_state.calibrationOption == InstrumentCalibrationInputOption::NovacInstrumentCalibrationFile)
    {
        // 2: User provides a wavelength calibration file, the program derives the instrument line shape
        m_initialCalibrationFileTypeFilter = "Wavelength Calibration Files\0*.clb\0Spectrum Files\0*.txt;*.xs\0\0";
        m_labelCalibration.SetWindowTextA("Wavelength Calibration");
        m_labelInstrumentLineShape.EnableWindow(FALSE);
        m_buttonBrowseInstrumentLineShape.EnableWindow(FALSE);
    }
    else if (this->m_state.calibrationOption == InstrumentCalibrationInputOption::WavelengthAndSlitFunctionFile)
    {
        // 1: User provides a wavelength calibration file and instrument line shape
        m_initialCalibrationFileTypeFilter = "Wavelength Calibration Files\0*.clb\0Spectrum Files\0*.txt;*.xs\0\0";
        m_labelCalibration.SetWindowTextA("Wavelength Calibration");
        m_labelInstrumentLineShape.EnableWindow(TRUE);
        m_buttonBrowseInstrumentLineShape.EnableWindow(TRUE);
    }
    else
    {
        // 0: User provides a extended STD file containing both wavelength calibration and instrument line shape
        m_initialCalibrationFileTypeFilter = "Extended Standard Files\0*.std\0\0";
        m_labelCalibration.SetWindowTextA("Calibration");
        m_labelInstrumentLineShape.EnableWindow(FALSE);
        m_buttonBrowseInstrumentLineShape.EnableWindow(FALSE);
    }
}
