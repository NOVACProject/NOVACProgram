// ScannerConfiguration.cpp : implementation file
//

#include "stdafx.h"
#include "../NovacMasterProgram.h"
#include "ScannerConfiguration.h"
#include "../Dialogs/QueryStringDialog.h"
#include "../Dialogs/SelectionDialog.h"
#include "../VolcanoInfo.h"
#include "../ObservatoryInfo.h"

using namespace ConfigurationDialog;

extern CVolcanoInfo g_volcanoes;
extern CObservatoryInfo g_observatories;

// CScannerConfiguration dialog

IMPLEMENT_DYNAMIC(CScannerConfiguration, CPropertyPage)
CScannerConfiguration::CScannerConfiguration()
    : CPropertyPage(CScannerConfiguration::IDD)
{
    m_configuration = nullptr;
}

CScannerConfiguration::~CScannerConfiguration()
{
    m_configuration = nullptr;
}

void CScannerConfiguration::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_STATIC_FRAME, m_frame);
    DDX_Control(pDX, IDC_SCANNER_TREE, m_scannerTree);
    DDX_Control(pDX, IDC_BUTTON_ADD_SCANNER, m_addScannerBtn);
    DDX_Control(pDX, IDC_BUTTON_REMOVE_SCANNER, m_removeScannerBtn);
}


BEGIN_MESSAGE_MAP(CScannerConfiguration, CPropertyPage)
    ON_NOTIFY(TVN_SELCHANGED, IDC_SCANNER_TREE, OnScannerSelectionChange)
    ON_BN_CLICKED(IDC_BUTTON_ADD_SCANNER, OnAddScanner)
    ON_BN_CLICKED(IDC_BUTTON_REMOVE_SCANNER, OnRemoveScanner)
END_MESSAGE_MAP()


// CScannerConfiguration message handlers

