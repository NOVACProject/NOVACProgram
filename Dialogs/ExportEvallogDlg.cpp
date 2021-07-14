#include "stdafx.h"
#include "../NovacMasterProgram.h"
#include "ExportEvallogDlg.h"

#include "../Common/Common.h"

#include "../PostFlux/PostFluxCalculator.h"

using namespace Dialogs;
using namespace novac;

// CExportEvallogDlg dialog

IMPLEMENT_DYNAMIC(CExportEvallogDlg, CPropertyPage)
CExportEvallogDlg::CExportEvallogDlg()
	: CPropertyPage(CExportEvallogDlg::IDD)
{
  m_exportFormat = 0;
}

CExportEvallogDlg::~CExportEvallogDlg()
{
}

void CExportEvallogDlg::DoDataExchange(CDataExchange* pDX)
{
  CPropertyPage::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_EXPORT_LOG_BTN, m_exportBtn);
  DDX_Radio(pDX, IDC_RADIO_EXPORTFORMAT_SCANDOAS, m_exportFormat);
}


BEGIN_MESSAGE_MAP(CExportEvallogDlg, CPropertyPage)
  ON_BN_CLICKED(IDC_BROWSE_EVALLOG_BTN, OnBrowseEvalLog)
  ON_BN_CLICKED(IDC_BROWSE_EXPORTFILE, OnBrowseExportFile)
  ON_BN_CLICKED(IDC_EXPORT_LOG_BTN, OnExportLog)
END_MESSAGE_MAP()


// CExportEvallogDlg message handlers

void CExportEvallogDlg::OnBrowseEvalLog()
{
  UpdateData(TRUE);

  CString evLog;
	evLog.Format("");
  TCHAR filter[512];
  int n = _stprintf(filter, "Evaluation Logs\0");
  n += _stprintf(filter + n + 1, "*.txt;\0");
  filter[n + 2] = 0;
  Common common;
  
  // let the user browse for an evaluation log file and if one is selected, read it
  if(common.BrowseForFile(filter, evLog)){

    // Update the screen
    SetDlgItemText(IDC_EDIT_EVALUATIONLOGPATH, evLog);

    CString exportFile;
    GetDlgItemText(IDC_EDIT_EXPORTFILE, exportFile);
    if(strlen(exportFile) > 0)
      m_exportBtn.EnableWindow(TRUE);
  }

}

void CExportEvallogDlg::OnBrowseExportFile()
{
  UpdateData(TRUE);

  CString evLog;
	evLog.Format("");
  TCHAR filter[512];
  int n = _stprintf(filter, "Evaluation Logs\0");
  n += _stprintf(filter + n + 1, "*.txt;\0");
  filter[n + 2] = 0;
  Common common;
  
  // let the user browse for an evaluation log file and if one is selected, read it
  if(common.BrowseForFile_SaveAs(filter, evLog)){

    // Update the screen
    SetDlgItemText(IDC_EDIT_EXPORTFILE, evLog);

    CString evallog;
    GetDlgItemText(IDC_EDIT_EVALUATIONLOGPATH, evallog);
    if(strlen(evallog) > 0)
      m_exportBtn.EnableWindow(TRUE);

  }
}


void CExportEvallogDlg::OnExportLog()
{
  UpdateData(TRUE);

  CString exportFile;
  GetDlgItemText(IDC_EDIT_EVALUATIONLOGPATH, m_evaluationLog);
  GetDlgItemText(IDC_EDIT_EXPORTFILE, exportFile);

  // Data checking
  if(strlen(m_evaluationLog) == 0 || strlen(exportFile) == 0)
    return;

  // Read the evaluation log
  if(SUCCESS != ReadEvaluationLog()){
    MessageBox("Could not parse evaluation logfile.");
  }

  // Save the new file
  FILE *f = fopen(exportFile, "w");
  if(NULL == f){
    MessageBox("Cannot open export file for writing. Export failed");
    return;
  }

  // 1. Write (a small version of) the header of the evaluation log file
  fprintf(f, "***ScanDOAS***\nFILETYPE=Scan Evaluation Log\n");
  fprintf(f, "STEPSPERROUND=200\nGASFACTOR=2.66\n");
  fprintf(f, "PLUME_HEIGHT=1000\nWIND_SPEED=10\nWIND_DIRECTION=180\n");
  fprintf(f, "%%--pos -- time -- sum1 -- sum2 -- chn -- basename\n");
  for(unsigned long k = 0; k < m_scan[0].GetEvaluatedNum(); ++k){
    fprintf(f, "MEAS=%.0lf\t", m_scan[0].GetScanAngle(k) / 1.8);
    fprintf(f, "100\t"); // time
    fprintf(f, "%d\t1", m_scan[0].GetSpecNum(k)); // sum1 & sum2
    fprintf(f, "0\tNN\n");  // chn & basename
  }
  fprintf(f, "\n");


  // 2. Iterate over all scans in the logfile
  for(int scanIndex = 0; scanIndex < m_scanNum; ++scanIndex){
    if(m_scan[scanIndex].GetSpecieNum(0) > 1)
      fprintf(f, "GPSTIME\tPOSITION\tORIGIN_COLUMN(Master)\tCOLUMN(Master)\tCOLUMN_ERROR(Master)\tAVERAGE_INTENSITY(Master)\tSTDFILE(Master)\tORIGIN_COLUMN(Slave)\tCOLUMN(Slave)\tCOLUMN_ERROR(Slave)\tAVERAGE_INTENSITY(Slave)\tSTDFILE(Slave)\n");
    else
      fprintf(f, "GPSTIME\tPOSITION\tORIGIN_COLUMN\tCOLUMN\tCOLUMN_ERROR\tAVERAGE_INTENSITY\tSTDFILE\n");

    for(unsigned long specIndex = 0; specIndex < m_scan[scanIndex].GetEvaluatedNum(); ++specIndex){
      // The GPS-Time
      const CDateTime *starttime = m_scan[scanIndex].GetStartTime(specIndex);
      fprintf(f, "%02d:%02d:%02d\t", starttime->hour, starttime->minute, starttime->second);

      // The Position
      fprintf(f, "%.0lf\t", m_scan[scanIndex].GetScanAngle(specIndex) / 1.8);

      for(int specieIndex = 0; specieIndex < m_scan[scanIndex].GetSpecieNum(specIndex); ++specieIndex){
        // The original column, corrected column and columnError
        double column = m_scan[scanIndex].GetColumn(specIndex, specieIndex);
        double columnError = m_scan[scanIndex].GetColumnError(specIndex, specieIndex);
        fprintf(f, "%9.3lf\t%9.3lf\t%9.3lf\t", column, column, columnError);

        // The intensity
        fprintf(f, "%.0lf\t", m_scan[scanIndex].GetPeakIntensity(specIndex));

        // The STD-file
        fprintf(f, "NN\t");
      }

      // end of this spectrum
      fprintf(f, "\n");
    }

    // end of this scan.
    fprintf(f, "\n");
  }

  fclose(f);

  MessageBox("Evaluation log file exported sucessfully!");
}
