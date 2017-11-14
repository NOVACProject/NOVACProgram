/**
 * Contains the declaration of a message log object.
 *
 * @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
 * @version		1.1 @ 2002/01/17
 */
#if !defined(__MESSAGELOG_H_20020117)
#define __MESSAGELOG_H_20020117

#include <iostream>
#include <stdarg.h>
#include <time.h>

/**
 * This class handles any neccessary actions needed to log messages and errors. 
 * The object is identified by its name and the location where it's instanciated.
 *
 * @author		\URL[Stefan Kraus]{http://stefan@00kraus.de} @ \URL[IWR, Image Processing Group]{http://klimt.iwr.uni-heidelberg.de}
 * @version		1.1 @ 2002/01/17
 */
class CMessageLog  
{
public:
	static CMessageLog mGlobal;

	/**
	 * Construct messaging object.
	 *
	 * @param szObjectName	The name of the current object. Should be set to the topmost derivied class's name.
	 * @param szModule		The name of the source file where the object is created.
	 * @param iLineNo		The line number in the source file where the object is created.
	 */
	CMessageLog(const char* szObjectName, const char* szModule = NULL, const int iLineNo = -1)
	{
		char* szObjName = (char*)szObjectName;
		if(!szObjectName)
			szObjName = "none";

		char* szName;
		if(szModule)
		{
			szName = strrchr(szModule, '\\');
			if(!szName)
			{
				szName = strrchr(szModule, '/');
				if(!szName)
					szName = (char*)szModule;
				else
					szName += 1;
			}
			else
				szName += 1;

			mID = new char[strlen(szObjName) + strlen(szName) + 32];
			sprintf(mID, "%s(%s@%d)", szObjName, szName, iLineNo);
		}
		else
		{

			mID = new char[strlen(szObjName) + 1];
			sprintf(mID, "%s", szObjName);
		}
	}

	/**
	 * Copy constructor.
	 * Creates a copy of the given object.
	 *
	 * @param cSource	The object to be copied.
	 */
	CMessageLog(const CMessageLog& cSource)
	{
		mID = new char[strlen(cSource.mID) + 1];
		strcpy(mID, cSource.mID);
	}

	/**
	 * Destructs the messaging object.
	 */
	~CMessageLog()
	{
		if(mID)
			delete(mID);
	}

	/**
	 * Sets the name of the message output file.
	 *
	 * @param szFileName	The name and/or path of the message output file.
	 * @param bAppend		If TRUE new messages are appended to the old file even if the program is restarted. Otherwise the file is erased on program start.
	 *
	 * @return	TRUE if successful, FALSE otherwise.
	 */
	static bool SetLogFile(const char* szFileName, bool bAppend = true)
	{
		// copy file name
		strcpy(mLogFile, szFileName);

		FILE* ioLog;

		// check for file access
		if(!bAppend)
			ioLog = fopen(mLogFile, "wt");
		else
			ioLog = fopen(mLogFile, "a+t");
		if(!ioLog)
		{
			mGlobal.Error(__LINE__, __FILE__, "Unable to access log file!\nSwitching to standard output!");
			mLogFile[0] = 0;
			return(false);
		}
		else
			fclose(ioLog);

		mMode |= LOGMODEFILE;

		return(true);
	}

	/**
	 * Sets the current log-level.
	 * You can use the log-level to control what messages actually are printed.
	 * The following log levels are defined:
	 *
	 * \Ref{LOGERROR}:		Use this level for error.
	 * \Ref{LOGCRITICAL}:	Use this level for critical warnings.
	 * \Ref{LOGWARNING}:	Use this level for warning.
	 * \Ref{LOGINFO}:		Use this level for additional information.
	 *
	 * @param iNewLogLevel	The new log-level.
	 *
	 * @param TRUE if successful, FALSE otherwise.
	 */
	static bool SetLogLevel(int iNewLogLevel)
	{
		if(iNewLogLevel < LOGERROR || iNewLogLevel > LOGINFO)
			return(false);

		mLogLevel = iNewLogLevel;
		return(true);
	}

