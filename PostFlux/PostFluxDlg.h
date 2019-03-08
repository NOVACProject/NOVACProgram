#pragma once
#include "afxwin.h"
#include "PostFluxCalculator.h"
#include "../Graphs/ScanGraph.h"
#include "../DlgControls/Label.h"
#include "../Common/WindField.h"
#include "../Common/WindFileReader.h"
#include "../Dialogs/FluxErrorDlg.h"
#include "afxcmn.h"

// CPostFluxDlg dialog

using namespace PostFlux;

class CPostFluxDlg : public CDialog
{
	DECLARE_DYNAMIC(CPostFluxDlg)

	typedef struct Show{
		bool	peakIntensity;
		bool	fitIntensity;
		bool	columnError;
		bool	delta;
		bool	chiSquare;
	}Show;

	typedef struct Colors{
		COLORREF	peakIntensity;
		COLORREF	fitIntensity;
		COLORREF	column;
		COLORREF	badColumn;
		COLORREF	delta;
		COLORREF	chiSquare;
		COLORREF	offset;
	}Colors;
	
	// The possible options for which offset to use
#define OFFSET_CALCULATE 0
#define OFFSET_CALCULATE_PARAM 1
#define OFFSET_USER 2

public:
	CPostFluxDlg(CWnd* pParent = NULL);	 // standard constructor
	virtual ~CPostFluxDlg();

// Dialog Data
	enum { IDD = IDD_POSTFLUX_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);		// DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	// ---------------------- CONSTANTS ---------------------------

	// ------------------ DIALOG COMPONENTS -------------------------

	/** The unit selection box */
	CComboBox m_fluxUnitCombo;

	/** The graph showing the information from the last scan */
	Graph::CScanGraph	m_scanGraph;

	/** The bounding frame for the graph */
	CStatic m_scanGraphFrame;

	/** The spin control, which lets the user select which scan to show. */
	CSpinButtonCtrl m_scanSpinCtrl;

	/** The slider that lets the user select measurement points from intensity */
	CSliderCtrl m_intensitySlider;

	/** The gas combo, let's the user select which gas he wants to calculate the flux
		for. */
	CComboBox m_gasCombo;

	/** The cone-angle combo, lets the user select the cone-angle of the system */
	CComboBox m_coneangleCombo;

	/** The flux-errors */
	Dialogs::CFluxErrorDlg	m_fluxErrorDlg;

	/** The input-boxes for the wind-speed and wind-direction */
	CStatic m_editWindSpeed, m_editWindDirection, m_editPlumeHeight;

	/** The input-box for the compass direction */
	CStatic m_editCompass;
	
	/** The input-box for the tilt */
	CStatic m_editTilt;
	
	/** The text-box with the flux */
	CStatic m_editFlux;
	
	/** The input-box for the selection of bad data points */
	CStatic m_editSelectedPoints;

	// ------------------- DATA -------------------------

	/** The post flux calculator */
	CPostFluxCalculator *m_calculator;

	/** The windfield to use */
	CWindField		m_windField;

	/** The wind-field reader, if any */
	FileHandler::CWindFileReader	*m_wfReader;

	/** The currently selected scan */
	int					 m_curScan;

	/** The currently selected specie (if there are several in the evaluationlog) */
	int					 m_curSpecie;

	/** A list of the datapoints selected for deletion */
	int					 m_selected[MAX_SPEC_PER_SCAN];

	/** The number of data points that are selected for deletion */
	int					 m_selectedNum;

	/** The list of possible cone-angles to use */
	float					m_coneAngles[5];

	/** The number of cone-angles in the list*/
	int						m_coneAngleNum;

	/** True if the intensity is represented as saturation ratios, not intensities. */
	bool					m_useSaturationRatio;

	// ------------------ METHODS ----------------------- 

	/** Initialzing the dialog */
	virtual BOOL OnInitDialog();

	/** Intitializing the plot-legend */
	void	InitLegend();

	/** Called when the user wants to browse for, and open a new evaluation log */
	afx_msg void OnBrowseEvallog();

	/** Called when the user wants to save a copy of the current evaluation log */
	afx_msg void OnSaveEvalLog();

	/** Called when the user wants to browse for a wind-field file */
	afx_msg void OnImportWindField();

	/** Called when the user is ready with changing the settings and wants to
			calculate the flux for the currently selected scan */
	afx_msg void OnCalcFlux();

	/** Called when the user wants to calculate the flux for all scans in this
			evaluation log, which are autmatically judged as being inside the plume. */
	afx_msg void OnCalculateFlux_AllScansInPlume();

