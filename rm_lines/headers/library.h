#pragma once
#include <vector>

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
#include <cstdint>
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

// FNV-1a hash algorithm for compile-time string hashing
// Produces better distribution than simple polynomial hashing
constexpr uint32_t hashString(const char *str, const uint32_t hash = 2166136261u) {
    return *str ? hashString(str + 1, (hash ^ static_cast<uint32_t>(*str)) * 16777619u) : hash;
}

std::string bytesToHexStr(const std::vector<uint8_t> &data);

void logDebug(const std::string &msg);

off_t getFileSize(FILE *file);

std::string getStackTrace();
