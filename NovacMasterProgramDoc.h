// NovacMasterProgramDoc.h : interface of the CNovacMasterProgramDoc class
//


#pragma once

class CNovacMasterProgramDoc : public CDocument
{
protected: // create from serialization only
	CNovacMasterProgramDoc();
	DECLARE_DYNCREATE(CNovacMasterProgramDoc)

// Attributes
public:

// Operations
public:

// Overrides
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// Implementation
public:
	virtual ~CNovacMasterProgramDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};


