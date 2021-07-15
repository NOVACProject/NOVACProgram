#pragma once

#include "../Graphs/GraphCtrl.h"

// CCalibratePixelToWavelengthDialog dialog

class WavelengthCalibrationController;

class CCalibratePixelToWavelengthDialog : public CPropertyPage
{
    DECLARE_DYNAMIC(CCalibratePixelToWavelengthDialog)

public:
    CCalibratePixelToWavelengthDialog(CWnd* pParent = nullptr);   // standard constructor
    virtual ~CCalibratePixelToWavelengthDialog();

    /** Initializes the controls and the dialog */
    virtual BOOL OnInitDialog();

    struct CalibratePixelToWavelengthDialogSetup
    {
    public:
        CalibratePixelToWavelengthDialogSetup() :
            m_initialCalibrationFile(""),
            m_instrumentLineshapeFile(""),
            m_solarSpectrumFile(""),
            m_calibrationOption(0)
        {
        }

        CString m_initialCalibrationFile;
        CString m_instrumentLineshapeFile;
        CString m_solarSpectrumFile;
        int m_calibrationOption;
    };

    // Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_CALIBRATE_WAVELENGTH_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()
public:

    CString m_inputSpectrumFile;
    CString m_darkSpectrumFile;

    CalibratePixelToWavelengthDialogSetup m_setup;

    CButton m_runButton;
    CButton m_saveButton;

    Graph::CGraphCtrl m_graph; // The plot where we can display the final calibration
    CStatic m_graphHolder; // holder for the graph, for easy ui access

    CListBox m_graphTypeList; // Selecting the type of plot

    CStatic m_wavelengthCalibrationLabel;
 
    afx_msg void OnClickedButtonBrowseSpectrum();
    afx_msg void OnClickedButtonBrowseSolarSpectrum();
    afx_msg void OnClickedButtonRun();
    afx_msg void OnClickedButtonBrowseSpectrumDark();
    afx_msg void OnClickedButtonSave();
    afx_msg void OnSelchangeListGraphType();
    afx_msg void OnButtonSelectInitialCalibration();

    afx_msg LRESULT OnCalibrationDone(WPARAM wParam, LPARAM lParam);

private:
    std::string SetupFilePath();
    void SaveSetup();
    void LoadLastSetup();
    void LoadDefaultSetup();

    void UpdateGraph();

    /// <summary>
    /// Updates the graph with inliers / outliers of the correspondences + the fitted polynomial
    /// </summary>
    void DrawPolynomialAndInliers();

    /// <summary>
    /// Draws the measured spectrum + keypoints
    /// </summary>
    void DrawMeasuredSpectrumAndKeypoints();

    /// <summary>
    /// Draws the fraunhofer spectrum + keypoints
    /// </summary>
    void DrawFraunhoferSpectrumAndKeypoints();

    /// <summary>
    /// Draws the fraunhofer + measured spectra + keypoints
    /// </summary>
    void DrawSpectraAndInliers();

    void HandleCalibrationFailure(const char* errorMessage);

    WavelengthCalibrationController* m_controller;

    char* m_initialCalibrationFileTypeFilter = nullptr;
};
