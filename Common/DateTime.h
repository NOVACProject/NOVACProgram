#pragma once

#ifndef DATETIME_H
#define DATETIME_H

class CDateTime
{
public:
	CDateTime(void);
	CDateTime(const CDateTime &t2);
	CDateTime(int y, int mo, int d, int h, int mi, int s);
	~CDateTime(void);

	// ---------------- DATA ------------------
	/** The year (0 - 16384) */
	unsigned short year;

	/** The month (1-12) */
	unsigned char month;

	/** The day (1-31) */
	unsigned char day;

	/** The hour (0-23) */
	unsigned char hour;

	/** The minute (0-59) */
	unsigned char minute;

	/** The second (0-59) */
	unsigned char second;

	// --------------- OPERATORS ------------------
	int		operator<(const CDateTime& t2) const;
	bool	operator==(const CDateTime& t2) const;
	CDateTime	&operator=(const CDateTime& t2);

	// --------------- METHODS --------------------

	/** Sets the time of this CDateTime-object to the current (local PC) time */
	void SetToNow();

	/** Increments the current time with the supplied number of seconds */
	void Increment(int seconds);

	/** Calculates the difference, in seconds, between two times.
			If t2 is later than t1, then the result will be negative. */
	static double	Difference(const CDateTime &t1, const CDateTime &t2);

private:

};


#endif