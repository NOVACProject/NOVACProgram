#pragma once

// CCreateStandardReferencesDialog dialog

#include <string>

namespace novac
{
    class StandardCrossSectionSetup;
}

class CCreateStandardReferencesDialog : public CDialog
{
    DECLARE_DYNAMIC(CCreateStandardReferencesDialog)

public:
    CCreateStandardReferencesDialog(CWnd* pParent = nullptr);   // standard constructor
    virtual ~CCreateStandardReferencesDialog();

    // Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_CALIBRATE_STANDARD_REFERENCES };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()

public:
    // Input from the calling dialog: the set of standard references to save.
    novac::StandardCrossSectionSetup* m_standardCrossSections = nullptr;

    // Input from the calling dialog: infix used to show a filtering has taken place. Default is empty string
    std::string m_fileNameFilteringInfix = "";

    /** This returns true if all the required input fields in this dialog have been setup.  */
    bool IsSetupCorrectly() const;

    // Output from calling this dialog. The full path and filename of each reference
    std::string ReferenceName(size_t referenceIdx, bool includFilteringInfix, bool includeDirectory = true) const;
    std::string FraunhoferReferenceName(bool includeDirectory = true) const;

    // ---- Message handlers ----
    afx_msg void OnClickedButtonBrowseOutputDirectory();
    afx_msg void OnChangeInstrumentName();
    afx_msg void OnChangeFileSuffix();

private:

    CString m_outputdirectory;
    CString m_instrumentName;
    CString m_fileNameSuffix;
    CButton m_okButton;
    CString m_outputFileNamesExplanation;

    void UpdateOutputFileNamesExplanation();

    std::string ReferenceFileName(const std::string& nameOfReference, bool includFilteringInfix, bool includeDirectory = true) const;

};
