// CreateReferencesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CreateReferencesDlg.h"
#include "afxdialogex.h"
#include "resource.h"
#include "Common/Common.h"
#include "Dialogs/ReferencePlotDlg.h"
#include <SpectralEvaluation/Spectra/ReferenceSpectrumConvolution.h>
#include <SpectralEvaluation/Evaluation/CrossSectionData.h>
#include <fstream>

// CCreateReferencesDlg dialog

IMPLEMENT_DYNAMIC(CCreateReferencesDlg, CDialog)

CCreateReferencesDlg::CCreateReferencesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_CREATE_REFERENCES_DIALOG, pParent)
{
}

CCreateReferencesDlg::~CCreateReferencesDlg()
{
}

void CCreateReferencesDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_BUTTON_RUNCONVOLUTION, m_runConvolutionBtn);
    DDX_Text(pDX, IDC_STATIC_SLITFUNCTIONFILENAME, m_slfFile);
    DDX_Text(pDX, IDC_STATIC_CALIBRATIONFILENAME, m_calibrationFile);
    DDX_Text(pDX, IDC_STATIC_CROSSSECTIONFILENAME, m_crossSectionFile);
    DDX_Text(pDX, IDC_STATIC_OUTPUTFILENAME, m_outputFile);
    DDX_Control(pDX, IDC_COMBO_WAVELENGTH_CONVERSION, m_wavelengthConversionCombo);
}

BOOL CCreateReferencesDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_wavelengthConversionCombo.SetCurSel(0);
    
    return TRUE;
}

BEGIN_MESSAGE_MAP(CCreateReferencesDlg, CDialog)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_SLF, &CCreateReferencesDlg::OnButtonBrowseSlf)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_CLB, &CCreateReferencesDlg::OnButtonBrowseCalibration)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_XS, &CCreateReferencesDlg::OnButtonBrowseHighResCrossSection)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_OUTPUT, &CCreateReferencesDlg::OnButtonBrowseOutput)
    ON_BN_CLICKED(IDC_BUTTON_RUNCONVOLUTION, &CCreateReferencesDlg::OnButtonRunConvolution)
END_MESSAGE_MAP()


// CCreateReferencesDlg message handlers


void CCreateReferencesDlg::OnButtonBrowseSlf()
{
    TCHAR filter[512];
    int n = _stprintf(filter, "Slit Function File\0");
    n += _stprintf(filter + n + 1, "*.slf;*.txt;\0");
    filter[n + 2] = 0;

    // let the user browse for an evaluation log file and if one is selected, read it
    if (Common::BrowseForFile(filter, m_slfFile))
    {
        UpdateRunConvolutionButton();
        UpdateData(FALSE);
    }
}

void CCreateReferencesDlg::OnButtonBrowseCalibration()
{
    TCHAR filter[512];
    int n = _stprintf(filter, "Wavelength Calibration File\0");
    n += _stprintf(filter + n + 1, "*.clb;*.txt;\0");
    filter[n + 2] = 0;

    // let the user browse for an evaluation log file and if one is selected, read it
    if (Common::BrowseForFile(filter, m_calibrationFile))
    {
        UpdateRunConvolutionButton();
        UpdateData(FALSE);
    }
}

void CCreateReferencesDlg::OnButtonBrowseHighResCrossSection()
{
    TCHAR filter[512];
    int n = _stprintf(filter, "Cross Section File\0");
    n += _stprintf(filter + n + 1, "*.xs;*.txt;\0");
    filter[n + 2] = 0;

    // let the user browse for an evaluation log file and if one is selected, read it
    if (Common::BrowseForFile(filter, m_crossSectionFile))
    {
        UpdateRunConvolutionButton();
        UpdateData(FALSE);
    }
}

