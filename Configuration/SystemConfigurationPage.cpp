#include "StdAfx.h"
#include "SystemConfigurationPage.h"

using namespace ConfigurationDialog;

IMPLEMENT_DYNAMIC(CSystemConfigurationPage, CPropertyPage)

CSystemConfigurationPage::CSystemConfigurationPage(void)
{
	CPropertyPage();
	m_configuration = NULL;
	m_curScanner = NULL;
	m_curSpec = NULL;
	m_scannerTree = NULL;
}

CSystemConfigurationPage::CSystemConfigurationPage(UINT nIDTemplate) : CPropertyPage(nIDTemplate)
{
	m_configuration = NULL;
	m_curScanner = NULL;
	m_curSpec = NULL;
	m_scannerTree = NULL;
}

CSystemConfigurationPage::~CSystemConfigurationPage(void)
{
}

void CSystemConfigurationPage::OnChangeScanner(){
	int curScan, curSpec;
	if(m_configuration == NULL)
		return;

	GetScanAndSpec(curScan, curSpec);

	m_curScanner = NULL;
	m_curSpec = NULL;

	// Set the scanner
	if(curScan == -1){
		if(m_configuration->scannerNum > 0)
			m_curScanner = &m_configuration->scanner[0];
	}else{
		m_curScanner = &m_configuration->scanner[curScan];
	}

	// Set the spectrometer
	if(m_curScanner != NULL){
		if(curSpec == -1){
			if(m_curScanner->specNum > 0)
				m_curSpec = &m_curScanner->spec[0];
		}else{
			m_curSpec = &m_curScanner->spec[curSpec];
		}
	}
}

void CSystemConfigurationPage::GetScanAndSpec(int &curScanner, int &curSpec){
	curSpec		= 0;
	curScanner	= 0;
	if(m_scannerTree == NULL){
		curSpec = curScanner = -1;
		return;
	}

	return m_scannerTree->GetCurScanAndSpec(curScanner, curSpec);
}