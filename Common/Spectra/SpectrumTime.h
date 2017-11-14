#pragma once


class CSpectrumTime
{
public:
	CSpectrumTime(void);
	~CSpectrumTime(void);

	unsigned short hr;
	unsigned short m;
	unsigned short sec;
	unsigned short msec;

	/** Assignment operator */
	CSpectrumTime &operator=(const CSpectrumTime &t2){
		this->hr = t2.hr;
		this->m = t2.m;
		this->sec = t2.sec;
		this->msec = t2.msec;
		return *this;
	}

	/** Comparison of two times. 
		@return 1 if this is smaller than t2. 
		@return 0 if this is larger than or equal to t2. */
	int operator<(const CSpectrumTime &t2){
		if(this->hr < t2.hr)
		return 1;
		if(this->hr > t2.hr)
		return 0;
		if(this->m < t2.m)
		return 1;
		if(this->m > t2.m)
		return 0;
		if(this->sec < t2.sec)
		return 1;
		if(this->sec > t2.sec)
		return 0;
		if(this->msec < t2.msec)
		return 1;
		if(this->msec > t2.msec)
		return 0;

		return 0;
	}
};
