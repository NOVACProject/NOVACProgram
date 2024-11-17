#include "stdafx.h"
#include "../NovacMasterProgram.h"
#include "reevaluationdlg.h"

namespace ReEvaluation
{

CReEvaluationDlg::CReEvaluationDlg(void)
{}

CReEvaluationDlg::~CReEvaluationDlg(void)
{}
BEGIN_MESSAGE_MAP(CReEvaluationDlg, CPropertySheet)
END_MESSAGE_MAP()

BOOL ReEvaluation::CReEvaluationDlg::OnInitDialog()
{
    BOOL bResult = CSimplePropertySheet::OnInitDialog();

    return bResult;
}

}  // namespace ReEvaluation
