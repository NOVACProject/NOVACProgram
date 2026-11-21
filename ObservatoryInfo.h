#pragma once

#define MAX_OBSERVATORIES 35

class CObservatoryInfo
{
public:
    CObservatoryInfo();

    /** The name of the observatories */
    CString m_name[MAX_OBSERVATORIES];
};
