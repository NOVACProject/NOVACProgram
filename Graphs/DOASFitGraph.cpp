#include "StdAfx.h"
#include "DOASFitGraph.h"
#include "../resource.h"
#include "../Common/Common.h"
#include <SpectralEvaluation/Spectra/Spectrum.h>

using namespace Graph;

CDOASFitGraph::CDOASFitGraph()
    : m_nReferences(0), m_fitLow(0), m_fitHigh(0)
{
    m_residual.resize(MAX_SPECTRUM_LENGTH);
    m_specieName.resize(MAX_N_REFERENCES);

    m_fitResult.resize(MAX_N_REFERENCES);
    for (int ii = 0; ii < MAX_N_REFERENCES; ++ii)
    {
        m_fitResult[ii].resize(MAX_SPECTRUM_LENGTH);
    }
}

CDOASFitGraph::~CDOASFitGraph()
{
}

BEGIN_MESSAGE_MAP(CDOASFitGraph, CGraphCtrl)
    ON_WM_CONTEXTMENU()

    ON_COMMAND(ID__SAVEASASCII, OnSaveAsAscii)
END_MESSAGE_MAP()

void CDOASFitGraph::DrawFit(int referenceIndex)
{
    double minV = 1e16, maxV = -1e16;
    static double oldMinV = 1e16, oldMaxV = -1e16;

    if (referenceIndex < 0 || referenceIndex >= m_nReferences)
    {
        return;
    }

    const long fitWidth = m_fitHigh - m_fitLow - 1;

    // The scaled cross section + the residual
    std::vector<double> CS_And_Residual;
    CS_And_Residual.resize(m_fitHigh);
    std::vector<double> number;
    number.resize(m_fitHigh);

    for (int pixelIdx = m_fitLow; pixelIdx < m_fitHigh; ++pixelIdx)
    {
        CS_And_Residual[pixelIdx] = m_fitResult[referenceIndex][pixelIdx] + m_residual[pixelIdx];
        number[pixelIdx] = pixelIdx;
    }

    // find the minimum and maximum value
    minV = Min(CS_And_Residual.data() + m_fitLow + 3, fitWidth - 6);
    maxV = Max(CS_And_Residual.data() + m_fitLow + 3, fitWidth - 6);

    if ((maxV - minV) < 0.25 * (oldMaxV - oldMinV))
    {
        oldMaxV = maxV;
        oldMinV = minV;
    }
    else
    {
        if (minV < oldMinV)
            oldMinV = minV;
        else
            minV = oldMinV;

        if (maxV > oldMaxV)
            oldMaxV = maxV;
        else
            maxV = oldMaxV;
    }

    if (minV < maxV)
    {
        // set the range for the plot
        SetRange(m_fitLow, m_fitHigh, 0, minV, maxV, 0);

        // draw the residual + the fitted (scaled & shifted) reference
        SetYAxisNumberFormat(Graph::FORMAT_GENERAL);
        SetPlotColor(RGB(255, 0, 0));
        XYPlot(number.data() + m_fitLow, CS_And_Residual.data() + m_fitLow, fitWidth, PLOT_FIXED_AXIS | PLOT_CONNECTED);

        // draw the fitted (scaled & shifted) reference
        SetPlotColor(RGB(0, 0, 255));
        XYPlot(number.data() + m_fitLow, m_fitResult[referenceIndex].data() + m_fitLow, fitWidth, PLOT_FIXED_AXIS | PLOT_CONNECTED);
    }
}

void CDOASFitGraph::OnContextMenu(CWnd* pWnd, CPoint point) {
    CMenu menu;
    VERIFY(menu.LoadMenu(IDR_MENU_SHOWFIT_CONTEXT));
    CMenu* pPopup = menu.GetSubMenu(0);
    ASSERT(pPopup != NULL);

    if (m_nReferences > 0 && m_fitHigh > 0 && m_fitLow < m_fitHigh) {
        pPopup->EnableMenuItem(ID__SAVEASBITMAP, MF_DISABLED | MF_GRAYED);

        pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
    }

}

void CDOASFitGraph::OnSaveAsAscii() {
    CString fileName;
    TCHAR filter[512];
    int n = _stprintf(filter, "Text File\0");
    n += _stprintf(filter + n + 1, "*.txt\0");
    filter[n + 2] = 0;
    Common common;

    // Make sure that there's something to save first...
    if (this->m_fitHigh == 0 || this->m_fitLow == 0 || this->m_nReferences == 0)
        return;

    if (common.BrowseForFile_SaveAs(filter, fileName)) {
        if (-1 == fileName.Find(".")) {
            fileName.AppendFormat(".txt");
        }

        // Save the data
        WriteAsciiFile(fileName);
    }
}

void CDOASFitGraph::WriteAsciiFile(const CString& fileName) {
    int i, j;

    FILE* f = fopen(fileName, "w");
    if (NULL != f) {
        fprintf(f, "Pixel\tResidual\t");
        for (j = 0; j < m_nReferences; ++j) {
            fprintf(f, "OD(%s)\t", m_specieName[j].c_str());
        }
        fprintf(f, "\n");

        for (i = m_fitLow; i < m_fitHigh; ++i) {
            fprintf(f, "%d\t%.4G\t", i, m_residual[i]);

            for (j = 0; j < m_nReferences; ++j) {
                fprintf(f, "%.4G\t", m_fitResult[j][i]);
            }
            fprintf(f, "\n");
        }


        fclose(f);
    }
}

