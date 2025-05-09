#pragma once

#include <vector>
#include <cstddef>
#include <cstdint>
#include <cassert>
#include <algorithm>
#include <memory>

#include "advanced/rect.h"

namespace RMLinesRenderer {
    class ImageBuffer {
    public:
        ImageBuffer() = default;

        void reference(const ImageBuffer& master);
        void allocate(size_t w, size_t h);

        void allocate(Vector size);

        void release();

        [[nodiscard]] uint32_t* scanline(size_t y);
        [[nodiscard]] const uint32_t* scanline(size_t y) const;
        void fill(uint32_t value) const;

        void exportRawData(uint32_t *dataPtr, size_t dataSize) const;

        size_t width = 0;
        size_t height = 0;

    private:
        std::shared_ptr<std::vector<uint32_t>> data;
    };
}