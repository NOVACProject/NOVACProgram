#include "stdafx.h"
#include "../NovacMasterProgram.h"
#include "ManualCompositionDlg.h"

// the settings...
#include "../Configuration/Configuration.h"

#include "../Communication/CommunicationController.h"

using namespace Dialogs;

extern CConfigurationSetting g_settings;	// <-- The settings
extern CWinThread *g_comm;								// <-- The communication controller

// CManualCompositionDlg dialog

IMPLEMENT_DYNAMIC(CManualCompositionDlg, CDialog)
CManualCompositionDlg::CManualCompositionDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CManualCompositionDlg::IDD, pParent)
{
    m_specNum = 0;

    m_plumeCentre = 0;
    m_plumeEdgeLow = -20;
    m_plumeEdgeHigh = 20;

    m_stepsPerRound[0] = 200;
    m_stepsPerRound[1] = 200;

    m_motorStepsComp[0] = 85;
    m_motorStepsComp[1] = 85;

    m_compass = 0;
}

CManualCompositionDlg::~CManualCompositionDlg()
{
}

void CManualCompositionDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);

    DDX_Control(pDX, IDC_LIST_SPECTROMETERS, m_spectrometerList);
    DDX_Control(pDX, IDC_EDIT_PLUMECENTRE, m_editPlumeCentre);

    DDX_Text(pDX, IDC_EDIT_PLUMECENTRE, m_plumeCentre);
    DDX_Text(pDX, IDC_EDIT_PLUMEDGE_LOW, m_plumeEdgeLow);
    DDX_Text(pDX, IDC_EDIT_PLUMEDGE_HIGH, m_plumeEdgeHigh);

    DDX_Text(pDX, IDC_EDIT_MOTORSTEPSCOMP, m_motorStepsComp[0]);
    DDX_Text(pDX, IDC_EDIT_STEPSPERROUND, m_stepsPerRound[0]);
    DDX_Text(pDX, IDC_EDIT_COMPASSDIR, m_compass);

}


BEGIN_MESSAGE_MAP(CManualCompositionDlg, CDialog)
    ON_BN_CLICKED(IDOK, OnSend)
END_MESSAGE_MAP()


// CManualCompositionDlg message handlers

/** Called to initialize the controls in the dialog*/
BOOL CManualCompositionDlg::OnInitDialog() {

    CDialog::OnInitDialog();

    /** Setting up the list */
    for (unsigned int k = 0; k < g_settings.scannerNum; ++k) {
        for (unsigned int i = 0; i < g_settings.scanner[k].specNum; ++i) {
            // add this spectrometer to the lists
            m_spectrometer[m_specNum].Format(g_settings.scanner[k].spec[i].serialNumber);
            m_spectrometerList.AddString(m_spectrometer[m_specNum]);
            ++m_specNum;
        }
    }

    // Select the first spectrometer in the list
    if (m_specNum > 0)
        m_spectrometerList.SetCurSel(0);

    // Set the focus to the motor-position edit-box
    m_editPlumeCentre.SetFocus();

    return FALSE; //<-- return false if we've set the focus to a control
}

