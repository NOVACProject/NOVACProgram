#include "StdAfx.h"
#include "CRatioCalculationDlg.h"
#include "../resource.h"
#include "CRatioScanFilesDialog.h"
#include "CRatioSetupDialog.h"
#include "CRatioEvaluationDialog.h"
#include "../Common/Common.h"

#include <SpectralEvaluation/DialogControllers/RatioCalculationController.h>

// CRatioCalculationDlg dialog

CRatioCalculationDlg::CRatioCalculationDlg()
    : CPropertySheet()
{
    m_controller = new RatioCalculationController(m_log);
    m_controller->LoadSetup(SetupFilePath());

    m_selectScanFilesPage = new CRatioScanFilesDialog(m_controller);
    m_selectScanFilesPage->Construct(IDD_RATIO_SCANFILES_DIALOG);

    m_setupEvaluationPage = new CRatioSetupDialog(m_controller);
    m_setupEvaluationPage->Construct(IDD_RATIO_WINDOW_SETUP_DIALOG);

    m_runEvaluationPage = new CRatioEvaluationDialog(m_controller);
    m_runEvaluationPage->Construct(IDD_RATIO_EVALUATE_DIALOG);

    AddPage(m_selectScanFilesPage);
    AddPage(m_setupEvaluationPage);
    AddPage(m_runEvaluationPage);
}

CRatioCalculationDlg::~CRatioCalculationDlg()
{
    delete m_selectScanFilesPage;
    delete m_setupEvaluationPage;
    delete m_runEvaluationPage;

    delete m_controller;
    m_controller = nullptr;
}

void CRatioCalculationDlg::DoDataExchange(CDataExchange* pDX)
{
    CPropertySheet::DoDataExchange(pDX);
}

BOOL CRatioCalculationDlg::OnInitDialog()
{
    BOOL bResult = CPropertySheet::OnInitDialog();

    // ------------ Get the buttons ---------------
    CWnd* pApply = this->GetDlgItem(ID_APPLY_NOW);
    CWnd* pCancel = this->GetDlgItem(IDCANCEL);
    CWnd* pOk = this->GetDlgItem(IDOK);

    // Remove each of the buttons
    if (pApply) {
        pApply->DestroyWindow();
    }
    if (pCancel) {
        pCancel->DestroyWindow();
    }
    if (pOk) {
        pOk->DestroyWindow();
    }

    return bResult;
}

BEGIN_MESSAGE_MAP(CRatioCalculationDlg, CPropertySheet)
END_MESSAGE_MAP()

// Persisting the setup to file
std::string CRatioCalculationDlg::SetupFilePath()
{
    Common common;
    common.GetExePath();
    CString path;
    path.Format("%sRatioCalculationDlg.config", common.m_exePath);
    return std::string(path);
}

// CRatioCalculationDlg message handlers
