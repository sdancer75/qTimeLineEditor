/************************************************************************************************************
 
  Copyright (C) 2008 by Paradox Interactive

 Component: Time Line Editor Interface

 Date     : July, 2008

 Author(s): George Papaioannou a.k.a sdancer75

 Notes    : Uses Keith Rule's MemDC for flickering avoid
          : Very heavy (or all) modified code from the ZTimeLineEditor
          : 
		  :Small scaling functions, does not fully implemented. In the drawTimeSplice is needed to adjust
		   the start point in pixels as well as the length since the float divisions leave remaining that 
		   affects the visualizations especially in case where InDurationFx + OutDurationFx = Length of the slice


****************************************************************************************************************/


#include "stdafx.h"
#include "ZTimeAlloc.h"
#include "TimeLineEditor.h"
#include "ZTimeAllocDoc.h"
#include "ChangeDurationDlg.h"
#include <math.h>
#include "MemDC.h"

/*//////////////////////////////////////////////////////////////*\
** MACROS
**\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CTimeLineEditor, CScrollView)

/*//////////////////////////////////////////////////////////////*\
** Automatic Object m_eventState Management (C/CC/D)'tors
**\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

CTimeLineEditor::CTimeLineEditor()
{
	m_nActiveSpan = -1;
	m_nActiveSlice = -1; //no slice is selected
	m_nTimeLineStateTime = 0;
	m_nTimeMax = 0; // Time Line seconds

	m_eventState = NOEVENT;	
	m_nIdealMajorTickPix = TICKERFREQMAJOR; //seconds
	m_nTimer = m_nBallonTimer = 0;
	
	m_bSnap = FALSE;
	m_bShowSBXInfoBallon = FALSE;


	m_MouseState = NOEVENT;
	m_MouseButtonState = NOBUTTONEVENT;
	m_MouseX = m_MouseY = 0;
	m_bScrollRight = m_bScrollLeft = FALSE;
	
	m_GlobalCurrentTime = 0;	
	m_fDownScaleFactor = 0; //not initialised in this case. If its true it will be down in the onDraw



}

CTimeLineEditor::~CTimeLineEditor()
{
}

/*//////////////////////////////////////////////////////////////*\
** Message Map
**\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

BEGIN_MESSAGE_MAP(CTimeLineEditor, CScrollView)
	//{{AFX_MSG_MAP(CTimeLineEditor)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_ERASEBKGND()
	ON_WM_RBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_CREATE()
	ON_WM_SETCURSOR()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
	ON_WM_SIZE()
	ON_COMMAND(ID_SLICEBODY_DURATION, &CTimeLineEditor::OnSlicebodyDuration)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_SLICEBODY_DELETE, &CTimeLineEditor::OnSlicebodyDelete)
END_MESSAGE_MAP()

/*//////////////////////////////////////////////////////////////*\
** Message Handlers: Basic
**\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

void CTimeLineEditor::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();
	
	SetTimeMax(3600);//give the time in seconds
	SetTimeLineScale();

	//Currently in this version dont support smaller visual ranges
	//SetTimeLineScale(HALFTIMELINE); //initialize to default - FULLTIMELINE, else call it directly from the application
	//SetTimeLineScale(SCALETOWINDOW); //initialize to default - FULLTIMELINE, else call it directly from the application
	
	//CreateNewSlice(ID=Span=Slice, StartTime, Length(Duration),InFxDuration,OutFxDuration)
	CreateNewSlice(0,35,180,60,10);//give the time in seconds
	//AddNewFxToSlice(Span, Slice, StartTime, Length(Duration),FxID)
	
	AddNewFxToSlice(m_nActiveSpan, m_nActiveSlice, 70,5,1);
	AddNewFxToSlice(m_nActiveSpan, m_nActiveSlice, 150,20,1);

	
	

	
}

// since I use Keith's Rule CMemoryDC, this would create flickering
//  and I already draw on the entire full screen
BOOL CTimeLineEditor::OnEraseBkgnd(CDC* pDC) 
{
	return TRUE;
}

int CTimeLineEditor::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CScrollView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	return 0;
}

/************************************************************************************************
OnLButtonDown

Takes care the Left mouse button events

**************************************************************************************************/

// relates to selection and checkboxes
void CTimeLineEditor::OnLButtonDown(UINT nFlags, CPoint point) 
{
	
	int			nStart, nLength, nFxInDuration, nFxOutDuration;
	double		dActionID;
	CPoint		M;
	CRect		FxSliceRect;
	int			start,length;
	double		actionID;	
	POINT		CursorPoint;

	
	
	m_eventState = NOEVENT; // reset the state
	m_MouseButtonState = LBUTTONDOWN;
		
	GetCursorPos (&CursorPoint);
	ScreenToClient(&CursorPoint);

	M.x = CursorPoint.x + GetScrollPosition().x; // augment the point
	M.y = CursorPoint.y + GetScrollPosition().y; // augment the point

	m_MouseX = point.x = CursorPoint.x;
	m_MouseY = point.y = CursorPoint.y;



	//CString str;
	//str.Format("OnLButtonDown X=%d	Y=%d\n",M.x,M.y);
	//TRACE(str);

	m_eventState = TestPos(M, m_nActiveSpan, m_nActiveSlice); //Did I click on something imporant



	if(m_eventState != NOEVENT)
	{
		switch(m_eventState)
		{

		case TIMELINEHANDLER:
			SetCapture();
			Refresh();
			break;
		case TIMELINEHEADER:
			SetCapture();	
			m_GlobalCurrentTime = M.x;
			Refresh();
			break;
		case OVERSLICE:
		case OVERSLICEFXINBODY:
		case OVERSLICEFXOUTBODY:
			SetCapture();	
			m_eventState = OVERSLICE;

			//now find the distance between the mouseXY hit and the start of the slice.
			//we do this because we dont want the mouseXY to comply with the slice Start
			//So the slice start is the its the current mouse XY - this difference
			m_TimeLineSpan.GetSlice(m_nActiveSpan,m_nActiveSlice,nStart,nLength,nFxInDuration,nFxOutDuration,dActionID);
			m_SliceMouseX_Minus_SliceStartDistance = (int) (M.x* m_fDownScaleFactor) - nStart;
			Refresh();
			break;
		case OVERSLICELEFTHANDLE:			
			Refresh();
			break;
		case OVERSLICERIGHTHANDLE:			
			Refresh();
			break;
		case OVERSLICEINFXLEFTHANDLE:			
			Refresh();
			break;
		case OVERSLICEOUTFXRIGHTHANDLE:			
			Refresh();
			break;
		case BODYSLICEFX:			
			m_TimeLineSpan.GetFxToSlice(m_nActiveSpan,m_nActiveSlice,m_nSBFxActive,start,length,actionID);
			m_SliceMouseX_Minus_SliceStartDistance = (int) (M.x* m_fDownScaleFactor) - start;
			Refresh();
			break;
		case BODYSLICEFXLEFTHANDLE:	
			m_TimeLineSpan.GetSlice(m_nActiveSpan,m_nActiveSlice,nStart,nLength,nFxInDuration,nFxOutDuration,dActionID);
			m_TimeLineSpan.GetFxToSlice(m_nActiveSpan,m_nActiveSlice,m_nSBFxActive,start,length,actionID);			
			
			//================ set the mouse cursor just in the left side handler.============ 
			//remember we have 4-pixel width thumb nail. So glue the mouse cursor
			//just in the left edge of the SBFx.


			CursorPoint.x = nStart+start;
			CursorPoint.y = point.y;
			
			m_MouseX = point.x = CursorPoint.x;
			m_MouseY = point.y = CursorPoint.y;

			m_SliceMouseX_Minus_SliceStartDistance = (int) (CursorPoint.x * m_fDownScaleFactor) - start;			
			
			ClientToScreen(&CursorPoint);
			SetCursorPos(CursorPoint.x-GetScrollPosition().x,CursorPoint.y-GetScrollPosition().y);
			//=================================================================================

			Refresh();
			break;
		case BODYSLICEFXRIGHTHANDLE:	
			m_TimeLineSpan.GetSlice(m_nActiveSpan,m_nActiveSlice,nStart,nLength,nFxInDuration,nFxOutDuration,dActionID);
			m_TimeLineSpan.GetFxToSlice(m_nActiveSpan,m_nActiveSlice,m_nSBFxActive,start,length,actionID);

			//================ set the mouse cursor just in the right side handler.============ 
			//remember we have 4-pixel width thumb nail. So glue the mouse cursor
			//just in the right edge of the SBFx.


			CursorPoint.x = nStart+start+length;
			CursorPoint.y = point.y;
			
			m_MouseX = point.x = CursorPoint.x;
			m_MouseY = point.y = CursorPoint.y;

			m_SliceMouseX_Minus_SliceStartDistance = (int) (CursorPoint.x * m_fDownScaleFactor) - (start+length);			
			
			ClientToScreen(&CursorPoint);
			SetCursorPos(CursorPoint.x-GetScrollPosition().x,CursorPoint.y-GetScrollPosition().y);
			//=================================================================================

			
			Refresh();
			break;
			
		default:
			break;	
		}
	}
	PostMessage(WM_SETCURSOR);	
	CScrollView::OnLButtonDown(nFlags, point);
}
/***************************************************************************************************
OnLButtonUp

Takes care the Left button up events

******************************************************************************************************/
void CTimeLineEditor::OnLButtonUp(UINT nFlags, CPoint point) 
{

	CRect		ClientArea;


	GetClientRect(ClientArea);
	
	m_MouseButtonState = LBUTTONUP;

	CPoint M = point + GetScrollPosition(); // augment the point to rendering space;

	m_MouseX = point.x;
	m_MouseY = point.y;



	//do this because if mouse is outside the client area (timer scroll enabled) and you release the LButton then the handler is not shown
	//because it is a few seconds in front of the viewable area. Adjust the time a few backward
	//The same is true when we have negative values or scroll to the left.
	if ( (point.x > ClientArea.right) && (m_bScrollRight) &&  (m_eventState==TIMELINEHANDLER))
		m_GlobalCurrentTime = GetScrollPosition().x + ClientArea.right-(TIMELINEHANDLERWIDTH / 2);

	if ( (point.x < ClientArea.left) && (m_bScrollLeft)  &&  (m_eventState==TIMELINEHANDLER))
		m_GlobalCurrentTime = GetScrollPosition().x + (TIMELINEHANDLERWIDTH / 2);


	if (GetCapture () == this)
         ReleaseCapture ();
	
	KillCurrentTimer();
	m_bScrollRight = m_bScrollLeft = FALSE;

	//set the state.Must be before the DetectCursorFromObjBelow
	m_eventState = NOEVENT;

	DetectCursorFromObjBellow(M);
	
	


	Refresh();



	CScrollView::OnLButtonUp(nFlags, point);
}

/*****************************************************************************************************
OnLButtonDblClk


Takes care the double clicks if any needed.

********************************************************************************************************/

void CTimeLineEditor::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	
	m_MouseButtonState = DCLICK;	

	
	CScrollView::OnLButtonDblClk(nFlags, point);
}

/*********************************************************************************************************
OnMouseMove

Take cares the mouse movements as well as selections ie change Objectslice positions, durations etc

**********************************************************************************************************/

