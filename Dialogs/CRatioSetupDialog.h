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

    CEdit m_fitLowSO2;
    CEdit m_fitHighSO2;
    CEdit m_fitLowBrO;
    CEdit m_fitHighBrO;

    // The browse for reference files buttons. The ones ending with '1' belongs to the SO2 window, the ones ending with '2' belongs to BrO
    afx_msg void OnBnClickedButtonBrowseSo21();
    afx_msg void OnBnClickedButtonBrowseO31();
    afx_msg void OnBnClickedButtonBrowseRing1();
    afx_msg void OnBnClickedButtonBrowseRingl41();
    afx_msg void OnBnClickedButtonBrowseBro2();
    afx_msg void OnBnClickedButtonBrowseSo22();
    afx_msg void OnBnClickedButtonBrowseO32();
    afx_msg void OnBnClickedButtonBrowseRing2();
    afx_msg void OnBnClickedButtonBrowseRingl42();

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()

private:

    // The controller which this dialog helps to setup. Notice that this dialog does not own the pointer and will not delete it.
    RatioCalculationController* m_controller;

    // Sets up the m_referenceGrid
    void InitReferenceFileControl();

public:

    /** The reference grid, enables the user to select reference files */
    GridListCtrl::CGridListCtrlEx m_referencesList;

    CImageList m_ImageList;
};
