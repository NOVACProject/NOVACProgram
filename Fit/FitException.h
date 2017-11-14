/**
 * Defines the basic exception class for errors in the fit routines. Any exception generated
 * should be derived from the class defined here.
 *
 * @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
 * @version		1.0 @ 2001/09/09
 */
#if !defined(FITEXCEPTION_H_001024)
#define FITEXCEPTION_H_001024

#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "MessageLog.h"

//#if defined(WIN32)
//#include "windows.h"
//#endif

namespace MathFit
{
#define EXCEPTION(classname) classname(__LINE__, __FILE__, #classname)

	/**
	* Basic exception class for fit exception.
	* To automatically set the module name and line number, use the macro
	* EXCEPTION(classname). This macro creates an instance of the given class
	* where the constructor of the class should expect the module name as
	* the first parameter and the line number as second. For example look at
	* \Ref{CVectorSizeMismatch}.
	*
	* @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
	* @version		1.0 @ 2001/09/09
	*/
	class CFitException : public CMessageLog
	{
	public:
		/**
		* Creates an empty exception object.
		*/
		CFitException() : CMessageLog("CFitException", __FILE__, __LINE__)
		{
			mMessage = NULL;
			mModule = NULL;
			mLineNo = 0;
			mErrorNo = 0;
		}

		/**
		* Creates an exception object with the given error data.
		*
		* @param szName	The name of the new exception object.
		* @param szMsg		The error message as string.
		*
		* @see	SetError
		*/
		CFitException(const char* szName, const char* szMsg, ...) : CMessageLog(szName)
		{
			va_list vaParams;

			va_start(vaParams, szMsg);
			SetError(szMsg, vaParams);
			va_end(vaParams);
		}

		/**
		* Creates an exception object with the given error data.
		*
		* @param iLine		The line number in the source module where the error happened.
		* @param szMod		The name of the source module where the error happened.
		* @param szName	The name of the new exception object.
		* @param szMsg		The error message as string.
		*
		* @see	SetError
		*/
		CFitException(int iLine, const char* szMod, const char* szName, const char* szMsg, ...) : CMessageLog(szName, szMod, iLine)
		{
			va_list vaParams;

			va_start(vaParams, szMsg);
			SetError(iLine, szMod, szMsg, vaParams);
			va_end(vaParams);
		}

		/**
		* Copies the content of the given exception object into the current one.
		*
		* @param cSource	The originating object.
		*/
		CFitException(CFitException& cSource) : CMessageLog(cSource)
		{
			if(cSource.mMessage)
			{
				mMessage = new char[strlen(cSource.mMessage) + 1];
				strcpy(mMessage, cSource.mMessage);
			}
			else
				mMessage = NULL;

			if(cSource.mModule)
			{
				mModule = new char[strlen(cSource.mModule) + 1];
				strcpy(mModule, cSource.mModule);
			}
			else
				mModule = NULL;

			mLineNo = cSource.mLineNo;
		}

		/**
		* Clears all allocated resources.
		*/
		~CFitException()
		{
			if(mMessage)
				delete(mMessage);
			if(mModule)
				delete(mModule);
		}											  

		/**
		* Sets the error messsage of the exception.
		*
		* @param szMsg		The error message as text. Can have format tags like {\bf printf}
		*/
		void SetError(const char* szMsg, ...)
		{
			mMessage = new char[CMessageLog::MAXMSGLEN];
			va_list vaParams;

			va_start(vaParams, szMsg);
			vsprintf(mMessage, szMsg, vaParams);
			va_end(vaParams);

			mModule = NULL;
			mLineNo = -1;
		}

		/**
		* Sets the error messsage of the exception.
		*
		* @param iLine		The line number in the source module where the error happened. Default: 0
		* @param szMod		The source module name where the error happend. Default: NULL (no module file)
		* @param szMsg		The error message as text. Can have format tags like {\bf printf}
		*/
		void SetError(int iLine, const char* szMod, const char* szMsg, ...)
		{
			mMessage = new char[CMessageLog::MAXMSGLEN];
			va_list vaParams;

			va_start(vaParams, szMsg);
			vsprintf(mMessage, szMsg, vaParams);
			va_end(vaParams);

			if(szMod != NULL)
			{
				mModule = new char[strlen(szMod) + 1];
				strcpy(mModule, szMod);
			}
			else
				mModule = NULL;

			mLineNo = iLine;

		}

		/**
		* Sets the error messsage of the exception.
		*
		* @param szMsg		The error message as text. Can have format tags like {\bf printf}
		* @param vaParams	Pointer to the optional parameters which are may be needed to format the message.
		*/
		void SetError(const char* szMsg, va_list vaParams)
		{
			mMessage = new char[CMessageLog::MAXMSGLEN];

			vsprintf(mMessage, szMsg, vaParams);

			mModule = NULL;
			mLineNo = -1;
		}

		/**
		* Sets the error messsage of the exception.
		*
		* @param iLine		The line number in the source module where the error happened. Default: 0
		* @param szMod		The source module name where the error happend. Default: NULL (no module file)
		* @param szMsg		The error message as text. Can have format tags like {\bf printf}
		* @param vaParams	Pointer to the optional parameters which are may be needed to format the message.
		*/
		void SetError(int iLine, const char* szMod, const char* szMsg, va_list vaParams)
		{
			mMessage = new char[CMessageLog::MAXMSGLEN];

			vsprintf(mMessage, szMsg, vaParams);

			if(szMod != NULL)
			{
				mModule = new char[strlen(szMod) + 1];
				strcpy(mModule, szMod);
			}
			else
				mModule = NULL;

			mLineNo = iLine;

		}

		/**
		* Displays the error to the user.
		* Depending on the compiler settings the error message it either put on the console
		* cerr-output stream or displayed as a Windows message box.
		*/
		void ReportError()
		{
			if(mModule)
				CMessageLog::Error(mLineNo, mModule, mMessage);
			else
				CMessageLog::Error(mMessage);
		}

	public:
		/**
		* Storage of the error message.
		*/
		char* mMessage;
		/**
		* Storage of the module name.
		*/
		char* mModule;
		/**
		* Storage of the source line number.
		*/
		int mLineNo;
		/**
		* Storage of the error ID.
		*/
		int mErrorNo;
	}; // class CException
}
#endif