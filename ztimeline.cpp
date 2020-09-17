#include "ztimeline.h"


/************ IMPLEMENTATION OF ZSliceInternalFx *****************************************************************/

bool	ZSliceInternalFx::operator <  (const ZSliceInternalFx & c) { return compare(c)< 0; } 
bool	ZSliceInternalFx::operator >  (const ZSliceInternalFx & c) { return compare(c)> 0; } 
bool	ZSliceInternalFx::operator == (const ZSliceInternalFx & c) { return compare(c)==0; } 
bool	ZSliceInternalFx::operator <= (const ZSliceInternalFx & c) { return compare(c)<=0; } 
bool	ZSliceInternalFx::operator >= (const ZSliceInternalFx & c) { return compare(c)>=0; } 

void ZSliceInternalFx::operator = (const ZSliceInternalFx & c)
{
	m_nTime_start		= c.m_nTime_start;
	m_nTime_length		= c.m_nTime_length;
	m_nFxActionID		= c.m_nFxActionID;
	
}

ZSliceInternalFx::ZSliceInternalFx()
{

	m_nTime_start = 0;
	m_nTime_length = 0;
	m_nFxActionID = 0;


}
ZSliceInternalFx::ZSliceInternalFx(int start, int length, double actionID)
{

	m_nTime_start = start;
	m_nTime_length = length;
	m_nFxActionID = actionID;


}

ZSliceInternalFx::~ZSliceInternalFx()
{

	m_nTime_start = 0;
	m_nTime_length = 0;
	m_nFxActionID = 0;


}

char ZSliceInternalFx::compare(const ZSliceInternalFx &c)
{
	
	if(m_nTime_start < c.m_nTime_start ) return -1;
	if(m_nTime_start == c.m_nTime_start) return  0;
	return 1;


}



int	ZSliceInternalFx::GetSliceFxStart()
{


	return m_nTime_start;
	

}

int	ZSliceInternalFx::GetSliceFxLength()
{

		
	return m_nTime_length;	


	
}
double ZSliceInternalFx::GetSliceFxActionID()
{

	return m_nFxActionID;
		
}

void ZSliceInternalFx::GetInternalFx(int &Start, int &Length, double &ActionID)
{

	Start = m_nTime_start;
	Length = m_nTime_length;
	ActionID = m_nFxActionID;

}

void ZSliceInternalFx::EditSliceFx(int start, int length, double actionID)
{

	m_nTime_start = start;
	m_nTime_length = length;
	m_nFxActionID = actionID;

			
}

void ZSliceInternalFx::GetFxToSlice(int &start, int &length, double &actionID)
{

	start = m_nTime_start;
	length = m_nTime_length;
	actionID = m_nFxActionID;

}



bool ZSliceInternalFx::isIn(int time)
{
	return time >= m_nTime_start && time <= (m_nTime_start+m_nTime_length) ? true : false;
}

/************ IMPLEMENTATION OF ZTIMESLICE *****************************************************************/




bool	ZTimeSlice::operator <  (const ZTimeSlice & c) { return compare(c)< 0; } 
bool	ZTimeSlice::operator >  (const ZTimeSlice & c) { return compare(c)> 0; } 
bool	ZTimeSlice::operator == (const ZTimeSlice & c) { return compare(c)==0; } 
bool	ZTimeSlice::operator <= (const ZTimeSlice & c) { return compare(c)<=0; } 
bool	ZTimeSlice::operator >= (const ZTimeSlice & c) { return compare(c)>=0; } 
void	ZTimeSlice::SetStart(int start) { m_time_start = start; } 
void	ZTimeSlice::SetLength(int length) { m_time_length = length; }
void	ZTimeSlice::SetWeight(double weight) { m_action_weight = weight; }
void	ZTimeSlice::SetTimeFxInDuration(int length) {	m_timeFxInDuration = length;	}
void	ZTimeSlice::SetTimeFxOutDuration(int length) {	m_timeFxOutDuration = length;	}

