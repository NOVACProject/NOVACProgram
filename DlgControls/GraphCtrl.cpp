#include "stdafx.h"
#include "math.h"
#include <atlimage.h>

#include "GraphCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__ ;
#endif

using namespace DlgControls;

CGraphCtrl::CFontOption::CFontOption(){
	this->height = 14;
	this->width = FW_NORMAL;
}

/////////////////////////////////////////////////////////////////////////////
// CGraphCtrl
CGraphCtrl::CGraphCtrl()
{
  // public variable for the number of decimal places on the axis
  m_nYDecimals = 3 ;
  m_nXDecimals = 1 ;
	m_nY2Decimals = 0;
	m_nX2Decimals = 0;

	// The miminum values for the range
	m_axisOptions.minimumRangeX		= 0;
	m_axisOptions.minimumRangeY		= 0;

	// set some initial values for the scaling until "SetRange" is called.
  // these are protected varaibles and must be set with SetRange
	m_axisOptions.first.left			= -95.0f;
	m_axisOptions.first.right			= 95.0f;
	m_axisOptions.first.bottom		= -10.0f;
	m_axisOptions.first.top				= 10.0f;
	m_axisOptions.first.formatX		= FORMAT_GENERAL;
	m_axisOptions.first.formatY		= FORMAT_GENERAL;
	m_axisOptions.first.equal			= false;
	m_axisOptions.first.margin		= 0.0f;

	m_axisOptions.drawRightUnits	= false;
	m_axisOptions.second.left			= -95.0f;
	m_axisOptions.second.right		= 95.0f;
	m_axisOptions.second.bottom		= -10.0f;
	m_axisOptions.second.top			= 10.0f;
	m_axisOptions.second.formatX	= FORMAT_GENERAL;
	m_axisOptions.second.formatY	= FORMAT_GENERAL;
	m_axisOptions.second.equal		= false;
	m_axisOptions.second.margin		= 0.0f;

	// background, grid and data colors
  // these are public variables and can be set directly
  m_colors.background = RGB(0, 0, 0);
  m_colors.grid       = RGB(0, 255, 255);
  m_colors.circles    = RGB(255, 255, 255);
	m_colors.plot       = RGB(255, 255, 255) ;  // see also SetPlotColor

	// The options for plotting
	m_plotOptions.circleRadius	= 4;
	m_plotOptions.lineWidth			= 1;

  // protected variables
	m_penPlot.CreatePen(PS_SOLID, m_plotOptions.lineWidth, m_colors.plot) ;
  m_brushBack.CreateSolidBrush(m_colors.background) ;

  // public member variables, can be set directly 
  m_strXUnitsString.Format("Samples") ;  // can also be set with SetXUnits
  m_strYUnitsString.Format("Y units") ;  // can also be set with SetYUnits

  // protected bitmaps to restore the memory DC's
  m_pbitmapOldGrid = NULL ;
  m_pbitmapOldPlot = NULL ;

	// The options for the grid lines
	m_gridOptions.Y().gridSpacing			= 5;
	m_gridOptions.Y().lowestGridLine	= -5;
	m_gridOptions.Y().active					= true;
	m_gridOptions.Y().autoScale				= true;

	m_gridOptions.X().gridSpacing			= 10;
	m_gridOptions.X().lowestGridLine	= 0;
	m_gridOptions.X().active					= false;
	m_gridOptions.X().autoScale				= false;

}  // CGraphCtrl

/////////////////////////////////////////////////////////////////////////////
CGraphCtrl::~CGraphCtrl()
{
  // just to be picky restore the bitmaps for the two memory dc's
  // (these dc's are being destroyed so there shouldn't be any leaks)
  if (m_pbitmapOldGrid != NULL)
    m_dcGrid.SelectObject(m_pbitmapOldGrid) ;  
  if (m_pbitmapOldPlot != NULL)
    m_dcPlot.SelectObject(m_pbitmapOldPlot) ;  

} // ~CGraphCtrl


BEGIN_MESSAGE_MAP(CGraphCtrl, CWnd)
  ON_WM_PAINT()
  ON_WM_SIZE()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CGraphCtrl message handlers

/////////////////////////////////////////////////////////////////////////////
BOOL CGraphCtrl::Create(DWORD dwStyle, const RECT& rect, 
                         CWnd* pParentWnd, UINT nID) 
{
  BOOL result ;
  static CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW) ;

  result = CWnd::CreateEx(WS_EX_CLIENTEDGE | WS_EX_STATICEDGE, 
                          className, NULL, dwStyle, 
                          rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top,
                          pParentWnd->GetSafeHwnd(), (HMENU)nID) ;
  if (result != 0)
    InvalidateCtrl() ;

  return result ;

} // Create


/////////////////////////////////////////////////////////////////////////////
void CGraphCtrl::SetXUnits(CString string)
{
  m_strXUnitsString = string ;
}  // SetXUnits

/////////////////////////////////////////////////////////////////////////////
void CGraphCtrl::SetYUnits(CString string)
{
  m_strYUnitsString = string ;
}  // SetYUnits


/** Sets the number format for the X-axis */
void	CGraphCtrl::SetXAxisNumberFormat(NUMBER_FORMAT format){
	m_axisOptions.first.formatX = format;
	double left		= m_axisOptions.first.left;
	double right	= m_axisOptions.first.right;
	SetGridSpacingX(left, right);
	m_axisOptions.first.left = (float)left;
	m_axisOptions.first.right = (float)right;
	InvalidateCtrl();
}

