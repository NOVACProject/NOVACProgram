#pragma once

#include <string>
#include "../Graphs/GraphCtrl.h"

// CCalibrateReferenes dialog

class ReferenceCreationController;

namespace novac
{
    class StandardCrossSectionSetup;
}

class CCalibrateReferencesDialog : public CPropertyPage
{
    DECLARE_DYNAMIC(CCalibrateReferencesDialog)

public:
    CCalibrateReferencesDialog(CWnd* pParent = nullptr);   // standard constructor
    virtual ~CCalibrateReferencesDialog();

    /** Initializes the controls and the dialog */
    virtual BOOL OnInitDialog();

    // Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_CALIBRATE_REFERENCES };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()
public:
    CComboBox m_crossSectionsCombo;

    CString m_calibrationFile;  //< the full calibration file, in the new xml-based format, OR the wavelength calibration file
    CString m_instrumentLineshapeFile; //< the instrument line shape file, provided if m_calibrationFile is a wavelength only file

    BOOL m_highPassFilterReference;
    BOOL m_inputInVacuum;

    CButton m_saveButton;
    CButton m_createStandardReferencesButton;

    Graph::CGraphCtrl m_graph; // The plot for the spectrum
    CStatic m_graphHolder;

    afx_msg void OnConvolutionOptionChanged();
    afx_msg void OnClickedBrowseCrossSection();
    afx_msg void OnClickedButtonRunCreateReference();
    afx_msg void OnClickedButtonSave();
    afx_msg void OnClickedButtonSelectCalibration();
    afx_msg void OnClickedButtonCreateStandardReferences();

private:
    std::string SetupFilePath();
    void SaveSetup();
    void LoadLastSetup();
    void LoadDefaultSetup();

    void UpdateReference();

    void UpdateGraph();

    ReferenceCreationController* m_controller;

    novac::StandardCrossSectionSetup* m_standardCrossSections;
};
