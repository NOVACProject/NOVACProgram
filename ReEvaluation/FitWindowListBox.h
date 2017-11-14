#pragma once

#include "ReEvaluator.h"

// CFitWindowListBox

namespace ReEvaluation{
	class CFitWindowListBox : public CListBox
	{
		DECLARE_DYNAMIC(CFitWindowListBox)

	public:
		CFitWindowListBox();
		virtual ~CFitWindowListBox();

		/** A pointer to the reevaluator that this object modifies */
		CReEvaluator *m_reeval;

		/** Called to populate the fit window list */
		void	PopulateList();

	protected:
		DECLARE_MESSAGE_MAP()

		/** Called when the user presses down the left mouse button */
		afx_msg void OnLButtonDown(UINT nFlags, CPoint point);

		/** Called to show the context menu */
		afx_msg void OnContextMenu(CWnd *pWnd, CPoint pos);

		/** Called to insert a new fit window into the list */
		afx_msg void OnInsertFitWindow();

		/** Called to load a set of fit window from a file */
		afx_msg void OnLoadFitWindows();

		/** Called to save a set of fit windows to file */
		afx_msg void OnSaveFitWindows();

		/** Called to rename a fit window */
		afx_msg void OnRenameWindow();

		/** Called to remove a fit window from the list */
		afx_msg void OnRemoveFitWindow();
	};
}