/** Sets the number format for the Y-axis */
void	CGraphCtrl::SetYAxisNumberFormat(NUMBER_FORMAT format){
	m_axisOptions.first.formatY = format;
	double top		= m_axisOptions.first.top;
	double bottom	= m_axisOptions.first.bottom;
	SetGridSpacingY(bottom, top);
	m_axisOptions.first.top			=	(float)top;
	m_axisOptions.first.bottom	= (float)bottom;
	InvalidateCtrl();
}


/////////////////////////////////////////////////////////////////////////////
void CGraphCtrl::SetGridColor(COLORREF color)
{
  m_colors.grid = color;

  // clear out the existing garbage, re-start with a clean plot
  InvalidateCtrl() ;

}  // SetGridColor


/////////////////////////////////////////////////////////////////////////////
void CGraphCtrl::SetPlotColor(COLORREF color, bool redraw)
{
  m_colors.plot = color ;

  m_penPlot.DeleteObject() ;
	m_penPlot.CreatePen(PS_SOLID, m_plotOptions.lineWidth, m_colors.plot) ;

  // clear out the existing garbage, re-start with a clean plot
  if(redraw)
    InvalidateCtrl() ;

}  // SetPlotColor


/////////////////////////////////////////////////////////////////////////////
void CGraphCtrl::SetBackgroundColor(COLORREF color)
{
  m_colors.background = color;

  m_brushBack.DeleteObject() ;
  m_brushBack.CreateSolidBrush(m_colors.background) ;

  // clear out the existing garbage, re-start with a clean plot
  InvalidateCtrl() ;

}  // SetBackgroundColor


/** Sets the color for the circles */
void CGraphCtrl::SetCircleColor(COLORREF color, bool clear){
  m_colors.circles = color;

  if(clear)
    InvalidateCtrl();
}

/////////////////////////////////////////////////////////////////////////////
void CGraphCtrl::IncreaseLineWidth(){
  m_penPlot.DeleteObject();
	m_penPlot.CreatePen(PS_SOLID, ++m_plotOptions.lineWidth, m_colors.plot) ;

  // clear out the existing garbage, re-start with a clean plot
  InvalidateCtrl() ;

}  // IncreaseLineWidth

/////////////////////////////////////////////////////////////////////////////
void CGraphCtrl::DecreaseLineWidth(){
	if(m_plotOptions.lineWidth == 0)
    return;

  m_penPlot.DeleteObject();
  m_penPlot.CreatePen(PS_SOLID, --m_plotOptions.lineWidth, m_colors.plot) ;

  // clear out the existing garbage, re-start with a clean plot
  InvalidateCtrl() ;

}  // DecreaseLineWidth

void CGraphCtrl::SetLineWidth(int width, bool clear){
	if(width > 0){
		m_plotOptions.lineWidth = width;
	  m_penPlot.DeleteObject();
		m_penPlot.CreatePen(PS_SOLID, m_plotOptions.lineWidth, m_colors.plot);

	  // clear out the existing garbage, re-start with a clean plot
		if(clear)
			InvalidateCtrl() ;
	}
}

