// ReEval_WindowDlg.cpp : implementation file
//

#include "stdafx.h"
#include "../NovacMasterProgram.h"
#include "ReEval_WindowDlg.h"
#include "../Dialogs/ReferencePropertiesDlg.h"
#include "../Dialogs/ReferencePlotDlg.h"
#include "../SpectralEvaluation/Utils.h"

using namespace ReEvaluation;
using namespace Evaluation;

// CReEval_WindowDlg dialog

IMPLEMENT_DYNAMIC(CReEval_WindowDlg, CPropertyPage)
CReEval_WindowDlg::CReEval_WindowDlg()
	: CPropertyPage(CReEval_WindowDlg::IDD)
{
}

CReEval_WindowDlg::~CReEval_WindowDlg()
{
}

void CReEval_WindowDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_REF_STATIC, m_referenceFrame);

	// the fit range and polynomial order
	CFitWindow &window = m_reeval->m_window[m_reeval->m_curWindow];
	DDX_Text(pDX, IDC_EDIT_FITLOW, window.fitLow);
	DDX_Text(pDX, IDC_EDIT_FITHIGH, window.fitHigh);
	DDX_Text(pDX, IDC_EDIT_POLYNOM, window.polyOrder);

	// The solar-spectrum
	DDX_Text(pDX, IDC_EDIT_SOLARSPECTRUMPATH, m_fraunhoferReferenceName);

	// The fit-types
	DDX_Radio(pDX, IDC_FIT_HP_DIV, (int &)window.fitType);

	// The extra options
	DDX_Check(pDX, IDC_CHECK_UV,            (int&)window.UV);
	DDX_Check(pDX, IDC_CHECK_FIND_OPTIMAL,  (int&)window.findOptimalShift);
	DDX_Check(pDX, IDC_CHECK_SHIFT_SKY,     (int&)window.shiftSky);

	//This list of fit-windows
	DDX_Control(pDX, IDC_FITWINDOW_LIST, m_windowList);

	// The three buttons, 'insert', 'remove', and 'properties'
	DDX_Control(pDX, IDC_BUTTON_INSERT_REFERENCE,				m_btnInsertRef);
	DDX_Control(pDX, IDC_BUTTON_REMOVE_REFERENCE,				m_btnRemoveRef);
	DDX_Control(pDX, IDC_BUTTON_REFERENCE_PROEPERTIES,	m_btnRefProp);

	// The shift-sky checkbox
	DDX_Control(pDX, IDC_CHECK_SHIFT_SKY,								m_checkShiftSky);
		
	// The 'find optimal shift' checkbox
	DDX_Control(pDX, IDC_CHECK_FIND_OPTIMAL,								m_checkFindOptimalShift);

	// The list of instrument-types
	//DDX_Control(pDX, IDC_COMBO_INSTRUMENTTYPE,					m_instrumentTypeCombo);
}


BEGIN_MESSAGE_MAP(CReEval_WindowDlg, CPropertyPage)
	// the user has selected another fit window
	ON_LBN_SELCHANGE(IDC_FITWINDOW_LIST,				OnChangeFitWindow)

	// the user has changed the type of the instrument
	//ON_CBN_SELCHANGE(IDC_COMBO_INSTRUMENTTYPE, OnChangeInstrumentType)

	// The user has pressed the 'insert' item on the reference-grid context menu
	ON_COMMAND(ID__INSERT, OnInsertReference)
	ON_BN_CLICKED(IDC_BUTTON_INSERT_REFERENCE, OnInsertReference)

	// The user has pressed the 'remove' item on the reference-grid context menu
	ON_COMMAND(ID__REMOVE, OnRemoveReference)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE_REFERENCE, OnRemoveReference)

	// The user has pressed the 'properties' item on the reference-grid context menu
	ON_COMMAND(ID__PROPERTIES, OnShowPropertiesWindow)
	ON_BN_CLICKED(IDC_BUTTON_REFERENCE_PROEPERTIES, OnShowPropertiesWindow)

	// The user has pressed the 'browse' button next to the solar-spectrum edit-box
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_SOLARSPECTRUM, OnBrowseSolarSpectrum)

	ON_BN_CLICKED(IDC_CHECK_FIND_OPTIMAL, SaveData)
	ON_BN_CLICKED(IDC_CHECK_UV,						SaveData)
	ON_BN_CLICKED(IDC_CHECK_SHIFT_SKY,		SaveData)
	ON_EN_CHANGE(IDC_EDIT_FITLOW,					SaveData)
	ON_EN_CHANGE(IDC_EDIT_FITHIGH,				SaveData)
	ON_EN_CHANGE(IDC_EDIT_POLYNOM,				SaveData)

	// When the user changes type of fitting
	ON_BN_CLICKED(IDC_FIT_HP_DIV,					SaveData)
	ON_BN_CLICKED(IDC_FIT_HP_SUB,					SaveData)
	ON_BN_CLICKED(IDC_FIT_POLY,						SaveData)

	ON_BN_CLICKED(IDC_BUTTON_REFERENCE_VIEW, &CReEval_WindowDlg::OnShowReferenceGraph)
