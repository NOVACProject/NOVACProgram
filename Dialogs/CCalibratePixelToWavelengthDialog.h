#pragma once
#include "afxdlgs.h"

#include "../Graphs/GraphCtrl.h"
#include "../NovacProgramLog.h"
#include <string>

// CCalibratePixelToWavelengthDialog dialog

class NovacProgramWavelengthCalibrationController;

namespace novac
{
    class StandardCrossSectionSetup;
}

/// <summary>
/// This is a helper structure for keeping togheter the options for the pixel-to-wavelength calibration
/// as well as for helping persisting these settings to disk.
/// </summary>
struct CalibratePixelToWavelengthDialogSetup
{
public:
    CalibratePixelToWavelengthDialogSetup() :
        m_calibrationOption(0),
        m_initialCalibrationFile(""),
        m_instrumentLineshapeFile(""),
        m_solarSpectrumFile(""),
        m_fitInstrumentLineShapeOption(0),
        m_fitInstrumentLineShapeRegionStart("320"),
        m_fitInstrumentLineShapeRegionStop("350"),
        m_fitInstrumentLineShapeOzoneReference("")
    {
    }

    int m_calibrationOption;            //< the type of instrument calibration file to use (.std or .clb + .slf)
    CString m_initialCalibrationFile;   //< path to the intial calibration file (either .std or .clb)
    CString m_instrumentLineshapeFile;  //< path to the initial instrument line shape file (.slf). Ususally not set if m_initialCalibrationFile is .std.
    CString m_solarSpectrumFile;        //< path to the high-res solar spectrum file
    int m_fitInstrumentLineShapeOption; //< the option for what type of instrument line shape to fit.
    CString m_fitInstrumentLineShapeRegionStart;
    CString m_fitInstrumentLineShapeRegionStop;
    CString m_fitInstrumentLineShapeOzoneReference; //< an optional ozone reference file which can be included into the instrument line shape fit routine.
    std::string m_spectrometerModelName; // A user-specified spectrometer model name. Empty if the user hasn't specified this.
};

class CCalibratePixelToWavelengthDialog : public CPropertyPage
{
    DECLARE_DYNAMIC(CCalibratePixelToWavelengthDialog)

public:
    CCalibratePixelToWavelengthDialog(CWnd* pParent = nullptr);   // standard constructor
    virtual ~CCalibratePixelToWavelengthDialog();

    /** Initializes the controls and the dialog */
    virtual BOOL OnInitDialog();

    // Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_CALIBRATE_WAVELENGTH_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()
public:

    CString m_inputSpectrumFile;

    CalibratePixelToWavelengthDialogSetup m_setup;

    CButton m_runButton;
    CButton m_saveCalibrationButton;
    CButton m_saveReferencesButton;

    Graph::CGraphCtrl m_graph; // The plot where we can display the final calibration
    CStatic m_graphHolder; // holder for the graph, for easy ui access

    CListBox m_graphTypeList; // Selecting the type of plot

    CListBox m_detailedResultList; // detailed presentation of the results

    CButton m_viewLogButton;

    CStatic m_greenLegendIcon;
    CStatic m_greenLegendLabel;
    CStatic m_redLegendIcon;
    CStatic m_redLegendLabel;

    afx_msg void OnClickedButtonBrowseSpectrum();
    afx_msg void OnClickedButtonRun();
    afx_msg void OnClickedButtonSaveCalibration();
    afx_msg void OnClickedButtonSaveReferences();
    afx_msg void OnBnClickedButtonViewLog();
    afx_msg void OnSelchangeListGraphType();
    afx_msg void OnBnClickedSetupWavelengthCalibration();

    afx_msg LRESULT OnCalibrationDone(WPARAM wParam, LPARAM lParam);

private:
    std::string SetupFilePath();
    void SaveSetup();
    void LoadLastSetup();
    void LoadDefaultSetup();

    void UpdateGraph();

    void UpdateResultList();

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
    /// Draws the original and fitted instrument line shape.
    /// </summary>
    void DrawFittedInstrumentLineShape();

    /// <summary>
    /// Draws the fraunhofer + measured spectra + keypoints
    /// </summary>
    void DrawSpectraAndInliers();

    void UpdateGreenLegend(bool show, const char* message = nullptr);
    void UpdateRedLegend(bool show, const char* message = nullptr);

    void HandleCalibrationFailure(const char* errorMessage);

    NovacProgramLog m_log;

    NovacProgramWavelengthCalibrationController* m_controller;

    novac::StandardCrossSectionSetup* m_standardCrossSections;

    char* m_initialCalibrationFileTypeFilter = nullptr;

};
