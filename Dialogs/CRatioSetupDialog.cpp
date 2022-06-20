// CRatioSetupDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "afxdialogex.h"
#include "../resource.h"
#include "CRatioSetupDialog.h"
#include <SpectralEvaluation/DialogControllers/RatioCalculationController.h>

#include "../Common/Common.h"

#include "../DlgControls/CGridListCtrlEx/CGridColumnTraitEdit.h"
#include "../DlgControls/CGridListCtrlEx/CGridColumnTraitText.h"
#include "../DlgControls/CGridListCtrlEx/CGridRowTraitXP.h"

// CRatioSetupDialog dialog

IMPLEMENT_DYNAMIC(CRatioSetupDialog, CPropertyPage)

CRatioSetupDialog::CRatioSetupDialog(RatioCalculationController* controller, CWnd* pParent /*=nullptr*/)
    : CPropertyPage(IDD_RATIO_WINDOW_SETUP_DIALOG), m_controller(controller)
{

}

CRatioSetupDialog::~CRatioSetupDialog()
{
    m_controller = nullptr;
}

BOOL CRatioSetupDialog::OnInitDialog() {
    CPropertyPage::OnInitDialog();

    InitReferenceFileControl();

    return TRUE;  // return TRUE unless you set the focus to a control
}

GridListCtrl::CGridColumnTraitImage* CreateCheckBoxColumn(int nStateImageIdx)
{
    auto includeInSO2Column = new GridListCtrl::CGridColumnTraitImage();
    includeInSO2Column->AddImageIndex(nStateImageIdx, _T(""), false);       // Unchecked (and not editable)
    includeInSO2Column->AddImageIndex(nStateImageIdx + 1, _T(""), false);	// Checked (and not editable)
    includeInSO2Column->SetToggleSelection(true);
    return includeInSO2Column;
}

void CRatioSetupDialog::InitReferenceFileControl()
{
    // Create an image list (requred for the check boxes)
    m_ImageList.Create(16, 16, ILC_COLOR16 | ILC_MASK, 1, 0);
    int nStateImageIdx = GridListCtrl::CGridColumnTraitImage::AppendStateImages(m_referencesList, m_ImageList);	// Add checkboxes
    m_referencesList.SetImageList(&m_ImageList, LVSIL_SMALL);

    auto pDefaultRowTrait = new GridListCtrl::CGridRowTraitXP;
    m_referencesList.SetDefaultRowTrait(pDefaultRowTrait);

    // Set the columns of the grid
    m_referencesList.InsertHiddenLabelColumn();	// Requires one never uses column 0
    m_referencesList.InsertColumnTrait(1, "Name", LVCFMT_LEFT, 100, -1, new GridListCtrl::CGridColumnTraitEdit());
    m_referencesList.InsertColumnTrait(2, "Path", LVCFMT_LEFT, 100, -1, new GridListCtrl::CGridColumnTraitEdit());
    m_referencesList.InsertColumnTrait(3, "Include in SO2 Fit", LVCFMT_CENTER, 100, -1, CreateCheckBoxColumn(nStateImageIdx));
    m_referencesList.InsertColumnTrait(4, "Include in BrO Fit", LVCFMT_CENTER, 100, -1, CreateCheckBoxColumn(nStateImageIdx));
    m_referencesList.InsertColumnTrait(5, "Calculate", LVCFMT_CENTER, 100, -1, CreateCheckBoxColumn(nStateImageIdx));

    // Add the default references
    for (int rowIdx = 0; rowIdx < static_cast<int>(m_controller->m_references.size()); ++rowIdx)
    {
        const auto& reference = m_controller->m_references[rowIdx];

        int columnIdx = 0;
        int nItem = m_referencesList.InsertItem(rowIdx, reference.m_name.c_str());
        m_referencesList.SetItemData(nItem, rowIdx);

        // Name
        m_referencesList.SetItemText(nItem, ++columnIdx, reference.m_name.c_str());

        // Path
        m_referencesList.SetItemText(nItem, ++columnIdx, reference.m_path.c_str());

        // Include in SO2 window
        if (reference.m_includeInMajor)
            m_referencesList.SetCellImage(nItem, ++columnIdx, nStateImageIdx + 1);
        else
            m_referencesList.SetCellImage(nItem, ++columnIdx, nStateImageIdx);

        // Include in BrO window
        if (reference.m_includeInMinor)
            m_referencesList.SetCellImage(nItem, ++columnIdx, nStateImageIdx + 1);
        else
            m_referencesList.SetCellImage(nItem, ++columnIdx, nStateImageIdx);

        // Automatically calculate
        if (reference.CanBeAutomaticallyCalculated())
        {
            if (reference.m_automaticallyCalculate)
                m_referencesList.SetCellImage(nItem, ++columnIdx, nStateImageIdx + 1);
            else
                m_referencesList.SetCellImage(nItem, ++columnIdx, nStateImageIdx);
        }
        else
        {
            // m_referencesList
        }
    }
}

