#pragma once

#include "../DlgControls/ReferenceFileControl.h"
#include "FitWindowListBox.h"
#include "afxwin.h"

// CReEval_WindowDlg dialog

namespace ReEvaluation
{
class CReEvaluator;

class CReEval_WindowDlg : public CPropertyPage
{
    DECLARE_DYNAMIC(CReEval_WindowDlg)

public:
    CReEval_WindowDlg(CReEvaluator& reeval);
    virtual ~CReEval_WindowDlg();

    // Dialog Data
    enum { IDD = IDD_REEVAL_WINDOW };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()

private:

    // --------------------------- PUBLIC DATA -------------------------- 

    /** The reference grid, enables the user to select reference files */
    DlgControls::CReferenceFileControl m_referenceGrid;

    /** The fit window list */
    CFitWindowListBox m_windowList;

    /** The electronic combo-box */
    CComboBox m_electronicCombo;

    /** The frame for defining the size of the reference file control */
    CStatic m_referenceFrame;

    /** The properties button */
    CButton m_btnRefProp;

    /** The insert new reference button */
    CButton m_btnInsertRef;

    /** The remove reference button */
    CButton m_btnRemoveRef;

    /** The 'UV' checkbox */
    CButton m_checkUV;

    /** The 'find optimal shift' checkbox */
    CButton m_checkFindOptimalShift;

    /** The 'shift-sky' checkbox */
    CButton m_checkShiftSky;

    /** A handle to the reevaluator */
    CReEvaluator& m_reeval;

    CString m_fraunhoferReferenceName;

    // This is a compatibility thing, where the FitWindow used to have a 'bool UV' flag
    // indicating the range of pixels used for offset removal.
    // This is now replaced with the 'offsetRange' which are explicitly setting the range of pixels.
    // Until the UI has been updated here, we need to keep this boolean...
    BOOL m_useUVOffsetRemovalRange = TRUE;

    // --------------------------- PRIVATE METHODS --------------------------

    /** Initialize the dialog and its controls */
    virtual BOOL OnInitDialog();

    /** Initialize the reference file control */
    void InitReferenceFileControl();

    /** Populate the reference file control */
    void PopulateReferenceFileControl();

    /** Updates the controls */
    void UpdateControls();

    /** Called when the user has changed the selected fit window */
    afx_msg void OnChangeFitWindow();

    /** Called when the user wants to insert a new reference file */
    afx_msg void OnInsertReference();

    /** Called when the user wants to remove a reference file */
    afx_msg void OnRemoveReference();

    /** Called when the user wants to inspect the properties of a reference file */
    afx_msg void OnShowPropertiesWindow();

    /** Called when the user wants to view reference graph */
    afx_msg void OnShowReferenceGraph();

    /** Saves the data the user typed in to the user interface */
    afx_msg void SaveData();

    /** Lets the user browse for a solar-spectrum */
    afx_msg void OnBrowseSolarSpectrum();

};
}