	/**
	 * Returns the current log level.
	 *
	 * @return The current log level.
	 */
	static int GetLogLevel()
	{
		return(mLogLevel);
	}

	/**
	 * Generates an information message.
	 * The message text can be formated as using {\bfprintf}.
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
		bool bRet = OutputMessage(LOGINFO, szFormat, vaVarList);

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
		bool bRet = OutputMessage(LOGINFO, szFormat, vaVarList, szModule, iLineNo);

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
		bool bRet = OutputMessage(LOGWARNING, szFormat, vaVarList);

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
		bool bRet = OutputMessage(LOGWARNING, szFormat, vaVarList, szModule, iLineNo);

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
		bool bRet = OutputMessage(LOGCRITICAL, szFormat, vaVarList);

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
		bool bRet = OutputMessage(LOGCRITICAL, szFormat, vaVarList, szModule, iLineNo);

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
		bool bRet = OutputMessage(LOGERROR, szFormat, vaVarList);

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
		bool bRet = OutputMessage(LOGERROR, szFormat, vaVarList, szModule, iLineNo);

		// end access to the variable argument list
		va_end(vaVarList);

		return(bRet);
	}

	/**
	 * Generates the message and displays it on the selected media.
	 * This function checks for the correct log level. Any message not matching the 
	 * defined log criteria will be immediately discarded. If neccessary message boxes,
	 * trace outputs and log files are created and maintained as defined by \Ref{SetOutputMode}.
	 *
	 * @param iLogLevel	The log level of the message.
	 * @param szFormat	The message format string. Uses the syntax of the printf format specifiers.
	 * @param vaVarList	Pointer to the beginning of the variable argument list for the message's parameters.
	 * @param szModule	The name of the source module from where the message was issued.
	 * @param iLineNo	The line number from where the message was issued.
	 *
	 * @return	TRUE if successful, FALSE otherwise
	 */
	bool OutputMessage(int iLogLevel, const char* szFormat, va_list vaVarList = NULL, const char* szModule = NULL, int iLineNo = -1)
	{
		// check for valid log level
		if(iLogLevel < LOGERROR || iLogLevel > LOGINFO)
			iLogLevel = LOGERROR;

		// check wheter we should display the current message anyway
		if(!CheckLogLevel(iLogLevel)	)
			return(true);

		// strip path definitions
		char* szModName = strrchr(szModule, '\\');
		if(!szModName)
		{
			szModName = strrchr(szModule, '/');
			if(!szModName)
				szModName = (char*)szModule;
			else
				szModName += 1;
		}
		else
			szModName += 1;

		char* szMsg = new char[MAXMSGLEN];

		// format the given message
		vsprintf(szMsg, szFormat, vaVarList);

		// if window mode is applied, display the message box
		if(mMode & LOGMODEWINDOW)
		{
			// only display a message box if an error occured or no other output media is selected
			if(!(mMode & (~LOGMODEWINDOW)) || iLogLevel == LOGERROR)
			{
				char* szOut = new char[1024];

				if(szModName)
					sprintf(szOut, "ID: %s\nModule: %s\nLine: %d\nError: %s", mID, szModName, iLineNo, szMsg);
				else
					sprintf(szOut, "ID: %s\nError: %s", mID, szMsg);

//#if defined(WIN32)
//				switch(iLogLevel)
//				{
//				case LOGERROR:
//					MessageBox(NULL, szOut, mLogLevelMsg[iLogLevel], MB_ICONSTOP | MB_OK);
//					break;
//				case LOGCRITICAL:
//					MessageBox(NULL, szOut, mLogLevelMsg[iLogLevel], MB_ICONEXCLAMATION | MB_OK);
//					break;
//				case LOGWARNING:
//					MessageBox(NULL, szOut, mLogLevelMsg[iLogLevel], MB_ICONEXCLAMATION | MB_OK);
//					break;
//				case LOGINFO:
//					MessageBox(NULL, szOut, mLogLevelMsg[iLogLevel], MB_ICONINFORMATION | MB_OK);
//					break;
//				default:
//					MessageBox(NULL, szOut, mLogLevelMsg[LOGERROR], MB_ICONSTOP | MB_OK);
//					break;
//				}
//#endif
				delete(szOut);
			}
		}

		if(mMode & (~LOGMODEWINDOW))
		{
			// build output message
			char* szOut = new char[1024];

			time_t timeNow = time(NULL);

			if(szModName)
				sprintf(szOut, "%.24s - %s from %s at {%s@%d}: %s\n", asctime(localtime(&timeNow)), mLogLevelMsg[iLogLevel], mID, szModName, iLineNo, szMsg);
			else
				sprintf(szOut, "%.24s - %s from %s: %s\n", asctime(localtime(&timeNow)), mLogLevelMsg[iLogLevel], mID, szMsg);

			// check for console output
			if(mMode & LOGMODECONSOLE)
			{
				// if we have the TRACE statement and are in debug mode, put it on the trace panel
#if defined(TRACE) && defined(_DEBUG)
				TRACE(szOut);
#endif
				// put the message on stdout
				std::cout << szOut;
			}

			// check for file output
			if((mMode & LOGMODEFILE) && mLogFile[0])
			{
				FILE* ioOut = fopen(mLogFile, "a+t");
				if(ioOut)
				{
					fputs(szOut, ioOut);
					fclose(ioOut);
				}
			}
			
			delete(szMsg);
			delete(szOut);
		}

		return(true);
	}

