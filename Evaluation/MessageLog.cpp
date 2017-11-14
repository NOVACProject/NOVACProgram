/**
 * Contains the implementation of a message log object.
 *
 * @author		\URL[Stefan Kraus]{http://skraus.org} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
 * @version		1.1 @ 2002/01/17
 */
#include "stdafx.h"
#include "MessageLog.h"

CMessageLog	CMessageLog::mGlobal("CGlobalScope", __FILE__, __LINE__);
char* CMessageLog::mLogLevelMsg[] = { "Error", "Critical", "Warning", "Information" };
char CMessageLog::mLogFile[256] = "";
int CMessageLog::mLogLevel = CMessageLog::LOGINFO;
int CMessageLog::mMode = CMessageLog::LOGMODECONSOLE;

/**
 * Generates an information message.
 * The message text can be formated as using {\bf printf}.
 *
 * @param	szFormat	The format string for the message. Optional parameters can be added.
 *
 * @return	TRUE if successful, FALSE otherwise.
 */
bool Info(const char* szFormat, ...)
{
	// the pointer to the variable argument list
	va_list vaVarList;

	// get the pointer to the beginning of the variable argument list
	va_start(vaVarList, szFormat);

	// output message
	bool bRet = CMessageLog::mGlobal.OutputMessage(CMessageLog::LOGINFO, szFormat, vaVarList);

	// end access to the variable argument list
	va_end(vaVarList);

	return(bRet);
}

/**
 * Generates an information message.
 * The message text can be formated as using {\bf printf}.
 * Additionally the module's name and the line number can be supplied.
 *
 * @param	iLineNo		The line number within the source module to which the message belongs.
 * @param	szModule	The name of the source module to which the message belongs.
 * @param	szFormat	The format string for the message. Optional parameters can be added.
 *
 * @return	TRUE if successful, FALSE otherwise.
 */
bool Info(int iLineNo, const char* szModule, const char* szFormat, ...)
{
	// the pointer to the variable argument list
	va_list vaVarList;

	// get the pointer to the beginning of the variable argument list
	va_start(vaVarList, szFormat);

	// output message
	bool bRet = CMessageLog::mGlobal.OutputMessage(CMessageLog::LOGINFO, szFormat, vaVarList, szModule, iLineNo);

	// end access to the variable argument list
	va_end(vaVarList);

	return(bRet);
}

/**
 * Generates a warning message.
 * The message text can be formated as using {\bf printf}.
 *
 * @param	szFormat	The format string for the message. Optional parameters can be added.
 *
 * @return	TRUE if successful, FALSE otherwise.
 */
bool Warning(const char* szFormat, ...)
{
	// the pointer to the variable argument list
	va_list vaVarList;

	// get the pointer to the beginning of the variable argument list
	va_start(vaVarList, szFormat);

	// output message
	bool bRet = CMessageLog::mGlobal.OutputMessage(CMessageLog::LOGWARNING, szFormat, vaVarList);

	// end access to the variable argument list
	va_end(vaVarList);

	return(bRet);
}

/**
 * Generates a warning message.
 * The message text can be formated as using {\bf printf}.
 * Additionally the module's name and the line number can be supplied.
 *
 * @param	iLineNo		The line number within the source module to which the message belongs.
 * @param	szModule	The name of the source module to which the message belongs.
 * @param	szFormat	The format string for the message. Optional parameters can be added.
 *
 * @return	TRUE if successful, FALSE otherwise.
 */
bool Warning(int iLineNo, const char* szModule, const char* szFormat, ...)
{
	// the pointer to the variable argument list
	va_list vaVarList;

	// get the pointer to the beginning of the variable argument list
	va_start(vaVarList, szFormat);

	// output message
	bool bRet = CMessageLog::mGlobal.OutputMessage(CMessageLog::LOGWARNING, szFormat, vaVarList, szModule, iLineNo);

	// end access to the variable argument list
	va_end(vaVarList);

	return(bRet);
}

/**
 * Generates a critical information message.
 * The message text can be formated as using {\bf printf}.
 *
 * @param	szFormat	The format string for the message. Optional parameters can be added.
 *
 * @return	TRUE if successful, FALSE otherwise.
 */
bool Critical(const char* szFormat, ...)
{
	// the pointer to the variable argument list
	va_list vaVarList;

	// get the pointer to the beginning of the variable argument list
	va_start(vaVarList, szFormat);

	// output message
	bool bRet = CMessageLog::mGlobal.OutputMessage(CMessageLog::LOGCRITICAL, szFormat, vaVarList);

	// end access to the variable argument list
	va_end(vaVarList);

	return(bRet);
}

/**
 * Generates a critical information message.
 * The message text can be formated as using {\bf printf}.
 * Additionally the module's name and the line number can be supplied.
 *
 * @param	iLineNo		The line number within the source module to which the message belongs.
 * @param	szModule	The name of the source module to which the message belongs.
 * @param	szFormat	The format string for the message. Optional parameters can be added.
 *
 * @return	TRUE if successful, FALSE otherwise.
 */
bool Critical(int iLineNo, const char* szModule, const char* szFormat, ...)
{
	// the pointer to the variable argument list
	va_list vaVarList;

	// get the pointer to the beginning of the variable argument list
	va_start(vaVarList, szFormat);

	// output message
	bool bRet = CMessageLog::mGlobal.OutputMessage(CMessageLog::LOGCRITICAL, szFormat, vaVarList, szModule, iLineNo);

	// end access to the variable argument list
	va_end(vaVarList);

	return(bRet);
}

/**
 * Generates an error message.
 * The message text can be formated as using {\bf printf}.
 *
 * @param	szFormat	The format string for the message. Optional parameters can be added.
 *
 * @return	TRUE if successful, FALSE otherwise.
 */
bool Error(const char* szFormat, ...)
{
	// the pointer to the variable argument list
	va_list vaVarList;

	// get the pointer to the beginning of the variable argument list
	va_start(vaVarList, szFormat);

	// output message
	bool bRet = CMessageLog::mGlobal.OutputMessage(CMessageLog::LOGERROR, szFormat, vaVarList);

	// end access to the variable argument list
	va_end(vaVarList);

	return(bRet);
}

/**
 * Generates an error message.
 * The message text can be formated as using {\bf printf}.
 * Additionally the module's name and the line number can be supplied.
 *
 * @param	iLineNo		The line number within the source module to which the message belongs.
 * @param	szModule	The name of the source module to which the message belongs.
 * @param	szFormat	The format string for the message. Optional parameters can be added.
 *
 * @return	TRUE if successful, FALSE otherwise.
 */
bool Error(int iLineNo, const char* szModule, const char* szFormat, ...)
{
	// the pointer to the variable argument list
	va_list vaVarList;

	// get the pointer to the beginning of the variable argument list
	va_start(vaVarList, szFormat);

	// output message
	bool bRet = CMessageLog::mGlobal.OutputMessage(CMessageLog::LOGERROR, szFormat, vaVarList, szModule, iLineNo);

	// end access to the variable argument list
	va_end(vaVarList);

	return(bRet);
}

