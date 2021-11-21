// NovacMasterProgramDoc.cpp : implementation of the CNovacMasterProgramDoc class
//

#include "stdafx.h"
#include "NovacMasterProgram.h"

#include "NovacMasterProgramDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CNovacMasterProgramDoc

IMPLEMENT_DYNCREATE(CNovacMasterProgramDoc, CDocument)

BEGIN_MESSAGE_MAP(CNovacMasterProgramDoc, CDocument)
END_MESSAGE_MAP()


// CNovacMasterProgramDoc construction/destruction

CNovacMasterProgramDoc::CNovacMasterProgramDoc()
{
}

CNovacMasterProgramDoc::~CNovacMasterProgramDoc()
{
}

BOOL CNovacMasterProgramDoc::OnNewDocument()
{
    if (!CDocument::OnNewDocument())
        return FALSE;

    // (SDI documents will reuse this document)

    return TRUE;
}




// CNovacMasterProgramDoc serialization

void CNovacMasterProgramDoc::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
    }
    else
    {
    }
}


// CNovacMasterProgramDoc diagnostics

#ifdef _DEBUG
void CNovacMasterProgramDoc::AssertValid() const
{
    CDocument::AssertValid();
}

void CNovacMasterProgramDoc::Dump(CDumpContext& dc) const
{
    CDocument::Dump(dc);
}
#endif //_DEBUG


// CNovacMasterProgramDoc commands