	/** Retrieves the wind-speed and -direction to use and fills the values
			into the correct place for calculating the flux */
	void	RetrieveWindField();

	/** Called when the user wants to calculate the wind-direction for the current
			scan using the plume-centre information. */
	afx_msg void OnCalculateWinddirection();

	/** This function removes the datapoints that have been selected */
	afx_msg void OnDeleteSelectedPoints();

	/** This function resets the points which the user has marked for deletion */
	afx_msg void OnResetDeletion();

	/** Called when the user wants to change gas for which the flux calculation is done.
			The gas affects only the conversion from ppmm to mg/m^2. */
	afx_msg void OnChangeGas();

	/** Called when the user changes the cone-angle */
	afx_msg void OnChangeConeAngle();

	/** Draws the currently selected scan */
	void DrawScan();

	/** Initializes the controls of the dialog according to the information in
		the most recently read evaluation log. */
	void InitializeControls();

	/** Updates the list of selected spectra */
	void UpdateSelectionList();

	/** Called when the user releases the intensity slider. 
			Updates the selection list. */
	afx_msg void OnReleaseIntensitySlider(NMHDR *pNMHDR, LRESULT *pResult);

	/** Called when the user presses one of the buttons of the m_scanSpinCtrl,
			leads to the selection of another scan with redrawing of the controls. */
	afx_msg void OnChangeSelectedScan(NMHDR *pNMHDR, LRESULT *pResult);

	/** Called when the user somehow has made a change to the list of data points
			selected for deletion */
	afx_msg void OnChangeSelectionList();

	/** Converts the calculated flux from kg/s to the currently selected unit */
	double	Convert(double flux);

	afx_msg void OnViewPeakIntensity();
	afx_msg void OnViewFitIntensity();
	afx_msg void OnViewColumnError();
	afx_msg void OnViewDelta();
	afx_msg void OnViewChiSquare();

	/** Called when the user has stopped editing the user offset edit-box */
	afx_msg void OnKillFocus_Offset();

	/** Called when the user has changed anything in the user offset edit-box */
	afx_msg void OnChangeEdit_Offset();

	/** Called when the user wants to change the way the offset is calculated*/
	afx_msg void OnChangeOffsetCalculation();

	/** Called when the user wants to see the next scan */
	afx_msg void OnShowPreviousScan();

	/** Called when the user wants to go back one scan */
	afx_msg void OnShowNextScan();

	/** Called when the user has clicked the 'compass-direction' label */
	afx_msg void OnClickedLabelCompassdirection();

	/** Called when the user wants to save the contents of the 
		scan-graph as a bitmap file */
	afx_msg void OnSaveScanGraph_AsBitmap();

	/** Called when the user wants to save the contents of the 
		scan-graph as an ASCII file */
	afx_msg void OnSaveScanGraph_AsASCII();

private:
	/** ?? */
	bool		m_automaticChange;

	/** The options for what to show in the graph */
	Show		m_show;

	/** The options for the colors of the graph */
	Colors	m_color;

	/** The option for how to get the offset, either calculate it
			automatically or get it from the user */
	int	 m_offsetOption;

	/** The user supplied offset */
	double	m_userOffset;

	/** Saves the offset saved in the user offset edit-box */
	void	SaveUserOffset(bool correct = true);
public:
	float m_chi2Limit;
	float m_intensityAbove;
	float m_intensityBelow;

	// The legends
	DlgControls::CLabel m_legendColumn;
	DlgControls::CLabel m_legendPeakIntensity;
	DlgControls::CLabel m_legendFitIntensity;
	DlgControls::CLabel m_legendDelta;
	DlgControls::CLabel m_legendChiSquare;
	CStatic m_labelColumn;
	CStatic m_labelPeakIntensity;
	CStatic m_labelFitIntensity;
	CStatic m_labelDelta;
	CStatic m_labelChiSquare;
	CButton m_calcFluxBtn;
	afx_msg void OnUpdateViewPeakintensity(CCmdUI *pCmdUI);
	afx_msg void OnUpdateViewFitintensity(CCmdUI *pCmdUI);
	afx_msg void OnUpdateViewColumnError(CCmdUI *pCmdUI);
	afx_msg void OnUpdateViewChiSquare(CCmdUI *pCmdUI);
	afx_msg void OnUpdateViewDelta(CCmdUI *pCmdUI);

	/** Initializes the menu somehow, necessary otherwise it is not possible to
			update the status of the menu due to a bug in Microsoft MFC, see:
			http://support.microsoft.com/kb/242577 */
	void OnInitMenuPopup(CMenu *pPopupMenu, UINT nIndex,BOOL bSysMenu);
};
