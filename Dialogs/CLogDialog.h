#pragma once

#include <vector>
#include <string>

// CLogDialog dialog

class CLogDialog : public CDialog
{
    DECLARE_DYNAMIC(CLogDialog)

public:
    CLogDialog(std::vector<std::string>& logEntries, CWnd* pParent = nullptr);
    virtual ~CLogDialog();

    /** Initializes the controls and the dialog */
    virtual BOOL OnInitDialog();

    // Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_VIEW_LOG_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()
public:
    CListBox m_listBox;

private:
    std::vector<std::string> m_logEntries;
};
