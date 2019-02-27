// PakFileInspector.cpp : implementation file
//

#include "stdafx.h"
#include "../NovacMasterProgram.h"
#include "PakFileInspector.h"
#include "../Common/Common.h"
#include <SpectralEvaluation/File/SpectrumIO.h>

// CPakFileInspector dialog
using namespace Dialogs;

IMPLEMENT_DYNAMIC(CPakFileInspector, CDialog)
CPakFileInspector::CPakFileInspector(CWnd* pParent /*=NULL*/)
	: CDialog(CPakFileInspector::IDD, pParent)
{
	m_curSpectrum = 0;
	m_spectrumNum = 0;

	m_timer = 0;
}

CPakFileInspector::~CPakFileInspector()
{
}

void CPakFileInspector::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	// The frames
	DDX_Control(pDX, IDC_PROPERTIES_FRAME,			m_propertiesFrame);
	DDX_Control(pDX, IDC_SPECTRUM_GRAPH_FRAME,	m_graphFrame);
	DDX_Control(pDX, IDC_SPECTRUM_TABLE_FRAME,	m_headerFrame);

	// The spin button
	DDX_Control(pDX, IDC_SPECTRUM_SPINBUTTON,		m_specSpinCtrl);
}


BEGIN_MESSAGE_MAP(CPakFileInspector, CDialog)
	ON_BN_CLICKED(IDC_OPEN_PAKFILE, OnOpenPakFile)

	// Changing the spectrum to show
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPECTRUM_SPINBUTTON,		OnChangeSpectrum)

	ON_MESSAGE(WM_ZOOM,							OnZoomGraph)

	ON_WM_TIMER()
END_MESSAGE_MAP()


