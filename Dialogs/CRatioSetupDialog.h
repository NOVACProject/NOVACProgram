#pragma once
#include "afxdialogex.h"
#include "../DlgControls/CGridListCtrlEx/CGridListCtrlEx.h"

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
    CListBox m_selectedReferencesSO2;
    CListBox m_selectedReferencesBrO;

    // Updates th m_fitLow... and m_fitHhigh...
    void UpdateFitParametersFromController();

    void UpdateDisplayedListOfReferencesPerWindow();

    void BrowseForReference(int referenceIdx);
};
