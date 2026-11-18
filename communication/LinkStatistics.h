#pragma once

#include <afxtempl.h>

namespace Communication{
	class CLinkStatistics
	{
	public:
		/** Default constructor */
		CLinkStatistics(void);

		/** Default destructor */
		~CLinkStatistics(void);

		/** Clearing the statistics */
		void Clear();

		// -------------------- Retrieving data -----------------------
		/** Getting the successrate for the number of downloads.
				@return the portion of number of attempts to download files
					that have succeeded (0 -> 1) */
		double GetDownloadSuccessRate() const;

		/** Getting the average download speed for this link [kb/s] */
		double GetAveragedDownloadSpeed() const;

		/** Returns the number of successful downloads today on this link */
		long	GetDownloadNum() const;

		/** Getting the successrate for the number of uploads
				@return the portion of number of attempts to upload files
					that have succeeded (0 -> 1) */
		double GetUploadSuccessRate() const;

		/** Getting the average upload speed for this link [kb/s] */
		double GetAveragedUploadSpeed() const;

		/** Returns the number of successful uploads today on this link */
		long	GetUploadNum() const;

		// ----------------------- Adding data -------------------------
		/** Append one download-speed to the history,
				this will also append one successfull download to the statistics */
		void	AppendDownloadSpeed(double speed);

		/** Append one failed download to the history */
		void	AppendFailedDownload();

		/** Append one upload-speed to the history
				this will also append one successfull upload to the statistics */
		void	AppendUploadSpeed(double speed);

		/** Append one failed upload to the history */
		void	AppendFailedUpload();
	protected:
		class CTransferInfo{
		public:
			CTransferInfo();
			~CTransferInfo();
			bool		success;	// true if the transfer is successfull
			double	speed;		// the speed of the transfer
			char		hour;			// the hour when the transfer was made (0->23)
			char		minute;		// the minute when the transfer was made (0->59)
			char		second;		// the second when the transfer was made (0->59)
			char		day;			// the day of the month when the transfer was made (0->31)
		};

		/** The information about the downloads */
		CList <CTransferInfo, CTransferInfo&>	m_downloads;

		/** The information about the uploads */
		CList <CTransferInfo, CTransferInfo&>	m_uploads;

		// ------------- Protected methods ---------------

		/** Trim the list of transfer-data so that they only contain data from
				today... */
		void	TrimLists();

		/** Retrieves the success-rate from the given list */
		double GetSuccessRate(const CList <CTransferInfo, CTransferInfo&>	&list) const;

		/** Gets the average speed from the given list */
		double	GetAverageSpeed(const CList <CTransferInfo, CTransferInfo&>	&list) const;

		/** Returns the number of successful transfers today on this link */
		long	GetSuccessfulNum(const CList <CTransferInfo, CTransferInfo&>	&list) const;

		/** Appends the given data-speed to the given list */
		void	AppendSuccessfulTransfer(double speed, CList <CTransferInfo, CTransferInfo&>	&list);

		/** Appends a failure to the given list */
		void	AppendFailedTransfer(CList <CTransferInfo, CTransferInfo&>	&list);
	};
}