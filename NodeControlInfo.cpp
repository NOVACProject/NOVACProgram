#include "StdAfx.h"
#include "nodecontrolinfo.h"
#include "Common/Common.h"

//--------------------------------class CControlInfo------------------//

CNodeControlInfo::CControlInfo::CControlInfo(void)
{}
CNodeControlInfo::CControlInfo::~CControlInfo(void)
{}
CNodeControlInfo::CControlInfo::CControlInfo(int mainIndex,int mode,CString spectrometerID,CString cfgFilePath)
{
	m_mainIndex				= mainIndex;
	m_mode						= mode;
	m_spectrometerID.Format("%s", spectrometerID);
	m_cfgFilePath.Format("%s", cfgFilePath);
}
//---------------------------class CNodeControlInfo------------------//
CNodeControlInfo::CNodeControlInfo(void)
{
	m_controlInfoArray.SetSize(1);
}

CNodeControlInfo::~CNodeControlInfo(void)
{
	m_controlInfoArray.RemoveAll();
}
//fill the nodes' information
void CNodeControlInfo::FillinNodeInfo(int mainIndex,int mode, const CString spectrometerID,CString cfgFilePath)
{
	m_controlInfoArray.SetAtGrow(mainIndex,
		new CControlInfo(mainIndex, mode, spectrometerID, cfgFilePath));

}
//return the sum of nodes
int CNodeControlInfo::GetNodeSum()
{
	return (int)m_controlInfoArray.GetCount();
}
//get node operating status
int CNodeControlInfo::GetNodeStatus(int mainIndex)
{
	int status = RUN_MODE;
	if(mainIndex < 0 || mainIndex >= m_controlInfoArray.GetSize()){
		return status;
	}else{
		CControlInfo* controlInfo = m_controlInfoArray.GetAt(mainIndex);
		status = controlInfo->m_mode;
		return status;
	}
}
//set node operating status
void CNodeControlInfo::SetNodeStatus(int mainIndex,int status, CString& filePath)
{
	// Error Check!
	if(mainIndex < 0 || mainIndex >= m_controlInfoArray.GetCount())
		return; // <-- added 2007.09.11
	CControlInfo* controlInfo = m_controlInfoArray.GetAt(mainIndex);
	controlInfo->m_mode = status;
	controlInfo->m_cfgFilePath = filePath;
	m_controlInfoArray.SetAt(mainIndex,controlInfo);
}
void CNodeControlInfo::GetNodeCfgFilePath(int mainIndex,CString& filePath)
{
	if(mainIndex < 0 || mainIndex >= m_controlInfoArray.GetSize()){
		filePath.Format("");
		return;
	}else{
		CControlInfo* controlInfo = m_controlInfoArray.GetAt(mainIndex);
		filePath = controlInfo->m_cfgFilePath;
	}
}
//get main index by spectrometer id
int CNodeControlInfo::GetMainIndex(CString spectrometerID)
{
	int i;
	CControlInfo* controlInfo;
	for(i=0; i< m_controlInfoArray.GetCount(); i++)
	{
		controlInfo = m_controlInfoArray.GetAt(i);
		if(Equals(controlInfo->m_spectrometerID, spectrometerID))
			return i;
	}
	return -1;
}
