#pragma once

#include "GridCtrl\GridCtrl.h"

#include "../Common/Common.h"
#include "../Common/ReferenceFile.h"

#include "../Evaluation/FitWindow.h"
#include "../Evaluation/Evaluation.h"

namespace DlgControls
{
  class CReferenceFileControl : public CGridCtrl
  {
  public:
    CReferenceFileControl(void);
    ~CReferenceFileControl(void);

    DECLARE_MESSAGE_MAP()

  public:
    /** Handle to the parent */
    CWnd *parent;

    /** Pointer to the fit window that this grid controls */
    Evaluation::CFitWindow *m_window;

    /** Called when the user has finished editing one cell */
    void OnEndEditCell(int nRow, int nCol, CString str);

   /** Called to show a context menu */
    afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);

  private:
    void ParseShiftOption(Evaluation::SHIFT_TYPE &option, double &value, CString &str);

  };
}