BOOL CPakFileInspector::OnInitDialog()
{
	CDialog::OnInitDialog();

	InitPropertiesList();
	InitHeaderList();
	InitGraph();

	m_specSpinCtrl.SetRange(0, 1);
  m_specSpinCtrl.SetPos(0);


	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CPakFileInspector::InitPropertiesList(){
	// Put out the list of properties...
	CRect rect;
	this->m_propertiesFrame.GetWindowRect(rect);
  int height = rect.bottom - rect.top;
  int width  = rect.right - rect.left;
  rect.top = 20; rect.bottom = height - 10;
  rect.left = 10; rect.right = width - 10;  
	int columnWidth = (int)(0.5*(rect.right - rect.left));
	m_propertyList.Create(WS_VISIBLE|WS_BORDER|LVS_REPORT, rect, &m_propertiesFrame, 65536);
	m_propertyList.InsertColumn(0, "", LVCFMT_LEFT, columnWidth);
	m_propertyList.InsertColumn(1, "", LVCFMT_LEFT, columnWidth);

	// Fill the list with labels...
	int index = 0;
	m_propertyList.InsertItem(index++,			"Number of spectra ");
	m_propertyList.InsertItem(index++,			"File size [kB]");
	m_propertyList.InsertItem(index++,			"Started at:");
	m_propertyList.InsertItem(index++,			"Stopped at:");
}

void CPakFileInspector::InitHeaderList(){
	Common common;

	// Put out the list of properties...
	CRect rect;
	this->m_headerFrame.GetWindowRect(rect);
  int height = rect.bottom - rect.top;
  int width  = rect.right - rect.left;
  rect.top = 0; rect.bottom = height;
  rect.left = 0; rect.right = width;
	int columnWidth = (int)(0.4*(rect.right - rect.left));
	m_headerList.Create(WS_VISIBLE|WS_BORDER|LVS_REPORT, rect, &m_headerFrame, 65536);
	m_headerList.InsertColumn(0, "", LVCFMT_LEFT, columnWidth);
	m_headerList.InsertColumn(1, "", LVCFMT_LEFT, columnWidth);

	// Fill the list with labels...
	int index = 0;
	m_headerList.InsertItem(index++,			"Header size [bytes]");
	m_headerList.InsertItem(index++,			"Header version ");
	m_headerList.InsertItem(index++,			"Size of compressed spectrum [bytes]");
	m_headerList.InsertItem(index++,			"Checksum ");
	m_headerList.InsertItem(index++,			"Spec. Name ");
	m_headerList.InsertItem(index++,			"Device ");
	m_headerList.InsertItem(index++,			"Start pixel ");
	m_headerList.InsertItem(index++,			"#Pixels ");
	m_headerList.InsertItem(index++,			"View angle [deg]");
	m_headerList.InsertItem(index++,			"#Exposures ");
	m_headerList.InsertItem(index++,			"Exposure time [ms] ");
	m_headerList.InsertItem(index++,			"Channel  ");
	m_headerList.InsertItem(index++,			"Flag ");
	m_headerList.InsertItem(index++,			"Date [ddmmyy]");
	m_headerList.InsertItem(index++,			"Start time [hhmmssdd]");
	m_headerList.InsertItem(index++,			"Stop time [hhmmssdd]");
	m_headerList.InsertItem(index++,			"Latitude [dd.ddddd]");
	m_headerList.InsertItem(index++,			"Longitude [dd.ddddd]");
	m_headerList.InsertItem(index++,			"Altitude ");
	m_headerList.InsertItem(index++,			"Measurement index ");
	m_headerList.InsertItem(index++,			"#Measurements in scan ");
	m_headerList.InsertItem(index++,			"Viewangle2 [deg]");
	m_headerList.InsertItem(index++,			"Compass dir [deg]");
	m_headerList.InsertItem(index++,			"Tilt X [deg]");
	m_headerList.InsertItem(index++,			"Tilt Y [deg]");
	m_headerList.InsertItem(index++,			"Temperature [°C]");
	m_headerList.InsertItem(index++,			"ConeAngle [deg]");
	m_headerList.InsertItem(index++,			"ADC[0] ");
	m_headerList.InsertItem(index++,			"ADC[1] ");
	m_headerList.InsertItem(index++,			"ADC[2] ");
	m_headerList.InsertItem(index++,			"ADC[3] ");
	m_headerList.InsertItem(index++,			"ADC[4] ");
	m_headerList.InsertItem(index++,			"ADC[5] ");
	m_headerList.InsertItem(index++,			"ADC[6] ");
	m_headerList.InsertItem(index++,			"ADC[7] ");

}

void CPakFileInspector::InitGraph(){
	Common common;

  // Initialize the scan graph
	CRect rect;
  this->m_graphFrame.GetWindowRect(rect);
  int height = rect.bottom - rect.top;
  int width  = rect.right - rect.left;
  rect.top = 0; rect.bottom = height;
  rect.left = 0; rect.right = width;  
  m_graph.Create(WS_VISIBLE | WS_CHILD, rect, &m_graphFrame);
	m_graph.SetRange(0, MAX_SPECTRUM_LENGTH, 1, 0.0, 4095.0, 1);
	m_graph.SetYUnits(common.GetString(AXIS_INTENSITY));
	m_graph.SetXUnits(common.GetString(AXIS_CHANNEL));
	m_graph.SetBackgroundColor(RGB(0, 0, 0)) ;
	m_graph.SetGridColor(RGB(255,255,255));//(192, 192, 255)) ;
	m_graph.SetPlotColor(RGB(0, 255,0)) ;
	m_graph.parentWnd = this;
	m_graph.ResetZoomRect();
}

// CPakFileInspector message handlers

void CPakFileInspector::OnOpenPakFile()
{
  CString fileName;
  Common common;
  TCHAR filter[512];
  int n = _stprintf(filter, "Pak-files\0");
  n += _stprintf(filter + n + 1, "*.pak\0");
  filter[n + 2] = 0;
	fileName.Format("");

	// 0. Turn off the zooming temporarily, this since if the user double-clicks
	//		in the dialog we want to show then this can be percieved as a zoom-click by the graph...
	m_graph.m_userZoomableGraph = false;

  // 1. Let the user browse for the reference file
	if(!common.BrowseForFile(filter, fileName)){
		m_graph.m_userZoomableGraph = true; // <-- Turn on the zoom again
    return;
	}

	// 2. Remember the file-name
	m_fileName.Format(fileName);
	SetDlgItemText(IDC_LABEL_PAKFILENAME, m_fileName);

	// 3. Check the selected .pak-file
	CheckPakFile();

	// 4. Turn on the zoom again in 0.5 second
	m_timer = this->SetTimer(0, 500, NULL);
}

void CPakFileInspector::CheckPakFile(){
	SpectrumIO::CSpectrumIO reader;
	CSpectrum spec;
	CFile *pFile = NULL;
	ULONGLONG fileSize = 0;
	CString str;
	CSpectrum firstSpectrum, lastSpectrum;

	// First check that the file exists...
	if(!IsExistingFile(m_fileName))
		return;

	// Get the size of the file
	try{
		pFile		= new CFile(m_fileName, CFile::modeRead | CFile::shareDenyNone);
		fileSize	= pFile->GetLength();
		fileSize	/= 1024; // we want the size in kB.
	}catch(CFileException* pEx){
		// Here I don't know what to do...
	}
	if(pFile != NULL){
		pFile->Close();
		delete pFile;
	}

	// Count the spectra
    const std::string fName((LPCSTR)m_fileName);
	m_spectrumNum = reader.CountSpectra(fName);

	// Get the start-time of the first spectrum
    reader.ReadSpectrum(fName, 0, firstSpectrum);

	// Get the stop-time of the last spectrum
    reader.ReadSpectrum(fName, m_spectrumNum - 1, lastSpectrum);

	// ---- Show the information to the user... ----
	int index = 0;
	str.Format("%d", m_spectrumNum);
	m_propertyList.SetItemText(index++, 1,		str);

	str.Format("%I64u", fileSize);
	m_propertyList.SetItemText(index++, 1, 		str);

	str.Format("%02d:%02d:%02d", firstSpectrum.m_info.m_startTime.hour, firstSpectrum.m_info.m_startTime.minute, firstSpectrum.m_info.m_startTime.second);
	m_propertyList.SetItemText(index++, 1, 		str);

	str.Format("%02d:%02d:%02d", lastSpectrum.m_info.m_startTime.hour, lastSpectrum.m_info.m_startTime.minute, lastSpectrum.m_info.m_startTime.second);
	m_propertyList.SetItemText(index++, 1, 		str);

	// --- Show the spectrum and its properties ---
	m_curSpectrum = 0;
	m_specSpinCtrl.SetRange32(0, m_spectrumNum);
	m_specSpinCtrl.SetPos32(m_curSpectrum);

	// Reset the scale of the graph
	m_graph.ResetZoomRect();

	DrawSpectrum();
	UpdateFileInfo();
	UpdateHeaderList();
}

void CPakFileInspector::OnChangeSpectrum(NMHDR *pNMHDR, LRESULT *pResult)
{
  LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);

  if(strlen(m_fileName) <= 0)
    return;

  // get the current spectrum index
  m_curSpectrum = m_specSpinCtrl.GetPos32();
  if(pNMUpDown->iDelta > 0)
    m_curSpectrum += 1;
  else
    m_curSpectrum -= 1;

  // set the limits for the selected spectrum
  m_curSpectrum = max(m_curSpectrum, 0);
  m_curSpectrum = min(m_curSpectrum, m_spectrumNum - 1);

	// Reset the scale of the graph
	m_graph.ResetZoomRect();

	// redraw the graph
  DrawSpectrum();

  // update the spectrum information
  UpdateHeaderList();

	// Update the file information
	UpdateFileInfo();

  *pResult = 0;
}

