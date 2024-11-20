// CRatioSetupDialog.cpp : implementation file
//

#include "stdafx.h"
#include "../resource.h"
#include "CRatioSetupDialog.h"
#include <SpectralEvaluation/DialogControllers/RatioCalculationController.h>

#include "../Common/Common.h"

#include "../DlgControls/CGridListCtrlEx/CGridColumnTraitEdit.h"
#include "../DlgControls/CGridListCtrlEx/CGridColumnTraitHyperLink.h"
#include "../DlgControls/CGridListCtrlEx/CGridColumnTraitText.h"
#include "../DlgControls/CGridListCtrlEx/CGridRowTraitXP.h"

#undef min
#undef max

#include <algorithm>

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
    , m_minScanAngle(_T(""))
    , m_maxScanAngle(_T(""))
    , m_minSaturationRatio(_T(""))
    , m_maxSaturationRatio(_T(""))
{
}

CRatioSetupDialog::~CRatioSetupDialog()
{
    m_controller = nullptr;
}

BOOL CRatioSetupDialog::OnInitDialog() {
    CPropertyPage::OnInitDialog();

    m_fitTypeCombo.AddString("Polynomial");
    m_fitTypeCombo.AddString("HP Divide by sky");

    m_unitCombo.SetCurSel(0);

    InitReferenceFileControl();

    UpdateFitParametersFromController();

    UpdateDisplayedListOfReferencesPerWindow();

    SetDlgItemText(IDC_STATIC_EXPLANATION, "These settings sets boundaries for which spectra to select as in-plume or out-of-plume spectrum. ");

    if (!m_toolTip.Create(this)) {
        TRACE0("Failed to create tooltip control\n");
    }
    m_toolTip.AddTool(&m_fitTypeCombo, "Select 'HP Divide by sky' if the references are High Pass filtered, otherwise 'Polynomial'");
    m_toolTip.AddTool(&m_unitCombo, "Select the unit the references are scaled to. For correct display of the column values.");
    m_toolTip.AddTool(&m_selectedReferencesSO2, "The following references will be included in the SO2 fit");
    m_toolTip.AddTool(&m_selectedReferencesBrO, "The following references will be included in the BrO fit");

    AddTooltipForControl(IDC_EDIT_FITLOW_SO2, "The shortest wavelength (nm) over which SO2 will be fitted");
    AddTooltipForControl(IDC_EDIT_FITHIGH_SO2, "The longest wavelength (nm) over which SO2 will be fitted");
    AddTooltipForControl(IDC_EDIT_FITLOW_BRO, "The shortest wavelength (nm) over which BrO will be fitted");
    AddTooltipForControl(IDC_EDIT_FITHIGH_BRO, "The longest wavelength (nm) over which BrO will be fitted");
    AddTooltipForControl(IDC_EDIT_POLYNOM, "The order of the polynomial included in the SO2 DOAS fit");
    AddTooltipForControl(IDC_EDIT_POLYNOM2, "The order of the polynomial included in the BrO DOAS fit");
    AddTooltipForControl(IDC_EDIT_MIN_IN_PLUME_SPECTRA, "The minimum required good spectra between the two edges of the plume for a BrO/SO2 ratio to be calculated, default 4");
    AddTooltipForControl(IDC_EDIT_MIN_OUT_OF_PLUME_SPECTRA, "The minimum required spectra out of the plume for a BrO/SO2 ratio to be calculated, default 10");
    AddTooltipForControl(IDC_EDIT_MIN_PLUME_COMPLETENESS, "The minimum completeness of the plume for a ratio to be calculated. Range 0.5 to 1.0, default 0.7");
    AddTooltipForControl(IDC_EDIT_MIN_PLUME_COLUMN_DIFFERENCE, "Minimum required SO2 column difference between in-plume and out-of-plume spectra");
    AddTooltipForControl(IDC_EDIT_MIN_SCAN_ANGLE, "The lowest scan angle for spectra to include. Range is -90 to +90. Default is -75");
    AddTooltipForControl(IDC_EDIT_MAX_SCAN_ANGLE, "The highest scan angle for spectra to include. Range is -90 to +90. Default is +75");
    AddTooltipForControl(IDC_EDIT_MIN_SATURATION_RATIO, "The lowest saturation ratio of spectra to include. Range is 0 to 1, default is 0.12");
    AddTooltipForControl(IDC_EDIT_MAX_SATURATION_RATIO, "The highest saturation ratio of spectra to include. Range is 0 to 1, default is 0.88");

    m_toolTip.SetMaxTipWidth(SHRT_MAX);
    m_toolTip.Activate(TRUE);

    return TRUE;  // return TRUE unless you set the focus to a control
}

