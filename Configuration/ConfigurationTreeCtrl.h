#pragma once

#include "Configuration.h"

// CConfigurationTreeCtrl

namespace ConfigurationDialog{

	class CConfigurationTreeCtrl : public CTreeCtrl
	{
		DECLARE_DYNAMIC(CConfigurationTreeCtrl)

	public:
		CConfigurationTreeCtrl();
		virtual ~CConfigurationTreeCtrl();

		/** The local copy of the configuration list */
		const CConfigurationSetting *m_configuration;

		/** Updates the tree control */
		void UpdateTree();

		/** Fills in the configuration tree control */
		void PopulateTreeControl();

		/** Retrieves the currently selected scanner and spectrometer */
		void GetCurScanAndSpec(int &curScan, int &curSpec);

	protected:
		DECLARE_MESSAGE_MAP()

	};
}