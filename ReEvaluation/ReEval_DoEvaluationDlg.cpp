#include "stdafx.h"
#include "../NovacMasterProgram.h"
#include "ReEval_DoEvaluationDlg.h"
#include "../Evaluation/EvaluationResultView.h"

using namespace ReEvaluation;
using namespace Evaluation;
using namespace novac;


UINT DoEvaluation(LPVOID pParam) {
    CReEvaluator *m_reeval = (CReEvaluator*)pParam;

    m_reeval->fRun = true;
    m_reeval->DoEvaluation();
    return 0;
}


// CReEval_DoEvaluationDlg dialog

IMPLEMENT_DYNAMIC(CReEval_DoEvaluationDlg, CPropertyPage)
CReEval_DoEvaluationDlg::CReEval_DoEvaluationDlg()
    : CPropertyPage(CReEval_DoEvaluationDlg::IDD)
    , m_showFit(0)
{
    m_reeval = NULL;
    m_curSpecie = 0;

    pReEvalThread = NULL;

    m_result = nullptr;
}

CReEval_DoEvaluationDlg::~CReEval_DoEvaluationDlg()
{
    m_reeval = NULL;
    pReEvalThread = NULL;

    m_result = nullptr;
}

void CReEval_DoEvaluationDlg::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    DDX_Radio(pDX, IDC_RADIO_SHOWFIT, m_showFit);
    DDX_Control(pDX, IDC_SPECIE_LIST, m_specieList);
    DDX_Control(pDX, IDC_TOTALFIT_FRAME, m_frameTotalfit);
    DDX_Control(pDX, IDC_REFFIT_FRAME, m_frameRefFit);
    DDX_Control(pDX, IDC_REEVAL_DOEVAL, m_btnDoEval);
    DDX_Control(pDX, IDC_REEVAL_CANCEL, m_btnCancel);
    DDX_Control(pDX, IDC_PROGRESSBAR, m_progressBar);
    DDX_Control(pDX, IDC_PROGRESSBAR2, m_progressBar2);
    DDX_Check(pDX, IDC_REEVAL_CHECK_PAUSE, m_reeval->m_pause);
}


BEGIN_MESSAGE_MAP(CReEval_DoEvaluationDlg, CPropertyPage)
    // Changed the specie to look  at
    ON_LBN_SELCHANGE(IDC_SPECIE_LIST, OnChangeSpecie)

    // Changed the selection of what to show in the 'totalGraph'
    ON_BN_CLICKED(IDC_RADIO_SHOWFIT, RedrawTotalFitGraph)
    ON_BN_CLICKED(IDC_RADIO_SHOW_RESIDUAL, RedrawTotalFitGraph)

    // The user starts or cancels the evaluation
    ON_BN_CLICKED(IDC_REEVAL_DOEVAL, OnDoEvaluation)
    ON_BN_CLICKED(IDC_REEVAL_CANCEL, OnCancelEvaluation)

    // To pause or not to pause between each spectrum-evaluation
    ON_BN_CLICKED(IDC_REEVAL_CHECK_PAUSE, OnBnClickedReevalCheckPause)

    ON_MESSAGE(WM_PROGRESS, OnProgress)
    ON_MESSAGE(WM_PROGRESS2, OnProgress2)
    ON_MESSAGE(WM_DONE, OnDone)
    ON_MESSAGE(WM_STATUSMSG, OnStatusUpdate)
    ON_MESSAGE(WM_EVAL_SUCCESS, OnEvaluatedSpectrum)
    ON_MESSAGE(WM_GOTO_SLEEP, OnEvaluationSleep)
END_MESSAGE_MAP()


// CReEval_DoEvaluationDlg message handlers

