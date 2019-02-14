// Configuration\EvaluationConfiguration.cpp : implementation file
//

#include "stdafx.h"
#include "../NovacMasterProgram.h"
#include "EvaluationConfigurationDlg.h"
#include "ScannerConfiguration.h"
#include "../SpectralEvaluation/Evaluation/ReferenceFile.h"
#include "../Dialogs/SelectionDialog.h"
#include "../Dialogs/ReferencePropertiesDlg.h"
#include "../Dialogs/ReferencePlotDlg.h"
#include "../VolcanoInfo.h"

using namespace ConfigurationDialog;
using namespace Evaluation;

// CEvaluationConfiguration dialog

IMPLEMENT_DYNAMIC(CEvaluationConfigurationDlg, CSystemConfigurationPage)

CEvaluationConfigurationDlg::CEvaluationConfigurationDlg()
	: CSystemConfigurationPage(CEvaluationConfigurationDlg::IDD)
{
	m_configuration = NULL;
	m_scannerTree = NULL;
	m_parent = NULL;
	m_channel = 0;
}

CEvaluationConfigurationDlg::~CEvaluationConfigurationDlg()
{
	m_configuration = NULL;
	m_scannerTree = NULL;
	m_parent = NULL;
}

void CEvaluationConfigurationDlg::DoDataExchange(CDataExchange* pDX)
{
	CSystemConfigurationPage::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_STATIC_REFERENCEFILES,		m_referenceStatic);
	DDX_Control(pDX, IDC_BUTTON_ADDREFERENCE,		m_addReferenceBtn);
	DDX_Control(pDX, IDC_BUTTON_REMOVEREFERENCE,	m_removeReferenceButton);

	// The labels (only used for showing tooltips);
	DDX_Control(pDX, IDC_LABEL_FITLOW,				m_labelFitFrom);
	DDX_Control(pDX, IDC_LABEL_FITHIGH,				m_labelFitTo);

	// The edits (only used for showing tooltips);
	DDX_Control(pDX, IDC_EDIT_FITLOW,				m_editFitFrom);
	DDX_Control(pDX, IDC_EDIT_FITHIGH,				m_editFitTo);

	if(m_curSpec != NULL){
		DDX_Text(pDX, IDC_EDIT_FITLOW,				m_curSpec->channel[m_channel].fitWindow.fitLow);
		DDX_Text(pDX, IDC_EDIT_FITHIGH,				m_curSpec->channel[m_channel].fitWindow.fitHigh);
	}
}


BEGIN_MESSAGE_MAP(CEvaluationConfigurationDlg, CSystemConfigurationPage)
	// insert a reference
	ON_BN_CLICKED(IDC_BUTTON_ADDREFERENCE, OnAddReferenceFile)
	ON_COMMAND(ID__INSERT, OnAddReferenceFile)

	// remove a reference
	ON_BN_CLICKED(IDC_BUTTON_REMOVEREFERENCE, OnRemoveReferenceFile)
	ON_COMMAND(ID__REMOVE, OnRemoveReferenceFile)

	// The user has pressed the 'properties' item on the reference-grid context menu
	ON_COMMAND(ID__PROPERTIES, OnShowPropertiesWindow)
	ON_BN_CLICKED(IDC_BUTTON_PROPERTIES, OnShowPropertiesWindow)

	// immediately save all changes made in the dialog
	ON_EN_CHANGE(IDC_EDIT_FITLOW, SaveData)
	ON_EN_CHANGE(IDC_EDIT_FITHIGH, SaveData)
	ON_BN_CLICKED(IDC_BUTTON_VIEW, &CEvaluationConfigurationDlg::OnShowReferenceGraph)
END_MESSAGE_MAP()


