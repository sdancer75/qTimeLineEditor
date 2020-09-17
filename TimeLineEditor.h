/************************************************************************************************************
 
  Copyright (C) 2008 by Paradox Interactibe

 Component: Time Line Editor Interface	

 Date     : July, 2008

 Author(s): George Papaioannou a.k.a sdancer75

 Notes    : Uses Keith Rule's MemDC for flickering avoid
          : Very heavy modified code from the ZTimeLineEditor
          : 


****************************************************************************************************************/



#if !defined(AFX_TIMELINEEDITOR_H__C7B56013_9349_4461_BABD_156E8EA6BBC0__INCLUDED_)
#define AFX_TIMELINEEDITOR_H__C7B56013_9349_4461_BABD_156E8EA6BBC0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define USE_MEMDC

#include "ztimeline.h"


enum EVENTS {

	NOEVENT,
	LEFTHANDLER,
	RIGHTHANDLER,
	SLICEHANDLER,	
	TIMELINEHANDLER,
	TIMELINEHEADER,
	OVERTIMELINEHANDLER,
	OVERSLICE,
	OVERSLICEFXINBODY,
	OVERSLICEFXOUTBODY,
	OVERSLICELEFTHANDLE,
	OVERSLICERIGHTHANDLE,
	OVERSLICEINFXLEFTHANDLE,
	OVERSLICEOUTFXRIGHTHANDLE,
	BODYSLICEFX,
	BODYSLICEFXLEFTHANDLE,
	BODYSLICEFXRIGHTHANDLE,	
}; 

enum MOUSEEVENTS {

	NOBUTTONEVENT,
	LBUTTONDOWN,
	LBUTTONUP,
	RBUTTONDOWN,
	RBUTTONUP,
	DCLICK,
	MBUTTONDOWN,
	MBUTTONUP,

};

enum SCALETIMELINE {

	FULLTIMELINE,
	HALFTIMELINE,
	SCALETOWINDOW,
	CUSTOMSCALE,

};

const int TIMELINEHANDLERWIDTH = 11; 
const int TIMELINEHANDLERHEIGHT = 15;
const int TIMELINEHANDLERTOPMARGIN = 3; //points to y of the timeline handler
const int TIMELINELABELWIDTH = 90; //size of the time label. It shows when you move the timeline handler
const int TIMELINELABELHEIGHT = 15; //height of the label

const int HEADERTOP = 20; //height of the tickers 
const int TICKERFREQMINOR = 10;
const int TICKERFREQMAJOR = 60;
const int HEADERBROWSER = 20; //height of the space where the timeline handler moves
const int TICKMAJORHEIGHT = HEADERTOP;
const int TICKMINORHEIGHT = HEADERTOP;
const int SPANTOPMARGIN = HEADERTOP + 2;
const int TIMELINETOOLBAR = 0; //height of the toolbar if exists
const int ENDPOINT = 7; //how much to subtract before the end of the time line. we use this just to force the handler to stop X pixels behide the physhical end for easer catch

// SLICE Constants
const int SLICEHEIGHT = 20; //for drawing purposes and its the width of the bitmap
const int SLICEBODYTILEWIDTH = 1; //for drawing purposes and its the width of the bitmap
const int SLICEEDGELEFTWIDTH = 3; //for drawing purposes and its the width of the bitmap
const int SLICEEDGERIGHTWIDTH = 3; //for drawing purposes and its the width of the bitmap 
const int SLICEINOUTFXWIDTH = 1; //for drawing purposes and its the width of the bitmap
const int FXTITLEWIDTH = 13; //for drawing purposes and its the width of the bitmap
const int FXTITLEHEIGHT = 13; //for drawing purposes and its the width of the bitmap

const int SLICELEFTHANDLEWIDTH = 2; //width of the slice body handler
const int SLICERIGHTHANDLEWIDTH = 2; //width of the slice body handler
const int SLICEBODYMINWIDTH = 0; //seconds min width when changing the In - Out FX

