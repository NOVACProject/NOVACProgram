// ReferencePropertiesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "../NovacMasterProgram.h"
#include "ReferencePropertiesDlg.h"
#include "../Common/Common.h"

using namespace Dialogs;
using namespace novac;

// CReferencePropertiesDlg dialog

IMPLEMENT_DYNAMIC(CReferencePropertiesDlg, CDialog)

CReferencePropertiesDlg::CReferencePropertiesDlg(novac::CReferenceFile& ref, CWnd* pParent /*=nullptr*/)
    : CDialog(CReferencePropertiesDlg::IDD, pParent), m_ref(ref)
{
    m_shiftOption = 1;
    m_squeezeOption = 1;

    m_shiftFixValue = 0.0;
    m_shiftLinkValue = 0.0;
    m_shiftLimitLow = -1.0;
    m_shiftLimitHigh = 1.0;
    m_squeezeFixValue = 1.0;
    m_squeezeLinkValue = 0.0;
    m_squeezeLimitLow = 0.9;
    m_squeezeLimitHigh = 1.1;
}

void CReferencePropertiesDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);

    DDX_Text(pDX, IDC_EDIT_SPECIE, m_specieName);
    DDX_Text(pDX, IDC_EDIT_PATH, m_referencePath);

    DDX_Radio(pDX, IDC_RADIO_SHIFT_FREE, m_shiftOption);
    DDX_Radio(pDX, IDC_RADIO_SQUEEZE_FREE, m_squeezeOption);

    DDX_Text(pDX, IDC_EDIT_SHIFT_FIX, m_shiftFixValue);
    DDX_Text(pDX, IDC_EDIT_SHIFT_LINK, m_shiftLinkValue);
    DDX_Text(pDX, IDC_EDIT_SHIFT_LOWLIMIT, m_shiftLimitLow);
    DDX_Text(pDX, IDC_EDIT_SHIFT_HIGHLIMIT, m_shiftLimitHigh);

    DDX_Text(pDX, IDC_EDIT_SQUEEZE_FIX, m_squeezeFixValue);
    DDX_Text(pDX, IDC_EDIT_SQUEEZE_LINK, m_squeezeLinkValue);
    DDX_Text(pDX, IDC_EDIT_SQUEEZE_LOWLIMIT, m_squeezeLimitLow);
    DDX_Text(pDX, IDC_EDIT_SQUEEZE_HIGHLIMIT, m_squeezeLimitHigh);

    // The controls
    DDX_Control(pDX, IDC_EDIT_SHIFT_LINK, m_editShiftLink);
    DDX_Control(pDX, IDC_EDIT_SQUEEZE_LINK, m_editSqueezeLink);
    DDX_Control(pDX, IDC_EDIT_SHIFT_FIX, m_editShiftFix);
    DDX_Control(pDX, IDC_EDIT_SQUEEZE_FIX, m_editSqueezeFix);
    DDX_Control(pDX, IDC_EDIT_SHIFT_LOWLIMIT, m_editShiftLowLimit);
    DDX_Control(pDX, IDC_EDIT_SHIFT_HIGHLIMIT, m_editShiftHighLimit);
    DDX_Control(pDX, IDC_EDIT_SQUEEZE_LOWLIMIT, m_editSqueezeLowLimit);
    DDX_Control(pDX, IDC_EDIT_SQUEEZE_HIGHLIMIT, m_editSqueezeHighLimit);
}


BEGIN_MESSAGE_MAP(CReferencePropertiesDlg, CDialog)
    ON_BN_CLICKED(IDC_BUTTON_BROWSE_REFERENCE, BrowseForReference)
END_MESSAGE_MAP()


// CReferencePropertiesDlg message handlers


/** Saves the contents of the dialog */
void CReferencePropertiesDlg::SaveData()
{
    UpdateData(TRUE); // <-- Save the data in the dialog	

    // Save the strings (converting CString to std::string)
    m_ref.m_specieName = std::string((LPCSTR)m_specieName);
    m_ref.m_path = std::string((LPCSTR)m_referencePath);

    // save the shift option
    switch (m_shiftOption)
    {
    case 0:
        m_ref.m_shiftOption = SHIFT_TYPE::SHIFT_FREE;
        m_ref.m_shiftValue = 0.0;
        break;
    case 1:
        m_ref.m_shiftOption = SHIFT_TYPE::SHIFT_FIX;
        m_ref.m_shiftValue = m_shiftFixValue;
        break;
    case 2:
        m_ref.m_shiftOption = SHIFT_TYPE::SHIFT_LINK;
        m_ref.m_shiftValue = m_shiftLinkValue;
        break;
    case 3:
        m_ref.m_shiftOption = SHIFT_TYPE::SHIFT_LIMIT;
        m_ref.m_shiftMaxValue = m_shiftLimitHigh;
        m_ref.m_shiftValue = m_shiftLimitLow;
        break;
    }

    // save the squeeze option
    switch (m_squeezeOption)
    {
    case 0:
        m_ref.m_squeezeOption = SHIFT_TYPE::SHIFT_FREE;
        m_ref.m_squeezeValue = 0.0;
        break;
    case 1:
        m_ref.m_squeezeOption = SHIFT_TYPE::SHIFT_FIX;
        m_ref.m_squeezeValue = m_squeezeFixValue;
        break;
    case 2:
        m_ref.m_squeezeOption = SHIFT_TYPE::SHIFT_LINK;
        m_ref.m_squeezeValue = m_squeezeLinkValue;
        break;
    case 3:
        m_ref.m_squeezeOption = SHIFT_TYPE::SHIFT_LIMIT;
        m_ref.m_squeezeValue = m_squeezeLimitLow;
        m_ref.m_squeezeMaxValue = m_squeezeLimitHigh;
        break;
    }
}

