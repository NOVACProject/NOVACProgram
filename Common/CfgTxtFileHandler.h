#pragma once
#include <Afxtempl.h>
#include "Common.h"

/** A <b>CCfgTxtFileHandler</b> is an object to read and write the simple
    cfg.txt - files found in the Novac-instruments. */

namespace FileHandler
{
    class CCfgTxtFileHandler
    {
    public:
        CCfgTxtFileHandler(void);
        ~CCfgTxtFileHandler(void);

        // ----------------------------------------------------------------------
        // ---------------------- PUBLIC DATA -----------------------------------
        // ----------------------------------------------------------------------

        class CMeasLine {
        public:
            int pos[2];
            int expTime;
            int sum1;
            int sum2;
            int chn;
            char baseName[12];
            int repetitions;
            int flag;
            CMeasLine();
            ~CMeasLine();
        };

        /** The settings for the pixels to use */
        int m_startChn;
        int m_stopChn;

        /** The SZA for when to start doing stratospheric measurements */
        int m_stratoAngle;

        /** The settings for the motor(s) */
        int m_motorStepsComp[2];
        int m_motorStepsPerRound[2];
        int m_motorDelay;
        int m_motorSkip;
        int m_nMotors; // the number of motors...

        /** The power settings */
        double m_batteryLimit;
        int m_powerSave;

        /** The settings for the exposure-time */
        double m_percent;
        int m_maxIntTime;
        int m_minIntTime;

        /** The settings for the FTP-server to use */
        int m_ftpServerIp[4];
        int m_ftpServerTimeOut;
        CString m_ftpServerUserName;
        CString m_ftpServerPassword;

        /** The cone-angle */
        double m_coneAngle;

        /** The setup */
        double m_compass;
        double m_tiltX;
        double m_tiltY;

        /** The measurement-lines */
        CArray<CMeasLine, CMeasLine&> m_measurements;
        int m_measurementNum;

        // ----------------------------------------------------------------------
        // --------------------- PUBLIC METHODS ---------------------------------
        // ----------------------------------------------------------------------

        /** Resets all parameters to their default values */
        void Clear();

        /** Reads and parses a given cfg.txt - file.
            The read in parameters will be stored in this object.
            @return 1 on success otherwise 0*/
        int ReadCfgTxt(const CString& fileName);

        /** Writes a cfg.txt - file.
            @return 1 on success otherwise 0*/
        int WriteCfgTxt(const CString& fileName);
    };
}