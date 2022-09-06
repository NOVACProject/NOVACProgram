#pragma once
#include "afxdlgs.h"

#include <string>

class RatioCalculationController;

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

    // The controller which performs the ratio calculations.
    // Notice that this is the owning class for the pointer and will delete it when done. 
    // The same pointer is referenced by each page in the dialog but this is the sole owner.
    RatioCalculationController* m_controller;

    // Persisting the setup between sessions
    std::string SetupFilePath();

};
