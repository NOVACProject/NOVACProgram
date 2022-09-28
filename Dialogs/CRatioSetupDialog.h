#pragma once
#include <afxdlgs.h>
#include "../DlgControls/CGridListCtrlEx/CGridListCtrlEx.h"

#include <string>

class RatioCalculationController;

// CRatioSetupDialog is the second page in the Ratio calculation dialog
// and makes it possible for the user to setup the properties for the DOAS fits.

class CRatioSetupDialog : public CPropertyPage
{
    DECLARE_DYNAMIC(CRatioSetupDialog)

public:
    CRatioSetupDialog(RatioCalculationController* controller, CWnd* pParent = nullptr);   // standard constructor
    virtual ~CRatioSetupDialog();

    /** Initializes the controls and the dialog */
    virtual BOOL OnInitDialog();

    /** Called when this is about to become inactive */
    virtual BOOL OnKillActive();

    /** Handling the tool tips */
    virtual BOOL PreTranslateMessage(MSG* pMsg);

    // Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_RATIO_WINDOW_SETUP_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()

private:

    afx_msg void OnClickInReferenceList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnKillfocusEditBox();
    afx_msg void OnSelchangeComboFitType();
    afx_msg void OnSelchangeComboReferenceUnit();
    afx_msg void OnCheckChangeRatioRequireTwoPlumeEdges();

    // The controller which this dialog helps to setup. Notice that this dialog does not own the pointer and will not delete it.
    RatioCalculationController* m_controller;

    // Sets up the m_referenceGrid
    void InitReferenceFileControl();

    /** The reference grid, enables the user to select reference files.
        This also makes it possible to select which fit window the references should be included in. */
    GridListCtrl::CGridListCtrlEx m_referencesList;

    enum ReferenceListColumns
    {
        NAME = 1,
        PATH,
        INCLUDE_SO2,
        INCLUDE_BRO,
        AUTOCALCULATE
    };

    CImageList m_ImageList;

    // Saving the fit ranges from the user, in nano meters.
    CString m_fitLowSO2;
    CString m_fitHighSO2;
    CString m_fitLowBrO;
    CString m_fitHighBrO;
    CString m_polyOrderSO2;
    CString m_polyOrderBrO;
    CString m_minInPlumeSpectrumColumn;
    CString m_minInPlumeSpectrumNumber;
    CString m_minOutOfPlumeSpectrumNumber;
    CString m_minInPlumeSpectrumColumnDifference;
    CString m_minPlumeCompleteness;
    CString m_minScanAngle;
    CString m_maxScanAngle;
    CString m_minSaturationRatio;
    CString m_maxSaturationRatio;

    BOOL m_requireVisiblePlumeEdges;

    CListBox m_selectedReferencesSO2;
    CListBox m_selectedReferencesBrO;
    CComboBox m_fitTypeCombo;
    CComboBox m_unitCombo;

    CToolTipCtrl m_toolTip;

    // Updates th m_fitLow... and m_fitHigh...
    void UpdateFitParametersFromController();

    void UpdateDisplayedListOfReferencesPerWindow();

    void BrowseForReference(int referenceIdx);

    void AddTooltipForControl(int dialogItemId, const char* toolTipText);
};
