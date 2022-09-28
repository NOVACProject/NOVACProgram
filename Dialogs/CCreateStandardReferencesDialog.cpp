// CCreateStandardReferencesDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "CCreateStandardReferencesDialog.h"
#include "afxdlgs.h"
#include "../resource.h"
#include "../Common/Common.h"
#include <SpectralEvaluation/Calibration/StandardCrossSectionSetup.h>
#include <SpectralEvaluation/File/File.h>
#include <sstream>

// CCreateStandardReferencesDialog dialog

IMPLEMENT_DYNAMIC(CCreateStandardReferencesDialog, CDialog)

CCreateStandardReferencesDialog::CCreateStandardReferencesDialog(CWnd* pParent /*=nullptr*/)
    : CDialog(IDD_CALIBRATE_STANDARD_REFERENCES, pParent)
    , m_outputdirectory(_T(""))
    , m_instrumentName(_T(""))
    , m_fileNameSuffix(_T(""))
    , m_outputFileNamesExplanation(_T(""))
{
}

CCreateStandardReferencesDialog::~CCreateStandardReferencesDialog()
{
}

void CCreateStandardReferencesDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_STANDARD_REFERENCES_OUTPUT_DIRECTORY, m_outputdirectory);
    DDX_Text(pDX, IDC_EDIT_INSTRUMENT_NAME, m_instrumentName);
    DDX_Text(pDX, IDC_EDIT_FILE_SUFFIX, m_fileNameSuffix);
    DDX_Control(pDX, IDOK, m_okButton);
    DDX_Text(pDX, IDC_STATIC_STANDARD_REFERENCES_OUTPUT_FILENAMES, m_outputFileNamesExplanation);
}

BEGIN_MESSAGE_MAP(CCreateStandardReferencesDialog, CDialog)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_STANDARD_REFERENCES_DIRECTORY, &CCreateStandardReferencesDialog::OnClickedButtonBrowseOutputDirectory)
    ON_EN_CHANGE(IDC_EDIT_INSTRUMENT_NAME, &CCreateStandardReferencesDialog::OnChangeInstrumentName)
    ON_EN_CHANGE(IDC_EDIT_FILE_SUFFIX, &CCreateStandardReferencesDialog::OnChangeFileSuffix)
END_MESSAGE_MAP()


// CCreateStandardReferencesDialog message handlers

bool CCreateStandardReferencesDialog::IsSetupCorrectly() const
{
    if (m_outputdirectory.GetLength() == 0)
    {
        return false;
    }

    return true;
}

std::string CCreateStandardReferencesDialog::ReferenceFileName(const std::string& nameOfReference, bool includFilteringInfix, bool includeDirectory) const
{
    std::stringstream dstFileNameStream;
    if (includeDirectory)
    {
        dstFileNameStream << m_outputdirectory << "\\";
    }

    if (m_instrumentName.GetLength() > 0)
    {
        dstFileNameStream << m_instrumentName << "_";
    }
    dstFileNameStream << nameOfReference;

    if (includFilteringInfix)
    {
        dstFileNameStream << m_fileNameFilteringInfix;
    }

    if (m_fileNameSuffix.GetLength() > 0)
    {
        dstFileNameStream << "_" << m_fileNameSuffix;
    }

    dstFileNameStream << ".txt";

    return dstFileNameStream.str();
}


std::string CCreateStandardReferencesDialog::FraunhoferReferenceName(bool includeDirectory) const
{
    return ReferenceFileName("Fraunhofer", false, includeDirectory);
}

std::string CCreateStandardReferencesDialog::ReferenceName(size_t referenceIdx, bool includFilteringInfix, bool includeDirectory) const
{
    return ReferenceFileName(m_standardCrossSections->ReferenceSpecieName(referenceIdx), includFilteringInfix, includeDirectory);
}

void CCreateStandardReferencesDialog::OnClickedButtonBrowseOutputDirectory()
{
    if (!Common::BrowseForDirectory(m_outputdirectory))
    {
        return;
    }

    if (IsSetupCorrectly())
    {
        m_okButton.EnableWindow(TRUE);
    }
    else
    {
        m_okButton.EnableWindow(FALSE);
    }

    UpdateOutputFileNamesExplanation();
}

void CCreateStandardReferencesDialog::OnChangeInstrumentName()
{
    UpdateData(TRUE); // Get the values from the UI
    UpdateOutputFileNamesExplanation();
}

void CCreateStandardReferencesDialog::OnChangeFileSuffix()
{
    UpdateData(TRUE); // Get the values from the UI
    UpdateOutputFileNamesExplanation();
}

void CCreateStandardReferencesDialog::UpdateOutputFileNamesExplanation()
{
    std::stringstream message;
    message << "This will save the following files: ";

    // Regular references
    for (size_t referenceIdx = 0; referenceIdx < m_standardCrossSections->NumberOfReferences(); ++referenceIdx)
    {
        message << std::endl;
        message << ReferenceName(referenceIdx, !m_standardCrossSections->IsAdditionalAbsorber(referenceIdx), false);
    }

    // Fraunhofer reference
    {
        message << std::endl;
        message << FraunhoferReferenceName(false);
    }

    m_outputFileNamesExplanation.Format("%s", message.str().c_str());

    UpdateData(FALSE); // Update the UI

}
