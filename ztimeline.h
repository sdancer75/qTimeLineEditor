/**********************************************************************************************************

This is part of the TimeLine Control of the Paradox Interactive DLL custom drawn object
ztimeline library is used as it is with slightly modified to comply with the my object
The original copyright is not removed.

Date Updated	: July 2008

--------------- original copyrights ----------------
 Zenerd Standard Library
 Copyright (C) 2000-2002 by Zenerd

 Component: Time Line Data-Structure

 Date     : October 2001

 Author(s): Jeffrey M. Barber

 --------------------------------------------------

 Notes    : NO BOUND CHECKING
		  : Modified to comply to my needs
		  : Comments are added by G. Papaioannou

 
************************************************************************************************************/

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <vector>
#include <algorithm>
using namespace std;



/***********************************************************************************************
Class ZSliceInternalFx

ZSliceInternalFx represents time slices between InFxDuration and OutFxDuration

Its used to represent fxs inside the body of the slice ie

						 /-------- Slice Body Fx or SBFx-----\
 _______________________/_____________________________________\________________________
|				|		|					|					|					 |
|  InFxDurtaion |		| ZSliceInternalFx 1|ZSliceInternalFx 2	|	OutFxDurtaion	 |
|_______________|_______|___________________|___________________|____________________|
				\-------------Body of the Slice ----------------/

The actual structure of the ZTimeSlice can hold 

1. start of the fx (represented by nTime_start member)
2. duration of the fx (represented by nTime_length member)
3. action ID of the fx (nFxActionID). It can used in any way you want

***********************************************************************************************/

class ZSliceInternalFx
{
	private:
		int		m_nTime_start; // start time
		int		m_nTime_length;
		double	m_nFxActionID;

		char compare(const ZSliceInternalFx &c);
		bool ZSliceInternalFx::isIn(int time);

	public:
		ZSliceInternalFx();
		ZSliceInternalFx(int start, int length, double actionID);
		~ZSliceInternalFx();

		int		GetSliceFxStart();
		int		GetSliceFxLength();
		double	GetSliceFxActionID();
		void	GetInternalFx(int &Start, int &Length, double &ActionID);

		void operator = (const ZSliceInternalFx &c);
		bool operator <  (const ZSliceInternalFx &c);
		bool operator >  (const ZSliceInternalFx &c);
		bool operator == (const ZSliceInternalFx &c);
		bool operator <= (const ZSliceInternalFx &c);
		bool operator >= (const ZSliceInternalFx &c);

		void EditSliceFx(int start, int length, double actionID);
		void GetFxToSlice(int &start, int &length, double &actionID);


};

/***********************************************************************************************
Class ZTimeSlice

TimeSlice represents the actual object on the timeline.

The actual structure of the ZTimeSlice can hold 

1. start of the slice (represented by m_time_start member)
2. duration of the slice (represented by m_time_length member)
3. action ID of the slice (m_action_weight by m_time_length member). It can used in any way you want

***********************************************************************************************/

class ZTimeSlice
{
	private:
		int							m_time_start; // start time
		int							m_time_length; // length of action
		int							m_timeFxInDuration; //duration of the in Fx
		int							m_timeFxOutDuration; //duration of the out Fx
		double						m_action_weight; // what is the action weight, or action details
		vector<ZSliceInternalFx>	m_InternalFx; //fxs inside the slice body. This is the part between m_timeFxInDuration & m_timeFxOutDuration

	private:
		char compare(const ZTimeSlice & c);
		void swap(int a, int b);
		void qsort(int l = -100, int r = -100);



	public:
		// basic management details
		ZTimeSlice();
		ZTimeSlice(const ZTimeSlice & c);
		ZTimeSlice(int start, int length, int TimeFxInDuration, int TimeFxOutDuration, double weight);
		~ZTimeSlice();

		void operator = (const ZTimeSlice & c);
		bool operator <  (const ZTimeSlice & c);
		bool operator >  (const ZTimeSlice & c);
		bool operator == (const ZTimeSlice & c);
		bool operator <= (const ZTimeSlice & c);
		bool operator >= (const ZTimeSlice & c);


		bool	isIn(int time);

		void	SetStart(int start);
		void	SetLength(int length);
		void	SetWeight(double weight);
		void	SetTimeFxInDuration(int length);
		void	SetTimeFxOutDuration(int length);

		
		
		int		GetSliceFxCount();
		int		GetStart();
		int		GetLength();
		double	GetWeight();
		int		GetTimeFxInDuration();
		int		GetTimeFxOutDuration();
		void	GetFxToSlice(int Fx, int &Start, int &Length, double &ActionID);


