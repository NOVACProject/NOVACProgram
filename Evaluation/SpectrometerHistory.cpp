#include "StdAfx.h"
#include "spectrometerhistory.h"

using namespace Evaluation;
using namespace novac;

CSpectrometerHistory::CScanInfo::CScanInfo() {
    plumeCentre[0] = 0;
    plumeCentre[1] = 0;
    plumeEdge[0] = 0;
    plumeEdge[1] = 0;
    plumeCompleteness = 0;
    exposureTime = 0;
    maxColumn = 0;
}
CSpectrometerHistory::CScanInfo::~CScanInfo() {
}

CSpectrometerHistory::CWMInfo::CWMInfo() {
}
CSpectrometerHistory::CWMInfo::~CWMInfo() {
}

CSpectrometerHistory::CSpectrometerHistory(void)
{
}

CSpectrometerHistory::~CSpectrometerHistory(void)
{
    //if(m_scanInfo.GetCount() > 0)
    //	m_scanInfo.RemoveAll();
    //if(m_windMeasurementTimes.GetCount() > 0)
    //	m_windMeasurementTimes.RemoveAll();
}

/** Appends a new set of results. */
void CSpectrometerHistory::AppendScanResult(const CScanResult& result, const std::string& specie) {
    CDateTime startTime;

    // 1. Create the information about when the scan-information arrived
    CDateTime now;
    now.SetToNow();

    // 1b. Get the start-time of the measurement
    if (SUCCESS != result.GetStartTime(0, startTime))
        return; // we couldn't read the start-time of the sky-spectrum. Something's very wrong...

    // 2. Create a new CScanInfo-object to insert into the list
    CScanInfo newInfo;
    newInfo.exposureTime = result.GetExposureTime(0);
    newInfo.plumeCentre[0] = max(-180, result.GetCalculatedPlumeCentre(0));
    newInfo.plumeCentre[1] = max(-180, result.GetCalculatedPlumeCentre(1));
    newInfo.plumeCompleteness = max(-180, result.GetCalculatedPlumeCompleteness());
    newInfo.maxColumn = result.GetMaxColumn(specie);
    newInfo.startTime = startTime;
    newInfo.arrived = now;
    result.GetCalculatedPlumeEdges(newInfo.plumeEdge[0], newInfo.plumeEdge[1]);

    // 2a. Get the range of scan-angles in the scan (typically from -90 to +90)
    newInfo.alpha[0] = +999.0;
    newInfo.alpha[1] = -999.0;
    for (unsigned long k = 0; k < result.GetEvaluatedNum(); ++k) {
        newInfo.alpha[0] = min(newInfo.alpha[0], result.GetScanAngle(k));
        newInfo.alpha[1] = max(newInfo.alpha[1], result.GetScanAngle(k));
    }

    // 3. Find the position where to insert the scan-result
    POSITION pos = m_scanInfo.GetHeadPosition();
    while (pos != nullptr) {
        CScanInfo info = m_scanInfo.GetAt(pos);
        if (info.startTime < startTime)
            break;
        m_scanInfo.GetNext(pos);
    }

    // 4. Insert the scan-result
    if (m_scanInfo.GetCount() == 0)
        m_scanInfo.AddHead(newInfo);
    else if (pos == nullptr)
        m_scanInfo.AddTail(newInfo);
    else
        m_scanInfo.InsertBefore(pos, newInfo);

    // 5. If there are too many results in the list, remove the oldest one
    if (m_scanInfo.GetCount() > MAX_SPECTROMETER_HISTORY) {
        m_scanInfo.RemoveTail();
    }
}

/** Appends the information that a wind-speed measurement has been performed
        at a specific time. */