	/**
	 * Sets the message output mode.
	 * You can specify how messages are displayed to the user by combining one or more of these modes:
	 *
	 * \Ref{LOGMODECONSOLE}:	The messages are printed to the standard output stream.
	 * \Ref{LOGMODEWINDOW}:		The messages are displayed as a Windows message box. (Only available in Windows executables!)
	 * \Ref{LOGMODEFILE}:		If specified, the messages are written to a file.
	 *
	 * @param iMode	The new output mode to be used. Default: LOGMODECONSOLE
	 */
	static void SetOutputMode(int iMode)
	{
		// check for non Windows environment
#if !defined(WIN32)
		if(iMode & LOGMODEWINDOW)
		{
			iMode &= ~LOGMODEWINDOW;
			iMode |= LOGMODECONSOLE;
		}
#endif

		// and set the new mode
		if(iMode == 0)
			mMode = LOGMODECONSOLE;
		else
			mMode = iMode;
	}

	/**
	 * Returns the current output mode.
	 *
	 * @return The current output mode.
	 */
	static int GetOutputMode()
	{
		return(mMode);
	}

	/**
	 * Defintion of the possible message output modes.
	 */
	enum ELogModes
	{ 
		/**
		 * Direct output to console.
		 */
		LOGMODECONSOLE = 1, 
		/**
		 * Direct output to message box.
		 */
		LOGMODEWINDOW = 2, 
		/**
		 * Direct output to a file.
		 */
		LOGMODEFILE = 4 
	};

	/**
	 * Definition of the possible message types.
	 */
	enum EMessageType
	{
		/**
		 * Defines an error message.
		 */
		LOGERROR = 0, 
		/**
		 * Defines a critical warning.
		 */
		LOGCRITICAL, 
		/**
		 * Defines a warning.
		 */
		LOGWARNING, 
		/**
		 * Defines an information message.
		 */
		LOGINFO 
	};

	/**
	 * Definition of global constants.
	 */
	enum EDefines
	{
		/**
		 * Defines the maximum length of a message.
		 */
		MAXMSGLEN = 512
	};

private:
	/**
	 * Checks wheter the given log level allows printing or not.
	 *
	 * @param aLevel	The log level of the message to be displayed.
	 *
	 * @return TRUE if the message can be displayed, FALSE otherwise.
	 */
	bool CheckLogLevel(int aLevel)
	{
		if(aLevel <= mLogLevel)
			return(true);
		return(false);
	}

	/**
	 * Contains the defintion of the error types.
	 */
	static char* mLogLevelMsg[];
	/**
	 * Contains the output's file name.
	 */
	static char mLogFile[256];
	/**
	 * Contains the current log level.
	 *
	 * @see SetLogLevel
	 */
	static int mLogLevel;
	/**
	 * Contains the current output mode.
	 *
	 * @see SetOutputMode
	 */
	static int mMode;
	/**
	 * Constains the objects identifier.
	 */
	char* mID;
};

