// OverView.cpp : implementation file
//

#include "stdafx.h"
#include "NovacMasterProgram.h"
#include "View_OverView.h"
#include "Configuration/Configuration.h"
#include "UserSettings.h"

extern CConfigurationSetting g_settings;
extern CUserSettings g_userSettings;       // <-- The users preferences
using namespace Graph;

// CView_OverView dialog

IMPLEMENT_DYNAMIC(CView_OverView, CPropertyPage)
CView_OverView::CView_OverView()
	: CPropertyPage(CView_OverView::IDD)
{
	m_evalDataStorage = NULL;
	m_commDataStorage = NULL;
}

CView_OverView::~CView_OverView()
{
	m_evalDataStorage = NULL;
	m_commDataStorage = NULL;
	for(int i = 0; i < m_graphs.GetCount(); ++i){
		CGraphCtrl *graph = m_graphs[i];
		delete graph;
	}
	m_graphs.RemoveAll();

	for(int i = 0; i < m_specLabel.GetCount(); ++i){
		CStatic *label = m_specLabel[i];
		delete label;
	}
	m_specLabel.RemoveAll();

	for(int i = 0; i < m_statusLabel.GetCount(); ++i){
		CStatic *label = m_statusLabel[i];
		delete label;
	}
	m_statusLabel.RemoveAll();

	for(int i = 0; i < m_fluxLabel.GetCount(); ++i){
		CStatic *label = m_fluxLabel[i];
		delete label;
	}
	m_fluxLabel.RemoveAll();
}

void CView_OverView::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CView_OverView, CPropertyPage)
	// Messages from other parts of the program
	ON_MESSAGE(WM_EVAL_SUCCESS, OnUpdateFluxes)
	ON_MESSAGE(WM_EVAL_FAILURE, OnUpdateFluxes)
END_MESSAGE_MAP()


// CView_OverView message handlers

BOOL CView_OverView::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	CString specString[50];
	CString statusString[50];
	CString fluxString[50];
	CRect thisRect, rect, rect2;
	int nSpectrometers = g_settings.scannerNum;
	int margin = 15;
	int labelWidth = 200;
	CString fluxAxisLabel;
	Common common;

	this->GetWindowRect(thisRect);

	if(nSpectrometers == 0)
		return TRUE;

	if(g_userSettings.m_fluxUnit == UNIT_TONDAY){
		fluxAxisLabel.Format("%s [ton/day]", common.GetString(AXIS_FLUX));
	}else{
		fluxAxisLabel.Format("%s [kg/s]", common.GetString(AXIS_FLUX));
	}
	
	// reduce the margin if there are many instruments, to make the page look nicer
	margin = margin / nSpectrometers;

	// The height of each graph
	int height = (thisRect.Height() - margin * (1 + nSpectrometers)) / nSpectrometers;

	rect.left = margin;
	rect.right = thisRect.Width() - labelWidth - margin;

	rect2.left  = rect.right + margin;
	rect2.right = thisRect.Width();

	// The font we want to use
	CFont *font = new CFont();
	font->CreateFont((int)(16 - 0.2*nSpectrometers), 0, 0, 0, FW_BOLD ,
	                     FALSE, FALSE, 0, ANSI_CHARSET,
	                     OUT_DEFAULT_PRECIS, 
	                     CLIP_DEFAULT_PRECIS,
	                     DEFAULT_QUALITY, 
	                     DEFAULT_PITCH|FF_SWISS, "Arial") ;

	// Create all the graphs
	for(int i = 0; i < nSpectrometers; ++i){
		rect.top    = i * (height + margin) + margin;
		rect.bottom = rect.top + height;

		// Create the graph
		CGraphCtrl *graph = new CGraphCtrl();

		if(i == nSpectrometers - 1){
			graph->SetXUnits(m_common.GetString(AXIS_LOCALTIME));
		}else{
			graph->HideXScale();
		}
		graph->Create(WS_VISIBLE | WS_CHILD, rect, this);
		graph->SetFontHeight((int)(14 - 0.3*nSpectrometers));
		graph->SetXAxisNumberFormat(FORMAT_TIME);
		graph->EnableGridLinesX(true);
		graph->SetYUnits(fluxAxisLabel);
		graph->SetLineWidth(2);               // Increases the line width to 2 pixels
		graph->SetMinimumRangeY(1.0);
		graph->SetPlotColor(RGB(255, 255, 255));
		graph->SetGridColor(RGB(255, 255, 255));
		graph->SetBackgroundColor(RGB(0, 0, 0));
		graph->SetRange(0, 24*3600-1, 0, 0, 100, 0);
		m_graphs.Add(graph);

		// Create the label with the name of the graph
		CStatic *label = new CStatic();

		rect2.top = rect.top;
		rect2.bottom = rect2.top + height;
		specString[i].Format("%s - %s", g_settings.scanner[i].spec[0].serialNumber, g_settings.scanner[i].site);
		label->Create(specString[i], WS_VISIBLE | WS_CHILD, rect2, this);
		label->SetFont(font);
		m_specLabel.Add(label);

		// Create the label with number of spectra today
		CStatic *statusLabel = new CStatic();

		rect2.top = rect.top + (rect.bottom - rect.top) * 1 / 5;
		rect2.bottom = rect2.top + 15;
		statusString[i].Format("Scans received today: 0");
		statusLabel->Create(statusString[i], WS_VISIBLE | WS_CHILD, rect2, this);
		statusLabel->SetFont(font);
		m_statusLabel.Add(statusLabel);

		// Create the label telling the average and std of the flux
		CStatic *fluxLabel = new CStatic();

		rect2.top    = rect.top + (rect.bottom - rect.top) * 2 / 5;
		rect2.bottom = rect2.top + 90;
		if(g_userSettings.m_fluxUnit == UNIT_TONDAY){
			fluxString[i].Format("%s\tavg: 0 [ton/day]\r\n\tstd: 0 [ton/day]", common.GetString(AXIS_FLUX));
		}else{
			fluxString[i].Format("%s\tavg: 0 [kg/s]\r\n\tstd: 0 [kg/s]", common.GetString(AXIS_FLUX));
		}
		fluxLabel->Create(fluxString[i], WS_VISIBLE | WS_CHILD, rect2, this);
		fluxLabel->SetFont(font);
		m_fluxLabel.Add(fluxLabel);
	}
	DrawFlux();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}



