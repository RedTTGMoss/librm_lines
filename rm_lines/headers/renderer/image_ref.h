#pragma once
#include <string>

struct ImageRef {
    int w, h, channels;
    unsigned char *data;

    static ImageRef load(const char *fileName);

    static ImageRef load(const std::string &fileName) {
        return load(fileName.c_str());
    }
};