/////////////////////////////////////////////////////////////////////////////
void CGraphCtrl::InvalidateCtrl()
{
  // There is a lot of drawing going on here - particularly in terms of 
  // drawing the grid.  Don't panic, this is all being drawn (only once)
  // to a bitmap.  The result is then BitBlt'd to the control whenever needed.
  int i ;
  size_t nCharacters ;
  CPen *oldPen ;
  CPen solidPen(PS_SOLID, 0, m_colors.grid) ;
  CFont axisFont, yUnitFont, *oldFont ;
  CString strTemp, xAxisStrMin[2], xAxisStrMax[2], yAxisStrMin[2], yAxisStrMax[2];

  // in case we haven't established the memory dc's
  CClientDC dc(this) ;  

  // if we don't have one yet, set up a memory dc for the grid
  if (m_dcGrid.GetSafeHdc() == NULL)
  {
    m_dcGrid.CreateCompatibleDC(&dc) ;
    m_bitmapGrid.CreateCompatibleBitmap(&dc, m_nClientWidth, m_nClientHeight) ;
    m_pbitmapOldGrid = m_dcGrid.SelectObject(&m_bitmapGrid) ;
  }
  
  m_dcGrid.SetBkColor (m_colors.background) ;

	// --------- Print the min-max numbers for the axes -------------
	PrintNumber(m_axisOptions.first.top,		m_nYDecimals, m_axisOptions.first.formatY, yAxisStrMax[0]);
	PrintNumber(m_axisOptions.first.bottom,	m_nYDecimals, m_axisOptions.first.formatY, yAxisStrMin[0]);
	PrintNumber(m_axisOptions.first.right,	m_nYDecimals, m_axisOptions.first.formatX, xAxisStrMax[0]);
	PrintNumber(m_axisOptions.first.left,		m_nYDecimals, m_axisOptions.first.formatX, xAxisStrMin[0]);
	if(m_axisOptions.drawRightUnits){
		PrintNumber(m_axisOptions.second.top,			m_nY2Decimals, m_axisOptions.second.formatY, yAxisStrMax[1]);
		PrintNumber(m_axisOptions.second.bottom,	m_nY2Decimals, m_axisOptions.second.formatY, yAxisStrMin[1]);
	}

  // ---- Clear the grid DC ----
  m_dcGrid.FillRect(m_rectClient, &m_brushBack) ;

  // draw the plot rectangle:
  // determine how wide the y axis scaling values are
	nCharacters = max(strlen(yAxisStrMax[0]), strlen(yAxisStrMin[0]));

  // add an extra space
  nCharacters = nCharacters + 2;

	m_rectPlot.left = m_rectClient.left + 6 * (long)nCharacters;

  // adjust the plot rectangle dimensions
  // assume 6 pixels per character (this may need to be adjusted)
	if(m_axisOptions.drawRightUnits)
  {
		PrintNumber(m_axisOptions.second.top, m_nY2Decimals, m_axisOptions.second.formatY, strTemp);
		nCharacters = strlen(strTemp);
		PrintNumber(m_axisOptions.second.bottom, m_nY2Decimals, m_axisOptions.second.formatY, strTemp);
		nCharacters = 1 + max(nCharacters, strlen(strTemp));

		m_rectPlot.right = m_rectClient.right - 6 * (long)nCharacters;
  }
  else
  {
	  m_rectPlot.right = m_rectClient.right - 30;
  }
  m_nPlotWidth    = m_rectPlot.Width() ;

  // draw the plot rectangle
  oldPen = m_dcGrid.SelectObject (&solidPen) ; 
  m_dcGrid.MoveTo (m_rectPlot.left, m_rectPlot.top) ;
  m_dcGrid.LineTo (m_rectPlot.right+1, m_rectPlot.top) ;
  m_dcGrid.LineTo (m_rectPlot.right+1, m_rectPlot.bottom+1) ;
  m_dcGrid.LineTo (m_rectPlot.left, m_rectPlot.bottom+1) ;
  m_dcGrid.LineTo (m_rectPlot.left, m_rectPlot.top) ;
  m_dcGrid.SelectObject (oldPen) ; 

  // draw the dotted grid lines, 
  // use SetPixel instead of a dotted pen - this allows for a 
  // finer dotted line and a more "technical" look

	// ---- Y axes ---------------
	if(m_gridOptions.Y().IsOn()){
		float step				= (float)(m_gridOptions.Y().Spacing() * m_dVerticalFactor);
		float	ymin				= (float)(m_rectPlot.bottom - (m_gridOptions.Y().Start() - m_axisOptions.first.bottom) * m_dVerticalFactor);
		for(float j = ymin; j > m_rectPlot.top; j -= step){
			for(i = m_rectPlot.left; i < m_rectPlot.right; i+= 4){
				m_dcGrid.SetPixel(i, (int)j, m_colors.grid);
			}
		}
	}
	// ---- X axes ---------------
	if(m_gridOptions.X().IsOn()){
		float step				= (float)(m_gridOptions.X().Spacing() * m_dHorizonFactor);
		float	xmin				= (float)(m_rectPlot.left + (m_gridOptions.X().Start() - m_axisOptions.first.left) * m_dHorizonFactor);
		for(float j = xmin; j < m_rectPlot.right; j += step){
			for(i = m_rectPlot.top; i < m_rectPlot.bottom; i+= 4){
				m_dcGrid.SetPixel((int)j, i, m_colors.grid);
			}
		}
	}

	// ------------- DRAW THE SCALE ON THE AXES -----------------------

  // create some fonts (horizontal and vertical)
  // use a height of 14 pixels and 900 weight 
  // (these may need to be adjusted depending on the display)
	axisFont.CreateFont (m_axisOptions.axisFont.height, 0, 0, 0, m_axisOptions.axisFont.width,
                       FALSE, FALSE, 0, ANSI_CHARSET,
                       OUT_DEFAULT_PRECIS, 
                       CLIP_DEFAULT_PRECIS,
                       DEFAULT_QUALITY, 
                       DEFAULT_PITCH|FF_SWISS, "Arial") ;
	yUnitFont.CreateFont (m_axisOptions.unitFont.height, 0, 900, 0, m_axisOptions.unitFont.width,
                       FALSE, FALSE, 0, ANSI_CHARSET,
                       OUT_DEFAULT_PRECIS, 
                       CLIP_DEFAULT_PRECIS,
                       DEFAULT_QUALITY, 
                       DEFAULT_PITCH|FF_SWISS, "Arial") ;
  
  // grab the horizontal font
  oldFont = m_dcGrid.SelectObject(&axisFont) ;

	// y max   
  m_dcGrid.SetTextColor (m_colors.grid);
  m_dcGrid.SetTextAlign (TA_RIGHT|TA_TOP);
  m_dcGrid.TextOut (m_rectPlot.left-4, m_rectPlot.top, yAxisStrMax[0]) ;

  // y min
  m_dcGrid.SetTextAlign (TA_RIGHT|TA_BASELINE) ;
  m_dcGrid.TextOut (m_rectPlot.left-4, m_rectPlot.bottom, yAxisStrMin[0]) ;

  // x min
  m_dcGrid.SetTextAlign (TA_LEFT|TA_TOP) ;
  m_dcGrid.TextOut (m_rectPlot.left, m_rectPlot.bottom+4, xAxisStrMin[0]) ;

  // x max
  m_dcGrid.SetTextAlign (TA_RIGHT|TA_TOP) ;
  m_dcGrid.TextOut (m_rectPlot.right, m_rectPlot.bottom+4, xAxisStrMax[0]) ;

  // x units
  m_dcGrid.SetTextAlign (TA_CENTER|TA_TOP) ;
  m_dcGrid.TextOut ((m_rectPlot.left+m_rectPlot.right)/2, 
                    m_rectPlot.bottom+4, m_strXUnitsString) ;

  // restore the font
  m_dcGrid.SelectObject(oldFont) ;

  // y units
  oldFont = m_dcGrid.SelectObject(&yUnitFont) ;
  m_dcGrid.SetTextAlign (TA_CENTER|TA_BASELINE) ;
  m_dcGrid.TextOut ((m_rectClient.left+m_rectPlot.left)/2, 
                    (m_rectPlot.bottom+m_rectPlot.top)/2, m_strYUnitsString) ;
  m_dcGrid.SelectObject(oldFont) ;

	// ------- THE SCALE ON THE SECOND Y-AXIS ---------------
  if(m_axisOptions.drawRightUnits)
  {
    oldFont = m_dcGrid.SelectObject(&axisFont);

		// Y max
	  m_dcGrid.SetTextAlign (TA_RIGHT|TA_TOP) ;
		m_dcGrid.TextOut(m_rectPlot.right + 6 * (1 + (int)strlen(yAxisStrMax[1])), m_rectPlot.top , yAxisStrMax[1]);

		// Y min
		m_dcGrid.SetTextAlign (TA_RIGHT|TA_BASELINE) ;
		m_dcGrid.TextOut(m_rectPlot.right + 6 * (1 + (int)strlen(yAxisStrMin[1])), m_rectPlot.bottom , yAxisStrMin[1]);

	  m_dcGrid.SelectObject(oldFont) ;

	  oldFont = m_dcGrid.SelectObject(&yUnitFont) ;
	  m_dcGrid.SetTextAlign (VTA_CENTER|VTA_BASELINE) ;
	  m_dcGrid.TextOut (m_rectClient.right-10, 
                      (m_rectPlot.bottom+m_rectPlot.top)/2, m_strRightYUnit) ;
	  m_dcGrid.SelectObject(oldFont) ;
  }

  // at this point we are done filling the the grid bitmap, 
  // no more drawing to this bitmap is needed until the setting are changed
  
  // if we don't have one yet, set up a memory dc for the plot
  if (m_dcPlot.GetSafeHdc() == NULL)
  {
    m_dcPlot.CreateCompatibleDC(&dc) ;
    m_bitmapPlot.CreateCompatibleBitmap(&dc, m_nClientWidth, m_nClientHeight) ;
    m_pbitmapOldPlot = m_dcPlot.SelectObject(&m_bitmapPlot) ;
  }

	// 2006-06-05: Removed!
  // make sure the plot bitmap is cleared
  //m_dcPlot.SetBkColor (m_colors.background) ;
  //m_dcPlot.FillRect(m_rectClient, &m_brushBack) ;

	// 2006-06-05: Repaint the plot with the grid as background.
	m_dcPlot.BitBlt(0, 0, m_nClientWidth, m_nClientHeight, &m_dcGrid, 0, 0, SRCCOPY) ;

	// finally, force the plot area to redraw
  InvalidateRect(m_rectClient) ;


} // InvalidateCtrl