BOOL CRatioSetupDialog::OnKillActive()
{
    // Move the focus away from the input-boxes, such that we can validate and save their content.
    m_unitCombo.SetFocus();

    return CPropertyPage::OnKillActive();
}

BOOL CRatioSetupDialog::PreTranslateMessage(MSG* pMsg)
{
    m_toolTip.RelayEvent(pMsg);

    return CPropertyPage::PreTranslateMessage(pMsg);
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

    CRect referenceCtrlRect;
    m_referencesList.GetWindowRect(referenceCtrlRect);
    const int defaultColumnWidth = 100; // the width of all the columns, except for the path
    const int pathColumnWidth = referenceCtrlRect.Width() - 4 * defaultColumnWidth - 10; // the width of the path, expands to (almost) the entire available width of the control

    // Set the columns of the grid
    m_referencesList.InsertHiddenLabelColumn();	// Requires one never uses column 0
    m_referencesList.InsertColumnTrait(1, "Name", LVCFMT_LEFT, defaultColumnWidth, -1, new GridListCtrl::CGridColumnTraitEdit());
    m_referencesList.InsertColumnTrait(2, "Path", LVCFMT_LEFT, pathColumnWidth, -1, CreateLinkColumn());
    m_referencesList.InsertColumnTrait(3, "Include in SO2 Fit", LVCFMT_CENTER, defaultColumnWidth, -1, CreateCheckBoxColumn(nStateImageIdx));
    m_referencesList.InsertColumnTrait(4, "Include in BrO Fit", LVCFMT_CENTER, defaultColumnWidth, -1, CreateCheckBoxColumn(nStateImageIdx));
    m_referencesList.InsertColumnTrait(5, "Calculate", LVCFMT_CENTER, defaultColumnWidth, -1, CreateCheckBoxColumn(nStateImageIdx));

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
    DDX_Text(pDX, IDC_EDIT_MIN_IN_PLUME_SPECTRA, m_minInPlumeSpectrumNumber);
    DDX_Text(pDX, IDC_EDIT_MIN_OUT_OF_PLUME_SPECTRA, m_minOutOfPlumeSpectrumNumber);
    DDX_Text(pDX, IDC_EDIT_MIN_PLUME_COMPLETENESS, m_minPlumeCompleteness);
    DDX_Text(pDX, IDC_EDIT_MIN_PLUME_COLUMN_DIFFERENCE, m_minInPlumeSpectrumColumnDifference);
    DDX_Text(pDX, IDC_EDIT_MIN_SCAN_ANGLE, m_minScanAngle);
    DDX_Text(pDX, IDC_EDIT_MAX_SCAN_ANGLE, m_maxScanAngle);
    DDX_Text(pDX, IDC_EDIT_MIN_SATURATION_RATIO, m_minSaturationRatio);
    DDX_Text(pDX, IDC_EDIT_MAX_SATURATION_RATIO, m_maxSaturationRatio);

    DDX_Check(pDX, IDC_CHECK_RATIO_REQUIRE_TWO_PLUME_EDGES, m_requireVisiblePlumeEdges);

    DDX_Control(pDX, IDC_LIST_REFERENCES_SO2, m_selectedReferencesSO2);
    DDX_Control(pDX, IDC_LIST_REFERENCES_BRO, m_selectedReferencesBrO);
    DDX_Control(pDX, IDC_COMBO_FIT_TYPE, m_fitTypeCombo);
    DDX_Control(pDX, IDC_COMBO_REFERENCE_UNIT, m_unitCombo);
}

