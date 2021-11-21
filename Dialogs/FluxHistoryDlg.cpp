// FluxHistoryDlg.cpp : implementation file
//

#include "stdafx.h"
#include "../resource.h"
#include "FluxHistoryDlg.h"
#include "afxdialogex.h"
#include <afxwin.h>
#include <ctime>
#include <fstream>
#include "../Configuration/Configuration.h"
#include "../Configuration/ConfigurationFileHandler.h"
#include "../UserSettings.h"
#include "../Common/FluxLogFileHandler.h"
#include <SpectralEvaluation/StringUtils.h>

extern CConfigurationSetting	g_settings;
extern CUserSettings			g_userSettings;

// FluxHistoryDlg dialog

using namespace Graph;
using namespace FileHandler;
using namespace novac;

IMPLEMENT_DYNAMIC(FluxHistoryDlg, CPropertyPage)

// Implemented in View_Scanner
void ResizeGraphControl(CGraphCtrl& controlToResize, CStatic& boundingFrame, CWnd* owningWindow);

FluxHistoryDlg::FluxHistoryDlg()
    : CPropertyPage(FluxHistoryDlg::IDD)
{
    m_evalDataStorage = NULL;
    m_scannerIndex = 0;
    m_serialNumber.Format("");
    m_siteName.Format("");
    m_lastDay = 0;
}

FluxHistoryDlg::~FluxHistoryDlg()
{
}

void FluxHistoryDlg::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);

    DDX_Control(pDX, IDC_FLUX_FRAME, m_frame);
    DDX_Control(pDX, IDC_FLUX_10DAY_FRAME, m_frame10);
    DDX_Control(pDX, IDC_FLUX_30DAY_FRAME, m_frame30);
}


BEGIN_MESSAGE_MAP(FluxHistoryDlg, CPropertyPage)
    ON_WM_SIZE()
    ON_MESSAGE(WM_EVAL_SUCCESS, OnEvalSuccess)
END_MESSAGE_MAP()


BOOL FluxHistoryDlg::OnInitDialog()
{
    CPropertyPage::OnInitDialog();

    m_lastDay = common.GetDay();

    m_minFlux = g_settings.scanner[m_scannerIndex].minFlux;
    m_maxFlux = g_settings.scanner[m_scannerIndex].maxFlux;

    // Initialize plots
    InitPlot();
    Init10DayPlot();
    Init30DayPlot();

    // Read evaluation logs 
    ReadFluxLogs();

    return TRUE;
}

void FluxHistoryDlg::InitPlot() {

    CRect rect;
    m_frame.GetWindowRect(rect);
    int height = rect.bottom - rect.top;
    int width = rect.right - rect.left;
    rect.top = 20;
    rect.bottom = height - 10;
    rect.left = 10;
    rect.right = width - 10;
    m_plot.Create(WS_VISIBLE | WS_CHILD, rect, &m_frame);

    Common common;
    m_plot.SetXUnits(common.GetString(AXIS_UTCTIME));
    m_plot.SetXAxisNumberFormat(FORMAT_DATETIME);
    if (g_userSettings.m_fluxUnit == UNIT_TONDAY) {
        m_plot.SetYUnits("Flux [ton/day]");
    }
    else {
        m_plot.SetYUnits("Flux [kg/s]");
    }
    m_plot.EnableGridLinesX(true);
    m_plot.SetPlotColor(RGB(255, 255, 255));
    m_plot.SetGridColor(RGB(255, 255, 255));
    m_plot.SetBackgroundColor(RGB(0, 0, 0));
    m_plot.SetCircleColor(RGB(255, 255, 255));
    m_plot.SetCircleRadius(3);

    SetRange();
}

void FluxHistoryDlg::Init10DayPlot() {

    CRect rect;
    m_frame10.GetWindowRect(rect);
    int height = rect.bottom - rect.top;
    int width = rect.right - rect.left;
    rect.top = 20;
    rect.bottom = height - 10;
    rect.left = 10;
    rect.right = width - 10;
    m_plot10.Create(WS_VISIBLE | WS_CHILD, rect, &m_frame10);

    Common common;
    m_plot10.SetXUnits(common.GetString(AXIS_UTCTIME));
    m_plot10.SetXAxisNumberFormat(FORMAT_DATE);
    if (g_userSettings.m_fluxUnit == UNIT_TONDAY) {
        m_plot10.SetYUnits("Flux [ton/day]");
    }
    else {
        m_plot10.SetYUnits("Flux [kg/s]");
    }
    m_plot10.EnableGridLinesX(true);
    m_plot10.SetPlotColor(RGB(255, 255, 255));
    m_plot10.SetGridColor(RGB(255, 255, 255));
    m_plot10.SetBackgroundColor(RGB(0, 0, 0));
    m_plot10.SetCircleColor(RGB(255, 255, 255));
    m_plot10.SetCircleRadius(3);
}

