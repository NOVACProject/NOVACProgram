#pragma once

#include "GraphCtrl.h"
#include "../Common/Common.h"
#include <SpectralEvaluation/Spectra/Spectrum.h>

namespace Graph {
    class CDOASFitGraph :
        public CGraphCtrl
    {
    public:
        CDOASFitGraph(void);
        ~CDOASFitGraph(void);

        /** the pixel range used for the fit */
        int m_fitLow;
        int m_fitHigh;

        /** The number of references included in the fit */
        int m_nReferences;

        /** A local copy of the residual of the last fit */
        double m_residual[MAX_SPECTRUM_LENGTH];

        /** A local copy of the last fit result */
        double m_fitResult[MAX_N_REFERENCES][MAX_SPECTRUM_LENGTH];

        /** A local copy of the names of the references fitted */
        CString m_specieName[MAX_N_REFERENCES];

        /** Draws the spectrum fit */
        void DrawFit(int refIndex = 0);

        DECLARE_MESSAGE_MAP()
        afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
        afx_msg void OnSaveAsAscii();

    private:
        void WriteAsciiFile(const CString& fileName);

    };
}