void CReferencePropertiesDlg::UpdateDlg()
{
    // Get the strings, converting between std::string and CString
    m_specieName = CString(m_ref.m_specieName.c_str());
    m_referencePath = CString(m_ref.m_path.c_str());

    // save the shift option
    switch (m_ref.m_shiftOption)
    {
    case 0: SHIFT_TYPE::SHIFT_FREE; m_shiftOption = 0; break;
    case 1: SHIFT_TYPE::SHIFT_FIX;	m_shiftOption = 1;
        m_shiftFixValue = m_ref.m_shiftValue;	break;
    case 2: SHIFT_TYPE::SHIFT_LINK; m_shiftOption = 2;
        m_shiftLinkValue = m_ref.m_shiftValue; break;
    case 3: SHIFT_TYPE::SHIFT_LIMIT; m_shiftOption = 3;
        m_shiftLimitHigh = m_ref.m_shiftMaxValue;
        m_shiftLimitLow = m_ref.m_shiftValue;	break;
    }

    // save the squeeze option
    switch (m_ref.m_squeezeOption)
    {
    case 0: SHIFT_TYPE::SHIFT_FREE; m_squeezeOption = 0; break;
    case 1: SHIFT_TYPE::SHIFT_FIX;  m_squeezeOption = 1; break;
        m_squeezeFixValue = m_ref.m_squeezeValue;	break;
    case 2: SHIFT_TYPE::SHIFT_LINK; m_squeezeOption = 2; break;
        m_squeezeLinkValue = m_ref.m_squeezeValue; break;
    case 3: SHIFT_TYPE::SHIFT_LIMIT; m_squeezeOption = 3;
        m_squeezeLimitHigh = m_ref.m_squeezeMaxValue;
        m_squeezeLimitLow = m_ref.m_squeezeValue;	break;
    }

    UpdateData(FALSE); // <-- Update the data in the dialog	
}
void Dialogs::CReferencePropertiesDlg::OnOK()
{

    // Save the data in the dialog
    SaveData();

    CDialog::OnOK();
}

void CReferencePropertiesDlg::BrowseForReference()
{

    // 1. Let the user browse for the reference file
    CString fileName = "";
    if (!Common::BrowseForReferenceFile(fileName))
    {
        return;
    }

    // 2. Set the path
    m_ref.m_path = std::string((LPCSTR)fileName);

    // 3. make a guess of the specie name
    CString specie;
    Common::GuessSpecieName(fileName, specie);
    if (strlen(specie) != 0)
    {
        m_ref.m_specieName = std::string((LPCSTR)specie);
    }

    UpdateData(FALSE);
}
BOOL Dialogs::CReferencePropertiesDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    // Initialize the tooltips
    InitToolTips();

    // update the controls in the dialog
    UpdateDlg();

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void Dialogs::CReferencePropertiesDlg::InitToolTips()
{
    // Don't initialize the tool tips twice
    if (m_toolTip.m_hWnd != nullptr)
        return;

    // Enable the tool tips
    if (!m_toolTip.Create(this))
    {
        TRACE0("Failed to create tooltip control\n");
    }

    m_toolTip.AddTool(&m_editShiftLink, "Which reference-file to link to\r\n0<=>Reference-file #1\r\n1<=>Reference-file #2\r\n...");
    m_toolTip.AddTool(&m_editSqueezeLink, "Which reference-file to link to\r\n0<=>Reference-file #1\r\n1<=>Reference-file #2\r\n...");
    m_toolTip.AddTool(&m_editShiftFix, "Number of pixels to shift this reference file");
    m_toolTip.AddTool(&m_editSqueezeFix, "Number of pixels to squeeze this reference file");
    m_toolTip.AddTool(&m_editShiftLowLimit, "Least number of pixels to shift this reference");
    m_toolTip.AddTool(&m_editShiftHighLimit, "Highest number of pixels to shift this reference");
    m_toolTip.AddTool(&m_editSqueezeLowLimit, "Least number of pixels to squeeze this reference");
    m_toolTip.AddTool(&m_editSqueezeHighLimit, "Highest number of pixels to squeeze this reference");

    m_toolTip.SetMaxTipWidth(SHRT_MAX);
    m_toolTip.Activate(TRUE);
}

BOOL Dialogs::CReferencePropertiesDlg::PreTranslateMessage(MSG* pMsg)
{
    m_toolTip.RelayEvent(pMsg);

    return CDialog::PreTranslateMessage(pMsg);
}