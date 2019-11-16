#pragma once

/** The <b>CVolcanoInfo</b>-class the programs database of known volcanoes with 
    name, position and associated observatory.
    This information is used throughout the program in order to retrieve information
    about the volcanoes.*/

#define MAX_VOLCANOES 35

class CVolcanoInfo
{
public:
    CVolcanoInfo();

    /** The number of volcanoes that are configured */
    unsigned int m_volcanoNum;

    /** The name of the volcano(es) */
    CString m_name[MAX_VOLCANOES];

    /** The simplified name of the volcano(es) */
    CString m_simpleName[MAX_VOLCANOES];

    /** The latitude of the peak(s) */
    double m_peakLatitude[MAX_VOLCANOES];

    /** The longitude of the peak(s) */
    double m_peakLongitude[MAX_VOLCANOES];

    /** The altitude of the peak(s) (masl) */
    double m_peakHeight[MAX_VOLCANOES];

    /** The number of hours to GMT, used to calculate the local-time from the GPS-time */
    double m_hoursToGMT[MAX_VOLCANOES];

    /** The observatory in charge of this volcano */
    int m_observatory[MAX_VOLCANOES];

    /** The number of volcanoes that are configured by the program
        (if m_preConfiguredVolcanoNum > m_volcanoNum then the user has added a volcano) */
    unsigned int m_preConfiguredVolcanoNum;

};
