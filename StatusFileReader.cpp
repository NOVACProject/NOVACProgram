#include "StdAfx.h"
#include "statusfilereader.h"
#include "Common/Common.h"
using namespace FileHandler;

CStatusFileReader::CStatusFileReader(void)
{
	Common commonObj;
	commonObj.GetExePath();
	m_workingPath = commonObj.m_exePath;
	memset(m_statusList,0,sizeof(char)* 1024);
}

CStatusFileReader::~CStatusFileReader(void)
{
}
void CStatusFileReader::SetWorkingPath(CString path)
{
	m_workingPath = path;
}
int CStatusFileReader::IsCommandReceived()
{
	int status = CheckStatus(COMMAND);
	return status;
}
int CStatusFileReader::IsStart()
{
	int status = 0;
	int startIndex,exitIndex;
	exitIndex = 0;
	startIndex = 0;
	Reorder();
	exitIndex = CheckStatus(STOPPROGRAM);
	startIndex = CheckStatus(STARTPROGRAM);
	if(startIndex > exitIndex)
		status = 1;
	return status;
}
int CStatusFileReader::IsExit()
{
	int status = 0;
	int startIndex,exitIndex;
	exitIndex = 0;
	startIndex = 0;
	Reorder();
	exitIndex = CheckStatus(STOPPROGRAM);
	startIndex = CheckStatus(STARTPROGRAM);
	if(startIndex < exitIndex)
		status = 1;
	return status;
}
int CStatusFileReader::IsPoweroff()
{
	int status = 0;
	int startIndex,exitIndex;
	exitIndex = 0;
	startIndex = 0;
	Reorder();
	exitIndex = CheckStatus(POWEROFF);
	startIndex = CheckStatus(POWERON);
	if(startIndex < exitIndex)
		status = 1;
	return status;
}
int CStatusFileReader::CheckStatus(unsigned char status)
{
	int i;
	for(i = 1023; i >=4 ; i--)
	{
		if(m_statusList[i] == status)
			return i;
	}	
  return 0;
}
void CStatusFileReader::ConvertStatusFile(CString fileFullName)
{
	int i;
	unsigned short last = 0;
    unsigned short first = 0;

	char txt[1024];
	unsigned short *p = (unsigned short*)txt;
	FILE* f = fopen(fileFullName,"rb");
	if(f != nullptr)
	{
		fread(p,1024,1,f);
		fclose(f);
		first=p[0];
		last=p[1];	//last is the latest status
	}	
	printf("first = %d, last = %d\n",first, last);
	printf("first msg is \n");
	StatusPrint(txt[first]);
	printf("last msg is \n");
	StatusPrint(txt[last]);
	for( i=first;i< 1024;i++)
	{
		printf("i = %d\n",i);
		StatusPrint(txt[i]);
	}
	for( i=4;i <= last;i++)
	{
		printf("i = %d\n",i);
		StatusPrint(txt[i]);
	}
    
}
void CStatusFileReader::StatusPrint(unsigned char s)
{
  switch(s)
    {
    case STARTPROGRAM:
      RecordStatus("Started Program"); break;
    case STOPPROGRAM:
      RecordStatus("Exiting program"); break;
    case WAIT4S:
      RecordStatus("Waiting 4 seconds"); break;
    case HOMEMOTOR:
      RecordStatus("Homing motor"); break;
    case MEMERR:
      RecordStatus("Not enough memory"); break;
    case USERINT:
      RecordStatus("Interrupted by user"); break;
    case EXECUTE:
      RecordStatus("Executing"); break;
    case DELETING: 
      RecordStatus("Deleting"); break;
    case OPENCOMERR:
      RecordStatus("Could not open COM port"); break;
    case GPSTIMEOUT:
      RecordStatus("GPS timeout"); break;
    case SETCPUTIME:
      RecordStatus("Setting CPU time&date from GPS time&date"); break;
    case USEDCPUTIME:
      RecordStatus("Used time from CPU"); break;
    case POWERON:
      RecordStatus("Power ON"); break;
    case POWEROFF:
      RecordStatus("Power OFF"); break;
    case MOVEMOTOROK:
      RecordStatus("MoveMotor Successful"); break;
    case MOTORDONE:
      RecordStatus("Motor done"); break;
    case ALREADYHOME:
      RecordStatus("Motor already at home. Will make one round to reset.\n"); break;
    case MOVEMOTOR:
      RecordStatus("MoveMotor"); break;
    case DOSCANDONE:
      RecordStatus("doscan DONE"); break;
    case DOSCANTIMEOUT:
      RecordStatus("Timeout in doscan"); break;
    case SPECREFUSE:
      RecordStatus("Spectrometer refuses to scan. Reinitializing");
    case DONEINITSPEC:
      RecordStatus("InitSpectrometer done"); break;
    case INITSPEC:
      RecordStatus("InitSpectrometer"); break;
    case WRITEOK:
      RecordStatus("Writing OK"); break;
    case NOTOK:
      RecordStatus("Not successful"); break;
    case CONTACTSPEC:
      RecordStatus("Got contact with spectrometer"); break;
    case NOCONTACTSPEC:
      RecordStatus("No contact with spectrometer"); break;
    case SERIALOPEN:
      RecordStatus("Opened serial port"); break;
    case SERIALOPENERR:
      RecordStatus("Could not open serial port"); break;
    case COMMAND:
      RecordStatus("Command"); break;
    case PAUSE:
      RecordStatus("Pause"); break;
    case RESUME:
      RecordStatus("Resume"); break;
    }
		
}
void CStatusFileReader::RecordStatus(char* txt)
{
	//puts(txt);
	CString filePath;
	filePath.Format("%sstatusDAT.txt",m_workingPath);
	 FILE *f = fopen(filePath, "a+");
  if(f != NULL){
    fprintf(f, "%s\n", txt);
    fclose(f);
  }
}
void CStatusFileReader::Reorder()
{	
	int i;
	unsigned short last = 0;
    unsigned short first = 0;
	char txt[1024];
	memset(txt,0,sizeof(char)*1024);
	unsigned short* p = (unsigned short*)txt;
	CString fileFullName;
	fileFullName.Format("%sstatus.dat",m_workingPath);
	FILE* f = fopen(fileFullName,"rb");
	if(f>(FILE *)0)
	{
		fread(p,1024,1,f);
		fclose(f);
		first=p[0];
		last=p[1];	//last is the latest status
	}	
	printf("first = %d,last = %d\n",first,last);
	int count = 4;
	if(first > last)
	{
		for( i=first;i< 1024;i++)
		{
			ASSERT(count < 1024);
			m_statusList[count] = txt[i];
			count++;
		}
		for( i=4;i <= last;i++)
		{		
			ASSERT(count < 1024);
			m_statusList[count] = txt[i];
			count++;
		}
	}
	else
	{
		for(i= first; i<= last;i++)
		{
			ASSERT(count < 1024);
			m_statusList[count] = txt[i];
			count++;
		}	
	}
}