bool Info(const char* szFormat, ...);
bool Info(int iLineNo, const char* szModule, const char* szFormat, ...);
bool Warning(const char* szFormat, ...);
bool Warning(int iLineNo, const char* szModule, const char* szFormat, ...);
bool Critical(const char* szFormat, ...);
bool Critical(int iLineNo, const char* szModule, const char* szFormat, ...);
bool Error(const char* szFormat, ...);
bool Error(int iLineNo, const char* szModule, const char* szFormat, ...);

/**
 * This macro calls the \Ref{Info} function including the current module name and line number.
 *
 * @param msg	The log message
 */
#define Info0(msg)								Info(__LINE__, __FILE__, msg)
/**
 * This macro calls the \Ref{Info} function including the current module name and line number.
 * This macro expects one optional parameters.
 *
 * @param msg	The log message
 * @param a1	The first optional parameter.
 */
#define Info1(msg,a1)							Info(__LINE__, __FILE__, msg ,a1)
/**
 * This macro calls the \Ref{Info} function including the current module name and line number.
 * This macro expects two optional parameters.
 *
 * @param msg	The log message
 * @param a1	The first optional parameter.
 * @param a2	The second optional parameter.
 */
#define Info2(msg,a1,a2)						Info(__LINE__, __FILE__, msg ,a1, a2)
/**
 * This macro calls the \Ref{Info} function including the current module name and line number.
 * This macro expects three optional parameters.
 *
 * @param msg	The log message
 * @param a1	The first optional parameter.
 * @param a2	The second optional parameter.
 * @param a3	The third optional parameter.
 */
#define Info3(msg,a1,a2,a3)						Info(__LINE__, __FILE__, msg ,a1, a2, a3)
/**
 * This macro calls the \Ref{Info} function including the current module name and line number.
 * This macro expects four optional parameters.
 *
 * @param msg	The log message
 * @param a1	The first optional parameter.
 * @param a2	The second optional parameter.
 * @param a3	The third optional parameter.
 * @param a4	The fourth optional parameter.
 */
#define Info4(msg,a1,a2,a3,a4)					Info(__LINE__, __FILE__, msg ,a1, a2, a3, a4)
/**
 * This macro calls the \Ref{Info} function including the current module name and line number.
 * This macro expects five optional parameters.
 *
 * @param msg	The log message
 * @param a1	The first optional parameter.
 * @param a2	The second optional parameter.
 * @param a3	The third optional parameter.
 * @param a4	The fourth optional parameter.
 * @param a5	The fifth optional parameter.
 */
#define Info5(msg,a1,a2,a3,a4,a5)				Info(__LINE__, __FILE__, msg ,a1, a2, a3, a4, a5)
/**
 * This macro calls the \Ref{Info} function including the current module name and line number.
 * This macro expects six optional parameters.
 *
 * @param msg	The log message
 * @param a1	The first optional parameter.
 * @param a2	The second optional parameter.
 * @param a3	The third optional parameter.
 * @param a4	The fourth optional parameter.
 * @param a5	The fifth optional parameter.
 * @param a6	The sixth optional parameter.
 */
#define Info6(msg,a1,a2,a3,a4,a5,a6)			Info(__LINE__, __FILE__, msg ,a1, a2, a3, a4, a5, a6)
/**
 * This macro calls the \Ref{Info} function including the current module name and line number.
 * This macro expects seven optional parameters.
 *
 * @param msg	The log message
 * @param a1	The first optional parameter.
 * @param a2	The second optional parameter.
 * @param a3	The third optional parameter.
 * @param a4	The fourth optional parameter.
 * @param a5	The fifth optional parameter.
 * @param a6	The sixth optional parameter.
 * @param a7	The seventh optional parameter.
 */