void CTimeLineEditor::OnMouseMove(UINT nFlags, CPoint point) 
{

	CRect		rect,FxSliceRect;
	int			nStart, nLength, nFxInDuration, nFxOutDuration,nSliceMoveNewStart;
	double		dActionID;
	int			nNewFxSize,nNewLength;
	int			newx;
	int			StartOfFirstSBFx,LengthOfLastSBFx;	
	int			nSBFxMoveNewStart,nSBFxNewLength;
	int			start,length,Neighbouring_start,Neighbouring_length;
	double		actionID,Neighbouring_actionID;

	int			nTemp_start, nTemp_length;
	double		dTemp_actionID;
	int			newSBFxActive;
	POINT		CursorPoint;
	CPoint		M;



	
	//this is used when the user releases the L-R-M MouseButton outside from the client area.

	if ( ( !(nFlags & MK_LBUTTON) ) && (m_MouseButtonState == LBUTTONDOWN) )
	{
		m_MouseButtonState = NOBUTTONEVENT;
		m_eventState = NOEVENT;
		Refresh();

	}
	
	if ( ( !(nFlags & MK_RBUTTON) ) && (m_MouseButtonState == RBUTTONDOWN) )
	{
		m_MouseButtonState = NOBUTTONEVENT;
		m_eventState = NOEVENT;
		Refresh();

	}

	if ( ( !(nFlags & MK_MBUTTON) ) && (m_MouseButtonState == MBUTTONDOWN) )
	{
		m_MouseButtonState = NOBUTTONEVENT;
		m_eventState = NOEVENT;
		Refresh();

	}

	
	
	GetCursorPos (&CursorPoint);
	ScreenToClient(&CursorPoint);

	M.x = CursorPoint.x + GetScrollPosition().x; // augment the point
	M.y = CursorPoint.y + GetScrollPosition().y; // augment the point

	m_MouseX = point.x = CursorPoint.x;
	m_MouseY = point.y = CursorPoint.y;


	//CString str;
	//str.Format("onMouseMove X=%d	Y=%d\n",M.x,M.y);
	//TRACE(str);


	// the left mouse button states
	if (m_eventState==BODYSLICEFX) 
	{
				newSBFxActive = m_nSBFxActive;

				GetClientRect(&rect);

				m_TimeLineSpan.GetSlice(m_nActiveSpan,m_nActiveSlice,nStart,nLength,nFxInDuration,nFxOutDuration,dActionID);
				
				//the m_nSBFxActive which shows the selected SBFx was set in the LMouseButtonDown event
				m_TimeLineSpan.GetFxToSlice(m_nActiveSpan,m_nActiveSlice,m_nSBFxActive,start,length,actionID);

				

				if ( (m_MouseX >=0) && (m_MouseX <= rect.Width()- (m_sScaleTimeLine == SCALETOWINDOW ? ENDPOINT : 0) ) )
				{
					//compute the new Start. Use the uncompressed timeline because the drawTimeSlice will convert the scaled values
					//automatically. So in this point nSliceMoveNewStart is pointing to the FULLSCALE timeline.
					//nSliceMoveNewStart =  nStart + round( (M.x-cpOldMouseXY.x)* m_fDownScaleFactor);						
					nSBFxMoveNewStart =  (int) (M.x * m_fDownScaleFactor) - m_SliceMouseX_Minus_SliceStartDistance;
					
					if (m_bSnap)
					{
							Snap(nSBFxMoveNewStart);

					}
					


					//check if we are out of our space					
					if ( (nSBFxMoveNewStart + length) > (nLength -nFxOutDuration) )
						  nSBFxMoveNewStart = nLength -nFxOutDuration - length;
										
					if (nSBFxMoveNewStart < (nFxInDuration ))
						  nSBFxMoveNewStart = nFxInDuration;

					//now check of collisions with other SBFxs
					//first the Left part. If its zero the its the first so don't make any computations
					if (m_nSBFxActive > 0)
					{
						m_TimeLineSpan.GetFxToSlice(m_nActiveSpan,m_nActiveSlice,m_nSBFxActive-1,nTemp_start,nTemp_length,dTemp_actionID);
						if (nSBFxMoveNewStart <= (nTemp_start+nTemp_length ) )
						{
								
							if ( (nSBFxMoveNewStart+length) <= nTemp_start)
								newSBFxActive=m_nSBFxActive-1;
							else
								nSBFxMoveNewStart = nTemp_start+nTemp_length;
						}

					}
					//this is the right part
					if ( (m_TimeLineSpan.GetSliceFxCount(m_nActiveSpan,m_nActiveSlice)-1) > 0 )
					{

						if ( m_nSBFxActive < (m_TimeLineSpan.GetSliceFxCount(m_nActiveSpan,m_nActiveSlice)-1) )
						{

							m_TimeLineSpan.GetFxToSlice(m_nActiveSpan,m_nActiveSlice,m_nSBFxActive+1,nTemp_start,nTemp_length,dTemp_actionID);

							//if mouse check is inside the next SBFx then stop it else if the user continue to drag the SBFx and the start pos 
							//of the selected block exceed the length of the previous block then jump it
							if ( (nSBFxMoveNewStart+length) >= nTemp_start)
							{
								if 	(nSBFxMoveNewStart < (nTemp_start+nTemp_length))							
									nSBFxMoveNewStart = nTemp_start - length;
								else
								//we have to update the m_nSBFxActive since with the editFxToSlice we sort every block and after the jump
								//the current selected Slice Body Fx will become the previous one.
									newSBFxActive=m_nSBFxActive+1;
							}

						}

					}



				}
				
			
				
				
				m_TimeLineSpan.EditFxToSlice(m_nActiveSpan,m_nActiveSlice,m_nSBFxActive,nSBFxMoveNewStart,length,actionID);	
				
				m_nSBFxActive = newSBFxActive; 

				Refresh();
				UpdateWindow();
		
	}

	if(m_eventState==BODYSLICEFXLEFTHANDLE)
	{
		
		m_TimeLineSpan.GetSlice(m_nActiveSpan,m_nActiveSlice,nStart,nLength,nFxInDuration,nFxOutDuration,dActionID);
		//the m_nSBFxActive which shows the selected SBFx was set in the LMouseButtonDown event
		m_TimeLineSpan.GetFxToSlice(m_nActiveSpan,m_nActiveSlice,m_nSBFxActive,start,length,actionID);
		

		nNewFxSize =  (int) (M.x * m_fDownScaleFactor) - ( nStart+start);
		nSBFxNewLength = length - nNewFxSize;

		
		//compute the new Start. Use the uncompressed timeline because the drawTimeSlice will convert the scaled values
		//automatically. So in this point nSliceMoveNewStart is pointing to the FULLSCALE timeline.							
		nSBFxMoveNewStart =  (int) (M.x * m_fDownScaleFactor) - m_SliceMouseX_Minus_SliceStartDistance;
		
		if (m_bSnap)
		{
				Snap(nSBFxMoveNewStart);

		}
		
		//check if we are out of our space					
		if ( (nSBFxNewLength) < 1  )
		{
			   nSBFxMoveNewStart = start+length-1;
			   nSBFxNewLength = 1;	
		}
							
		if (nSBFxMoveNewStart < (nFxInDuration ))
		{
			  nSBFxMoveNewStart = nFxInDuration;		
			  nSBFxNewLength = (start+length) - nSBFxMoveNewStart;
		}

		//check collision with the previous SBFx
		if  ( m_nSBFxActive > 0  )
		{
			m_TimeLineSpan.GetFxToSlice(m_nActiveSpan,m_nActiveSlice,m_nSBFxActive-1,Neighbouring_start,Neighbouring_length,Neighbouring_actionID);
			if (nSBFxMoveNewStart < (Neighbouring_start+Neighbouring_length))
			{
					nSBFxMoveNewStart = Neighbouring_start+Neighbouring_length;
					nSBFxNewLength = (start+length) - nSBFxMoveNewStart;
			}
			
		}
		

		m_TimeLineSpan.EditFxToSlice(m_nActiveSpan,m_nActiveSlice,m_nSBFxActive,nSBFxMoveNewStart,nSBFxNewLength,actionID);


		Refresh();
		UpdateWindow();

	}

	if(m_eventState==BODYSLICEFXRIGHTHANDLE)
	{
		
		m_TimeLineSpan.GetSlice(m_nActiveSpan,m_nActiveSlice,nStart,nLength,nFxInDuration,nFxOutDuration,dActionID);
		//the m_nSBFxActive which shows the selected SBFx was set in the LMouseButtonDown event
		m_TimeLineSpan.GetFxToSlice(m_nActiveSpan,m_nActiveSlice,m_nSBFxActive,start,length,actionID);
		

		nNewFxSize =  (int) (M.x * m_fDownScaleFactor) - ( nStart+start+length);
		nSBFxNewLength = length + nNewFxSize;

		
		//compute the new Start. Use the uncompressed timeline because the drawTimeSlice will convert the scaled values
		//automatically. So in this point nSliceMoveNewStart is pointing to the FULLSCALE timeline.							
		nSBFxMoveNewStart =  (int) (M.x * m_fDownScaleFactor) - m_SliceMouseX_Minus_SliceStartDistance;
		
		if (m_bSnap)
		{
				Snap(nSBFxMoveNewStart);

		}
	
		//check if we are out of our space					
		if ( (nSBFxNewLength) < 1  )
		{
			   
			   nSBFxNewLength = 1;	
		}
							
		if ((nStart+start+nSBFxNewLength) > (nStart + nLength - nFxOutDuration ))
		{
			  
			  nSBFxNewLength = nSBFxNewLength - ( (nStart+start+nSBFxNewLength) - (nStart + nLength -nFxOutDuration ));
			  
			 
		}

		//check collision with the next SBFx
		if  ( m_nSBFxActive < (m_TimeLineSpan.GetSliceFxCount(m_nActiveSpan,m_nActiveSlice)-1)  )
		{
			m_TimeLineSpan.GetFxToSlice(m_nActiveSpan,m_nActiveSlice,m_nSBFxActive+1,Neighbouring_start,Neighbouring_length,Neighbouring_actionID);
			if ((start + nSBFxNewLength) > Neighbouring_start)
					nSBFxNewLength = Neighbouring_start - start;
			
		}


		m_TimeLineSpan.EditFxToSlice(m_nActiveSpan,m_nActiveSlice,m_nSBFxActive,start,nSBFxNewLength,actionID);


		Refresh();
		UpdateWindow();

	}

   
	// the left mouse button states
	if ( (m_eventState==OVERSLICE) || (m_eventState==OVERSLICEFXINBODY) || (m_eventState==OVERSLICEFXOUTBODY) )
	{
		
			
			GetClientRect(&rect);
			
			//Scroll window in any case except when scale = SCALETOWINDOW
			//automatically scroll when the handler reach the edges;
			if  ( ( point.x >= rect.Width() ) && (m_sScaleTimeLine != SCALETOWINDOW) )
			{
				

				m_bScrollRight = TRUE;
				m_bScrollLeft = FALSE;
				if (m_nTimer == 0)
					m_nTimer = SetTimer (IDT_TIMER1, 10, NULL);
				


			}
			//Scroll window in any case except when scale = SCALETOWINDOW
			else if ( (point.x < 0) && (m_sScaleTimeLine != SCALETOWINDOW) )
			{
				

				m_bScrollLeft = TRUE;
				m_bScrollRight = FALSE;
				if (m_nTimer == 0)
					m_nTimer = SetTimer (IDT_TIMER1, 10, NULL);


			}
			else 
			// if no scroll do the normal job
			{
				
				
				m_TimeLineSpan.GetSlice(m_nActiveSpan,m_nActiveSlice,nStart,nLength,nFxInDuration,nFxOutDuration,dActionID);

				if ( (m_MouseX >=0) && (m_MouseX <= rect.Width()- (m_sScaleTimeLine == SCALETOWINDOW ? ENDPOINT : 0) ) )
				{
					//compute the new Start. Use the uncompressed timeline because the drawTimeSlice will convert the scaled values
					//automatically. So in this point nSliceMoveNewStart is pointing to the FULLSCALE timeline.
					//nSliceMoveNewStart =  nStart + round( (M.x-cpOldMouseXY.x)* m_fDownScaleFactor);						
					nSliceMoveNewStart = (int) (M.x * m_fDownScaleFactor) - m_SliceMouseX_Minus_SliceStartDistance;
					
					if (m_bSnap)
					{
							Snap(nSliceMoveNewStart);

					}
					


					//check if we are out of our space					
					if ( (nSliceMoveNewStart + nLength) > m_nTimeMax)
						  nSliceMoveNewStart = m_nTimeMax - nLength;
										
					if (nSliceMoveNewStart < 0)
						  nSliceMoveNewStart = 0;



				}
				
				//do these checks since we use the SetCapture / ReleaseCapture Mouse and we dont want artefacts
				//but only in case that we have scaled to SCALETOWINDOW since in different case we want to scroll and
				//we treat such messages totally different.
				else if ( (m_MouseX < 0 ) && (m_sScaleTimeLine == SCALETOWINDOW) ) 
					nSliceMoveNewStart = 0;
				else if ( (m_MouseX > (rect.Width()-ENDPOINT) )  && (m_sScaleTimeLine == SCALETOWINDOW) ) 
					nSliceMoveNewStart = m_nTimeMax - nLength;
				
				
					
				m_TimeLineSpan.Edit(m_nActiveSpan,m_nActiveSlice, nSliceMoveNewStart ,nLength,nFxInDuration,nFxOutDuration,dActionID);

				
				m_bScrollRight = FALSE;
				m_bScrollLeft = FALSE;

				Refresh();
				UpdateWindow();
			}
				

			
	
	}




	if(m_eventState==OVERSLICELEFTHANDLE)
	{
		
		m_TimeLineSpan.GetSlice(m_nActiveSpan,m_nActiveSlice,nStart,nLength,nFxInDuration,nFxOutDuration,dActionID);
		

		nNewFxSize = ((int) (M.x * m_fDownScaleFactor) - ( nStart+nFxInDuration));




		if ( (nFxInDuration+nNewFxSize) <= 0 )
			nNewFxSize = - nFxInDuration;
		//check if we hit a SBFx from the Left edge
		else if ( (StartOfFirstSBFx=HitLeftFxSplice(nStart+nFxInDuration+nNewFxSize)) != -1)
			nNewFxSize = StartOfFirstSBFx  - (nStart+nFxInDuration);
		else if ( (nFxInDuration+nNewFxSize) >= (nLength - nFxOutDuration) - SLICEBODYMINWIDTH )
			nNewFxSize =  (nLength - nFxOutDuration) - nFxInDuration - SLICEBODYMINWIDTH;
		else if (m_bSnap) //do the snapping when no out of borders occures
		{
				
				newx = nStart + nNewFxSize + nFxInDuration;
				
				Snap(newx);

				nNewFxSize = newx - nStart;

				if ( (nFxInDuration+nNewFxSize) <= 0 )
					nNewFxSize = nFxInDuration;
				
				nFxInDuration = 0;

		}


		m_TimeLineSpan.Edit	   (m_nActiveSpan,m_nActiveSlice,nStart,nLength,nFxInDuration+nNewFxSize,nFxOutDuration,dActionID);
		Refresh();
		UpdateWindow();

	}

	if(m_eventState==OVERSLICERIGHTHANDLE)
	{
		
		m_TimeLineSpan.GetSlice(m_nActiveSpan,m_nActiveSlice,nStart,nLength,nFxInDuration,nFxOutDuration,dActionID);
		

		nNewFxSize =(nStart + ( nLength-nFxOutDuration) - (int) (M.x * m_fDownScaleFactor));

		nNewLength = nLength - nFxInDuration - (nFxOutDuration + nNewFxSize );

	
		//check to see if we hit a SBFx from the right edge
		if ( (LengthOfLastSBFx=HitRightFxSplice(nStart + ( nLength-nFxOutDuration) - nNewFxSize)) != -1)	
			nNewFxSize = (nStart + ( nLength-nFxOutDuration)) - LengthOfLastSBFx;
		else if ( nNewLength <  SLICEBODYMINWIDTH)
			nNewFxSize = nLength - nFxInDuration - nFxOutDuration;		
		else if ( (nFxOutDuration+nNewFxSize) < 0 )
			nNewFxSize =  -nFxOutDuration;
		else if (m_bSnap) //do the snapping when no out of borders occures
		{
				

				newx = nLength - (nFxOutDuration+nNewFxSize) + nStart;
				
				Snap(newx);

				nNewFxSize = (nStart+nLength) - newx;
				

				nNewLength = nLength - nFxInDuration - nNewFxSize;
				if ( nNewLength <  SLICEBODYMINWIDTH)
						nNewFxSize = nFxOutDuration;
				
				
				nFxOutDuration = 0;

		}
		
			
		
		m_TimeLineSpan.Edit	   (m_nActiveSpan,m_nActiveSlice,nStart,nLength,nFxInDuration,nFxOutDuration+nNewFxSize,dActionID);
		Refresh();
		UpdateWindow();


	}

	if(m_eventState==OVERSLICEINFXLEFTHANDLE)
	{
		
		m_TimeLineSpan.GetSlice(m_nActiveSpan,m_nActiveSlice,nStart,nLength,nFxInDuration,nFxOutDuration,dActionID);
		

		nNewFxSize = (int) (M.x * m_fDownScaleFactor) - nStart;


		if( (LengthOfLastSBFx=HitRightFxSplice(nStart + ( nLength-nFxOutDuration) - nNewFxSize)) != -1)	
			nNewFxSize = (nStart + ( nLength-nFxOutDuration)) - LengthOfLastSBFx;
		else if ( (nStart+nNewFxSize) <= 0 )
			nNewFxSize = - nStart;
		else if ( (nFxInDuration+nNewFxSize) >= (nLength - nFxOutDuration) - SLICEBODYMINWIDTH )
			nNewFxSize =  (nLength - nFxOutDuration) - nFxInDuration - SLICEBODYMINWIDTH;
		else if (m_bSnap) //do the snapping when no out of borders occures
		{
	

				newx = nStart + nNewFxSize;
				
				Snap(newx);
				
				nLength = nLength+(nStart-newx);
				nStart = newx;
				nNewFxSize = 0;


		}

		

		m_TimeLineSpan.Edit	   (m_nActiveSpan,m_nActiveSlice,nStart+nNewFxSize,nLength-nNewFxSize,nFxInDuration,nFxOutDuration,dActionID);
		Refresh();
		UpdateWindow();

	}

	if(m_eventState==OVERSLICEOUTFXRIGHTHANDLE)
	{
		
		m_TimeLineSpan.GetSlice(m_nActiveSpan,m_nActiveSlice,nStart,nLength,nFxInDuration,nFxOutDuration,dActionID);
		

		nNewFxSize =  (int) (M.x * m_fDownScaleFactor) - (nStart + nLength);	
		
		nNewLength = nLength - nFxInDuration - (nFxOutDuration - nNewFxSize );

		//check if we hit a SBFx from the Left edge
		if( (LengthOfLastSBFx=HitRightFxSplice(nStart + ( nLength-nFxOutDuration) + nNewFxSize)) != -1)			
			nNewFxSize = LengthOfLastSBFx - (nStart + ( nLength-nFxOutDuration));		
		else if ( nNewLength <  SLICEBODYMINWIDTH)
			nNewFxSize = -(nLength - nFxInDuration - nFxOutDuration);		
		else if ( nStart + nLength + nNewFxSize >  m_nTimeMax)
			nNewFxSize = 0;	
		else if (m_bSnap) //do the snapping when no out of borders occures
		{
	
			
				newx = nStart+nLength+nNewFxSize;
				
				Snap(newx);

				nNewFxSize = newx - nStart;

				nNewLength = nNewFxSize - nFxInDuration - nFxOutDuration;

				if ( nNewLength <  SLICEBODYMINWIDTH)
					nNewFxSize = nLength;

				nLength = 0;

		}		
		

		m_TimeLineSpan.Edit	   (m_nActiveSpan,m_nActiveSlice,nStart,nLength+nNewFxSize,nFxInDuration,nFxOutDuration,dActionID);
		Refresh();
		UpdateWindow();

	}


	
	if (m_eventState == TIMELINEHANDLER) 
	{


		m_GlobalCurrentTime = GetTimeHandlerX(M.x);
					
		if (m_bSnap)
		{
				
				Snap(m_GlobalCurrentTime);


		}

		Refresh();
		UpdateWindow();

	

		//Scroll window in any case except when scale = SCALETOWINDOW
		if (m_sScaleTimeLine != SCALETOWINDOW) 
		{
			
			GetClientRect(&rect);

			//automatically scroll when the handler reach the edges;
			if ( ( point.x >= rect.Width() ) && (M.x < m_nScaledTimeMax) )
			{
				

				m_bScrollRight = TRUE;
				m_bScrollLeft = FALSE;
				if (m_nTimer == 0)
					m_nTimer = SetTimer (IDT_TIMER1, 10, NULL);
				


			}
			else if ( (point.x < 0) && (M.x > 0) )
			{
				

				m_bScrollLeft = TRUE;
				m_bScrollRight = FALSE;
				if (m_nTimer == 0)
					m_nTimer = SetTimer (IDT_TIMER1, 10, NULL);


			}
			else
			{
				m_bScrollRight = FALSE;
				m_bScrollLeft = FALSE;
			}
		}		


	}



	//if no Event during the xButtonDown then there is no event and the mouse buttons are free.
	//So check if there is a "hot" object bellow mouse cursor and change the cursor accordingly
	//if so change the m_MouseState state accordingly so the OnSetCursor will update the cursor
	DetectCursorFromObjBellow(M);





	if ( (m_MouseButtonState == LBUTTONUP) || (m_MouseButtonState == RBUTTONUP) || (m_MouseButtonState == MBUTTONUP))
		  m_MouseButtonState = NOBUTTONEVENT;


	CScrollView::OnMouseMove(nFlags, point);
}

