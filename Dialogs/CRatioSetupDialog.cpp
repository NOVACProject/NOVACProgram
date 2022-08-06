// CRatioSetupDialog.cpp : implementation file
//

#include "StdAfx.h"
#include "afxdialogex.h"
#include "../resource.h"
#include "CRatioSetupDialog.h"
#include <SpectralEvaluation/DialogControllers/RatioCalculationController.h>

#include "../Common/Common.h"

#include "../DlgControls/CGridListCtrlEx/CGridColumnTraitEdit.h"
#include "../DlgControls/CGridListCtrlEx/CGridColumnTraitHyperLink.h"
#include "../DlgControls/CGridListCtrlEx/CGridColumnTraitText.h"
#include "../DlgControls/CGridListCtrlEx/CGridRowTraitXP.h"

// CRatioSetupDialog dialog

IMPLEMENT_DYNAMIC(CRatioSetupDialog, CPropertyPage)

CRatioSetupDialog::CRatioSetupDialog(RatioCalculationController* controller, CWnd* pParent /*=nullptr*/)
    : CPropertyPage(IDD_RATIO_WINDOW_SETUP_DIALOG), m_controller(controller)
    , m_fitLowSO2(_T(""))
    , m_fitHighSO2(_T(""))
    , m_fitLowBrO(_T(""))
    , m_fitHighBrO(_T(""))
    , m_polyOrderSO2(_T(""))
    , m_polyOrderBrO(_T(""))
{

}

CRatioSetupDialog::~CRatioSetupDialog()
{
    m_controller = nullptr;
}

