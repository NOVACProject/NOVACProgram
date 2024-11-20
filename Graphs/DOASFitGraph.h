#pragma once

#include <string>
#include <vector>

#include "GraphCtrl.h"

namespace Graph
{
    class CDOASFitGraph :
        public CGraphCtrl
    {
    public:
        CDOASFitGraph();
        ~CDOASFitGraph();

        /** the pixel range used for the fit */
        int m_fitLow;
        int m_fitHigh;

        /** The number of references included in the fit */
        int m_nReferences;

        /** A local copy of the residual of the last fit */
        std::vector<double> m_residual;

        /** A local copy of the last fit result.
            First index is reference, second is pixel. */
        std::vector<std::vector<double>> m_fitResult;

        /** A local copy of the names of the references fitted */
        std::vector<std::string> m_specieName;

        /** Draws the spectrum fit */
        void DrawFit(int refIndex = 0);

        DECLARE_MESSAGE_MAP()
        afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
        afx_msg void OnSaveAsAscii();

    private:
        void WriteAsciiFile(const CString& fileName);

    };
}
