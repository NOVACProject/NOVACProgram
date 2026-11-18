// OScopeCtrl.cpp : implementation file//

#include "stdafx.h"
#include "math.h"

#include "OScopeCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__ ;
#endif

using namespace Graph;

/////////////////////////////////////////////////////////////////////////////
// COScopeCtrl
COScopeCtrl::COScopeCtrl()
{
  // since plotting is based on a LineTo for each new point
  // we need a starting point (i.e. a "previous" point)
  // use 0.0 as the default first point.
  // these are public member variables, and can be changed outside
  // (after construction).  Therefore m_perviousPosition could be set to
  // a more appropriate value prior to the first call to SetPosition.
  m_dPreviousPosition =   0.0 ;

  // public variable for the number of decimal places on the y axis
  m_nYDecimals = 3 ;
  m_nXDecimals = 1 ;
  // set some initial values for the scaling until "SetRange" is called.
  // these are protected varaibles and must be set with SetRange
  // in order to ensure that m_dRange is updated accordingly
  m_dLowerLimit = -10.0 ;
  m_dUpperLimit =  10.0 ;
  m_dRange      =  m_dUpperLimit - m_dLowerLimit ;   // protected member variable

  m_dLeftLimit	= -95.0;
  m_dUpperLimit = 95.0;
  m_dRange		= m_dRightLimit - m_dLeftLimit;
  // m_nShiftPixels determines how much the plot shifts (in terms of pixels) 
  // with the addition of a new data point
  m_nShiftPixels     = 4 ;
  m_nHalfShiftPixels = m_nShiftPixels/2 ;                     // protected
  m_nPlotShiftPixels = m_nShiftPixels + m_nHalfShiftPixels ;  // protected

  // background, grid and data colors
  // these are public variables and can be set directly
  m_colors.background = RGB(0, 0, 0);
  m_colors.grid       = RGB(0, 255, 255);
  m_colors.circles    = RGB(255, 255, 255);
  m_crPlotColor       = RGB(255, 255, 255) ;  // see also SetPlotColor

  // the line widht of the plot, also public and can be set directly
  m_crPlotLineWidth = 1;

  // protected variables
  m_penPlot.CreatePen(PS_SOLID, m_crPlotLineWidth, m_crPlotColor) ;
  m_penPlot2.CreatePen(PS_SOLID, m_crPlotLineWidth, RGB(0,255,0)) ;
  m_brushBack.CreateSolidBrush(m_colors.background) ;

  // public member variables, can be set directly 
  m_strXUnitsString.Format("Samples") ;  // can also be set with SetXUnits
  m_strYUnitsString.Format("Y units") ;  // can also be set with SetYUnits

  // protected bitmaps to restore the memory DC's
  m_pbitmapOldGrid = NULL ;
  m_pbitmapOldPlot = NULL ;

  cnt = 0; //counter for drawing rect
  rectWidth = 24;//(int)(1196/nSum);  //width of one rectangle
          
  axisFlag = 0;
  digitsFlag =0;
}  // COScopeCtrl

/////////////////////////////////////////////////////////////////////////////
COScopeCtrl::~COScopeCtrl()
{
  // just to be picky restore the bitmaps for the two memory dc's
  // (these dc's are being destroyed so there shouldn't be any leaks)
  if (m_pbitmapOldGrid != NULL)
    m_dcGrid.SelectObject(m_pbitmapOldGrid) ;  
  if (m_pbitmapOldPlot != NULL)
    m_dcPlot.SelectObject(m_pbitmapOldPlot) ;  

} // ~COScopeCtrl


BEGIN_MESSAGE_MAP(COScopeCtrl, CWnd)
  //{{AFX_MSG_MAP(COScopeCtrl)
  ON_WM_PAINT()
  ON_WM_SIZE()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// COScopeCtrl message handlers