#define Info7(msg,a1,a2,a3,a4,a5,a6,a7)			Info(__LINE__, __FILE__, msg ,a1, a2, a3, a4, a5, a6, a7)
/**
 * This macro calls the \Ref{Info} function including the current module name and line number.
 * This macro expects eight optional parameters.
 *
 * @param msg	The log message
 * @param a1	The first optional parameter.
 * @param a2	The second optional parameter.
 * @param a3	The third optional parameter.
 * @param a4	The fourth optional parameter.
 * @param a5	The fifth optional parameter.
 * @param a6	The sixth optional parameter.
 * @param a7	The seventh optional parameter.
 * @param a8	The eighth optional parameter.
 */
#define Info8(msg,a1,a2,a3,a4,a5,a6,a7,a8)		Info(__LINE__, __FILE__, msg ,a1, a2, a3, a4, a5, a6, a7, a8)
/**
 * This macro calls the \Ref{Info} function including the current module name and line number.
 * This macro expects nine optional parameters.
 *
 * @param msg	The log message
 * @param a1	The first optional parameter.
 * @param a2	The second optional parameter.
 * @param a3	The third optional parameter.
 * @param a4	The fourth optional parameter.
 * @param a5	The fifth optional parameter.
 * @param a6	The sixth optional parameter.
 * @param a7	The seventh optional parameter.
 * @param a8	The eighth optional parameter.
 * @param a9	The ninth optional parameter.
 */
#define Info9(msg,a1,a2,a3,a4,a5,a6,a7,a8,a9)	Info(__LINE__, __FILE__, msg ,a1, a2, a3, a4, a5, a6, a7, a8, a9)

/**
 * This macro calls the \Ref{Warning} function including the current module name and line number.
 *
 * @param msg	The log message
 */
#define Warning0(msg)								Warning(__LINE__, __FILE__, msg)
/**
 * This macro calls the \Ref{Warning} function including the current module name and line number.
 * This macro expects one optional parameters.
 *
 * @param msg	The log message
 * @param a1	The first optional parameter.
 */
#define Warning1(msg,a1)							Warning(__LINE__, __FILE__, msg ,a1)
/**
 * This macro calls the \Ref{Warning} function including the current module name and line number.
 * This macro expects two optional parameters.
 *
 * @param msg	The log message
 * @param a1	The first optional parameter.
 * @param a2	The second optional parameter.
 */
#define Warning2(msg,a1,a2)						Warning(__LINE__, __FILE__, msg ,a1, a2)
/**
 * This macro calls the \Ref{Warning} function including the current module name and line number.
 * This macro expects three optional parameters.
 *
 * @param msg	The log message
 * @param a1	The first optional parameter.
 * @param a2	The second optional parameter.
 * @param a3	The third optional parameter.
 */
#define Warning3(msg,a1,a2,a3)						Warning(__LINE__, __FILE__, msg ,a1, a2, a3)
/**
 * This macro calls the \Ref{Warning} function including the current module name and line number.
 * This macro expects four optional parameters.
 *
 * @param msg	The log message
 * @param a1	The first optional parameter.
 * @param a2	The second optional parameter.
 * @param a3	The third optional parameter.
 * @param a4	The fourth optional parameter.
 */
#define Warning4(msg,a1,a2,a3,a4)					Warning(__LINE__, __FILE__, msg ,a1, a2, a3, a4)
/**
 * This macro calls the \Ref{Warning} function including the current module name and line number.
 * This macro expects five optional parameters.
 *
 * @param msg	The log message
 * @param a1	The first optional parameter.
 * @param a2	The second optional parameter.
 * @param a3	The third optional parameter.
 * @param a4	The fourth optional parameter.
 * @param a5	The fifth optional parameter.
 */
#define Warning5(msg,a1,a2,a3,a4,a5)				Warning(__LINE__, __FILE__, msg ,a1, a2, a3, a4, a5)
/**
 * This macro calls the \Ref{Warning} function including the current module name and line number.
 * This macro expects six optional parameters.
 *
 * @param msg	The log message
 * @param a1	The first optional parameter.
 * @param a2	The second optional parameter.
 * @param a3	The third optional parameter.
 * @param a4	The fourth optional parameter.
 * @param a5	The fifth optional parameter.
 * @param a6	The sixth optional parameter.
 */
