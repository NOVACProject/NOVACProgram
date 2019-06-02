#include "StdAfx.h"
#include "NodeControlInfo.h"
#include "Common/Common.h"

//--------------------------------class CControlInfo------------------//

namespace Communication
{
    CNodeControlInfo::CControlInfo::CControlInfo(int mainIndex, DeviceMode mode, CString spectrometerID, CString cfgFilePath)
    {
        m_mainIndex = mainIndex;
        m_mode = mode;
        m_spectrometerID.Format("%s", (LPCTSTR)spectrometerID);
        m_cfgFilePath.Format("%s", (LPCTSTR)cfgFilePath);
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

    void CNodeControlInfo::FillinNodeInfo(int mainIndex, DeviceMode mode, const CString spectrometerID, CString cfgFilePath)
    {
        m_controlInfoArray.SetAtGrow(mainIndex, new CControlInfo(mainIndex, mode, spectrometerID, cfgFilePath));

    }

    int CNodeControlInfo::GetNodeSum()
    {
        return (int)m_controlInfoArray.GetCount();
    }

    DeviceMode CNodeControlInfo::GetNodeStatus(int mainIndex)
    {
        if (mainIndex < 0 || mainIndex >= m_controlInfoArray.GetSize())
        {
            return DeviceMode::Run;
        }
        else
        {
            CControlInfo* controlInfo = m_controlInfoArray.GetAt(mainIndex);
            return controlInfo->m_mode;
        }
    }

    //set node operating status
    void CNodeControlInfo::SetNodeStatus(int mainIndex, DeviceMode status, CString& filePath)
    {
        // Error Check!
        if (mainIndex < 0 || mainIndex >= m_controlInfoArray.GetCount())
            return; // <-- added 2007.09.11
        CControlInfo* controlInfo = m_controlInfoArray.GetAt(mainIndex);
        controlInfo->m_mode = status;
        controlInfo->m_cfgFilePath = filePath;
        m_controlInfoArray.SetAt(mainIndex, controlInfo);
    }
    void CNodeControlInfo::GetNodeCfgFilePath(int mainIndex, CString& filePath)
    {
        if (mainIndex < 0 || mainIndex >= m_controlInfoArray.GetSize()) {
            filePath.Format("");
            return;
        }
        else {
            CControlInfo* controlInfo = m_controlInfoArray.GetAt(mainIndex);
            filePath = controlInfo->m_cfgFilePath;
        }
    }
    //get main index by spectrometer id
    int CNodeControlInfo::GetMainIndex(CString spectrometerID)
    {
        int i;
        CControlInfo* controlInfo;
        for (i = 0; i < m_controlInfoArray.GetCount(); i++)
        {
            controlInfo = m_controlInfoArray.GetAt(i);
            if (Equals(controlInfo->m_spectrometerID, spectrometerID))
                return i;
        }
        return -1;
    }

}