/////////////////////////////////////////////////////////////////////////////
BOOL COScopeCtrl::Create(DWORD dwStyle, const RECT& rect, 
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
void COScopeCtrl::SetRange(double dLower, double dUpper, int nDecimalPlaces)
{
  ASSERT(dUpper > dLower) ;

  m_dLowerLimit     = dLower ;
  m_dUpperLimit     = dUpper ;
  m_nYDecimals      = nDecimalPlaces ;
  m_dRange          = m_dUpperLimit - m_dLowerLimit ;
  m_dVerticalFactor = (double)m_nPlotHeight / m_dRange ; 
  
  // clear out the existing garbage, re-start with a clean plot
  InvalidateCtrl() ;

}  // SetRange


/////////////////////////////////////////////////////////////////////////////
void COScopeCtrl::SetXUnits(CString string)
{
  m_strXUnitsString = string ;

  // clear out the existing garbage, re-start with a clean plot
  InvalidateCtrl() ;

}  // SetXUnits

/////////////////////////////////////////////////////////////////////////////
void COScopeCtrl::SetYUnits(CString string)
{
  m_strYUnitsString = string ;

  // clear out the existing garbage, re-start with a clean plot
  InvalidateCtrl() ;

}  // SetYUnits

/////////////////////////////////////////////////////////////////////////////
void COScopeCtrl::SetGridColor(COLORREF color)
{
  m_colors.grid = color;

  // clear out the existing garbage, re-start with a clean plot
  InvalidateCtrl() ;

}  // SetGridColor


/////////////////////////////////////////////////////////////////////////////
void COScopeCtrl::SetPlotColor(COLORREF color, bool clear)
{
  m_crPlotColor = color ;

  m_penPlot.DeleteObject() ;
  m_penPlot.CreatePen(PS_SOLID, m_crPlotLineWidth, m_crPlotColor) ;

  // clear out the existing garbage, re-start with a clean plot
  if(clear)
    InvalidateCtrl() ;

}  // SetPlotColor


/////////////////////////////////////////////////////////////////////////////
void COScopeCtrl::SetBackgroundColor(COLORREF color)
{
  m_colors.background = color;

  m_brushBack.DeleteObject() ;
  m_brushBack.CreateSolidBrush(m_colors.background) ;

  // clear out the existing garbage, re-start with a clean plot
  InvalidateCtrl() ;

}  // SetBackgroundColor


/** Sets the color for the circles */
void COScopeCtrl::SetCircleColor(COLORREF color, bool clear){
  m_colors.circles = color;

  if(clear)
    InvalidateCtrl();
}

/////////////////////////////////////////////////////////////////////////////
void COScopeCtrl::IncreaseLineWidth(){
  m_penPlot.DeleteObject();
  m_penPlot.CreatePen(PS_SOLID, ++m_crPlotLineWidth, m_crPlotColor) ;

  // clear out the existing garbage, re-start with a clean plot
  InvalidateCtrl() ;

}  // IncreaseLineWidth

/////////////////////////////////////////////////////////////////////////////
void COScopeCtrl::DecreaseLineWidth(){
  if(m_crPlotLineWidth == 0)
    return;

  m_penPlot.DeleteObject();
  m_penPlot.CreatePen(PS_SOLID, --m_crPlotLineWidth, m_crPlotColor) ;

  // clear out the existing garbage, re-start with a clean plot
  InvalidateCtrl() ;

}  // DecreaseLineWidth

/////////////////////////////////////////////////////////////////////////////
void COScopeCtrl::InvalidateCtrl()
{
  // There is a lot of drawing going on here - particularly in terms of 
  // drawing the grid.  Don't panic, this is all being drawn (only once)
  // to a bitmap.  The result is then BitBlt'd to the control whenever needed.
  int i ;
  int nCharacters ;
  int nTopGridPix, nMidGridPix, nBottomGridPix ;
  int yLowDigits, yHighDigits;
  CPen *oldPen ;
  CPen solidPen(PS_SOLID, 0, m_colors.grid) ;
  CFont axisFont, yUnitFont, *oldFont ;
  CString strTemp ;

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

  // fill the grid background
  m_dcGrid.FillRect(m_rectClient, &m_brushBack) ;

  // draw the plot rectangle:
  // determine how wide the y axis scaling values are
  if(fabs(m_dUpperLimit) > 1e4){
    int nPowersOfTen = (int)log10(fabs(m_dUpperLimit)); 
    nCharacters = 2 + abs((int)log10(fabs(nPowersOfTen))) ;
  }else{
    nCharacters = abs((int)log10(fabs(m_dUpperLimit))) ;
  }
  if(fabs(m_dLowerLimit) > 1e4){
    int nPowersOfTen = (int)log10(fabs(m_dLowerLimit)); 
    nCharacters = max(nCharacters, 2 + abs((int)log10(fabs(nPowersOfTen)))) ;
  }else{
    nCharacters = max(nCharacters, abs((int)log10(fabs(m_dLowerLimit)))) ;
  }


  // add the units digit, decimal point and a minus sign, and an extra space
  // as well as the number of decimal places to display
  nCharacters = nCharacters + 4 + m_nYDecimals ;  

  // adjust the plot rectangle dimensions
  // assume 6 pixels per character (this may need to be adjusted)
  if(axisFlag)
  {
	  m_rectPlot.left = m_rectClient.left + 8*(nCharacters) ;
  }
  else
  {
	  m_rectPlot.left = m_rectClient.left + 6*(nCharacters) ;
  }
  m_rectPlot.right = m_rectClient.right - 30;//6*(nCharacters) ;
  m_nPlotWidth    = m_rectPlot.Width() ;

  // draw the plot rectangle
  oldPen = m_dcGrid.SelectObject (&solidPen) ; 
  m_dcGrid.MoveTo (m_rectPlot.left, m_rectPlot.top) ;
  m_dcGrid.LineTo (m_rectPlot.right+1, m_rectPlot.top) ;
  m_dcGrid.LineTo (m_rectPlot.right+1, m_rectPlot.bottom+1) ;
  m_dcGrid.LineTo (m_rectPlot.left, m_rectPlot.bottom+1) ;
  m_dcGrid.LineTo (m_rectPlot.left, m_rectPlot.top) ;
  m_dcGrid.SelectObject (oldPen) ; 

  // draw the dotted lines, 
  // use SetPixel instead of a dotted pen - this allows for a 
  // finer dotted line and a more "technical" look
  nMidGridPix    = (m_rectPlot.top + m_rectPlot.bottom)/2 ;
  nTopGridPix    = nMidGridPix - m_nPlotHeight/4 ;
  nBottomGridPix = nMidGridPix + m_nPlotHeight/4 ;

  for (i=m_rectPlot.left; i<m_rectPlot.right; i+=4)
  {
    m_dcGrid.SetPixel (i, nTopGridPix,    m_colors.grid) ;
    m_dcGrid.SetPixel (i, nMidGridPix,    m_colors.grid) ;
    m_dcGrid.SetPixel (i, nBottomGridPix, m_colors.grid) ;
  }

  // create some fonts (horizontal and vertical)
  // use a height of 14 pixels and 300 weight 
  // (these may need to be adjusted depending on the display)
  axisFont.CreateFont (14, 0, 0, 0, 300,
                       FALSE, FALSE, 0, ANSI_CHARSET,
                       OUT_DEFAULT_PRECIS, 
                       CLIP_DEFAULT_PRECIS,
                       DEFAULT_QUALITY, 
                       DEFAULT_PITCH|FF_SWISS, "Arial") ;
  yUnitFont.CreateFont (14, 0, 900, 0, 300,
                       FALSE, FALSE, 0, ANSI_CHARSET,
                       OUT_DEFAULT_PRECIS, 
                       CLIP_DEFAULT_PRECIS,
                       DEFAULT_QUALITY, 
                       DEFAULT_PITCH|FF_SWISS, "Arial") ;
  
  // grab the horizontal font
  oldFont = m_dcGrid.SelectObject(&axisFont) ;
// y max   
  m_dcGrid.SetTextColor (m_colors.grid) ;
  m_dcGrid.SetTextAlign (TA_RIGHT|TA_TOP) ;

  if((m_dUpperLimit<1.0)&&(m_dUpperLimit>=0))    
  {	
	  yHighDigits = GetZeroNumber(m_dUpperLimit);
	  strTemp.Format ("%.*lfE-%d", m_nYDecimals, m_dUpperLimit*pow(10.0,yHighDigits),yHighDigits) ;
  }
  else
  {
    if(fabs(m_dUpperLimit) > 1e4){
      int nPowersOfTen = (int)log10(fabs(m_dUpperLimit));
      strTemp.Format("%.*lfE%d", m_nYDecimals, m_dUpperLimit/pow(10.0, nPowersOfTen), nPowersOfTen);
    }else{
    	strTemp.Format ("%.*lf", m_nYDecimals, m_dUpperLimit) ;
    }
  }
  m_dcGrid.TextOut (m_rectPlot.left-4, m_rectPlot.top, strTemp) ;

  // y min
  m_dcGrid.SetTextAlign (TA_RIGHT|TA_BASELINE) ;

  if(fabs(m_dLowerLimit) < 1.0)
  {
	  yLowDigits = GetZeroNumber(m_dLowerLimit);
	  strTemp.Format("%.*lfE-%d", m_nYDecimals, m_dLowerLimit*pow(10.0,yLowDigits), yLowDigits);
  }
  else
  {
    if(fabs(m_dLowerLimit) > 1e4){
      int nPowersOfTen = (int)log10(fabs(m_dLowerLimit));
      strTemp.Format("%.*lfE%d", m_nYDecimals, m_dLowerLimit/pow(10.0, nPowersOfTen), nPowersOfTen);
    }else{
    	strTemp.Format ("%.*lf", m_nYDecimals, m_dLowerLimit) ;
    }
  }
  m_dcGrid.TextOut (m_rectPlot.left-4, m_rectPlot.bottom, strTemp) ;

  // x min
  m_dcGrid.SetTextAlign (TA_LEFT|TA_TOP) ;
   strTemp.Format ("%.*lf", m_nXDecimals, m_dLeftLimit) ;//add
  m_dcGrid.TextOut (m_rectPlot.left, m_rectPlot.bottom+4, strTemp) ;

  // x max
  m_dcGrid.SetTextAlign (TA_RIGHT|TA_TOP) ;
  strTemp.Format("%.*lf",m_nXDecimals, m_dRightLimit); //("%d", m_nPlotWidth/m_nShiftPixels) ; 
  m_dcGrid.TextOut (m_rectPlot.right, m_rectPlot.bottom+4, strTemp) ;

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

  if(axisFlag > 0)
  {
    oldFont = m_dcGrid.SelectObject(&axisFont) ;
	  m_dcGrid.SetTextAlign (TA_RIGHT|TA_TOP) ;
	  m_dcGrid.TextOut (m_rectPlot.right+28, m_rectPlot.top , m_strYMax) ;
	  m_dcGrid.SetTextAlign (TA_RIGHT|TA_BASELINE) ;
	  m_dcGrid.TextOut (m_rectPlot.right +20, m_rectPlot.bottom , m_strYMin) ;	
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

  // make sure the plot bitmap is cleared
  m_dcPlot.SetBkColor (m_colors.background) ;
  m_dcPlot.FillRect(m_rectClient, &m_brushBack) ;

  // finally, force the plot area to redraw
  InvalidateRect(m_rectClient) ;

} // InvalidateCtrl


/////////////////////////////////////////////////////////////////////////////
double COScopeCtrl::AppendPoint(double dNewPoint)
{
  // append a data point to the plot
  // return the previous point

  double dPrevious ;
  
  dPrevious = m_dCurrentPosition ;
  m_dCurrentPosition = dNewPoint ;
  DrawPoint() ;
//	DrawMyPoint();
  Invalidate() ;

  return dPrevious ;

} // AppendPoint
 
////////////////////////////////////////////////////////////////////////////
void COScopeCtrl::AppendRect(double dNewColumn, double dDegree , int nSum)
{
	m_dCurrentPosition = dNewColumn;
	m_xPosition			= dDegree;
	DrawRect(nSum);
	cnt++;
	Invalidate();

}

void COScopeCtrl::AppendMyPoint(double dNewPoint)
{
	//double dPrevious ;


	m_dCurrentPosition = dNewPoint ;

	Invalidate();

}


void COScopeCtrl::AppendCircle(double dNewColumn, double dDegree,double dVerticalRange)
{
	m_dCurrentPosition  = dNewColumn;
	m_xPosition			= dDegree;
	double factor		= (double)m_nPlotHeight / dVerticalRange;
	DrawCircle(factor);
	Invalidate();

}
////////////////////////////////////////////////////////////////////////////
void COScopeCtrl::OnPaint() 
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
    // first drop the grid on the memory dc
    memDC.BitBlt(0, 0, m_nClientWidth, m_nClientHeight, 
                 &m_dcGrid, 0, 0, SRCCOPY) ;
    // now add the plot on top as a "pattern" via SRCPAINT.
    // works well with dark background and a light plot
    memDC.BitBlt(0, 0, m_nClientWidth, m_nClientHeight, 
                 &m_dcPlot, 0, 0, SRCPAINT) ;  //SRCPAINT
    // finally send the result to the display
    dc.BitBlt(0, 0, m_nClientWidth, m_nClientHeight, 
              &memDC, 0, 0, SRCCOPY) ;
  }

  memDC.SelectObject(oldBitmap) ;

} // OnPaint

