#pragma once
#include <afxtempl.h>
#include "../Evaluation/FluxResult.h"
#include "../Common/FluxLogFileHandler.h"
#include "../Graphs/GraphCtrl.h"

// CSummarizeFluxDataDlg dialog
namespace Dialogs {
    class CSummarizeFluxDataDlg : public CDialog
    {
        DECLARE_DYNAMIC(CSummarizeFluxDataDlg)

    public:
        CSummarizeFluxDataDlg(CWnd* pParent = NULL);   // standard constructor
        virtual ~CSummarizeFluxDataDlg();

        // Dialog Data
        enum { IDD = IDD_SUMMARIZE_POSTFLUX };

    protected:
        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

        DECLARE_MESSAGE_MAP()
    public:
        // --------------------- DIALOG COMPONENTS -----------------------
        /** The frame holding the graph */
        CStatic	m_graphFrame;

        /** The graph */
        Graph::CGraphCtrl	m_graph;

        // --------------------- PUBLIC DATA -----------------------

        /** True if we should also look in sub-directories of the given directory */
        BOOL				m_includeSubDirectories;

        /** True if we should only look for post-flux-log files,
                False if we should only look for real-time flux-log files */
        BOOL				m_lookForPostFluxLogs;

        /** What to show in the main window */
        int					m_showOption;

        /** The list of flux-log files that we have found */
        CList <CString, CString&> m_fluxLogFiles;

        /** The read-in flux results */
        CList <Evaluation::CFluxResult, Evaluation::CFluxResult&> m_fluxResults;

        // ---------------------- PUBLIC METHODS -----------------

        /** Called when the user wants to make a search for files */
        afx_msg void OnSearchForFluxLogFiles();

        /** Searches for flux-log files in the given sub-directory.
                Results are appended to the 'm_fluxLogfiles' list */
        void SearchForFluxLogFiles(const CString& directory);

        /** Reads in the data in the files 'm_fluxLogFiles' into the data-list 'm_fluxResults' */
        void ReadFluxLogFiles();

        /** Called to initialize the dialog and it's components */
        virtual BOOL OnInitDialog();
    };
}