BEGIN_MESSAGE_MAP(CRatioSetupDialog, CPropertyPage)
    ON_NOTIFY(LVN_ENDLABELEDIT, IDC_REFERENCES_LIST, OnClickInReferenceList)
    ON_EN_KILLFOCUS(IDC_EDIT_FITLOW_SO2, &CRatioSetupDialog::OnKillfocusEditBox)
    ON_EN_KILLFOCUS(IDC_EDIT_FITHIGH_SO2, &CRatioSetupDialog::OnKillfocusEditBox)
    ON_EN_KILLFOCUS(IDC_EDIT_POLYNOM, &CRatioSetupDialog::OnKillfocusEditBox)

    ON_EN_KILLFOCUS(IDC_EDIT_FITLOW_BRO, &CRatioSetupDialog::OnKillfocusEditBox)
    ON_EN_KILLFOCUS(IDC_EDIT_FITHIGH_BRO, &CRatioSetupDialog::OnKillfocusEditBox)
    ON_EN_KILLFOCUS(IDC_EDIT_POLYNOM2, &CRatioSetupDialog::OnKillfocusEditBox)

    ON_EN_KILLFOCUS(IDC_EDIT_MIN_IN_PLUME_SPECTRA, &CRatioSetupDialog::OnKillfocusEditBox)
    ON_EN_KILLFOCUS(IDC_EDIT_MIN_OUT_OF_PLUME_SPECTRA, &CRatioSetupDialog::OnKillfocusEditBox)
    ON_EN_KILLFOCUS(IDC_EDIT_MIN_PLUME_COMPLETENESS, &CRatioSetupDialog::OnKillfocusEditBox)
    ON_EN_KILLFOCUS(IDC_EDIT_MIN_SCAN_ANGLE, &CRatioSetupDialog::OnKillfocusEditBox)
    ON_EN_KILLFOCUS(IDC_EDIT_MAX_SCAN_ANGLE, &CRatioSetupDialog::OnKillfocusEditBox)
    ON_EN_KILLFOCUS(IDC_EDIT_MIN_SATURATION_RATIO, &CRatioSetupDialog::OnKillfocusEditBox)
    ON_EN_KILLFOCUS(IDC_EDIT_MAX_SATURATION_RATIO, &CRatioSetupDialog::OnKillfocusEditBox)

    ON_CBN_SELCHANGE(IDC_COMBO_FIT_TYPE, &CRatioSetupDialog::OnSelchangeComboFitType)
    ON_CBN_SELCHANGE(IDC_COMBO_REFERENCE_UNIT, &CRatioSetupDialog::OnSelchangeComboReferenceUnit)
    ON_BN_CLICKED(IDC_CHECK_RATIO_REQUIRE_TWO_PLUME_EDGES, &CRatioSetupDialog::OnCheckChangeRatioRequireTwoPlumeEdges)
END_MESSAGE_MAP()


// CRatioSetupDialog message handlers

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
    bool changedSettings = false;

    if (colIdx == ReferenceListColumns::INCLUDE_SO2)
    {
        const bool newValue = (imgIdx == 1);
        changedSettings = m_controller->m_references[rowIdx].m_includeInMajor != newValue;
        m_controller->m_references[rowIdx].m_includeInMajor = newValue;
    }
    else if (colIdx == ReferenceListColumns::INCLUDE_BRO)
    {
        const bool newValue = (imgIdx == 1);
        changedSettings = m_controller->m_references[rowIdx].m_includeInMinor != newValue;
        m_controller->m_references[rowIdx].m_includeInMinor = (imgIdx == 1);
    }
    else if (colIdx == ReferenceListColumns::AUTOCALCULATE && m_controller->m_references[rowIdx].CanBeAutomaticallyCalculated())
    {
        const bool newValue = (imgIdx == 1);
        changedSettings = m_controller->m_references[rowIdx].m_automaticallyCalculate != newValue;
        m_controller->m_references[rowIdx].m_automaticallyCalculate = (imgIdx == 1);
    }

    if (changedSettings)
    {
        m_controller->ResetResults();
    }

    UpdateDisplayedListOfReferencesPerWindow();
}

void CRatioSetupDialog::BrowseForReference(int referenceIdx)
{
    const int colIdx = ReferenceListColumns::PATH;

    // Let the user browse for a reference file
    CString crossSectionFile;
    if (!Common::BrowseForReferenceFile(crossSectionFile))
    {
        return;
    }

    // Update the model
    const std::string newValue = std::string((LPCSTR)crossSectionFile);
    const bool changedSettings = m_controller->m_references[referenceIdx].m_path != newValue;
    m_controller->m_references[referenceIdx].m_path = newValue;

    if (changedSettings)
    {
        m_controller->ResetResults();
    }

    // Update the UI
    m_referencesList.SetItemText(referenceIdx, colIdx, m_controller->m_references[referenceIdx].m_path.c_str());
    if (m_controller->m_references[referenceIdx].m_automaticallyCalculate)
    {
        m_referencesList.SetCellImage(referenceIdx, ReferenceListColumns::AUTOCALCULATE, 0);
    }
    UpdateDisplayedListOfReferencesPerWindow();
}

