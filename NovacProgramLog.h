#pragma once

#include <SpectralEvaluation/Log.h>

class NovacProgramLog : public novac::ILogger {

public:
    NovacProgramLog();

    virtual void Debug(const std::string& message) override;
    virtual void Debug(const novac::LogContext& c, const std::string& message) override;

    virtual void Information(const std::string& message) override;
    virtual void Information(const novac::LogContext& c, const std::string& message) override;

    virtual void Error(const std::string& message) override;
    virtual void Error(const novac::LogContext& c, const std::string& message) override;

};