void CSpectrometerHistory::AppendWindMeasurement(int year, int month, int day, int hour, int minute, int second) {

    // 1. Create a new CDateTime-object
    CDateTime nyTid = CDateTime(year, month, day, hour, minute, second);

    // 2. Create the information about when the information arrived
    CDateTime now;
    now.SetToNow();

    // 3. Create a new CWMInfo object to insert
    CWMInfo	newInfo;
    newInfo.arrived = now;
    newInfo.startTime = nyTid;

    // 3. Find the position where to insert the time
    POSITION pos = m_windMeasurementTimes.GetHeadPosition();
    while (pos != nullptr) {
        CWMInfo wmInfo = m_windMeasurementTimes.GetAt(pos);
        if (wmInfo.startTime < nyTid)	break;
        m_windMeasurementTimes.GetNext(pos);
    }

    // 3. Insert the scan-result
    if (m_windMeasurementTimes.GetCount() == 0)
        m_windMeasurementTimes.AddHead(newInfo);
    else if (pos == nullptr)
        m_windMeasurementTimes.AddTail(newInfo);
    else
        m_windMeasurementTimes.InsertBefore(pos, newInfo);

    // 4. If there are too many results in the list, remove the oldest one
    if (m_windMeasurementTimes.GetCount() > MAX_SPECTROMETER_HISTORY) {
        m_windMeasurementTimes.RemoveTail();
    }
}

/** Appends the information that a composition measurement has been performed
        at a specific time. */
void CSpectrometerHistory::AppendCompMeasurement(int year, int month, int day, int hour, int minute, int second) {

    // 1. Create a new CDateTime-object
    CDateTime nyTid = CDateTime(year, month, day, hour, minute, second);

    // 2. Create the information about when the information arrived
    CDateTime now;
    now.SetToNow();

    // 3. Create a new CWMInfo object to insert
    CWMInfo	newInfo;
    newInfo.arrived = now;
    newInfo.startTime = nyTid;

    // 3. Find the position where to insert the time
    POSITION pos = m_compMeasurementTimes.GetHeadPosition();
    while (pos != nullptr) {
        CWMInfo wmInfo = m_compMeasurementTimes.GetAt(pos);
        if (wmInfo.startTime < nyTid)	break;
        m_compMeasurementTimes.GetNext(pos);
    }

    // 3. Insert the scan-result
    if (m_compMeasurementTimes.GetCount() == 0)
        m_compMeasurementTimes.AddHead(newInfo);
    else if (pos == nullptr)
        m_compMeasurementTimes.AddTail(newInfo);
    else
        m_compMeasurementTimes.InsertBefore(pos, newInfo);

    // 4. If there are too many results in the list, remove the oldest one
    if (m_compMeasurementTimes.GetCount() > MAX_SPECTROMETER_HISTORY) {
        m_compMeasurementTimes.RemoveTail();
    }
}

/** Returns the number of seconds passed since the last scan
        arrived. Return -1 if no scans has arrived */
int	CSpectrometerHistory::SecondsSinceLastScan() {

    // 1. If no scan has arrived, return -1
    if (m_scanInfo.GetCount() == 0)
        return -1;

    // 2. The arrival-time of the last scan
    CScanInfo	lastInfo = m_scanInfo.GetAt(m_scanInfo.GetHeadPosition());

    // 3. The local time now
    CDateTime	now;
    now.SetToNow();

    // 4. Calculate the difference...
    double secondsPassed = CDateTime::Difference(now, lastInfo.arrived);

    // 5. Return
    return (int)fabs(secondsPassed);
}

/** Returns the time of the last scan performed (in GMT) */
int CSpectrometerHistory::GetStartTimeOfLastScan(CDateTime& dt) {
    // 1. If no scan has arrived, return -1
    if (m_scanInfo.GetCount() == 0)
        return -1;

    // 2. The arrival-time of the last scan
    CScanInfo	lastInfo = m_scanInfo.GetAt(m_scanInfo.GetHeadPosition());

    // 3. Set the time
    dt = lastInfo.startTime;

    return 0;
}

/** Returns the number of seconds passed since the last wind-measurement
        arrived. Return -1 if no wind-measurement has arrived */