BOOL CEvaluationConfigurationDlg::OnInitDialog()
{
	Common common;

	CDialog::OnInitDialog();

	// Initialize the reference grid control
	InitReferenceFileControl();
	PopulateReferenceFileControl();

	UpdateData(FALSE);

	// Set up the tool tips
	InitToolTips();


	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

/** Setup the tool tips */
void CEvaluationConfigurationDlg::InitToolTips(){
	if(m_toolTip.m_hWnd != NULL)
		return;

	// Enable the tool tips
	if(!m_toolTip.Create(this)){
		TRACE0("Failed to create tooltip control\n"); 
	}
	m_toolTip.AddTool(&m_addReferenceBtn,		IDC_BUTTON_ADDREFERENCE);
	m_toolTip.AddTool(&m_removeReferenceButton,	IDC_BUTTON_REMOVEREFERENCE);
	m_toolTip.AddTool(&m_labelFitFrom,			IDC_EDIT_FITLOW);
	m_toolTip.AddTool(&m_editFitFrom,			IDC_EDIT_FITLOW);
	m_toolTip.AddTool(&m_labelFitTo,			IDC_EDIT_FITHIGH);
	m_toolTip.AddTool(&m_editFitTo,				IDC_EDIT_FITHIGH);

	m_toolTip.SetMaxTipWidth(SHRT_MAX);
	m_toolTip.Activate(TRUE);
}

BOOL CEvaluationConfigurationDlg::OnKillActive(){
	//if((currentSpec >= 0) && (currentSpec < MAX_NUMBER_OF_SCANNING_INSTRUMENTS)){
	//  CConfigurationSetting::SpectrometerSetting *spec = &m_configuration->scanner[currentScanner].spec[currentSpec];
	//  
	//  // Check if there's any references defined
	//  long numberOfReferences = spec->channel[m_channel].fitWindow.nRef;
	//  if(numberOfReferences <= 0){
	//    CString msg;
	//    msg.Format("There are no references defined for %s. The program will not run properly without one defined. Please add at least one reference file", spec->serialNumber);
	//    MessageBox(msg, "Error", MB_OK);
	//    return 0;
	//  }
	//}

	return CSystemConfigurationPage::OnKillActive();
}

void CEvaluationConfigurationDlg::InitReferenceFileControl(){
	if(m_configuration != NULL && m_curSpec != NULL){
		m_referenceFileCtrl.m_window = &m_curSpec->channel[m_channel].fitWindow;
	}

	CRect rect;
	m_referenceStatic.GetWindowRect(&rect);
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;

	if(m_referenceFileCtrl.m_hWnd != NULL)
		return;

	rect.top = 20;
	rect.left = 10;
	rect.right = width - 20;
	rect.bottom = height - 10;
	this->m_referenceFileCtrl.Create(rect, &m_referenceStatic, 0);

	m_referenceFileCtrl.InsertColumn("Name");
	m_referenceFileCtrl.SetColumnWidth(0, (int)(rect.right / 9));
	m_referenceFileCtrl.InsertColumn("Reference File");
	m_referenceFileCtrl.SetColumnWidth(1, (int)(rect.right * 5 / 8));
	m_referenceFileCtrl.InsertColumn("Shift");
	m_referenceFileCtrl.SetColumnWidth(2, (int)(rect.right / 8));
	m_referenceFileCtrl.InsertColumn("Squeeze");
	m_referenceFileCtrl.SetColumnWidth(3, (int)(rect.right / 8));

	m_referenceFileCtrl.SetFixedRowCount(1);
	m_referenceFileCtrl.SetEditable(TRUE); /* make sure the user can edit the positions */
	m_referenceFileCtrl.SetRowCount(3);
	m_referenceFileCtrl.EnableTitleTips(FALSE);	// <-- Disable the small title tips
	m_referenceFileCtrl.parent = this;
}

/** Fills in the reference file control */
void CEvaluationConfigurationDlg::PopulateReferenceFileControl(){
	long i;

	if(m_configuration == NULL || m_curSpec == NULL)
		return;

	// Clear the control
	m_referenceFileCtrl.DeleteNonFixedRows();

	m_referenceFileCtrl.m_window = &m_curSpec->channel[m_channel].fitWindow;

	long numberOfReferences = m_curSpec->channel[m_channel].fitWindow.nRef;
	m_referenceFileCtrl.SetRowCount(2 + numberOfReferences);

	for(i = 0; i < numberOfReferences; ++i){
		Evaluation::CReferenceFile &ref = m_curSpec->channel[m_channel].fitWindow.ref[i];

		m_referenceFileCtrl.SetItemTextFmt(i+1, 0, CString(ref.m_specieName.c_str()));
		m_referenceFileCtrl.SetItemTextFmt(i+1, 1, CString(ref.m_path.c_str()));

		if(ref.m_shiftOption== SHIFT_FREE)
		m_referenceFileCtrl.SetItemTextFmt(1 + i, 2, "free");

		if(ref.m_shiftOption == SHIFT_FIX)
		m_referenceFileCtrl.SetItemTextFmt(1 + i, 2, "fix to %.2lf", ref.m_shiftValue);

		if(ref.m_shiftOption == SHIFT_LINK)
		m_referenceFileCtrl.SetItemTextFmt(1 + i, 2, "link to %.2lf", ref.m_shiftValue);

		if(ref.m_shiftOption == SHIFT_LIMIT)
		m_referenceFileCtrl.SetItemTextFmt(1 + i, 2, "limit to %.2lf", ref.m_shiftValue);

		if(ref.m_squeezeOption == SHIFT_FREE)
		m_referenceFileCtrl.SetItemTextFmt(1 + i, 3, "free");

		if(ref.m_squeezeOption == SHIFT_FIX)
		m_referenceFileCtrl.SetItemTextFmt(1 + i, 3, "fix to %.2lf", ref.m_squeezeValue);

		if(ref.m_squeezeOption == SHIFT_LINK)
		m_referenceFileCtrl.SetItemTextFmt(1 + i, 3, "link to %.2lf", ref.m_squeezeValue);

		if(ref.m_squeezeOption == SHIFT_LIMIT)
		m_referenceFileCtrl.SetItemTextFmt(1 + i, 3, "limit to %.2lf", ref.m_squeezeValue);
	}
	// the last line should be cleared
	m_referenceFileCtrl.SetItemTextFmt(i + 1, 0, "");
	m_referenceFileCtrl.SetItemTextFmt(i + 1, 1, "");
	m_referenceFileCtrl.SetItemTextFmt(i + 1, 2, "");
	m_referenceFileCtrl.SetItemTextFmt(i + 1, 3, "");

	// Update the grid
	m_referenceFileCtrl.UpdateData(FALSE);
}

/** Adding a reference file */
void CEvaluationConfigurationDlg::OnAddReferenceFile(){
	CString fileName;
	TCHAR filter[512];
	int n = _stprintf(filter, "Reference files\0");
	n += _stprintf(filter + n + 1, "*.txt;*.xs\0");
	filter[n + 2] = 0;
	Common common;

	if(m_curSpec == NULL)
		return;

	// 1. Check if the reference-file-list is full
	if(m_curSpec->channel[m_channel].fitWindow.nRef == MAX_N_REFERENCES){
		MessageBox("Cannot add more reference files to this scanning instrument");
		return;
	}

	// 2. Let the user browse for the file
	fileName.Format("");
	if(!common.BrowseForFile(filter, fileName)){
		return;
	}
	m_curSpec->channel[m_channel].fitWindow.ref[m_curSpec->channel[m_channel].fitWindow.nRef].m_path = std::string(fileName);

	// 3. Make a guess for the specie name
	CString specie;
	Common::GuessSpecieName(fileName, specie);
	if(strlen(specie) != 0){
		m_curSpec->channel[m_channel].fitWindow.ref[m_curSpec->channel[m_channel].fitWindow.nRef].m_specieName = std::string((LPCSTR)specie);
	}else{
		// Could not guess for a specie name. Let the user select one.
		Dialogs::CSelectionDialog selectionDlg;
		CString selection;
		std::string species[] = {"SO2", "O3", "NO2", "Ring", "BrO"};
		int nSpecies = 5;
		int index = 0;
		for(int i = 0; i < nSpecies; ++i){
		    bool insert = true;

		    /** Check if there's already specie with the specified name inserted 
			    If so */
		    for(int j = 0; j < m_curSpec->channel[m_channel].fitWindow.nRef; ++j){
			    if(m_curSpec->channel[m_channel].fitWindow.ref[j].m_specieName.compare(species[i]) == 0)
                {
			        insert = false;
			        break;
			    }
		    }
		    if(insert)
            {
			    selectionDlg.m_option[index++].Format("%s", species[i].c_str());
            }
		}
		selectionDlg.m_windowText.Format("Select Specie");
		selectionDlg.m_currentSelection = &selection;
		INT_PTR ret = selectionDlg.DoModal();

		if(IDCANCEL == ret)
		return;

		m_curSpec->channel[m_channel].fitWindow.ref[m_curSpec->channel[m_channel].fitWindow.nRef].m_specieName = std::string((LPCSTR)selection);
	}
	m_curSpec->channel[m_channel].fitWindow.nRef += 1;

	PopulateReferenceFileControl();
}

/** Removign a reference file */
void CEvaluationConfigurationDlg::OnRemoveReferenceFile(){
	/** Get the range of selected cells */
	CCellRange cellRange = m_referenceFileCtrl.GetSelectedCellRange();
	int minRow = cellRange.GetMinRow() - 1;
	int nRows = cellRange.GetRowSpan();

	if(m_curSpec == NULL)
		return;

	if(nRows <= 0){
		if(m_curSpec->channel[m_channel].fitWindow.nRef > 1)
		return;   /* nothing selected*/
		else
		minRow = 0; /* if there's only one reference file, assume that the user want's to remove it, even though it's not marked*/
	}

	if(nRows > 1)
		return; /** Several lines selected */

	if(minRow >= m_curSpec->channel[m_channel].fitWindow.nRef)
		return; // The last, empty, line is selected for removal


	// the 'are you sure?' - message
	CString message;
	message.Format("Are you sure you want to delete the reference for specie %s ?", m_curSpec->channel[m_channel].fitWindow.ref[minRow].m_specieName.c_str());
	int answer = MessageBox(message, "Delete Reference File", MB_YESNO);
	if(IDNO == answer)
		return;

	// remove the reference file
	for(int i = minRow; i < m_curSpec->channel[m_channel].fitWindow.nRef; ++i){
		m_curSpec->channel[m_channel].fitWindow.ref[i] = m_curSpec->channel[m_channel].fitWindow.ref[i+1];
	}
	m_curSpec->channel[m_channel].fitWindow.nRef -= 1;

	PopulateReferenceFileControl();

}

void CEvaluationConfigurationDlg::SaveData(){
	UpdateData(TRUE);
}

void CEvaluationConfigurationDlg::UpdateDlg(){
	UpdateData(FALSE);
}

void CEvaluationConfigurationDlg::OnChangeScanner(){
	if(UpdateData(TRUE)){
		CSystemConfigurationPage::OnChangeScanner();
		UpdateData(FALSE);

		PopulateReferenceFileControl();
	}
}

BOOL ConfigurationDialog::CEvaluationConfigurationDlg::PreTranslateMessage(MSG* pMsg){
	m_toolTip.RelayEvent(pMsg);

	return CSystemConfigurationPage::PreTranslateMessage(pMsg);
}

BOOL ConfigurationDialog::CEvaluationConfigurationDlg::OnSetActive()
{
	UpdateData(FALSE);
	PopulateReferenceFileControl();
	return CSystemConfigurationPage::OnSetActive();
}


/** Showing the properties of a reference file */
void CEvaluationConfigurationDlg::OnShowPropertiesWindow(){
	Dialogs::CReferencePropertiesDlg refDlg;
	int minRow, nRows;

	if(m_curSpec == NULL)
		return;

	// save the data in the dialog
	UpdateData(TRUE);

	// Get the currently selected fit window
	CFitWindow &window = m_curSpec->channel[m_channel].fitWindow;

	// if there's no reference file, then there's nothing to show the properties for
	if(window.nRef <= 0)
		return;

	if(window.nRef == 1){
		minRow = 0;
	}else{
		// Get the selected reference file
		CCellRange cellRange = m_referenceFileCtrl.GetSelectedCellRange();
		minRow = cellRange.GetMinRow() - 1;
		nRows = cellRange.GetRowSpan();

		if(nRows <= 0){
			if (m_curSpec->channel[m_channel].fitWindow.nRef > 1) {
				MessageBox("Please select a reference file.", "Properties");
				return;   /* nothing selected*/

			}
			else
				minRow = 0; /* if there's only one reference file, assume that the user want's to remove it, even though it's not marked*/
		}

		if (nRows > 1) {
			MessageBox("Please select a single reference file.", "Properties");
			return; // <-- Several lines selected
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

void ConfigurationDialog::CEvaluationConfigurationDlg::OnShowReferenceGraph()
{

	if (m_curSpec == NULL)
		return;

	// save the data in the dialog
	UpdateData(TRUE);

	// Get the currently selected fit window
	CFitWindow &window = m_curSpec->channel[m_channel].fitWindow;

	// if there's no reference file, then there's nothing to show
	if (window.nRef <= 0) {
		return;
	}

	// Open the dialog
	Dialogs::CReferencePlotDlg dlg;
	dlg.m_window = &m_curSpec->channel[m_channel].fitWindow;
	dlg.DoModal();
}
