#pragma once

#include "../Common/Common.h"

// CManualCompositionDlg dialog
namespace Dialogs {

    class CManualCompositionDlg : public CDialog
    {
        DECLARE_DYNAMIC(CManualCompositionDlg)

    public:
        /** Default constructor */
        CManualCompositionDlg(CWnd* pParent = NULL);   // standard constructor

        /** Default destructor */
        virtual ~CManualCompositionDlg();

        // Dialog Data
        enum { IDD = IDD_STARTDLG_COMPOSITIONMEAS };

    protected:
        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

        DECLARE_MESSAGE_MAP()

        /** Called to initialize the controls in the dialog*/
        virtual BOOL OnInitDialog();

        /** When the user presses the 'Send' - button */
        afx_msg void OnSend();

        /** The edit box with the plume centre position */
        CEdit		m_editPlumeCentre;

        /** The list of spectrometers in the dialog */
        CListBox m_spectrometerList;

        /** The list of spectrometers */
        CString m_spectrometer[MAX_NUMBER_OF_SCANNING_INSTRUMENTS];

        /** The number of spectrometers in the list */
        long		m_specNum;

        /** The centre position of the plume */
        long	m_plumeCentre;

        /** The low edge of the plume */
        long m_plumeEdgeLow;

        /** The high edge of the plume */
        long m_plumeEdgeHigh;

        /** The number of steps per round for the motor */
        long m_stepsPerRound[2];

        /** The motors 'steps compensation'*/
        long m_motorStepsComp[2];

        /** The compass direction */
        double	m_compass;
    };
}