END_MESSAGE_MAP()


// CReEval_WindowDlg message handlers

BOOL CReEval_WindowDlg::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	// Initialize the fit window-list
	m_windowList.m_reeval = this->m_reeval;
	m_windowList.PopulateList();
	m_windowList.SetCurSel(0);

	// Initialize the types of instruments
	//m_instrumentTypeCombo.AddString("Gothenburg");
	//m_instrumentTypeCombo.AddString("Heidelberg");
	//m_instrumentTypeCombo.SetCurSel(0);

	// Initialize the reference grid control
	InitReferenceFileControl();

	// Populate the reference grid control
	PopulateReferenceFileControl();

	// Update the controls
	UpdateControls();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CReEval_WindowDlg::InitReferenceFileControl(){

	// Get the dimensions of the reference frame
	CRect rect;
	m_referenceFrame.GetWindowRect(&rect);
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;

	rect.top = 20;
	rect.left = 10;
	rect.right = width - 20;
	rect.bottom = height - 10;

	// Create the grid
	m_referenceGrid.Create(rect, &m_referenceFrame, 0);
	m_referenceGrid.parent = this;

	// Set the columns of the grid
	m_referenceGrid.InsertColumn("Name");
	m_referenceGrid.SetColumnWidth(0, (int)(rect.right / 9));
	m_referenceGrid.InsertColumn("Reference File");
	m_referenceGrid.SetColumnWidth(1, (int)(rect.right * 5 / 8));
	m_referenceGrid.InsertColumn("Shift");
	m_referenceGrid.SetColumnWidth(2, (int)(rect.right / 6));
	m_referenceGrid.InsertColumn("Squeeze");
	m_referenceGrid.SetColumnWidth(3, (int)(rect.right / 6));

	// Make sure that there are two empty rows
	m_referenceGrid.SetRowCount(3);

	// Makes sure that the user cannot edit the titles of the grid
	m_referenceGrid.SetFixedRowCount(1);

	// make sure the user can edit items in the grid
	m_referenceGrid.SetEditable(TRUE); 

	// Disable the small title tips
	m_referenceGrid.EnableTitleTips(FALSE);
}

void CReEval_WindowDlg::OnChangeFitWindow()
{
	int curWindow = m_windowList.GetCurSel();
	if(curWindow < 0)
		curWindow = 0;

	// set the currently selected fit window
	m_reeval->m_curWindow = curWindow;

	// update the reference grid
	m_referenceGrid.m_window = &m_reeval->m_window[curWindow];
	PopulateReferenceFileControl();

	// Update the controls
	UpdateControls();

	// Update the rest of the controls on the screen
	UpdateData(FALSE);
}

