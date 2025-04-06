#ifndef LIBRARY_H
#define LIBRARY_H

#ifdef _WIN32
    #define EXPORT extern "C" __declspec(dllexport)
#else
#define EXPORT extern "C" __attribute__((visibility("default")))
#endif
#include <string>

#define PI 3.14159265358979323846

typedef void (*LogFunc)(const char *); // Function pointer type for logging functions

// Logger
static LogFunc globalLogger = nullptr;
EXPORT void setLogger(LogFunc logger);

void logMessage(const std::string &msg);


// Error logger
static LogFunc globalErrorLogger = nullptr;
EXPORT void setErrorLogger(LogFunc errorLogger);

void logError(const std::string &msg);

// Debug logger
static LogFunc globalDebugLogger = nullptr;
EXPORT void setDebugLogger(LogFunc debugLogger);

void logDebug(const std::string &msg);

#endif //LIBRARY_H
