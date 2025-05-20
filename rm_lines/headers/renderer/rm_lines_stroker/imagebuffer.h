#pragma once

#include "advanced/math.h"
#include <vector>
#include <cstddef>
#include <cstdint>
#include <cassert>
#include <algorithm>
#include <memory>


namespace RMLinesRenderer {
    class ImageBuffer {
    public:
        ImageBuffer() = default;

        void reference(const ImageBuffer &master);

        void allocate(size_t w, size_t h);

        void allocate(AdvancedMath::Vector size);

        void release();

        [[nodiscard]] uint32_t *scanline(size_t y);

        [[nodiscard]] const uint32_t *scanline(size_t y) const;

        void fill(uint32_t value) const;

        void exportRawData(uint32_t *dataPtr, size_t dataSize, bool antialias, int smoothRadius = 1) const;

        size_t width = 0;
        size_t height = 0;

    private:
        std::shared_ptr<std::vector<uint32_t> > data;
    };
}