int		ZTimeSlice::GetSliceFxCount() {return m_InternalFx.size(); }
int		ZTimeSlice::GetStart() { return m_time_start; }
int		ZTimeSlice::GetLength() { return m_time_length; }
double	ZTimeSlice::GetWeight() { return m_action_weight; }
int		ZTimeSlice::GetTimeFxInDuration() {return m_timeFxInDuration; }
int		ZTimeSlice::GetTimeFxOutDuration() {return m_timeFxOutDuration; }

void	ZTimeSlice::GetFxToSlice(int Fx, int &Start, int &Length, double &ActionID) 
{
	m_InternalFx[Fx].GetFxToSlice(Start, Length, ActionID);

}
void	ZTimeSlice::Edit(int start, int length, int TimeFxInDuration, int TimeFxOutDuration, double weight) { m_time_start = start; m_time_length = length; m_timeFxInDuration=TimeFxInDuration; m_timeFxOutDuration=TimeFxOutDuration; m_action_weight = weight; }


ZTimeSlice::ZTimeSlice()
{
	m_time_start	=	0;
	m_time_length	=	-1;
	m_timeFxInDuration = 0;
	m_timeFxOutDuration = 0;
	m_action_weight =   0.0;
}

// Copy Constructor, mirror of operator = 
//100%
ZTimeSlice::ZTimeSlice(const ZTimeSlice & c)
{
	*this = c;
}

ZTimeSlice::ZTimeSlice(int start, int length, int TimeFxInDuration, int TimeFxOutDuration, double weight)
{
	m_time_start		= start;
	m_time_length		= length;
	m_timeFxInDuration	= TimeFxInDuration;
	m_timeFxOutDuration	= TimeFxOutDuration;
	m_action_weight		= weight;
}

// destructor
ZTimeSlice::~ZTimeSlice()
{
	// ZERO THE MEMORY, for Aethistics (SP?)
	m_time_start		= 0;
	m_time_length		= 0;
	m_action_weight		= 0;
	m_timeFxInDuration  = 0;
	m_timeFxOutDuration  = 0;
}

void ZTimeSlice::operator = (const ZTimeSlice & c)
{
	m_time_start		= c.m_time_start;
	m_time_length		= c.m_time_length;
	m_action_weight		= c.m_action_weight;
	m_timeFxInDuration  = c.m_timeFxInDuration;
	m_timeFxOutDuration  = c.m_timeFxOutDuration;
}

char ZTimeSlice::compare(const ZTimeSlice & c)
{
	if(m_time_start < c.m_time_start ) return -1;
	if(m_time_start == c.m_time_start) return  0;
	return 1;
}

int ZTimeSlice::get_time_end()
{
	return m_time_start + m_time_length;
}

bool ZTimeSlice::isIn(int time)
{
	return time >= m_time_start && time <= get_time_end() ? true : false;
}



void ZTimeSlice::Delete(int ID)
{

	for(unsigned int i = ID; i < m_InternalFx.size() - 1 ; i++)
	{
		m_InternalFx[i] = m_InternalFx[i+1];
	}

	m_InternalFx.pop_back();


}


void ZTimeSlice::EditSliceFx(int FxSlice, int start, int length, double actionID)
{

	m_InternalFx[FxSlice].EditSliceFx(start, length, actionID);
	qsort();
}

void ZTimeSlice::swap(int a, int b)
{
	ZSliceInternalFx T;
	T = m_InternalFx[a];
	m_InternalFx[a] = m_InternalFx[b];
	m_InternalFx[b] = T;
}

void ZTimeSlice::qsort(int l, int r)
{
/*
	if(l==-100&&r==-100)
	{
		qsort(0, m_InternalFx.size()-1);
	}
	else
	{
		int i, j;
		if(r > l)
		{
			ZSliceInternalFx v;
			v = m_InternalFx[r];
			i = l - 1;
			j = r;
			
			for(;;)
			{
				while(m_InternalFx[++i] < v) ;
				while(m_InternalFx[--j] > v) ;
				if(i>=j) 
					break;
				swap(i,j);
			}
			swap(i,r);
			qsort(l,i-1);
			qsort(i+1, r);

		}
	}

*/

  std::sort(m_InternalFx.begin(), m_InternalFx.end());

}


