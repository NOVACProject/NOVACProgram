#include "StdAfx.h"
#include "DOASFitGraph.h"
#include "../resource.h"

using namespace Graph;

CDOASFitGraph::CDOASFitGraph(void)
{
	m_nReferences = 0;
	m_fitLow = 0;
	m_fitHigh = 0;
}

CDOASFitGraph::~CDOASFitGraph(void)
{
}

BEGIN_MESSAGE_MAP(CDOASFitGraph, CGraphCtrl)
	ON_WM_CONTEXTMENU()
	
	ON_COMMAND(ID__SAVEASASCII, OnSaveAsAscii)
END_MESSAGE_MAP()

/** Draws the spectrum fit */
void CDOASFitGraph::DrawFit(int refIndex){
	double minV = 1e16, maxV = -1e16;
	static double oldMinV = 1e16, oldMaxV = -1e16;

	if(refIndex < 0 || refIndex >= m_nReferences)
		return;

	long fitWidth = m_fitHigh- m_fitLow- 1;

	// The scaled cross section + the residual
	double *CS_And_Residual = new double[m_fitHigh];
	double *number = new double[m_fitHigh];
	for(int i = m_fitLow; i < m_fitHigh; ++i){
		CS_And_Residual[i] = m_fitResult[refIndex][i] + m_residual[i];
		number[i] = i;
	}

	// find the minimum and maximum value
	minV = Min(CS_And_Residual + m_fitLow + 3, fitWidth - 6);
	maxV = Max(CS_And_Residual + m_fitLow + 3, fitWidth - 6);

	if((maxV - minV) < 0.25 * (oldMaxV - oldMinV)){
		oldMaxV = maxV;
		oldMinV = minV;
	}else{
		if(minV < oldMinV)
			oldMinV = minV;
		else
			minV = oldMinV;

		if(maxV > oldMaxV)
			oldMaxV = maxV;
		else
			maxV = oldMaxV;
	}

	if(minV < maxV){
		// set the range for the plot
		this->SetRange(m_fitLow, m_fitHigh, 0, minV, maxV, 0);

		// draw the residual + the fitted (scaled & shifted) reference
		this->SetPlotColor(RGB(255,0,0));
		this->XYPlot(number + m_fitLow, CS_And_Residual + m_fitLow, fitWidth, PLOT_FIXED_AXIS | PLOT_CONNECTED);

		// draw the fitted (scaled & shifted) reference
		this->SetPlotColor(RGB(0,0,255));
		this->XYPlot(number + m_fitLow, m_fitResult[refIndex] + m_fitLow, fitWidth, PLOT_FIXED_AXIS | PLOT_CONNECTED);
	}

	delete[] CS_And_Residual;
	delete[] number;
	
}

void CDOASFitGraph::OnContextMenu(CWnd* pWnd, CPoint point){
	CMenu menu;
	VERIFY(menu.LoadMenu(IDR_MENU_SHOWFIT_CONTEXT));
	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT(pPopup != NULL);

	if(m_nReferences > 0 && m_fitHigh > 0 && m_fitLow < m_fitHigh){
		pPopup->EnableMenuItem(ID__SAVEASBITMAP, MF_DISABLED | MF_GRAYED);

		pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
	}

}

void CDOASFitGraph::OnSaveAsAscii(){
	CString fileName;
	TCHAR filter[512];
	int n = _stprintf(filter, "Text File\0");
	n += _stprintf(filter + n + 1, "*.txt\0");
	filter[n + 2] = 0;
	Common common;

	// Make sure that there's something to save first...
	if(this->m_fitHigh == 0 || this->m_fitLow == 0 || this->m_nReferences == 0)
		return;

	if(common.BrowseForFile_SaveAs(filter, fileName)){
		if(-1 == fileName.Find(".")){
			fileName.AppendFormat(".txt");
		}

		// Save the data
		WriteAsciiFile(fileName);
	}
}

void CDOASFitGraph::WriteAsciiFile(const CString &fileName){
	int i, j;

	FILE *f = fopen(fileName, "w");
	if(NULL != f){
		fprintf(f, "Pixel\tResidual\t");
		for(j = 0; j < m_nReferences; ++j){
			fprintf(f, "OD(%s)\t", (LPCSTR)m_specieName[j]);
		}
		fprintf(f, "\n");
	
		for(i = m_fitLow; i < m_fitHigh; ++i){
			fprintf(f, "%d\t%.4G\t", i, m_residual[i]);
			
			for(j = 0; j < m_nReferences; ++j){
				fprintf(f, "%.4G\t", m_fitResult[j][i]);
			}
			fprintf(f, "\n");
		}
		
	
		fclose(f);
	}
}

