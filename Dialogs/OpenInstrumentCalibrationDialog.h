#pragma once

#include "afxdlgs.h"

#include <SpectralEvaluation/DialogControllers/InstrumentCalibrationInput.h>

// OpenInstrumentCalibrationDialog dialog

class OpenInstrumentCalibrationDialog : public CDialog
{
    DECLARE_DYNAMIC(OpenInstrumentCalibrationDialog)

public:
    OpenInstrumentCalibrationDialog(CWnd* pParent = nullptr);   // standard constructor
    virtual ~OpenInstrumentCalibrationDialog();

    /** Initializes the controls and the dialog */
    virtual BOOL OnInitDialog();

    // Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_CALIBRATE_OPEN_CALIBRATION };
#endif

    /// <summary>
    /// This is the setup which the user has selected
    /// </summary>
    InstrumentCalibrationInput m_state;

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()
public:
    CComboBox m_comboDataType;

    CStatic m_labelCalibration;

    CStatic m_labelInstrumentLineShape;
    CButton m_buttonBrowseInstrumentLineShape;

    afx_msg void OnButtonBrowseLineShape();
    afx_msg void OnButtonBrowseCalibration();
    afx_msg void OnSelchangeDataType();

private:
    char* m_initialCalibrationFileTypeFilter = nullptr;

    void SetInitialCalibrationFiletypeFilter();

};