int	CSpectrometerHistory::SecondsSinceLastWindMeas() {

    // 1. If no wind-measurement has arrived, return -1
    if (m_windMeasurementTimes.GetCount() == 0)
        return -1;

    // 2. The arrival-time of the last wind-measurement (in the time of the observatory PC we're running on)
    CWMInfo	lastInfo = m_windMeasurementTimes.GetAt(m_windMeasurementTimes.GetHeadPosition());

    // 3. The local time now
    CDateTime now;
    now.SetToNow();
    // now.Increment(_timezone); // convert from local-time to GMT. CHANGED 2019-02-19 as this is incorrect unless specifically set.

    // 4. Calculate the difference...
    double secondsPassed = CDateTime::Difference(now, lastInfo.arrived); // CHANGED 2019-02-19 from being the avg.time of the scan to instead the time the scan was downloaded. To not having to bother about time-zone settings.

    // 5. Return
    return (int)fabs(secondsPassed);
}

/** Returns the number of seconds passed since the last composition measurement
        arrived. Return -1 if no composition measurement has arrived */
int	CSpectrometerHistory::SecondsSinceLastCompMeas() {

    // 1. If no composition-measurement has arrived, return -1
    if (m_compMeasurementTimes.GetCount() == 0)
        return -1;

    // 2. The arrival-time of the last composition-measurement (in the time of the observatory PC we're running on)
    CWMInfo	lastInfo = m_compMeasurementTimes.GetAt(m_compMeasurementTimes.GetHeadPosition());

    // 3. The local time now
    CDateTime now;
    now.SetToNow();

    // 4. Calculate the difference...
    double secondsPassed = CDateTime::Difference(now, lastInfo.arrived);

    // 5. Return
    return (int)fabs(secondsPassed);
}

/** Returns the number of scans that have arrived from this spectrometer
        today. */
int	CSpectrometerHistory::GetNumScans() {
    // 1. If no scan has arrived...
    if (m_scanInfo.GetCount() == 0)
        return 0;

    // 2. Get todays date
    CDateTime now;
    now.SetToNow();

    // 3. Count the number of scans with todays date
    int	nScans = 0;
    POSITION pos = m_scanInfo.GetHeadPosition();
    while (pos != 0) {
        CScanInfo info = m_scanInfo.GetNext(pos);
        CDateTime arrived = info.arrived;
        CDateTime startTime = info.startTime;
        // TODO!! The startTime-date should be converted to local-time!!!
        if (arrived.year == now.year && arrived.month == now.month && arrived.day == now.day) {
            if (startTime.year == now.year && startTime.month == now.month && fabs(startTime.day - now.day) <= 1) {
                ++nScans;
            }
        }
    }

    return nScans;
}

/** Returns the average number of seconds passed between the starting of the
        last 5-10 scans. Return -1 if no scans has arrived */
double	CSpectrometerHistory::GetScanInterval() {

    // 1. If no scan has arrived, return -1
    if (m_scanInfo.GetCount() == 0)
        return -1;

    // 2. If there are no scans arrived today. return -1
    if (GetNumScans() == 0)
        return -1;

    // 3. Get todays date
    CDateTime now;
    now.SetToNow();

    // 4. Go through the list of scans and see the start-times
    CDateTime lastStartTime;
    double avgTimeDiff = 0;
    int		nScans = 0;
    POSITION pos = m_scanInfo.GetHeadPosition();

    while (pos != nullptr) {
        CScanInfo info = m_scanInfo.GetNext(pos);
        CDateTime arrived = info.arrived;
        CDateTime startTime = info.startTime;

        // TODO!! The startTime-date should be converted to local-time!!!
        if (arrived.year == now.year && arrived.month == now.month && arrived.day == now.day) {
            if (startTime.year == now.year && startTime.month == now.month && fabs(startTime.day - now.day) <= 1) {
                // if this is the first scan, we cannot calculate any time difference
                if (lastStartTime.year == 0) {
                    lastStartTime = startTime;
                    continue;
                }

                // calculate the difference in time
                double timeDifference = fabs(CDateTime::Difference(startTime, lastStartTime));
                avgTimeDiff += timeDifference;
                lastStartTime = startTime;
                ++nScans;
            }
        }
    }//end while

    // 5s. Calculate and return the average time difference
    avgTimeDiff /= (double)nScans;
    return avgTimeDiff;
}

/** Returns the average plume-centre during the
        last 'scansToAverage' scans today */
