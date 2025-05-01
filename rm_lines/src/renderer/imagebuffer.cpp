#include "renderer/imagebuffer.h"

void ImageBuffer::allocate(const size_t w, const size_t h) {
    assert(!data);
    width = w;
    height = h;
    data = std::make_shared<std::vector<uint32_t>>(width * height);
}

void ImageBuffer::reference(const ImageBuffer& master) {
    assert(!data);
    width = master.width;
    height = master.height;
    data = master.data;
}

void ImageBuffer::release() {
    data.reset();
    width = height = 0;
}

uint32_t* ImageBuffer::scanline(const size_t y) {
    assert(data && y < height);
    return data->data() + y * width;
}

const uint32_t* ImageBuffer::scanline(const size_t y) const {
    assert(data && y < height);
    return data->data() + y * width;
}

void ImageBuffer::fill(const uint32_t value) const {
    if (data) {
        std::ranges::fill(*data, value);
    }
}