void FluxHistoryDlg::Init30DayPlot() {

    CRect rect;
    m_frame30.GetWindowRect(rect);
    int height = rect.bottom - rect.top;
    int width = rect.right - rect.left;
    rect.top = 20;
    rect.bottom = height - 10;
    rect.left = 10;
    rect.right = width - 10;
    m_plot30.Create(WS_VISIBLE | WS_CHILD, rect, &m_frame30);

    Common common;
    m_plot30.SetXUnits(common.GetString(AXIS_UTCTIME));
    m_plot30.SetXAxisNumberFormat(FORMAT_DATE);
    if (g_userSettings.m_fluxUnit == UNIT_TONDAY) {
        m_plot30.SetYUnits("Flux [ton/day]");
    }
    else {
        m_plot30.SetYUnits("Flux [kg/s]");
    }
    m_plot30.EnableGridLinesX(true);
    m_plot30.SetPlotColor(RGB(255, 255, 255));
    m_plot30.SetGridColor(RGB(255, 255, 255));
    m_plot30.SetBackgroundColor(RGB(0, 0, 0));
    m_plot30.SetCircleColor(RGB(255, 255, 255));
    m_plot30.SetCircleRadius(3);

}
void FluxHistoryDlg::DrawPlot() {
    // variables
    const int BUFFER_SIZE = 500;
    double time[BUFFER_SIZE];
    double allFluxes[BUFFER_SIZE];
    int fluxOk[BUFFER_SIZE];
    double goodFluxes[BUFFER_SIZE];
    double badFluxes[BUFFER_SIZE];
    double goodTime[BUFFER_SIZE];
    double badTime[BUFFER_SIZE];
    int nGoodFluxes = 0;
    int nBadFluxes = 0;

    // Get the data
    int dataLength = m_evalDataStorage->GetFluxData(m_serialNumber, time, allFluxes, fluxOk, BUFFER_SIZE);

    // If there's no data then don't draw anything
    if (dataLength <= 0)
        return;

    // sort the data into good values and bad values
    for (int k = 0; k < dataLength; ++k) {
        if (fluxOk[k]) {
            goodFluxes[nGoodFluxes] = allFluxes[k];
            goodTime[nGoodFluxes] = time[k];
            ++nGoodFluxes;
        }
        else {
            badFluxes[nBadFluxes] = allFluxes[k];
            badTime[nBadFluxes] = time[k];
            ++nBadFluxes;
        }
    }

    // remove the old plot
    m_plot.CleanPlot();

    // Set the unit of the plot
    if (g_userSettings.m_fluxUnit == UNIT_TONDAY) {
        m_plot.SetYUnits("Flux [ton/day]");
    }
    else {
        m_plot.SetYUnits("Flux [kg/s]");
    }

    SetRange();

    m_evalDataStorage->GetTimeData(m_serialNumber, time, BUFFER_SIZE, true);

    // First draw the bad values, then the good ones
    m_plot.SetCircleColor(RGB(150, 150, 150));
    m_plot.XYPlot(badTime, badFluxes, nBadFluxes, CGraphCtrl::PLOT_FIXED_AXIS | CGraphCtrl::PLOT_CIRCLES);

    m_plot.SetCircleColor(RGB(255, 255, 255));
    m_plot.XYPlot(goodTime, goodFluxes, nGoodFluxes, CGraphCtrl::PLOT_FIXED_AXIS | CGraphCtrl::PLOT_CIRCLES);
}