static novac::WavelengthRange ParseWavelengthRange(const char* low, const char* high)
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

    bool changedSettings = false;

    // Get the new values from the UI and save to the controller.
    const auto newSO2Range = ParseWavelengthRange((LPCSTR)m_fitLowSO2, (LPCSTR)m_fitHighSO2);
    changedSettings = changedSettings || !newSO2Range.Equals(m_controller->m_so2FitRange, 0.05);
    m_controller->m_so2FitRange = newSO2Range;

    const auto newBroRange = ParseWavelengthRange((LPCSTR)m_fitLowBrO, (LPCSTR)m_fitHighBrO);
    changedSettings = changedSettings || !newBroRange.Equals(m_controller->m_broFitRange, 0.05);
    m_controller->m_broFitRange = newBroRange;

    const auto newPolyOrderSO2 = std::atoi((LPCSTR)m_polyOrderSO2);
    changedSettings = changedSettings || newPolyOrderSO2 != m_controller->m_so2PolynomialOrder;
    m_controller->m_so2PolynomialOrder = newPolyOrderSO2;

    const auto newPolyOrderBrO = std::atoi((LPCSTR)m_polyOrderBrO);
    changedSettings = changedSettings || newPolyOrderBrO != m_controller->m_broPolynomialOrder;
    m_controller->m_broPolynomialOrder = newPolyOrderBrO;

    const auto newMinNumberOfSpectraInPlume = std::max(1, std::atoi((LPCSTR)m_minInPlumeSpectrumNumber));
    changedSettings = changedSettings || newMinNumberOfSpectraInPlume != m_controller->m_ratioEvaluationSettings.minNumberOfSpectraInPlume;
    m_controller->m_ratioEvaluationSettings.minNumberOfSpectraInPlume = newMinNumberOfSpectraInPlume;

    const auto newNumberOfSpectraOutsideOfPlume = std::max(1, std::atoi((LPCSTR)m_minOutOfPlumeSpectrumNumber));
    changedSettings = changedSettings || newNumberOfSpectraOutsideOfPlume != m_controller->m_ratioEvaluationSettings.numberOfSpectraOutsideOfPlume;
    m_controller->m_ratioEvaluationSettings.numberOfSpectraOutsideOfPlume = newNumberOfSpectraOutsideOfPlume;

    const auto newMinPlumeCompleteness = std::max(0.5, std::min(1.0, std::atof((LPCSTR)m_minPlumeCompleteness)));
    changedSettings = changedSettings || newMinPlumeCompleteness != m_controller->m_ratioEvaluationSettings.minimumPlumeCompleteness;
    m_controller->m_ratioEvaluationSettings.minimumPlumeCompleteness = newMinPlumeCompleteness;

    const auto newMinimumInPlumeColumnDifference = std::atof((LPCSTR)m_minInPlumeSpectrumColumnDifference);
    changedSettings = changedSettings || newMinimumInPlumeColumnDifference != m_controller->m_ratioEvaluationSettings.minimumInPlumeColumnDifference;
    m_controller->m_ratioEvaluationSettings.minimumInPlumeColumnDifference = newMinimumInPlumeColumnDifference;

    const auto newMinScanAngle = std::atof((LPCSTR)m_minScanAngle);
    changedSettings = changedSettings || newMinScanAngle != m_controller->m_ratioEvaluationSettings.minimumScanAngle;
    m_controller->m_ratioEvaluationSettings.minimumScanAngle = newMinScanAngle;

    const auto newMaxScanAngle = std::atof((LPCSTR)m_maxScanAngle);
    changedSettings = changedSettings || newMaxScanAngle != m_controller->m_ratioEvaluationSettings.maximumScanAngle;
    m_controller->m_ratioEvaluationSettings.maximumScanAngle = newMaxScanAngle;

    const auto newMinSaturationRatio = std::atof((LPCSTR)m_minSaturationRatio);
    changedSettings = changedSettings || newMinSaturationRatio != m_controller->m_ratioEvaluationSettings.minSaturationRatio;
    m_controller->m_ratioEvaluationSettings.minSaturationRatio = newMinSaturationRatio;

    const auto newMaxSaturationRatio = std::atof((LPCSTR)m_maxSaturationRatio);
    changedSettings = changedSettings || newMaxSaturationRatio != m_controller->m_ratioEvaluationSettings.maxSaturationRatio;
    m_controller->m_ratioEvaluationSettings.maxSaturationRatio = newMaxSaturationRatio;

    if (changedSettings)
    {
        m_controller->ResetResults();
    }

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

    m_minScanAngle.Format("%.1lf", m_controller->m_ratioEvaluationSettings.minimumScanAngle);
    m_maxScanAngle.Format("%.1lf", m_controller->m_ratioEvaluationSettings.maximumScanAngle);

    m_minSaturationRatio.Format("%.2lf", m_controller->m_ratioEvaluationSettings.minSaturationRatio);
    m_maxSaturationRatio.Format("%.2lf", m_controller->m_ratioEvaluationSettings.maxSaturationRatio);

    m_minInPlumeSpectrumNumber.Format("%d", m_controller->m_ratioEvaluationSettings.minNumberOfSpectraInPlume);
    m_minOutOfPlumeSpectrumNumber.Format("%d", m_controller->m_ratioEvaluationSettings.numberOfSpectraOutsideOfPlume);
    m_minPlumeCompleteness.Format("%.1lf", m_controller->m_ratioEvaluationSettings.minimumPlumeCompleteness);
    m_requireVisiblePlumeEdges = m_controller->m_ratioEvaluationSettings.requireVisiblePlumeEdges;

    if (m_controller->m_crossSectionUnit == novac::CrossSectionUnit::ppmm)
    {
        m_minInPlumeSpectrumColumnDifference.Format("%.0f", m_controller->m_ratioEvaluationSettings.minimumInPlumeColumnDifference);
        SetDlgItemText(IDC_STATIC_UNIT_MIN_IN_PLUME_COLUMN, "[ppmm]");
    }
    else
    {
        m_minInPlumeSpectrumColumnDifference.Format("%.2g", m_controller->m_ratioEvaluationSettings.minimumInPlumeColumnDifference);
        SetDlgItemText(IDC_STATIC_UNIT_MIN_IN_PLUME_COLUMN, "[molec/cm2]");
    }

    switch (m_controller->m_doasFitType)
    {
    case novac::FIT_TYPE::FIT_POLY:
        m_fitTypeCombo.SetCurSel(0);
        break;
    case novac::FIT_TYPE::FIT_HP_DIV:
        m_fitTypeCombo.SetCurSel(1);
        break;
    case novac::FIT_TYPE::FIT_HP_SUB:
        m_fitTypeCombo.SetCurSel(2);
        break;
    }

    switch (m_controller->m_crossSectionUnit)
    {
    case novac::CrossSectionUnit::ppmm:
        m_unitCombo.SetCurSel(0);
        break;
    default:
        m_unitCombo.SetCurSel(1);
        break;
    }

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