BOOL CScannerConfiguration::OnInitDialog()
{
    static CString titles[MAX_CHANNEL_NUM];

    CPropertyPage::OnInitDialog();

    // construct the sheet
    m_sheet.Construct("", this);

    // the location configuration
    m_pageLocation.Construct(IDD_CONFIGURE_LOCATION);
    m_pageLocation.m_configuration = m_configuration;
    m_pageLocation.m_scannerTree = &m_scannerTree;
    m_pageLocation.m_parent = this;
    m_pageLocation.m_pPSP->dwFlags |= PSP_PREMATURE;

    // the evaluation configuration
    for (int k = 0; k < MAX_CHANNEL_NUM; ++k) {
        m_pageEvaluation[k].Construct(IDD_CONFIGURE_EVALUATION);
        m_pageEvaluation[k].m_configuration = m_configuration;
        m_pageEvaluation[k].m_scannerTree = &m_scannerTree;
        m_pageEvaluation[k].m_parent = this;
        m_pageEvaluation[k].m_channel = k;
        if (k == 0)
            titles[k].Format("Evaluation-Master");
        else
            titles[k].Format("Evaluation-Slave %d", k);
        m_pageEvaluation[k].m_pPSP->dwFlags |= PSP_USETITLE | PSP_PREMATURE;
        m_pageEvaluation[k].m_pPSP->pszTitle = titles[k].GetBuffer();
        m_showEvalPage[k] = true;   /** m_showEvalPage[i] is true if m_pageEvaluation[i] is visible */
        m_showWindPage = true;      /** m_showWindPage is true if the wind configuration page is visible */
        //m_showVIIPage	 = true;    /** m_showVIIPage is true if the version2 configuration page is visible */
    }

    // the wind configuration
    m_pageWind.Construct(IDD_CONFIGURE_WIND);
    m_pageWind.m_configuration = m_configuration;
    m_pageWind.m_scannerTree = &m_scannerTree;
    m_pageWind.m_parent = NULL;
    m_pageWind.m_pPSP->dwFlags |= PSP_PREMATURE;

    // the advanced V-II configuration
    //m_pageVII.Construct(IDD_CONFIGURE_VII_ADVANCED);
    //m_pageVII.m_configuration = m_configuration;
    //m_pageVII.m_scannerTree = &m_scannerTree;
    //m_pageVII.m_parent = NULL;
    //m_pageVII.m_pPSP->dwFlags |= PSP_PREMATURE;

    // the communication configuration
    m_pageCommunication.Construct(IDD_CONFIGURE_COM_PORT);
    m_pageCommunication.m_configuration = m_configuration;
    m_pageCommunication.m_scannerTree = &m_scannerTree;

    // the dark configuration
    //m_pageDark.Construct(IDD_CONFIGURE_DARK);
    //m_pageDark.m_configuration = m_configuration;
    //m_pageDark.m_scannerTree = &m_scannerTree;

    // the remote configuration
    //m_pageRemote.Construct(IDD_CONFIGURATION_REMOTE);
    //m_pageRemote.m_configuration = m_configuration;
    //m_pageRemote.m_scannerTree = &m_scannerTree;
    //m_pageRemote.m_pPSP->dwFlags |= PSP_PREMATURE;

    // instrument calibration
    m_pageCalibration.Construct(IDD_CONFIGURE_CALIBRATION);
    m_pageCalibration.m_configuration = m_configuration;
    m_pageCalibration.m_channel = 0;
    m_pageCalibration.m_scannerTree = &m_scannerTree;

    // add the pages to the sheet
    m_sheet.AddPage(&m_pageLocation);
    for (int k = 0; k < MAX_CHANNEL_NUM; ++k)
        m_sheet.AddPage(&m_pageEvaluation[k]);
    m_sheet.AddPage(&m_pageWind);
    //m_sheet.AddPage(&m_pageVII);
    m_sheet.AddPage(&m_pageCommunication);
    m_sheet.AddPage(&m_pageCalibration);

    // find the position of the sheet
    CRect rect, rect2;
    int margin = 25;
    this->m_frame.GetWindowRect(rect);
    int width = rect.right - rect.left - margin;
    int height = rect.bottom - rect.top - margin;
    this->m_scannerTree.GetWindowRect(rect2);

    m_sheet.Create(this, WS_CHILD | WS_VISIBLE | WS_TABSTOP);
    m_sheet.ModifyStyleEx(0, WS_EX_CONTROLPARENT);
    m_sheet.MoveWindow(rect2.right + 2 * margin - rect2.left, margin, width, height, TRUE);


    // populate the tree control
    PopulateScannerList();

    // Enable the tool tips
    if (!m_toolTip.Create(this)) {
        TRACE0("Failed to create tooltip control\n");
    }
    m_toolTip.AddTool(&m_addScannerBtn, IDC_BUTTON_ADD_SCANNER);
    m_toolTip.AddTool(&m_removeScannerBtn, IDC_BUTTON_REMOVE_SCANNER);
    m_toolTip.AddTool(&m_scannerTree, IDC_SCANNER_TREE);

    // adding tooltips for the tabs in the window
    CTabCtrl* tabPtr = m_sheet.GetTabControl();
    tabPtr->GetItemRect(0, &rect);
    m_toolTip.AddTool(tabPtr, IDD_CONFIGURE_LOCATION, &rect, IDD_CONFIGURE_LOCATION);
    tabPtr->GetItemRect(1, &rect);
    m_toolTip.AddTool(tabPtr, IDD_CONFIGURE_EVALUATION, &rect, IDD_CONFIGURE_EVALUATION);
    tabPtr->GetItemRect(2, &rect);
    m_toolTip.AddTool(tabPtr, IDD_CONFIGURE_COM_PORT, &rect, IDD_CONFIGURE_COM_PORT);

    tabPtr->SetToolTips(&m_toolTip);
    tabPtr->EnableToolTips(TRUE);

    m_toolTip.SetMaxTipWidth(SHRT_MAX);
    m_toolTip.Activate(TRUE);

    // if there's no scanners configured then we need to call 'OnChangeScanner'
    //	manually to make sure that the controls are enabled/disabled properly
    if (m_configuration->scannerNum == 0) {
        OnChangeScanner();
        m_removeScannerBtn.EnableWindow(FALSE);
    }

    return TRUE;  // return TRUE unless you set the focus to a control
}

void CScannerConfiguration::OnScannerSelectionChange(NMHDR* pNMHDR, LRESULT* pResult) {
    OnChangeScanner();
}