bool ZTimeSlice::AddSliceFx(int start, int length, double actionID)
{
	
		

	//if start or length is outside of the slice body return false as an error
	if (start+GetStart() < GetStart() + GetTimeFxInDuration())
		return false;
	if (length > GetLength() - GetTimeFxOutDuration())
		return false;

	
/*
	//now try to find collisions with other Fxs in the body
	
	for(int i = 0; i < m_InternalFx.size() - 1; i++)
	{

			m_InternalFx.
			if(isIn(cStart,cStart + cLength,  Start)) return i;
			if(isIn(cStart,cStart + cLength,  Start + Length)) return i;
			if(isIn( Start, Start +  Length, cStart)) return i;
			if(isIn( Start, Start +  Length, cStart + cLength)) return i;

	}	
	*/
	ZSliceInternalFx add(start, length, actionID); 	
	m_InternalFx.push_back(add); 
	qsort();

	return true;
}

/************ IMPLEMENTATION OF ZTimeSpan *****************************************************************/

bool	ZTimeSpan::operator <  (const ZTimeSpan &c) { return compare(c)< 0; } 
bool	ZTimeSpan::operator >  (const ZTimeSpan &c) { return compare(c)> 0; } 
bool	ZTimeSpan::operator == (const ZTimeSpan &c) { return compare(c)==0; } 
bool	ZTimeSpan::operator <= (const ZTimeSpan &c) { return compare(c)<=0; } 
bool	ZTimeSpan::operator >= (const ZTimeSpan &c) { return compare(c)>=0; } 
void	ZTimeSpan::SetOwner(int owner) { m_owner = owner; }
void	ZTimeSpan::SetAction(int action) { m_action = action; }
int		ZTimeSpan::GetOwner() { return m_owner ; }
int		ZTimeSpan::GetAction() { return m_action ; }
void	ZTimeSpan::Edit(int slice, int start, int length, int TimeFxInDuration, int TimeFxOutDuration, double weight) { m_splices[slice].Edit(start, length, TimeFxInDuration, TimeFxOutDuration, weight); }
int		ZTimeSpan::GetSliceCount() { return m_splices.size(); }
int		ZTimeSpan::GetSliceFxCount(int slice) { return m_splices[slice].GetSliceFxCount(); }
void	ZTimeSpan::GetSlice(int Slice, int &Start, int &Length, int &TimeFxInDuration, int &TimeFxOutDuration, double &Weight) {	Start = m_splices[Slice].GetStart();Length = m_splices[Slice].GetLength();TimeFxInDuration=m_splices[Slice].GetTimeFxInDuration();TimeFxOutDuration=m_splices[Slice].GetTimeFxOutDuration();Weight = m_splices[Slice].GetWeight();}



ZTimeSpan::ZTimeSpan(int owner, int action)
{
	m_owner = owner;
	m_action = action;
}

ZTimeSpan::~ZTimeSpan()
{
	m_owner = 0;
	m_action = 0;
	m_splices.clear();
}

ZTimeSpan::ZTimeSpan(const ZTimeSpan & c)
{
	*this = c;
}

void ZTimeSpan::operator = (const ZTimeSpan &c)
{
	m_action = c.m_action;
	m_owner = c.m_owner;
	m_splices.clear();
	for(unsigned int i = 0 ; i < c.m_splices.size(); i++)
	{
		m_splices.push_back(c.m_splices[i]);
	}
}
// basic manager for the time span and time splices


bool ZTimeSpan::AddSplice(int start, int length, int TimeFxInDuration, int TimeFxOutDuration, double weight)
{
	ZTimeSlice Add(start,length,TimeFxInDuration,TimeFxOutDuration,weight);
	m_splices.push_back(Add);
	qsort();
	return true;
}

void ZTimeSpan::swap(int a, int b)
{
	ZTimeSlice T;
	T = m_splices[a];
		m_splices[a] =	m_splices[b];
						m_splices[b] = T;
}

void ZTimeSpan::qsort(int l, int r)
{
	if(l==-100&&r==-100)
	{
		qsort(0, m_splices.size()-1);
	}
	else
	{
		int i, j;
		if(r > l)
		{
			ZTimeSlice v;
			v = m_splices[r];
			i = l - 1;
			j = r;
			
			for(;;)
			{
				while(m_splices[++i] < v) ;
				while(m_splices[--j] > v) ;
				if(i>=j) break;
				swap(i,j);
			}
			swap(i,r);
			qsort(l,i-1);
			qsort(i+1, r);

		}
	}
}