void FluxHistoryDlg::ReadFluxLogs() {
    // clean plots
    if (!IsWindow(m_frame10.m_hWnd) || !IsWindow(m_frame30.m_hWnd)) {
        return;
    }
    m_plot10.CleanPlot();
    m_plot30.CleanPlot();

    // get flux units
    double unitConversionFactor = 0;
    if (g_userSettings.m_fluxUnit == UNIT_TONDAY) {
        unitConversionFactor = 1.0;
    }
    else {
        unitConversionFactor = 2.5e15;
    }

    // set time range
    SetHistoryRange();

    // get current time UTC
    time_t rawtime;
    struct tm* utc;
    time(&rawtime);
    utc = gmtime(&rawtime);
    // get time offset
    time_t offset = mktime(utc) - rawtime;
    if (utc->tm_isdst) {
        offset -= 3600;
    }
    // make it midnight (00:00:00) of current day
    utc->tm_hour = 0;
    utc->tm_min = 0;
    utc->tm_sec = 0;

    // Read last 30 days worth of flux logs
    CString path, dateStr;
    CWaitCursor wait;
    for (int day = 0; day < 30; day++) {
        FileHandler::CFluxLogFileHandler fluxLogReader;

        // get date
        utc->tm_sec -= (SECONDS_IN_DAY);
        time_t epochDay = mktime(utc);
        int year = utc->tm_year + 1900;
        int month = utc->tm_mon + 1;
        int mday = utc->tm_mday;
        dateStr.Format("%04d.%02d.%02d", year, month, mday);

        // Read file
        path.Format("%sOutput\\%s\\%s\\FluxLog_%s_%s.txt",
            (LPCTSTR)g_settings.outputDirectory,
            (LPCTSTR)dateStr,
            (LPCTSTR)m_serialNumber,
            (LPCTSTR)m_serialNumber,
            (LPCTSTR)dateStr);

        // check if file exists
        std::ifstream ifile(path);
        if (!ifile) {
            continue;
        }

        // Try to read the flux-log
        fluxLogReader.m_fluxLog.Format(path);
        if (FAIL == fluxLogReader.ReadFluxLog())
            return;

        // check if there are fluxes in log
        int fluxNum = fluxLogReader.m_fluxesNum;
        if (fluxNum <= 0) {
            continue;
        }

        // Plot the read in data
        m_goodFluxNum[day] = 0;
        m_badFluxNum[day] = 0;
        for (int j = 0; j < fluxNum; ++j) {
            Evaluation::CFluxResult& fr = fluxLogReader.m_fluxes[j];
            CDateTime st = fr.m_startTime;
            int startsec = st.hour * 3600 + st.minute * 60 + st.second;
            double epoch = (double)(epochDay + startsec - offset); // unsure why offset substraction is needed but it is
            if (fr.m_fluxOk && m_goodFluxNum[day] < 500) {
                m_goodTime[day][m_goodFluxNum[day]] = epoch;
                m_goodFlux[day][m_goodFluxNum[day]] = fr.m_flux * unitConversionFactor;
                m_goodFluxNum[day]++;
            }
            if (!fr.m_fluxOk && m_badFluxNum[day] < 500) {
                m_badTime[day][m_badFluxNum[day]] = epoch;
                m_badFlux[day][m_badFluxNum[day]] = fr.m_flux * unitConversionFactor;
                m_badFluxNum[day]++;
            }
        }
    }
    wait.Restore();
}

void FluxHistoryDlg::DrawHistoryPlots() {
    // plot
    for (int day = 0; day < 30; day++) {
        if (day < 10) {
            // First draw the bad values, then the good ones
            m_plot10.SetCircleColor(RGB(150, 150, 150));
            m_plot10.XYPlot(m_badTime[day], m_badFlux[day], m_badFluxNum[day], CGraphCtrl::PLOT_FIXED_AXIS | CGraphCtrl::PLOT_CIRCLES);

            m_plot10.SetCircleColor(RGB(255, 255, 255));
            m_plot10.XYPlot(m_goodTime[day], m_goodFlux[day], m_goodFluxNum[day], CGraphCtrl::PLOT_FIXED_AXIS | CGraphCtrl::PLOT_CIRCLES);
        }

        // First draw the bad values, then the good ones
        m_plot30.SetCircleColor(RGB(150, 150, 150));
        m_plot30.XYPlot(m_badTime[day], m_badFlux[day], m_badFluxNum[day], CGraphCtrl::PLOT_FIXED_AXIS | CGraphCtrl::PLOT_CIRCLES);

        m_plot30.SetCircleColor(RGB(255, 255, 255));
        m_plot30.XYPlot(m_goodTime[day], m_goodFlux[day], m_goodFluxNum[day], CGraphCtrl::PLOT_FIXED_AXIS | CGraphCtrl::PLOT_CIRCLES);
    }
}
// FluxHistoryDlg message handlers