/*****************************************************************************************************************
DetectCursorFromObjBellow

Detect if there is a hot object below the current x,y position of the mouse cursor
and if its true change the shape accordingly
******************************************************************************************************************/
void CTimeLineEditor::DetectCursorFromObjBellow(CPoint M) 
{
	m_bShowSBXInfoBallon = FALSE;


	if(m_eventState==NOEVENT)
	{
		// since TestPos is a m_eventState Driven function, it must be in the if statement
		EVENTS res = TestPos(M, m_nActiveSpan, m_nActiveSlice); //Did I click on something important		
		
		
		
		if(res != NOEVENT)
		{
			
			switch(res)
			{
			case OVERTIMELINEHANDLER:
				 m_MouseState = OVERTIMELINEHANDLER;
				 PostMessage(WM_SETCURSOR);
				break;
			case OVERSLICE:
				 m_MouseState = OVERSLICE;
				 PostMessage(WM_SETCURSOR);
				 break;
			case OVERSLICELEFTHANDLE:
				 m_MouseState = OVERSLICELEFTHANDLE;
				 PostMessage(WM_SETCURSOR);
				 break;
			case OVERSLICERIGHTHANDLE:
				 m_MouseState = OVERSLICERIGHTHANDLE;
				 PostMessage(WM_SETCURSOR);
				 break;
			case OVERSLICEINFXLEFTHANDLE:
				 m_MouseState = OVERSLICEINFXLEFTHANDLE;
				 PostMessage(WM_SETCURSOR);
				 break;
			case OVERSLICEOUTFXRIGHTHANDLE:
				 m_MouseState = OVERSLICEOUTFXRIGHTHANDLE;
				 PostMessage(WM_SETCURSOR);
				 break;
			case BODYSLICEFX:
				 m_MouseState = BODYSLICEFX;
				 if (m_nTimer == 0)
					m_nBallonTimer = SetTimer (IDT_TIMER1, 1000, NULL);
				 PostMessage(WM_SETCURSOR);				 
				 break;
			case OVERSLICEFXINBODY:
				 m_MouseState = OVERSLICEFXINBODY;
				 if (m_nTimer == 0)
					m_nBallonTimer = SetTimer (IDT_TIMER1, 1000, NULL);
				 PostMessage(WM_SETCURSOR);				 
				 break;
			case OVERSLICEFXOUTBODY:
				 m_MouseState = OVERSLICEFXOUTBODY;
				 if (m_nTimer == 0)
					m_nBallonTimer = SetTimer (IDT_TIMER1, 1000, NULL);
				 PostMessage(WM_SETCURSOR);				 
				 break;
			case BODYSLICEFXLEFTHANDLE:
				 m_MouseState = BODYSLICEFXLEFTHANDLE;
				 PostMessage(WM_SETCURSOR);
				 break;
			case BODYSLICEFXRIGHTHANDLE:
				 m_MouseState = BODYSLICEFXRIGHTHANDLE;
				 PostMessage(WM_SETCURSOR);
				 break;


			default:


				break;	
			}


	
		}
		else
			m_MouseState = NOEVENT;
	}


	Refresh();
	UpdateWindow();
}

/************************************************************************************************************
OnRButtonUp

Used to open popmenus analogus to the hit position

***************************************************************************************************************/
void CTimeLineEditor::OnRButtonUp(UINT nFlags, CPoint point) 
{
	m_MouseButtonState = RBUTTONUP;


	CScrollView::OnRButtonUp(nFlags, point);
}

/********************************************************************************************************************
OnRButtonDown

Used to open popmenus analogus to the hit position


**********************************************************************************************************************/
void CTimeLineEditor::OnRButtonDown(UINT nFlags, CPoint point) 
{
	m_MouseButtonState = RBUTTONDOWN;


	//PostMessage(WM_SETCURSOR);
	CScrollView::OnRButtonDown(nFlags, point);
}

/*******************************************************************************************************
OnDraw

This is the main draw center. It called automatically from the framework.
Anyway all drawing functions starts from there.


********************************************************************************************************/

void CTimeLineEditor::OnDraw(CDC* pDCZ)
{
	// Variables
	CRect		BckGr;
	CRect		dT;
	CFont		tLabelO;
	CFont		tLabelA;
	CFont		tWeight;
	CString		MyOwner;
	CString		MyAction;
	CString		TempOwner = "";
	CString		WeightVal;
	int			nStart, nLength,nTimeFxInDuration,nTimeFxOutDuration;
	double		dActionID;
	



	// enable CMemoryDC to avoid flickering
#ifdef USE_MEMDC
	CMemoryDC memDC(pDCZ);
	CMemoryDC * pDC = & memDC;
#else	
	CDC * pDC = pDCZ;
#endif


	// Render the Top Part of the BackGround
	BckGr = GetBckTopRect();
	drawBackground(pDC,BckGr);


	drawTop(pDC);

	if ( (m_nActiveSpan != -1) && (m_nActiveSlice != -1) )
	{

		m_TimeLineSpan.GetSlice(m_nActiveSpan, m_nActiveSlice, nStart, nLength, nTimeFxInDuration, nTimeFxOutDuration, dActionID);
		drawTimeSplice(pDC, m_nActiveSpan, nStart, nLength, nTimeFxInDuration, nTimeFxOutDuration);	
		drawFxSplice(pDC,m_nActiveSpan, m_nActiveSlice);
		drawShowSBFxInfoBallon(pDC,nStart,nLength, "Left sided pazzle roller","3600 sec");

	}
	

	drawGlobalTimeLineHandler(pDC,m_GlobalCurrentTime);
	




}

/*//////////////////////////////////////////////////////////////*\
** Debug
**\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#ifdef _DEBUG
void CTimeLineEditor::AssertValid() const
{
	CScrollView::AssertValid();
}

void CTimeLineEditor::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}
#endif //_DEBUG


/*************************************************************************************************************
SetTimeLineScale

Call this from your own function to set the TimeLineScale mode.Ie Zoom In and Zoom Out functions.
You can choose from the FULLTIMELINE (this is the default mode), HALFTIMELINE, SCALETOWINDOW, CUSTOMSCALE 
In the CUSTOMSCALE mode you have to define also the scalefactor. If the scalefactor is to big (ie 5 times down
from the actual full timeline) and the width is smaller than the actual window then the function 
transform it automatically to SCALETOWINDOW.



***************************************************************************************************************/
void CTimeLineEditor::SetTimeLineScale(SCALETIMELINE typeofscale, int scalefactor)
{

	CRect		winRect;

	GetClientRect(&winRect);	

	switch (typeofscale)
	{
		case HALFTIMELINE :

			//we can not downscale, so call the fulltime option that will make the appropriate corrections
			if ( m_nTimeMax <= ( winRect.Width () - ENDPOINT) )
			{
				SetTimeLineScale();
				break;
			}

			m_sScaleTimeLine = HALFTIMELINE;
			m_fDownScaleFactor = (float) 2; //shorten the  timelinehandler by the half.
			m_nScaledTimeMax = (int) (m_nTimeMax / m_fDownScaleFactor); 
			break;

		case SCALETOWINDOW:
			
			//we can not downscale, so call the fulltime option that will make the appropriate corrections
			if ( m_nTimeMax <= ( winRect.Width () - ENDPOINT) )
			{
				SetTimeLineScale();
				break;
			}


			m_sScaleTimeLine = SCALETOWINDOW;
			m_fDownScaleFactor = (float) m_nTimeMax / ( winRect.Width () - ENDPOINT); //make timelinehandler equal to the client window area.
			m_nScaledTimeMax = winRect.Width ();
			break;
		case CUSTOMSCALE  :
			//we can not downscale, so call the fulltime option that will make the appropriate corrections
			if ( m_nTimeMax <= ( winRect.Width () - ENDPOINT) )
			{
				SetTimeLineScale();
				break;
			}

			m_sScaleTimeLine = CUSTOMSCALE;
			m_fDownScaleFactor = (float) scalefactor; //how many seconds will add in every pixel-move of the timelinehandler.
			m_nScaledTimeMax = (int) ( m_nTimeMax / m_fDownScaleFactor ); 
			
			//if custom scale is smaller than the client window then transform it to SCALETOWINDOW
			if (m_nScaledTimeMax < winRect.Width ())
				SetTimeLineScale (SCALETOWINDOW);
			break;
		default:
			m_sScaleTimeLine = FULLTIMELINE;
			m_fDownScaleFactor = 1; //this 1 by 1 second / pixel-move of the timelinehandler.
			m_nScaledTimeMax = m_nTimeMax;
			
			if ( m_nTimeMax < ( winRect.Width () - ENDPOINT) )
			{
				m_nTimeMax = winRect.Width () - ENDPOINT;
				m_nScaledTimeMax = m_nTimeMax;

			}

			break;
	}

	UpdateScrollSizes();
}

/***********************************************************************************************************
GetTickerFreqMinor

returns the space between 2 major ticks. The MajorTicks has already
insured that the space here is even, so the minor ticks will spread equally.
**************************************************************************************************************/
int CTimeLineEditor::GetTickerFreqMinor()
{

	
	int		nFindSpace;
	int		nMajorTickSpace;



	if(m_sScaleTimeLine != FULLTIMELINE)
	{
		
		nMajorTickSpace	 = GetTickerFreqMajor();

				
		for (nFindSpace=4;nFindSpace>=0;nFindSpace-=1)
		{
			
			
			//if the nMajorTickSpace can hold more than nFindSpace minor tickers then its our value
			//just break the loop and return the current nFindSpace value
			if ((nMajorTickSpace % nFindSpace) == 0)
				break;
		
		}

		//nFindSpace never rich this value but do it anyway.
		if (nFindSpace <= 1)
			nFindSpace = -1;
		


	}
	else
		return TICKERFREQMINOR;


	//if space its too small to draw minor ticks then draw below the major ticks
	if (nFindSpace == -1)
		nFindSpace = nMajorTickSpace;


	return nMajorTickSpace / nFindSpace;

}

/********************************************************************************************************
GetTickerFreqMajor

returns the space between the major ticks transcated to the upper integer
just to make the spaces equal.
***********************************************************************************************************/
int CTimeLineEditor::GetTickerFreqMajor()
{
	
	int	  nMinutes;


	
	if(m_sScaleTimeLine != FULLTIMELINE)
	{

		nMinutes = m_nTimeMax / 60;		
		nMinutes = m_nScaledTimeMax / nMinutes ;


		
		//if nMinutes space > 0 then go normal
		if (nMinutes > 0)
		{
			//Now make nMinutes even (divisible by 2), so the minor ticks are spaced equally.
			if ( (nMinutes % 2) != 0 )
				nMinutes++;

			if (nMinutes == 0)
				nMinutes = 2;
		}
		//else if nMinutes <=0 (ie 0.8888) means that the client Width its so small that the space 
		//among major ticks are smaller than 1 pixel width. In this case just make it constant 2 pixels width
		//and change the scaleFactor
		else
		{
			nMinutes = 2;
		}
		 
	}
	else
		nMinutes = TICKERFREQMAJOR;



	return nMinutes;

}
/*****************************************************************************************************
GetTimeLineWidth

It returns the TimeLineWidth (from the drawing perspective of view since the actual time width is reported from the 
m_nTimeMax variable) accordingly SCALETIMELINE selected. In case of the FULLTIMELINE, the drawing
perspective of view is equal with the actual timeline indicated from the m_nTimeMax

******************************************************************************************************/

int CTimeLineEditor::GetTimeLineWidth()
{
	CRect myR;
	GetClientRect(myR);
	if(m_sScaleTimeLine == SCALETOWINDOW)
	{
		return myR.Width()-ENDPOINT;
	}
	else if(m_sScaleTimeLine == HALFTIMELINE)
	{
		return (int) ( m_nTimeMax / m_fDownScaleFactor );
	}
	else if(m_sScaleTimeLine == CUSTOMSCALE)
	{
		//currently not supported is here for future development
		return (int) ( m_nTimeMax / m_fDownScaleFactor );
	}

	//here we are in FULLTIMELINE
	return m_nTimeMax;
}




int CTimeLineEditor::GetTimeToX(int Time)
{
	return (int) (Time / m_fDownScaleFactor);
}

int CTimeLineEditor::GetSliceLeft(int Time)
{
	return GetTimeToX(Time);
}

int CTimeLineEditor::GetSliceRight(int Time, int Length)
{
	return GetTimeToX(Time+Length);
}

void CTimeLineEditor::Snap(int &Val)
{
	// if we are snaping, snap it to the minor ticks
	// basically, a integer-base<TICKERFREQMINOR> truncate 
	
	Val /= (int) ( GetTickerFreqMinor() * m_fDownScaleFactor);
	Val *= (int) ( GetTickerFreqMinor() * m_fDownScaleFactor);

}



/***************************************************************************************************************
DetermineSpan

Returns the number of the span in a specific mouse Y position.
Its not used in this lib since we use only 1 span.


****************************************************************************************************************/
int CTimeLineEditor::DetermineSpan(int y)
{
	
	return 0;
}

int CTimeLineEditor::GetTimeHandlerX(int x)
{
	if( x < 0) 
		return 0;

	if( x > GetTimeLineWidth()) 
		return m_nTimeMax;

	return x;
}


/*****************************************************************************************************
GetTimeFromX

Returns the time at the input parameter X
******************************************************************************************************/

int CTimeLineEditor::GetTimeFromX(int x)
{
	if(x < 0) 
		return 0;

	if(x > GetTimeLineWidth()) 
		return m_nTimeMax;
	
	return (x * m_nTimeMax) / GetTimeLineWidth();
}

/*****************************************************************************************************
 TestPos
 
 This function do the actual mouse hit test. If its finds something interesting
 then it returns the corresponding event. The events are enum items at the enum EVENTS structure in
 the header.
 
 ******************************************************************************************************/

EVENTS CTimeLineEditor::TestPos(CPoint M, int &Span, int &Slice)
{
	return TestPos(M.x, M.y, Span, Slice);
}