char ZTimeSpan::compare(const ZTimeSpan &c)
{
	if(m_owner < c.m_owner) return -1;
	if(m_owner > c.m_owner) return 1;
	// since owner is equal
	if(m_action < c.m_action) return -1;
	if(m_action > c.m_action) return 1;
	// actions are equal
	return 0;
}


void ZTimeSpan::GetFxToSlice(int Slice, int Fx,  int &Start,  int &Length, double &ActionID)
{
	m_splices[Slice].GetFxToSlice(Fx, Start, Length, ActionID);
}

int ZTimeSpan::GetSliceAt(int time)
{
	for(unsigned int i = 0; i < m_splices.size() ; i++)
	{
		if(m_splices[i].isIn(time)==true) return i;
	}
	return -1;
}

void ZTimeSpan::Delete(int slice)
{
	for(unsigned int i = slice; i < m_splices.size() - 1 ; i++)
	{
		m_splices[i] = m_splices[i+1];
	}
	m_splices.pop_back();
}





bool ZTimeSpan::AddSliceFx(int slice, int start, int length, double actionID)
{

	
	if (m_splices[slice].AddSliceFx(start, length, actionID))
		return true;
	
	
	
	return false;
	


}

void ZTimeSpan::EditSliceFx(int slice, int FxSlice,int start, int length, double actionID)
{

	m_splices[slice].EditSliceFx(FxSlice, start, length, actionID);


}



/************ IMPLEMENTATION OF ZTimeLine *****************************************************************/

int		ZTimeLine::GetSpanCount() { return m_ZTimeSpans.size(); }
int		ZTimeLine::GetSliceCount(int Span) { return m_ZTimeSpans[Span].GetSliceCount(); }
int		ZTimeLine::GetSliceFxCount(int Span, int Slice) {return m_ZTimeSpans[Span].GetSliceFxCount(Slice);}
void	ZTimeLine::GetSlice(int Span, int Slice, int &Start, int &Length, int &TimeFxInDuration, int &TimeFxOutDuration, double &Weight) { m_ZTimeSpans[Span].GetSlice(Slice,Start,Length,TimeFxInDuration ,TimeFxOutDuration,Weight); }
void	ZTimeLine::SetAction(int Span, int Action) { m_ZTimeSpans[Span].SetAction(Action); }
int		ZTimeLine::GetAction(int Span) { return m_ZTimeSpans[Span].GetAction(); }
void	ZTimeLine::SetOwner(int Span, int Owner) { m_ZTimeSpans[Span].SetOwner(Owner); }
int		ZTimeLine::GetOwner(int Span) { return m_ZTimeSpans[Span].GetOwner(); }


ZTimeLine::ZTimeLine()
{
}

ZTimeLine::~ZTimeLine()
{
	m_ZTimeSpans.clear();
}

ZTimeLine::ZTimeLine(const ZTimeLine & c)
{
	*this = c;
}

void ZTimeLine::operator = (const ZTimeLine & c)
{
	for(unsigned int i=0;i<c.m_ZTimeSpans.size();i++)
	{
		m_ZTimeSpans.push_back(c.m_ZTimeSpans[i]);
	}
}
void ZTimeLine::swap(int a, int b)
{
	ZTimeSpan T;
		      T = m_ZTimeSpans[a];
			      m_ZTimeSpans[a] = m_ZTimeSpans[b];
								   m_ZTimeSpans[b] = T;
}
void ZTimeLine::qsort(int l, int r)
{
	if(l==-100&&r==-100)
	{
		qsort(0, m_ZTimeSpans.size()-1);
	}
	else
	{
		int i, j;
		if(r > l)
		{
			ZTimeSpan v;
			v = m_ZTimeSpans[r];
			i = l - 1;
			j = r;
			
			for(;;)
			{
				while(m_ZTimeSpans[++i] < v) ;
				while(m_ZTimeSpans[--j] > v) ;
				if(i>=j) break;
				swap(i,j);
			}
			swap(i,r);
			qsort(l,i-1);
			qsort(i+1, r);

		}
	}
}


