// OScopeCtrl.h : header file
//

#ifndef __OScopeCtrl_H__
#define __OScopeCtrl_H__

/////////////////////////////////////////////////////////////////////////////
// COScopeCtrl window

namespace Graph
{
  /** <b>COScopeCtrl</b> is our old class for drawing the output from the program in
      slightly fancy way. */
  class COScopeCtrl : public CWnd
  {

    typedef struct Colors{
      COLORREF  background;
      COLORREF  grid;
      COLORREF  circles;
    }Colors;

  public:
    /** Default constructor */
    COScopeCtrl();

    /** Default destructor */
    virtual ~COScopeCtrl();

    /** Used when the plot is used in oscilloscope-mode. This function will 
        simply append one data point to the plot. */
    double AppendPoint(double dNewPoint);

    /** Sets the range of the X-axis */
    void SetRange(double dLower, double dUpper, int nDecimalPlaces=1);

    /** Sets the unit label of the X-axis */
    void SetXUnits(CString string);

    /** Sets the unit label of the Y-axis */
    void SetYUnits(CString string);

    /** Sets the color of the background grid */
    void SetGridColor(COLORREF color);

    /** Sets the color with which the data is drawn. If the parameter 'clear' is
        set to true then the plot will be cleared, otherwise it will stay. */
    void SetPlotColor(COLORREF color, bool clear = true);

    /** Sets the color of the background */
    void SetBackgroundColor(COLORREF color);

    /** Sets the color for the circles */
    void SetCircleColor(COLORREF color, bool clear = false);

    /** Increases the linewith when drawing lines. */
    void IncreaseLineWidth();

    /** Decreases the linewidht when drawing lines */
    void DecreaseLineWidth();

    /** Marks the whole plot for redrawing */
    void InvalidateCtrl();

    /** ??? */
    void DrawPoint();

    /** ??? */
    void DrawPoint(double* yPositions, int pointSum,int beginX);

    /** ??? */
    void Reset();

    /** Customized function for drawing points with lines connecting them. 
        @param yPositions - the y values for the points.
        @param pointSum - the number of points to draw. 
        @param clear - if 'clear' is true the plot will be cleared before the points are drawn. */
    void DrawMyPoint(double* yPositions, int pointSum, bool clear);

    /** Customized function for drawing points with lines connecting them.
        @param yPositions - the y values for the points.
        @param pointSum - the number of points to draw.
        @param xrange - the largest x-value (the smallest is assumed to be zero)
        @param yrange - the largest y-value (the smallest is assumed to be zero)
        @param clear - if 'clear' is true the plot will be cleared before the points are drawn.*/
    void DrawMyPoint(double* yPositions, int pointSum, double xrange, double yrange, bool clear=true);

    void DrawLine(int type, double value,COLORREF pColor,int lineType);

    /** ??? */
    void DrawCircle();

    /** ?? */
    void DrawCircle(double verticalFactor);

  // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(COScopeCtrl)
    public:
    virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID=NULL);
    //}}AFX_VIRTUAL

    /** */
	  void SetDotLines(int number);

    /** */
    void SetMoreDigits(int pDigits);

    /** */
    void DrawLeftUnit(CString strYMin, CString strYMax, CString strName);

    /** */
	  void DrawCircles(double* x, double* y, double verticalRange, int nSum);

    /** */
    void DrawRects(double* dColumns, double* dDegrees,int nSum, bool clear = true);

    /** */
    void AppendRect(double dNewColumn, double dDegree, int nSum);

    /** */
    void AppendMyPoint(double dNewPoint);

    /** */
	  void AppendCircle(double dNewColumn, double dDegree,double dVerticalRange);

    /** */
    void DrawRect(int nSum);

    /** */
    void CleanPlot();

    /** */
    int  GetZeroNumber(double p);

    /** */
    void SetRange(double dLeft, double dRight, int nXDecimal, double dLower, double dUpper, int nYDecimal);

    /** */
    int m_nShiftPixels;          // amount to shift with each new point 
    int m_nYDecimals;
    int m_nXDecimals;				//---
    CString m_strXUnitsString;
    CString m_strYUnitsString;

    Colors   m_colors;

    COLORREF m_crPlotColor;        // data color  
    int      m_crPlotLineWidth;

    double m_dCurrentPosition;   // current position , y position
    double m_dPreviousPosition;  // previous position

    double m_xPosition;	// x position 

    // Generated message map functions
  protected:
    //{{AFX_MSG(COScopeCtrl)
    afx_msg void OnPaint();
    afx_msg void OnSize(UINT nType, int cx, int cy); 
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

  private:
    /** The number of digits that the scale should be printed with */
	  int digitsFlag;

    /** The unit of the right y-axis */
	  CString m_strRightYUnit;

    /** The unit of the left y-axis */
	  CString m_strYMin;

    /** The unit of the x-axis */
	  CString m_strYMax;

    /** axisFlag is set to 1 if we should draw the right y-axis */
    int axisFlag;
	  int rectWidth;

    /** The current position of the pen (X) */
	  int m_curY;

    /** The current position of the pen (Y) */
	  int m_curX;

    /** The previous position of the pen (X) */
	  int m_prevY;

    /** The previous position of the pen (Y) */
	  int m_prevX;

    int m_nHalfShiftPixels;
    int m_nPlotShiftPixels;
    int m_nClientHeight;
    int m_nClientWidth;
    int m_nPlotHeight;
    int m_nPlotWidth;

    double m_dLowerLimit;        // lower bounds
    double m_dUpperLimit;        // upper bounds
    double m_dRange;
    double m_dVerticalFactor;
    
    double m_dLeftLimit;			//----left bound
    double m_dRightLimit;
    double m_dWRange;
    double m_dHorizonFactor;

    CRect  m_rectClient;
    CRect  m_rectPlot;

    /** The pen for drawing the plot */
    CPen   m_penPlot;
    CPen   m_penPlot2;

    /** The brush for drawing the background */
    CBrush m_brushBack;

    CDC     m_dcGrid;
    CDC     m_dcPlot;
    CBitmap *m_pbitmapOldGrid;
    CBitmap *m_pbitmapOldPlot;
    CBitmap m_bitmapGrid;
    CBitmap m_bitmapPlot;

	  long cnt;
  };
}

/////////////////////////////////////////////////////////////////////////////
#endif
