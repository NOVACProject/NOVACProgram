#include "stdafx.h"
#include "../NovacMasterProgram.h"
#include "AdvancedFTPUploadSettings.h"
#include "Configuration.h"
#include <SpectralEvaluation/DateTime.h>

using namespace ConfigurationDialog;

// CAdvancedFTPUploadSettings dialog

IMPLEMENT_DYNAMIC(CAdvancedFTPUploadSettings, CDialog)
CAdvancedFTPUploadSettings::CAdvancedFTPUploadSettings(CWnd* pParent /*=NULL*/)
    : CDialog(CAdvancedFTPUploadSettings::IDD, pParent), m_configuration(nullptr)
{
}

CAdvancedFTPUploadSettings::~CAdvancedFTPUploadSettings()
{
    m_configuration = nullptr;
}

void CAdvancedFTPUploadSettings::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_START_HR, m_startHr);
    DDX_Text(pDX, IDC_EDIT_START_MIN, m_startMin);
    DDX_Text(pDX, IDC_EDIT_START_SEC, m_startSec);
    DDX_Text(pDX, IDC_EDIT_STOP_HR, m_stopHr);
    DDX_Text(pDX, IDC_EDIT_STOP_MIN, m_stopMin);
    DDX_Text(pDX, IDC_EDIT_STOP_SEC, m_stopSec);
}


BEGIN_MESSAGE_MAP(CAdvancedFTPUploadSettings, CDialog)
END_MESSAGE_MAP()


// CAdvancedFTPUploadSettings message handlers

BOOL CAdvancedFTPUploadSettings::OnInitDialog() {

    // initialize the values for the start-time
    novac::SplitToHourMinuteSecond(m_configuration->ftpSetting.ftpStartTime, m_startHr, m_startMin, m_startSec);

    // initialize the values for the stop-time
    novac::SplitToHourMinuteSecond(m_configuration->ftpSetting.ftpStopTime, m_stopHr, m_stopMin, m_stopSec);

    // fill in the data in the form
    UpdateData(FALSE);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}


void CAdvancedFTPUploadSettings::GetNecessaryComponentSize(CWnd *wnd, CString &str, int &width, int &height) {
    TEXTMETRIC   tm;
    CDC *dc = wnd->GetDC();
    CFont *oldFont = dc->SelectObject(wnd->GetFont());
    dc->GetTextMetrics(&tm);

    CSize sz = dc->GetTextExtent(str);

    // Add the avg width to prevent clipping
    sz.cx += tm.tmAveCharWidth;

    // Select the old font back into the DC
    dc->SelectObject(oldFont);
    wnd->ReleaseDC(dc);

    width = sz.cx;
    height = sz.cy;
}

void CAdvancedFTPUploadSettings::OnOK()
{
    if (UpdateData(TRUE)) { // <-- first save the data in the dialog
        m_configuration->ftpSetting.ftpStartTime = this->m_startHr * 3600 + this->m_startMin * 60 + this->m_startSec;
        m_configuration->ftpSetting.ftpStopTime = this->m_stopHr * 3600 + this->m_stopMin * 60 + this->m_stopSec;
    }

    CDialog::OnOK();
}