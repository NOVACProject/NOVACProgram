// ReferencePlotDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ReferencePlotDlg.h"

using namespace Dialogs;

//extern CString g_exePath;  // <-- This is the path to the executable. This is a global variable and should only be changed in DMSpecView.cpp

						   // CReferencePlotDlg dialog


IMPLEMENT_DYNAMIC(CReferencePlotDlg, CDialog)
CReferencePlotDlg::CReferencePlotDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CReferencePlotDlg::IDD, pParent)
{
	for (int k = 0; k < MAX_N_REFERENCES; ++k) {
		m_data[k] = new double[4096];
		m_dataLength[k] = 0;
	}
}

CReferencePlotDlg::~CReferencePlotDlg()
{
	for (int k = 0; k < MAX_N_REFERENCES; ++k) {
		delete[] m_data[k];
		m_data[k] = NULL;
	}
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

		specieName.Format("%s", (LPCTSTR)m_window->ref[k].m_specieName);
		m_label[k].Create(specieName, WS_VISIBLE | WS_CHILD, rect, this);
		m_label[k].SetFont(font);
	}

	// We need to read in the reference files...
	ReadReferences();

	// fill in the X-array
	for (int k = m_window->fitLow; k <= m_window->fitHigh; ++k) {
		this->m_number[k] = k;
	}

	// .. before we can draw them
	DrawGraph();

	return TRUE;
}

/** Called to read in the references */
void CReferencePlotDlg::ReadReferences() {
	CFileException exceFile;
	CStdioFile fileRef[100];
	CString szLine;
	CString fileName;
	long valuesReadNum = 0;
	double tmpDouble;
	int nColumns;
	int i = 0;

	int nReferenceFiles = m_window->nRef; // the number of files to read in

										  // test reading data from reference files
	for (i = 0; i < nReferenceFiles; ++i)
	{
		// get a local pointer to this reference. For simpler syntax
		double *fValue = this->m_data[i];
		m_dataLength[i] = 0;

		// Check if this is a relative or absolute path
		if (IsExistingFile(m_window->ref[i].m_path)) {
			fileName.Format(m_window->ref[i].m_path);
		}
		else {
			//fileName.Format("%s%s", g_exePath, m_window->ref[i].m_path);
			fileName.Format("%s", (LPCTSTR)m_window->ref[i].m_path);
		}

		if (!fileRef[i].Open(fileName, CFile::modeRead | CFile::typeText, &exceFile))
		{
			continue;
		}
		else {
			// read reference spectrum into a Vector
			while (fileRef[i].ReadString(szLine))
			{
				char* szToken = (char*)(LPCSTR)szLine;

				while (szToken = strtok(szToken, "\n"))
				{
					nColumns = sscanf(szToken, "%lf\t%lf", &tmpDouble, &fValue[m_dataLength[i]]);
					if (nColumns == 1)
						fValue[m_dataLength[i]] = tmpDouble;
					if (nColumns < 1 || nColumns > 2)
						break;

					// init to get next token
					szToken = NULL;
				}
				++m_dataLength[i];
				if (m_dataLength[i] == 4096)
					break;
			}
			fileRef[i].Close();
		}
	}
	return;
}

/** Called to draw the graphs */
void CReferencePlotDlg::DrawGraph() {
	int nReferenceFiles = m_window->nRef; // the number of files to read in

										  // draw the screen
	for (int k = 0; k < nReferenceFiles; ++k) {
		m_graphs[k].XYPlot(m_number + m_window->fitLow, m_data[k] + m_window->fitLow, m_window->fitHigh - m_window->fitLow);
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
