#pragma once

#include "../Common/Common.h"
#include "../DlgControls/MeasGrid.h"
#include "SystemConfigurationPage.h"
#include "../Communication/SerialControllerWithTx.h"
#include "afxcmn.h"
#include "afxwin.h"

using namespace Communication;

namespace ConfigurationDialog
{

	class CRemoteConfigurationDlg : public CSystemConfigurationPage
	{
		DECLARE_DYNAMIC(CRemoteConfigurationDlg)

	public:
		CRemoteConfigurationDlg();
		virtual ~CRemoteConfigurationDlg();

	// Dialog Data
		enum { IDD = IDD_CONFIGURATION_REMOTE };


	protected:
		virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

		DECLARE_MESSAGE_MAP()

	public:
		/** Initializing the dialog */
		virtual BOOL OnInitDialog();

		/** Handling the tool tips */
		virtual BOOL PreTranslateMessage(MSG* pMsg); 

		/** Called when the user has changed the currently selected scanner */
		void OnChangeScanner();

		/** The 'upload configuration file' button */
		CButton	m_uploadConfiguration;

		/** The 'download configuration file' button */
		CButton	m_downConfiguration;

		// ------- LOCAL DATA STORAGE --------------

		/** The ip-address of the ftp-server */
		CIPAddressCtrl m_ftpServerIp;

		/** The user name of the ftp-server */
		CEdit m_ftpServerUserName;

		/** The password  of the ftp-server */
		CEdit m_ftpServerPassword;

		/** The first channel to transfer */
		CEdit m_channelStart;

		/** The last channel to transfer */
		CEdit m_channelStop;

		/** Should the spectra be evaluate one at a time or one scan at a time? */
		CButton m_realTime;

		/** The number of steps in one round of the motor */
		CEdit m_motorStepsPerRound;

		/** The position of the motor switch */
		CEdit m_motorStepComp;

		/** The delay between each motor step (in ms) */
		CEdit m_motorDelay;

		/** The maximum exposure time */
		CEdit m_exposureTimeMax;

		/** The exposure level of the spectra */
		CEdit m_exposureTimePercent;

		/** The measurement grid */
		CMeasGrid m_measGrid;

		/** The frame for the measurement */
		CStatic m_measurementFrame;

		/**object of serial controller*/
		CSerialControllerWithTx *m_SerialController;

		/**object of serial controller*/
		CSerialControllerWithTx *m_SerialControllerWithTx;

		/**configuration file path*/
		CString m_cfgPath;

		/** The full filename and path of the (local) configuration file */
		CString m_cfgFile;

		/**cfg log path**/
		CString m_cfgLog;

		/** Uploading the configuration file */
		afx_msg void OnUploadRemoteCfg();

		/** Downloading the configuration file */
		afx_msg void OnDownloadRemoteCfg();

		/** Inserting one row into the measurement table */
		afx_msg void OnInsertMeasurement();

		/** Deleting one row into the measurement table*/
		afx_msg void OnDeleteMeasurement();

		/** the datastructrue to store measurements */
		class CMeasurement{
		public:
		long  pos;            // the motor position
		long  inttime;        // the integration time
		long  intsum;         // the sum in spectrometer
		long  extsum;         // the sum in computer
		short chn;            // the channel
		char  name[30];       // basename
		int   rep;            // the repetitions
		unsigned char flag;   // ???
		};

		typedef struct RemoteConfiguration{
			// The measurement positions
			CMeasurement    m_meas[MAX_SPEC_PER_SCAN];
			int             m_measNum;
			// The spectrum transfer
			int  m_startChannel;
			int  m_stopChannel;
			int  m_realTime;
			// The settings for exposure time
			double  m_maxExptime;
			double  m_percent;
			// The motor
			int  m_delay;
			int  m_stepsPerRound;
			int  m_motorStepComp;
			int  m_skipmotor;
			// The ftp
			int             m_ftpTimeOut;
			BYTE            m_ftpIP[4];
			CString         m_ftpUserName;
			CString         m_ftpPassword;
			// The setup geometry
			double  m_compass;
			double  m_roll;
			double  m_pitch;
			double  m_temp;
			// debug-level
			int             m_debugflag;

		}RemoteConfiguration;

		RemoteConfiguration		m_remoteConf;

		/** The index of the currently selected scanner */
		int m_currentScanner;

		/** Reads a cfg.txt - file */
		int ReadCfg(const CString &fileName);

		/**Write cfg.txt file to upload*/
		int WriteCfg(const CString &fileName, bool append = false);

	private:

		//  ------------------- PRIVATE DATA -----------------


		//  ------------------- PRIVATE FUNCTIONS -----------------
	
		/** get com port  setting*/
		int GetComSetting();

		/** Initialize the measurement table */
		void InitMeasGrid();

		/** Fill in the items in the measurement table */
		void PopulateMeasGrid();

		/** Save the items in the measurement table */
		void SaveMeasGrid();

		/** write a line in one file*/
		void WriteALineInFile(CString filename, CString line);
	};
}