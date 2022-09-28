#include "StdAfx.h"
#include "SpectrumGraph.h"
#include "../Common/Common.h"

using namespace Graph;

CSpectrumGraph::CSpectrumGraph(void)
{
    m_zoomRect.minLambda = 0.0;
    m_zoomRect.maxLambda = 0.0;
    m_zoomRect.minIntens = 0.0;
    m_zoomRect.maxIntens = 0.0;
}

CSpectrumGraph::~CSpectrumGraph(void)
{
}


BEGIN_MESSAGE_MAP(CSpectrumGraph, CGraphCtrl)
    ON_WM_MOUSEMOVE()
    ON_WM_PAINT()
    ON_WM_LBUTTONUP()
    ON_WM_LBUTTONDOWN()
    ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()

void CSpectrumGraph::OnMouseMove(UINT nFlags, CPoint point)
{
    // TODO: Add your message handler code here and/or call default

    CGraphCtrl::OnMouseMove(nFlags, point);

    double alpha = (point.x - m_rectPlot.left) / (double)(m_rectPlot.right - m_rectPlot.left);

    m_curLambda = alpha * m_axisOptions.first.right + (1 - alpha) * m_axisOptions.first.left;

    alpha = (point.y - m_rectPlot.top) / (double)(m_rectPlot.bottom - m_rectPlot.top);
    m_curIntens = alpha * m_axisOptions.first.bottom + (1 - alpha) * m_axisOptions.first.top;

    //  parentWnd->PostMessage(WM_SHOW_LATLONG);

        // If the user is dragging with the left mouse button pressed
    if (nFlags & MK_LBUTTON) {
        if (!m_userZoomableGraph)
            return; // the user is not allowed to zoom, quit it...

        if (!m_zooming)
            m_zooming = true;

        // in case we haven't established the memory dc's
        CBitmap memBitmap;

        // Copy the m_dcRoute to m_dcPlot!
        m_dcPlot.BitBlt(0, 0, m_nClientWidth, m_nClientHeight, &m_dcRoute, 0, 0, SRCCOPY);

        DrawShadedRect(lbdIntens, lbdLambda, m_curIntens, m_curLambda);
        return;
    }
    if (m_zooming) {
        m_zooming = false;

        // in case we haven't established the memory dc's
        CBitmap memBitmap;

        // Copy the m_dcRoute to m_dcPlot!
        m_dcPlot.BitBlt(0, 0, m_nClientWidth, m_nClientHeight, &m_dcRoute, 0, 0, SRCCOPY);

        DrawShadedRect(0, 0, 0, 0);
    }
}

void CSpectrumGraph::OnLButtonUp(UINT nFlags, CPoint point)
{
    CGraphCtrl::OnLButtonUp(nFlags, point);

    double alpha = (point.x - m_rectPlot.left) / (double)(m_rectPlot.right - m_rectPlot.left);

    m_curLambda = alpha * m_axisOptions.first.right + (1 - alpha) * m_axisOptions.first.left;

    alpha = (point.y - m_rectPlot.top) / (double)(m_rectPlot.bottom - m_rectPlot.top);
    m_curIntens = alpha * m_axisOptions.first.bottom + (1 - alpha) * m_axisOptions.first.top;

    if (m_zooming && m_userZoomableGraph) {
        // Remove any previously drawn squares...
        m_dcPlot.BitBlt(0, 0, m_nClientWidth, m_nClientHeight, &m_dcRoute, 0, 0, SRCCOPY);
        Invalidate(); // <-- Redraw everything
        m_zooming = false;

        m_zoomRect.minIntens = min(m_curIntens, lbdIntens);
        m_zoomRect.maxIntens = max(m_curIntens, lbdIntens);
        m_zoomRect.minLambda = min(m_curLambda, lbdLambda);
        m_zoomRect.maxLambda = max(m_curLambda, lbdLambda);

        parentWnd->PostMessage(WM_ZOOM);
    }
    else {
        //   parentWnd->PostMessage(WM_LBU_IN_GPSPLOT);
    }

}

void CSpectrumGraph::OnLButtonDown(UINT nFlags, CPoint point)
{
    CGraphCtrl::OnLButtonDown(nFlags, point);

    double alpha = (point.x - m_rectPlot.left) / (double)(m_rectPlot.right - m_rectPlot.left);

    m_curLambda = alpha * m_axisOptions.first.right + (1 - alpha) * m_axisOptions.first.left;

    alpha = (point.y - m_rectPlot.top) / (double)(m_rectPlot.bottom - m_rectPlot.top);
    m_curIntens = alpha * m_axisOptions.first.bottom + (1 - alpha) * m_axisOptions.first.top;

    // Remember where the left mouse button was pressed the last time
    lbdLambda = m_curLambda;
    lbdIntens = m_curIntens;

    // if we don't have one yet, set up a memory dc for the plot
    if (m_dcRoute.GetSafeHdc() == NULL)
    {
        CClientDC dc(this);
        m_dcRoute.CreateCompatibleDC(&dc);
        m_bitmapRoute.CreateCompatibleBitmap(&dc, m_nClientWidth, m_nClientHeight);
        m_pbitmapOldRoute = m_dcRoute.SelectObject(&m_bitmapRoute);
    }
    // Copy the m_dcPlot to m_dcRoute!
    m_dcRoute.BitBlt(0, 0, m_nClientWidth, m_nClientHeight, &m_dcPlot, 0, 0, SRCCOPY);

    //  parentWnd->PostMessage(WM_LBD_IN_GPSPLOT);
}

