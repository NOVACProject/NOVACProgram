#pragma once

#include <afxtempl.h>
#include <SpectralEvaluation/DateTime.h>


/** <b>CCombinerThread</b> is an abstract class designed to be inherited
        by CWinThread-object which have as purpose to take two evaluation-logs,
        in real-time, and combine them in some way to retrieve a result.

        Known children are:
            CGeometryEvaluator - calculates plume-heights in real-time from normal scans
            CWindEvaluator		- calculates wind-speeds in real-time from wind-measurements
        */

class CCombinerThread : public CWinThread
{

protected:
    /** Default constructor */
    CCombinerThread();

    /** Default destructor */
    virtual ~CCombinerThread();

public:
    /** Called when this thread is about to start */
    virtual BOOL InitInstance();

    /** Called when the message queue is empty */
    virtual BOOL OnIdle(LONG lCount);

    /** Called when the thread is to be stopped */
    afx_msg void OnQuit(WPARAM wp, LPARAM lp);

protected:
    DECLARE_MESSAGE_MAP()

    // ----------------------------------------------------------------------
    // ---------------------- PROTECTED DATA ----------------------------------
    // ----------------------------------------------------------------------

    /**  The maximum number of files that will be returned by 'FindMatchingEvalLog' */
    static const int MAX_MATCHING_FILES = 3;

    /** The maximum number of seconds that an evaluation-log file will stay in the list.
            This needs to be a rather large time, since we are not getting the results in
            real-time from the instruments but rather at some later time. */
#ifdef _DEBUG
    static const int MAX_RESIDENCE_TIME = 2 * 86400;
#else
    static const int MAX_RESIDENCE_TIME = 2 * 86400;
#endif

    /** a small class for storing the evaluation-logs */
    class CEvalLogFile {
    public:
        CEvalLogFile();
        ~CEvalLogFile();
        CString   fileName;     // <-- the filename of the evaluation log
        int       volcanoIndex; // <-- the volcano index tells us which volcano this measurement comes from
        novac::CDateTime arrived;      // <-- the (local PC) time when this thread found out about the file
        long      matched;      // <-- how many evaluation-files that this file has been matched with already
    };

    /** A list containing all the evaluation-log files that we know about.
        Stored with the name of the file and the time when they arrived.
        The list is sorted wrt the arrival time with the newest in the beginning of the list.
        If no matching eval-log file has arrived within a reasonable amount of
        time the eval-log will be removed from the list. */
    CList <CEvalLogFile, CEvalLogFile&>	m_evalLogs;

    /** Searches the list of evaluation logs and tries to find a log-file
            which matches the given evaluation-log.
            @return - the number of matching files found, this can be no more than MAX_MATCHING_FILES */
    virtual int	FindMatchingEvalLog(const CString& evalLog, CString match[MAX_MATCHING_FILES], int volcanoIndex) = 0;

    /** Inserts the given evaluation log into the correct position into the list. */
    virtual void InsertIntoList(const CString& evalLog, int volcanoIndex);

    /** Performs a cleaning of the list of evaluation-logs. Combined
            calculations can only be performed using two or more evaluation-logs
            together. The files are stored in the list, waiting for matching log-files
            to arrive. When long enough time has passed, the files are removed from
            the list. */
    virtual void	CleanEvalLogList();


};


