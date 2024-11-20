#pragma once

#include <SpectralEvaluation/Evaluation/FitWindow.h>
#include "../Graphs/GraphCtrl.h"
#include "../Resource.h"

// CReferencePlotDlg dialog

namespace Dialogs
{

/** The class <b>CReferencePlotDlg</b> opens a simple dialog
and plots the reference-files in the supplied fit-window
in the given fit-region.
*/
class CReferencePlotDlg : public CDialog
{
    DECLARE_DYNAMIC(CReferencePlotDlg)

public:
    CReferencePlotDlg(novac::CFitWindow& window, CWnd* pParent = NULL);
    virtual ~CReferencePlotDlg();

    // Dialog Data
    enum { IDD = IDD_REFERENCE_PLOT_DLG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()

protected:
    // ----------------- DIALOG COMPONENTS -------------------

    /** The graphs, one for each reference file */
    Graph::CGraphCtrl m_graphs[MAX_N_REFERENCES];

    /** The labels for the references */
    CStatic m_label[MAX_N_REFERENCES];

    /** Called when we're opening the window */
    virtual BOOL OnInitDialog();

    /** Called when the window is re-sized */
    afx_msg void OnSize(UINT nType, int cx, int cy);

    /** Called to read in the references */
    void ReadReferences();

    /** Called to draw the graphs */
    void DrawGraph();

    // ----------------- PRIVATE DATA -------------------

    /** The data in the reference files */
    std::vector<std::vector<double>> m_data;
    double m_number[4096];

    /** The window for which we want to see the references */
    novac::CFitWindow& m_window;

};
}