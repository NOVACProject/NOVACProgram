#include "StdAfx.h"
#include "SystemConfigurationPage.h"

using namespace ConfigurationDialog;

IMPLEMENT_DYNAMIC(CSystemConfigurationPage, CPropertyPage)

CSystemConfigurationPage::CSystemConfigurationPage(void)
{
    CPropertyPage();
    m_configuration = nullptr;
    m_curScanner = nullptr;
    m_curSpec = nullptr;
    m_scannerTree = nullptr;
}

CSystemConfigurationPage::CSystemConfigurationPage(UINT nIDTemplate) : CPropertyPage(nIDTemplate)
{
    m_configuration = nullptr;
    m_curScanner = nullptr;
    m_curSpec = nullptr;
    m_scannerTree = nullptr;
}

CSystemConfigurationPage::~CSystemConfigurationPage(void)
{
}

void CSystemConfigurationPage::OnChangeScanner() {
    int curScan, curSpec;
    if (m_configuration == nullptr)
        return;

    GetScanAndSpec(curScan, curSpec);
    if (curScan == -1 || curSpec == -1) {
        return;
    }

    m_curScanner = nullptr;
    m_curSpec = nullptr;

    // Set the scanner
    m_curScanner = &m_configuration->scanner[curScan];

    // Set the spectrometer
    if (m_curScanner != nullptr) {
        m_curSpec = &m_curScanner->spec[curSpec];
    }
}

void CSystemConfigurationPage::GetScanAndSpec(int& curScanner, int& curSpec) {
    curSpec = 0;
    curScanner = 0;
    if (m_scannerTree == nullptr) {
        curSpec = curScanner = -1;
        return;
    }

    return m_scannerTree->GetCurScanAndSpec(curScanner, curSpec);
}