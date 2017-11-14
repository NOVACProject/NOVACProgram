#pragma once

#include "SimplePropertySheet.h"

namespace Dialogs
{

	// CImportDlg

	class CImportDlg : public CSimplePropertySheet
	{
		DECLARE_DYNAMIC(CImportDlg)

	public:
		CImportDlg();
		virtual ~CImportDlg();

	protected:
		DECLARE_MESSAGE_MAP()
	};
}