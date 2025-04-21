#include "v5/reader.h"

#include <cstring>

bool V5Reader::readHeader() {
    char header[V5_HEADER_SIZE];
    fread(header, V5_HEADER_SIZE, 1, file);

    if (memcmp(header, V5_HEADER, V5_HEADER_SIZE) == 0) {
        logDebug("Found V5 header");
        return true;
    }
    return false;
}

bool V5Reader::readBlockInfo() {
    return false; // Not implemented
}
