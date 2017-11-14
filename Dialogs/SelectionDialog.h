#pragma once
#include "afxwin.h"

namespace Dialogs
{

  // CSelectionDialog dialog

  class CSelectionDialog : public CDialog
  {
	  DECLARE_DYNAMIC(CSelectionDialog)

  public:
	  CSelectionDialog(CWnd* pParent = NULL);   // standard constructor
	  virtual ~CSelectionDialog();

  // Dialog Data
	  enum { IDD = IDD_QUERY_SELECTION_DIALOG };

    static const int MAX_OPTIONS = 50;

    /** The options in the combo-box */
    CString m_option[MAX_OPTIONS];

    /** The index of the selected string */
    CString  *m_currentSelection;

    /** The window text */
    CString m_windowText;

  protected:
	  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	  DECLARE_MESSAGE_MAP()
  public:
    CComboBox m_comboBox;
    virtual BOOL OnInitDialog();
  protected:
    virtual void OnOK();
  };

}