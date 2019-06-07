#pragma once
#include "afxwin.h"

namespace Communication
{
	/**<b>CSerialCOM</b> is a base class for serial communication. Its base class
	*is CWinThread.
	*/
	class CSerialCOM :
		public CWinThread
	{
	public:
		CSerialCOM(void);
		~CSerialCOM(void);

		/**The structure for SerialPort*/
		struct SerialPort
		{
			int COMPort;
			int baudrate;
			int parity;
			int length;
			int stopBit;
			int fRTS;
			bool fCTS;
		};
		struct SerialPort m_Port;
		HANDLE	hComPort;
		char	sourceBuffer[256];//old 10
		int		sourceBufferPointer;
		/** flag to judge wheter DTR is used */
		bool  m_DTRControl; 
	public:
		/**Set the  information for the serial port to be used.
		*@param COMPort the COM port number
		*@param baudrate the baudrate for serial link communication
		*@param parity use parity or not
		*@param length length of transmited byte, 5~8
		*@param stopBit stop bits ,1~2
		*@param fRTS
		*/
		void SetSerialPort(int COMPort,int baudrate,int parity,int length,int stopBit,int fRTS,bool fCTS);
		int InitialSerialPort();
		BOOL WriteSerial(const void *sendText,long sentByteNum);
		/**Close serial communication*/
		void CloseSerialPort();
		/**Read information from serial port.
		*@param receiveBuf The buffer for recieved data
		*@param receiveBufferSize The size of the buffer to store received data
		*/
		long ReadSerial(void *receiveBuf,long receiveBufferSize);
		/**Check serial port
		*@param timeout the time to check serial, in miliseconds
		*/
		int CheckSerial(long timeOut);
		/**Flush serial port, clean up serial port
		*@param timeout the time to check serial, in miliseconds
		*/
		void FlushSerialPort(long timeOut);
		/**Receive a pile of data
		*@param timeout the time to check serial, in miliseconds
		*@param receiveBuf the buffer for received data
		*@param receiveBufferSize receiving buffer size
		*/
		int ReceiveFile(int timeout,char* receiveBuf,long receiveBufferSize);
		/**Retrieve data from serial port
		*@param buffer the buffer to put in data
		*@param length the size of the retrieved data
		*@param timeout the longest waiting time
		*/
		int GetSerialData(void *buffer,int length,int timeout);
		void SetDTRControl(bool DTRFlag);
	};
}