BOOL CRatioSetupDialog::OnInitDialog() {
    CPropertyPage::OnInitDialog();

    InitReferenceFileControl();

    UpdateFitParametersFromController();

    UpdateDisplayedListOfReferencesPerWindow();

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

GridListCtrl::CGridColumnTraitHyperLink* CreateLinkColumn()
{
    auto pathCol = new GridListCtrl::CGridColumnTraitHyperLink();
    pathCol->SetShellOperation("");
    pathCol->SetShellApplication("");
    pathCol->SetShellFilePrefix("");
    return pathCol;
}

void CRatioSetupDialog::InitReferenceFileControl()
{
    // Create an image list (requred for the check boxes)
    m_ImageList.Create(16, 16, ILC_COLOR16 | ILC_MASK, 1, 0);
    int nStateImageIdx = GridListCtrl::CGridColumnTraitImage::AppendStateImages(m_referencesList, m_ImageList);	// Add checkboxes
    m_referencesList.SetImageList(&m_ImageList, LVSIL_SMALL);

    // auto pDefaultRowTrait = new GridListCtrl::CGridRowTraitXP;
    // m_referencesList.SetDefaultRowTrait(pDefaultRowTrait);

    // Set the columns of the grid
    m_referencesList.InsertHiddenLabelColumn();	// Requires one never uses column 0
    m_referencesList.InsertColumnTrait(1, "Name", LVCFMT_LEFT, 100, -1, new GridListCtrl::CGridColumnTraitEdit());
    m_referencesList.InsertColumnTrait(2, "Path", LVCFMT_LEFT, 100, -1, CreateLinkColumn());
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
        m_referencesList.SetItemText(nItem, ReferenceListColumns::NAME, reference.m_name.c_str());

        // Path
        if (reference.m_path.length() == 0)
        {
            m_referencesList.SetItemText(nItem, ReferenceListColumns::PATH, "Not selected");
        }
        else
        {
            m_referencesList.SetItemText(nItem, ReferenceListColumns::PATH, reference.m_path.c_str());
        }

        // Include in SO2 window
        if (reference.m_includeInMajor)
            m_referencesList.SetCellImage(nItem, ReferenceListColumns::INCLUDE_SO2, nStateImageIdx + 1);
        else
            m_referencesList.SetCellImage(nItem, ReferenceListColumns::INCLUDE_SO2, nStateImageIdx);

        // Include in BrO window
        if (reference.m_includeInMinor)
            m_referencesList.SetCellImage(nItem, ReferenceListColumns::INCLUDE_BRO, nStateImageIdx + 1);
        else
            m_referencesList.SetCellImage(nItem, ReferenceListColumns::INCLUDE_BRO, nStateImageIdx);

        // Automatically calculate
        if (reference.CanBeAutomaticallyCalculated())
        {
            if (reference.m_automaticallyCalculate)
                m_referencesList.SetCellImage(nItem, ReferenceListColumns::AUTOCALCULATE, nStateImageIdx + 1);
            else
                m_referencesList.SetCellImage(nItem, ReferenceListColumns::AUTOCALCULATE, nStateImageIdx);
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
    DDX_Control(pDX, IDC_REFERENCES_LIST, m_referencesList);
    DDX_Text(pDX, IDC_EDIT_FITLOW_SO2, m_fitLowSO2);
    DDX_Text(pDX, IDC_EDIT_FITHIGH_SO2, m_fitHighSO2);
    DDX_Text(pDX, IDC_EDIT_FITLOW_BRO, m_fitLowBrO);
    DDX_Text(pDX, IDC_EDIT_FITHIGH_BRO, m_fitHighBrO);
    DDX_Text(pDX, IDC_EDIT_POLYNOM, m_polyOrderSO2);
    DDX_Text(pDX, IDC_EDIT_POLYNOM2, m_polyOrderBrO);
    DDX_Control(pDX, IDC_LIST_REFERENCES_SO2, m_selectedReferencesSO2);
    DDX_Control(pDX, IDC_LIST_REFERENCES_BRO, m_selectedReferencesBrO);
}

BEGIN_MESSAGE_MAP(CRatioSetupDialog, CPropertyPage)
    ON_NOTIFY(LVN_ENDLABELEDIT, IDC_REFERENCES_LIST, OnClickInReferenceList)
    ON_EN_KILLFOCUS(IDC_EDIT_FITLOW_SO2, &CRatioSetupDialog::OnKillfocusEditBox)
    ON_EN_KILLFOCUS(IDC_EDIT_FITHIGH_SO2, &CRatioSetupDialog::OnKillfocusEditBox)
    ON_EN_KILLFOCUS(IDC_EDIT_POLYNOM, &CRatioSetupDialog::OnKillfocusEditBox)

    ON_EN_KILLFOCUS(IDC_EDIT_FITLOW_BRO, &CRatioSetupDialog::OnKillfocusEditBox)
    ON_EN_KILLFOCUS(IDC_EDIT_FITHIGH_BRO, &CRatioSetupDialog::OnKillfocusEditBox)
    ON_EN_KILLFOCUS(IDC_EDIT_POLYNOM2, &CRatioSetupDialog::OnKillfocusEditBox)

END_MESSAGE_MAP()


// CRatioSetupDialog message handlers

// This could be a common filter in the entire program, a commonly used filter.
static const TCHAR* referenceFileFilter = "Reference files\0*.txt;*.xs\0";

void CRatioSetupDialog::OnClickInReferenceList(NMHDR* pNMHDR, LRESULT* pResult)
{
    LV_DISPINFO* listViewInfo = reinterpret_cast<LV_DISPINFO*>(pNMHDR);
    const int rowIdx = listViewInfo->item.iItem;
    const int colIdx = listViewInfo->item.iSubItem;

    if (rowIdx < 0 || rowIdx >= static_cast<int>(m_controller->m_references.size()))
    {
        // Invalid data...
        return;
    }
    if (colIdx == ReferenceListColumns::PATH)
    {
        BrowseForReference(rowIdx);
        return;
    }

    // Not selecting a path, possibly just checking a box. Get the status of the check-boxes in the UI
    const int imgIdx = m_referencesList.GetCellImage(rowIdx, colIdx);

    if (colIdx == ReferenceListColumns::INCLUDE_SO2)
    {
        m_controller->m_references[rowIdx].m_includeInMajor = (imgIdx == 1);
    }
    else if (colIdx == ReferenceListColumns::INCLUDE_BRO)
    {
        m_controller->m_references[rowIdx].m_includeInMinor = (imgIdx == 1);
    }
    else if (colIdx == ReferenceListColumns::AUTOCALCULATE && m_controller->m_references[rowIdx].CanBeAutomaticallyCalculated())
    {
        m_controller->m_references[rowIdx].m_automaticallyCalculate = (imgIdx == 1);
    }

    UpdateDisplayedListOfReferencesPerWindow();
 }

void CRatioSetupDialog::BrowseForReference(int referenceIdx)
{
    const int colIdx = ReferenceListColumns::PATH;

    // Let the user browse for a reference file
    CString crossSectionFile;
    if (!Common::BrowseForFile(referenceFileFilter, crossSectionFile))
    {
        return;
    }

    // Update the model
    m_controller->m_references[referenceIdx].m_path = std::string((LPCSTR)crossSectionFile);

    // Update the UI
    m_referencesList.SetItemText(referenceIdx, colIdx, m_controller->m_references[referenceIdx].m_path.c_str());
    if (m_controller->m_references[referenceIdx].m_automaticallyCalculate)
    {
        m_referencesList.SetCellImage(referenceIdx, ReferenceListColumns::AUTOCALCULATE, 0);
    }
    UpdateDisplayedListOfReferencesPerWindow();
}

novac::WavelengthRange ParseWavelengthRange(const char* low, const char* high)
{
    // Attempt to parse in the current locale
    double fitLow = std::atof(low);
    double fitHigh = std::atof(high);

    return novac::WavelengthRange(fitLow, fitHigh);
}

void CRatioSetupDialog::OnKillfocusEditBox()
{
    // Get the values from the UI
    UpdateData(TRUE);

    // Update the controller
    m_controller->m_so2FitRange = ParseWavelengthRange((LPCSTR)m_fitLowSO2, (LPCSTR)m_fitHighSO2);
    m_controller->m_broFitRange = ParseWavelengthRange((LPCSTR)m_fitLowBrO, (LPCSTR)m_fitHighBrO);
    m_controller->m_so2PolynomialOrder = std::atoi((LPCSTR)m_polyOrderSO2);
    m_controller->m_broPolynomialOrder = std::atoi((LPCSTR)m_polyOrderBrO);

    UpdateFitParametersFromController();
}

void CRatioSetupDialog::UpdateFitParametersFromController()
{
    m_fitLowSO2.Format("%.1lf", m_controller->m_so2FitRange.low);
    m_fitHighSO2.Format("%.1lf", m_controller->m_so2FitRange.high);
    m_polyOrderSO2.Format("%d", m_controller->m_so2PolynomialOrder);

    m_fitLowBrO.Format("%.1lf", m_controller->m_broFitRange.low);
    m_fitHighBrO.Format("%.1lf", m_controller->m_broFitRange.high);
    m_polyOrderBrO.Format("%d", m_controller->m_broPolynomialOrder);

    // Update the UI with the values
    UpdateData(FALSE);
}

void CRatioSetupDialog::UpdateDisplayedListOfReferencesPerWindow()
{
    m_selectedReferencesSO2.ResetContent();
    m_selectedReferencesBrO.ResetContent();

    for (size_t idx = 0; idx < m_controller->m_references.size(); ++idx)
    {
        if (m_controller->m_references[idx].m_path == "" && !m_controller->m_references[idx].m_automaticallyCalculate)
        {
            continue;
        }

        if (m_controller->m_references[idx].m_includeInMajor)
        {
            m_selectedReferencesSO2.AddString(CString(m_controller->m_references[idx].m_name.c_str()));
        }

        if (m_controller->m_references[idx].m_includeInMinor)
        {
            m_selectedReferencesBrO.AddString(CString(m_controller->m_references[idx].m_name.c_str()));
        }
    }
}