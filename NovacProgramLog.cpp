#include "stdafx.h"
#include "NovacProgramLog.h"
#include "Common/Common.h"
#include <sstream>


NovacProgramLog::NovacProgramLog()
{
}


static void Output(const char* level, std::string message)
{
    std::stringstream s;
    s << level << message << std::endl;
    ShowMessage(s.str());
}

static void Output(const char* level, const novac::LogContext& c, std::string message)
{
    std::stringstream s;
    s << level << c << message << std::endl;
    ShowMessage(s.str());
}

void NovacProgramLog::Debug(const std::string& message)
{
    Output("[Debug] ", message);
}
void NovacProgramLog::Debug(const novac::LogContext& c, const std::string& message)
{
    Output("[Debug] ", c, message);
}

void NovacProgramLog::Information(const std::string& message)
{
    Output("", message);
}
void NovacProgramLog::Information(const novac::LogContext& c, const std::string& message)
{
    Output("", c, message);
}

void NovacProgramLog::Error(const std::string& message)
{
    Output("[Error] ", message);
}
void NovacProgramLog::Error(const novac::LogContext& c, const std::string& message)
{
    Output("[Error] ", c, message);
}