EVENTS CTimeLineEditor::TestPos(int x, int y, int &Span, int &Slice)
{
	
	int		nTime;
	int		nLength;
	int		nFxInDuration,nFxOutDuration;
	double  dAction;

	int		start,length;
	double	actionID;
	CRect	FxSliceRect;

	CPoint P(x,y);
	


	//check for the timeline handler
	if ( GetTimeLineHandlerRect(m_GlobalCurrentTime).PtInRect(P) )
	{
		
		if (m_MouseButtonState == LBUTTONDOWN)				
			return TIMELINEHANDLER;		
		else if ( (m_MouseButtonState == NOBUTTONEVENT) || (m_MouseButtonState == LBUTTONUP) )			
			return OVERTIMELINEHANDLER;

		
	}
	
	//check if clicked in the timeline header 
	if ( (GetTimeLineHeaderRect().PtInRect(P) == TRUE) && (m_MouseButtonState == LBUTTONDOWN) )	
		return TIMELINEHEADER;

	
	//check for the slice inside the span
	if (m_TimeLineSpan.GetSpanCount() > 0)
	{
		
		if(GetSpanEdge(Span).PtInRect(P)==TRUE)
		{



				m_TimeLineSpan.GetSlice(Span,Slice,nTime,nLength,nFxInDuration,nFxOutDuration,dAction);
				
				
			

				//the series we make theses checks are important since the OVERSLICELEFTHANDLE & OVERSLICERIGHTHANDLE
				//occupy space from the Slice body - because normally handlers have 1 pixel width, which means that you have to
				//be a magician to accomplish to point it with the mouse - which is SLICEEDGELEFTWIDTH & SLICEEDGERIGHTWIDTH 
				//respectively.

				if(GetSliceFxInBody (Span, nTime, nLength, nFxInDuration).PtInRect(P)==TRUE)
				{ 
					
					return OVERSLICEFXINBODY;
				}

				if(GetSliceFxOutBody (Span, nTime, nLength, nFxOutDuration).PtInRect(P)==TRUE)
				{ 
					
					return OVERSLICEFXOUTBODY;
				}


				if(GetSliceHandleLeft (Span, nTime, nLength, nFxInDuration).PtInRect(P)==TRUE)
				{ 
					
					return OVERSLICELEFTHANDLE;
				}

				if(GetSliceHandleRight(Span, nTime, nLength, nFxOutDuration).PtInRect(P)==TRUE)
				{ 
					
					return OVERSLICERIGHTHANDLE;
				}

				if(GetSliceHandleInFxLeft(Span, nTime, nLength, nFxOutDuration).PtInRect(P)==TRUE)
				{ 
					
					return OVERSLICEINFXLEFTHANDLE;
				}

				if(GetSliceHandleOutFxRight(Span, nTime, nLength, nFxOutDuration).PtInRect(P)==TRUE)
				{ 
					
					return OVERSLICEOUTFXRIGHTHANDLE;
				}

				
				for (int i=0; i<m_TimeLineSpan.GetSliceFxCount(Span,Slice);i++)
				{
					m_TimeLineSpan.GetFxToSlice(Span,Slice,i,start,length,actionID);
					FxSliceRect = GetSBFxBodyRect(Span, start+nTime, length); //GetSliceRect takes in mind the ScaleFactor.
				
					//CString str;
					//str.Format("x=%d   y=%d   start=%d  nTime=%d   length=%d  FxSliceRect.left=%d  FxSliceRect.right=%d \n",x,y,start,nTime,length,FxSliceRect.left,FxSliceRect.right  );
					//TRACE(str);

					if(FxSliceRect.PtInRect(P)==TRUE)
					{ 					
						m_nSBFxActive = i;
						//now check if the mouse is over
						//the left or right handle

						if (GetSBFxHandleLeft(Span,start+nTime,length).PtInRect(P) == TRUE)
						{

							return BODYSLICEFXLEFTHANDLE;

						}

						if (GetSBFxHandleRight(Span,start+nTime,length).PtInRect(P) == TRUE)
						{
							return BODYSLICEFXRIGHTHANDLE;

						}



						//if not return on over body
						return BODYSLICEFX;
					}					

				}
				


				if ( (GetSliceBodyRect(Span, nTime, nLength, nFxInDuration, nFxOutDuration).PtInRect(P)==TRUE) )
				{ 
					
					return OVERSLICE;
				}
			
			
		}

	}
	
	

	return NOEVENT;
}


/*************************************************************************************************
SetTimeMax

Sets the timeline length in seconds

**************************************************************************************************/
void CTimeLineEditor::SetTimeMax(int Max)
{
	CRect	ClientWidth;

	GetClientRect(ClientWidth);

	//Make Time at least double of the current width
	if (Max < (ClientWidth.Width () * 2 ) )
		Max = ClientWidth.Width () * 2;

	//Now make it even

	if ( (Max % 2) != 0)
		Max++;
	
	
	m_nTimeMax = Max;

	//compute the total Hour:Min:Sec and save it.

	int nTime = m_nTimeMax;
	m_nTotalHour = nTime / 3600; //how many hours;
	nTime = m_nTimeMax - (m_nTotalHour*3600);
	m_nTotalMin  = nTime / 60;
	nTime = nTime - (m_nTotalMin * 60) ;
	m_nTotalSec = nTime;
	m_TotalTime = CTimeSpan(0,m_nTotalHour, m_nTotalMin, m_nTotalSec); 
	
	// update the scroll bars
	UpdateScrollSizes();

}

/*************************************************************************************************
Refresh

Just force invalidate to repaint the view

***************************************************************************************************/
void CTimeLineEditor::Refresh()
{
	InvalidateRect(NULL);
}

/*//////////////////////////////////////////////////////////////*\
** Rendering Helper Functions
**\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/


/************************************************************************************************************************
drawInFxLeftHandler

Draws the FX-In slice above the timeline


**************************************************************************************************************************/
void CTimeLineEditor::drawInFxLeftHandler(CDC *pDC,int YDist, int Time, int Duration)
{
   CBitmap		BmpSliceEdgeLeft;	
   CBitmap		BmpSliceInOutFxTile;
   CBitmap		BmpFXTitle;
   CRect		FxTitleRect;
   

   //draw the Edge in bitmap 
   BmpSliceEdgeLeft.LoadBitmap (IDB_SLICEEDGELEFT);	
   DrawTransparent(&BmpSliceEdgeLeft,pDC,Time-SLICEEDGELEFTWIDTH,YDist,SLICEEDGELEFTWIDTH,SLICEHEIGHT,RGB(255,0,255));

   if (Duration > 0)
   {
	   //draw the tile of the FX out
	   BmpSliceInOutFxTile.LoadBitmap (IDB_SLICEINOUTFXTILE);	
	   DrawBitmap(&BmpSliceInOutFxTile,pDC,Time,YDist,SLICEINOUTFXWIDTH,SLICEHEIGHT,Time+Duration);


	   //draw the Fx label only if the duration is bigger than X cos else there is no space to write
	   if (Duration >= 12) 
	   {
		   FxTitleRect.left =  Time + ( Duration / 2 ) - 6;
		   FxTitleRect.right = Time + Duration;
		   FxTitleRect.top = YDist + (HEADERTOP / 2) - 6;
		   FxTitleRect.bottom = YDist + HEADERTOP;


		   BmpFXTitle.LoadBitmap (IDB_FX);
		   DrawTransparent(&BmpFXTitle,pDC,FxTitleRect.left,FxTitleRect.top,FXTITLEWIDTH,FXTITLEHEIGHT,RGB(255,0,255));

	   }


   }


}


/************************************************************************************************************************
drawOutFxRightHandler

Draws the FX-Out slice above the timeline


**************************************************************************************************************************/
void CTimeLineEditor::drawOutFxRightHandler(CDC *pDC,int YDist, int Duration, int EndPoint)
{
   CBitmap		BmpSliceEdgeRight;
   CBitmap		BmpSliceInOutFxTile;
   CBitmap		BmpFXTitle;
   CRect		FxTitleRect;
   int			Time;
 
   Time = EndPoint - Duration;

   if (Duration >  0)
   {
	   BmpSliceInOutFxTile.LoadBitmap (IDB_SLICEINOUTFXTILE);	
	   DrawBitmap(&BmpSliceInOutFxTile,pDC,Time,YDist,SLICEINOUTFXWIDTH,SLICEHEIGHT,EndPoint);

	   //draw the Fx label only if the duration is bigger than X cos else there is no space to write
	   if (Duration >= 12)
	   {
		   
		   FxTitleRect.left =  Time + ( Duration / 2 ) - 6;
		   FxTitleRect.right = Time + Duration;
		   FxTitleRect.top = YDist + (HEADERTOP / 2) - 6;
		   FxTitleRect.bottom = YDist + HEADERTOP;

		   BmpFXTitle.LoadBitmap (IDB_FX);
		   DrawTransparent(&BmpFXTitle,pDC,FxTitleRect.left,FxTitleRect.top,FXTITLEWIDTH,FXTITLEHEIGHT,RGB(255,0,255));

	   }
   }

   //draw the Edge out bitmap
   BmpSliceEdgeRight.LoadBitmap (IDB_SLICEEDGERIGHT);	
   DrawTransparent(&BmpSliceEdgeRight,pDC,Time + Duration,YDist,SLICEEDGERIGHTWIDTH,SLICEHEIGHT,RGB(255,0,255));


}

/**************************************************************************************************************************
drawTimeSliceMoveEventLabel

Shows a label that displays the current start time of the slice as well as its duration


***************************************************************************************************************************/
void CTimeLineEditor::drawTimeSliceMoveEventLabel(CDC *pDC, int Time, int Length, CString str)
{

	CRect		ClientArea,TimeTipLabel;
	int			nXDistance;
	CBrush		*pOldBrush;
	CBrush		FillColor(RGB(185,246,185));
	CPen		*pOldPen,penOutline;
	CFont		*pOldFont,tFont;	
	CSize		csTextSize;
	

	GetClientRect (ClientArea);
	nXDistance = GetScaledTime(Time);

	penOutline.CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
	pOldPen = pDC->SelectObject(&penOutline);
	pDC->SetBkMode(TRANSPARENT);

	tFont.CreateFont(12, 0, 0, 0,0, 0,0,0,0,0,0,0,0,"Arial");
	pOldFont = pDC->SelectObject(&tFont);
	pDC->SetBkColor(GetSysColor(COLOR_WINDOW));
	pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextColor(RGB(0,0,0));

	
	csTextSize=pDC->GetTextExtent(str);
	
	 //If handler is approaching the end of the client area (left side) then exchange the time label to the left side.
	if  ( (GetScaledTime(Time+Length) > ClientArea.Width()+GetScrollPosition().x) || (m_bScrollRight) || (GetScaledTime(Time)+csTextSize.cx >= m_nScaledTimeMax ) )
	{

		 TimeTipLabel.left	 = (GetScrollPosition().x + ClientArea.Width()) - csTextSize.cx - 15;
		 TimeTipLabel.top	 = GetScrollPosition().y+TIMELINEHANDLERTOPMARGIN;
		 TimeTipLabel.right  = TimeTipLabel.left + csTextSize.cx + 10;	 	 
		 TimeTipLabel.bottom = TimeTipLabel.top + TIMELINELABELHEIGHT;

	}
	else if   ( (m_bScrollLeft) || (GetScaledTime(Time) < GetScrollPosition().x) )
	{
		 TimeTipLabel.left	 = GetScrollPosition().x + 5 ;
		 TimeTipLabel.top	 = GetScrollPosition().y+TIMELINEHANDLERTOPMARGIN;
		 TimeTipLabel.right  = TimeTipLabel.left + csTextSize.cx + 10;	 	 
		 TimeTipLabel.bottom = TimeTipLabel.top + TIMELINELABELHEIGHT;


	}
	else
	{
		 //in normal conditions show label at the time point
		 TimeTipLabel.left	 = GetScaledTime(Time);
		 TimeTipLabel.top	 = GetScrollPosition().y+TIMELINEHANDLERTOPMARGIN;
		 TimeTipLabel.right  = TimeTipLabel.left + csTextSize.cx + 10;	 	 
		 TimeTipLabel.bottom = TimeTipLabel.top + TIMELINELABELHEIGHT;


	}
	 
	 pOldBrush = pDC->SelectObject(&FillColor);
	 pDC->Rectangle(TimeTipLabel.left,TimeTipLabel.top,TimeTipLabel.right+1,TimeTipLabel.bottom);

	 
	 TimeTipLabel.left	 += 5; //spaces from the left of the label rec
	 TimeTipLabel.top	 += 2; //spaces from the top of the label rec
	 

	 pDC->DrawText(str, TimeTipLabel, DT_BOTTOM);

	 
	 //restore old objects
	 pDC->SelectObject(pOldBrush);
	 pDC->SelectObject(pOldFont);
	 pDC->SelectObject(pOldPen);

}

/************************************************************************************************************************
drawFxSplice

Draws the Fx slices inside the slice body (the place between InFxDuration and OutFxDuration);


**************************************************************************************************************************/
void CTimeLineEditor::drawFxSplice(CDC *pDC, int Span, int Slice)
{
	int			start, length;
	double		actionID;

	int			nTime,nLength,nFxInDuration,nFxOutDuration;
	double		dAction;

	CBrush		*pOldBrush;
	CBrush		FillColor(RGB(46,102,179));
	CRect		FxSliceRect;
    CPen		*pOldPen, penColor;
   
	
	penColor.CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
	pOldPen = pDC->SelectObject(&penColor);	
	pOldBrush = pDC->SelectObject(&FillColor);
    
	m_TimeLineSpan.GetSlice(Span,Slice,nTime,nLength,nFxInDuration,nFxOutDuration,dAction);

	for (int i=0; i<m_TimeLineSpan.GetSliceFxCount(Span,Slice);i++)
	{
		m_TimeLineSpan.GetFxToSlice(Span,Slice,i,start,length,actionID);
		FxSliceRect = GetSliceRect(Span, start+nTime, length); //GetSliceRect takes in mind the ScaleFactor.
		pDC->Rectangle(FxSliceRect.left ,FxSliceRect.top ,FxSliceRect.right ,FxSliceRect.bottom);
		

	}

	pDC->SelectObject (pOldBrush);
	pDC->SelectObject (pOldPen);

}