/////////////////////////////////////////////////////////////////////////////
void COScopeCtrl::DrawPoint()
{
  // this does the work of "scrolling" the plot to the left
  // and appending a new data point all of the plotting is 
  // directed to the memory based bitmap associated with m_dcPlot
  // the will subsequently be BitBlt'd to the client in OnPaint
  
  int currX, prevX, currY, prevY ;

  CPen *oldPen ;
  CRect rectCleanUp ;

  if (m_dcPlot.GetSafeHdc() != NULL)
  {
    // shift the plot by BitBlt'ing it to itself 
    // note: the m_dcPlot covers the entire client
    //       but we only shift bitmap that is the size 
    //       of the plot rectangle
    // grab the right side of the plot (exluding m_nShiftPixels on the left)
    // move this grabbed bitmap to the left by m_nShiftPixels

    m_dcPlot.BitBlt(m_rectPlot.left, m_rectPlot.top+1, 
                    m_nPlotWidth, m_nPlotHeight, &m_dcPlot, 
                    m_rectPlot.left+m_nShiftPixels, m_rectPlot.top+1, 
                    SRCCOPY) ;

    // establish a rectangle over the right side of plot
    // which now needs to be cleaned up proir to adding the new point
    rectCleanUp = m_rectPlot ;
    rectCleanUp.left  = rectCleanUp.right - m_nShiftPixels ;

    // fill the cleanup area with the background
    m_dcPlot.FillRect(rectCleanUp, &m_brushBack) ;

    // draw the next line segement

    // grab the plotting pen
    oldPen = m_dcPlot.SelectObject(&m_penPlot) ;

    // move to the previous point
    prevX = m_rectPlot.right-m_nPlotShiftPixels ;
    prevY = m_rectPlot.bottom - 
            (long)((m_dPreviousPosition - m_dLowerLimit) * m_dVerticalFactor) ;
    m_dcPlot.MoveTo (prevX, prevY) ;

    // draw to the current point
    currX = m_rectPlot.right-m_nHalfShiftPixels ;
    currY = m_rectPlot.bottom -
            (long)((m_dCurrentPosition - m_dLowerLimit) * m_dVerticalFactor) ;
    m_dcPlot.LineTo (currX, currY) ;

    // restore the pen 
    m_dcPlot.SelectObject(oldPen) ;

    // if the data leaks over the upper or lower plot boundaries
    // fill the upper and lower leakage with the background
    // this will facilitate clipping on an as needed basis
    // as opposed to always calling IntersectClipRect
    if ((prevY <= m_rectPlot.top) || (currY <= m_rectPlot.top))
      m_dcPlot.FillRect(CRect(prevX, m_rectClient.top, currX+1, m_rectPlot.top+1), &m_brushBack) ;
    if ((prevY >= m_rectPlot.bottom) || (currY >= m_rectPlot.bottom))
      m_dcPlot.FillRect(CRect(prevX, m_rectPlot.bottom+1, currX+1, m_rectClient.bottom+1), &m_brushBack) ;

    // store the current point for connection to the next point
    m_dPreviousPosition = m_dCurrentPosition ;

  }

} // end DrawPoint