void CCreateReferencesDlg::OnButtonBrowseOutput()
{
    TCHAR filter[512];
    int n = _stprintf(filter, "Reference File\0");
    n += _stprintf(filter + n + 1, "*.xs;*.txt;\0");
    filter[n + 2] = 0;

    // let the user browse for an evaluation log file and if one is selected, read it
    if (Common::BrowseForFile_SaveAs(filter, m_outputFile))
    {
        UpdateRunConvolutionButton();
        UpdateData(FALSE);
    }
}

void CCreateReferencesDlg::UpdateRunConvolutionButton()
{
    if (m_slfFile.GetLength() > 4 &&
        m_calibrationFile.GetLength() > 4 &&
        m_crossSectionFile.GetLength() > 4 &&
        m_outputFile.GetLength() > 4)
    {
        m_runConvolutionBtn.EnableWindow(TRUE);
    }
    else
    {
        m_runConvolutionBtn.EnableWindow(FALSE);
    }
}

WavelengthConversion GetWavelengthConversion(const CComboBox& comboSelector)
{
    if (0 == comboSelector.GetCurSel())
    {
        return WavelengthConversion::None;
    }
    else
    {
        return WavelengthConversion::VacuumToAir;
    }
}

void CCreateReferencesDlg::OnButtonRunConvolution()
{
    const std::string slf((LPCSTR)m_slfFile);
    const std::string calibration((LPCSTR)m_calibrationFile);
    const std::string highResReference((LPCSTR)m_crossSectionFile);
    const WavelengthConversion conversion = GetWavelengthConversion(m_wavelengthConversionCombo);

    Evaluation::CCrossSectionData result;

    bool success = Evaluation::ConvolveReference(
        calibration,
        slf,
        highResReference,
        result,
        conversion);

    if (!success)
    {
        MessageBox("Failed to create reference, check the input files and try again", "Failed to convolve reference", MB_OK);
        return;
    }
    if (0 == result.m_crossSection.size())
    {
        MessageBox("Failed to create reference, result has zero length. Check the input files and try again", "Failed to convolve reference", MB_OK);
        return;
    }

    // check that the result is not all zeros
    size_t nonZeroRegionFrom = 0;
    size_t nonZeroRegionTo = 0;
    for (size_t idx = 0; idx < result.m_crossSection.size(); ++idx)
    {
        if (std::abs(result.m_crossSection[idx]) > 1e-40)
        {
            nonZeroRegionFrom = idx;
            break;
        }
    }
    for (size_t idx = nonZeroRegionFrom + 1; idx < result.m_crossSection.size(); ++idx)
    {
        if (std::abs(result.m_crossSection[idx]) < 1e-40)
        {
            nonZeroRegionTo = idx;
            break;
        }
    }
    if (nonZeroRegionTo - nonZeroRegionFrom == 0)
    {
        MessageBox("Failed to create reference, result is all zeros. Check the input files and try again", "Failed to convolve reference", MB_OK);
        return;
    }


    HighPassFilter(result, true);

    // Save the result to file
    const std::string outputPath((LPCSTR)m_outputFile);
    std::ofstream dstFile{ outputPath, std::ios::out};
    if (!dstFile.is_open())
    {
        MessageBox("Failed to save output to file, check the filename and try again", "Failed to save reference", MB_OK);
        return;
    }

    if (result.m_waveLength.size() == result.m_crossSection.size())
    {
        for (size_t ii = 0; ii < result.m_crossSection.size(); ++ii)
        {
            dstFile << result.m_waveLength[ii] << "\t" << result.m_crossSection[ii] << "\n";
        }
    }
    else
    {
        for (size_t ii = 0; ii < result.m_crossSection.size(); ++ii)
        {
            dstFile << result.m_crossSection[ii] << "\n";
        }
    }

    // Show the result to the user
    Evaluation::CFitWindow window;
    window.fitLow = nonZeroRegionFrom;
    window.fitHigh = nonZeroRegionTo;
    window.ref[0].m_path = outputPath;
    window.nRef = 1;

    Dialogs::CReferencePlotDlg dlg;
    dlg.m_window = &window;
    dlg.DoModal();

}

