#pragma once


// CLabel
namespace DlgControls{
	class CLabel : public CStatic
	{
		DECLARE_DYNAMIC(CLabel)

	public:
		CLabel();
		virtual ~CLabel();

		/** Sets the text color */
		void SetForegroundColor(COLORREF rgb);

		/** Sets the background color */
		void SetBackgroundColor(COLORREF rgb);

		/** Called when the window should be updated. 
				Used to change the colors of the text/backgroun. */
		afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);

	protected:
		DECLARE_MESSAGE_MAP()

		/** The color of the text */
		COLORREF	m_textColor;

		/** The color of the background */
		COLORREF	m_backgroundColor;

		/** The brush used for drawing the background */
		CBrush		m_backgroundBrush;
	};
}