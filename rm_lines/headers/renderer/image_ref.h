#pragma once
#include <string>
#include <memory>

struct ImageRef {
    int w, h, channels;
    std::shared_ptr<unsigned char> data;

    static ImageRef load(const char *fileName);

    static ImageRef load(const std::string &fileName) {
        return load(fileName.c_str());
    }
};
