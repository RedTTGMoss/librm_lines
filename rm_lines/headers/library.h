#pragma once

// Export macro
#ifdef _WIN32
    #define EXPORT extern "C" __declspec(dllexport)
#else
#define EXPORT extern "C" __attribute__((visibility("default")))
#endif
// Branch prediction macros
#if defined(_MSC_VER)
#define IS_LIKELY(x)      (x)
#define IS_UNLIKELY(x)    (x)
#else
#define IS_LIKELY(x)      __builtin_expect(!!(x), 1)
#define IS_UNLIKELY(x)    __builtin_expect(!!(x), 0)
#endif
#include <cstdio>
#include <string>
// ReSharper disable once CppUnusedIncludeDirective
#include <fcntl.h>

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

// Debug mode
static bool debugMode = false;
EXPORT void setDebugMode(bool debug);
EXPORT bool getDebugMode();

void logDebug(const std::string &msg);

off_t getFileSize(FILE *file);

std::string getStackTrace();
