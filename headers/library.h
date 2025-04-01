#ifndef LIBRARY_H
#define LIBRARY_H

#ifdef _WIN32
    #define EXPORT extern "C" __declspec(dllexport)
#else
    #define EXPORT extern "C" __attribute__((visibility("default")))
#endif
#include <string>

#define PI 3.14159265358979323846

// Logging function pointer type
typedef void (*LogFunc)(const char*);

// Global log function (can be set externally)
static LogFunc globalLogger = nullptr;

EXPORT void setLogger(LogFunc logger);

void logMessage(const std::string& msg);

// Logging function pointer type
typedef void (*ErrorFunc)(const char*);

// Global log function (can be set externally)
static ErrorFunc globalErrorLogger = nullptr;

EXPORT void setErrorLogger(ErrorFunc errorLogger);

void logError(const std::string& msg);

#endif //LIBRARY_H