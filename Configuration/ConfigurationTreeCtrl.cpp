// ConfigurationTreeCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "../NovacMasterProgram.h"
#include "ConfigurationTreeCtrl.h"

using namespace ConfigurationDialog;

// CConfigurationTreeCtrl

IMPLEMENT_DYNAMIC(CConfigurationTreeCtrl, CTreeCtrl)
CConfigurationTreeCtrl::CConfigurationTreeCtrl()
{
	m_configuration = NULL;
}

CConfigurationTreeCtrl::~CConfigurationTreeCtrl()
{
	m_configuration = NULL;
}


BEGIN_MESSAGE_MAP(CConfigurationTreeCtrl, CTreeCtrl)
END_MESSAGE_MAP()

/** Updates the tree control */
void CConfigurationTreeCtrl::UpdateTree(){
	if(m_configuration == NULL)
		return;

	UINT count = GetCount();
	HTREEITEM hTree = GetRootItem();

	for(UINT i = 0; i < count; ++i){
		SetItemText(hTree, m_configuration->scanner[i].volcano);

		hTree = GetNextItem(hTree, TVGN_NEXT);
	}
}

void CConfigurationTreeCtrl::PopulateTreeControl(){
	unsigned int i, j, k;

	if(m_configuration == NULL)
		return;
	if(m_configuration->scannerNum == 0)
		return;

	DeleteAllItems();

	int nVolcanoes = 1; // there's at least one volcano if there's one instrument
	CString volcanoes[32]; // the available volcanoes
	int scanners[32];	// an index showing which volcano scanner 'i' belongs to
	memset(scanners, -1, 32*sizeof(int));

	// Get the name of the first volcano
	volcanoes[0].Format(m_configuration->scanner[0].volcano);
	scanners[0] = 0;

	// First check if all scanners belong to the same volcano
	if(m_configuration->scannerNum == 1){
		// There's only one scanner. It obviously monitors only one volcano
		nVolcanoes = 1;
	}else{
		// Compare each volcano-name to the names in the list,
		//	if this scanner monitors a volcano not in the list - then insert it
		for(i = 1; i < m_configuration->scannerNum; ++i){
			bool found = false;
			for(j = 0; j <= nVolcanoes; ++j){
				if(Equals(m_configuration->scanner[i].volcano, volcanoes[j])){
					found = true;
					scanners[i] = j;
					break;
				}else{
					
				}
			}
			if(!found){
				volcanoes[nVolcanoes].Format(m_configuration->scanner[i].volcano);
				scanners[i] = nVolcanoes;
				++nVolcanoes;
			}
		}
		// Done! We now got a list of all volcanoes
	}

	HTREEITEM hTree;
	HTREEITEM *firstItem = NULL;
	int first = true;
	// now loop through each of the volcanoes and for each, insert the 
	//	scanners that belong to this volcano
	for(i = 0; i < nVolcanoes; ++i){
		hTree = InsertItem(volcanoes[i]);
		
		for(j = 0; j < m_configuration->scannerNum; ++j){
			if(scanners[j] == i){
				for (k = 0; k < m_configuration->scanner[j].specNum; ++k) {
					HTREEITEM item = InsertItem(m_configuration->scanner[j].spec[k].serialNumber, hTree);
					if (first) {
						firstItem = &item;
						first = false;
					}
				}
			}
		}
		Expand(hTree, TVE_EXPAND);
	}

	ModifyStyle(0, TVS_LINESATROOT);

	// Select the first scanner
	if(firstItem != NULL){
		SelectItem(*firstItem);		
	}
}

// CConfigurationTreeCtrl message handlers
void CConfigurationTreeCtrl::GetCurScanAndSpec(int &curScanner, int &curSpec){
	unsigned int k, j;

	curSpec = curScanner = -1;

	// Get the selected item.
	HTREEITEM hTree = GetSelectedItem();
	if(hTree == NULL){
		// nothing selected
		return;
	}

	// if this is a volcano, show the first child
	if(ItemHasChildren(hTree)){
		hTree = GetNextItem(hTree, TVGN_CHILD);
		SelectItem(hTree);
		return;
	}

	// Get the serial-number of the spectrometer
	CString &serial = GetItemText(hTree);

	// look for the serial-number in the list of scanners
	for(k = 0; k < m_configuration->scannerNum; ++k){
		for(j = 0; j < m_configuration->scanner[k].specNum; ++j){
			if(Equals(serial, m_configuration->scanner[k].spec[j].serialNumber)){
				curScanner = k;
				curSpec = j;
				return;
			}
		}
	}

	return;
}