double CSpectrometerHistory::GetPlumeCentre(int scansToAverage, int motor) {
    int nScans = 0;
    double plumeCentre = 0.0;

    ASSERT(motor <= 1);

    // 1. Get todays date
    CDateTime now;
    now.SetToNow();

    // 2. Go through the list and average over the last 'scanstoAverage'-last
    //		scans
    POSITION pos = m_scanInfo.GetHeadPosition();
    while (pos != 0) {
        CScanInfo info = m_scanInfo.GetNext(pos);
        CDateTime	tid = info.startTime;
        if (tid.year == now.year && tid.month == now.month && fabs(tid.day - now.day) <= 1) {

            // If any measurement shows that there's no plume, return -180
            if (info.plumeCentre[motor] < -100)
                return -180.0;

            // Average
            plumeCentre += info.plumeCentre[motor];
            nScans += 1;

            if (nScans == scansToAverage)
                pos = nullptr; // <-- break the while loop
        }
    }

    // 3. Check if we have enough scans today
    if (nScans < scansToAverage)
        return -180.0;

    // 4. Calculate and return the average
    plumeCentre /= (double)nScans;

    return plumeCentre;
}

/** Returns the average positions of the edges of the plume during the
        last 'scansToAverage' scans today. Returns false if there are
        fewer than 'scansToAverage' scans collected today OR
        if any of the last 'scansToAverage' misses the plume. */
bool	CSpectrometerHistory::GetPlumeEdges(int scansToAverage, double& lowEdge, double& highEdge) {
    int		nScans = 0;
    lowEdge = 0.0;
    highEdge = 0.0;

    // 1. Get todays date
    CDateTime now;
    now.SetToNow();

    // 2. Go through the list and average over the last 'scanstoAverage'-last
    //		scans
    POSITION pos = m_scanInfo.GetHeadPosition();
    while (pos != 0) {
        CScanInfo info = m_scanInfo.GetNext(pos);
        CDateTime	tid = info.startTime;
        if (tid.year == now.year && tid.month == now.month && fabs(tid.day - now.day) <= 1) {
            // If any measurement shows that there's no plume, return false
            if (info.plumeCentre[0] < -100)
                return false;

            // Avearage
            lowEdge += info.plumeEdge[0];
            highEdge += info.plumeEdge[1];
            nScans += 1;

            if (nScans == scansToAverage)
                pos = nullptr; // <-- break the while loop
        }
    }

    // 3. Check if we have enough scans today
    if (nScans < scansToAverage)
        return false;

    // 4. Calculate and return the average
    lowEdge /= (double)nScans;
    highEdge /= (double)nScans;

    return true;
}

/** Returns the average plume completeness during the
        last 'scansToAverage' scans today */
double CSpectrometerHistory::GetPlumeCompleteness(int scansToAverage) {
    int nScans = 0;
    double plumeCompleteness = 0.0;

    // 1. Get todays date
    CDateTime now;
    now.SetToNow();

    // 2. Go through the list and average over the last 'scanstoAverage'-last
    //		scans
    POSITION pos = m_scanInfo.GetHeadPosition();
    while (pos != 0) {
        CScanInfo info = m_scanInfo.GetNext(pos);
        CDateTime	tid = info.startTime;
        if (tid.year == now.year && tid.month == now.month && fabs(tid.day - now.day) <= 1) {
            // If any measurement shows that there's no plume, return -180
            if (info.plumeCompleteness < -100)
                return -1.0;

            // Avearage
            plumeCompleteness += info.plumeCompleteness;
            nScans += 1;

            if (nScans == scansToAverage)
                pos = nullptr; // <-- break the while loop
        }
    }

    // 3. Check if we have enough scans today
    if (nScans < scansToAverage)
        return -180.0;

    // 4. Calculate and return the average
    plumeCompleteness /= (double)nScans;

    return plumeCompleteness;
}

/** Returns the maximum and minimum values of the plume-centre
        over the last 'scansToAverage' scans today. Returns false if
        fewer than 'scansToAverage' scans are collected today OR
        if any of the last 'scansToAverage' misses the plume. */
