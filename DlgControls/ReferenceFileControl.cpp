#include "StdAfx.h"
#include "../NovacMasterProgram.h"
#include "referencefilecontrol.h"
#include "../Common/Common.h"

using namespace DlgControls;
using namespace Evaluation;

CReferenceFileControl::CReferenceFileControl(void)
{
  m_window = NULL;
  parent = NULL;
}

CReferenceFileControl::~CReferenceFileControl(void)
{
  m_window = NULL;
  parent = NULL;
}

BEGIN_MESSAGE_MAP(CReferenceFileControl, CGridCtrl)
  ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

/* Called when the user has edited one cell */
void CReferenceFileControl::OnEndEditCell(int nRow, int nCol, CString str){
  CGridCtrl::OnEndEditCell(nRow, nCol, str);  

  int index = nRow - 1;
  if(index < 0 || index > MAX_FIT_WINDOWS)
    return; // TODO - add a message

  // A handle to the reference file
  Evaluation::CReferenceFile &ref = m_window->ref[index];

  // If the name was changed
  if(nCol == 0){
    ref.m_specieName = std::string((LPCSTR)str);
  }

  // If the path was changed
  if(nCol == 1){
    if(strlen(str) != 0){
      FILE *f = fopen(str, "r");
      if(f != 0){
        ref.m_path = std::string((LPCSTR)str);
        m_window->nRef = max(m_window->nRef, index+1);    // update the number of references, if necessary
        fclose(f);
      }else{
        MessageBox("Cannot read that reference file", "Error", MB_OK);
        return;
      }
    }
  }

  // If we're editing the shift and squeeze of an unknown reference, quit
  if(ref.m_specieName.size() == 0 && ref.m_path.size() == 0)
    return;

  // If the shift was changed
  if(nCol == 2){
    ParseShiftOption(ref.m_shiftOption, ref.m_shiftValue, str);
    switch(ref.m_shiftOption){
      case SHIFT_FREE:  SetItemTextFmt(nRow, 2, "free"); break;
      case SHIFT_FIX:   SetItemTextFmt(nRow, 2, "fix to %.4lf", ref.m_shiftValue); break;
      case SHIFT_LINK:  SetItemTextFmt(nRow, 2, "link to %.4lf", ref.m_shiftValue); break;
    }
  }

  // If the squeeze was changed
  if(nCol == 3){
    ParseShiftOption(ref.m_squeezeOption, ref.m_squeezeValue, str);
    switch(ref.m_squeezeOption){
      case SHIFT_FREE:  SetItemTextFmt(nRow, 2, "free"); break;
      case SHIFT_FIX:   SetItemTextFmt(nRow, 2, "fix to %.4lf", ref.m_squeezeValue); break;
      case SHIFT_LINK:  SetItemTextFmt(nRow, 2, "link to %.4lf", ref.m_squeezeValue); break;
    }
  }

  // if this is the last line in the grid, add one more line
  if(nRow == GetRowCount()-1 && GetRowCount() < MAX_N_REFERENCES + 1){
    if(strlen(str) > 0)
      SetRowCount(GetRowCount() + 1);
  }

  return;
}

void CReferenceFileControl::ParseShiftOption(Evaluation::SHIFT_TYPE &option, double &value, CString &str){
  char tmpStr[512];
  char txt[512];
  str.MakeLower();
  sprintf(txt, "%s", (LPCSTR)str);
  char *pt = 0;

  // 1. Shift Fixed 
  if((pt = strstr(txt, "fix to")) || (pt = strstr(txt, "fixed to")) || (pt = strstr(txt, "set to"))){
    option = SHIFT_FIX;
    if(0 == sscanf(pt, "%s to %lf", &tmpStr, &value))
      value = 0;
  }

  // 2. Shift Linked
  if((pt = strstr(txt, "link to")) || (pt = strstr(txt, "linked to"))){
    option = SHIFT_LINK;
    if(0 == sscanf(pt, "%s to %lf", &tmpStr, &value))
      value = 0;
  }

  // 3. Shift free
  if(pt = strstr(txt, "free")){
    option = SHIFT_FREE;
  }
}

void CReferenceFileControl::OnContextMenu(CWnd* pWnd, CPoint point){

  if(this->parent == NULL)
    return;

  CMenu menu;
  VERIFY(menu.LoadMenu(IDR_REEVAL_CONTEXTMENU));
  CMenu* pPopup = menu.GetSubMenu(0);
  ASSERT(pPopup != NULL);

  CCellRange cellRange = GetSelectedCellRange();
  int minRow = cellRange.GetMinRow() - 1;
  int nRows = cellRange.GetRowSpan();

	if(nRows <= 0 && m_window->nRef > 1){ /* nothing selected*/
    pPopup->EnableMenuItem(ID__REMOVE,			MF_DISABLED | MF_GRAYED);
    pPopup->EnableMenuItem(ID__PROPERTIES,	MF_DISABLED | MF_GRAYED);
  }
	if(m_window->nRef == 0){
    pPopup->EnableMenuItem(ID__REMOVE,			MF_DISABLED | MF_GRAYED);
    pPopup->EnableMenuItem(ID__PROPERTIES,	MF_DISABLED | MF_GRAYED);
	}

  pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, parent);
}