/** Initializes the graphs */
void  CReEval_DoEvaluationDlg::InitializeGraphs() {
    CRect rect;
    Common common;
    int margin = -4;

    // The total fit graph
    m_frameTotalfit.GetWindowRect(&rect);
    rect.bottom -= rect.top + margin;
    rect.right -= rect.left + margin;
    rect.top = margin;
    rect.left = margin;
    m_GraphTotal.Create(WS_VISIBLE | WS_CHILD, rect, &m_frameTotalfit);
    m_GraphTotal.SetRange(0, MAX_SPECTRUM_LENGTH, 0, 0.0, 4095.0, 1);
    m_GraphTotal.SetYUnits(common.GetString(AXIS_INTENSITY));
    m_GraphTotal.SetXUnits(common.GetString(AXIS_CHANNEL));
    m_GraphTotal.SetBackgroundColor(RGB(0, 0, 0));
    m_GraphTotal.SetGridColor(RGB(255, 255, 255));//(192, 192, 255)) ;
    m_GraphTotal.SetPlotColor(RGB(255, 0, 0));

    // The reference fit graph
    m_frameRefFit.GetWindowRect(&rect);
    rect.bottom -= rect.top + margin;
    rect.right -= rect.left + margin;
    rect.top = margin;
    rect.left = margin;
    m_GraphRef.Create(WS_VISIBLE | WS_CHILD, rect, &m_frameRefFit);
    m_GraphRef.SetRange(0, MAX_SPECTRUM_LENGTH, 0, 0.0, 4095.0, 2);
    m_GraphRef.SetYUnits(common.GetString(AXIS_INTENSITY));
    m_GraphRef.SetXUnits(common.GetString(AXIS_CHANNEL));
    m_GraphRef.SetBackgroundColor(RGB(0, 0, 0));
    m_GraphRef.SetGridColor(RGB(255, 255, 255));//(192, 192, 255)) ;
    m_GraphRef.SetPlotColor(RGB(255, 0, 0));
}

void  CReEval_DoEvaluationDlg::PopulateRefList()
{
    m_specieList.ResetContent();

    CEvaluationResult lastResult;

    if (m_result != nullptr &&
        m_result->GetEvaluatedNum() > 0 &&
        m_result->GetResult(m_result->GetEvaluatedNum() - 1, lastResult))
    {
        for (int ii = 0; ii < (int)lastResult.m_referenceResult.size(); ++ii)
        {
            CString name(lastResult.m_referenceResult[ii].m_specieName.c_str());
            m_specieList.AddString(name);
        }
    }
    else
    {
        const CFitWindow &window = m_reeval->m_window[m_reeval->m_curWindow];

        for (int ii = 0; ii < window.nRef; ++ii)
        {
            CString name(window.ref[ii].m_specieName.c_str());
            m_specieList.AddString(name);
        }
    }

    // set the selection
    m_specieList.SetCurSel(m_curSpecie);
}

