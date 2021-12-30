#pragma once

#include "../Common/Common.h"

namespace Dialogs {

    class CManualWindDlg : public CDialog
    {
        DECLARE_DYNAMIC(CManualWindDlg)

    public:
        /** Default constructor */
        CManualWindDlg(CWnd* pParent = NULL);

        /** Default destructor */
        virtual ~CManualWindDlg();

        // Dialog Data
        enum { IDD = IDD_WIND_STARTDLG };

    protected:
        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

        /** Called to initialize the controls in the dialog*/
        virtual BOOL OnInitDialog();

        DECLARE_MESSAGE_MAP()

        /** When the user presses the 'Send' - button */
        afx_msg void OnSend();

        /** When the user presses changes the selected spectrometer */
        afx_msg void OnChangeSpectrometer();

        /** The list of spectrometers in the dialog */
        CListBox m_spectrometerList;

        /** The list of spectrometers */
        CString m_spectrometer[MAX_NUMBER_OF_SCANNING_INSTRUMENTS];

        /** The number of spectrometers in the list */
        long		m_specNum;

        /** The edit-boxes for the motor position(s) */
        CEdit		m_editMotorPosition, m_editMotorPosition2;

        /** The edit-boxes for the number of steps per round */
        CEdit		m_editStepsPerRound, m_editStepsPerRound2;

        /** The edit-boxes for the motor steps compensation */
        CEdit		m_editMotorStepsComp, m_editMotorStepsComp2;

        /** The desired motor position */
        long		m_motorPosition[2];

        /** The desired number of spectra to co-add in the spectrometer */
        long		m_sum1;

        /** The desired number of repetitions */
        long		m_repetitions;

        /** The number of steps per round for the motor */
        long m_stepsPerRound[2];

        /** The motors 'steps compensation'*/
        long m_motorStepsComp[2];

        /** The compass direction */
        double	m_compass;

        /** The desired intensity level */
        double	m_percent;

        /** The maximum exposuretime */
        double	m_maxExpTime;
    };
}