////////////////////////////////////////////////////////////////////////////
void CGraphCtrl::OnPaint() 
{
  CPaintDC dc(this) ;  // device context for painting
  CDC memDC ;
  CBitmap memBitmap ;
  CBitmap* oldBitmap ; // bitmap originally found in CMemDC

  // no real plotting work is performed here, 
  // just putting the existing bitmaps on the client

  // to avoid flicker, establish a memory dc, draw to it 
  // and then BitBlt it to the client
  memDC.CreateCompatibleDC(&dc) ;
  memBitmap.CreateCompatibleBitmap(&dc, m_nClientWidth, m_nClientHeight) ;
  oldBitmap = (CBitmap *)memDC.SelectObject(&memBitmap) ;

  if (memDC.GetSafeHdc() != NULL)
  {
		// Changed 2006-06-05
    //// first drop the grid on the memory dc
    //memDC.BitBlt(0, 0, m_nClientWidth, m_nClientHeight, 
    //             &m_dcGrid, 0, 0, SRCCOPY) ;
    //// now add the plot on top as a "pattern" via SRCPAINT.
    //// works well with dark background and a light plot
    //memDC.BitBlt(0, 0, m_nClientWidth, m_nClientHeight, 
    //             &m_dcPlot, 0, 0, SRCPAINT) ;  //SRCPAINT

    // 2006-06-05: finally send the result to the display
    dc.BitBlt(0, 0, m_nClientWidth, m_nClientHeight, 
              &m_dcPlot, 0, 0, SRCCOPY) ;
  }

  memDC.SelectObject(oldBitmap) ;

} // OnPaint

