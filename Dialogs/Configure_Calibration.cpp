#include "stdafx.h"
#include "../DMSpec.h"
#include "Configure_Calibration.h"


// Configure_Calibration dialog

IMPLEMENT_DYNAMIC(Configure_Calibration, CPropertyPage)

Configure_Calibration::Configure_Calibration(CWnd* pParent /*=nullptr*/)
    : CPropertyPage(IDD_CONFIGURE_CALIBRATION, pParent)
{

}

Configure_Calibration::~Configure_Calibration()
{
}

void Configure_Calibration::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(Configure_Calibration, CPropertyPage)
END_MESSAGE_MAP()


// Configure_Calibration message handlers
