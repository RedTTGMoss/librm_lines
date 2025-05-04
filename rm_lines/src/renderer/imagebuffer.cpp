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

void ImageBuffer::exportRawData(uint32_t *dataPtr, size_t dataSize) const {
    if (!data) {
        throw std::runtime_error("ImageBuffer not allocated");
    }
    if (!dataPtr) {
        throw std::invalid_argument("Data pointer is null");
    }
    if (dataSize < data->size() * sizeof(uint32_t)) {
        throw std::invalid_argument(std::format("Data pointer size {} is smaller than image size {}", dataSize, data->size() * sizeof(uint32_t)));
    }
    logError(std::format("Exporting {} bytes for size {} [{}x{}]", data->size() * sizeof(uint32_t), data->size(), width, height));
    std::ranges::copy(*data, dataPtr);

}