/************************************************************************************************************************
drawTimeSplice

Draws the slice above the timeline


**************************************************************************************************************************/
void CTimeLineEditor::drawTimeSplice(CDC *pDC, int Span, int Time, int Length, int TimeFxInDuration, int TimeFxOutDuration)
{

	CBitmap		BmpSliceBodyHandler;
	CPen		PenHandler,*pOldPen; 
	CRect		Slice;
	CString		sTime;
	int			nTime,nHour,nMin,nSec;



	
   Slice = GetSliceRect(Span, Time, Length); //GetSliceRect takes in mind the ScaleFactor.


 
   //Draw Slice body only if there is space for this (when in out fxs are smaller than the whole body)


	BmpSliceBodyHandler.LoadBitmap (IDB_SLICEBODYTILE);	
	DrawBitmap(&BmpSliceBodyHandler,pDC,Slice.left ,Slice.top,SLICEBODYTILEWIDTH,SLICEHEIGHT,GetScaledTime(Time+Length));

	//Draw FX Int
	drawInFxLeftHandler(pDC,Slice.top,Slice.left,GetScaledTime(TimeFxInDuration));
 
	//draw the tile of the FX out	   
	drawOutFxRightHandler(pDC,Slice.top,GetScaledTime(TimeFxOutDuration),GetScaledTime(Time+Length));


   


   /*********************************************
	Draw the verical black lines at the edges of
	FxInDuration and FxOutDuration. This is optional
   **********************************************/
   //select pen for the handler vert line pointer

   PenHandler.CreatePen(PS_SOLID  ,1 , RGB(0,0,0) );
   pOldPen = pDC->SelectObject(&PenHandler);

   //draw vert line pointer
   pDC->MoveTo(Slice.left+GetScaledTime(TimeFxInDuration)-1,Slice.top);
   pDC->LineTo(Slice.left+GetScaledTime(TimeFxInDuration)-1,Slice.top + HEADERTOP);
    

 
   //draw vert line pointer
   pDC->MoveTo(Slice.left+GetScaledTime(Length - TimeFxOutDuration),Slice.top);
   pDC->LineTo(Slice.left+GetScaledTime(Length - TimeFxOutDuration),Slice.top + HEADERTOP);

   pDC->SelectObject(&pOldPen);

   /*************************************************/	




   //if left button is pressed --> Slice movement then show info label
   if ( (m_eventState == OVERSLICE) && ( m_MouseButtonState == LBUTTONDOWN) )
   {
	 //in case the handler is at the end of the time line
	 //force to show the total time. We use this because we have
	 //small divergence of the float decimals rejection
	 
	 if (Time == m_nScaledTimeMax) 
		 nTime = m_nScaledTimeMax;
	 else
		 nTime = Time;

	 nHour = (nTime / 3600); //how many hours;
	 nTime = nTime - (nHour*3600);
	 nMin  = (nTime / 60);//mins
	 nTime = nTime - (nMin * 60) ;
	 nSec   = nTime;//seconds	 
	 CTimeSpan CurrentTime(0, nHour, nMin, nSec); 


	 //draw the time
	 sTime.Format("%d sec",Length);
	 sTime = CurrentTime.Format(_T("%H:%M:%S")) + " - " + sTime; 	   
	 	 
	 drawTimeSliceMoveEventLabel(pDC, Time, Length, sTime);
   }

   //if left button is pressed --> FxInDuration movement then show info label
   if ( (m_eventState == OVERSLICELEFTHANDLE) && ( m_MouseButtonState == LBUTTONDOWN) )
   {
	 //in case the handler is at the end of the time line
	 //force to show the total time. We use this because we have
	 //small divergence of the float decimals rejection
	 
	 if (TimeFxInDuration == m_nScaledTimeMax) 
		 nTime = m_nScaledTimeMax;
	 else
		 nTime = Time+TimeFxInDuration;

	 nHour = (nTime / 3600); //how many hours;
	 nTime = nTime - (nHour*3600);
	 nMin  = (nTime / 60);//mins
	 nTime = nTime - (nMin * 60) ;
	 nSec   = nTime;//seconds	 
	 CTimeSpan CurrentTime(0, nHour, nMin, nSec); 

	 

	 //draw the time
	 sTime.Format("%d sec",TimeFxInDuration);
	 sTime = CurrentTime.Format(_T("%H:%M:%S")) + " - " + sTime; 	   
	 
	 drawTimeSliceMoveEventLabel(pDC, Time, Length, sTime);
   }

   //if left button is pressed --> FxInDuration movement then show info label
   if ( (m_eventState == OVERSLICERIGHTHANDLE) && ( m_MouseButtonState == LBUTTONDOWN) )
   {
	 //in case the handler is at the end of the time line
	 //force to show the total time. We use this because we have
	 //small divergence of the float decimals rejection
	 
	 if (TimeFxInDuration == m_nScaledTimeMax) 
		 nTime = m_nScaledTimeMax;
	 else
		 nTime = Time + (Length - TimeFxOutDuration);

	 nHour = (nTime / 3600); //how many hours;
	 nTime = nTime - (nHour*3600);
	 nMin  = (nTime / 60);//mins
	 nTime = nTime - (nMin * 60) ;
	 nSec   = nTime;//seconds	 
	 CTimeSpan CurrentTime(0, nHour, nMin, nSec); 

	 

	 //draw the time
	 sTime.Format("%d sec",TimeFxOutDuration);
	 sTime = CurrentTime.Format(_T("%H:%M:%S")) + " - " + sTime; 	   
	 
	 drawTimeSliceMoveEventLabel(pDC, Time, Length, sTime);
   }

   //if left button is pressed --> start of slice FxInDuration movement then show info label
   if ( (m_eventState == OVERSLICEINFXLEFTHANDLE) && ( m_MouseButtonState == LBUTTONDOWN) )
   {
	 //in case the handler is at the end of the time line
	 //force to show the total time. We use this because we have
	 //small divergence of the float decimals rejection
	 
	 if (TimeFxInDuration == m_nScaledTimeMax) 
		 nTime = m_nScaledTimeMax;
	 else
		 nTime = Time;

	 nHour = (nTime / 3600); //how many hours;
	 nTime = nTime - (nHour*3600);
	 nMin  = (nTime / 60);//mins
	 nTime = nTime - (nMin * 60) ;
	 nSec   = nTime;//seconds	 
	 CTimeSpan CurrentTime(0, nHour, nMin, nSec); 

	 

	 //draw the time
	 sTime.Format("%d sec",Length);
	 sTime = CurrentTime.Format(_T("%H:%M:%S")) + " - " + sTime; 	   
	 
	 drawTimeSliceMoveEventLabel(pDC, Time, Length, sTime);
   }

   //if left button is pressed --> end of slice FxOutDuration movement then show info label
   if ( (m_eventState == OVERSLICEOUTFXRIGHTHANDLE) && ( m_MouseButtonState == LBUTTONDOWN) )
   {
	 //in case the handler is at the end of the time line
	 //force to show the total time. We use this because we have
	 //small divergence of the float decimals rejection
	 
	 if (TimeFxInDuration == m_nScaledTimeMax) 
		 nTime = m_nScaledTimeMax;
	 else
		 nTime = Time+Length;

	 nHour = (nTime / 3600); //how many hours;
	 nTime = nTime - (nHour*3600);
	 nMin  = (nTime / 60);//mins
	 nTime = nTime - (nMin * 60) ;
	 nSec   = nTime;//seconds	 
	 CTimeSpan CurrentTime(0, nHour, nMin, nSec); 

	 

	 //draw the time
	 sTime.Format("%d sec",Length);
	 sTime = CurrentTime.Format(_T("%H:%M:%S")) + " - " + sTime; 	   
	 
	 drawTimeSliceMoveEventLabel(pDC, Time, Length, sTime);
   }


}



/**************************************************************************************************************
drawTop

1. Draw the ToolBar part (if exists)
2. Draw the Browser part (where the timeline handler resides)
3. Draw the Top part (where the tickers resides)
4. Draw the minor tickers (gray color) if exists. if the space is too small the you get a -1 value which means
   dont draw minor ticks
5. Draw the major ticks
6. Draw the ENDPOINT part. Its a small green rectange in the end of the timeline just to mark the end and
   practically just to make the things easier when you are trying to catch the handler since at the end
   is a semi-hidden and it may be hard to catch. You can disable this thing if you set ENDPOINT = 0 in the
   header.
7. Calls the drawGlobalTimeLineHandler with the current time pointer from the m_GlobalCurrentTime to draw
   the handler.



***************************************************************************************************************/
void CTimeLineEditor::drawTop(CDC *pDC)
{
	CRect		Top, Browser, ToolBar,EndPoint,ClientArea;
	int			nTimeLineWidth;
	int			nCustonMajorTickSpace,nMajorTicks;
	int			nCustonMinorTickSpace;
	int			nLoopX;
	int			nDrawStartY, nDrawEndY;
	CPen		MinorTicks;
	CPen		MajorTicks;
	CPen		BroswerTicks;
	CPen		*pOldPen;
	CFont		*pOldFont,tFont;
	CRect		crBrowserText;
	CString		strTime;
	int			nTime,nHour,nMin,nSec;
	int			nCounter;
	

	GetClientRect(&Top);
	

	ClientArea = Browser = ToolBar = Top;

	// where the toolbar resides (if exists)
	ToolBar.left += GetScrollPosition().x;
	ToolBar.right += ToolBar.left + GetTimeLineWidth();
	ToolBar.top += GetScrollPosition().y;
	ToolBar.bottom = ToolBar.top + TIMELINETOOLBAR;	
	
	// the tickers timeline
	Top.left += GetScrollPosition().x;
	Top.right += Top.left + GetTimeLineWidth();
	Top.top += GetScrollPosition().y + HEADERBROWSER + TIMELINETOOLBAR;
	Top.bottom = Top.top + HEADERTOP;

	//where the timeline handler resides	
	Browser.left += GetScrollPosition().x;
	Browser.right += Browser.left + GetTimeLineWidth();
	Browser.top += GetScrollPosition().y + TIMELINETOOLBAR;
	Browser.bottom = Browser.top + HEADERBROWSER;



	pDC->FillSolidRect(ToolBar, RGB(255,0,0));
	pDC->FillSolidRect(Browser, RGB(240,240,240));
	pDC->FillSolidRect(Top, RGB(255,255,255));
 
	nTimeLineWidth = GetTimeLineWidth();
	BroswerTicks.CreatePen(PS_SOLID  ,1 , RGB(100,100,200) );
	pOldPen = pDC->SelectObject(&BroswerTicks);
	MinorTicks.CreatePen(PS_SOLID  ,1 , RGB(200,200,200) );	
	MajorTicks.CreatePen(PS_SOLID  ,1 , RGB(255,0,0) );




	//draw browse ticks

	
	tFont.CreateFont(12, 0, 0, 0,0, 0,0,0,0,0,0,0,0,"Arial");
	pOldFont = pDC->SelectObject(&tFont);
	pDC->SetBkColor(GetSysColor(COLOR_WINDOW));
	pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextColor(RGB(100,100,100));


	
	nMajorTicks = GetTickerFreqMajor();
	nCustonMinorTickSpace = GetTickerFreqMinor(); 




	//here compute the space between time code in major ticks that is shown at the browswer
	//make it dynamically and chage the location of the time everytime the window size changes.
	//Its used only to find the space for the time label.
	if (m_sScaleTimeLine == FULLTIMELINE) 
		nCustonMajorTickSpace = TICKERFREQMAJOR;
	else
	{
		nCustonMajorTickSpace = 0;
		nCounter=1;
		while(nCustonMajorTickSpace <= 0)
		{
			
			nCustonMajorTickSpace = ( (nMajorTicks*nCounter) / (TIMELINELABELWIDTH / 2));
			nCounter++;
			
		}

		nCustonMajorTickSpace = nMajorTicks * (nCounter-1);
		

	}




	for(nLoopX = GetScrollPosition().x; nLoopX<=GetScrollPosition().x + ClientArea.right; nLoopX+=1)
	{
		
		if ( (nLoopX % (nMajorTicks)) == 0)
		{
			
			if ( (nLoopX % (nCustonMajorTickSpace)) == 0)
			{

				nDrawStartY = GetScrollPosition().y + TIMELINETOOLBAR;
				nDrawEndY = GetScrollPosition().y + TIMELINETOOLBAR + HEADERBROWSER;
				
				crBrowserText.top = nDrawStartY + (HEADERBROWSER / 2) - 5;
				crBrowserText.left = nLoopX - (TIMELINELABELWIDTH / 4);
				crBrowserText.right = nLoopX + (TIMELINELABELWIDTH / 4);
				crBrowserText.bottom = crBrowserText.top + TIMELINELABELHEIGHT;
				
				//in case the handler is at the end of the time line
				//force to show the total time. We use this because we have
				//small divergence of the float decimals rejection
				if (nLoopX == m_nScaledTimeMax) 
					 nTime = m_nTimeMax;
				else
					 nTime = (int) (nLoopX * m_fDownScaleFactor);

				nHour = (nTime / 3600); //how many hours;
				nTime = nTime - (nHour*3600);
				nMin  = (nTime / 60);//mins
				nTime = nTime - (nMin * 60) ;
				nSec   = nTime;//seconds

				 
				CTimeSpan CurrentTime(0, nHour, nMin, nSec); 

				//draw the time		
				strTime = CurrentTime.Format(_T("%H:%M:%S")); 			
				pDC->DrawText(strTime, crBrowserText, DT_VCENTER | DT_CENTER);
			}


			pDC->SelectObject(&BroswerTicks);
			nDrawStartY = GetScrollPosition().y + TIMELINETOOLBAR;
			nDrawEndY = GetScrollPosition().y + TIMELINETOOLBAR + HEADERBROWSER;
			
			pDC->MoveTo(nLoopX, nDrawStartY);
			pDC->LineTo(nLoopX, nDrawStartY + 5);

			pDC->MoveTo(nLoopX, nDrawEndY);
			pDC->LineTo(nLoopX, nDrawEndY - 5);


			pDC->SelectObject(&MajorTicks);
			nDrawStartY = GetScrollPosition().y+HEADERBROWSER+TIMELINETOOLBAR;
			nDrawEndY = GetScrollPosition().y+TICKMAJORHEIGHT+HEADERBROWSER+TIMELINETOOLBAR;
			pDC->MoveTo(nLoopX, nDrawStartY);
			pDC->LineTo(nLoopX, nDrawEndY);

		

		
		}
			
		else if (  (nLoopX % nCustonMinorTickSpace) == 0)
		{
			pDC->SelectObject(&BroswerTicks);
			nDrawStartY = GetScrollPosition().y + TIMELINETOOLBAR;
			nDrawEndY = GetScrollPosition().y + TIMELINETOOLBAR + HEADERBROWSER;
			
			pDC->MoveTo(nLoopX, nDrawStartY);
			pDC->LineTo(nLoopX, nDrawStartY + 3);

			pDC->MoveTo(nLoopX, nDrawEndY);
			pDC->LineTo(nLoopX, nDrawEndY - 3);

			nDrawStartY += HEADERBROWSER;
			nDrawEndY += TICKMINORHEIGHT;
			
			pDC->SelectObject(&MinorTicks);
			pDC->MoveTo(nLoopX, nDrawStartY);
			pDC->LineTo(nLoopX, nDrawEndY);



		}
	

	}

	
	



	if (ENDPOINT > 0)
	{
		EndPoint.left = nTimeLineWidth;
		EndPoint.top = nDrawStartY;
		EndPoint.bottom = nDrawEndY;
		EndPoint.right = EndPoint.left + ENDPOINT;
		pDC->FillSolidRect(EndPoint, RGB(0,255,0));
	}


	pDC->SelectObject(pOldPen);
	pDC->SelectObject(pOldFont);
	
}
/********************************************************************************************************************
DrawSemiTransparentBitmap

Draws a SemiTransparent bitmap.


More information
To display a bitmap (Say A) over another (B) to produce a result where A and B is visible you must divide each pixel of A 
by a constant, divide each pixel of B by another constant and add the two results. 

Result = A*a + B*(1-a)    where "a" is a weight factor

This method is much slower but produces better results than semi-transparent bitmaps. But if you are interested in speed 
you should use the semi-transparent method explained here. 
The first BitBlt remove the higher bit of each pixel in the destination context and the second add the source context
over the destination context. This will result in some saturation of bright colors but is not a problem in most applications. 

Result = (A and 0x7F) + B

If you don't worry at all about saturation you can remove all but the last BitBlt. This will result in most case to an 
image with some saturation if you use bright colors. 

Result = A + B

If A and B are big the result could be greater than 255 and result in saturation (modulo). 

********************************************************************************************************************/

void CTimeLineEditor::DrawSemiTransparentBitmap(CDC *pDstDC, int x, int y, int nWidth, int nHeight, CDC* pSrcDC, int xSrc, int ySrc)
{
    CDC dcCompatible;
    CBitmap *pBitmapOld;
    CBitmap bm;

    dcCompatible.CreateCompatibleDC(pDstDC);
    bm.CreateCompatibleBitmap(pDstDC, nWidth, nHeight);
    pBitmapOld = dcCompatible.SelectObject(&bm);
    dcCompatible.FillSolidRect(CRect(0, 0, nWidth, nHeight), RGB(0x7F, 0x7F, 0x7F));
    pDstDC->BitBlt(x, y, nWidth, nHeight, &dcCompatible, 0, 0, SRCAND);
    dcCompatible.SelectObject(pBitmapOld);

    pDstDC->BitBlt(x, y, nWidth, nHeight, pSrcDC, 0, 0, SRCPAINT);
}