void CPakFileInspector::DrawSpectrum(){
	if(TryReadSpectrum())
		return;

	Graph::CSpectrumGraph::plotRange range;
	GetPlotRange(range);

  m_graph.CleanPlot();
	m_graph.SetRange(range.minLambda, range.maxLambda, 0, range.minIntens, range.maxIntens, 0);
	m_graph.SetPlotColor(RGB(0, 255,0));

	m_graph.XYPlot(NULL, m_spectrum.m_data, m_spectrum.m_length, Graph::CGraphCtrl::PLOT_FIXED_AXIS | Graph::CGraphCtrl::PLOT_CONNECTED);
}

void CPakFileInspector::UpdateHeaderList(){
	int index = 0;
	CString str;

	str.Format("%d",	m_spectrumHeader.hdrsize);
	m_headerList.SetItemText(index++, 1,	str);

	str.Format("%d",	m_spectrumHeader.hdrversion);
	m_headerList.SetItemText(index++, 1,	str);

	str.Format("%d",	m_spectrumHeader.size);
	m_headerList.SetItemText(index++, 1,	str);

	str.Format("%d",	m_spectrumHeader.checksum);
	m_headerList.SetItemText(index++, 1,	str);

	str.Format("%s",	m_spectrumHeader.name);
	m_headerList.SetItemText(index++, 1,	str);

	str.Format("%s",	m_spectrumHeader.instrumentname);
	m_headerList.SetItemText(index++, 1,	str);

	str.Format("%d",	m_spectrumHeader.startc);
	m_headerList.SetItemText(index++, 1,	str);

	str.Format("%d",	m_spectrumHeader.pixels);
	m_headerList.SetItemText(index++, 1,	str);

	str.Format("%d",	m_spectrumHeader.viewangle);
	m_headerList.SetItemText(index++, 1,	str);

	str.Format("%d",	m_spectrumHeader.scans);
	m_headerList.SetItemText(index++, 1,	str);

	str.Format("%d",	m_spectrumHeader.exptime);
	m_headerList.SetItemText(index++, 1,	str);

	str.Format("%d",	m_spectrumHeader.channel);
	m_headerList.SetItemText(index++, 1,	str);

	str.Format("%d",	m_spectrumHeader.flag);
	m_headerList.SetItemText(index++, 1,	str);

	str.Format("%06d",	m_spectrumHeader.date);
	m_headerList.SetItemText(index++, 1,	str);

	str.Format("%08d",	m_spectrumHeader.starttime);
	m_headerList.SetItemText(index++, 1,	str);

	str.Format("%08d",	m_spectrumHeader.stoptime);
	m_headerList.SetItemText(index++, 1,	str);

	str.Format("%lf",	m_spectrumHeader.lat);
	m_headerList.SetItemText(index++, 1,	str);

	str.Format("%lf",	m_spectrumHeader.lon);
	m_headerList.SetItemText(index++, 1,	str);

	str.Format("%d",	m_spectrumHeader.altitude);
	m_headerList.SetItemText(index++, 1,	str);

	str.Format("%d",	m_spectrumHeader.measureidx);
	m_headerList.SetItemText(index++, 1,	str);

	str.Format("%d",	m_spectrumHeader.measurecnt);
	m_headerList.SetItemText(index++, 1,	str);

	str.Format("%d",	m_spectrumHeader.viewangle2);
	m_headerList.SetItemText(index++, 1,	str);

	str.Format("%d",	m_spectrumHeader.compassdir);
	m_headerList.SetItemText(index++, 1,	str);

	str.Format("%d",	m_spectrumHeader.tiltX);
	m_headerList.SetItemText(index++, 1,	str);

	str.Format("%d",	m_spectrumHeader.tiltY);
	m_headerList.SetItemText(index++, 1,	str);

	str.Format("%f",	m_spectrumHeader.temperature);
	m_headerList.SetItemText(index++, 1,	str);

	str.Format("%d",	m_spectrumHeader.coneangle);
	m_headerList.SetItemText(index++, 1,	str);

	for(int i = 0; i < 8; ++i){
		str.Format("%d",	m_spectrumHeader.ADC[i]);
		m_headerList.SetItemText(index++, 1,	str);
	}
}