void COScopeCtrl::DrawPoint(double* yPositions, int pointSum,int beginX)
{
  // this does the work of "scrolling" the plot to the left
  // and appending a new data point all of the plotting is 
  // directed to the memory based bitmap associated with m_dcPlot
  // the will subsequently be BitBlt'd to the client in OnPaint
  
  CPen *oldPen ;
  int i;
  if (m_dcPlot.GetSafeHdc() != NULL)
  {
    m_dcPlot.BitBlt(m_rectPlot.left, m_rectPlot.top+1, 
                    m_nPlotWidth, m_nPlotHeight, &m_dcPlot, 
                    m_rectPlot.left, m_rectPlot.top+1, 
                    SRCCOPY) ;

	  //initiate the beginning point for DrawMyPoint()
	  m_prevX = m_rectPlot.left +
					(long)((beginX - m_dLeftLimit) *m_dHorizonFactor); 
	  m_prevY = m_rectPlot.bottom -
					(long)((yPositions[beginX] - m_dLowerLimit) * m_dVerticalFactor);

		// grab the plotting pen
		oldPen = m_dcPlot.SelectObject(&m_penPlot) ;

    for(i=beginX +1; i< beginX + pointSum ; i++)
		{      
			m_dcPlot.MoveTo (m_prevX, m_prevY) ;

			m_curX = m_rectPlot.left + (long)((i- m_dLeftLimit)*m_dHorizonFactor); 
			m_curY = m_rectPlot.bottom -
					(long)((yPositions[i] - m_dLowerLimit) * m_dVerticalFactor) ;
			m_dcPlot.LineTo (m_curX, m_curY) ;
			m_prevX = m_curX;
			m_prevY = m_curY;
		}
		m_dcPlot.SelectObject(oldPen);
	  
	  
	// if the data leaks over the upper or lower plot boundaries
    // fill the upper and lower leakage with the background
    // this will facilitate clipping on an as needed basis
    // as opposed to always calling IntersectClipRect
      m_dcPlot.FillRect(CRect(m_rectClient.left, m_rectClient.top, m_rectPlot.right, m_rectPlot.top+1), &m_brushBack) ;

      m_dcPlot.FillRect(CRect(m_rectClient.left, m_rectPlot.bottom+1, m_rectClient.right+1, m_rectClient.bottom+1), &m_brushBack) ;
	
      m_dcPlot.FillRect(CRect(m_rectPlot.right, m_rectPlot.top+1, m_rectClient.right+1, m_rectPlot.bottom+1), &m_brushBack) ;
 
  }

   Invalidate() ;


} // end DrawPoint

