#pragma once

#include <afxtempl.h>

namespace Communication
{
class CLinkStatistics
{
public:
    /** Default constructor */
    CLinkStatistics(void);

    /** Default destructor */
    ~CLinkStatistics(void);

    /** Clearing the statistics */
    void Clear();

    // ----------------------- Adding data -------------------------
    /** Append one download-speed to the history,
            this will also append one successfull download to the statistics */
    void AppendDownloadSpeed(double speed);

    /** Append one failed download to the history */
    void AppendFailedDownload();

    /** Append one upload-speed to the history
            this will also append one successfull upload to the statistics */
    void AppendUploadSpeed(double speed);

    /** Append one failed upload to the history */
    void AppendFailedUpload();

protected:
    class CTransferInfo
    {
    public:
        CTransferInfo();
        ~CTransferInfo();
        bool success; // true if the transfer is successfull
        double speed;  // the speed of the transfer
        char hour;   // the hour when the transfer was made (0->23)
        char minute;  // the minute when the transfer was made (0->59)
        char second;  // the second when the transfer was made (0->59)
        char day;   // the day of the month when the transfer was made (0->31)
    };

    /** The information about the downloads */
    CList<CTransferInfo, CTransferInfo&> m_downloads;

    /** The information about the uploads */
    CList<CTransferInfo, CTransferInfo&> m_uploads;

        // ------------- Protected methods ---------------

    /** Trim the list of transfer-data so that they only contain data from
            today... */
    void TrimLists();

    /** Appends the given data-speed to the given list */
    void AppendSuccessfulTransfer(double speed, CList<CTransferInfo, CTransferInfo&>& list);

    /** Appends a failure to the given list */
    void AppendFailedTransfer(CList<CTransferInfo, CTransferInfo&>& list);
};
}
