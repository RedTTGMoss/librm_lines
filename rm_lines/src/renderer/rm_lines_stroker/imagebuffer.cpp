#include "renderer/rm_lines_stroker/imagebuffer.h"

#include <format>

#include "library.h"
#include "advanced/math.h"

namespace RMLinesRenderer {
    void ImageBuffer::allocate(const size_t w, const size_t h) {
        assert(!data);
        width = w;
        height = h;
        data = std::make_shared<std::vector<uint32_t> >(width * height);
    }

    void ImageBuffer::allocate(const AdvancedMath::Vector size) {
        assert(!data);
        width = static_cast<uint32_t>(size.x);
        height = static_cast<uint32_t>(size.y);
        data = std::make_shared<std::vector<uint32_t> >(width * height);
    }

    void ImageBuffer::reference(const ImageBuffer &master) {
        assert(!data);
        width = master.width;
        height = master.height;
        data = master.data;
    }

    void ImageBuffer::release() {
        data.reset();
        width = height = 0;
    }

    uint32_t *ImageBuffer::scanline(const size_t y) {
        assert(data && y < height);
        return data->data() + y * width;
    }

    const uint32_t *ImageBuffer::scanline(const size_t y) const {
        assert(data && y < height);
        return data->data() + y * width;
    }

    void ImageBuffer::fill(const uint32_t value) const {
        if (data) {
            std::ranges::fill(*data, value);
        }
    }

    void ImageBuffer::exportRawData(uint32_t *dataPtr, size_t dataSize, bool antialias, int smoothRadius) const {
        if (!data) {
            throw std::runtime_error("ImageBuffer not allocated");
        }
        if (!dataPtr) {
            throw std::invalid_argument("Data pointer is null");
        }
        if (dataSize < data->size() * sizeof(uint32_t)) {
            throw std::invalid_argument(std::format("Data pointer size {} is smaller than image size {}", dataSize,
                                                    data->size() * sizeof(uint32_t)));
        }
        if (!antialias) {
            std::ranges::copy(*data, dataPtr);
        } else {
            std::vector<uint32_t> tmp(data->size());
            for (size_t y = 0; y < height; y++) {
                for (size_t x = 0; x < width; x++) {
                    int rsum = 0, gsum = 0, bsum = 0, asum = 0;
                    int count = 0;
                    // Using smoothRadius to define the neighborhood size
                    for (int dy = -smoothRadius; dy <= smoothRadius; dy++) {
                        size_t ny = y + dy;
                        if (ny >= height) continue;
                        for (int dx = -smoothRadius; dx <= smoothRadius; dx++) {
                            size_t nx = x + dx;
                            if (nx >= width) continue;
                            uint32_t pix = (*data)[ny * width + nx];
                            // Assume format: 0xRRGGBBAA
                            rsum += (pix >> 24) & 0xff;
                            gsum += (pix >> 16) & 0xff;
                            bsum += (pix >> 8) & 0xff;
                            asum += pix & 0xff;
                            count++;
                        }
                    }
                    uint32_t r = static_cast<uint32_t>(rsum / count) & 0xff;
                    uint32_t g = static_cast<uint32_t>(gsum / count) & 0xff;
                    uint32_t b = static_cast<uint32_t>(bsum / count) & 0xff;
                    uint32_t a = static_cast<uint32_t>(asum / count) & 0xff;
                    tmp[y * width + x] = (r << 24) | (g << 16) | (b << 8) | a;
                }
            }
            std::ranges::copy(tmp, dataPtr);
        }
    }
}