void COScopeCtrl::DrawMyPoint(double* yPositions, int pointSum, bool clear)
{
  CPen *oldPen ;
  int i;//, j;
 // long averageY = 0;
  if (m_dcPlot.GetSafeHdc() != NULL)
  {
    m_dcPlot.BitBlt(m_rectPlot.left, m_rectPlot.top+1, 
                    m_nPlotWidth, m_nPlotHeight, &m_dcPlot, 
                    m_rectPlot.left, m_rectPlot.top+1, 
                    SRCCOPY) ;



	  //initiate the beginning point for DrawMyPoint()
	  m_prevX = m_rectPlot.left ;
	  m_prevY = m_rectPlot.bottom;

	  //If the m_dHorizonFactor is too low, plot will skip a number of points .
	  int resolution = (int)(1/m_dHorizonFactor); 
	  if(resolution > 0)
	  {	 
		for(i=0; i< pointSum - resolution+1; i++)
		{      
			// grab the plotting pen
      if(clear)
  	    oldPen = m_dcPlot.SelectObject(&m_penPlot);
      else
	      oldPen = m_dcPlot.SelectObject(&m_penPlot2);

      // move to the previous point
			m_dcPlot.MoveTo (m_prevX, m_prevY) ;

			m_curX =m_prevX + 1; //m_dHorizonFactor;
			m_curY = m_rectPlot.bottom -
					(long)((yPositions[i] - m_dLowerLimit) * m_dVerticalFactor) ;
			m_dcPlot.LineTo (m_curX, m_curY) ;
			m_prevX = m_curX;
			m_prevY = m_curY;   
			// restore the pen 
			m_dcPlot.SelectObject(oldPen) ;
	  
			i = i+ resolution -1;  

		}
	  }
	  else
	  {
	  
	  	for(i=0; i< pointSum ; i++)
		{      
			// grab the plotting pen
      if(clear)
  	    oldPen = m_dcPlot.SelectObject(&m_penPlot);
      else
	      oldPen = m_dcPlot.SelectObject(&m_penPlot2);

      // move to the previous point
			m_dcPlot.MoveTo (m_prevX, m_prevY) ;

			m_curX = (int)(m_prevX + m_dHorizonFactor);
			m_curY = m_rectPlot.bottom -
					(long)((yPositions[i] - m_dLowerLimit) * m_dVerticalFactor) ;
			m_dcPlot.LineTo (m_curX, m_curY) ;
			m_prevX = m_curX;
			m_prevY = m_curY;   
			m_dcPlot.SelectObject(oldPen) ; 		

		}
	  
	  }
	// if the data leaks over the upper or lower plot boundaries
    // fill the upper and lower leakage with the background
    // this will facilitate clipping on an as needed basis
    // as opposed to always calling IntersectClipRect
      m_dcPlot.FillRect(CRect(m_rectClient.left, m_rectClient.top, m_rectPlot.right, m_rectPlot.top+1), &m_brushBack) ;

      m_dcPlot.FillRect(CRect(m_rectClient.left, m_rectPlot.bottom+1, m_rectClient.right+1, m_rectClient.bottom+1), &m_brushBack) ;
	
      m_dcPlot.FillRect(CRect(m_rectPlot.right, m_rectPlot.top+1, m_rectClient.right+1, m_rectPlot.bottom+1), &m_brushBack) ;
 
  }

   Invalidate(clear) ;
}

void COScopeCtrl::DrawMyPoint(double* yPositions, int pointSum,double xrange,double yrange, bool clear)
{
  CPen *oldPen ;
  int i,resolution;
  double xFactor,yFactor;
  if (m_dcPlot.GetSafeHdc() != NULL)
  {
    m_dcPlot.BitBlt(m_rectPlot.left, m_rectPlot.top+1, 
                    m_nPlotWidth, m_nPlotHeight, &m_dcPlot, 
                    m_rectPlot.left, m_rectPlot.top+1, 
                    SRCCOPY) ;
	  //initiate the beginning point for DrawMyPoint()
	  m_prevX = m_rectPlot.left ;
	  m_prevY = m_rectPlot.bottom;
		if(xrange>0)
			xFactor = m_nPlotWidth/xrange;
		else
			xFactor = m_dHorizonFactor;
		if(yrange>0)
			yFactor = m_nPlotHeight/yrange;
		else
			yFactor = m_dVerticalFactor;
	  //If the m_dHorizonFactor is too low, plot will skip a number of points .
	//  int resolution = (int)(1/m_dHorizonFactor); 	 
		  resolution = (int)(1/xFactor);
	  if(resolution > 0)
	  {	 
		for(i=0; i< pointSum - resolution+1; i++)
		{      
			// grab the plotting pen
      if(clear)
  			oldPen = m_dcPlot.SelectObject(&m_penPlot2) ;
      else
	        oldPen = m_dcPlot.SelectObject(&m_penPlot) ;

			// move to the previous point
			m_dcPlot.MoveTo (m_prevX, m_prevY) ;

			m_curX =m_prevX + 1; //m_dHorizonFactor;
			m_curY = m_rectPlot.bottom -
					(long)(yPositions[i] * yFactor) ;
			m_dcPlot.LineTo (m_curX, m_curY) ;
			m_prevX = m_curX;
			m_prevY = m_curY;   
			// restore the pen 
			m_dcPlot.SelectObject(oldPen) ;
	  
			i = i+ resolution -1;  

		}
	  }
	  else
	  {
	  
	  	for(i=0; i< pointSum ; i++)
		{      
			// grab the plotting pen
			oldPen = m_dcPlot.SelectObject(&m_penPlot) ;
			// move to the previous point
			m_dcPlot.MoveTo (m_prevX, m_prevY) ;

			m_curX = (int)(m_prevX + xFactor);
			m_curY = m_rectPlot.bottom -
					(long)(yPositions[i] * yFactor) ;
			m_dcPlot.LineTo (m_curX, m_curY) ;
			m_prevX = m_curX;
			m_prevY = m_curY;   
			m_dcPlot.SelectObject(oldPen) ; 		

		}
	  
	  }
	// if the data leaks over the upper or lower plot boundaries
    // fill the upper and lower leakage with the background
    // this will facilitate clipping on an as needed basis
    // as opposed to always calling IntersectClipRect
      m_dcPlot.FillRect(CRect(m_rectClient.left, m_rectClient.top, m_rectPlot.right, m_rectPlot.top+1), &m_brushBack) ;

      m_dcPlot.FillRect(CRect(m_rectClient.left, m_rectPlot.bottom+1, m_rectClient.right+1, m_rectClient.bottom+1), &m_brushBack) ;
	
      m_dcPlot.FillRect(CRect(m_rectPlot.right, m_rectPlot.top+1, m_rectClient.right+1, m_rectPlot.bottom+1), &m_brushBack) ;
 
  }

   Invalidate(clear) ;


}

