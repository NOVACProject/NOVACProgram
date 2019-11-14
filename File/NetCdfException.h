#pragma once

namespace FileHandler
{

class NetCdfException : public std::exception
{
public:
    NetCdfException(const char* message, int statusCode)
        : std::exception(message)
    {
        this->statusCode = statusCode;
    }

    // TODO: format these error codes, such that they are human-readable
    int statusCode = 0;
};

}
