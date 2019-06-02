#include "StdAfx.h"
#include "NodeControlInfo.h"
#include "../Common/Common.h"

namespace Communication
{
    CNodeControlInfo::CControlInfo::CControlInfo(int mainIndex, DeviceMode mode, const CString& spectrometerID, const CString& localFilePath)
        : m_mainIndex(mainIndex),
            m_mode(mode),
            m_spectrometerID(spectrometerID),
            m_localPathToFileToUpload(localFilePath)
    {
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

    void CNodeControlInfo::FillinNodeInfo(int mainIndex, DeviceMode mode, const CString spectrometerID)
    {
        m_controlInfoArray.SetAtGrow(mainIndex, new CControlInfo(mainIndex, mode, spectrometerID, ""));
    }

    int CNodeControlInfo::GetNodeSum()
    {
        return (int)m_controlInfoArray.GetCount();
    }

    DeviceMode CNodeControlInfo::GetNodeStatus(int mainIndex) const
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

    void CNodeControlInfo::SetNodeStatus(int mainIndex, DeviceMode status)
    {
        if (mainIndex < 0 || mainIndex >= m_controlInfoArray.GetCount())
        {
            return; // <-- added 2007.09.11
        }

        CControlInfo* controlInfo = m_controlInfoArray.GetAt(mainIndex);
        controlInfo->m_mode = status;
        controlInfo->m_localPathToFileToUpload = "";
        m_controlInfoArray.SetAt(mainIndex, controlInfo);
    }

    void CNodeControlInfo::SetNodeToUploadFile(int mainIndex, CString& localFilePath)
    {
        if (mainIndex < 0 || mainIndex >= m_controlInfoArray.GetCount())
        {
            return; // <-- added 2007.09.11
        }

        CControlInfo* controlInfo = m_controlInfoArray.GetAt(mainIndex);
        controlInfo->m_mode = DeviceMode::FileUpload;
        controlInfo->m_localPathToFileToUpload = localFilePath;
        m_controlInfoArray.SetAt(mainIndex, controlInfo);
    }


    void CNodeControlInfo::GetNodeCfgFilePath(int mainIndex, CString& filePath) const
    {
        if (mainIndex < 0 || mainIndex >= m_controlInfoArray.GetSize()) {
            filePath.Format("");
            return;
        }
        else
        {
            CControlInfo* controlInfo = m_controlInfoArray.GetAt(mainIndex);
            filePath = controlInfo->m_localPathToFileToUpload;
        }
    }

    int CNodeControlInfo::GetMainIndex(CString spectrometerID) const
    {
        for (int i = 0; i < m_controlInfoArray.GetCount(); i++)
        {
            CControlInfo* controlInfo = m_controlInfoArray.GetAt(i);
            if (Equals(controlInfo->m_spectrometerID, spectrometerID))
            {
                return i;
            }
        }
        return -1;
    }
}