void CSpectrumGraph::OnRButtonDown(UINT nFlags, CPoint point)
{
    CGraphCtrl::OnRButtonDown(nFlags, point);

    // zoom back to default values
    m_zooming = false;

    m_zoomRect.maxIntens = 0.0;
    m_zoomRect.minIntens = 0.0;
    m_zoomRect.maxLambda = 0.0;
    m_zoomRect.minLambda = 0.0;

    parentWnd->PostMessage(WM_ZOOM);
}

BOOL CSpectrumGraph::Create(DWORD dwStyle, const RECT& rect,
    CWnd* pParentWnd, UINT nID)
{
    BOOL result;
    static CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW);

    result = CWnd::CreateEx(WS_EX_CLIENTEDGE | WS_EX_STATICEDGE,
        className, NULL, dwStyle,
        rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
        pParentWnd->GetSafeHwnd(), (HMENU)nID);

    if (result != 0) {
        InvalidateCtrl();
        SetYAxisNumberFormat(FORMAT_BINARY);
    }
    return result;

} // Create


void CSpectrumGraph::OnPaint() {
    CGraphCtrl::OnPaint();
}

/** Draws a shaded rectangle between with the two given points
        as corners */
void CSpectrumGraph::DrawShadedRect(double lat1, double lon1, double lat2, double lon2) {
    double x[5], y[5];
    double maxX, minX, maxY, minY;
    double xFactor, yFactor, offsLeft, offsBottom;
    int curX, curY, prevX, prevY;
    double left = (double)m_rectPlot.left;
    double right = (double)m_rectPlot.right;
    double top = (double)m_rectPlot.top;
    double bottom = (double)m_rectPlot.bottom;
    AxisOptions::FloatRect curAxis; // The current axis (either first or second axis)
    int i;

    // Copy the current pen
    LOGPEN logpen;
    m_penPlot.GetLogPen(&logpen);

    // Get the opposite of the grid color
    BYTE R = (BYTE)255 - GetRValue(m_colors.background);
    BYTE G = (BYTE)255 - GetGValue(m_colors.background);
    BYTE B = (BYTE)255 - GetBValue(m_colors.background);
    COLORREF complement = RGB(R, G, B);

    // Make a new pen with this color
    m_penPlot.DeleteObject();
    m_penPlot.CreatePen(PS_DOT, 1, complement);

    // Draw the rectangle
    x[4] = x[0] = x[1] = lon1;
    x[2] = x[3] = lon2;
    y[4] = y[0] = y[3] = lat1;
    y[1] = y[2] = lat2;

    // make sure there's a memory dc for the plot
    if (m_dcRoute.GetSafeHdc() == NULL)
        return;

    GetDataRange(x, y, 5, maxX, minX, maxY, minY);

    // Get the current axis
    curAxis = m_axisOptions.first;

    // ------------ CALCULATE THE TRANSFORM FROM DATA POINT TO PIXELS ---------------
    GetTransform(offsLeft, offsBottom, xFactor, yFactor, curAxis);

    m_dcPlot.SelectObject(&m_penPlot);

    // The starting point
    prevX = (int)(left + (x[0] - offsLeft) * xFactor);
    prevY = (int)(bottom - (y[0] - offsBottom) * yFactor);

    for (i = 0; i < 5; ++i) {
        // Calculate the next point...
        curX = (int)(left + xFactor * (x[i] - offsLeft));
        curY = (int)(bottom - (y[i] - offsBottom) * yFactor);

        // Draw connected lines
        m_dcPlot.MoveTo(prevX, prevY);
        m_dcPlot.LineTo(curX, curY);

        prevX = curX;
        prevY = curY;
    }

    // Restore the pen
    m_penPlot.DeleteObject();
    m_penPlot.CreatePenIndirect(&logpen);

    FinishPlot();

    // Invalidate the active region
    // TODO: Use InvalidateRect instead!!
    Invalidate();

}

/** Gets the coordinate values that the user wants to zoom into.
        If the user does not want to zoom, the values in 'rect' will be zero. */
void CSpectrumGraph::GetZoomRect(struct plotRange& range) {
    if (m_zoomRect.minIntens == m_zoomRect.maxIntens && m_zoomRect.minIntens == 0) {
        range.maxIntens = range.maxLambda = range.minIntens = range.minLambda = 0.0;
        return;
    }
    else {
        range.maxIntens = m_zoomRect.maxIntens;
        range.maxLambda = m_zoomRect.maxLambda;
        range.minIntens = m_zoomRect.minIntens;
        range.minLambda = m_zoomRect.minLambda;
        return;
    }
}

/** Resets the range of the plot */
void CSpectrumGraph::ResetZoomRect() {
    m_zoomRect.minIntens = 0.0;
    m_zoomRect.maxIntens = 0.0;
    m_zoomRect.minLambda = 0.0;
    m_zoomRect.maxLambda = 0.0;

    return;
}