void CRatioSetupDialog::OnSelchangeComboFitType()
{
    const int selection = m_fitTypeCombo.GetCurSel();
    bool changedSettings = false;

    switch (selection)
    {
    case 1:
        changedSettings = m_controller->m_doasFitType != novac::FIT_TYPE::FIT_HP_DIV;
        m_controller->m_doasFitType = novac::FIT_TYPE::FIT_HP_DIV;
        break;
    case 2:
        changedSettings = m_controller->m_doasFitType != novac::FIT_TYPE::FIT_HP_SUB;
        m_controller->m_doasFitType = novac::FIT_TYPE::FIT_HP_SUB;
        break;
    default:
        changedSettings = m_controller->m_doasFitType != novac::FIT_TYPE::FIT_POLY;
        m_controller->m_doasFitType = novac::FIT_TYPE::FIT_POLY;
        break;
    }

    if (changedSettings)
    {
        m_controller->ResetResults();
    }
}

void CRatioSetupDialog::OnSelchangeComboReferenceUnit()
{
    const int selection = m_unitCombo.GetCurSel();

    switch (selection)
    {
    case 1:
        // User selected molec/cm2
        m_controller->m_crossSectionUnit = novac::CrossSectionUnit::cm2_molecule;
        break;
    case 0:
        // User selected ppmm
        m_controller->m_crossSectionUnit = novac::CrossSectionUnit::ppmm;
    }

    // This does not reset the results of the controller as this is basically just for display and doesn't affect the results.

    UpdateFitParametersFromController();
}

void CRatioSetupDialog::AddTooltipForControl(int dialogItemId, const char* toolTipText)
{
    auto item = GetDlgItem(dialogItemId);

    if (item != nullptr)
    {
        m_toolTip.AddTool(item, toolTipText);
    }
}

void CRatioSetupDialog::OnCheckChangeRatioRequireTwoPlumeEdges()
{
    // Update data from the UI
    UpdateData(TRUE);

    const bool newValue = (TRUE == m_requireVisiblePlumeEdges);
    const bool changedSettings = m_controller->m_ratioEvaluationSettings.requireVisiblePlumeEdges != newValue;
    m_controller->m_ratioEvaluationSettings.requireVisiblePlumeEdges = newValue;

    if (changedSettings)
    {
        m_controller->ResetResults();
    }

    // Update the UI
    UpdateFitParametersFromController();
}
