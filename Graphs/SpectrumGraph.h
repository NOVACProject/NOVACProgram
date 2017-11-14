#pragma once
#include "GraphCtrl.h"

namespace Graph{
	class CSpectrumGraph :
		public CGraphCtrl
	{
	public:
		CSpectrumGraph(void);
		~CSpectrumGraph(void);

		/** the size of this graph */
		CRect rect; 

		/** handle to the parent window, for message handling */
		CWnd *parentWnd; 

		/** Current wavelength */
		double	m_curLambda;
		
		/** Current intensity */
		double	m_curIntens;

		/** The lambda and intensity where the left mouse button was pressed
				down the last time. */
		double	lbdLambda, lbdIntens;

		/** true if the user is right now trying to zoom into the graph */
		bool		m_zooming;

		/** True if the user should be able to zoom in the graph, otherwise false */
		bool		m_userZoomableGraph;

		/** The coordinates (lat & long) into which the user wants to zoom. */
		struct plotRange{
			double minLambda;
			double maxLambda;
			double minIntens;
			double maxIntens;
		};
		struct plotRange m_zoomRect;

		/** A DC to draw something on... */
		CDC     m_dcRoute;
		CBitmap *m_pbitmapOldRoute;
		CBitmap m_bitmapRoute;

	DECLARE_MESSAGE_MAP()

		/** Redraws the graph */
		afx_msg	void OnPaint();

		/** Draws a shaded rectangle between with the two given points
				as corners */
		void DrawShadedRect(double lambda1, double intens1, double lambda2, double intens2);

	public:
		/** Called when the user moves the mouse over the graph */
		afx_msg void OnMouseMove(UINT nFlags, CPoint point);

		/* distance calculations */
		afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

		/* distance calculations */
		afx_msg void OnLButtonDown(UINT nFlags, CPoint point);

		/* zooming out */
		afx_msg void OnRButtonDown(UINT nFlags, CPoint point);

		/// ---------------------- OVERLOADED FUNCTIONS ----------------------
		/** Creates the graph */
		virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID=NULL);

		// ------------------ Communication ------------------------

		/** Gets the coordinate values that the user wants to zoom into.
				If the user does not want to zoom, the values in 'rect' will be zero. */
		void	GetZoomRect(struct plotRange &range);

		/** Resets the range of the plot */
		void ResetZoomRect();

	};
}