BOOL CReEval_DoEvaluationDlg::OnInitDialog()
{
    CPropertyPage::OnInitDialog();

    // Initialize the graphs
    InitializeGraphs();

    // Initialize the list
    PopulateRefList();

    // initialize the controls
    m_btnCancel.EnableWindow(FALSE);
    m_btnDoEval.EnableWindow(TRUE);
    m_progressBar.SetRange(0, 1000);
    m_progressBar.SetPos(0);
    m_progressBar2.SetRange(0, 1000);
    m_progressBar2.SetPos(0);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CReEval_DoEvaluationDlg::OnSetActive()
{
    // fill in the items in the list
    PopulateRefList();

    return CPropertyPage::OnSetActive();
}

void CReEval_DoEvaluationDlg::OnChangeSpecie()
{
    m_curSpecie = (long)m_specieList.GetCurSel();

    // Redraw the reference-screen
    DrawReference();
}

void CReEval_DoEvaluationDlg::OnDoEvaluation()
{
    // save the data in the dialog
    UpdateData(TRUE);

    if (m_reeval->fRun && m_reeval->m_sleeping) {
        pReEvalThread->ResumeThread();
    }
    else {
        m_reeval->pView = this;

        // start the reevaluation thread
        pReEvalThread = AfxBeginThread(DoEvaluation, (LPVOID)(m_reeval), THREAD_PRIORITY_BELOW_NORMAL, 0, 0, NULL);
    }

    // update the window
    SetDlgItemText(IDC_REEVAL_STATUSBAR, "Evaluating");
    m_btnCancel.EnableWindow(TRUE);
    m_btnDoEval.EnableWindow(FALSE);
}

void CReEval_DoEvaluationDlg::OnCancelEvaluation()
{

    // If the 'pause' - button is pressed and the thread is waiting for the 
    //	user to press 'next'...
    if (m_reeval->fRun && m_reeval->m_sleeping) {
        pReEvalThread->ResumeThread();
    }


    // Quit the re-evaluation thread
    DWORD dwExitCode;
    HANDLE hThread = this->pReEvalThread->m_hThread;
    if (hThread != NULL && GetExitCodeThread(hThread, &dwExitCode) && dwExitCode == STILL_ACTIVE) {
        AfxGetApp()->BeginWaitCursor();
        this->m_reeval->Stop();
        this->m_reeval->m_pause = FALSE;

        WaitForSingleObject(hThread, INFINITE);
        AfxGetApp()->EndWaitCursor();
    }

    // update the window
    m_progressBar.SetRange(0, 1000);
    m_progressBar.SetPos(0);
    m_progressBar2.SetRange(0, 1000);
    m_progressBar2.SetPos(0);

    SetDlgItemText(IDC_REEVAL_STATUSBAR, "Evaluation cancelled");
    m_btnCancel.EnableWindow(FALSE);
    m_btnDoEval.EnableWindow(TRUE);
    m_btnDoEval.SetWindowText("&Do Evaluation");
}

LRESULT CReEval_DoEvaluationDlg::OnEvaluatedSpectrum(WPARAM wp, LPARAM lp) {
    // if the reevaluator stopped, don't do anything
    if (!m_reeval->fRun) {
        CEvaluationResultView* resultview = (CEvaluationResultView *)wp;
        CScanResult * result = (CScanResult *)lp;

        // Clean up the pointers which we were given
        m_result.reset();
        delete result;
        delete resultview;

        return 0;
    }

    int lastWindowUsed = 0; // which window in the re-evaluator was used last time we received an evaluated spectrum?

    // Capture the spectrum (remember to delete this later)
    CEvaluationResultView* resultview = (CEvaluationResultView *)wp;

    m_result.reset((CScanResult *)lp);

    // a handle to the fit window
    CFitWindow &window = m_reeval->m_window[m_reeval->m_curWindow];
    int fitLow = window.fitLow - resultview->measuredSpectrum.m_info.m_startChannel;
    int fitHigh = window.fitHigh - resultview->measuredSpectrum.m_info.m_startChannel;

    // If the fit-window has changed, then change the list of references
    if (m_reeval->m_curWindow != lastWindowUsed) {
        PopulateRefList();
    }
    lastWindowUsed = m_reeval->m_curWindow;

    // 1. Draw the resulting fit for one of the references
    {
        // copy the fitted cross-sections to a local variable
        for (int k = 0; k < m_result->GetSpecieNum(0); ++k)
        {
            for (int i = fitLow; i < fitHigh; ++i)
            {
                m_GraphRef.m_fitResult[k][i] = resultview->scaledReference[k].m_data[i];    // fit result is the scaled cross section of the chosen specie
            }
            m_GraphRef.m_specieName[k] = CString(m_result->GetSpecieName(0, k).c_str());
            m_GraphRef.m_nReferences = m_result->GetSpecieNum(0);
        }

        // also copy the residual
        for (int i = fitLow; i < fitHigh; ++i)
        {
            m_GraphRef.m_residual[i] = resultview->residual.m_data[i];
        }

        DrawReference();
    }

    // 2. Draws the whole fit
    if (m_showFit == 0) {
        // copy the spectrum to the local variable
        for (int i = 0; i < window.specLength; ++i) {
            spectrum[i] = resultview->measuredSpectrum.m_data[i];
        }
        DrawFit();
    }
    else {
        // copy the residual to the local variable
        for (int i = fitLow; i < fitHigh; ++i) {
            residual[i] = resultview->residual.m_data[i];
        }

        DrawResidual();
    }

    // Clean up the pointer which we were given
    delete resultview;

    return 0;
}

void CReEval_DoEvaluationDlg::RedrawTotalFitGraph() {

    if (m_showFit == 0)
        DrawFit();
    else
        DrawResidual();

}

void CReEval_DoEvaluationDlg::DrawReference()
{
    // a handle to the fit window
    CFitWindow &window = m_reeval->m_window[m_reeval->m_curWindow];

    // The reference that we shall draw
    int refIndex = m_specieList.GetCurSel();
    if (refIndex < 0)
    {
        m_specieList.SetCurSel(0);
        refIndex = 0;
    }

    // the width of the fit region (fitHigh - fitLow)
    m_GraphRef.m_fitLow = window.fitLow - window.startChannel;
    m_GraphRef.m_fitHigh = window.fitHigh - window.startChannel;

    // Draw the fit
    m_GraphRef.DrawFit(refIndex);

    // update the labels
    if (m_result != nullptr)
    {
        CString columnStr, shiftStr, squeezeStr, deltaStr, chi2Str;

        int specIndex = m_result->GetEvaluatedNum() - 1;
        double column = m_result->GetColumn(specIndex, refIndex);
        double columnError = m_result->GetColumnError(specIndex, refIndex);
        double shift = m_result->GetShift(specIndex, refIndex);
        double shiftError = m_result->GetShiftError(specIndex, refIndex);
        double squeeze = m_result->GetSqueeze(specIndex, refIndex);
        double squeezeError = m_result->GetSqueezeError(specIndex, refIndex);

        columnStr.Format("Column %.2G ± %.2G", column, columnError);
        shiftStr.Format("Shift: %.2lf ± %.2lf", shift, shiftError);
        squeezeStr.Format("Squeeze: %.2lf ± %.2lf", squeeze, squeezeError);
        deltaStr.Format("Delta: %.2e", m_result->GetDelta(specIndex));
        chi2Str.Format("Chi²: %.2e", m_result->GetChiSquare(specIndex));

        SetDlgItemText(IDC_REEVAL_LBL_COLUMN, columnStr);
        SetDlgItemText(IDC_REEVAL_LBL_SHIFT, shiftStr);
        SetDlgItemText(IDC_REEVAL_LBL_SQUEEZE, squeezeStr);
        SetDlgItemText(IDC_REEVAL_LBL_DELTA, deltaStr);
        SetDlgItemText(IDC_REEVAL_LBL_CHISQUARE, chi2Str);
    }
}

void CReEval_DoEvaluationDlg::DrawFit() {
    double minV = 1e16, maxV = -1e16;
    static double oldMinV = 1e16, oldMaxV = -1e16;

    // a handle to the fit window
    CFitWindow &window = m_reeval->m_window[m_reeval->m_curWindow];

    // the width of the fit region (fitHigh - fitLow)
    int fitLow = window.fitLow;
    int fitHigh = window.fitHigh;
    double fitWidth = fitHigh - fitLow - 1;

    /* show the fit */

    // find the minimum and maximum value
    for (int i = fitLow + 3; i < fitHigh - 3; ++i) {
        minV = std::min(minV, spectrum[i]);
        maxV = std::max(maxV, spectrum[i]);
    }
    if (maxV <= minV)
        maxV = minV + 1;

    if ((maxV - minV) < 0.25 * (oldMaxV - oldMinV)) {
        oldMaxV = maxV;
        oldMinV = minV;
    }
    else {
        if (minV < oldMinV)
            oldMinV = minV;
        else
            minV = oldMinV;

        if (maxV > oldMaxV)
            oldMaxV = maxV;
        else
            maxV = oldMaxV;
    }

    // set the range for the plot
    m_GraphTotal.SetRange(fitLow, fitHigh, 0, minV, maxV, 0);

    // draw the spectrum
    m_GraphTotal.SetPlotColor(RGB(255, 0, 0));
    m_GraphTotal.DrawPoint(spectrum, (int)fitWidth, fitLow);

    return;
}

void CReEval_DoEvaluationDlg::DrawResidual() {
    double minV = 1e16, maxV = -1e16;
    static double oldMinV = 1e16, oldMaxV = -1e16;

    /* show the residual */

    CFitWindow &window = m_reeval->m_window[m_reeval->m_curWindow];

    // the width of the fit region (fitHigh - fitLow)
    int fitLow = window.fitLow;
    int fitHigh = window.fitHigh;
    double fitWidth = fitHigh - fitLow - 1;


    // find the minimum and maximum value
    for (int i = fitLow + 3; i < fitHigh - 3; ++i) {
        minV = std::min(minV, residual[i]);
        maxV = std::max(maxV, residual[i]);
    }
    if ((maxV - minV) < 0.25 * (oldMaxV - oldMinV)) {
        oldMaxV = maxV;
        oldMinV = minV;
    }
    else {
        if (minV < oldMinV)
            oldMinV = minV;
        else
            minV = oldMinV;

        if (maxV > oldMaxV)
            oldMaxV = maxV;
        else
            maxV = oldMaxV;
    }

    // set the range for the plot
    m_GraphTotal.SetRange(fitLow, fitHigh, 0, minV, maxV, 1);

    // draw the spectrum
    m_GraphTotal.SetPlotColor(RGB(255, 0, 0));
    m_GraphTotal.DrawPoint(residual, (int)fitWidth, fitLow);
}

LRESULT CReEval_DoEvaluationDlg::OnStatusUpdate(WPARAM wp, LPARAM lp) {

    return 0;
}

LRESULT CReEval_DoEvaluationDlg::OnDone(WPARAM wp, LPARAM lp) {
    // update the window
    m_progressBar.SetRange(0, 1000);
    m_progressBar.SetPos(0);
    m_progressBar2.SetRange(0, 1000);
    m_progressBar2.SetPos(0);

    SetDlgItemText(IDC_REEVAL_STATUSBAR, "Evaluation Done");
    m_btnCancel.EnableWindow(FALSE);
    m_btnDoEval.EnableWindow(TRUE);
    m_btnDoEval.SetWindowText("&Do Evaluation");

    return 0;
}

LRESULT CReEval_DoEvaluationDlg::OnProgress(WPARAM wp, LPARAM lp) {
    if (m_reeval->fRun) // Check if the evaluation is still running
    {
        double progress = (double)wp;
        m_progressBar.SetPos((int)(progress));
    }

    return 0;
}

LRESULT CReEval_DoEvaluationDlg::OnProgress2(WPARAM wp, LPARAM lp) {
    if (m_reeval->fRun) // Check if the evaluation is still running
    {
        long curFileIndex = (long)wp;
        long fileNum = (long)lp;

        float progress = (curFileIndex + 1) / (float)fileNum;

        m_progressBar2.SetPos((int)(progress * 1000.0f));

        CString msg;
        msg.Format("spec %ld out of %ld", curFileIndex + 1, fileNum);
        SetDlgItemText(IDC_REEVAL_STATUSBAR2, msg);
    }

    return 0;
}

LRESULT CReEval_DoEvaluationDlg::OnEvaluationSleep(WPARAM wp, LPARAM lp) {
    SetDlgItemText(IDC_REEVAL_STATUSBAR, "Waiting...");
    m_btnCancel.EnableWindow(TRUE);
    m_btnDoEval.EnableWindow(TRUE);
    m_btnDoEval.SetWindowText("&Next...");

    return 0;
}
void CReEval_DoEvaluationDlg::OnBnClickedReevalCheckPause()
{
    UpdateData(TRUE);
}
