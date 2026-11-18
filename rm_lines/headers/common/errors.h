#pragma once
#include <stdexcept>

class GenericError : public std::runtime_error {
public:
    explicit GenericError(const std::string &message)
        : std::runtime_error(message) {
    }

    virtual int getErrorCode() const {
        return -1;
    }
};

class TreeInstability : public GenericError {
public:
    explicit TreeInstability(const std::string &message)
        : GenericError(message) {
    }

    int getErrorCode() const override {
        return -2;
    }
};
