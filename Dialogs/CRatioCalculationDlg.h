#pragma once

// This is the CRatioCalculationDlg dialog where the user can calculate BrO to SO2 ratios from measuerd data.
class CRatioCalculationDlg : public CPropertySheet
{
public:
    CRatioCalculationDlg();   // standard constructor
    virtual ~CRatioCalculationDlg();

    virtual BOOL OnInitDialog();

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()

private:
    CPropertyPage* m_selectScanFilesPage;
    CPropertyPage* m_setupEvaluationPage;
    CPropertyPage* m_runEvaluationPage;

};