/** Update the file information */
void CPakFileInspector::UpdateFileInfo(){
	CString str;

	str.Format("Showing spectrum number %d (%s)", 1+m_curSpectrum, m_spectrumHeader.name);
	SetDlgItemText(IDC_LABEL_FILEINFO, str);

}

int CPakFileInspector::TryReadSpectrum(){
	SpectrumIO::CSpectrumIO reader;
	CString message;
	char headerBuffer[16384];
	int headerSize=0;

	// Read the spectrum
    const std::string fName((LPCSTR)m_fileName);
    const bool ret = reader.ReadSpectrum(fName, m_curSpectrum, m_spectrum, headerBuffer, 16384, &headerSize);

	if(!ret){
//    switch(reader.m_lastError){
      //case CSpectrumIO::ERROR_EOF:                   message.Format("Spectrum number %d is corrupt - EOF found", m_curSpec); break;
      //case CSpectrumIO::ERROR_COULD_NOT_OPEN_FILE:   message.Format("Could not open spectrum file"); break;
      //case CSpectrumIO::ERROR_CHECKSUM_MISMATCH:     message.Format("Spectrum number %d is corrupt - Checksum mismatch", m_curSpec); break;
      //case CSpectrumIO::ERROR_SPECTRUM_TOO_LARGE:    message.Format("Spectrum number %d is corrupt - Spectrum too large", m_curSpec); break;
      //case CSpectrumIO::ERROR_SPECTRUM_NOT_FOUND:    message.Format("Spectrum number %d not found. End of file.", m_curSpec); break; // m_BtnNextSpec.EnableWindow(FALSE); break;
      //case CSpectrumIO::ERROR_DECOMPRESS:            message.Format("Spectrum number %d is corrupt - Decompression error", m_curSpec); break;
//    }
		return 1;
	}

	// copy the header to the 'm_spectrumHeader'
	memcpy(&m_spectrumHeader, headerBuffer, headerSize * sizeof(char));

	return 0;
}


/** Gets the range of the plot */
void CPakFileInspector::GetPlotRange(Graph::CSpectrumGraph::plotRange &range){
	Graph::CSpectrumGraph::plotRange rect;

	// See if the user has determined any range...
	m_graph.GetZoomRect(rect);
	if(fabs(rect.maxLambda) > 0.1){
		range = rect;
		return;
	}else{
		long maxV;
		if(m_spectrum.m_info.m_numSpec > 0)
			maxV = 4095 * m_spectrum.m_info.m_numSpec;
		else
			maxV = (long)(4095.0f * (m_spectrum.m_info.m_peakIntensity / 4095.0f));

		range.minIntens = 0.0;
		range.maxIntens	= maxV;
		range.minLambda = 0.0;
		range.maxLambda = m_spectrum.m_length;

		return;
	}
}

/** Zooming in the graph */
LRESULT CPakFileInspector::OnZoomGraph(WPARAM wParam, LPARAM lParam){
	this->DrawSpectrum();

	return 0;
}

void Dialogs::CPakFileInspector::OnTimer(UINT nIDEvent)
{
	m_graph.m_userZoomableGraph = true;

	m_timer = KillTimer(m_timer);

	CDialog::OnTimer(nIDEvent);
}