void CScannerConfiguration::OnChangeScanner() {

    // Get the currently selected spectrometer and scanning instrument

    if (m_sheet.m_hWnd == nullptr)
        return;

    int currentScanner = 0;
    int currentSpec = 0;
    GetScannerAndSpec(currentScanner, currentSpec);

    if (currentScanner == -1 || currentSpec == -1)
    {
        // no scanner and no spectrometer selected - this will not work. Disable all controls
        if (m_configuration->scannerNum == 0) {
            m_sheet.ShowWindow(FALSE);
        }
        return;
    }

    m_sheet.ShowWindow(TRUE);

    // Get the number of channels on the spectrometer
    int nChannels = m_configuration->scanner[currentScanner].spec[currentSpec].channelNum;

    // ---- This is a terribly ugly way to re-arrange the pages ----
    bool change = false;
    for (int k = 0; k < nChannels; ++k) {
        if (!m_showEvalPage[k]) {
            m_showEvalPage[k] = true;
            m_sheet.AddPage(&m_pageEvaluation[k]);
            change = true;
        }
    }
    for (int k = nChannels; k < MAX_CHANNEL_NUM; ++k) {
        if (m_showEvalPage[k]) {
            m_showEvalPage[k] = false;
            m_sheet.RemovePage(&m_pageEvaluation[k]);
            change = true;
        }
    }

    // Show or not to show the Version2 page
    //if (m_showVIIPage) {
    //	m_sheet.RemovePage(&m_pageVII);
    //	m_showVIIPage = false;
    //}

    // show or not to show the windpage
    if (nChannels <= 1 && m_showWindPage) {
        m_sheet.RemovePage(&m_pageWind);
        m_showWindPage = false;
    }

    if (nChannels > 1 && !m_showWindPage) {
        m_sheet.AddPage(&m_pageWind);
        m_showWindPage = true;
    }

    if (change) {
        int active = m_sheet.GetActiveIndex();
        m_sheet.RemovePage(&m_pageCommunication);
        m_sheet.RemovePage(&m_pageCalibration);
        m_sheet.AddPage(&m_pageCommunication);
        m_sheet.AddPage(&m_pageCalibration);
        m_sheet.SetActivePage(active);
    }

    // Tell all windows that we've changed scanner
    m_pageCommunication.OnChangeScanner();
    m_pageLocation.OnChangeScanner();
    m_pageCalibration.OnChangeScanner();
    for (int k = 0; k < MAX_CHANNEL_NUM; ++k) {
        if (m_showEvalPage[k])
        {
            m_pageEvaluation[k].OnChangeScanner();
        }
    }
    //if(m_showVIIPage)
    //	m_pageVII.OnChangeScanner();
    if (m_showWindPage)
        m_pageWind.OnChangeScanner();
    //m_pageRemote.OnChangeScanner();
}

/** Called when the user has clicked the button 'Add Scanner' */
void CScannerConfiguration::OnAddScanner() {

    // 1. Check if there's any space to insert any scanner
    if (MAX_NUMBER_OF_SCANNING_INSTRUMENTS == m_configuration->scannerNum) {
        MessageBox("Cannot add scanning instrument to list. Too many instrument configured already");
        return;
    }

    // 2. Ask the user for the volcano where the scanner should be placed
    Dialogs::CSelectionDialog volcanoDialog;
    CString volcano;
    for (unsigned int k = 0; k < g_volcanoes.m_volcanoNum; ++k) {
        volcanoDialog.m_option[k].Format(g_volcanoes.m_name[k]);
    }
    volcanoDialog.m_option[g_volcanoes.m_volcanoNum].Format("Other");
    volcanoDialog.m_windowText.Format("What's the volcano name?");
    volcanoDialog.m_currentSelection = &volcano;
    INT_PTR ret = volcanoDialog.DoModal();

    if (IDCANCEL == ret) // if the user clicked 'cancel' then don't insert anything
        return;

    if (volcano == "Other") {
        m_pageLocation.AddAVolcano();
        unsigned int idx = g_volcanoes.m_volcanoNum - 1;
        volcano = g_volcanoes.m_name[idx];
    }

    // 3. Ask the user for the serial number
    Dialogs::CQueryStringDialog serialNumberDialog;
    CString serialNumber;
    serialNumberDialog.m_windowText.Format("The serial number of the spectrometer?");
    serialNumberDialog.m_inputString = &serialNumber;
    ret = serialNumberDialog.DoModal();

    if (IDCANCEL == ret) // if the user clicked 'cancel' then don't insert anything
        return;

    // 4. Ask the user for the number of channels on the spectrometer
    Dialogs::CSelectionDialog channelsDialog;
    CString channel;
    for (int k = 0; k < MAX_CHANNEL_NUM; ++k) {
        channelsDialog.m_option[k].Format("%d", k + 1);
    }
    channelsDialog.m_windowText.Format("How many channels are there on this spectrometer?");
    channelsDialog.m_currentSelection = &channel;
    ret = channelsDialog.DoModal();

    if (IDCANCEL == ret) // if the user clicked 'cancel' then don't insert anything
        return;

    // 4b. Get the number of channels chosen
    int nChannels;
    int sret = sscanf(channel, "%d", &nChannels);

    // 5. Make a guess for the observatory name
    if (m_configuration->scannerNum >= 1) {
        m_configuration->scanner[m_configuration->scannerNum].observatory.Format(m_configuration->scanner[m_configuration->scannerNum - 1].observatory);
    }
    else {
        // search for the volcano in the list of volcanoes. If found, then use the preconfigured observatory
        for (unsigned int i = 0; i < g_volcanoes.m_volcanoNum; ++i) {
            if (Equals(g_volcanoes.m_name[i], volcano)) {
                int obs = g_volcanoes.m_observatory[i];
                m_configuration->scanner[m_configuration->scannerNum].observatory.Format(g_observatories.m_name[obs]);
                break;
            }
        }
    }

    // 6. insert the new scanning instrument
    m_configuration->scanner[m_configuration->scannerNum].spec[0].serialNumber.Format("%s", (LPCSTR)serialNumber);
    m_configuration->scanner[m_configuration->scannerNum].spec[0].channelNum = nChannels;
    m_configuration->scanner[m_configuration->scannerNum].volcano.Format("%s", (LPCSTR)volcano);
    m_configuration->scanner[m_configuration->scannerNum].specNum = 1;
    m_configuration->scannerNum += 1;

    // 7. Update the scanner list
    unsigned int i = m_configuration->scannerNum - 1;
    HTREEITEM hTree = m_scannerTree.InsertItem(m_configuration->scanner[i].volcano);
    for (unsigned int j = 0; j < m_configuration->scanner[i].specNum; ++j) {
        m_scannerTree.InsertItem(m_configuration->scanner[i].spec[j].serialNumber, hTree);
    }
    m_scannerTree.Expand(hTree, TVE_EXPAND);
    m_scannerTree.SelectItem(hTree);
    if (m_configuration->scannerNum == 1)
        m_scannerTree.SetRedraw();

    // 8. Update the name of the file that the program should read from the server...
    for (unsigned int i = 0; i < g_volcanoes.m_volcanoNum; ++i) {
        if (Equals(g_volcanoes.m_name[i], volcano)) {
            m_configuration->windSourceSettings.windFieldFile.Format("sftp://ors20.see.chalmers.se/Wind/wind_%s.txt", (LPCSTR)g_volcanoes.m_simpleName[i]);
        }
    }

    // Since there's now at least one scanner in the list,
    //	make sure that it's possible to remove it...
    m_removeScannerBtn.EnableWindow(TRUE);

    /** Update the dialog */
    UpdateData(FALSE);
}

