#pragma once

#define MAX_OBSERVATORIES 35

class CObservatoryInfo
{
public:
	CObservatoryInfo(void);
	~CObservatoryInfo(void);

	/** The number of observatories */
	unsigned int	m_observatoryNum;

	/** The name of the observatories */
	CString				m_name[MAX_OBSERVATORIES];

};
