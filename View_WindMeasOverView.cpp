// View_WindMeasOverView.cpp : implementation file
//

#include "stdafx.h"
#include "NovacMasterProgram.h"
#include "View_WindMeasOverView.h"

#include "Configuration/Configuration.h"

extern CConfigurationSetting g_settings;

using namespace Graph;

// CView_WindMeasOverView dialog

IMPLEMENT_DYNAMIC(CView_WindMeasOverView, CPropertyPage)
CView_WindMeasOverView::CView_WindMeasOverView()
	: CPropertyPage(CView_WindMeasOverView::IDD)
{
	m_evalDataStorage = NULL;
}

CView_WindMeasOverView::~CView_WindMeasOverView()
{
	m_evalDataStorage = NULL;

	// Clear the labels
	POSITION pos = m_serialLegend.GetHeadPosition();
	while(pos != NULL){
		CLegend *leg = m_serialLegend.GetNext(pos);
		delete leg;
	}
}

void CView_WindMeasOverView::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CView_WindMeasOverView, CPropertyPage)
	ON_MESSAGE(WM_CORR_SUCCESS, OnUpdateGraphs)
END_MESSAGE_MAP()


// CView_WindMeasOverView message handlers
BOOL CView_WindMeasOverView::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	CString serialString[50];
	CRect thisRect, graphRect, labelRect, textRect;
	int margin			= 15; // the margin between each element
	int labelWidth	= 80; // the width of each label
	int	labelHeight	= 15;	// the height of each label
	COLORREF colors[] = {RGB(255, 0, 0), RGB(0, 255, 0), RGB(0, 0, 255), RGB(255, 255, 0), RGB(255, 0, 255), RGB(0, 255, 255)};
	int nColors = 6;

	// The number of double spectrometers
	int nSpectrometers = 0;
	for(unsigned int index = 0; index < g_settings.scannerNum; ++index){
		if(g_settings.scanner[index].spec[0].channelNum == 2 || g_settings.scanner[index].instrumentType == INSTR_HEIDELBERG){
			++nSpectrometers;
		}
	}
	// if there are no double-spectrometers then we don't have anything to do
	if(nSpectrometers == 0)
		return TRUE;

	// The size of the window.
	this->GetWindowRect(thisRect);

	// The legends are arranged in a matrix with up to four in each row
	//	and as many rows as are needed
	int nRows = 1 + (int)(nSpectrometers / 4);

	// Calculate the size of the graph
	graphRect.left	= margin;
	graphRect.right	= thisRect.Width() - 2 * margin;
	graphRect.top		= margin;
	graphRect.bottom= thisRect.Height() - 2*margin - nRows * labelHeight;

	// The font we want to use
	CFont *font = new CFont();
	font->CreateFont((int)(16 - 0.2*nSpectrometers), 0, 0, 0, FW_BOLD ,
                       FALSE, FALSE, 0, ANSI_CHARSET,
                       OUT_DEFAULT_PRECIS, 
                       CLIP_DEFAULT_PRECIS,
                       DEFAULT_QUALITY, 
                       DEFAULT_PITCH|FF_SWISS, "Arial") ;

	// Create the graph
	m_graph.SetXUnits(m_common.GetString(AXIS_LOCALTIME));
	m_graph.Create(WS_VISIBLE | WS_CHILD, graphRect, this);
	m_graph.SetFontHeight((int)(14 - 0.3*nSpectrometers));
	m_graph.SetXAxisNumberFormat(FORMAT_TIME);
	m_graph.SetYUnits("[m/s]");
	m_graph.SetLineWidth(2);               // Increases the line width to 2 pixels
	m_graph.SetMinimumRangeY(1.0);
	m_graph.SetGridColor(RGB(255, 255, 255));
	m_graph.SetBackgroundColor(RGB(0, 0, 0));
	m_graph.SetRange(0, 24*3600-1, 0, 0, 20, 0);

	// Create the legends
	int labelNo = -1;
	for(unsigned long index = 0; index < g_settings.scannerNum; ++index){
		if(g_settings.scanner[index].spec[0].channelNum == 2 || g_settings.scanner[index].instrumentType == INSTR_HEIDELBERG){
			++labelNo;
			// the size and position of the colored square
			labelRect.top			= graphRect.bottom + margin / 2;
			labelRect.bottom	= labelRect.top + labelHeight;
			labelRect.left		= graphRect.left + (labelNo % 4) * (labelWidth + labelHeight);
			labelRect.right		= labelRect.left + labelHeight;

			// The size and position of the text label containing the serial-number
			textRect.top			= labelRect.top;
			textRect.bottom		= labelRect.bottom;
			textRect.left			= labelRect.right + 2;
			textRect.right		= textRect.left + labelWidth;

			// Create it all...
			CLegend *legend = new CLegend();

			legend->m_color	= colors[labelNo % nColors];

			legend->m_serial.Format(g_settings.scanner[index].spec[0].serialNumber);
			legend->m_label.Create("",							WS_VISIBLE | WS_CHILD | WS_BORDER, labelRect, this);
			legend->m_label.SetBackgroundColor(legend->m_color);
			legend->m_text.Create(legend->m_serial, WS_VISIBLE | WS_CHILD, textRect, this);
			legend->m_text.SetFont(font);

			m_serialLegend.AddTail(legend);
		}
	}

  // Enable the tool tips
  if(!m_toolTip.Create(this)){
    TRACE0("Failed to create tooltip control\n"); 
  }
	m_toolTip.AddTool(&m_graph,		"Shows an overview of the wind-measurements performed today");

	return TRUE;  // return TRUE unless you set the focus to a control
}

