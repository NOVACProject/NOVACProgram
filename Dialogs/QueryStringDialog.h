#pragma once

#include "afxwin.h"

namespace Dialogs{

  class CQueryStringDialog : public CDialog
  {
	  DECLARE_DYNAMIC(CQueryStringDialog)

  public:
		/** Default constructor */
	  CQueryStringDialog(CWnd* pParent = NULL);

		/** Default destructor */
		virtual ~CQueryStringDialog();

		// Dialog Data
		enum { IDD = IDD_QUERY_STRING_DIALOG };

    /** The window text */
    CString   m_windowText;

    /** The returned string */
    CString   *m_inputString;

		/** When the user presses the 'ok' button */
		virtual void OnOK();
  protected:
		virtual BOOL OnInitDialog();
	  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	  DECLARE_MESSAGE_MAP()

		/** The input edit-box */
		CEdit m_editBox;
  };
}
