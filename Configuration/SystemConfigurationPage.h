#pragma once
#include "afxdlgs.h"

#include "../Common/Common.h"
#include "Configuration.h"
#include "ConfigurationTreeCtrl.h"

namespace ConfigurationDialog
{
    /** The <b>CSystemConfigurationPage</b> is a class that handles the common parts of:
            CLocationConfigurationDlg
            CRemoteConfigurationDlg
            CEvaluationConfigurationDlg
            CConfigureCOMPortDlg
            CCalibrationConfigurationDlg
        */
    class CSystemConfigurationPage :
        public CPropertyPage
    {
        DECLARE_DYNAMIC(CSystemConfigurationPage)
    public:
        CSystemConfigurationPage(void);
        CSystemConfigurationPage(UINT nIDTemplate);
        virtual ~CSystemConfigurationPage(void);

        /** the actual configuration */
        CConfigurationSetting* m_configuration;

        /** The scanner list */
        CConfigurationTreeCtrl* m_scannerTree;

        /** The tooltip control */
        CToolTipCtrl m_toolTip;

        /** A handle to the parent window */
        CWnd* m_parent;

        /** A pointer to the currently selected scanner */
        CConfigurationSetting::ScanningInstrumentSetting* m_curScanner;

        /** A pointer to the currently selected spectrometer */
        CConfigurationSetting::SpectrometerSetting* m_curSpec;

        /** Called by the CScannerConfigurationDlg when the user wants to change
                the currently selected scanner. */
        virtual void OnChangeScanner();

        /** Gets the currently selected scanner and spectrometer */
        void GetScanAndSpec(int& scanner, int& spec);
    };
}