const int SLICEINFXDURATION = 2; //default in fx duration is Seconds
const int SLICEOUTFXDURATION = 2; //default out fx duration is Seconds






/////////////////////////////////////////////////////////////////////////////
// CTimeLineEditor view
class CTimeLineEditor : public CScrollView
{


	// Final output Operations 
public:
	void	SetTimeMax(int Max);
	
	
	void	Refresh();
	int 	GetTimeLineStateTime();
	void	SetHandlerPos(int time);
	void	CreateNewSlice(int ID, int StartTime, int Duration, int TimeFxInDuration=SLICEINFXDURATION, int TimeFxOutDuration=SLICEOUTFXDURATION, int ActionID = -1);
	BOOL	AddNewFxToSlice(int Span, int Slice, int Time, int Length, double ActionID);
	void	SelectSlice(int ID);
	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTimeLineEditor)
protected:
	virtual BOOL OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll = TRUE);
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnInitialUpdate();     // first time after construct
	virtual BOOL OnScrollBy(CSize sizeScroll, BOOL bDoScroll = TRUE);
	//}}AFX_VIRTUAL

	CTimeLineEditor(); // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CTimeLineEditor)
private:

	// Rectangle Processing, for simpler Rendering procedure

	CRect	GetSpanEdge(int Span);
	CRect	GetSliceHandleRight(int Span, int Time, int Length, int OutFxDuration);
	CRect	GetSliceHandleOutFxRight(int Span, int Time, int Length, int OutFxDuration);
	CRect	GetSliceHandleLeft(int Span, int Time, int Length, int InFxDuration);
	CRect	GetSliceHandleInFxLeft(int Span, int Time, int Length, int InFxDuration);
	CRect	GetSliceFxInBody(int Span, int Time, int Length, int InFxDuration);
	CRect	GetSliceFxOutBody(int Span, int Time, int Length, int OutFxDuration);
	CRect	GetSliceRect(int Span, int Time, int Length);
	CRect	GetSBFxHandleRight(int Span, int Time, int Length);
	CRect	GetSBFxHandleLeft(int Span, int Time, int Length);
	CRect	GetSBFxBodyRect(int Span, int Time, int Length);
	CRect	GetSliceBodyRect(int Span, int Time, int Length, int InFxDuration, int OutFxDuration);
	CRect	GetBckTopRect();
	CRect	GetBckBottomRect();
	CRect	GetTimeRect(int Time);
	void	ExpandRect(CRect & SliceRect);
	CRect   GetTimeLineHandlerRect(int time);
	CRect   GetTimeLineHeaderRect(); //the space where the TLHandler moves


	// Various Helper Functions
	void	UseFont(CFont &font, int height);
	void	UpdateScrollSizes();

	// Position Functions for Scalable Rendering
	int		GetSliceLeft(int Time); // in Pixels
	int		GetSliceRight(int Time, int Length); // in Pixels
	int		GetTimeLineWidth();
	int 	GetTickerFreqMinor();
	int  	GetTickerFreqMajor();
	void	SetTimeLineScale(SCALETIMELINE typeofscale = FULLTIMELINE, int scalefactor = 1); //scalefactor is only valid when CUSTOMSCALE, currently not supported.
	void	SetTimeLineStateTime(int time);
	int		GetScaledTime(int time);//convert time analogous to the scale factor.



