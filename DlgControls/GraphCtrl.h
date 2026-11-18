// OScopeCtrl.h : header file
//

#ifndef __GraphCtrl_H__
#define __GraphCtrl_H__

#include "../Common/Common.h"

/////////////////////////////////////////////////////////////////////////////
// CGraphCtrl window

namespace DlgControls
{
	/** Defining directions */
	const enum DIRECTION {HORIZONTAL, VERTICAL};

	/** Defining line styles */
	const enum LINE_STYLE {STYLE_SOLID, STYLE_DASHED};

	/** Defining number formats */
	const enum NUMBER_FORMAT {FORMAT_GENERAL, FORMAT_TIME, FORMAT_DATE};

	
		/** <b>CGraphCtrl</b> is our class for drawing the output from the program in
      slightly fancy way. */
  class CGraphCtrl : public CWnd
  {
		protected:
			typedef struct Colors{
				COLORREF	plot;
				COLORREF  background;
				COLORREF  grid;
				COLORREF  circles;
			}Colors;

			class CFontOption{
			public:
				CFontOption();
				int				height;
				int				width;
			};

			typedef struct GridOptions{
				typedef struct GridLineOptions{
					double	lowestGridLine;		// The X- (or Y-) value for the lowest grid line
					double	gridSpacing;			// The distance between the grid lines
					bool	active;						// true if the grid is active
					bool	autoScale;				// If true then the scale will be set to match the grid-lines

					inline double Start()		{return lowestGridLine; }
					inline double Spacing()	{return gridSpacing; }
					inline bool	 IsOn()		  {return active; }
				}GridLineOptions;

				GridLineOptions		line[2];
				inline GridLineOptions &X() { return line[0]; }
				inline GridLineOptions &Y() { return line[1]; }

			}GridOptions;

			typedef struct AxisOptions{
				typedef struct FloatRect{
					double right;
					double left;
					double top;
					double bottom;
					double margin;
					NUMBER_FORMAT formatX;	// The number format for the x-axis
					NUMBER_FORMAT formatY;	// The number format for the y-axis
					bool	equal;						// True if the X- and Y- axes should have equal scaling
				}FloatRect;

				FloatRect first;					// The range for the primary axis, shown to the left and down
				FloatRect second;					// The range for the second axis, shown to the right and up

				bool drawRightUnits;			// if'drawRightUnits' is true, then there will be a unit drawn on the right hand side of the plot
				double	minimumRangeX;			// The minimum range for the X-axis
				double	minimumRangeY;			// The minimum range for the Y-axis
				CFontOption		axisFont;
				CFontOption		unitFont;
			}AxisOptions;

			typedef struct PlotOptions{
				int		lineWidth;
				int		circleRadius;
			}PlotOptions;

  public:
		/** Constants used for the options for plotting */
		static const int PLOT_SECOND_AXIS = 0x0001;
		static const int PLOT_CONNECTED		= 0x0002;
		static const int PLOT_CIRCLES			= 0x0004;
		static const int PLOT_FIXED_AXIS  = 0x0008;
		static const int PLOT_BAR_SHOW_X  = 0x0010;

    /** Default constructor */
    CGraphCtrl();

    /** Default destructor */
    virtual ~CGraphCtrl();

		/** Creation override */
    virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID=NULL);

		/** Sets the margin space around the graph */
		void SetMarginSpace(double margin){
			m_axisOptions.first.margin = margin;
		}

		/** Sets the unit label of the X-axis */
    void SetXUnits(CString string);

    /** Sets the unit label of the Y-axis */
    void SetYUnits(CString string);

		/** Sets and enables the unit for the second Y-axis, to the right */
    void SetSecondYUnit(CString string);

		/** Sets the width of the fonts used. Use FW_REGULAR, FW_NORMAL or FW_BOLD. */
		void SetFontWidth(int width){
			m_axisOptions.axisFont.width = width;
			m_axisOptions.unitFont.width = width;
			InvalidateCtrl();
		}

		/** Returns the maximum value of the X-axis */
		double	GetXMax(int dim = 0){
			return (dim == 0) ? m_axisOptions.first.right : m_axisOptions.second.right;
		}

		/** Returns the minimum value of the X-axis */
		double	GetXMin(int dim = 0){
			return (dim == 0) ? m_axisOptions.first.left : m_axisOptions.second.left;
		}
		/** Returns the maximum value of the Y-axis */
		double	GetYMax(int dim = 0){
			return (dim == 0) ? m_axisOptions.first.top : m_axisOptions.second.top;
		}

