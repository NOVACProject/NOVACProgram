#pragma once
namespace FileHandler
{	
	#define STARTPROGRAM 1
	#define STOPPROGRAM 2
	#define WAIT4S 3
	#define HOMEMOTOR 4
	#define MEMERR 5
	#define USERINT 6
	#define EXECUTE 7
	#define DELETING 8
	#define OPENCOMERR 9
	#define GPSTIMEOUT 10
	#define SETCPUTIME 11
	#define USEDCPUTIME 12
	#define POWERON 13
	#define POWEROFF 14
	#define MOVEMOTOROK 15
	#define MOTORDONE 16
	#define ALREADYHOME 17
	#define MOVEMOTOR 18
	#define DOSCANDONE 19
	#define DOSCANTIMEOUT 20
	#define SPECREFUSE 21
	#define DONEINITSPEC 22
	#define INITSPEC 23
	#define WRITEOK 24
	#define NOTOK 25
	#define CONTACTSPEC 26
	#define NOCONTACTSPEC 27
	#define SERIALOPEN 28
	#define SERIALOPENERR 29
	#define COMMAND 30
	#define PAUSE 31
	#define RESUME 32

	class CStatusFileReader
	{
	public:
		CStatusFileReader(void);
		~CStatusFileReader(void);
		/**reorder the status records, from index=4 to index = 1023, records are listed
		*from oldest to latest.
		*@param fileFullName full path and name of the status.dat file
		*/
		void Reorder();

		/**check latest 10 status records to find out one which is same as the input value
		*@param status the status to be found
		*return 0 - if status is not found
		*return 1 - if status is found
		*/
		int CheckStatus(unsigned char status);
		/**Check if command was received
		*return 0 - if is not received
		*return 1 - if it is received
		*/
		int IsCommandReceived();
		/**Check if kongo exited
		*return 1 - exited
		*/
		int IsExit();
		/**check if kongo is started*/
		int IsStart();
		/**check if kongo is power off*/
		int IsPoweroff();
		void StatusPrint(unsigned char s);
		void RecordStatus(char* txt);
		/**Convert status file status.dat into readable txt file*/
		void ConvertStatusFile(CString fileFullName);
		void SetWorkingPath(CString path);
	protected:
		CString m_workingPath;
		char m_statusList[1024];
	};
}