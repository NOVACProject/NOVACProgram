#pragma once

#include "../Graphs/SpectrumGraph.h"
#include <SpectralEvaluation/Spectra/Spectrum.h>
#include <SpectralEvaluation/File/MKPack.h>

// CPakFileInspector dialog

namespace Dialogs {
    class CPakFileInspector : public CDialog
    {
        DECLARE_DYNAMIC(CPakFileInspector)

    public:
        CPakFileInspector(CWnd* pParent = NULL);   // standard constructor
        virtual ~CPakFileInspector();

        // Dialog Data
        enum { IDD = IDD_CHECK_PAKFILE };

    protected:
        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

        DECLARE_MESSAGE_MAP()
        // ------------------- PROTECTED DATA -----------------------------

        /** The name of the .pak-file opened */
        CString m_fileName;

        /** The current spectrum in the .pak-file */
        int m_curSpectrum;

        /** The total number of spectra in the .pak-file */
        int m_spectrumNum;

        /** The contents of the spectrum 'm_curspectrum' */
        novac::CSpectrum m_spectrum;

        /** The header of the current spectrum */
        novac::MKZYhdr m_spectrumHeader;

        /** The timer, to restore the zoomability of the graph 0.5 seconds after
                the user has opened the file */
        UINT_PTR m_timer;

        // ------------------ PROTECTED DIALOG COMPONENTS -------------------------

        /** the properties frame */
        CStatic m_propertiesFrame;

        /** The frame for the graph */
        CStatic m_graphFrame;

        /** The frame for the spectrum header info */
        CStatic m_headerFrame;

        /** The list of the properties of the .pak-file */
        CListCtrl m_propertyList;

        /** The list of the header-items of the current spectrum */
        CListCtrl m_headerList;

        /** The spectrum graph */
        Graph::CSpectrumGraph m_graph;

        /** The spin control, which lets the user select which spectrum to show. */
        CSpinButtonCtrl m_specSpinCtrl;

        // ------------------- PROTECTED METHODS -----------------------------

        /** Checks the .pak-file specified as 'm_fileName' and updates the screen
                for the properties of the file. */
        void CheckPakFile();

        /** Initialize the properties-list */
        void InitPropertiesList();

        /** Initialize the header - list */
        void InitHeaderList();

        /** Initialize the graph */
        void InitGraph();

        /** Draws the currently selected spectrum from the currently selected spectrum-file
                    into the spectrum graph. */
        void  DrawSpectrum();

        /** Update the header list */
        void  UpdateHeaderList();

        /** Update the file information */
        void UpdateFileInfo();

        /** Tries to read the spectrum number 'm_curSpectrum' from 'm_fileName' */
        int TryReadSpectrum();

        /** Gets the range of the plot */
        void GetPlotRange(Graph::CSpectrumGraph::plotRange& range);

        /** Zooming in the graph */
        LRESULT OnZoomGraph(WPARAM wParam, LPARAM lParam);

    public:
        // --------------- PUBLIC METHODS --------------------------
        /** Called to open a .pak-file to inspect */
        afx_msg void OnOpenPakFile();

        /** Called when the user presses one of the buttons of the m_specSpinCtrl,
                leads to the selection of another spectrum with redrawing of the controls. */
        afx_msg void OnChangeSpectrum(NMHDR* pNMHDR, LRESULT* pResult);

        /** Initializes the controls in the dialog */
        virtual BOOL OnInitDialog();

        /** Called on timer-events */
        afx_msg void OnTimer(WPARAM nIDEvent);
    };
}