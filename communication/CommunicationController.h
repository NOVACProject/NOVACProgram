#pragma once
#include "afxwin.h"
#include "../resource.h"
#include "../Common/common.h"
#include "FTPHandler.h"
#include "SerialControllerWithTx.h"

#include "../NodeControlInfo.h"

UINT ConnectBySerialWithTX(LPVOID pParam);
UINT ConnectBySerialTXZM(LPVOID pParam);
UINT ConnectByFTP(LPVOID pParam);
void Pause(double startTime, double stopTime, int serialID);

namespace Communication
{
#define SERIAL_CONNECTION 1
#define FTP_CONNECTION 2
#define HTTP_CONNECTION 3

    class CCommunicationController
        :public CWinThread
    {
        class CFTPInfo
        {
        public:
            CFTPInfo();
            ~CFTPInfo();
            CFTPInfo(CString ftpIP, CString userName, CString password);
            //-- variables ---//
            CString m_ftpIP;
            CString m_userName;
            CString m_password;
        };
        class CSerialInfo
        {
        public:
            /**index in the total setting list*/
            int m_mainIndex;
            /**sleep status, true - sleeping*/
            bool m_sleepFlag;
            /**medium type*/
            int m_medium;
            CSerialInfo(void);
            ~CSerialInfo(void);
            CSerialInfo(int index, bool sleep, int medium);
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
        void SetSerialConnections();
        void Pause(double startTime, double stopTime, int serialID);

        void SleepAllNodes(int connectionType);

        /**start ftp connections for polling scanners*/
        void StartFTP();

        //---------------------------------------------//
        //-------------public variables----------------//
        //---------------------------------------------//

        // store the indexes of serial connection in configuration.xml
        CArray<CSerialInfo*, CSerialInfo*> m_serialList;

        // store the indexes of ftp connection in configuration.xml
        CList<int, int> m_ftpList;

        // store the indexes of http connection in configuration.xml
        CList<int, int> m_httpList;

        //the sum of the serial connections
        int m_totalSerialConnection;

        //the sum of the ftp connections
        int m_totalFTPConnection;

        //the sum of the http connections
        int m_totalHttpConnection;

        /** The array of serial-controllers */
        CArray<CSerialControllerWithTx*, CSerialControllerWithTx*> m_SerialControllerTx;

        /** The array of ftp-handlers */
        CArray<CFTPHandler*, CFTPHandler*> m_FTPHandler;

        /** store node control informations */
        CNodeControlInfo* m_nodeControl;
    };
}