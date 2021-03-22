#pragma once
#include "afxwin.h"
#include <memory>

namespace Communication
{
#define SERIAL_CONNECTION 1
#define FTP_CONNECTION 2
#define HTTP_CONNECTION 3
#define DIRECTORY_POLLING 4

    class CFTPHandler;
    class CSerialControllerWithTx;
    class CNodeControlInfo;

    class CCommunicationController
        :public CWinThread
    {

    private:
        struct CSerialInfo
        {
        public:
            /** index in the total setting list */
            int m_mainIndex;

            /** sleep status, true - sleeping */
            bool m_sleepFlag;

            /**medium type*/
            int m_medium;

            CSerialInfo(int index, bool sleep, int medium)
                : m_mainIndex(index), m_sleepFlag(sleep), m_medium(medium)
                {}
        };

    public:
        CCommunicationController(void);
        ~CCommunicationController(void);

        DECLARE_DYNCREATE(CCommunicationController);
        DECLARE_MESSAGE_MAP()


        //---------------------------------------------//
        //-------------public functions----------------//
        //---------------------------------------------//
        /** Called when the thred is starting */
        virtual BOOL InitInstance();

        /** Called when the thread is to be stopped */
        afx_msg void OnQuit(WPARAM wp, LPARAM lp);

        /** Called when a special cfgOnce file will be uploaded
            @param wp is a pointer to a CString object telling the
                spectrometer ID
            @param lp - full path of the file to be uploaded. */
        afx_msg void OnUploadCfgOnce(WPARAM wp, LPARAM lp);

        /**start communication threads*/
        void StartCommunicationThreads();

    public:

        /**set parameters for all the serial connections*/
        void SetupSerialConnections();

        void SleepAllNodes(int connectionType);

        /**start ftp connections for polling scanners*/
        void StartFTP();

        /** start directory polling */
        void StartDirectoryPolling();

        //---------------------------------------------//
        //-------------public variables----------------//
        //---------------------------------------------//

        // store the indexes of serial connection in configuration.xml
        CArray<CSerialInfo*, CSerialInfo*> m_serialList;

        /** The array of serial-controllers */
        CArray<CSerialControllerWithTx*, CSerialControllerWithTx*> m_SerialControllerTx;

        /** The array of ftp-handlers */
        CArray<CFTPHandler*, CFTPHandler*> m_FTPHandler;

        /** store node control informations */
        std::unique_ptr<CNodeControlInfo> m_nodeControl;

        //the sum of the serial connections
        int m_totalSerialConnection;

    private:
        // store the indexes of ftp connection in configuration.xml
        CList<int, int> m_ftpList;

        //the sum of the ftp connections
        int m_totalFTPConnection;

        // store the indexes of http connection in configuration.xml
        CList<int, int> m_httpList;

        //the sum of the http connections
        int m_totalHttpConnection;

        // store the indexes of directory polling connection in configuration.xml
        CList<int, int> m_dirPolList;

        // the sum of the directory polling connections
        int m_totalDirectoryPolling;
      

    };
}