		void	Delete(int ID);
		void	Edit(int start, int length, int TimeFxInDuration, int TimeFxOutDuration, double weight);
		bool	AddSliceFx(int start, int length, double actionID);
		void	EditSliceFx(int FxSlice,int start, int length, double actionID);
		int		get_time_end();
};


/*************************************************************************************************************
class ZTimeSpan

Time span is used when we need multiple time lines one down the other. Its the railroad where the
TimeSlice sits on top.

It holds the ZTimeSlice structure in the m_splices as well as the 2 member vars for the ownwer and the action

***************************************************************************************************************/

class ZTimeSpan
{
	private:
		vector<ZTimeSlice> m_splices;
		int m_action;
		int m_owner;
	private:
		void swap(int a, int b);
		void qsort(int l = -100, int r = -100);
		char compare(const ZTimeSpan & c);
	public:
		ZTimeSpan(int owner = 0, int action = 0);
		ZTimeSpan(const ZTimeSpan & c);
		~ZTimeSpan();
		
		void operator =  (const ZTimeSpan &c);
		bool operator <  (const ZTimeSpan &c);
		bool operator >  (const ZTimeSpan &c);
		bool operator == (const ZTimeSpan &c);
		bool operator <= (const ZTimeSpan &c);
		bool operator >= (const ZTimeSpan &c);


		// data insertion
		bool	AddSplice(int start, int length, int TimeFxInDuration, int TimeFxOutDuration, double weight);
		void	SetOwner(int owner);
		void	SetAction(int action);

		int		GetOwner();
		int		GetAction();
		int		GetSliceAt(int time);
		void	GetFxToSlice(int Slice, int Fx,  int &Start,  int &Length, double &ActionID);

		void	Edit(int slice, int start, int length, int TimeFxInDuration, int TimeFxOutDuration, double weight);
		void	Delete(int slice);

		int		GetSliceCount();
		int		GetSliceFxCount(int slice);
		void	GetSlice(int Slice, int &Start, int &Length, int &TimeFxInDuration, int &TimeFxOutDuration, double &Weight);
		

		bool	AddSliceFx(int slice, int start, int length, double actionID);
		void	EditSliceFx(int slice, int FxSlice,int start, int length, double actionID);
		
};


/*************************************************************************************************************
class ZTimeLine

ZTimeline -> ZTimeSpan -> ZTimeSlice

ZTimeLine can hold many ZTimeSpans which in turns can hold many ZTimeSlice
***************************************************************************************************************/


#ifndef ZENERD_TIMELINE
#define ZENERD_TIMELINE


class ZTimeLine
{
	private:
		vector<ZTimeSpan> m_ZTimeSpans;
		void swap(int a, int b);
		void qsort(int l = -100, int r = -100);

	public:
		ZTimeLine();
		ZTimeLine(const ZTimeLine & c);
		~ZTimeLine();
		void operator=(const ZTimeLine & c);

		// Regarding Time Spans
		int		GetSpanCount();
		bool	AddTimeSpan(int Owner, int Action);
		int		GetTimeSpan(int Owner, int Action);

		void	SetAction(int Span, int Action);
		void	SetOwner(int Span, int Owner);

		int		GetAction(int Span);
		int		GetOwner(int Span);
		
		// Regarging Slice Body Fxs;

		bool	AddNewFxToSlice(int Span, int Slice, int Start, int Length, double ActionID);
		void	GetFxToSlice(int Span,int Slice, int Fx, int &Start, int &Length, double &ActionID);
		void	EditFxToSlice(int Span,int Slice, int Fx, int Start, int Length, double ActionID);
		
		// Regarding Time Slices
		int		GetSliceCount(int Span);
		int		GetSliceFxCount(int Span, int Slice);
		bool	AddTimeSlice(int Span, int Time, int Length, int TimeFxInDuration, int TimeFxOutDuration, double Weight);
		int		GetTimeSlice(int Span, int Time);
		int		GetSlice(int Span, int Time); // why did I do this?, not sure


		// Unified Structure
		void	GetSlice(int Span, int Slice, int &Start, int &Length, int &TimeFxInDuration, int &TimeFxOutDuration, double &Weight);// this gets the data
		void	Edit(int Span, int Slice, int Start, int Length, int TimeFxInDuration, int TimeFxOutDuration, double Weight);
		void	Delete(int Span, int Slice = -1);
		
		// verification, note: no internal means for collision are used since they may or may not be needed
		int		isCollision(int Span, int Skip, int Start, int Length);// is there a collision between this data and other data

		
};

#endif


