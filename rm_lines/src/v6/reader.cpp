#include "v6/reader.h"

#include <cstring>

bool V6Reader::readHeader() {
    char header[V6_HEADER_SIZE];
    fread(header, V6_HEADER_SIZE, 1, file);

    if (memcmp(header, V6_HEADER, V6_HEADER_SIZE) == 0) {
        logDebug("Found V6 header");
        return true;
    }
    return false;
}

bool V6Reader::buildTree() {
    // Dummy implementation
    return false;
}
