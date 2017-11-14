#pragma once

/** CVersion is a simple class to remember the version number of the program */
class CVersion
{
public:
	CVersion(void);
	~CVersion(void);


	/** The major version number */
	static const int majorNumber = 1;

	/** The minor version number */
	static const int minorNumber = 82;
};
