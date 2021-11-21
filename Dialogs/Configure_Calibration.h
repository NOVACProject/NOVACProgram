#pragma once

#include <memory>
#include "MobileConfiguration.h"

// Configure_Calibration dialog

class Configure_Calibration : public CPropertyPage
{
    DECLARE_DYNAMIC(Configure_Calibration)

public:
    Configure_Calibration(CWnd* pParent = nullptr);   // standard constructor
    virtual ~Configure_Calibration();

    // Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_CONFIGURE_CALIBRATION };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()
};
