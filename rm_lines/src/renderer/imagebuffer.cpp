#include "renderer/imagebuffer.h"

#include <format>

#include "library.h"
#include "advanced/rect.h"

void ImageBuffer::allocate(const size_t w, const size_t h) {
    assert(!data);
    width = w;
    height = h;
    data = std::make_shared<std::vector<uint32_t>>(width * height);
}
void ImageBuffer::allocate(const Vector size) {
    assert(!data);
    width = size.x;
    height = size.y;
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

void ImageBuffer::exportRawData(uint32_t **outData, size_t *outSize) const {
    if (!data) {
        throw std::runtime_error("ImageBuffer not allocated");
    }
    if (!outData || !outSize) {
        throw std::invalid_argument("Output data or size pointer is null");
    }
    logError(std::format("Exporting {} bytes for size {} [{}x{}]", data->size() * sizeof(uint32_t), data->size(), width, height));
    *outSize = data->size() * sizeof(uint32_t);
    *outData = new uint32_t[data->size()];
    std::ranges::copy(*data, *outData);

}