void CReEval_WindowDlg::PopulateReferenceFileControl(){
	int curWindow = m_windowList.GetCurSel();

	if(curWindow == -1)
		curWindow = 0;

	// Clear the control
	m_referenceGrid.DeleteNonFixedRows();

	CFitWindow &window = m_reeval->m_window[curWindow];
	m_referenceGrid.m_window = &window;

	// make sure that there's always one empty line at the end
	m_referenceGrid.SetRowCount(window.nRef + 2);

	// if there's no references defined
	if(window.nRef == 0){
		m_btnRefProp.EnableWindow(FALSE);
		m_btnRemoveRef.EnableWindow(FALSE);
	}else{
		m_btnRefProp.EnableWindow(TRUE);
		m_btnRemoveRef.EnableWindow(TRUE);
	}

	for(int i = 0; i < window.nRef; ++i){

		CReferenceFile &ref = window.ref[i];

        CString specieName(ref.m_specieName.c_str());
        CString path(ref.m_path.c_str());

		m_referenceGrid.SetItemTextFmt(1 + i, 0, specieName);
		m_referenceGrid.SetItemTextFmt(1 + i, 1, path);

		if(ref.m_shiftOption == SHIFT_FREE)
            m_referenceGrid.SetItemTextFmt(1 + i, 2, "free");

		if(ref.m_shiftOption == SHIFT_FIX)
            m_referenceGrid.SetItemTextFmt(1 + i, 2, "fixed to %.2lf", ref.m_shiftValue);

		if(ref.m_shiftOption == SHIFT_LINK)
            m_referenceGrid.SetItemTextFmt(1 + i, 2, "linked to %.2lf", ref.m_shiftValue);

		if(ref.m_shiftOption == SHIFT_LIMIT)
            m_referenceGrid.SetItemTextFmt(1 + i, 2, "limited to %.2lf", ref.m_shiftValue);

		if(ref.m_squeezeOption == SHIFT_FREE)
            m_referenceGrid.SetItemTextFmt(1 + i, 3, "free");

		if(ref.m_squeezeOption == SHIFT_FIX)
            m_referenceGrid.SetItemTextFmt(1 + i, 3, "fixed to %.2lf", ref.m_squeezeValue);

		if(ref.m_squeezeOption == SHIFT_LINK)
            m_referenceGrid.SetItemTextFmt(1 + i, 3, "linked to %.2lf", ref.m_squeezeValue);

		if(ref.m_squeezeOption == SHIFT_LIMIT)
            m_referenceGrid.SetItemTextFmt(1 + i, 3, "limited to %.2lf", ref.m_squeezeValue);
	}

	// make sure that the last line is clear
	m_referenceGrid.SetItemTextFmt(window.nRef, 0, "");
	m_referenceGrid.SetItemTextFmt(window.nRef, 1, "");
	m_referenceGrid.SetItemTextFmt(window.nRef, 2, "");
	m_referenceGrid.SetItemTextFmt(window.nRef, 3, "");
}

/** Called when the user wants to insert a new reference file */
void CReEval_WindowDlg::OnInsertReference(){
	CString fileName;
	Common common;
	TCHAR filter[512];
	int n = _stprintf(filter, "Reference files\0");
	n += _stprintf(filter + n + 1, "*.txt;*.xs\0");
	filter[n + 2] = 0;
	fileName.Format("");

	// save the data in the dialog
	UpdateData(TRUE);

	// Get the currently selected fit window
	int curSel = m_windowList.GetCurSel();
	if(curSel < 0)
		curSel = 0;
	CFitWindow &window = m_reeval->m_window[curSel];

	// Let the user browse for the reference file
	if(!common.BrowseForFile(filter, fileName))
		return;

	// The user has selected a new reference file, insert it into the list

	// 1. Set the path
	window.ref[window.nRef].m_path = std::string((LPCTSTR)fileName);

	// 2. make a guess of the specie name
	CString specie;
	Common::GuessSpecieName(fileName, specie);
	if(strlen(specie) != 0){
		window.ref[window.nRef].m_specieName = std::string((LPCTSTR)specie);
	}

	// 3. update the number of references
	window.nRef += 1;

	// If this is the first reference inserted, also make a guess for the window name
	//	 (if the user has not already given the window a name)
	if(window.nRef == 1 && strlen(specie) != 0 && EqualsIgnoringCase(window.name, "SO2")){
		window.name = std::string((LPCTSTR)specie);
		m_windowList.PopulateList();
		m_windowList.SetCurSel(0);
	}

	// Update the grid
	PopulateReferenceFileControl();

}

/** Called when the user wants to remove a reference file */
void CReEval_WindowDlg::OnRemoveReference(){

	// save the data in the dialog
	UpdateData(TRUE);

	// Get the currently selected fit window
	int curSel = m_windowList.GetCurSel();
	if(curSel < 0)
		curSel = 0;
	CFitWindow &window = m_reeval->m_window[curSel];

	// if there's no reference file, then there's nothing to remove
	if(window.nRef <= 0)
		return;

	// Get the selected reference file
	CCellRange cellRange = m_referenceGrid.GetSelectedCellRange();
	int minRow = cellRange.GetMinRow() - 1;
	int nRows = cellRange.GetRowSpan();

	if(nRows <= 0 || nRows > 1) /* nothing selected or several lines selected */
		return;

	// move every reference file in the list down one step
	for(int i = minRow; i < window.nRef - 1; ++i){
		window.ref[i] = window.ref[i+1];
	}

	// reduce the number of references by one
	window.nRef -= 1;

	// Update the reference grid
	PopulateReferenceFileControl();
}