/////////////////////////////////////////////////////////////////////////////
void COScopeCtrl::OnSize(UINT nType, int cx, int cy) 
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
  m_nPlotHeight = m_rectPlot.Height() ;
  m_nPlotWidth  = m_rectPlot.Width() ;

  // set the scaling factor for now, this can be adjusted 
  // in the SetRange functions
  m_dVerticalFactor = (double)m_nPlotHeight / m_dRange ; 

  m_dHorizonFactor = (double)m_nPlotWidth / m_dWRange;

} // OnSize


/////////////////////////////////////////////////////////////////////////////
void COScopeCtrl::Reset()
{
  // to clear the existing data (in the form of a bitmap)
  // simply invalidate the entire control
  InvalidateCtrl() ;
}

void COScopeCtrl::SetRange(double dLeft, double dRight, int nXDecimal, double dLower, double dUpper, int nYDecimal)
{
  ASSERT(dUpper > dLower) ;
  ASSERT(dRight > dLeft);

  m_dLowerLimit     = dLower ;
  m_dUpperLimit     = dUpper ;
  m_nYDecimals      = nYDecimal ;
  m_dRange          = m_dUpperLimit - m_dLowerLimit ;
  m_dVerticalFactor = (double)m_nPlotHeight / m_dRange ; 

  m_dLeftLimit      = dLeft;
  m_dRightLimit     = dRight;
  m_nXDecimals		= nXDecimal;
  m_dWRange			= m_dRightLimit - m_dLeftLimit;
  m_dHorizonFactor  = (double)m_nPlotWidth / m_dWRange;
  
  // clear out the existing garbage, re-start with a clean plot
  InvalidateCtrl() ;

}

void COScopeCtrl::CleanPlot()
{
  InvalidateCtrl();
}

void COScopeCtrl::DrawRect(int nSum)
{

	
	CRect rect;
    CString strTemp;

	if(m_dcPlot.GetSafeHdc() != NULL)
	{
		m_dcPlot.BitBlt(m_rectPlot.left, m_rectPlot.top, 
                    m_nPlotWidth, m_nPlotHeight, &m_dcPlot, 
                    m_rectPlot.left, m_rectPlot.top, 
                    SRCCOPY) ;

		int topLeftx,topLefty,bottomRightx,bottomRighty;
		topLeftx = m_rectPlot.left + (long)((m_xPosition - m_dLeftLimit)*m_dHorizonFactor) ;
					
		topLefty = m_rectPlot.bottom - (long)((m_dCurrentPosition 
		  - m_dLowerLimit)*m_dVerticalFactor);
		bottomRightx = topLeftx + rectWidth;
		bottomRighty = m_rectPlot.bottom-(long)((-m_dLowerLimit)*m_dVerticalFactor);	//when 0 is in the middle

	
		rect = CRect(topLeftx,topLefty,bottomRightx,bottomRighty);

		m_dcPlot.FillSolidRect(rect,m_crPlotColor);

    m_dcPlot.SetTextColor (m_colors.grid) ;
		m_dcPlot.SetBkMode(TRANSPARENT); 

		strTemp.Format ("%.*lf", m_nYDecimals, m_dCurrentPosition) ;
		m_dcPlot.TextOut (topLeftx, topLefty - 8, strTemp) ;


		if (topLefty <= m_rectPlot.top)
		 m_dcPlot.FillRect(CRect(topLeftx, topLefty, m_rectPlot.right, m_rectPlot.top), &m_brushBack) ;
		if (bottomRightx >= m_rectPlot.right)
		 m_dcPlot.FillRect(CRect(m_rectPlot.right, topLefty,bottomRightx,bottomRighty), &m_brushBack) ;
		if(topLefty >= m_rectPlot.bottom)
		  m_dcPlot.FillRect(CRect(topLeftx,topLefty,bottomRightx,m_rectPlot.bottom), &m_brushBack);
		if(topLeftx <m_rectPlot.left)
		  m_dcPlot.FillRect(CRect(topLeftx,topLefty,m_rectPlot.left,m_rectPlot.bottom), &m_brushBack);

	}
}

void COScopeCtrl::DrawCircle()
{
	const int r = 3;
	CRect rect;

	if(m_dcPlot.GetSafeHdc() != NULL)
	{
     m_dcPlot.BitBlt(m_rectPlot.left, m_rectPlot.top+1, 
                    m_nPlotWidth, m_nPlotHeight, &m_dcPlot, 
                    m_rectPlot.left, m_rectPlot.top+1, 
                    SRCCOPY) ;

     int topLeftx,topLefty,bottomRightx,bottomRighty;
	 topLeftx = m_rectPlot.left + (long)((m_xPosition - m_dLeftLimit)*m_dHorizonFactor)- rectWidth/2 - r;
					
	 topLefty = m_rectPlot.bottom - (long)((m_dCurrentPosition 
		  - m_dLowerLimit)*m_dVerticalFactor)-r;
	
	 bottomRightx = topLeftx+r;
	 bottomRighty = topLefty+r;

   m_dcPlot.FillSolidRect(topLeftx,topLefty,r,r,m_colors.grid);
	  if (topLefty <= m_rectPlot.top)
	     m_dcPlot.FillRect(CRect(topLeftx, topLefty, m_rectPlot.right, m_rectPlot.top), &m_brushBack) ;
	  if (topLeftx >= m_rectPlot.right)
		 m_dcPlot.FillRect(CRect(m_rectPlot.left, topLefty,m_rectPlot.right,m_rectPlot.bottom), &m_brushBack) ;

	}
}

