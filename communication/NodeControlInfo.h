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
        FileUpload = 4
    };

    class CNodeControlInfo
    {
    private:
        struct CControlInfo
        {
        public:
            /** Index of the device in the main configuration */
            int m_mainIndex;

            /** The current mode of the device */
            DeviceMode m_mode;

            /** The ID of the spectrometer */
            CString m_spectrometerID;

            /** The local path to a file which should be uploaded. Set when m_mode == DeviceMode::FileUpload. */
            CString m_localPathToFileToUpload;

            CControlInfo(int mainIndex, DeviceMode mode, const CString& spectrometerID, const CString& localFilePath);
        };

    public:
        //--------------------------------------------------------------//
        //-------------------------public functions---------------------//
        //--------------------------------------------------------------//
        CNodeControlInfo(void);
        ~CNodeControlInfo(void);

        /**get the total number of nodes
            *return total number of nodes */
        int GetNodeSum();

        /** Gets the status of the given node, run, off, sleep, run in special mode
            @param mainIndex - the index in the array , also in the configuration file
            @return the status */
        DeviceMode GetNodeStatus(int mainIndex) const;

        /** Retrieves the location to which cfgonce.txt and cfg.txt should be uploaded */
        void GetNodeCfgFilePath(int mainIndex, CString& filePath) const;

        /**set node status - run , off, sleep, run in special mode
            *@param mainIndex - the index in the array , also in the configuration file
            *@param status		- the node operating status */
        void SetNodeStatus(int mainIndex, DeviceMode status);

        /** Set the device to upload a file */
        void SetNodeToUploadFile(int mainIndex, CString& localFilePath);

        /** Fill node info into the private array*/
        void FillinNodeInfo(int mainIndex, DeviceMode mode, const CString spectrometerID);

        /** Retrieves the index of the spectrometer with the given ID in the configuration. */
        int GetMainIndex(CString spectrometerID) const;

    private:

        CArray<CControlInfo*, CControlInfo*> m_controlInfoArray;

        /** Total number of nodes */
        int m_totalNodeNum;
    };
}
