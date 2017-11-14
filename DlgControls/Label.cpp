// Label.cpp : implementation file
//

#include "stdafx.h"
#include "../NovacMasterProgram.h"
#include "Label.h"

using namespace DlgControls;

IMPLEMENT_DYNAMIC(CLabel, CStatic)
CLabel::CLabel()
{
	m_textColor = RGB(0, 0, 0);
	m_backgroundColor = RGB(236, 233, 216); // <-- normal Windows background color
	m_backgroundBrush.CreateSolidBrush(m_backgroundColor);
}

CLabel::~CLabel()
{
}


BEGIN_MESSAGE_MAP(CLabel, CStatic)
	ON_WM_CTLCOLOR_REFLECT()
END_MESSAGE_MAP()

/** Sets the text color */
void CLabel::SetForegroundColor(COLORREF rgb){
	m_textColor = rgb;
}

/** Sets the background color */
void CLabel::SetBackgroundColor(COLORREF rgb){
	m_backgroundColor = rgb;
	m_backgroundBrush.DeleteObject();
	m_backgroundBrush.CreateSolidBrush(m_backgroundColor);
}

HBRUSH CLabel::CtlColor(CDC* pDC, UINT nCtlColor){
	// TODO: Return a non-NULL brush if the parent's 
	//handler should not be called

	//set text color
	pDC->SetTextColor(m_textColor);

	//set the text's background color
	pDC->SetBkColor(m_backgroundColor);

  //return the brush used for background this sets control background
  return m_backgroundBrush;
}