private:
	int		GetTimeToX(int Time);
	int		GetTimeHandlerX(int x);
	void	KillCurrentTimer();
	int		round(float num);//just a round function


	
	// Test a mouse click, note: sets TempTime, TempLength, TempWeight
	EVENTS	TestPos(int x, int y, int &Span, int &Slice);
	EVENTS	TestPos(CPoint M, int &Span, int &Slice);
	void	DetectCursorFromObjBellow(CPoint point);

	// inverse position functions
	int		GetTimeFromX(int x);
	int		DetermineSpan(int y);


	// rendering functions

	void	drawTop(CDC *pDC);
	void	drawInFxLeftHandler(CDC *pDC,int YDist, int Time, int Duration);
	void	drawOutFxRightHandler(CDC *pDC,int YDist, int Duration, int EndPoint);
	void	drawTimeSliceMoveEventLabel(CDC *pDC, int Time, int Length, CString str);
	void	drawTimeSplice(CDC *pDC, int Span, int Time, int Length, int TimeFxInDuration, int TimeFxOutDuration);	
	void	drawFxSplice(CDC *pDC, int Span, int Slice);
	void	drawBackground(CDC *pDC, CRect & BckGr);
	void	drawGlobalTimeLineHandler(CDC *pDC, int Time);
	HBITMAP CreateBitmapMask(HBITMAP hSourceBitmap, DWORD dwWidth, DWORD dwHeight, COLORREF crTransColor);	
	void	DrawTransparent(CBitmap *image, CDC *pDC, int x, int y, int Width, int Height, COLORREF crColour);
	void	DrawBitmap(CBitmap *image, CDC *pDC, int x, int y, int Width, int Height, int x2); 
	void	DrawSemiTransparentBitmap(CDC *pDstDC, int x, int y, int nWidth, int nHeight, CDC* pSrcDC, int xSrc, int ySrc);
	void	drawShowSBFxInfoBallon(CDC *pDC,int Time, int Length, CString str, CString strDuration="");//shows an info box when mouse is over of the SBFx of x Seconds



	//General functions
	void	Snap(int &Val);
	int		HitLeftFxSplice(int nNewFxSize);
	int		HitRightFxSplice(int nNewFxSize);


// Attributes
private:

	UINT_PTR		m_nTimer,m_nBallonTimer;
	BOOL			m_bScrollRight;
	BOOL			m_bScrollLeft;
	ZTimeLine		m_TimeLineSpan;
	double			m_dActiveAction;
	int				m_nActiveSpan;
	int				m_nActiveSlice;
	int				m_nTimeLineStateTime; //this is the output time of the custom control
	int				m_MouseX, m_MouseY;



	EVENTS			m_eventState; //state if something has pressed like Left Mouse Button
	EVENTS			m_MouseState; //state from mouse movement to change the mouse cursor if something is under cursor
	MOUSEEVENTS		m_MouseButtonState;//shows the mouse state
	

	bool			m_bSnap;
	int				m_GlobalCurrentTime; // here we keep the time that the timeline handler shows.

	int				m_nTotalHour;
	int				m_nTotalMin;
	int				m_nTotalSec;
	CTimeSpan		m_TotalTime;
	float			m_fDownScaleFactor;
	int				m_SliceMouseX_Minus_SliceStartDistance;
	int				m_nSBFxActive;
	BOOL			m_bShowSBXInfoBallon;

public:
	int				m_nTimeMax; //its the timeline in pixels (=seconds in this case) x1 scale factor
	int				m_nScaledTimeMax; //its the timeline in pixels scaled down by 1/m_fDownScaleFactor in case of HALFTIMELINE,SCALETOWINDOW or CUSTOMSCALE,	
	int				m_nIdealMajorTickPix; //secs for the major tick
	int				CheckWidth;
	SCALETIMELINE	m_sScaleTimeLine;



// Implementation
protected:
	virtual ~CTimeLineEditor();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CTimeLineEditor)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
//	afx_msg void OnNcLButtonUp(UINT nHitTest, CPoint point);
//	afx_msg void OnNcMouseMove(UINT nHitTest, CPoint point);
//	afx_msg void OnNcLButtonUp(UINT nHitTest, CPoint point);
//	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSlicebodyDuration();
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnSlicebodyDelete();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TIMELINEEDITOR_H__C7B56013_9349_4461_BABD_156E8EA6BBC0__INCLUDED_)
