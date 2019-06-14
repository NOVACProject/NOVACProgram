#pragma once
#include "afxwin.h"


// CCreateReferencesDlg dialog

class CCreateReferencesDlg : public CDialog
{
	DECLARE_DYNAMIC(CCreateReferencesDlg)

public:
	CCreateReferencesDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCreateReferencesDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CREATE_REFERENCES_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnButtonBrowseSlf();
    afx_msg void OnButtonBrowseCalibration();
    afx_msg void OnButtonBrowseHighResCrossSection();
    afx_msg void OnButtonBrowseOutput();
    afx_msg void OnButtonRunConvolution();
    CButton m_runConvolutionBtn;

private:
    CString m_slfFile;
    CString m_calibrationFile;
    CString m_crossSectionFile;
    CString m_outputFile;
    CComboBox m_wavelengthConversionCombo;

    void UpdateRunConvolutionButton();
};
