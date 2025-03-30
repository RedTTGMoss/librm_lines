#include "library.h"


EXPORT void setLogger(LogFunc logger) {
    global_logger = logger;
}

void logMessage(const std::string& msg) {
    if (global_logger) {
        global_logger(msg.c_str());
    }
}