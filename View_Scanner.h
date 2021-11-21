#pragma once
#include "afxwin.h"

#include "Common/Common.h"
#include "EvaluatedDataStorage.h"
#include "CommunicationDataStorage.h"

#include "Graphs/GraphCtrl.h"
#include "DlgControls/Label.h"

/** The <b>CView_Scanner</b>-class is a class that takes care of showing the result
    from and the status of individual scanners in the main window of the program.
    There's one instance of CView_Scanner for each Scanning instrument in the local network.*/

class CView_Scanner : public CPropertyPage
{
    DECLARE_DYNAMIC(CView_Scanner)

public:
    CView_Scanner();
    virtual ~CView_Scanner();

    virtual BOOL OnInitDialog();

    /** Fixes the layout of the main components to match the
            resolution of the screen. */
    void SetLayout();

    // Dialog Data
    enum { IDD = IDD_VIEW_SCANNERSTATUS };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    afx_msg LRESULT OnUpdateEvalStatus(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnUpdateWindParam(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnEvaluatedScan(WPARAM wParam, LPARAM lParam);

    DECLARE_MESSAGE_MAP()


public:
    // --------------- EVENT HANDLERS ------------------------------
    afx_msg void OnBnClickedButtonSetWindfield();

    /** */
    virtual BOOL PreTranslateMessage(MSG* pMsg);

    /** Called when the user selects this page */
    virtual BOOL OnSetActive();

    // ----------------- PUBLIC DATA ------------------------------

    /** A common object for doing common things */
    Common										m_common;

    /** A pointer to a shared instance of 'CEvaluatedDataStorage' */
    CEvaluatedDataStorage* m_evalDataStorage;

    /** A pointer to a shared instance of 'CCommunicationDataStorage' */
    CCommunicationDataStorage* m_commDataStorage;

    /** The index into g_settings for the scanner that
                is connected to this view. */
    unsigned int							m_scannerIndex;

    /** The serial number of 'this' spectrometer */
    CString										m_serial;

    /** The name of the site where this spectrometer is siturated */
    CString										m_siteName;

    // ----------- PUBLIC METHODS ---------------
    /** Set time range for current day plot */
    void SetTodayRange();

    /** Draws the column for the last collected scan.*/
    void DrawColumn();

    /** Draws the flux for the last day */
    void DrawFlux();

    /** Draws the columns for the last day */
    void DrawColumnDay();

protected:
    // ----------- DIALOG CONTROLS ---------------
    /** The borders for the column and flux plots */
    CStatic m_lastScanFrame, m_todayScanFrame;

    /** The column plot */
    Graph::CGraphCtrl m_lastScanPlot;

    /** The flux plot in normal measurement mode.
    Turns into column plot if single direction mode. */
    Graph::CGraphCtrl m_todayPlot;

    /** Whether plots should be displayed as time series. */
    bool m_isTimeSeries = false;

    /** Status light to show the status of the evaluation */
    CStatic m_statusLight;

    /** The label next to the evaluation status light */
    CStatic m_statusEval;

    /** status light to show communication status */
    CStatic m_commStatusLight;

    /** The label next to the communication status light */
    CStatic m_statusScanner;

    // the meteorology
    CEdit		m_windDirection, m_windSpeed, m_plumeHeight;
    double	windDirection, windSpeed, plumeHeight;
    CButton m_setButton;
    CStatic m_labelWindDirection, m_labelWindSpeed, m_labelPlumeHeight;

    // the configuration information
    CStatic m_infoSerial, m_infoCompass, m_infoLatitude, m_infoLongitude;

    // The 'Last scan'-information
    CStatic m_scanInfo_Starttime, m_scanInfo_StopTime;
    CStatic m_scanInfo_Date;
    CStatic m_scanInfo_ExpTime, m_scanInfo_NumSpec;

    /** the tool tips */
    CToolTipCtrl  m_toolTip;

    /** The legend for the column color */
    DlgControls::CLabel m_legend_ColumnColor;

    /** The legend for the peak intensity color */
    DlgControls::CLabel m_legend_PeakIntensity;

    /** The legend for the fitRegion intensity color */
    DlgControls::CLabel m_legend_FitIntensity;

    /** The bitmaps for the red, yellow and green lights */
    CBitmap	m_greenLight, m_yellowLight, m_redLight;

    // ----------------- PROTECTED DATA ------------------------------

    /** The date and time the last received scan started */
    novac::CDateTime	m_lastScanStart;

    /** The date and time of the most recent scan received */
    novac::CDateTime	m_mostRecentScanStart;

    /** The channel that the last scan used */
    int				m_lastScanChannel;

    // ----------------- PROTECTED METHODS ------------------------------

    /** Executes the script specifyed in the g_settings to call when a full scan
            has been received. */
    void ExecuteScript_Image(const CString& imageFileName);
public:
    afx_msg void OnSize(UINT nType, int cx, int cy);
};