#define Warning6(msg,a1,a2,a3,a4,a5,a6)			Warning(__LINE__, __FILE__, msg ,a1, a2, a3, a4, a5, a6)
/**
 * This macro calls the \Ref{Warning} function including the current module name and line number.
 * This macro expects seven optional parameters.
 *
 * @param msg	The log message
 * @param a1	The first optional parameter.
 * @param a2	The second optional parameter.
 * @param a3	The third optional parameter.
 * @param a4	The fourth optional parameter.
 * @param a5	The fifth optional parameter.
 * @param a6	The sixth optional parameter.
 * @param a7	The seventh optional parameter.
 */
#define Warning7(msg,a1,a2,a3,a4,a5,a6,a7)			Warning(__LINE__, __FILE__, msg ,a1, a2, a3, a4, a5, a6, a7)
/**
 * This macro calls the \Ref{Warning} function including the current module name and line number.
 * This macro expects eight optional parameters.
 *
 * @param msg	The log message
 * @param a1	The first optional parameter.
 * @param a2	The second optional parameter.
 * @param a3	The third optional parameter.
 * @param a4	The fourth optional parameter.
 * @param a5	The fifth optional parameter.
 * @param a6	The sixth optional parameter.
 * @param a7	The seventh optional parameter.
 * @param a8	The eighth optional parameter.
 */
#define Warning8(msg,a1,a2,a3,a4,a5,a6,a7,a8)		Warning(__LINE__, __FILE__, msg ,a1, a2, a3, a4, a5, a6, a7, a8)
/**
 * This macro calls the \Ref{Warning} function including the current module name and line number.
 * This macro expects nine optional parameters.
 *
 * @param msg	The log message
 * @param a1	The first optional parameter.
 * @param a2	The second optional parameter.
 * @param a3	The third optional parameter.
 * @param a4	The fourth optional parameter.
 * @param a5	The fifth optional parameter.
 * @param a6	The sixth optional parameter.
 * @param a7	The seventh optional parameter.
 * @param a8	The eighth optional parameter.
 * @param a9	The ninth optional parameter.
 */
#define Warning9(msg,a1,a2,a3,a4,a5,a6,a7,a8,a9)	Warning(__LINE__, __FILE__, msg ,a1, a2, a3, a4, a5, a6, a7, a8, a9)

/**
 * This macro calls the \Ref{Critical} function including the current module name and line number.
 *
 * @param msg	The log message
 */
#define Critical0(msg)								Critical(__LINE__, __FILE__, msg)
/**
 * This macro calls the \Ref{Critical} function including the current module name and line number.
 * This macro expects one optional parameters.
 *
 * @param msg	The log message
 * @param a1	The first optional parameter.
 */
#define Critical1(msg,a1)							Critical(__LINE__, __FILE__, msg ,a1)
/**
 * This macro calls the \Ref{Critical} function including the current module name and line number.
 * This macro expects two optional parameters.
 *
 * @param msg	The log message
 * @param a1	The first optional parameter.
 * @param a2	The second optional parameter.
 */
#define Critical2(msg,a1,a2)						Critical(__LINE__, __FILE__, msg ,a1, a2)
/**
 * This macro calls the \Ref{Critical} function including the current module name and line number.
 * This macro expects three optional parameters.
 *
 * @param msg	The log message
 * @param a1	The first optional parameter.
 * @param a2	The second optional parameter.
 * @param a3	The third optional parameter.
 */
#define Critical3(msg,a1,a2,a3)						Critical(__LINE__, __FILE__, msg ,a1, a2, a3)
/**
 * This macro calls the \Ref{Critical} function including the current module name and line number.
 * This macro expects four optional parameters.
 *
 * @param msg	The log message
 * @param a1	The first optional parameter.
 * @param a2	The second optional parameter.
 * @param a3	The third optional parameter.
 * @param a4	The fourth optional parameter.
 */