		/** Returns the minimum value of the Y-axis */
		double	GetYMin(int dim = 0){
			return (dim == 0) ? m_axisOptions.first.bottom : m_axisOptions.second.bottom;
		}

		/** Sets the number format for the X-axis */
		void	SetXAxisNumberFormat(NUMBER_FORMAT format);

		/** Sets the number format for the Y-axis */
		void	SetYAxisNumberFormat(NUMBER_FORMAT format);

		/** Sets the color of the background grid */
    void SetGridColor(COLORREF color);

    /** Sets the color with which the data is drawn. If the parameter 'clear' is
        set to true then the plot will be cleared, otherwise it will stay. */
    void SetPlotColor(COLORREF color, bool redraw = false);

    /** Sets the color of the background */
    void SetBackgroundColor(COLORREF color);

    /** Sets the color for the circles */
    void SetCircleColor(COLORREF color, bool clear = false);

    /** Increases the linewith when drawing lines. */
    void IncreaseLineWidth();

    /** Decreases the linewidth when drawing lines */
    void DecreaseLineWidth();

		/** Sets the width of lines drawn */
		void	SetLineWidth(int width, bool clear = false);

		/** Sets the radius of the circles drawn */
		void	SetCircleRadius(int radius){
			m_plotOptions.circleRadius = radius;
		}
		
		/** Marks the whole plot for redrawing */
    void InvalidateCtrl();

    /** Customized function for drawing points with lines connecting them. 
        @param yPositions - the y values for the points.
        @param pointSum - the number of points to draw. 
				@param plotOption - Specifies the options for drawing the plot. Must be 
					one of, or a combination of, the parameters 'PLOT_...' defined in the 
					CGraphCtrl class. */
		void Plot(double* yPositions, int pointSum, int plotOption = PLOT_CONNECTED){
			return XYPlot(NULL, yPositions, pointSum, plotOption | PLOT_CONNECTED);
		}

		/** Creates an XY-graph 
				@param xPosition - the x-values.
				@param yPosition - the y-values.
				@param pointSum - the number of points that will be drawn.
				if xPositions is NULL then the yPositions will be drawn against their index in the array.
				*/
		void XYPlot(double *xPosition, double *yPosition, long pointSum, int plotOption = PLOT_CONNECTED){
			XYPlot(xPosition, yPosition, NULL, pointSum, plotOption);
		}

		/** Creates an XY-graph with a customizable color of the plot. 
				@param xPosition - the x-values.
				@param yPosition - the y-values.
				@param color - the color value, the lowest value in this set will be blue and the highest red.
					If this is null then the common plot color will be used.
				@param pointSum - the number of points that will be drawn.
				if xPositions is NULL then the yPositions will be drawn against their index in the array.
				*/
		void XYPlot(double *xPosition, double *yPosition, double *color, long pointSum, int plotOption = PLOT_CONNECTED);

		/** Draws a horizontal or vertical solid line across the whole plot area.
				@param direction - the orientation of the line. 0 means horizontal line, 1 means vertical line
				@param value - the logical value at which the line will be drawn. */
		void DrawLine(DIRECTION direction, double value){
			return DrawLine(direction, value, m_colors.plot, STYLE_SOLID);
		}

		/** Draws a horizontal or vertical line across the whole plot area.
				@param direction - the orientation of the line. 0 means horizontal line, 1 means vertical line
				@param value - the logical value at which the line will be drawn.
				@param pColor -  the color of the line
				@param lineType - the style of the line. 0 means dashed line, 1 means solid line.
				@param plotOption - the options for how to plot the line */
    void DrawLine(DIRECTION direction, double value, COLORREF pColor, LINE_STYLE lineStyle = STYLE_SOLID, int plotOption = PLOT_FIXED_AXIS);

    /** Draws the data set as a set of small circles. The plotOptions decides the 
				look of the final plot. plotOptions must be one of, or a combination of, 
				the parameters 'PLOT_...' defined in the CGraphCtrl class.*/
		void DrawCircles(double* x, double* y, int nSum, int plotOption = PLOT_CIRCLES){
			return XYPlot(x, y, nSum, plotOption | PLOT_CIRCLES);
		}

    /** Draws the data set as a set of small circles. The plotOptions decides the 
				look of the final plot. plotOptions must be one of, or a combination of, 
				the parameters 'PLOT_...' defined in the CGraphCtrl class.*/
		void DrawCircles(double* x, double* y, double *color, int nSum, int plotOption = PLOT_CIRCLES){
			return XYPlot(x, y, color, nSum, plotOption | PLOT_CIRCLES);
		}
		
