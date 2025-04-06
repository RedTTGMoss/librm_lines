#include "v5/reader.h"

#include <cstring>

bool V5Reader::readHeader() {
    if (memcmp(data_, V5_HEADER, V5_HEADER_SIZE) == 0) {
        logDebug("Found V5 header");
        return true;
    }
    return false;
}

bool V5Reader::readBlockInfo() {
    return false; // Not implemented
}
