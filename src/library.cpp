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

#ifdef _WIN32
#include <windows.h>
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")

std::string getStackTrace() {
    void* stack[100];
    unsigned short frames = CaptureStackBackTrace(0, 100, stack, NULL);
    SYMBOL_INFO* symbol = (SYMBOL_INFO*)calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
    symbol->MaxNameLen = 255;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

    std::string stackTrace;
    HANDLE process = GetCurrentProcess();
    SymInitialize(process, NULL, TRUE);

    for (unsigned int i = 0; i < frames; i++) {
        SymFromAddr(process, (DWORD64)(stack[i]), 0, symbol);
        stackTrace += std::to_string(frames - i - 1) + ": " + symbol->Name + "\n";
    }

    free(symbol);
    return stackTrace;
}

#else
#include <execinfo.h>
#include <sstream>

std::string getStackTrace() {
    void *buffer[100];
    int nptrs = backtrace(buffer, 100);
    char **strings = backtrace_symbols(buffer, nptrs);

    std::ostringstream stackTrace;
    for (int i = 0; i < nptrs; i++) {
        stackTrace << i << ": " << strings[i] << "\n";
    }

    free(strings);
    return stackTrace.str();
}
#endif