		/** Prints out a string onto the plot.
				The function generates a text-box into which the string will be 
					drawn centered using default font. 		*/
		void DrawTextBox(double x_min, double x_max, double y_min, double y_max, CString &str);

		/** Creates a bar chart, 
				@param xPositions - the x-positions for each bar.
				@param yValues - the height of each bar
				@param pointSum - the total number of points to draw.
				@param plotOptions - the options for the plot. Must be PLOT_SECOND_AXIS or PLOT_FIXED_AXIS.
				If xPositions is NULL then the bars will be drawn on 0, 1, 2, ...*/
    void BarChart(double *xPositions, double *yValues, int pointSum, int plotOption = 0);

    /** Draws a vectorfield. The vector field is defined at the points
        'x' and 'y'. The x-, and y-component of each vector is defined 
        in 'u', and 'v' respectively.
        @param x - The x-coordinates where the vectorfield is defined
        @param y - The y-coordinates where the vectorfield is defined
        @param u - The x-component of the vector field
        @param v - The y-component of the vector field        */
    void DrawVectorField(double *start_x, double *start_y, double *x_comp, double *y_comp, int nSum);

    /** Draws a small vector */
    void DrawVector(double start_x, double start_y, double x_comp, double y_comp, bool invalidate = true);

    /** Clears the plot completely. */
    void CleanPlot();

		/** Enables/disables the grid lines on the X-axis */
		void EnableGridLinesX(bool enable = true){
			this->m_gridOptions.X().active = enable;
		}

		/** Enables/disables the grid lines on the Y-axis */
		void EnableGridLinesY(bool enable = true){
			this->m_gridOptions.Y().active = enable;
		}

		/** Sets the scaling of the x- and y-axes so that the range covered by both axes is the same */
		inline void SetAxisEqual(){
			m_axisOptions.first.equal = true;
		}

		/** Sets the range of both the X and Y-axes*/
		void SetRange(double dLeft, double dRight, int nXDecimal, double dLower, double dUpper, int nYDecimal);

		/** Sets the range of both the second X and second Y-axes*/
		void SetSecondRange(double dLeft, double dRight, int nXDecimal, double dLower, double dUpper, int nYDecimal);

		/** Sets the minimum range of the X-axis */
		void SetMinimumRangeX(double width){
			this->m_axisOptions.minimumRangeX = width;
		}

		/** Sets the minimum range of the Y-axis */
		void SetMinimumRangeY(double height){
			this->m_axisOptions.minimumRangeY = height;
		}

		/** Sets the range of the X-axis */
		void SetRangeX(double left, double right, int decimals, bool invalidate = true);

		/** Sets the range of the Y-axis */
		void SetRangeY(double lower, double upper, int decimals, bool invalidate = true);

		/** Sets the range for the second Y-axis */
		void SetSecondRangeY(double lower, double upper, int decimals, bool invalidate = true);

		/** Sets the range for the second X-axis */
		void SetSecondRangeX(double left, double right, int decimals, bool invalidate = true);

		/** Saves the current graph in a file using the supplied file-name */
		int SaveGraph(const CString &fileName);

	protected:
    /** The number of decimals on the Y-axis */
    int m_nYDecimals;

		/** The number of decimals on the X-axis */
    int m_nXDecimals;

		/** The number of decimals on the second y-axis */
		int	m_nY2Decimals;

		/** The number of decimals on the second x-axis */
		int m_nX2Decimals;

		/** The unit of the x-axis */
		CString m_strXUnitsString;

		/** The unit of the y-axis */
    CString m_strYUnitsString;

		/** The options for the colors to use when drawing the plot */
    Colors			m_colors;

		/** The options for how to draw the grid */
		GridOptions			m_gridOptions;

		/** The options for how to draw the axis */
		AxisOptions			m_axisOptions;

		/** The options for the thickness of the lines/circles plotted */
		PlotOptions			m_plotOptions;

		/** Called to repaint the graph, essentially only copies the bitmaps to the screen */
    afx_msg void OnPaint();

		/** Called when the size of the graph has been changed */
    afx_msg void OnSize(UINT nType, int cx, int cy); 

    DECLARE_MESSAGE_MAP()

    /** The unit of the right y-axis */
	  CString m_strRightYUnit;

    /** The unit of the left y-axis */
	  CString m_strYMin;

    /** The unit of the x-axis */
	  CString m_strYMax;

		/** The bounding box for the client area (the total plot) */
    CRect  m_rectClient;

		/** The height of the client area (the total plot) */
    int m_nClientHeight;

		/** The width of the client area (the total plot) */
    int m_nClientWidth;

