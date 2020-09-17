// ZTimeAllocDoc.h : interface of the CZTimeAllocDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_ZTIMEALLOCDOC_H__391B13C1_BF80_46B4_AB4D_E72C6AEFCD57__INCLUDED_)
#define AFX_ZTIMEALLOCDOC_H__391B13C1_BF80_46B4_AB4D_E72C6AEFCD57__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ztimeline.h"

class CZTimeAllocDoc : public CDocument
{
protected: // create from serialization only
	CZTimeAllocDoc();
	DECLARE_DYNCREATE(CZTimeAllocDoc)

// Attributes
public:
	ZTimeLine MyTestTimeLine; // Some Data-structure for holding many things
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CZTimeAllocDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	CString GetOwnerName(int Owner);
	CString GetActionName(int Action);
	ZTimeLine * GetCurrentTimeLine();
	virtual ~CZTimeAllocDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CZTimeAllocDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ZTIMEALLOCDOC_H__391B13C1_BF80_46B4_AB4D_E72C6AEFCD57__INCLUDED_)
