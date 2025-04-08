#include "library.h"
#include <iostream>

// Logger
EXPORT void setLogger(const LogFunc logger) {
    globalLogger = logger;
}

void logMessage(const std::string &msg) {
    if (globalLogger) {
        globalLogger(msg.c_str());
    }
}

// Error logger
EXPORT void setErrorLogger(const LogFunc errorLogger) {
    globalErrorLogger = errorLogger;
}

void logError(const std::string &msg) {
    if (globalErrorLogger) {
        globalErrorLogger(msg.c_str());
    } else {
        std::cerr << msg << std::endl;
    }
}

// Debug logger
EXPORT void setDebugLogger(const LogFunc debugLogger) {
    globalDebugLogger = debugLogger;
}

void logDebug(const std::string &msg) {
    if (globalDebugLogger) {
        globalDebugLogger(msg.c_str());
    }
}

off_t getFileSize(const int fd) {
    struct stat st;
    if (fstat(fd, &st) == -1) return -1;
    return st.st_size;
}