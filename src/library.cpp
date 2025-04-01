#include "library.h"
#include <iostream>

EXPORT void setLogger(const LogFunc logger) {
    globalLogger = logger;
}

void logMessage(const std::string& msg) {
    if (globalLogger) {
        globalLogger(msg.c_str());
    }
}
EXPORT void setErrorLogger(const LogFunc errorLogger) {
    globalErrorLogger = errorLogger;
}

void logError(const std::string& msg) {
    if (globalErrorLogger) {
        globalErrorLogger(msg.c_str());
    } else {
        std::cerr << msg << std::endl;
    }
}