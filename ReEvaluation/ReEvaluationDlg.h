#pragma once
#include "afxdlgs.h"

#include "../Dialogs/SimplePropertySheet.h"

namespace ReEvaluation
{

	class CReEvaluationDlg :  public Dialogs::CSimplePropertySheet
	{
	public:
		CReEvaluationDlg(void);
		~CReEvaluationDlg(void);

		DECLARE_MESSAGE_MAP()
		virtual BOOL OnInitDialog();
	};
}