#include "v6/reader.h"

#include <cstring>

bool V6Reader::readHeader() {
    if (memcmp(data_, V6_HEADER, V6_HEADER_SIZE) == 0) {
        logMessage("Found V6 header");
        return true;
    }
    logMessage("Couldn't find V6 header");
    return false;
}

bool V6Reader::buildTree() {
    // Dummy implementation
    return false;
}