bool ZTimeLine::AddTimeSpan(int Owner, int Action)
{
	ZTimeSpan Temp;
	Temp.SetOwner(Owner);
	Temp.SetAction(Action);
	m_ZTimeSpans.push_back(Temp);
	qsort();
	return true;
}

int ZTimeLine::GetTimeSpan(int Owner, int Action)
{
	int jump;
	jump = m_ZTimeSpans.size() - 1;
	int pos = 0;
	ZTimeSpan Test(Owner, Action);
	while(jump > 0)
	{
		// return conditions
		if(m_ZTimeSpans[pos] == Test)
			return pos;
		if(m_ZTimeSpans[pos+jump] == Test)
			return pos + jump;
		
		// jumping
		if(m_ZTimeSpans[pos+jump] > Test)
		{
			jump /=2;
		}

		if(m_ZTimeSpans[pos+jump] < Test)
		{
			pos+=jump;
		}

		// failure conditions
		if(pos==m_ZTimeSpans.size()) return -1;
		while((unsigned int) pos+jump >= m_ZTimeSpans.size()) 
			jump/=2;
	}
	if(m_ZTimeSpans[pos] == Test)
		return pos;
	return -1;
}

void ZTimeLine::GetFxToSlice(int Span,int Slice, int Fx, int &Start, int &Length, double &ActionID)
{

	m_ZTimeSpans[Span].GetFxToSlice(Slice, Fx,  Start,  Length, ActionID);
}

void ZTimeLine::EditFxToSlice(int Span,int Slice, int Fx, int Start, int Length, double ActionID)
{

	m_ZTimeSpans[Span].EditSliceFx(Slice, Fx,  Start,  Length, ActionID);
}

bool ZTimeLine::AddNewFxToSlice(int Span, int Slice, int Start, int Length, double ActionID)
{

	if (m_ZTimeSpans[Span].AddSliceFx(Slice, Start,Length,ActionID))
		return true;
	
	
	return false;

}

bool ZTimeLine::AddTimeSlice(int Span, int Time, int Length, int TimeFxInDuration, int TimeFxOutDuration, double Weight)
{
	return m_ZTimeSpans[Span].AddSplice(Time, Length, TimeFxInDuration, TimeFxOutDuration,  Weight);
}

int ZTimeLine::GetTimeSlice(int Span, int Time)
{
	return m_ZTimeSpans[Span].GetSliceAt(Time);
}
void ZTimeLine::Edit(int Span, int Slice, int Start, int Length, int TimeFxInDuration, int TimeFxOutDuration, double Weight)
{
	m_ZTimeSpans[Span].Edit(Slice,Start,Length, TimeFxInDuration, TimeFxOutDuration,Weight);
}
void ZTimeLine::Delete(int Span, int Slice)
{
	if(Slice>=0)
	{
		// delete a slice of a span
		m_ZTimeSpans[Span].Delete(Slice);
	}
	else
	{
		// delete a span
		for(unsigned int i = Span; i < m_ZTimeSpans.size() - 1 ; i++)
		{
			m_ZTimeSpans[i] = m_ZTimeSpans[i+1];
		}
		m_ZTimeSpans.pop_back();
	}
}



bool isIn(int min, int max, int test)
{
	if(min<=test && test<=max) return true;
	return false;
}

int	ZTimeLine::isCollision(int Span, int Skip, int Start, int Length)
{
	int cStart, cLength, cTimeFxInDuration, cTimeFxOutDuration;
	double w;
	for(int i = 0; i < m_ZTimeSpans[Span].GetSliceCount(); i++)
	{
		if(i!=Skip)
		{
			m_ZTimeSpans[Span].GetSlice(i,cStart, cLength, cTimeFxInDuration, cTimeFxOutDuration, w);
			if(isIn(cStart,cStart + cLength,  Start)) return i;
			if(isIn(cStart,cStart + cLength,  Start + Length)) return i;
			if(isIn( Start, Start +  Length, cStart)) return i;
			if(isIn( Start, Start +  Length, cStart + cLength)) return i;
		}
	}
	return -1;
}

int ZTimeLine::GetSlice(int Span, int Time)
{
	return m_ZTimeSpans[Span].GetSliceAt(Time);
}