bool CSpectrometerHistory::GetPlumeCentreVariation(int scansToAverage, int motor, double& centreMin, double& centreMax) {
    int nScans = 0;
    centreMin = 180.0;
    centreMax = -180.0;

    ASSERT(motor <= 1);

    // 1. Get todays date
    CDateTime now;
    now.SetToNow();

    // 2. Go through the list and check the variations
    POSITION pos = m_scanInfo.GetHeadPosition();
    while (pos != 0) {
        CScanInfo info = m_scanInfo.GetNext(pos);
        CDateTime	tid = info.startTime;
        if (tid.year == now.year && tid.month == now.month && fabs(tid.day - now.day) <= 1) {
            // If any measurement shows that there's no plume, return false
            if (info.plumeCentre[motor] < -100)
                return false;

            // get the ranges
            centreMin = min(centreMin, info.plumeCentre[motor]);
            centreMax = max(centreMax, info.plumeCentre[motor]);
            nScans += 1;

            if (nScans == scansToAverage)
                pos = nullptr; // <-- break the while loop
        }
    }

    // 3. Check if we have enough scans today
    if (nScans < scansToAverage)
        return false;

    // 4. We've got all the data we need, return.
    return true;
}

/** Returns the average exposure-time during the last
        'scansToAverage' scans today. returns -1 if there are fewer
        than 'scansToAverage' scans collected today */
double CSpectrometerHistory::GetExposureTime(int scansToAverage) {
    int nScans = 0;
    double expTime = 0.0;

    // 1. Get todays date
    CDateTime now;
    now.SetToNow();

    // 2. Go through the list and average over the last 'scanstoAverage'-last
    //		scans
    POSITION pos = m_scanInfo.GetHeadPosition();
    while (pos != 0) {
        CScanInfo info = m_scanInfo.GetNext(pos);
        CDateTime	tid = info.startTime;
        if (tid.year == now.year && tid.month == now.month && fabs(tid.day - now.day) <= 1) {
            // Avearage
            expTime += info.exposureTime;
            nScans += 1;

            if (nScans == scansToAverage)
                pos = nullptr; // <-- break the while loop
        }
    }

    // 3. Check if we have enough scans today
    if (nScans < scansToAverage)
        return -1.0;

    // 4. Calculate and return the average
    expTime /= (double)nScans;

    return expTime;
}

/** Returns the peak column averaged over the last
        'scansToAverage' scans today. returns -1 if there are fewer
        than 'scansToAverage' scans collected today */
double	CSpectrometerHistory::GetColumnMax(int scansToAverage) {
    int nScans = 0;
    double maxColumn = 0.0;

    // 1. Get todays date
    CDateTime now;
    now.SetToNow();

    // 2. Go through the list and average over the last 'scanstoAverage'-last
    //		scans
    POSITION pos = m_scanInfo.GetHeadPosition();
    while (pos != 0) {
        CScanInfo info = m_scanInfo.GetNext(pos);
        CDateTime	tid = info.startTime;
        if (tid.year == now.year && tid.month == now.month && fabs(tid.day - now.day) <= 1) {
            // Average
            maxColumn += info.maxColumn;
            nScans += 1;

            if (nScans == scansToAverage)
                pos = nullptr; // <-- break the while loop
        }
    }

    // 3. Check if we have enough scans today
    if (nScans < scansToAverage)
        return -1.0;

    // 4. Calculate and return the average
    maxColumn /= (double)nScans;

    return maxColumn;
}

/** Returns the range of zenith-angles for the last scan
        (Normally these are from -90 to +90)
        If no scan collected today has been received then alpha_min will be +999.0
        and alpha_max will be -999.0				*/
void	CSpectrometerHistory::GetAlphaRange(double& alpha_min, double& alpha_max) {
    alpha_min = +999.0;
    alpha_max = -999.0;

    POSITION pos = m_scanInfo.GetHeadPosition();
    if (pos == nullptr) {
        return; // no scan today
    }

    CScanInfo info = m_scanInfo.GetNext(pos);

    alpha_min = info.alpha[0];
    alpha_max = info.alpha[1];

    return;
}
