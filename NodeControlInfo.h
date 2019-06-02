#pragma once
#include <afxtempl.h>

namespace Communication
{
    // These are the types of modes which the device can be in
    enum class DeviceMode
    {
        Sleep = 0,
        Run = 1,
        Pause = 2,
        Off = 3,
        Special = 4
    };

    class CNodeControlInfo
    {
        class CControlInfo
        {
        public:
            int m_mainIndex;
            DeviceMode m_mode;
            CString m_spectrometerID;
            CString m_cfgFilePath;

            CControlInfo(int mainIndex, DeviceMode mode, CString spectrometerID, CString cfgFilePath);
        };
    public:
        //-------------------------public functions---------------------//
        //--------------------------------------------------------------//
        //--------------------------------------------------------------//
        CNodeControlInfo(void);
        ~CNodeControlInfo(void);

        /**get the total number of nodes
            *return total number of nodes */
        int GetNodeSum();

        /** Gets the status of the given node, run, off, sleep, run in special mode
            @param mainIndex - the index in the array , also in the configuration file
            @return the status */
        DeviceMode GetNodeStatus(int mainIndex);

        void GetNodeCfgFilePath(int mainIndex, CString& filePath);

        /**set node status - run , off, sleep, run in special mode
            *@param mainIndex - the index in the array , also in the configuration file
            *@param status		- the node operating status
            *@param filePath	- the file to be uploaded */
        void SetNodeStatus(int mainIndex, DeviceMode status, CString& filePath);

        /** Fill node info into the private array*/
        void FillinNodeInfo(int mainIndex, DeviceMode mode, const CString spectrometerID, CString cfgFilePath);

        int	 GetMainIndex(CString spectrometerID);

    private:

        CArray<CControlInfo*, CControlInfo*> m_controlInfoArray;

        /** Total number of nodes */
        int m_totalNodeNum;
    };
}
