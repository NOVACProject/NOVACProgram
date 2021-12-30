#include "stdafx.h"
#include "../NovacMasterProgram.h"
#include "SummarizeFluxDataDlg.h"
#include "../Configuration/Configuration.h"


extern CConfigurationSetting	g_settings;

// CSummarizeFluxDataDlg dialog

using namespace Dialogs;

IMPLEMENT_DYNAMIC(CSummarizeFluxDataDlg, CDialog)
CSummarizeFluxDataDlg::CSummarizeFluxDataDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CSummarizeFluxDataDlg::IDD, pParent)
{
    m_includeSubDirectories = TRUE;
    m_lookForPostFluxLogs = TRUE;
}

CSummarizeFluxDataDlg::~CSummarizeFluxDataDlg()
{
}

void CSummarizeFluxDataDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Check(pDX, IDC_CHECK_INCLUDE_SUBDIRS, m_includeSubDirectories);
    DDX_Check(pDX, IDC_CHECK_POSTFLUXLOGS, m_lookForPostFluxLogs);

    DDX_Radio(pDX, IDC_RADIO_SHOWOPTION, m_showOption);

    DDX_Control(pDX, IDC_FRAME_GRAPH, m_graphFrame);
}


BEGIN_MESSAGE_MAP(CSummarizeFluxDataDlg, CDialog)
    ON_BN_CLICKED(IDC_BUTTON_SEARCH, OnSearchForFluxLogFiles)
END_MESSAGE_MAP()


BOOL CSummarizeFluxDataDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    Common common;

    CRect rect;
    m_graphFrame.GetWindowRect(rect);
    int height = rect.bottom - rect.top;
    int width = rect.right - rect.left;
    rect.top = 20; rect.bottom = height - 10;
    rect.left = 10; rect.right = width - 10;

    m_graph.Create(WS_VISIBLE | WS_CHILD, rect, &m_graphFrame);
    m_graph.SetXUnits(common.GetString(AXIS_TIMEOFDAY));
    m_graph.SetYUnits(common.GetString(AXIS_FLUX));
    m_graph.SetBackgroundColor(RGB(0, 0, 0));
    m_graph.SetPlotColor(RGB(255, 0, 0));
    m_graph.SetGridColor(RGB(255, 255, 255));
    m_graph.SetRange(0, 86399, 0, 0, 100, 0);

    // Set the directory to search in...
    SetDlgItemText(IDC_EDIT_DIRECTORY, g_settings.outputDirectory);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

/** Called when the user wants to make a search for files */
void CSummarizeFluxDataDlg::OnSearchForFluxLogFiles()
{
    CString directory, message;

    // Reset the old data
    m_fluxLogFiles.RemoveAll();

    // Get the directory to search in...
    GetDlgItemText(IDC_EDIT_DIRECTORY, directory);

    // If the directory is empty, then search in the output-directory
    if (directory.GetLength() < 4) {
        directory.Format("%sOutput", (LPCSTR)g_settings.outputDirectory);
    }

    // Search for files in that directory...
    SearchForFluxLogFiles(directory);

    // Tell the user how many files we've found
    message.Format("%d flux-files found", m_fluxLogFiles.GetCount());
    MessageBox(message);

    // Read in the data from the files
    ReadFluxLogFiles();
}

/** Searches for flux-log files in the given sub-directory.
        Results are appended to the 'm_fluxLogfiles' list */
void CSummarizeFluxDataDlg::SearchForFluxLogFiles(const CString& directory) {
    WIN32_FIND_DATA FindFileData;
    char fileToFind[MAX_PATH];
    CString fileName, fullFileName;

    /** Go through the filenames */
    if (m_includeSubDirectories)
        sprintf(fileToFind, "%s\\*", (LPCSTR)directory);
    else {
        if (m_lookForPostFluxLogs)
            sprintf(fileToFind, "%s\\PostFluxLog*.txt", (LPCSTR)directory);
        else
            sprintf(fileToFind, "%s\\FluxLog*.txt", (LPCSTR)directory);
    }

    // Search for files
    HANDLE hFile = FindFirstFile(fileToFind, &FindFileData);

    if (hFile == INVALID_HANDLE_VALUE) {
        return; // no files found
    }

    do {
        fullFileName.Format("%s\\%s", (LPCSTR)directory, (LPCSTR)FindFileData.cFileName);
        fileName.Format("%s", FindFileData.cFileName);

        // don't include the current and the parent directories
        if (fileName.GetLength() == 2 && Equals(fileName, ".."))
            continue;
        if (fileName.GetLength() == 1 && Equals(fileName, "."))
            continue;

        // 1. Is this a directory?
        if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            // This is a directory. If we are to search sub-directories then go into the directory
            if (m_includeSubDirectories) {
                SearchForFluxLogFiles(fullFileName);
                continue;
            }
        }

        // 2. Is this what we're looking for?
        if (!Equals(fileName.Right(4), ".txt"))
            continue; // <-- filename has to end in '.txt'

        // 3. Ok, the file-name ends in .txt, but does it start with what we want it to do?
        if (m_lookForPostFluxLogs && Equals(fileName.Left(11), "PostFluxLog")) {
            m_fluxLogFiles.AddTail(fullFileName);
        }
        else if (Equals(fileName.Left(7), "FluxLog")) {
            m_fluxLogFiles.AddTail(fullFileName);
        }

    } while (0 != FindNextFile(hFile, &FindFileData));

    FindClose(hFile);
}

/** Reads in the data in the files 'm_fluxLogFiles' into the data-list 'm_fluxResults' */
void CSummarizeFluxDataDlg::ReadFluxLogFiles() {
    FileHandler::CFluxLogFileHandler* reader = NULL;

    POSITION pos = m_fluxLogFiles.GetHeadPosition();
    while (pos != NULL) {
        // get the next file-name in the list
        CString& fileName = m_fluxLogFiles.GetNext(pos);

        // make a new reader
        reader = new FileHandler::CFluxLogFileHandler();
        reader->m_fluxLog.Format(fileName);

        // Parse the file
        if (SUCCESS != reader->ReadFluxLog())
            continue; // <-- could not parse the file

        // Import the data from the reader
        for (int k = 0; k < reader->m_fluxesNum; ++k) {
            m_fluxResults.AddTail(reader->m_fluxes.GetAt(k));
        }
    }

}


