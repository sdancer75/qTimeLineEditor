// ZTimeAllocDoc.cpp : implementation of the CZTimeAllocDoc class
//

#include "stdafx.h"
#include "ZTimeAlloc.h"

#include "ZTimeAllocDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CZTimeAllocDoc

IMPLEMENT_DYNCREATE(CZTimeAllocDoc, CDocument)

BEGIN_MESSAGE_MAP(CZTimeAllocDoc, CDocument)
	//{{AFX_MSG_MAP(CZTimeAllocDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CZTimeAllocDoc construction/destruction

CZTimeAllocDoc::CZTimeAllocDoc()
{
	// TODO: add one-time construction code here
	/*
	int i, j;
	for(i=0;i<4;i++)
		for(j=0;j<6;j++)
			MyTestTimeLine.AddTimeSpan(i,j);
		MyTestTimeLine.AddTimeSlice(0,1000,10000, 10.0);

		*/
}

CZTimeAllocDoc::~CZTimeAllocDoc()
{
}

BOOL CZTimeAllocDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CZTimeAllocDoc serialization

void CZTimeAllocDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CZTimeAllocDoc diagnostics

#ifdef _DEBUG
void CZTimeAllocDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CZTimeAllocDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CZTimeAllocDoc commands

ZTimeLine * CZTimeAllocDoc::GetCurrentTimeLine()
{
// better code here is needed
	return & MyTestTimeLine;
}



CString CZTimeAllocDoc::GetActionName(int Action)
{
	CString name;
	name.Format("Action: %d", Action);
	return name;
}

CString CZTimeAllocDoc::GetOwnerName(int Owner)
{
	CString name;
	name.Format("Owner: %d", Owner);
	return name;
}
