#ifndef LIBRARY_H
#define LIBRARY_H

#ifdef _WIN32
    #define EXPORT extern "C" __declspec(dllexport)
#else
    #define EXPORT extern "C" __attribute__((visibility("default")))
#endif
#include <string>

// Logging function pointer type
typedef void (*LogFunc)(const char*);

// Global log function (can be set externally)
static LogFunc global_logger = nullptr;

EXPORT void setLogger(LogFunc logger);

void logMessage(const std::string& msg);

#endif //LIBRARY_H