/** When the user presses the 'Send' - button */
void CManualCompositionDlg::OnSend() {
    CString *fileName = new CString();
    CString *serialNumber = new CString();
    CString dateTime, message;
    double alpha;
    Common common;

    // 1. Get values the user has written in the dialog
    UpdateData(TRUE);

    // 2. Get a handle to the selected spectrometer
    int curSel = m_spectrometerList.GetCurSel();
    if (curSel < 0) {
        MessageBox("Please Select a spectrometer");
        return;
    }
    serialNumber->Format(m_spectrometer[curSel]);
    CConfigurationSetting::ScanningInstrumentSetting *scanner = NULL;
    for (unsigned int k = 0; k < g_settings.scannerNum; ++k) {
        if (Equals(g_settings.scanner[k].spec[0].serialNumber, *serialNumber)) {
            scanner = &g_settings.scanner[k];
            break;
        }
    }

    // 3. Get the directory where to temporarily store the cfgonce.txt
    if (strlen(g_settings.outputDirectory) > 0) {
        fileName->Format("%s\\Temp\\%s\\cfgonce.txt", (LPCSTR)g_settings.outputDirectory, (LPCSTR)*serialNumber);
    }
    else {
        common.GetExePath();
        fileName->Format("%s\\cfgonce.txt", (LPCSTR)common.m_exePath);
    }
    FILE *f = fopen(*fileName, "w");
    if (f == NULL) {
        message.Format("Could not open %s for writing. Upload failed!!", (LPCSTR)*fileName);
        MessageBox(message, "Error");
        return;
    }

    // The number of degrees for each step the motor makes
    double stepAngle = 360.0 / m_stepsPerRound[0];

    // The maximum exposure time
    double maxExpTime = 200;

    // The number of measurement positions inside the plume...
    int measurementsInPlume = (int)(abs(m_plumeEdgeHigh - m_plumeEdgeLow) / stepAngle);
    if (measurementsInPlume < 5)
    {
        fclose(f);
        return; // there's not enough space to make 5 measurements in the plume. Quit!
    }
    measurementsInPlume = min(measurementsInPlume, 8); // we can't make more than 8 measurements

    double alphaStep = abs(m_plumeEdgeHigh - m_plumeEdgeLow) / measurementsInPlume;
    alphaStep = stepAngle * floor(alphaStep / stepAngle); // the steps has to be a multiple of 1.8 degrees

    // The number of repetitions for the normal measurements, in the Manne-box only
    int	repetitions = 67;

    // 4. Write the configuration-file

    // 4a. A small header 
    common.GetDateTimeText(dateTime);
    fprintf(f, "%% -------------Modified at %s------------\n\n", (LPCSTR)dateTime);

    // 4c. Write the Spectrum transfer information
    fprintf(f, "%% The following channels defines which channels in the spectra that will be transferred\n");
    fprintf(f, "STARTCHN=0\n");
    fprintf(f, "STOPCHN=2047\n\n");

    // 4d. Don't use real-time collection
    fprintf(f, "%% If Realtime=1 then the spectra will be added to work.pak one at a time.\n");
    fprintf(f, "%% If RealTime=0 then the spectra will be added to work.pak one scan at a time\n");
    fprintf(f, "REALTIME=0\n\n");

    // 4e. Write the motor information
    fprintf(f, "%% StepsPerRound defines the number of steps the steppermotor divides one round into\n");
    fprintf(f, "STEPSPERROUND=%ld\n", m_stepsPerRound[0]);
    fprintf(f, "MOTORSTEPCOMP=%ld\n", m_motorStepsComp[0]);
    fprintf(f, "%% If Skipmotor=1 then the scanner will not be used. ONLY FOR TESTING PURPOSES\n");
    fprintf(f, "SKIPMOTOR=0\n");
    fprintf(f, "DELAY=%d\n\n", 200);

    // 4f. Write the geometry (compass, tilt...)
    fprintf(f, "%% The geometry: compassDirection  tiltX(=roll)  tiltY(=pitch)  temperature\n");
    fprintf(f, "COMPASS=%.1lf 0.0 0.0\n\n", m_compass);

    // 4g. Write other things
    fprintf(f, "%% Percent defines how big part of the spectrometers dynamic range we want to use\n");
    fprintf(f, "PERCENT=%.2lf\n\n", 0.6);
    fprintf(f, "%% The maximum integration time that we allow the spectrometer to use. In milli seconds\n");
    fprintf(f, "MAXINTTIME=%.0lf\n\n", maxExpTime);
    fprintf(f, "%% The debug-level, the higher number the more output will be created\n");
    fprintf(f, "DEBUG=1\n\n");

    // 4h. Write the measurement information
    fprintf(f, "%% sum1 is inside the specrometer [1 to 15]\n%%-----pos----time-sum1-sum2--chn--basename----- repetitions\n");

    // 4i. The offset-measurement
    fprintf(f, "MEAS=100 3 15 100 0 offset 1 0\n");

    // 4j. The dark-current measurement
    fprintf(f, "MEAS=100 10000 1 1 0 dark_cur 1 0\n");

    // 4k. Each of the measurements

    // 4k1. The first of the measurements outside the plume...
    alpha = (min(m_plumeEdgeLow, m_plumeEdgeHigh) - 90) / 2;
    fprintf(f, "MEAS=%.0lf -1 15 %d 0 sky 1 0\n", alpha / stepAngle, repetitions);

    // 3k2. The measurements inside the plume
    alpha = m_plumeCentre - (measurementsInPlume - 1) * alphaStep / 2;
    for (int k = 0; k < measurementsInPlume; ++k) {
        fprintf(f, "MEAS=%.0lf -1 15 %d 0 comp 1 0\n", alpha / stepAngle, repetitions);

        alpha += alphaStep;
    }

    // 3k2. The second of the measurements outside the plume...
    alpha = (max(m_plumeEdgeLow, m_plumeEdgeHigh) + 90) / 2;
    fprintf(f, "MEAS=%.0lf -1 15 %d 0 sky 1 0\n", alpha / stepAngle, repetitions);

    // Close the file
    fclose(f);

    // Tell the communication controller that we want to upload a file
    if (g_comm != NULL)
    {
        g_comm->PostThreadMessage(WM_UPLOAD_CFGONCE, (WPARAM)serialNumber, (LPARAM)fileName);
    }

    CDialog::OnOK();
}