//This is used when draw the plot of spectrum intensity

void COScopeCtrl::DrawCircle(double verticalFactor)
{
	const int r = 6;
	CRect rect;

	if(m_dcPlot.GetSafeHdc() != NULL)
	{
     m_dcPlot.BitBlt(m_rectPlot.left, m_rectPlot.top+1, 
                    m_nPlotWidth, m_nPlotHeight, &m_dcPlot, 
                    m_rectPlot.left, m_rectPlot.top+1, 
                    SRCCOPY) ;

     int topLeftx,topLefty;
	 topLeftx = m_rectPlot.left + (long)((m_xPosition - m_dLeftLimit)*m_dHorizonFactor)+ rectWidth/2 -r/2 ;
					
	 topLefty = m_rectPlot.bottom - (long)(m_dCurrentPosition*verticalFactor)-r/2;	//here the m_dCurrrentPosition is (m_dCurrentPosition -0)

   m_dcPlot.FillSolidRect(topLeftx,topLefty,r,r,m_colors.grid);

	  if (topLefty <= m_rectPlot.top)
	     m_dcPlot.FillRect(CRect(topLeftx, topLefty, m_rectPlot.right, m_rectPlot.top), &m_brushBack) ;
	  if (topLeftx >= m_rectPlot.right)
		 m_dcPlot.FillRect(CRect(m_rectPlot.left, topLefty,m_rectPlot.right,m_rectPlot.bottom), &m_brushBack) ;

	}
}

void COScopeCtrl::DrawRects(double *dColumns, double *dDegrees, int nSum, bool clear)
{

	CRect rect;
  CString strTemp;
	CFont axisFont,*oldFont;
	int topLeftx,topLefty,bottomRightx,bottomRighty;
	int i;
	double degreeRange;
	if(m_dcPlot.GetSafeHdc() != NULL)
	{
		m_dcPlot.BitBlt(m_rectPlot.left, m_rectPlot.top, 
                    m_nPlotWidth, m_nPlotHeight, &m_dcPlot, 
                    m_rectPlot.left, m_rectPlot.top, 
                    SRCCOPY) ;

    axisFont.CreateFont (10, 0, 0, 0, 300,
                       FALSE, FALSE, 0, ANSI_CHARSET,
                       OUT_DEFAULT_PRECIS, 
                       CLIP_DEFAULT_PRECIS,
                       DEFAULT_QUALITY, 
                       DEFAULT_PITCH|FF_SWISS, "Arial") ;
		 
		degreeRange = fabs(dDegrees[nSum-1] - dDegrees[0]); 
    rectWidth = (int)fabs((m_rectPlot.right-m_rectPlot.left)*degreeRange/((m_dRightLimit - m_dLeftLimit)*nSum));

    m_dcPlot.SetTextColor (m_colors.grid) ;
		m_dcPlot.SetBkMode(TRANSPARENT); 

		for(i = 0;i<nSum;i++)
		{							
			strTemp.Format ("%d", i) ;	
		
			topLeftx = m_rectPlot.left + (long)((dDegrees[i] - m_dLeftLimit)*m_dHorizonFactor)-rectWidth/2 ;
						
			topLefty = m_rectPlot.bottom - (long)((dColumns[i] - m_dLowerLimit)*m_dVerticalFactor);
		
			bottomRightx = topLeftx + rectWidth;
		
			bottomRighty = m_rectPlot.bottom-(long)((-m_dLowerLimit)*m_dVerticalFactor);	//when 0 is in the middle
			
			rect = CRect(topLeftx,topLefty,bottomRightx,bottomRighty);

			m_dcPlot.FillSolidRect(rect,m_crPlotColor);
			oldFont = m_dcPlot.SelectObject(&axisFont) ;
			m_dcPlot.TextOut (topLeftx, bottomRighty - 8, strTemp) ;
			m_dcPlot.SelectObject(oldFont);
		}
		
		m_dcPlot.FillRect(CRect(m_rectClient.left, m_rectClient.top, m_rectClient.right, m_rectPlot.top+1), &m_brushBack) ;

        m_dcPlot.FillRect(CRect(m_rectClient.left, m_rectPlot.bottom+1, m_rectClient.right+1, m_rectClient.bottom+1), &m_brushBack) ;
	
        m_dcPlot.FillRect(CRect(m_rectPlot.right, m_rectPlot.top+1, m_rectClient.right+1, m_rectPlot.bottom+1), &m_brushBack) ;
 
		m_dcPlot.FillRect(CRect(m_rectClient.left,m_rectClient.top,m_rectPlot.left,m_rectClient.bottom), &m_brushBack);

    Invalidate(clear);
	}
}