LRESULT CView_OverView::OnUpdateFluxes(WPARAM wParam, LPARAM lParam){
	// Re-draw the screen
	DrawFlux();

	return 0;
}

void CView_OverView::DrawFlux(){
	double maxFlux;
	const int BUFFER_SIZE = 200;
	double allFluxes[BUFFER_SIZE], tid[BUFFER_SIZE];
	int fluxOk[BUFFER_SIZE];
	double goodFluxes[BUFFER_SIZE], badFluxes[BUFFER_SIZE];
	double goodTime[BUFFER_SIZE], badTime[BUFFER_SIZE];
	int nGoodFluxes = 0, nBadFluxes = 0;
	Common common;

	CString serial, statusStr, tempStr, fluxAxisLabel;
	double avgFlux, stdFlux;

	int nSpectrometers = g_settings.scannerNum;

	for(int i = 0; i < nSpectrometers; ++i){
		// The serialnumber
		serial.Format(g_settings.scanner[i].spec[0].serialNumber);

		// Get the data
		int dataLength = m_evalDataStorage->GetFluxData(serial, tid, allFluxes, fluxOk, BUFFER_SIZE);

		// sort the data into good values and bad values
		nGoodFluxes = 0;
		nBadFluxes = 0;
		for(int k = 0; k < dataLength; ++k){
			if(fluxOk[k]){
				goodFluxes[nGoodFluxes] = allFluxes[k];
				goodTime[nGoodFluxes]   = tid[k];
				++nGoodFluxes;
			}else{
				badFluxes[nBadFluxes] = allFluxes[k];
				badTime[nBadFluxes]   = tid[k];
				++nBadFluxes;
			}
		}


		// The graph to draw in
		CGraphCtrl	*graph = m_graphs.GetAt(i);

		// Clear the plot
		graph->CleanPlot();

		// Set the unit of the plot
		if(g_userSettings.m_fluxUnit == UNIT_TONDAY){
			fluxAxisLabel.Format("%s [ton/day]", common.GetString(AXIS_FLUX));
		}else{
			fluxAxisLabel.Format("%s [kg/s]", common.GetString(AXIS_FLUX));
		}
		graph->SetYUnits(fluxAxisLabel);

		// If there's no data, don't draw anything
		if(dataLength == 0)
			continue;

		// Get the ranges for the flux data
		maxFlux = Max(allFluxes, dataLength);

		// set the range for the plot
		graph->SetRangeY(0, maxFlux, 1);

		// First draw the bad values, then the good ones
		graph->SetCircleColor(RGB(150, 150, 150));
		graph->XYPlot(badTime, badFluxes, nBadFluxes, CGraphCtrl::PLOT_FIXED_AXIS | CGraphCtrl::PLOT_CIRCLES);

		graph->SetCircleColor(RGB(255, 255, 255));
		graph->XYPlot(goodTime, goodFluxes, nGoodFluxes, CGraphCtrl::PLOT_FIXED_AXIS | CGraphCtrl::PLOT_CIRCLES);

		// Update the status-label...
		statusStr.Format("Scans received today: %02d", dataLength);
		m_statusLabel.GetAt(i)->SetWindowText(statusStr);

		// Update the flux-label
		// 1. Flux
		m_evalDataStorage->GetFluxStat(serial, avgFlux, stdFlux);
		if(UNIT_KGS == g_userSettings.m_fluxUnit){
			tempStr.Format("%s\tavg: %.0lf [kg/s]\r\n\tstd: %.0lf [kg/s]\r\n", common.GetString(AXIS_FLUX), avgFlux, stdFlux);
		}else{
			tempStr.Format("%s\tavg: %.0lf [ton/day]\r\n\tstd: %.0lf [ton/day]\r\n", common.GetString(AXIS_FLUX), avgFlux, stdFlux);
		}

		m_fluxLabel.GetAt(i)->SetWindowText(tempStr);
	}
}

BOOL CView_OverView::OnSetActive()
{
	// Redraw the screen
	DrawFlux();

	return CPropertyPage::OnSetActive();
}