/********************************************************************************************************************
DrawBitmap

Draws a bitmap. If tileWidth > 1 then it repeats the bitmap until x2 reaches


********************************************************************************************************************/

void CTimeLineEditor::DrawBitmap(CBitmap *image, CDC *pDC, int x, int y, int Width, int Height, int x2)
{


	CDC dcImage;

	// Create memory dcs for the image
	dcImage.CreateCompatibleDC(pDC);
	
	// Select the image into the appropriate dc
	CBitmap* pOldBitmapImage = dcImage.SelectObject(image);

	// Do the work
	for (int i=x;i<(x2);i += Width)
		pDC->BitBlt(i, y, Width, Height, &dcImage, 0, 0, SRCCOPY);	
	

	// Restore settings
	dcImage.SelectObject(pOldBitmapImage);
	


}


/********************************************************************************************************************
DrawTransparent

It takes a bitmap and an a chroma key (crColour), creates automatically the mask (Black & white) for this bitmap and
then to draws to the x,y position makes the part of the chroma key color transparent.


********************************************************************************************************************/

void CTimeLineEditor::DrawTransparent(CBitmap *image, CDC *pDC, int x, int y, int Width, int Height, COLORREF crColour)
{
	

	const int CAPS1 = 94;
	const int C1_TRANSPARENT = 1;
	const int NEWTRANSPARENT = 3;

	
	COLORREF crOldBack = pDC->SetBkColor(RGB(255, 255, 255));
	COLORREF crOldText = pDC->SetTextColor(RGB(0, 0, 0));
	CDC dcImage, dcTrans;

	// Create two memory dcs for the image and the mask
	dcImage.CreateCompatibleDC(pDC);
	dcTrans.CreateCompatibleDC(pDC);

	// Select the image into the appropriate dc
	CBitmap* pOldBitmapImage = dcImage.SelectObject(image);

	// Create the mask bitmap
	CBitmap bitmapTrans;
	int nWidth = Width;
	int nHeight = Height;
	bitmapTrans.CreateBitmap(nWidth, nHeight, 1, 1, NULL);

	// Select the mask bitmap into the appropriate dc
	CBitmap* pOldBitmapTrans = dcTrans.SelectObject(&bitmapTrans);

	// Build mask based on transparent colour
	dcImage.SetBkColor(crColour);
	dcTrans.BitBlt(0, 0, nWidth, nHeight, &dcImage, 0, 0, SRCCOPY);

	// Only attempt this if device supports functionality.
/*
	if(pDC->GetDeviceCaps(CAPS1) & C1_TRANSPARENT)
	{
	   AfxMessageBox("It supports it");
		// Special transparency background mode
		  //oldMode = SetBkMode(hdcDest, NEWTRANSPARENT);
		  //rgbBk = SetBkColor(hdcDest, rgbTransparent);
	   // Actual blt is a simple source copy; transparency is automatic.
		  //BitBlt(hdcDest, x, y, Width, Height hdcSrc, x, y, SRCCOPY);
		  //SetBkColor(hdcDest, rgbBk);
		  //SetBkMode(hdcDest, oldMode);
	}
*/

	// Do the work - True Mask method - cool if not actual display
	pDC->BitBlt(x, y, nWidth, nHeight, &dcImage, 0, 0, SRCINVERT);
	pDC->BitBlt(x, y, nWidth, nHeight, &dcTrans, 0, 0, SRCAND);
	pDC->BitBlt(x, y, nWidth, nHeight, &dcImage, 0, 0, SRCINVERT);

	// Restore settings
	dcImage.SelectObject(pOldBitmapImage);
	dcTrans.SelectObject(pOldBitmapTrans);
	pDC->SetBkColor(crOldBack);
	pDC->SetTextColor(crOldText);

	
}


/**************************************************************************************************
drawGlobalTimeLineHandler

Just draw the handler in the window. If the mouse is catching the handler then it also draws the
time label.


****************************************************************************************************/

void CTimeLineEditor::drawGlobalTimeLineHandler(CDC *pDC, int Time)
{
CPen		Handler; 
CPen		penOutline;
CPen		*pOldPen;
CBrush		FillColor(RGB(250,250,230));
CBrush		*pOldBrush;
CFont		tFont;
CFont		*pOldFont;
CRect		TimeTipLabel;
CDC			MemDCTimeLineHandler,MemDCTimeLineHandlerMask;
CBitmap		BmpTimeLineHandler, BmpTimeLineHandlerMask;
CRect		ClientArea;
int			nTime;
CString		sTime;
int			nHour,nMin,nSec;
int			nXDistance;
	

 //At the end of the physical client width we have a safe ENDPOINT finish
 //we use ENDPOINT just to make it clearer to user where the end of the timeline is
 //Now in case the mouse x is bigger of our m_nScaledTimeMax which means that is somewhere
 // inside the ENDPOINT and the normal physical end of the window just draw the handler
 //as it was in our virtual end which points the m_nScaledTimeMax
 if (Time >= m_nScaledTimeMax)  
 {
	 Time = m_GlobalCurrentTime = m_nScaledTimeMax;
 }
 
 GetClientRect(ClientArea);
 
 nXDistance = Time;


 //this is used to stop the handler not exactly at the end of the time but ENDPOINT pixels behide.
 //if m_sScaleTimeLine == FULLTIMELINE or HALFTIMELINE or CUSTOMSCALE go there and Only if the eventState == TIMELINEHANDLER
 if (m_eventState == TIMELINEHANDLER)
 {
 
		 if (m_sScaleTimeLine != SCALETOWINDOW)
		 {
			if ( (m_MouseX > ClientArea.right) || (Time == m_nScaledTimeMax) ) 
				 nXDistance = GetScrollPosition().x + ClientArea.right - ENDPOINT;
		 }
		 else
		 //if m_sScaleTimeLine == SCALETOWINDOW go there
		 //in this case the ENDPOINT is shown from the beggining
		 {
			if (m_MouseX > (ClientArea.right-ENDPOINT) )
				 nXDistance = GetScrollPosition().x + ClientArea.right - ENDPOINT;

		 }

		 if (m_MouseX < (ClientArea.left) )
			 nXDistance = GetScrollPosition().x + ClientArea.left; //ClientArea.left is usally =0, but anyway just to be sure
 }
//else if no caching the handler and we are the end, force to go ENDPOINT pixels back
 else if (Time == m_nScaledTimeMax)
	nXDistance = GetScrollPosition().x + ClientArea.right - ENDPOINT;


 //select pen for the handler vert line pointer
 Handler.CreatePen(PS_SOLID  ,1 , RGB(0,0,0) );
 pOldPen = pDC->SelectObject(&Handler);

 //draw vert line pointer
 pDC->MoveTo(nXDistance,GetScrollPosition().y+TIMELINEHANDLERTOPMARGIN+TIMELINETOOLBAR);
 pDC->LineTo(nXDistance,GetScrollPosition().y+HEADERTOP+HEADERBROWSER+TIMELINETOOLBAR);
 
 //draw handler
 BmpTimeLineHandler.LoadBitmap (IDB_TIMELINEHANDLER);	
 DrawTransparent(&BmpTimeLineHandler,pDC,nXDistance-5,TIMELINEHANDLERTOPMARGIN+TIMELINETOOLBAR,TIMELINEHANDLERWIDTH,TIMELINEHANDLERHEIGHT,RGB(255,0,255));
 

 //Draw Time Label next to handler only if LButton is Down
 if (m_eventState == TIMELINEHANDLER)
 {
	
	 //If handler is approaching the end of the client area (left side) then exchange the time label to the left side.
	if (  (m_MouseX >= ClientArea.Width()- TIMELINELABELWIDTH - 5 ) && (m_GlobalCurrentTime  <= m_nScaledTimeMax) )
	{
			
		TimeTipLabel.left	 = nXDistance - TIMELINEHANDLERWIDTH - TIMELINELABELWIDTH + 1; //add 1 for equal spaces between left and right
		TimeTipLabel.top	 = GetScrollPosition().y+TIMELINEHANDLERTOPMARGIN;
		TimeTipLabel.right  = TimeTipLabel.left + TIMELINELABELWIDTH;
		TimeTipLabel.bottom = TimeTipLabel.top + TIMELINELABELHEIGHT;			


	}
	else
	{
	 //else timeline is not approaching the right edge, so keep the label to the right side.
	 TimeTipLabel.left	 = nXDistance+TIMELINEHANDLERWIDTH;
	 TimeTipLabel.top	 = GetScrollPosition().y+TIMELINEHANDLERTOPMARGIN;
	 TimeTipLabel.right  = TimeTipLabel.left + TIMELINELABELWIDTH;	 	 
	 TimeTipLabel.bottom = TimeTipLabel.top + TIMELINELABELHEIGHT;

	}
	 
	 pOldBrush = pDC->SelectObject(&FillColor);
 

	 penOutline.CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
	 pDC->SelectObject(&penOutline);

	 pDC->Rectangle(TimeTipLabel.left,TimeTipLabel.top,TimeTipLabel.right,TimeTipLabel.bottom);

	 tFont.CreateFont(12, 0, 0, 0,0, 0,0,0,0,0,0,0,0,"Arial");
	 pOldFont = pDC->SelectObject(&tFont);
	 pDC->SetBkColor(GetSysColor(COLOR_WINDOW));
	 pDC->SetBkMode(TRANSPARENT);
	 pDC->SetTextColor(RGB(0,0,0));
	 
	 TimeTipLabel.left	 += 5; //spaces from the left of the label rec
	 TimeTipLabel.top	 += 2; //spaces from the top of the label rec
	 
	 //in case the handler is at the end of the time line
	 //force to show the total time. We use this because we have
	 //small divergence of the float decimals rejection
	 if (Time == m_nScaledTimeMax) 
		 nTime = m_nTimeMax;
	 else
		 nTime = (int) (Time * m_fDownScaleFactor);

	 SetTimeLineStateTime(nTime); // use this to update the current handler time as an output reference of this custom object

	 nHour = (nTime / 3600); //how many hours;
	 nTime = nTime - (nHour*3600);
	 nMin  = (nTime / 60);//mins
	 nTime = nTime - (nMin * 60) ;
	 nSec   = nTime;//seconds

	 
	 CTimeSpan CurrentTime(0, nHour, nMin, nSec); 

	 //draw the time
	 sTime = CurrentTime.Format(_T("%H:%M:%S")) + " / " + m_TotalTime.Format(_T("%H:%M:%S")); 
	 pDC->DrawText(sTime, TimeTipLabel, DT_BOTTOM);

	 
	 //restore old objects
	 pDC->SelectObject(pOldBrush);
 	 pDC->SelectObject(pOldFont);
 	 pDC->SelectObject(pOldPen);


 }


pDC->SelectObject(pOldPen);
}

/**********************************************************************************************
drawShowSBFxInfo

shows an info box when mouse is over of the SBFx for x Seconds. It show the str text

************************************************************************************************/

void CTimeLineEditor::drawShowSBFxInfoBallon(CDC *pDC,int Time, int Length, CString str,CString strDuration)
{

	CRect		ClientArea,TimeTipLabel;
	int			nXDistance;
	CBrush		*pOldBrush;
	CBrush		FillColor(RGB(120,120,150));
	CPen		*pOldPen,penOutline;
	CFont		*pOldFont,tFont;	
	CSize		csTextSize;
	int			nTime;
	CBitmap		BmpLabelIcon;	
	
	if (m_bShowSBXInfoBallon)
	{
			GetClientRect (ClientArea);
			nXDistance = GetScaledTime(Time);

			penOutline.CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
			pOldPen = pDC->SelectObject(&penOutline);
			pDC->SetBkMode(TRANSPARENT);

			tFont.CreateFont(13, 0, 0, 0,0, 0,0,0,0,0,0,ANTIALIASED_QUALITY,0,"Arial");
			pOldFont = pDC->SelectObject(&tFont);
			pDC->SetBkColor(GetSysColor(COLOR_WINDOW));
			pDC->SetBkMode(TRANSPARENT);
			pDC->SetTextColor(RGB(255,255,55));

 			pOldBrush = pDC->SelectObject(&FillColor);

			csTextSize=pDC->GetTextExtent("    "+str+"  "+strDuration+"     ");

			
			 //If handler is approaching the end of the client area (left side) then exchange the time label to the left side.
			if  ( (GetScaledTime(Time+Length) > ClientArea.Width()+GetScrollPosition().x) || (m_bScrollRight) || (GetScaledTime(Time)+csTextSize.cx >= m_nScaledTimeMax ) )
			{

				 TimeTipLabel.left	 = (GetScrollPosition().x + ClientArea.Width()) - csTextSize.cx - 5;
				 TimeTipLabel.top	 = GetScrollPosition().y+TIMELINEHANDLERTOPMARGIN-1;
				 TimeTipLabel.right  = TimeTipLabel.left + csTextSize.cx;	 	 
				 TimeTipLabel.bottom = TimeTipLabel.top + csTextSize.cy + 2;

			}
			else if   ( (m_bScrollLeft) || (GetScaledTime(Time) < GetScrollPosition().x) )
			{
				 TimeTipLabel.left	 = GetScrollPosition().x + 5 ;
				 TimeTipLabel.top	 = GetScrollPosition().y+TIMELINEHANDLERTOPMARGIN-1;
				 TimeTipLabel.right  = TimeTipLabel.left + csTextSize.cx;	 	 
				 TimeTipLabel.bottom = TimeTipLabel.top + csTextSize.cy + 2;


			}
			else
			{
				 //in normal conditions show label at the time point
				 TimeTipLabel.left	 = GetScaledTime(Time);
				 TimeTipLabel.top	 = GetScrollPosition().y+TIMELINEHANDLERTOPMARGIN-1;
				 TimeTipLabel.right  = TimeTipLabel.left + csTextSize.cx;	 	 
				 TimeTipLabel.bottom = TimeTipLabel.top + csTextSize.cy + 2;


			}
			 
			 pDC->Rectangle(TimeTipLabel.left,TimeTipLabel.top,TimeTipLabel.right+1,TimeTipLabel.bottom);

  
  
   

		   //draw Icon before text
		    BmpLabelIcon.LoadBitmap (IDB_LABELICON);	
		    DrawTransparent(&BmpLabelIcon,pDC,TimeTipLabel.left,TimeTipLabel.top,16,16,RGB(255,0,255));


			 
			 TimeTipLabel.left	 += 20; //spaces from the left of the label rec
			 TimeTipLabel.top	 += 1; //spaces from the top of the label rec
			 
			 //in case the handler is at the end of the time line
			 //force to show the total time. We use this because we have
			 //small divergence of the float decimals rejection
			 
			 if (Time == m_nScaledTimeMax) 
				 nTime = m_nScaledTimeMax;
			 else
				 nTime = Time;

			 


			 //show the text			
			 pDC->DrawText(str, TimeTipLabel, DT_BOTTOM);
			 if (strDuration != "")
			 {
				pDC->SetTextColor(RGB(255,255,255));					
				CSize textsize=pDC->GetTextExtent(str);				
				TimeTipLabel.left	 += textsize.cx+10;
				pDC->DrawText(strDuration, TimeTipLabel, DT_BOTTOM);
			 }
			 
			 //restore old objects
			 pDC->SelectObject(pOldBrush);
			 pDC->SelectObject(pOldFont);
			 pDC->SelectObject(pOldPen);
	}
}


/**********************************************************************************************
drawBackground

Just fill the background before any drawing on top of this

************************************************************************************************/

void CTimeLineEditor::drawBackground(CDC *pDC, CRect & BckGr)
{
	pDC->FillSolidRect(BckGr, GetSysColor(COLOR_3DFACE));
}