void COScopeCtrl::DrawCircles(double *x, double *y, double verticalRange,int nSum)
{
	double verticalFactor	= (double)m_nPlotHeight / verticalRange;
	int r = 4;
	CRect rect;
	int topLeftx,topLefty,i;

	if(m_dcPlot.GetSafeHdc() != NULL)
	{
    m_dcPlot.BitBlt(m_rectPlot.left, m_rectPlot.top+1, 
                    m_nPlotWidth, m_nPlotHeight, &m_dcPlot, 
                    m_rectPlot.left, m_rectPlot.top+1, 
                    SRCCOPY) ;


  for(i = 0; i < nSum; i++)
  {
		topLeftx = m_rectPlot.left + (long)((x[i] - m_dLeftLimit)*m_dHorizonFactor) -r/2 ;
		topLefty = m_rectPlot.bottom - (long)(y[i]*verticalFactor)-r/2;	//here the m_dCurrrentPosition is (m_dCurrentPosition -0)
    m_dcPlot.FillSolidRect(topLeftx,topLefty,r,r, m_colors.circles);
	}

  m_dcPlot.FillRect(CRect(m_rectClient.left, m_rectClient.top, m_rectClient.right, m_rectPlot.top+1), &m_brushBack) ;
  m_dcPlot.FillRect(CRect(m_rectClient.left, m_rectPlot.bottom+1, m_rectClient.right+1, m_rectClient.bottom+1), &m_brushBack) ;
  m_dcPlot.FillRect(CRect(m_rectPlot.right, m_rectPlot.top+1, m_rectClient.right+1, m_rectPlot.bottom+1), &m_brushBack) ;

	m_dcPlot.FillRect(CRect(m_rectClient.left,m_rectClient.top,m_rectPlot.left,m_rectClient.bottom), &m_brushBack);
	
	Invalidate();
	}
}

void COScopeCtrl::DrawLeftUnit(CString strYMin, CString strYMax, CString strName)
{
	
	m_strYMin = strYMin;
	m_strYMax = strYMax;
	m_strRightYUnit = strName;
	axisFlag = 1;

}

void COScopeCtrl::SetMoreDigits(int pDigits)
{
	digitsFlag = pDigits;
}


int COScopeCtrl::GetZeroNumber(double p)
{
	int n=0;
	int length;
	int i;
	
	TCHAR zero = '0';	
	CString strTmp;
	p = fabs(p);
	if(p>=1.0)
		return 0;
	strTmp.Format("%f",p);

	strTmp.TrimRight(zero);
	length = strTmp.GetLength();
	for(i=0;i<length-2;i++)
	{
		if(strTmp.GetAt(i+2) == zero)
			n++;
		else
			break;
	}
	n = n+1;

	return n;

}

void COScopeCtrl::SetDotLines(int number)
{
	int nGridPix,i ,n,unitWidth;
	int nTopGridPix, nMidGridPix, nBottomGridPix ;
 
	n=0;

	nMidGridPix    = (m_rectPlot.top + m_rectPlot.bottom)/2 ;
	nTopGridPix    = nMidGridPix - m_nPlotHeight/4 ;
	nBottomGridPix = nMidGridPix + m_nPlotHeight/4 ;

	for (i=m_rectPlot.left; i<m_rectPlot.right; i+=4)
	{
    m_dcGrid.SetPixel (i, nTopGridPix,    m_colors.background) ;
		m_dcGrid.SetPixel (i, nMidGridPix,    m_colors.background) ;
		m_dcGrid.SetPixel (i, nBottomGridPix, m_colors.background) ;
	}

	number = 10;	
	unitWidth = m_nPlotHeight/number;

	for(n=1;n<=number;n++)
	{
		for (i=m_rectPlot.left; i<m_rectPlot.right; i+=4)
		{

			nGridPix = m_rectPlot.bottom - unitWidth*n;
			m_dcGrid.SetPixel (i, nGridPix,   m_colors.grid) ;
			
		}
	}

}

/**Draw a line paralell to x-axis or y-axis
*@type=0,draw a line paralell to x-axis; type=1,draw a line paralell to y-axis
*@value - the logic value, not in pixel
*@pColor - define the color of the line
*@lineType - define the line is dash or solid. 0 - dash, 1 - solid
**/
void COScopeCtrl::DrawLine(int type, double value,COLORREF pColor,int lineType)
{
	int leftX,rightX,topY,bottomY;
	CPen *oldPen ;
  CPen pen;
	if(m_dcPlot.GetSafeHdc() != NULL)
	{
		m_dcPlot.BitBlt(m_rectPlot.left, m_rectPlot.top, 
                    m_nPlotWidth, m_nPlotHeight, &m_dcPlot, 
                    m_rectPlot.left, m_rectPlot.top, 
                    SRCCOPY) ;

    if(lineType == 1)
			pen.CreatePen(PS_SOLID, 0, pColor);
		else if(lineType == 0)
			pen.CreatePen(PS_DASH, 0, pColor);

		oldPen = m_dcPlot.SelectObject(&pen) ;

    if(type == 1){
			leftX = m_rectPlot.left + (long)((value - m_dLeftLimit)*m_dHorizonFactor) ;
			topY = m_rectPlot.top;			
			bottomY = m_rectPlot.bottom;
				// move to the previous point
			m_dcPlot.MoveTo (leftX, topY) ;
			m_dcPlot.LineTo (leftX, bottomY) ;
    }else{
			leftX = m_rectPlot.left;
			rightX = m_rectPlot.right;
			topY = m_rectPlot.bottom - (long)((value - m_dLowerLimit)*m_dVerticalFactor) ;
			m_dcPlot.MoveTo (leftX, topY) ;
			m_dcPlot.LineTo (rightX, topY) ;
		}
	
	
		m_dcPlot.SelectObject(oldPen) ;
		m_penPlot2.Detach();
		m_dcPlot.FillRect(CRect(m_rectClient.left, m_rectClient.top, m_rectClient.right, m_rectPlot.top+1), &m_brushBack) ;
		m_dcPlot.FillRect(CRect(m_rectClient.left, m_rectPlot.bottom+1, m_rectClient.right+1, m_rectClient.bottom+1), &m_brushBack) ;
		m_dcPlot.FillRect(CRect(m_rectPlot.right, m_rectPlot.top+1, m_rectClient.right+1, m_rectPlot.bottom+1), &m_brushBack) ;
		m_dcPlot.FillRect(CRect(m_rectClient.left,m_rectClient.top,m_rectPlot.left,m_rectClient.bottom), &m_brushBack);
		
		Invalidate() ;
	}
}
