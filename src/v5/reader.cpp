#include "v5/reader.h"

#include <cstring>

bool V5Reader::readHeader() {
    if (memcmp(data_, V5_HEADER, V5_HEADER_SIZE) == 0) {
        logMessage("Found V5 header");
        return true;
    }
    logMessage("Couldn't find V5 header");
    return false;
}

bool V5Reader::buildTree() {
    // Dummy implementation
    return false;
}