void CGraphCtrl::XYPlot(double *xPosition, double *yPosition, double *color, long pointSum, int plotOption){
	CPen *oldPen ;
	int i, r = m_plotOptions.circleRadius;
	float xFactor,yFactor, offsLeft, offsBottom;
	int curX, curY, prevX, prevY;
	float left		= (float)m_rectPlot.left;
	float right		= (float)m_rectPlot.right;
	float top			= (float)m_rectPlot.top;
	float bottom	= (float)m_rectPlot.bottom;
	double maxX, minX, maxY, minY;
	double minC, maxC, halfC, halfC_inv;
	COLORREF curColor = m_colors.circles;
	AxisOptions::FloatRect curAxis; // The current axis (either first or second axis)

	// -- If there's nothing to plot, just return at once.
	if(pointSum == 0)
		return;

	// make sure there's a memory dc for the plot
	if(m_dcPlot.GetSafeHdc() == NULL)
		return;

	// Prepare the plot...
	if((plotOption & PLOT_SECOND_AXIS) || (plotOption & PLOT_FIXED_AXIS)){
		// For plotting on the second axis, or for fixed axes, we don't want to 
		//	change the scale of the axes, it's enough just to know the range of the data
		GetDataRange(xPosition, yPosition, pointSum, maxX, minX, maxY, minY);		
	}else{
		// Get the ranges for the data and scale the axes accordingly
		PreparePlot(xPosition, yPosition, pointSum, maxX, minX, maxY, minY);
	}

	// Prepare the coloring of the plot
	if(color != NULL){
		maxC = Max(color, pointSum);
		minC = Min(color, pointSum);
		halfC = (maxC - minC) / 2;
		halfC_inv = 1.0 / halfC;
	}

	// Get the current axis
	if(plotOption & PLOT_SECOND_AXIS){
		curAxis = m_axisOptions.second;
	}else{
		curAxis = m_axisOptions.first;
	}

	// ------------ CALCULATE THE TRANSFORM FROM DATA POINT TO PIXELS ---------------
	GetTransform(offsLeft, offsBottom, xFactor, yFactor, curAxis);

	// ---------- START PLOTTING ----------------

	// The starting point
	if(xPosition == NULL)
		prevX = (int)left;
	else
		prevX = (int)(left + (xPosition[0] - offsLeft)*xFactor);
	prevY = (int)(bottom - (yPosition[0] - offsBottom) * yFactor);

	// Take the pen to use
	oldPen = m_dcPlot.SelectObject(&m_penPlot) ;

	// Loop through all points in the data set
	for(i = 0; i < pointSum; ++i)
	{
		// Calculate the next point...
		if(xPosition == NULL)
			curX = (int)(left + xFactor * (i - offsLeft));
		else
			curX = (int)(left + xFactor * (xPosition[i] - offsLeft) );
		curY = (int)(bottom - (yPosition[i] - offsBottom) * yFactor);

		// Calculate which color to use...
		if(color != NULL){
			double a;
			if((color[i] - minC) < halfC){
				a = max(0, color[i] - minC) * halfC_inv;
				curColor = RGB(0, 255*a, 255*(1 - a));
			}else{
				a = max(0, color[i] - halfC) * halfC_inv;
				curColor = RGB(255 * a, 255 * (1 - a), 0);
			}
		}

		// Draw connected lines
		if(plotOption & PLOT_CONNECTED){
			m_dcPlot.MoveTo(prevX, prevY);
			m_dcPlot.LineTo(curX, curY);
		}

		// Draw circles
		if(plotOption & PLOT_CIRCLES){
			m_dcPlot.FillSolidRect(curX - r/2, curY - r/2, r, r, curColor);
		}

		prevX = curX;
		prevY = curY;

	}// end for

	// restore the pen 
	m_dcPlot.SelectObject(oldPen);

	
	// if the data leaks over the upper or lower plot boundaries
	// fill the upper and lower leakage with the background
	// this will facilitate clipping on an as needed basis
	// as opposed to always calling IntersectClipRect
	FinishPlot();
}