/*******************************************************************************************
UpdateSizes

Update the ScrollBar sizes according to the TimeLine length

********************************************************************************************/
void CTimeLineEditor::UpdateScrollSizes()
{
	CSize sizeTotal;
	sizeTotal.cy = 0;
	sizeTotal.cx = GetTimeLineWidth() + ENDPOINT;
	SetScrollSizes(MM_TEXT, sizeTotal);
}

/*******************************************************************************************
UseFont

Creates a specific font for the drawing purposes


*******************************************************************************************/
void CTimeLineEditor::UseFont(CFont &font, int height)
{
	font.CreateFont(height, 0, 0, 0,0, 0,0,0,0,0,0,0,0,"Arial");
}




/************************************************************************************************
 GetTimeLineHandlerRect
 
 Returns the rectangle where the TimeLineHandler resides
 Its used for the mouse hit tests
*************************************************************************************************/
CRect CTimeLineEditor::GetTimeLineHandlerRect(int time)
{
	CRect ret;
	
	ret.left   = time  - ( (TIMELINEHANDLERWIDTH / 2) + 5 );
	ret.right  = time  + ( (TIMELINEHANDLERWIDTH / 2) + 5 );

	ret.top    = TIMELINEHANDLERTOPMARGIN + + TIMELINETOOLBAR;
	ret.bottom = ret.top + TIMELINEHANDLERHEIGHT ;

	return ret;
}

/**********************************************************************************************
GetTimeLineHeaderRect


Rectangle Processing for the whole timeline Header (Its the place where the handler resides)
Its used for the mouse hit tests, to inform when the user click inside the timeline header to move 
the handler at this specific point
*************************************************************************************************/
CRect CTimeLineEditor::GetTimeLineHeaderRect()
{
	CRect ret,ClientArea;
	
	GetClientRect(ClientArea);

	ret.left   = GetScrollPosition().x;
	ret.right  = ret.left + ClientArea.Width ();

	ret.top    = GetScrollPosition().y + TIMELINETOOLBAR;
	ret.bottom = ret.top + HEADERBROWSER;

	return ret;
}

/***********************************************************************************************************************
GetSpanEdge


Returns the span (which is the timeline bar) rectangle
************************************************************************************************************************/
CRect CTimeLineEditor::GetSpanEdge(int Span)
{
	CRect SpanEdge;
	SpanEdge.left = GetScrollPosition().x;
	SpanEdge.right = SpanEdge.left + GetTimeLineWidth();
	SpanEdge.top = HEADERTOP + TIMELINETOOLBAR;
	SpanEdge.bottom = SpanEdge.top + SLICEHEIGHT;
	return SpanEdge;
}

/**********************************************************************************************************************
GetSliceRect

The rect of the slice



************************************************************************************************************************/
CRect CTimeLineEditor::GetSliceRect(int Span, int Time, int Length)
{
	CRect Slice;
	Slice.left = GetSliceLeft(Time);
	Slice.right = GetSliceRight(Time, Length);
	Slice.top = GetScrollPosition().y + HEADERBROWSER + TIMELINETOOLBAR;
	Slice.bottom = Slice.top + HEADERTOP;
	return Slice;
}

/**********************************************************************************************************************
GetSliceBodyRect

The rect of the slice body



************************************************************************************************************************/
CRect CTimeLineEditor::GetSliceBodyRect(int Span, int Time, int Length, int InFxDuration, int OutFxDuration)
{
	CRect Slice;
	Slice.left = GetSliceLeft(Time) + GetScaledTime(InFxDuration);
	Slice.right = GetSliceRight(Time, Length) - GetScaledTime(OutFxDuration);
	Slice.top = GetScrollPosition().y + HEADERBROWSER + TIMELINETOOLBAR;
	Slice.bottom = Slice.top + HEADERTOP;
	return Slice;
}


/**********************************************************************************************************************
GetSliceFxInBody

The rectangle of the left handle side FX = InFxDuration
***********************************************************************************************************************/
CRect CTimeLineEditor::GetSliceFxInBody(int Span, int Time, int Length, int InFxDuration)
{
	CRect Handle;
	Handle.left =  GetSliceLeft(Time);
	Handle.right = Handle.left  + GetScaledTime(InFxDuration) - SLICELEFTHANDLEWIDTH;
	Handle.top =    GetScrollPosition().y + HEADERBROWSER + TIMELINETOOLBAR;
	Handle.bottom = Handle.top + HEADERTOP;
	return Handle;
}

/**********************************************************************************************************************
GetSliceFxOutBody

The rectangle of the right handle side FX = OutFxDuration
***********************************************************************************************************************/
CRect CTimeLineEditor::GetSliceFxOutBody(int Span, int Time, int Length, int OutFxDuration)
{
	CRect Handle;
	Handle.left =  GetSliceLeft(Time) + GetScaledTime(Length - OutFxDuration) + SLICERIGHTHANDLEWIDTH;
	Handle.right = Handle.left  + GetScaledTime(OutFxDuration);
	Handle.top =    GetScrollPosition().y + HEADERBROWSER + TIMELINETOOLBAR;
	Handle.bottom = Handle.top + HEADERTOP;
	return Handle;
}

/**********************************************************************************************************************
GetSliceHandleLeft

The rectangle of the left handle of the slice (it is the black vert line between slice and InFxDuration)
***********************************************************************************************************************/
CRect CTimeLineEditor::GetSliceHandleLeft(int Span, int Time, int Length, int InFxDuration)
{
	CRect Handle;
	Handle.right =  GetSliceLeft(Time) + GetScaledTime(InFxDuration);
	Handle.left =   Handle.right - SLICELEFTHANDLEWIDTH;
	Handle.top =    GetScrollPosition().y + HEADERBROWSER + TIMELINETOOLBAR;
	Handle.bottom = Handle.top + HEADERTOP;
	return Handle;
}
/*********************************************************************************************************************
GetSliceHandleRight


The rectangle of the right handle of the slice (it is the black vert line between slice and OutFxDuration)
***********************************************************************************************************************/
CRect CTimeLineEditor::GetSliceHandleRight(int Span, int Time, int Length, int OutFxDuration)
{
	CRect Handle;
	Handle.left = GetSliceRight(Time, Length) - GetScaledTime(OutFxDuration);
	Handle.right = Handle.left + SLICERIGHTHANDLEWIDTH;
	Handle.top = GetScrollPosition().y + HEADERBROWSER + TIMELINETOOLBAR;
	Handle.bottom = Handle.top + HEADERTOP;
	return Handle;
}



/**********************************************************************************************************************
GetSliceHandleInFxLeft

The rectangle of the most left handle of the slice (InFxDuration handle)
***********************************************************************************************************************/
CRect CTimeLineEditor::GetSliceHandleInFxLeft(int Span, int Time, int Length, int InFxDuration)
{
	CRect Handle;
	Handle.right =  GetSliceLeft(Time);
	Handle.left =   Handle.right - (SLICEEDGELEFTWIDTH * 2);
	Handle.top =    GetScrollPosition().y + HEADERBROWSER + TIMELINETOOLBAR;
	Handle.bottom = Handle.top + HEADERTOP;
	return Handle;
}


/**********************************************************************************************************************
GetSBFxBodyRect

The rect of the slice



************************************************************************************************************************/
CRect CTimeLineEditor::GetSBFxBodyRect(int Span, int Time, int Length)
{
	CRect Slice;
	Slice.left = GetSliceLeft(Time);
	Slice.right = GetSliceRight(Time, Length)+1; //add one since the PtInRect does not include the right most side.
	Slice.top = GetScrollPosition().y + HEADERBROWSER + TIMELINETOOLBAR;
	Slice.bottom = Slice.top + HEADERTOP;
	return Slice;
}

/*********************************************************************************************************************
GetSBFxHandleRight


The rectangle of the right handle of the SBFx (it is the right most vert line of the SBFx body)
***********************************************************************************************************************/
CRect CTimeLineEditor::GetSBFxHandleRight(int Span, int Time, int Length)
{
	CRect Handle;
	Handle.left =  GetSliceRight(Time, Length)-1;	
	Handle.right = Handle.left + 3;	
	Handle.left--;
	Handle.top = GetScrollPosition().y + HEADERBROWSER + TIMELINETOOLBAR;
	Handle.bottom = Handle.top + HEADERTOP;
	return Handle;
}



/**********************************************************************************************************************
GetSBFxHandleLeft

The rectangle of the most left handle of the slice (it is the left most vert line of the SBFx body)
Each handler has 4 pixel wide
***********************************************************************************************************************/
CRect CTimeLineEditor::GetSBFxHandleLeft(int Span, int Time, int Length)
{
	CRect Handle;
	Handle.left =   GetSliceLeft(Time); //from the currect point -1
	Handle.right = Handle.left + 3;	//add SLICEINOUTFXWIDTH=1 since PtInRect does not take in mind the right most side
	Handle.left--;
	Handle.top =    GetScrollPosition().y + HEADERBROWSER + TIMELINETOOLBAR;
	Handle.bottom = Handle.top + HEADERTOP;
	return Handle;
}
/*********************************************************************************************************************
GetSliceHandleOutFxRight


The rectangle of the most right handle of the slice (OutFxDuration handle)
***********************************************************************************************************************/
CRect CTimeLineEditor::GetSliceHandleOutFxRight(int Span, int Time, int Length, int OutFxDuration)
{
	CRect Handle;
	Handle.left = GetSliceRight(Time, Length);
	Handle.right = Handle.left + (SLICEEDGERIGHTWIDTH * 2);
	Handle.top = GetScrollPosition().y + HEADERBROWSER + TIMELINETOOLBAR;
	Handle.bottom = Handle.top + HEADERTOP;
	return Handle;
}



/*****************************************************************************************************
GetScaledTime

It returns the scaled time when a scale other than a FULLTIMELINE is selected




********************************************************************************************************/
int	CTimeLineEditor::GetScaledTime(int time)
{
	int nScaledTime;


	nScaledTime = (int) ( time / m_fDownScaleFactor );

	if (nScaledTime > m_nScaledTimeMax)
			nScaledTime = m_nScaledTimeMax;

	return nScaledTime; 


}
/*****************************************************************************************************
GetBckTopRect

The drawing background for the timeline window


********************************************************************************************************/
CRect CTimeLineEditor::GetBckTopRect()
{
	CRect BckGr;
	GetClientRect(BckGr);
	BckGr.top = 0;
	BckGr.bottom = SPANTOPMARGIN;
	BckGr.left += GetScrollPosition().x;
	BckGr.right += GetScrollPosition().x;
	return BckGr;
}
/*******************************************************************************************************
GetBckBottomRect
we have no extra heights Not used !! DELETE AT THE FINAL STAGE


********************************************************************************************************/
CRect CTimeLineEditor::GetBckBottomRect()
{
	CRect BckGr;
	GetClientRect(BckGr);
	BckGr.top = 0;
	BckGr.bottom+= GetScrollPosition().y;
	BckGr.left += GetScrollPosition().x;
	BckGr.right += GetScrollPosition().x;
	if(BckGr.bottom<BckGr.top)
	{
		BckGr.bottom = BckGr.top + 1;
	}
	return BckGr;
}
/*********************************************************************************************************
GetTimeRect

Returns the rect for the actual time line. Not used !! DELETE AT THE FINAL STAGE

**********************************************************************************************************/
CRect CTimeLineEditor::GetTimeRect(int Time)
{
	CRect tRect;
	tRect.left = GetTimeToX(Time);
	tRect.right = tRect.left + 100;
	tRect.top = GetScrollPosition().y + HEADERBROWSER - 13;
	tRect.bottom = GetScrollPosition().y + HEADERBROWSER - 1;
	return tRect;
}

void CTimeLineEditor::ExpandRect(CRect &SliceRect)
{
	SliceRect.left   -= SliceRect.Width();
	SliceRect.right  += SliceRect.Width();// yes, it changes, besides the point
	SliceRect.top    -= SliceRect.Height();
	SliceRect.bottom += SliceRect.Height();// yes, it changes, besides the point
}

/*********************************************************************************
OnScrollBy

Everytime the framework manipulate the view using the scrollbar just update
the view


**********************************************************************************/
BOOL CTimeLineEditor::OnScrollBy(CSize sizeScroll, BOOL bDoScroll) 
{
	// TODO: Add your specialized code here and/or call the base class
	Refresh();
	return CScrollView::OnScrollBy(sizeScroll, bDoScroll);
}
/*********************************************************************************
OnScroll

Everytime the framework manipulate the view using the scrollbar just update
the view


**********************************************************************************/
BOOL CTimeLineEditor::OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll) 
{
	// TODO: Add your specialized code here and/or call the base class
	Refresh();
	return CScrollView::OnScroll(nScrollCode, nPos, bDoScroll);
}


/********************************************************************************
OnSetCursor
Set the mouse cursor in various positions.



*********************************************************************************/
BOOL CTimeLineEditor::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// TODO: Add your message handler code here and/or call default

   // only change the cursor when it's within the client area
		

		//ON EVENTS MOUSE CURSORS
		switch(m_eventState)
		{
			case LEFTHANDLER: // if the left handle is moved

				break;
			case RIGHTHANDLER: // if the right handle is moved

				break;
			case SLICEHANDLER: // if the whole is moved
					//SetCursor(::LoadCursor(NULL, IDC_APPSTARTING) );
					//return TRUE;
				break;
			case TIMELINEHANDLER: // if the whole is moved
					::SetCursor(AfxGetApp()->LoadCursor(IDC_HANDCATCH));
					return TRUE;
				break;
			case OVERSLICE: // if the whole is moved
			case OVERSLICEFXINBODY:
			case OVERSLICEFXOUTBODY:
					::SetCursor(AfxGetApp()->LoadCursor(IDC_HANDCATCH));
					return TRUE;
				break;
			case OVERSLICELEFTHANDLE:
					::SetCursor(AfxGetApp()->LoadCursor(IDC_RESIZEHOR));
					return TRUE;
				break;
			case OVERSLICERIGHTHANDLE:
					::SetCursor(AfxGetApp()->LoadCursor(IDC_RESIZEHOR));
					return TRUE;
				break;
			case OVERSLICEINFXLEFTHANDLE:
					::SetCursor(AfxGetApp()->LoadCursor(IDC_RESIZEHORFLAT));
					return TRUE;
				break;
			case OVERSLICEOUTFXRIGHTHANDLE:
					::SetCursor(AfxGetApp()->LoadCursor(IDC_RESIZEHORFLAT));
					return TRUE;
				break;
			case BODYSLICEFX: // if the whole SBFx is moved
					::SetCursor(AfxGetApp()->LoadCursor(IDC_HANDCATCH));
					return TRUE;
				break;
			case BODYSLICEFXLEFTHANDLE: 
					::SetCursor(AfxGetApp()->LoadCursor(IDC_RESIZEHOR));
					return TRUE;
				break;
			case BODYSLICEFXRIGHTHANDLE: 
					::SetCursor(AfxGetApp()->LoadCursor(IDC_RESIZEHOR));
					return TRUE;
				break;



			default:
				break;
		}


		//ON HOVER MOUSE CURSORS
		if (m_eventState == NOEVENT)
		{
			switch(m_MouseState)
			{

				case OVERTIMELINEHANDLER:
						::SetCursor(AfxGetApp()->LoadCursor(IDC_HANDFREE));
						return TRUE;
					break;
				case OVERSLICE:
				case OVERSLICEFXINBODY:
				case OVERSLICEFXOUTBODY:
						::SetCursor(AfxGetApp()->LoadCursor(IDC_HANDFREE));
						return TRUE;
					break;
				case OVERSLICELEFTHANDLE:
						::SetCursor(AfxGetApp()->LoadCursor(IDC_RESIZEHOR));
						return TRUE;
					break;
				case OVERSLICERIGHTHANDLE:
						::SetCursor(AfxGetApp()->LoadCursor(IDC_RESIZEHOR));
						return TRUE;
					break;
				case OVERSLICEINFXLEFTHANDLE:
						::SetCursor(AfxGetApp()->LoadCursor(IDC_RESIZEHORFLAT));
						return TRUE;
					break;
				case OVERSLICEOUTFXRIGHTHANDLE:
						::SetCursor(AfxGetApp()->LoadCursor(IDC_RESIZEHORFLAT));
						return TRUE;
					break;
			case BODYSLICEFX: // if the whole SBFx is moved
					::SetCursor(AfxGetApp()->LoadCursor(IDC_HANDFREE));
					return TRUE;
				break;
			case BODYSLICEFXLEFTHANDLE: 
					::SetCursor(AfxGetApp()->LoadCursor(IDC_RESIZEHOR));
					return TRUE;
				break;
			case BODYSLICEFXRIGHTHANDLE: 
					::SetCursor(AfxGetApp()->LoadCursor(IDC_RESIZEHOR));
					return TRUE;
				break;

				default:
					break;
			}
		}



	return CScrollView::OnSetCursor(pWnd, nHitTest, message);
}