#define Critical4(msg,a1,a2,a3,a4)					Critical(__LINE__, __FILE__, msg ,a1, a2, a3, a4)
/**
 * This macro calls the \Ref{Critical} function including the current module name and line number.
 * This macro expects five optional parameters.
 *
 * @param msg	The log message
 * @param a1	The first optional parameter.
 * @param a2	The second optional parameter.
 * @param a3	The third optional parameter.
 * @param a4	The fourth optional parameter.
 * @param a5	The fifth optional parameter.
 */
#define Critical5(msg,a1,a2,a3,a4,a5)				Critical(__LINE__, __FILE__, msg ,a1, a2, a3, a4, a5)
/**
 * This macro calls the \Ref{Critical} function including the current module name and line number.
 * This macro expects six optional parameters.
 *
 * @param msg	The log message
 * @param a1	The first optional parameter.
 * @param a2	The second optional parameter.
 * @param a3	The third optional parameter.
 * @param a4	The fourth optional parameter.
 * @param a5	The fifth optional parameter.
 * @param a6	The sixth optional parameter.
 */
#define Critical6(msg,a1,a2,a3,a4,a5,a6)			Critical(__LINE__, __FILE__, msg ,a1, a2, a3, a4, a5, a6)
/**
 * This macro calls the \Ref{Critical} function including the current module name and line number.
 * This macro expects seven optional parameters.
 *
 * @param msg	The log message
 * @param a1	The first optional parameter.
 * @param a2	The second optional parameter.
 * @param a3	The third optional parameter.
 * @param a4	The fourth optional parameter.
 * @param a5	The fifth optional parameter.
 * @param a6	The sixth optional parameter.
 * @param a7	The seventh optional parameter.
 */
#define Critical7(msg,a1,a2,a3,a4,a5,a6,a7)			Critical(__LINE__, __FILE__, msg ,a1, a2, a3, a4, a5, a6, a7)
/**
 * This macro calls the \Ref{Critical} function including the current module name and line number.
 * This macro expects eight optional parameters.
 *
 * @param msg	The log message
 * @param a1	The first optional parameter.
 * @param a2	The second optional parameter.
 * @param a3	The third optional parameter.
 * @param a4	The fourth optional parameter.
 * @param a5	The fifth optional parameter.
 * @param a6	The sixth optional parameter.
 * @param a7	The seventh optional parameter.
 * @param a8	The eighth optional parameter.
 */
#define Critical8(msg,a1,a2,a3,a4,a5,a6,a7,a8)		Critical(__LINE__, __FILE__, msg ,a1, a2, a3, a4, a5, a6, a7, a8)
/**
 * This macro calls the \Ref{Critical} function including the current module name and line number.
 * This macro expects nine optional parameters.
 *
 * @param msg	The log message
 * @param a1	The first optional parameter.
 * @param a2	The second optional parameter.
 * @param a3	The third optional parameter.
 * @param a4	The fourth optional parameter.
 * @param a5	The fifth optional parameter.
 * @param a6	The sixth optional parameter.
 * @param a7	The seventh optional parameter.
 * @param a8	The eighth optional parameter.
 * @param a9	The ninth optional parameter.
 */
#define Critical9(msg,a1,a2,a3,a4,a5,a6,a7,a8,a9)	Critical(__LINE__, __FILE__, msg ,a1, a2, a3, a4, a5, a6, a7, a8, a9)

/**
 * This macro calls the \Ref{Error} function including the current module name and line number.
 *
 * @param msg	The log message
 */
#define Error0(msg)								Error(__LINE__, __FILE__, msg)
/**
 * This macro calls the \Ref{Error} function including the current module name and line number.
 * This macro expects one optional parameters.
 *
 * @param msg	The log message
 * @param a1	The first optional parameter.
 */
#define Error1(msg,a1)							Error(__LINE__, __FILE__, msg ,a1)
/**
 * This macro calls the \Ref{Error} function including the current module name and line number.
 * This macro expects two optional parameters.
 *
 * @param msg	The log message
 * @param a1	The first optional parameter.
 * @param a2	The second optional parameter.
 */