void CReEval_WindowDlg::SaveData()
{
	UpdateData(TRUE);

    CFitWindow &window          = m_reeval->m_window[m_reeval->m_curWindow];
    window.fraunhoferRef.m_path = std::string((LPCSTR)m_fraunhoferReferenceName);

	// Update the controls
	UpdateControls();
}

/** Called when the user wants to inspect the properties of a reference file */
void CReEval_WindowDlg::OnShowPropertiesWindow(){
	Dialogs::CReferencePropertiesDlg refDlg;
	int minRow, nRows;

	// save the data in the dialog
	UpdateData(TRUE);

	// Get the currently selected fit window
	int curSel = m_windowList.GetCurSel();
	if(curSel < 0)
		curSel = 0;
	CFitWindow &window = m_reeval->m_window[curSel];

	// if there's no reference file, then there's nothing to remove
	if(window.nRef <= 0)
		return;

	if(window.nRef == 1){
		minRow = 0;
	}else{
		// Get the selected reference file
		CCellRange cellRange = m_referenceGrid.GetSelectedCellRange();
		minRow = cellRange.GetMinRow() - 1;
		nRows = cellRange.GetRowSpan();

		if (nRows <= 0 || nRows > 1) { /* nothing selected or several lines selected */
			MessageBox("Please select a single reference file.", "Properties");
			return;
		}
	}

	// The selected referencefile
	CReferenceFile *ref = new CReferenceFile(window.ref[minRow]);
	refDlg.m_ref = ref;
	INT_PTR ret = refDlg.DoModal();

	if(ret == IDOK){
		window.ref[minRow] = *ref;
		PopulateReferenceFileControl();
	}

	delete ref;
}

void ReEvaluation::CReEval_WindowDlg::OnShowReferenceGraph()
{
		// save the data in the dialog
	UpdateData(TRUE);
	
	// Get the currently selected fit window
	int curSel = m_windowList.GetCurSel();
	if (curSel < 0)
		curSel = 0;
	CFitWindow &window = m_reeval->m_window[curSel];

	// if there's no reference file, then there's nothing to remove
	if (window.nRef <= 0)
		return;

	// Open the dialog
	Dialogs::CReferencePlotDlg dlg;
	dlg.m_window = m_reeval->m_window;
	dlg.DoModal();
}

//void CReEval_WindowDlg::OnChangeInstrumentType(){
//	int curType = m_instrumentTypeCombo.GetCurSel();
//	if(curType < 0)
//		curType = 0;
//
//	// Set the current type
//	switch(curType){
//		case 0:		m_reeval->m_instrumentType = INSTR_GOTHENBURG; break;
//		case 1:		m_reeval->m_instrumentType = INSTR_HEIDELBERG; break;
//		default:	m_reeval->m_instrumentType = INSTR_GOTHENBURG; break;
//	};
//}

/** Updates the controls */
void CReEval_WindowDlg::UpdateControls(){
	// Update the shift-sky check-box
	if(m_reeval->m_window[m_reeval->m_curWindow].fitType == FIT_HP_DIV){
		m_checkShiftSky.EnableWindow(FALSE);
	}else{
		m_checkShiftSky.EnableWindow(TRUE);
	}
}

void CReEval_WindowDlg::OnBrowseSolarSpectrum(){
	CString fileName;
	Common common;
	TCHAR filter[512];
	int n = _stprintf(filter, "Reference files\0");
	n += _stprintf(filter + n + 1, "*.txt;*.xs\0");
	filter[n + 2] = 0;
	fileName.Format("");

	// Find the values of the screen
	UpdateData(TRUE);

	// Let the user browse for the solar spectrum file
	if(!common.BrowseForFile(filter, fileName))
		return;

	// The user has selected a solar-spectrum file
	m_reeval->m_window[m_reeval->m_curWindow].fraunhoferRef.m_path = std::string((LPCTSTR)fileName);

	// Disable the 'Find Optimal Shift' check-box
	m_checkFindOptimalShift.EnableWindow(FALSE);

	// Update the window
	UpdateData(FALSE);

}