		/** The bounding box for the plot area (the area inside the axes) */
    CRect  m_rectPlot;

		/** The height of the plot area (the area inside the axes) */
		int m_nPlotHeight;

		/** The width of the plot area (the area inside the axes) */
		int m_nPlotWidth;

		/** The vertical scaling factor */
    double m_dVerticalFactor;
    double m_dHorizonFactor;


    /** The pen for drawing the plot */
    CPen   m_penPlot;

    /** The brush for drawing the background */
    CBrush m_brushBack;

    CDC     m_dcGrid;
    CDC     m_dcPlot;
    CBitmap *m_pbitmapOldGrid;
    CBitmap *m_pbitmapOldPlot;
    CBitmap m_bitmapGrid;
    CBitmap m_bitmapPlot;

		/** Sets the grid spacing for the desired dimension, given that the
				output on the plot is supposed to be plotted as a general number */
		void SetGridSpacing(double &left, double &right, int dim, NUMBER_FORMAT format);

		/** Sets the grid spacing for the desired dimension, given that the
				output on the plot is supposed to be plotted as a general number */
		void SetGridSpacing(float &left, float &right, int dim, NUMBER_FORMAT format);

		/** Sets the grid spacing for the X-Axis using the given limits */
		void SetGridSpacingX(double &left, double &right){
			SetGridSpacing(left, right, 0, m_axisOptions.first.formatX);
		}

		/** Sets the grid spacing for the X-Axis using the given limits */
		void SetGridSpacingX(float &left, float &right){
			SetGridSpacing(left, right, 0, m_axisOptions.first.formatX);
		}

		/** Sets the grid spacing for the Y-Axis using the given limits */
		void SetGridSpacingY(double &lower, double &upper){
			SetGridSpacing(lower, upper, 1, m_axisOptions.first.formatY);
		}

		/** Sets the grid spacing for the Y-Axis using the given limits */
		void SetGridSpacingY(float &lower, float &upper){
			SetGridSpacing(lower, upper, 1, m_axisOptions.first.formatY);
		}

		/** Prepares for plotting */
		template <class T> void PreparePlot(T *xValues, T *yValues, int pointSum,
			T &maxX, T &minX, T &maxY, T &minY){
			GetDataRange(xValues, yValues, pointSum, maxX, minX, maxY, minY);

			// Set the range for the plot
			SetRange(minX, maxX, m_nXDecimals, minY, maxY, m_nYDecimals);
		}

		/** Prepares for plotting on the secondary axis */
		template <class T> void GetDataRange(T *xValues, T *yValues, int pointSum,
			T &maxX, T &minX, T &maxY, T &minY){
			Common common;
			if(xValues == NULL){
				maxX = pointSum;
				minX = 0;
			}else{
				maxX = Max(xValues, pointSum);
				minX = Min(xValues, pointSum);
			}
			minY = Min(yValues, pointSum);
			maxY = Max(yValues, pointSum);
		}

		/** Finishes the plotting */
		inline void FinishPlot(){
			// Changed 2006-06-05
			m_dcPlot.BitBlt(0, 0, m_nClientWidth, m_rectPlot.top+1, &m_dcGrid, 0, 0, SRCCOPY);
			m_dcPlot.BitBlt(0, 0, m_rectPlot.left+1, m_nClientHeight, &m_dcGrid, 0, 0, SRCCOPY);
			m_dcPlot.BitBlt(m_rectPlot.right, m_rectPlot.top, (m_nClientWidth-m_rectPlot.right), m_nPlotHeight, &m_dcGrid, m_rectPlot.right, m_rectPlot.top, SRCCOPY);
			m_dcPlot.BitBlt(0, m_rectPlot.bottom, m_nClientWidth, (m_nClientHeight - m_rectPlot.bottom), &m_dcGrid, 0, m_rectPlot.bottom, SRCCOPY);
		}

		/** Creates the transform which converts from data point to screen - coordinates */
		template <class T> void GetTransform(T &leftOffset, T &bottomOffset, T &xScaling, T &yScaling, AxisOptions::FloatRect &coordinate){
			leftOffset		= coordinate.left;
			bottomOffset	= coordinate.bottom;
			xScaling			= m_nPlotWidth / (coordinate.right - coordinate.left);
			yScaling			= m_nPlotHeight/ (coordinate.top	 - coordinate.bottom);
		}

		/** Pretty prints the number into the supplied string. Used for the scaling of the axes */
		void	PrintNumber(double number, int nDecimals, NUMBER_FORMAT format, CString &str);
  };
}

/////////////////////////////////////////////////////////////////////////////
#endif
