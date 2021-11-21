#include "stdafx.h"
#include "NovacMasterProgram.h"
#include "CombinerThread.h"

#include <math.h>

using namespace novac;

// --------------------------------------------------------
// ----------------- CEvalLogFile -------------------------
// --------------------------------------------------------
CCombinerThread::CEvalLogFile::CEvalLogFile() {
}

CCombinerThread::CEvalLogFile::~CEvalLogFile() {
}

// --------------------------------------------------------
// --------------- CCombinerThread ------------------------
// --------------------------------------------------------

CCombinerThread::CCombinerThread()
{
}

CCombinerThread::~CCombinerThread()
{
    m_evalLogs.RemoveAll();
}

BOOL CCombinerThread::InitInstance()
{
    // TODO:  perform and per-thread initialization here
    return TRUE;
}

void CCombinerThread::OnQuit(WPARAM wp, LPARAM lp) {
    m_evalLogs.RemoveAll();
}

/** Called when the message queue is empty */
BOOL CCombinerThread::OnIdle(LONG lCount) {
    return CWinThread::OnIdle(lCount);
}

BEGIN_MESSAGE_MAP(CCombinerThread, CWinThread)
END_MESSAGE_MAP()


// CCombinerThread message handlers
/** Inserts the given evaluation log into the correct position into the list. */
void CCombinerThread::InsertIntoList(const CString& evalLog, int volcanoIndex) {
    CEvalLogFile newLog;

    // 1. Create a novel CEvalLogFile object to insert into the list
    newLog.arrived.SetToNow();
    newLog.fileName.Format(evalLog);
    newLog.volcanoIndex = volcanoIndex;
    newLog.matched = 0;

    // 2. Insert the scan-result
    m_evalLogs.AddHead(newLog);
}

/** Performs a cleaning of the list of evaluation-logs. Wind-speed
        calculations can only be performed using two or more evaluation-logs
        together. The files are stored in the list, waiting for matching log-files
        to arrive. When long enough time has passed, the files are removed from
        the list. */
void CCombinerThread::CleanEvalLogList() {
    CDateTime now;

    // 1. Get the time now;
    now.SetToNow();

    // 2. Go through the list and remove all items which are too old
    POSITION pos = m_evalLogs.GetTailPosition();
    while (pos != NULL) {
        // 2a. get a reference to the arrival-time
        CDateTime& arrived = m_evalLogs.GetAt(pos).arrived;

        // 2b. Calculate the amount of time that has elapsed since the file arrived
        double secondsPassed = fabs(CDateTime::Difference(arrived, now));

        // 2c. If the time is longer than the maximum allowed time, remove
        //			the rest of the list
        if (secondsPassed > MAX_RESIDENCE_TIME) {
            m_evalLogs.GetPrev(pos);
            m_evalLogs.RemoveTail();
        }
        else {
            // if this item has been less than 'MAX_RESIDENCE_TIME' 
            //	in the list, then the previous items in the list
            //	have been there shorter time, no need to even look at them...
            return;
        }
    }
}