void FluxHistoryDlg::OnSize(UINT nType, int cx, int cy)
{
    CPropertyPage::OnSize(nType, cx, cy);

    const int rightMargin = 20;

    CRect thisClientRegion;
    GetWindowRect(thisClientRegion);
    this->ScreenToClient(thisClientRegion);

    const int oneThirdScreenHeight = (int)(thisClientRegion.Height() / 3);

    // Adjust the width of each of the column plots
    if (this->m_frame.m_hWnd != NULL) {
        CRect frameClientRegion;
        m_frame.GetWindowRect(&frameClientRegion);
        this->ScreenToClient(frameClientRegion);

        frameClientRegion.top = 0;
        frameClientRegion.left = 0;
        frameClientRegion.right = thisClientRegion.right - rightMargin;
        frameClientRegion.bottom = oneThirdScreenHeight;
        m_frame.MoveWindow(frameClientRegion);

        ResizeGraphControl(m_plot, m_frame, this);
    }

    if (this->m_frame10.m_hWnd != NULL) {
        CRect frameClientRegion;
        m_frame10.GetWindowRect(&frameClientRegion);
        this->ScreenToClient(frameClientRegion);

        frameClientRegion.top = oneThirdScreenHeight;
        frameClientRegion.left = 0;
        frameClientRegion.right = thisClientRegion.right - rightMargin;
        frameClientRegion.bottom = 2 * oneThirdScreenHeight;
        m_frame10.MoveWindow(frameClientRegion);

        ResizeGraphControl(m_plot10, m_frame10, this);
    }

    if (this->m_frame30.m_hWnd != NULL) {
        CRect frameClientRegion;
        m_frame30.GetWindowRect(&frameClientRegion);
        this->ScreenToClient(frameClientRegion);

        frameClientRegion.top = 2 * oneThirdScreenHeight;
        frameClientRegion.left = 0;
        frameClientRegion.right = thisClientRegion.right - rightMargin;
        frameClientRegion.bottom = 3 * oneThirdScreenHeight;
        m_frame30.MoveWindow(frameClientRegion);

        ResizeGraphControl(m_plot30, m_frame30, this);
    }
    if (this->m_frame.m_hWnd != NULL) {
        RedrawAll();
    }
}

BOOL FluxHistoryDlg::OnSetActive()
{
    return CPropertyPage::OnSetActive();
}

void FluxHistoryDlg::SetRange() {
    struct tm* tm;
    time_t t;
    time(&t);
    tm = gmtime(&t);
    time_t endtime = t;
    time_t starttime = endtime - SECONDS_IN_DAY;
    m_plot.SetRange(starttime, endtime, 0, m_minFlux, m_maxFlux, 0);
}

void FluxHistoryDlg::SetHistoryRange() {
    // get end time
    struct tm* tm;
    time_t t;
    time(&t);
    tm = gmtime(&t);
    time_t endtime = t - (3600 * tm->tm_hour + 60 * tm->tm_min + tm->tm_sec);
    // get start time and set 10 day plot
    time_t starttime = endtime - SECONDS_IN_DAY * 10;
    m_plot10.SetRange(starttime, endtime, 0, m_minFlux, m_maxFlux, 0);
    // get start time and set 30 day plot
    starttime = endtime - SECONDS_IN_DAY * 30;
    m_plot30.SetRange(starttime, endtime, 0, m_minFlux, m_maxFlux, 0);
}
void FluxHistoryDlg::RedrawAll() {
    DrawPlot();
    DrawHistoryPlots();
}

LRESULT FluxHistoryDlg::OnEvalSuccess(WPARAM wParam, LPARAM lParam) {
    DrawPlot();

    // check to see if new UTC day. If so, draw history plot too.
    int today = common.GetDay();
    if (m_lastDay == today) {
        return 0;
    }
    ReadFluxLogs();
    DrawHistoryPlots();
    m_lastDay = today;
    return 0;
}