/****************************************************************************************************
SetTimeLineStateTime

Sets the Current Time Position. This is private. Get the position with the GetTimeLineStateTime()
this is equal to the internal m_GlobalCurrentTime

*******************************************************************************************************/

void CTimeLineEditor::SetTimeLineStateTime(int time)
{
	
	m_nTimeLineStateTime = time;

}

/********************************************************************************************************
GetTimeLineStateTime

Gets the Current Time Position. This is public. This is where the handler points in
the timeline.

*********************************************************************************************************/

int CTimeLineEditor::GetTimeLineStateTime()
{

	return m_nTimeLineStateTime;

}

/*********************************************************************************************************
OnTimer

The timer is used to scroll the timeline either in left or right when the mouse
cursor is outside the client area and the LMouseButton is pressed in the TimeLineHandler. 
Its necessery to use timer when we want to keep scrolling even there is no mouse event occurs 
in non client area. Ie, in the idle of mouse (no mouse move or no button events) we can not occur 
events to use for scrolling, so our timer will do the job. We will the timer in LMouseUp or
when we reached the end or the start of the timeline.

********************************************************************************************************/

void CTimeLineEditor::OnTimer(UINT_PTR nIDEvent)
{
	

	CRect		ClientRect;
	int			nStep;
	CSize		nTotalScrollSize;
	CPoint		pt;
	int			nStart, nLength, nFxInDuration, nFxOutDuration,nSliceMoveNewStart;
	double		dActionID;
	CPoint		cpOldMouseXY;



		GetClientRect(&ClientRect);

		//*******************************************
		//Scroll Timeline while draging the handler
		//*******************************************
		if (m_eventState == TIMELINEHANDLER) 
		{
			//automatically scroll when the handler reach the edges;
			if ( m_bScrollRight )
			{


				nStep = m_MouseX - ClientRect.right; //the differnce between MouseX - right client endge
				//the bigger the difference then bigger step to scroll.
			
				if (nStep <=0)
					nStep = 1;

				pt.y = GetScrollPosition().y;
				pt.x = GetScrollPosition().x + nStep;
				ScrollToPosition(pt);
				m_GlobalCurrentTime += nStep;
				if (m_GlobalCurrentTime > m_nScaledTimeMax)
				{
					m_GlobalCurrentTime = m_nScaledTimeMax;
					nTotalScrollSize = GetTotalSize();
					pt.x = nTotalScrollSize.cx;
					ScrollToPosition(pt);
					KillCurrentTimer(); //we are in the end. stop timer.
				}

				Refresh();
				UpdateWindow();



			}
			else if ( m_bScrollLeft )
			{

				nStep = m_MouseX; //m_MouseX now may have negative value due the SetCapture/ReleaseCapture mouse functions
				//the bigger the difference then bigger step to scroll.
			
				if (nStep >= 0)
					nStep = 1;


				pt.y = GetScrollPosition().y;
				pt.x = GetScrollPosition().x + nStep; //dont forget nStep has negative values
				ScrollToPosition(pt);
				m_GlobalCurrentTime += nStep;
				if (m_GlobalCurrentTime <= 0)
				{
					m_GlobalCurrentTime = 0;
					pt.x = 0;
					ScrollToPosition(pt);
					KillCurrentTimer(); //we are in the end. stop timer.
				}

				Refresh();
				UpdateWindow();



			}

		}
		
		//*****************************************
		//Scroll timeline while dragging the Slice
		//*****************************************
		if (m_eventState == OVERSLICE) 
		{
			//automatically scroll when the handler reach the edges;
			if ( m_bScrollRight )
			{


				nStep = m_MouseX - ClientRect.right; //the differnce between MouseX - right client endge
				//the bigger the difference then bigger step to scroll.
			
				if (nStep <=0)
					nStep = 1;

				pt.y = GetScrollPosition().y;
				pt.x = GetScrollPosition().x + nStep;
				ScrollToPosition(pt);
				


				//Get current values
				m_TimeLineSpan.GetSlice(m_nActiveSpan,m_nActiveSlice,nStart,nLength,nFxInDuration,nFxOutDuration,dActionID);
				
				//use the uncompressed time since the drawTimeSlice will do the job of scaling
				nSliceMoveNewStart = nStart + (int) (nStep * m_fDownScaleFactor);

				if ( (nSliceMoveNewStart + nLength) > m_nTimeMax)
				{
					nSliceMoveNewStart = m_nTimeMax - nLength;
					nTotalScrollSize = GetTotalSize();
					pt.x = nTotalScrollSize.cx;
					ScrollToPosition(pt);
					KillCurrentTimer(); //we are in the end. stop timer.
				}

				//set new values
				m_TimeLineSpan.Edit(m_nActiveSpan,m_nActiveSlice, nSliceMoveNewStart ,nLength,nFxInDuration,nFxOutDuration,dActionID);
				
				
				Refresh();
				UpdateWindow();



			}
			else if ( m_bScrollLeft )
			{

				nStep = m_MouseX; //m_MouseX now may have negative value due the SetCapture/ReleaseCapture mouse functions
				//the bigger the difference then bigger step to scroll.
			
				if (nStep >= 0)
					nStep = 1;


				pt.y = GetScrollPosition().y;
				pt.x = GetScrollPosition().x + nStep; //dont forget nStep has negative values
				ScrollToPosition(pt);

				//Get current values
				m_TimeLineSpan.GetSlice(m_nActiveSpan,m_nActiveSlice,nStart,nLength,nFxInDuration,nFxOutDuration,dActionID);
				
				//use the uncompressed time since the drawTimeSlice will do the job of scaling
				nSliceMoveNewStart = nStart + (int) (nStep * m_fDownScaleFactor);
		
				if ( (nSliceMoveNewStart) <= 0)
				{
					nSliceMoveNewStart = 0;					
					pt.x = 0;
					ScrollToPosition(pt);
					KillCurrentTimer(); //we are in the end. stop timer.
				}

				//set new values
				m_TimeLineSpan.Edit(m_nActiveSpan,m_nActiveSlice, nSliceMoveNewStart ,nLength,nFxInDuration,nFxOutDuration,dActionID);				

				Refresh();
				UpdateWindow();



			}

		}


		//*****************************************
		//Count time before it shows the Ballon Info
		//for the SBFx slices
		//*****************************************
		if ( (m_nBallonTimer != 0) && (m_eventState == NOEVENT) )
		{
			//The time is over but maybe the user has 
			//already moved the mouse cursor or just drag
			//the slice. So do a last check 
			if ( (m_MouseState == BODYSLICEFX)  || (m_MouseState == OVERSLICEFXINBODY) 	|| (m_MouseState == OVERSLICEFXOUTBODY))
			{
				m_bShowSBXInfoBallon = TRUE;
				Refresh();
				UpdateWindow();
			}

			KillTimer(m_nBallonTimer); //we are in the end. stop timer.
			m_nBallonTimer=0;
		}

	CScrollView::OnTimer(nIDEvent);
}


/*****************************************************************************************************************
KillCurrentTimer

Just destroys the active timer and removes any WM_TIMER events may be in the queue

*****************************************************************************************************************/
void CTimeLineEditor::KillCurrentTimer()
{
	MSG		msg;

	if (m_nTimer != 0 )
		KillTimer(m_nTimer);

	m_nTimer = 0;
	
	while	(PeekMessage (&msg, m_hWnd, WM_TIMER, WM_TIMER, PM_REMOVE));// Eat any extra WM_TIMER messages

}

/*****************************************************************************************************************
SetHandlerPos

Move the handler at a specific time (seconds)
*****************************************************************************************************************/
void CTimeLineEditor::SetHandlerPos(int time)
{



}

/****************************************************************************************************************
CreateNewSlice

Creates a new slice in the timeline control.


*****************************************************************************************************************/
void CTimeLineEditor::CreateNewSlice(int ID, int StartTime, int Duration, int TimeFxInDuration, int TimeFxOutDuration, int ActionID)
{

	m_TimeLineSpan.AddTimeSpan(ID,ActionID);
	m_TimeLineSpan.AddTimeSlice(ID, StartTime, Duration, TimeFxInDuration, TimeFxOutDuration, ActionID);
	SelectSlice(ID); //make it active

}

/****************************************************************************************************************
AddNewFxToSlice

Creates a new fx into the slice body 


*****************************************************************************************************************/
BOOL CTimeLineEditor::AddNewFxToSlice(int Span, int Slice, int Time, int Length, double ActionID)
{

	return m_TimeLineSpan.AddNewFxToSlice(Span, Slice, Time, Length, ActionID);

}

/****************************************************************************************************************
SelectSlice

Creates a new slice in the timeline control.


*****************************************************************************************************************/
void CTimeLineEditor::SelectSlice(int ID)
{

	m_nActiveSlice = ID;
	m_nActiveSpan = ID;
	Refresh(); //force repaint to show the new selected slice



}
/**************************************************************************************************************
round

A workaround for the simple round function of a float number




***************************************************************************************************************/
int CTimeLineEditor::round(float num)
{
	return (int) floor(num + 0.5); 

}
/**************************************************************************************************************
OnSize

reinitialize things that has to do with the client window dimensions.


***************************************************************************************************************/
void CTimeLineEditor::OnSize(UINT nType, int cx, int cy)
{
	CScrollView::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	
	//change the size only when the window has initialized and we have set things
	if (m_nTimeMax > 0)
	{
		SetTimeMax(m_nTimeMax);
		SetTimeLineScale(m_sScaleTimeLine,(int) m_fDownScaleFactor );

	}

}


/*****************************************************************************************************************
OnSlicebodyDuration

Just Opens a dialog and changes the duration of the Slice Body
It called from the pop up menu (right click)



*******************************************************************************************************************/
void CTimeLineEditor::OnSlicebodyDuration()
{
	// TODO: Add your command handler code here
	int					 nStart, nLength, nFxInDuration, nFxOutDuration;
	double				 dActionID;
	CChangeDurationDlg	 dlg;

	m_TimeLineSpan.GetSlice(m_nActiveSpan,m_nActiveSlice,nStart,nLength,nFxInDuration,nFxOutDuration,dActionID);

	dlg.m_nSliceDuration = nLength;
	if (dlg.DoModal () == IDOK)
	{

		nLength = _tstoi(dlg.m_strDuration);
		m_TimeLineSpan.Edit(m_nActiveSpan,m_nActiveSlice, nStart ,nLength,nFxInDuration,nFxOutDuration,dActionID);
		Refresh();
	}
	
}

/*****************************************************************************************************************
OnContextMenu

Mouse Right Click take care function. It just pops up a menu selection.




*******************************************************************************************************************/

void CTimeLineEditor::OnContextMenu(CWnd* pWnd, CPoint point)
{
	// TODO: Add your message handler code here

	
	
	CPoint		M;
	
	ScreenToClient(&point);
		
	M = point + GetScrollPosition(); // augment the point to rendering space;


	EVENTS res = TestPos(M, m_nActiveSpan, m_nActiveSlice); //Did I click on something important

	ClientToScreen(&point);

	if(res != NOEVENT)
	{
		switch(res)
		{


		case OVERSLICE:
			m_eventState = NOEVENT; // reset the state

		    CMenu menu;
			menu.LoadMenu (IDR_CONTEXT_SLICEBODY);
			CMenu* pContextMenu = menu.GetSubMenu (0);

		
			pContextMenu->TrackPopupMenu (TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, point.x, point.y, AfxGetMainWnd ());
			return;

		
		
		}
	}

	//
    // Call the base class if the shape was not clicked.
    //
    CWnd::OnContextMenu (pWnd, point);

}


/*****************************************************************************************************************
OnSlicebodyDelete

Deletes the active SliceBody. It called from the pop up menu (right click)




*******************************************************************************************************************/
void CTimeLineEditor::OnSlicebodyDelete()
{
	// TODO: Add your command handler code here


	m_TimeLineSpan.Delete (m_nActiveSpan);
	SelectSlice(-1);
	m_eventState = NOEVENT;
}

/*****************************************************************************************************************
HitLeftFxSplice

Checks nNewFxSize with every Body Slice Fx Start position. If the nNewFxSize relies inside of any Body Slice Fx then
return the outter left position (start var of the outter left SBfx) of the body slice fx group.
In other case (no collision) return -1



*******************************************************************************************************************/

int	CTimeLineEditor::HitLeftFxSplice(int nNewFxSize)
{
	int			start, length;
	double		actionID;

	int			nStart,nLength,nFxInDuration,nFxOutDuration;
	double		dActionID;


	//check to see if we have any SBFx 
	if (m_TimeLineSpan.GetSliceFxCount(m_nActiveSpan, m_nActiveSlice) > 0 ) 
	{
	
		//Get the first. They are sorted so dont bother to check all the SBFxs
		m_TimeLineSpan.GetFxToSlice(m_nActiveSpan,m_nActiveSpan,0,start,length,actionID);

		//Get some infos from the active Slice
		m_TimeLineSpan.GetSlice(m_nActiveSpan,m_nActiveSlice,nStart,nLength,nFxInDuration,nFxOutDuration,dActionID);

		if ( (nNewFxSize) >= (nStart + start) )
			  return (nStart+start);

	}

		


	return -1;

}

/*****************************************************************************************************************
HitRightFxSplice

Checks nNewFxSize with every Body Slice Fx length position. If the nNewFxSize relies inside of any Body Slice Fx then
return the outter right position (start+length var of the outter right SBfx) of the body slice fx group.
In other case (no collision) return -1



*******************************************************************************************************************/

int	CTimeLineEditor::HitRightFxSplice(int nNewFxSize)
{
	int			start, length;
	double		actionID;

	int			nStart,nLength,nFxInDuration,nFxOutDuration;
	double		dActionID;
	int			nLastSliceFx;


	//check to see if we have any SBFx 
	if (m_TimeLineSpan.GetSliceFxCount(m_nActiveSpan, m_nActiveSlice) > 0 ) 
	{
	
		nLastSliceFx = m_TimeLineSpan.GetSliceFxCount(m_nActiveSpan, m_nActiveSlice) - 1;

		//Get the last. They are sorted so dont bother to check all the SBFxs
		m_TimeLineSpan.GetFxToSlice(m_nActiveSpan,m_nActiveSpan,nLastSliceFx,start,length,actionID);

		//Get some infos from the active Slice
		m_TimeLineSpan.GetSlice(m_nActiveSpan,m_nActiveSlice,nStart,nLength,nFxInDuration,nFxOutDuration,dActionID);
		
		if ( nNewFxSize <= (nStart + start + length))
			  return (nStart + start + length);

	}


	return -1;

}