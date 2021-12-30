#include "StdAfx.h"
#include "observatoryinfo.h"

// The global database of observatories
CObservatoryInfo g_observatories;

CObservatoryInfo::CObservatoryInfo()
{
    // Setting up the information about the observatories
    int index = 0;

    m_name[index].Format("bira_iasb"); // 0

    ++index;
    m_name[index].Format("chalmers"); // 1

    ++index;
    m_name[index].Format("ifm_geomar"); // 2

    ++index;
    m_name[index].Format("igepn"); // 3

    ++index;
    m_name[index].Format("ineter"); // 4

    ++index;
    m_name[index].Format("ingeominas"); // 5

    ++index;
    m_name[index].Format("ingv_ct"); // 6

    ++index;
    m_name[index].Format("ingv_pa"); // 7

    ++index;
    m_name[index].Format("insivumeh"); // 8

    ++index;
    m_name[index].Format("ipgp"); // 9

    ++index;
    m_name[index].Format("mit"); // 10

    ++index;
    m_name[index].Format("ovg"); // 11

    ++index;
    m_name[index].Format("ovsicori"); // 12

    ++index;
    m_name[index].Format("snet"); // 13

    ++index;
    m_name[index].Format("ucam"); // 14

    ++index;
    m_name[index].Format("uhei"); // 15

    ++index;
    m_name[index].Format("umbc"); // 16

    ++index;
    m_name[index].Format("unam"); // 17

    m_observatoryNum = index + 1;
}