#define Error2(msg,a1,a2)						Error(__LINE__, __FILE__, msg ,a1, a2)
/**
 * This macro calls the \Ref{Error} function including the current module name and line number.
 * This macro expects three optional parameters.
 *
 * @param msg	The log message
 * @param a1	The first optional parameter.
 * @param a2	The second optional parameter.
 * @param a3	The third optional parameter.
 */
#define Error3(msg,a1,a2,a3)						Error(__LINE__, __FILE__, msg ,a1, a2, a3)
/**
 * This macro calls the \Ref{Error} function including the current module name and line number.
 * This macro expects four optional parameters.
 *
 * @param msg	The log message
 * @param a1	The first optional parameter.
 * @param a2	The second optional parameter.
 * @param a3	The third optional parameter.
 * @param a4	The fourth optional parameter.
 */
#define Error4(msg,a1,a2,a3,a4)					Error(__LINE__, __FILE__, msg ,a1, a2, a3, a4)
/**
 * This macro calls the \Ref{Error} function including the current module name and line number.
 * This macro expects five optional parameters.
 *
 * @param msg	The log message
 * @param a1	The first optional parameter.
 * @param a2	The second optional parameter.
 * @param a3	The third optional parameter.
 * @param a4	The fourth optional parameter.
 * @param a5	The fifth optional parameter.
 */
#define Error5(msg,a1,a2,a3,a4,a5)				Error(__LINE__, __FILE__, msg ,a1, a2, a3, a4, a5)
/**
 * This macro calls the \Ref{Error} function including the current module name and line number.
 * This macro expects six optional parameters.
 *
 * @param msg	The log message
 * @param a1	The first optional parameter.
 * @param a2	The second optional parameter.
 * @param a3	The third optional parameter.
 * @param a4	The fourth optional parameter.
 * @param a5	The fifth optional parameter.
 * @param a6	The sixth optional parameter.
 */
#define Error6(msg,a1,a2,a3,a4,a5,a6)			Error(__LINE__, __FILE__, msg ,a1, a2, a3, a4, a5, a6)
/**
 * This macro calls the \Ref{Error} function including the current module name and line number.
 * This macro expects seven optional parameters.
 *
 * @param msg	The log message
 * @param a1	The first optional parameter.
 * @param a2	The second optional parameter.
 * @param a3	The third optional parameter.
 * @param a4	The fourth optional parameter.
 * @param a5	The fifth optional parameter.
 * @param a6	The sixth optional parameter.
 * @param a7	The seventh optional parameter.
 */
#define Error7(msg,a1,a2,a3,a4,a5,a6,a7)			Error(__LINE__, __FILE__, msg ,a1, a2, a3, a4, a5, a6, a7)
/**
 * This macro calls the \Ref{Error} function including the current module name and line number.
 * This macro expects eight optional parameters.
 *
 * @param msg	The log message
 * @param a1	The first optional parameter.
 * @param a2	The second optional parameter.
 * @param a3	The third optional parameter.
 * @param a4	The fourth optional parameter.
 * @param a5	The fifth optional parameter.
 * @param a6	The sixth optional parameter.
 * @param a7	The seventh optional parameter.
 * @param a8	The eighth optional parameter.
 */
#define Error8(msg,a1,a2,a3,a4,a5,a6,a7,a8)		Error(__LINE__, __FILE__, msg ,a1, a2, a3, a4, a5, a6, a7, a8)
/**
 * This macro calls the \Ref{Error} function including the current module name and line number.
 * This macro expects nine optional parameters.
 *
 * @param msg	The log message
 * @param a1	The first optional parameter.
 * @param a2	The second optional parameter.
 * @param a3	The third optional parameter.
 * @param a4	The fourth optional parameter.
 * @param a5	The fifth optional parameter.
 * @param a6	The sixth optional parameter.
 * @param a7	The seventh optional parameter.
 * @param a8	The eighth optional parameter.
 * @param a9	The ninth optional parameter.
 */
#define Error9(msg,a1,a2,a3,a4,a5,a6,a7,a8,a9)	Error(__LINE__, __FILE__, msg ,a1, a2, a3, a4, a5, a6, a7, a8, a9)

#endif // !defined(__MESSAGELOG_H_20020117)