/** Called when the user has clicked the button 'Remove Scanner' */
void CScannerConfiguration::OnRemoveScanner() {
    int currentScanner, currentSpec;
    GetScannerAndSpec(currentScanner, currentSpec);
    if (currentScanner == -1 || currentSpec == -1) {
        return;
    }

    CString message;
    message.Format("Are you sure you want to remove scanner: %s from the list?", (LPCSTR)m_configuration->scanner[currentScanner].spec[currentSpec].serialNumber);
    int answer = MessageBox(message, NULL, MB_YESNO);
    if (IDNO == answer) {
        return;
    }

    // The currently selected item in the tree
    HTREEITEM hTree = m_scannerTree.GetSelectedItem();

    // Move the focus to another node in the tree. This lets the 
    // dialogs update their data before we delete anything
    HTREEITEM parent = m_scannerTree.GetParentItem(hTree);
    HTREEITEM sibling = m_scannerTree.GetNextSiblingItem(hTree);
    if (sibling == NULL) {
        sibling = m_scannerTree.GetPrevSiblingItem(hTree);
    }
    if (sibling == NULL) {
        HTREEITEM next = m_scannerTree.GetNextItem(hTree, TVGN_NEXTVISIBLE);
        if (next == NULL) {
            HTREEITEM first = m_scannerTree.GetFirstVisibleItem();
            m_scannerTree.SelectItem(first);
        }
        else {
            m_scannerTree.SelectItem(next);
        }
    }
    else {
        m_scannerTree.SelectItem(sibling);
    }

    /** remove the scanner from the list */
    for (int i = currentScanner; i < MAX_NUMBER_OF_SCANNING_INSTRUMENTS - 1; ++i) {
        m_configuration->scanner[i] = m_configuration->scanner[i + 1];
    }

    /** Update the scanner list */
    m_scannerTree.DeleteItem(hTree);
    if (parent != NULL && sibling == NULL) {
        m_scannerTree.DeleteItem(parent);
    }

    m_configuration->scannerNum -= 1;

    /** disable Remove button if no more scanners in tree */
    if (m_configuration->scannerNum < 1) {
        m_removeScannerBtn.EnableWindow(FALSE);

        // TODO: reset page defaults
    }

    /** Update the dialog */
    UpdateData(FALSE);
}

BOOL ConfigurationDialog::CScannerConfiguration::PreTranslateMessage(MSG* pMsg) {
    m_toolTip.RelayEvent(pMsg);

    return CPropertyPage::PreTranslateMessage(pMsg);
}


void ConfigurationDialog::CScannerConfiguration::PopulateScannerList() {
    m_scannerTree.m_configuration = this->m_configuration;
    m_scannerTree.PopulateTreeControl();

    return;
}

void CScannerConfiguration::GetScannerAndSpec(int& curScanner, int& curSpec) {
    curSpec = 0;
    curScanner = 0;
    HTREEITEM hTree = m_scannerTree.GetSelectedItem();
    if (hTree == NULL) {
        // nothing selected
        curSpec = -1;
        curScanner = -1;
        return;
    }

    return m_scannerTree.GetCurScanAndSpec(curScanner, curSpec);
}