BOOL CView_WindMeasOverView::OnSetActive()
{
	// Redraw the screen
	DrawWindGraphs();

	return CPropertyPage::OnSetActive();
}

BOOL CView_WindMeasOverView::PreTranslateMessage(MSG* pMsg)
{
  m_toolTip.RelayEvent(pMsg);

	return CPropertyPage::PreTranslateMessage(pMsg);
}

LRESULT CView_WindMeasOverView::OnUpdateGraphs(WPARAM wParam, LPARAM lParam){
	// Re-draw the screen
	DrawWindGraphs();

	return 0;
}

void CView_WindMeasOverView::DrawWindGraphs(){
	const int BUFFER_SIZE = 200;
	double timeBuffer[BUFFER_SIZE];
	double wsBuffer[BUFFER_SIZE];
	double wseBuffer[BUFFER_SIZE];
	double goodWs[BUFFER_SIZE],		badWs[BUFFER_SIZE];
	double goodTime[BUFFER_SIZE],	badTime[BUFFER_SIZE];
	CString serialString;
	int nGoodWs = 0, nBadWs = 0;


	// Clear the plot
	m_graph.CleanPlot();

	// loop through all double-spectrometers
	POSITION pos = m_serialLegend.GetHeadPosition();
	while(pos != NULL){
		CLegend *legend = m_serialLegend.GetNext(pos);

		// get the serial of the spectrometer
		CString &serial = legend->m_serial;

		// Get the data
		int dataLength = m_evalDataStorage->GetWindMeasurementData(serial, timeBuffer, wsBuffer, wseBuffer, BUFFER_SIZE);

		// Sort the data into good measurements and bad ones
		for(int k = 0; k < dataLength; ++k){
			if(wseBuffer[k] > 0.0){
				goodWs[nGoodWs]			= wsBuffer[k];
				goodTime[nGoodWs]		= timeBuffer[k];
				++nGoodWs;
			}else{
				badWs[nBadWs]				= wsBuffer[k];
				badTime[nBadWs]			= timeBuffer[k];
				++nBadWs;
			}
		}

		// ---------- Plot the data ------------

		// Draw the wind-measurements as function of time
		if(0 != dataLength){
			// First draw the bad values
			m_graph.SetCircleColor(RGB(150, 150, 150));
			m_graph.XYPlot(badTime, badWs, nBadWs, CGraphCtrl::PLOT_CIRCLES | CGraphCtrl::PLOT_FIXED_AXIS);

			// Then draw the good values
			m_graph.SetCircleColor(RGB(255, 255, 255));
			m_graph.XYPlot(goodTime, goodWs, nGoodWs, CGraphCtrl::PLOT_CIRCLES | CGraphCtrl::PLOT_FIXED_AXIS);
		}
	}
}