#include "StdAfx.h"
#include "serialcom.h"

using namespace Communication;
CSerialCOM::CSerialCOM(void)
{
	m_DTRControl = false;
}

CSerialCOM::~CSerialCOM(void)
{
}
void CSerialCOM::SetSerialPort(int COMPort,int baudrate,int parity,int length,int stopBit,int fRTS,bool fCTS)
{
	m_Port.baudrate = baudrate;
	m_Port.COMPort = COMPort;
	m_Port.length = length;
	m_Port.parity = parity;
	m_Port.fRTS = fRTS;
	m_Port.stopBit = stopBit;
	m_Port.fCTS = fCTS;

}
int CSerialCOM::InitialSerialPort()
{
	DCB dcb;
	CString portStr;
	
	// creat serial port
	if(m_Port.COMPort > 9)
		portStr.Format("\\\\.\\COM%d", m_Port.COMPort);
	else
		portStr.Format("COM%d",m_Port.COMPort);
	hComPort = CreateFile( portStr,
					GENERIC_READ | GENERIC_WRITE,
					0,
					0,
					OPEN_EXISTING,
					FILE_FLAG_WRITE_THROUGH,// | FILE_FLAG_OPEN_NO_RECALL| FILE_FLAG_OVERLAPPED,
					0);
	if (hComPort == INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	// clear dcb (data control block)
	FillMemory(&dcb, sizeof(dcb), 0);

	if (!GetCommState(hComPort, &dcb))     // get current DCB
	  return 0;

	// set dcb parameters
	dcb.BaudRate        = m_Port.baudrate;
	dcb.ByteSize        = m_Port.length;//8;
	dcb.Parity          = m_Port.parity;//NOPARITY;
	dcb.StopBits        = m_Port.stopBit;//ONESTOPBIT;
	dcb.fAbortOnError   = FALSE;
	dcb.fDsrSensitivity = 0;
	dcb.fInX            = 0;
	dcb.fRtsControl     = m_Port.fRTS;
	dcb.fDtrControl     = 0;
	dcb.fOutxCtsFlow = m_Port.fCTS;//0;//TRUE

	if (!SetCommState(hComPort, &dcb))
	{
		return 0;
	}
	if(m_DTRControl)
	{ 
		EscapeCommFunction(hComPort,CLRDTR);  
		Sleep(100);
		EscapeCommFunction(hComPort,SETDTR); // try
	}
	return 1;
}
void CSerialCOM::CloseSerialPort()
{
	if(m_DTRControl)
		EscapeCommFunction(hComPort,CLRDTR);  //TRY 
	if(hComPort!=NULL)
		CloseHandle(hComPort);
	hComPort = NULL;
}
//-----------------------------------------------------------------
BOOL CSerialCOM::WriteSerial(void *sendText,long sentByteNum)
{
	DWORD dwWritten;
	BOOL result = WriteFile(hComPort, sendText, sentByteNum, &dwWritten, NULL);
	return result;
}

long CSerialCOM::ReadSerial(void *receiveBuf,long receiveBufferSize)
{
	char *bufPointer;//char *bp;
	long readByteNum;//lreal;
	COMMTIMEOUTS timeouts;
	DWORD dwRead;

	// read buffer pointer
	bufPointer=(char*)receiveBuf;

	// copy data to read buffer
	// break when the buffer is full or the source buffer is empty
	for(readByteNum=0;readByteNum<receiveBufferSize && sourceBufferPointer;readByteNum++)
		{
			bufPointer[readByteNum]=sourceBuffer[readByteNum];
			sourceBufferPointer--;
		}

	// if read buffer is full, return
	if(receiveBufferSize==readByteNum) 
	  return(readByteNum);

	// the source buffer is empty, no data available in the source buffer any more

	timeouts.ReadIntervalTimeout = MAXDWORD;
	timeouts.ReadTotalTimeoutMultiplier = 0;
	timeouts.ReadTotalTimeoutConstant = 0;

	if ((!SetCommTimeouts(hComPort, &timeouts)))
	{
	}
	// read data from hComPort file to read buffer
	if (!ReadFile(hComPort, &bufPointer[readByteNum], receiveBufferSize - readByteNum, &dwRead, NULL)) {
		return 0;
	}

	// total read data length
	readByteNum+=dwRead;

	return(readByteNum);
}

// check serial. if there is data in serial port, save it to sourceBuffer
int CSerialCOM::CheckSerial(long timeOut)
{
	DWORD dwRead;
	COMMTIMEOUTS timeouts;

	// if sourceBuffer is not empty, return 1
	// if sourceBuffer is empty, read data from serial port and save to sourceBuffer
	if(sourceBufferPointer) return(1);

	GetCommTimeouts(hComPort,&timeouts);

	timeouts.ReadIntervalTimeout = MAXWORD;
	timeouts.ReadTotalTimeoutMultiplier = 1;
	timeouts.ReadTotalTimeoutConstant = timeOut;

	if (SetCommTimeouts(hComPort, &timeouts)==0)
	{
		printf("Error setting time-outs in CheckSerial.\n");
		return 0;
	}

	// read data from serial port buffer, save data to sourceBuffer.
	// sourceBufferPointer is the data counter
	if (!ReadFile(hComPort, sourceBuffer, 1, &dwRead, NULL)) {
		return(0);
	}
	if(dwRead == 0)
		return(0);
	sourceBufferPointer+=dwRead;

	return(1);
}

int CSerialCOM::ReceiveFile(int timeout,char* receiveBuf,long receiveBufferSize)
{
	char* bufferPointer;
	bufferPointer = receiveBuf;
	int i,j;
	CheckSerial(timeout); //wait first byte to come

	i=0;

	while((CheckSerial(timeout/10))&&(i<receiveBufferSize))
	{
		j=ReadSerial(&bufferPointer[i],receiveBufferSize - i); 
		i+=j;
	}
	return i;
}

// clear serial buffer
void CSerialCOM::FlushSerialPort(long timeOut)
{
	char txt[1];
	// read data from serial port until there is not data in
	//the serial buffer
	while(CheckSerial(timeOut))
		ReadSerial(&txt,1);
}
//retrieve serial port data
int CSerialCOM::GetSerialData(void *buffer,int length,int timeout)
{
	long i,j;
	i=0;
	do
		{  
		if(CheckSerial(timeout)==0) 
			break;
		j = ReadSerial((unsigned char *)buffer + i,length - i);
		i+=j;
		} while(i<length);
	return(i);
}
void CSerialCOM::SetDTRControl(bool DTRFlag)
{
	m_DTRControl = DTRFlag;
}