void CRatioSetupDialog::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_FITLOW_SO2, m_fitLowSO2);
    DDX_Control(pDX, IDC_EDIT_FITHIGH_SO2, m_fitHighSO2);
    DDX_Control(pDX, IDC_EDIT_FITLOW_BRO, m_fitLowBrO);
    DDX_Control(pDX, IDC_EDIT_FITHIGH_BRO, m_fitHighBrO);
    DDX_Control(pDX, IDC_REFERENCES_LIST, m_referencesList);
}


BEGIN_MESSAGE_MAP(CRatioSetupDialog, CPropertyPage)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_SO2_1, &CRatioSetupDialog::OnBnClickedButtonBrowseSo21)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_O3_1, &CRatioSetupDialog::OnBnClickedButtonBrowseO31)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_RING_1, &CRatioSetupDialog::OnBnClickedButtonBrowseRing1)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_RINGL4_1, &CRatioSetupDialog::OnBnClickedButtonBrowseRingl41)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_BRO_2, &CRatioSetupDialog::OnBnClickedButtonBrowseBro2)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_SO2_2, &CRatioSetupDialog::OnBnClickedButtonBrowseSo22)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_O3_2, &CRatioSetupDialog::OnBnClickedButtonBrowseO32)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_RING_2, &CRatioSetupDialog::OnBnClickedButtonBrowseRing2)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_RINGL4_2, &CRatioSetupDialog::OnBnClickedButtonBrowseRingl42)
END_MESSAGE_MAP()


// CRatioSetupDialog message handlers

// This could be a common filter in the entire program, a commonly used filter.
static const TCHAR* referenceFileFilter = "Reference files\0*.txt;*.xs\0";

void BrowseForReferenceFile(std::unordered_map<StandardDoasSpecie, ReferenceForRatioCalculation>& destination, const StandardDoasSpecie& specie)
{
    CString crossSectionFile;
    if (!Common::BrowseForFile(referenceFileFilter, crossSectionFile))
    {
        return;
    }
    destination[specie].m_path = std::string((LPCSTR)crossSectionFile);
}

void CRatioSetupDialog::OnBnClickedButtonBrowseSo21()
{
    // BrowseForReferenceFile(m_controller->m_so2References, StandardDoasSpecie::SO2);
}

void CRatioSetupDialog::OnBnClickedButtonBrowseO31()
{
    // BrowseForReferenceFile(m_controller->m_so2References, StandardDoasSpecie::O3);
}

void CRatioSetupDialog::OnBnClickedButtonBrowseRing1()
{
    // BrowseForReferenceFile(m_controller->m_so2References, StandardDoasSpecie::RING);
}

void CRatioSetupDialog::OnBnClickedButtonBrowseRingl41()
{
    // BrowseForReferenceFile(m_controller->m_so2References, StandardDoasSpecie::RING_LAMBDA4);
}

void CRatioSetupDialog::OnBnClickedButtonBrowseBro2()
{
    // BrowseForReferenceFile(m_controller->m_broReferences, StandardDoasSpecie::BRO);
}

void CRatioSetupDialog::OnBnClickedButtonBrowseSo22()
{
    // BrowseForReferenceFile(m_controller->m_broReferences, StandardDoasSpecie::SO2);
}

void CRatioSetupDialog::OnBnClickedButtonBrowseO32()
{
    // BrowseForReferenceFile(m_controller->m_broReferences, StandardDoasSpecie::O3);
}

void CRatioSetupDialog::OnBnClickedButtonBrowseRing2()
{
    // BrowseForReferenceFile(m_controller->m_broReferences, StandardDoasSpecie::RING);
}

void CRatioSetupDialog::OnBnClickedButtonBrowseRingl42()
{
    // BrowseForReferenceFile(m_controller->m_broReferences, StandardDoasSpecie::RING_LAMBDA4);
}