/** Saves the current graph in a file using the supplied file-name */
int CGraphCtrl::SaveGraph(const CString &fileName){
	CImage image;

	// make sure there's a memory dc for the plot
	if(m_dcPlot.GetSafeHdc() == NULL)
		return 1;

	// Copy the current graph to the image
	image.Attach(HBITMAP(m_bitmapPlot));

	// Save the graph to a file with the supplied filename
	image.Save(fileName);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
void CGraphCtrl::OnSize(UINT nType, int cx, int cy) 
{
  CWnd::OnSize(nType, cx, cy) ;

  // NOTE: OnSize automatically gets called during the setup of the control
  
  GetClientRect(m_rectClient) ;

  // set some member variables to avoid multiple function calls
  m_nClientHeight = m_rectClient.Height() ;
  m_nClientWidth  = m_rectClient.Width() ;

  // the "left" coordinate and "width" will be modified in 
  // InvalidateCtrl to be based on the width of the y axis scaling
  m_rectPlot.left   = 20 ;  
  m_rectPlot.top    = 10 ;
  m_rectPlot.right  = m_rectClient.right-10 ;
  m_rectPlot.bottom = m_rectClient.bottom-25 ;

  // set some member variables to avoid multiple function calls
  m_nPlotHeight			= m_rectPlot.Height() ;
  m_nPlotWidth			= m_rectPlot.Width() ;

  // set the scaling factor for now, this can be adjusted 
  // in the SetRange functions
  m_dVerticalFactor = (double)m_nPlotHeight / (m_axisOptions.first.top - m_axisOptions.first.bottom) ; 
	m_dHorizonFactor	= (double)m_nPlotWidth / (m_axisOptions.first.right - m_axisOptions.first.left);

  // change the memory dc for the grid
  if(m_dcGrid.GetSafeHdc() != NULL){
    CClientDC dc(this);
    m_bitmapGrid.Detach();
    m_bitmapGrid.DeleteObject();
    m_bitmapGrid.CreateCompatibleBitmap(&dc, m_nClientWidth, m_nClientHeight) ;
    m_pbitmapOldGrid = m_dcGrid.SelectObject(&m_bitmapGrid) ;
  }

  // change the memory dc for the plot
  if(m_dcPlot.GetSafeHdc() != NULL){
    CClientDC dc(this);
    m_bitmapPlot.Detach();
    m_bitmapPlot.DeleteObject();
    m_bitmapPlot.CreateCompatibleBitmap(&dc, m_nClientWidth, m_nClientHeight) ;
    m_pbitmapOldPlot = m_dcPlot.SelectObject(&m_bitmapPlot) ;
  }

} // OnSize


/////////////////////////////////////////////////////////////////////////////

void CGraphCtrl::CleanPlot()
{
  InvalidateCtrl();
}

/** Prints out a string onto the plot.
		The function generates a text-box into which the string will be 
			drawn centered using default font. 		*/
void CGraphCtrl::DrawTextBox(double x_min, double x_max, double y_min, double y_max, CString &str){
	CFont tmpFont, *oldFont;
	double xFactor,yFactor, offsLeft, offsBottom;
	

	// Transform the coordinates to pixels
	GetTransform(offsLeft, offsBottom, xFactor, yFactor, m_axisOptions.first);

	// The boundary for the Text-Box
	RECT rect;
	rect.left		= m_rectPlot.left + (x_min - offsLeft) * xFactor;
	rect.right	= m_rectPlot.left + (x_max - offsLeft) * xFactor;
	rect.top		= m_rectPlot.bottom - (y_min - offsBottom) * yFactor;
	rect.bottom = m_rectPlot.bottom - (y_max - offsBottom) * yFactor;

	// The position...
	int X, Y;
	X		= m_rectPlot.left + (x_min - offsLeft) * xFactor;
	Y		= m_rectPlot.bottom - (y_min - offsBottom) * yFactor;

	// Create the font
	tmpFont.CreateFont(m_axisOptions.axisFont.height, 0, 0, 0, m_axisOptions.axisFont.width,
                       FALSE, FALSE, 0, ANSI_CHARSET,
                       OUT_DEFAULT_PRECIS, 
                       CLIP_DEFAULT_PRECIS,
                       DEFAULT_QUALITY, 
                       DEFAULT_PITCH|FF_SWISS, "Arial");

	// grab the new font
	oldFont = m_dcPlot.SelectObject(&tmpFont);

	m_dcPlot.SetTextColor(m_colors.plot);
	m_dcPlot.SetBkColor(m_colors.background);
	m_dcPlot.SetTextAlign(TA_CENTER | TA_BASELINE);
	m_dcPlot.ExtTextOut((rect.right + rect.left) / 2, rect.top, ETO_CLIPPED, &rect, str, NULL);

  // restore the font
  m_dcPlot.SelectObject(oldFont) ;
}

void CGraphCtrl::BarChart(double *xPosition, double *yValues, int pointSum, int plotOption)
{

	CRect rect;
  CString strTemp;
	CFont axisFont,*oldFont;
	AxisOptions::FloatRect curAxis; // The current axis (either first or second axis)
	int topLeftx,topLefty,bottomRightx,bottomRighty;
	double maxX, minX, maxY, minY;
	float xFactor,yFactor, offsLeft, offsBottom;
	Common common;
	int i;

	// -- If there's nothing to plot, just return at once.
	if(pointSum == 0)
		return;

	// make sure there's a memory dc for the plot
	if(m_dcPlot.GetSafeHdc() == NULL)
		return;

  axisFont.CreateFont (10, 0, 0, 0, 300,
                      FALSE, FALSE, 0, ANSI_CHARSET,
                      OUT_DEFAULT_PRECIS, 
                      CLIP_DEFAULT_PRECIS,
                      DEFAULT_QUALITY, 
                      DEFAULT_PITCH|FF_SWISS, "Arial") ;

	// Prepare the plot...
	if((plotOption & PLOT_SECOND_AXIS) || (plotOption & PLOT_FIXED_AXIS)){
		// For plotting on the second axis, or for fixed axes, we don't want to 
		//	change the scale of the axes, it's enough just to know the range of the data
		GetDataRange(xPosition, yValues, pointSum, maxX, minX, maxY, minY);		
	}else{
		// Get the ranges for the data and scale the axes accordingly
		PreparePlot(xPosition, yValues, pointSum, maxX, minX, maxY, minY);
	}

	// Get the current axis
	if(plotOption & PLOT_SECOND_AXIS){
		curAxis = m_axisOptions.second;
	}else{
		curAxis = m_axisOptions.first;
	}

	// ------------ CALCULATE THE TRANSFORM FROM DATA POINT TO PIXELS ---------------
	GetTransform(offsLeft, offsBottom, xFactor, yFactor, curAxis);

	// --- The width of each rectangle ---
	float rectWidth				= (float)fabs((m_rectPlot.right - m_rectPlot.left)*(maxX - minX) / ((curAxis.right - curAxis.left) * pointSum));
	float half_rectWidth	= rectWidth * 0.5f;

  m_dcPlot.SetTextColor(m_colors.grid) ;
	m_dcPlot.SetBkMode(TRANSPARENT); 

	// ---------- START PLOTTING ----------------

	bottomRighty = m_rectPlot.bottom + (long)(offsBottom * yFactor);	//when 0 is in the middle

	for(i = 0; i < pointSum; ++i){
		float X;
		if(xPosition == NULL)
			X = (float)(i - offsLeft) * xFactor;
		else
			X = (float)(xPosition[i] - offsLeft) * xFactor;


		topLeftx				= m_rectPlot.left		+ (int)(X - half_rectWidth);
		topLefty				= m_rectPlot.bottom - (int)(((float)yValues[i] - offsBottom) * yFactor);
		bottomRightx		= m_rectPlot.left		+ (int)(X + half_rectWidth);

		rect = CRect(topLeftx,topLefty,bottomRightx,bottomRighty);

		// Fill the rectangle
		m_dcPlot.FillSolidRect(rect,m_colors.plot);

		// Draw the number below the rectangle
		if(plotOption & PLOT_BAR_SHOW_X && xPosition != NULL){
			strTemp.Format("%.0lf", xPosition[i]);
		}else{
			strTemp.Format("%d", i);
		}
		oldFont = m_dcPlot.SelectObject(&axisFont) ;
		m_dcPlot.TextOut(topLeftx, bottomRighty - 8, strTemp) ;
		m_dcPlot.SelectObject(oldFont);
	}

	FinishPlot();
}

/** Draws a vectorfield. The vector field is defined at the points
    'x' and 'y'. The x-, and y-component of each vector is defined 
    in 'u', and 'v' respectively.
    @param x - The x-coordinates where the vectorfield is defined
    @param y - The y-coordinates where the vectorfield is defined
    @param u - The x-component of the vector field
    @param v - The y-component of the vector field        */
void CGraphCtrl::DrawVectorField(double *start_x, double *start_y, double *x_comp, double *y_comp, int nSum){
  int point;

  // Draw each of the data points
  for(point = 0; point < nSum; ++point){
    DrawVector(start_x[point], start_y[point], x_comp[point], y_comp[point], false);
  }

  Invalidate(FALSE);
}

/** Draws a small vector */
void CGraphCtrl::DrawVector(double start_x, double start_y, double x_comp, double y_comp, bool invalidate){
	double x[2] = {start_x, start_x + x_comp};
	double y[2] = {start_y, start_y + y_comp};

	XYPlot(x, y, 2, PLOT_FIXED_AXIS | PLOT_CONNECTED);
}

void CGraphCtrl::SetSecondYUnit(CString string)
{
	m_strRightYUnit = string;
	m_axisOptions.drawRightUnits = true;
}

/**Draw a line paralell to x-axis or y-axis
*@direction - HORIZONTAL or VERTICAL
*@value - the logic value, not in pixel
*@pColor - define the color of the line
*@lineType - define the line is dash or solid. 0 - dash, 1 - solid
**/
void CGraphCtrl::DrawLine(DIRECTION direction, double value,COLORREF pColor, LINE_STYLE lineStyle, int plotOption)
{
	int x0, x1, y0, y1;
	CPen *oldPen ;
  CPen pen;
	AxisOptions::FloatRect curAxis;
	float offsLeft, offsBottom, xFactor, yFactor;

	if(m_dcPlot.GetSafeHdc() == NULL)
		return;

	m_dcPlot.SetBkColor(m_colors.background);

	// --- The style of the line ---
	switch(lineStyle){
		case STYLE_SOLID:		pen.CreatePen(PS_SOLID, 0, pColor); break;
		case STYLE_DASHED:	pen.CreatePen(PS_DASH,	0, pColor); break;
		default:						pen.CreatePen(PS_SOLID, 0, pColor);
	}	
	oldPen = m_dcPlot.SelectObject(&pen) ;

	// Get the current axis
	if(plotOption & PLOT_SECOND_AXIS){
		curAxis = m_axisOptions.second;
	}else{
		curAxis = m_axisOptions.first;
	}

	// ------------ CALCULATE THE TRANSFORM FROM DATA POINT TO PIXELS ---------------
	GetTransform(offsLeft, offsBottom, xFactor, yFactor, curAxis);

	// --- Draw the line ---
  if(direction == VERTICAL){
		x0			=	m_rectPlot.left + (long)((value - offsLeft)*xFactor) ;
		y0			= m_rectPlot.top;
		x1			= x0;
		y1			= m_rectPlot.bottom;
	}
	if(direction == HORIZONTAL){
		x0			= m_rectPlot.left;
		y0			= m_rectPlot.bottom - (long)((value - offsBottom)*yFactor) ;
		x1			= m_rectPlot.right;
		y1			= y0;
	}
	m_dcPlot.MoveTo(x0, y0);
	m_dcPlot.LineTo(x1, y1);

	m_dcPlot.SelectObject(oldPen) ;

	// --- Clean up ---
	FinishPlot();

	// --- Redraw ---
	Invalidate();
}

void CGraphCtrl::SetGridSpacing(float &lower, float &upper, int dim, NUMBER_FORMAT format){
	double low = lower;
	double up  = upper;
	SetGridSpacing(low, up, dim, format);
	lower = (float)low;
	up = (float)upper;
}

void CGraphCtrl::SetGridSpacing(double &lower, double &upper, int dim, NUMBER_FORMAT format){
	if(m_gridOptions.line[dim].IsOn()){
		double range				= upper - lower;								// The range
		double rangeNorm, magnitude;
		float intervals[50];
		int nIntervals;

		if(format == FORMAT_TIME){
			intervals[0] =14400.0f;
			intervals[1] = 3600.0f;
			intervals[2] = 1800.0f;
			intervals[3] =  300.0f;
			intervals[4] =   60.0f;
			intervals[5] =   30.0f;
			intervals[6] =    1.0f;
			rangeNorm			= range;
			nIntervals		= 7;
			magnitude			= 1.0;
		}
		if(format == FORMAT_GENERAL){
			intervals[0]				=	1.0f;
			intervals[1]				=	0.5f;
			intervals[2]				=	0.2f;
			intervals[3]				= 0.1f;
			magnitude						= pow(10, round(log10(range)));		// The order of magnitude of the range
			double magnitudeInv = 1/magnitude;									// The inverse magnitude
			rangeNorm						= range * magnitudeInv;					// Normalize to the range [0-10]
			nIntervals					= 4;
		}

		for(int i = 0; i < nIntervals; ++i){
			if(rangeNorm > 3 * intervals[i]){
				m_gridOptions.line[dim].gridSpacing = (float)(intervals[i] * magnitude);
				break;
			}	
		}

		m_gridOptions.line[dim].lowestGridLine	= m_gridOptions.line[dim].gridSpacing * (float)floor(lower / m_gridOptions.line[dim].gridSpacing);
		if(m_gridOptions.line[dim].lowestGridLine == lower)
			m_gridOptions.line[dim].lowestGridLine += m_gridOptions.line[dim].Spacing();

		if(m_gridOptions.line[dim].autoScale){
			// make a suggestion of how we would like to change the scaling
			lower		= m_gridOptions.line[dim].lowestGridLine - m_gridOptions.line[dim].gridSpacing;
			upper		= ceil(upper / m_gridOptions.line[dim].gridSpacing) * m_gridOptions.line[dim].gridSpacing;
		}
	}
}

void CGraphCtrl::SetRange(double dLeft, double dRight, int nXDecimal, double dLower, double dUpper, int nYDecimal){
	if(m_axisOptions.first.equal){
		double xRange = dRight - dLeft;
		double yRange = dUpper - dLower;
		if(xRange > yRange){
			dLower -= (xRange - yRange)/2;
			dUpper += (xRange - yRange)/2;
		}
		if(xRange < yRange){
			dLeft  -= (yRange - xRange)/2;
			dRight += (yRange - xRange)/2;
		}
		m_gridOptions.X().autoScale = false;
		m_gridOptions.Y().autoScale = false;
	}

	SetRangeX(dLeft, dRight, nXDecimal, false);
	SetRangeY(dLower, dUpper, nYDecimal, false);
	InvalidateCtrl();
}
void CGraphCtrl::SetRangeX(double left, double right, int decimals, bool invalidate){
	if(left > right)
		right = left + 1;
	SetGridSpacingX(left, right);
	if(right - left < m_axisOptions.minimumRangeX)
		right = left + m_axisOptions.minimumRangeX;

	m_axisOptions.first.left    = (float)left - m_axisOptions.first.margin;
	m_axisOptions.first.right   = (float)right + m_axisOptions.first.margin;
	m_axisOptions.second.left    = (float)left - m_axisOptions.first.margin;
	m_axisOptions.second.right   = (float)right + m_axisOptions.first.margin;
	m_nYDecimals							 = decimals ;
	m_dHorizonFactor					 = (double)m_nPlotWidth / (m_axisOptions.first.right - m_axisOptions.first.left) ; 
	if(invalidate)
		InvalidateCtrl();
}

void CGraphCtrl::SetRangeY(double lower, double upper, int decimals, bool invalidate){
	if(lower > upper)
		upper = lower + 1;
	if(upper - lower < m_axisOptions.minimumRangeY)
		upper = lower + m_axisOptions.minimumRangeY;
	SetGridSpacingY(lower, upper);

	m_axisOptions.first.bottom    = (float)lower - m_axisOptions.first.margin;
	m_axisOptions.first.top				= (float)upper + m_axisOptions.first.margin;
	m_nYDecimals									= decimals ;
	m_dVerticalFactor							= (double)m_nPlotHeight / (m_axisOptions.first.top - m_axisOptions.first.bottom) ; 
	if(invalidate)
		InvalidateCtrl();
}

/** Sets the range of both the second X and second Y-axes*/
void CGraphCtrl::SetSecondRange(double dLeft, double dRight, int nXDecimal, double dLower, double dUpper, int nYDecimal){
	if(m_axisOptions.second.equal){
		double xRange = dRight - dLeft;
		double yRange = dUpper - dLower;
		if(xRange > yRange){
			dLower -= (xRange - yRange)/2;
			dUpper += (xRange - yRange)/2;
		}
		if(xRange < yRange){
			dLeft  -= (yRange - xRange)/2;
			dRight += (yRange - xRange)/2;
		}
	}

	SetSecondRangeX(dLeft,	dRight, nXDecimal, false);
	SetSecondRangeY(dLower, dUpper, nYDecimal, false);
	InvalidateCtrl();
}

/** Sets the range for the second Y-axis */
void CGraphCtrl::SetSecondRangeY(double lower, double upper, int decimals, bool invalidate){
	ASSERT(lower < upper) ;
	m_axisOptions.second.bottom = (float)lower ;
	m_axisOptions.second.top    = (float)upper ;
	m_nY2Decimals								= decimals ;
	if(invalidate)
		InvalidateCtrl();
}

/** Sets the range for the second Y-axis */
void CGraphCtrl::SetSecondRangeX(double left, double right, int decimals, bool invalidate){
	ASSERT(left < right) ;
	m_axisOptions.second.left			= (float)left ;
	m_axisOptions.second.right    = (float)right ;
	m_nX2Decimals									= decimals ;
	if(invalidate)
		InvalidateCtrl();
}

/** Pretty prints the number into the supplied string. Used for the scaling of the axes */
void	CGraphCtrl::PrintNumber(double number, int nDecimals, NUMBER_FORMAT format, CString &str){

	// -------- printing a general number ------------
	if(FORMAT_GENERAL == format){
		int exponential;
		if(number < 1.0 && number > 0.0){
			exponential = (int)(1 + fabs(log10(fabs(number))));
			str.Format("%.*lfE-%d", nDecimals, number * pow(10.0,exponential),exponential) ;
			return;
		}
		if(fabs(number) > 1e4){
			exponential = (int)log10(fabs(number));
			str.Format("%.*lfE%d", nDecimals, number / pow(10.0, exponential), exponential);
		}else{
			str.Format("%.*lf", nDecimals, number);
		}
		return;
	}

	// -------- printing a time of day ------------
	if(FORMAT_TIME == format){
		// interpret the number as seconds after midnight.
		int number_int = (int)number;

		int hours			= (int)(number / 3600.0);
		int minutes		= (number_int - hours * 3600) / 60;
		int seconds		= number_int % 60;
		str.Format("%02d:%02d:%02d", hours, minutes, seconds);
		return;
	}

}
