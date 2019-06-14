// ReferencePlotDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ReferencePlotDlg.h"
#include <SpectralEvaluation/Evaluation/CrossSectionData.h>

using namespace Dialogs;

IMPLEMENT_DYNAMIC(CReferencePlotDlg, CDialog)
CReferencePlotDlg::CReferencePlotDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CReferencePlotDlg::IDD, pParent)
{
}

CReferencePlotDlg::~CReferencePlotDlg()
{
}

void CReferencePlotDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CReferencePlotDlg, CDialog)
    ON_WM_SIZE()
END_MESSAGE_MAP()


// CReferencePlotDlg message handlers

BOOL CReferencePlotDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    if (m_window == NULL || m_window->nRef == 0)
        return TRUE; // if there's no window then don't do anything...

    unsigned int nReferences = m_window->nRef;

    CRect thisWindowRect, rect;
    int margin = 5; // the space between each plot
    int leftMargin = 25; // the space to the left, for specie names
    int plotHeight = 100; // the height of each plot
    int labelWidth = 10; // the width of the labels
    int titleBarHeight = 30; // the height of the title bar...
    CString specieName; // the name of the species

                        // Get the height of this window
    GetWindowRect(thisWindowRect);

    // Make sure that all references fit into the window
    thisWindowRect.bottom = thisWindowRect.top + nReferences * plotHeight + (nReferences + 1)* margin + titleBarHeight;
    MoveWindow(thisWindowRect);

    // rect is the size and location of each graph
    rect.left = leftMargin;
    rect.right = thisWindowRect.Width() - labelWidth - margin;

    // Create each of the graphs
    for (unsigned int k = 0; k < nReferences; ++k) {
        rect.top = k * (plotHeight + margin) + margin;
        rect.bottom = rect.top + plotHeight;

        if (k == nReferences - 1) {
            m_graphs[k].SetXUnits("Pixel");
        }
        else {
            m_graphs[k].HideXScale();
        }

        m_graphs[k].Create(WS_VISIBLE | WS_CHILD, rect, this);
        m_graphs[k].SetYUnits("");
        m_graphs[k].SetLineWidth(2);               // Increases the line width to 2 pixels
        m_graphs[k].SetMinimumRangeY(1e-45);
        m_graphs[k].SetPlotColor(RGB(255, 0, 0));
        m_graphs[k].SetGridColor(RGB(255, 255, 255));
        m_graphs[k].SetBackgroundColor(RGB(0, 0, 0));
        m_graphs[k].SetRange(m_window->fitLow, m_window->fitHigh, 0, 0, 100, 0);
    }

    // Create the labels also...
    CFont *font = new CFont();
    font->CreateFont((int)(14 - 0.2*nReferences), 0, 0, 0, FW_BOLD,
        FALSE, FALSE, 0, ANSI_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_SWISS, "Arial");
    rect.left = 0;
    rect.right = leftMargin;
    for (unsigned int k = 0; k < nReferences; ++k) {
        rect.top = k * (plotHeight + margin) + margin + plotHeight / 2;
        rect.bottom = rect.top + plotHeight / 3;

        specieName.Format("%s", m_window->ref[k].m_specieName.c_str());
        m_label[k].Create(specieName, WS_VISIBLE | WS_CHILD, rect, this);
        m_label[k].SetFont(font);
    }

    // We need to read in the reference files...
    ReadReferences();

    // fill in the X-array
    for (int k = m_window->fitLow; k <= m_window->fitHigh; ++k)
    {
        this->m_number[k] = k;
    }

    // .. before we can draw them
    DrawGraph();

    return TRUE;
}

/** Called to read in the references */
void CReferencePlotDlg::ReadReferences()
{
    // test reading data from reference files
    for (int i = 0; i < m_window->nRef; ++i)
    {
        // Read the file
        if (0 != m_window->ref[i].ReadCrossSectionDataFromFile())
        {
            // ERROR... Tell the user ?
            std::vector<double> thisReference(0, 8192); // create a reference with all zeros.
            m_data.push_back(thisReference);
            continue;
        }
        else
        {
            std::vector<double> thisReference = m_window->ref[i].m_data->m_crossSection;
            m_data.push_back(thisReference);
        }
    }
}

void CReferencePlotDlg::DrawGraph()
{
    for (int k = 0; k < m_window->nRef; ++k)
    {
        m_graphs[k].XYPlot(m_number + m_window->fitLow, m_data[k].data() + m_window->fitLow, m_window->fitHigh - m_window->fitLow);
    }

}
void Dialogs::CReferencePlotDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialog::OnSize(nType, cx, cy);

    unsigned int k;
    CRect graphRect, specieRect;

    if (!IsWindow(m_graphs[0].m_hWnd))
        return;

    unsigned int nReferences = m_window->nRef;

    int margin = 5;				// the space between each plot
    int leftMargin = 25;		// the space to the left, for specie names
    int labelWidth = 10;		// the width of the labels
    int titleBarHeight = 30;	// the height of the title bar...
    int plotHeight = (cy - nReferences * margin) / nReferences; // the height of each graph

                                                                // The width of each graph
    graphRect.left = leftMargin;
    graphRect.right = cx - labelWidth - margin;

    // The width of each label
    specieRect.left = 0;
    specieRect.right = leftMargin;

    // Move the graphs to their right positions
    for (k = 0; k < nReferences; ++k) {
        // The location of each graph
        graphRect.top = k * (plotHeight + margin) + margin;
        graphRect.bottom = graphRect.top + plotHeight;

        // move the graph in place
        m_graphs[k].MoveWindow(graphRect);

        // The location of each label
        specieRect.top = k * (plotHeight + margin) + margin + plotHeight / 2;
        specieRect.bottom = specieRect.top + plotHeight / 3;

        // move the specie in place
        m_label[k].MoveWindow(specieRect);
    }

    // also draw the graphs
    DrawGraph();
}
