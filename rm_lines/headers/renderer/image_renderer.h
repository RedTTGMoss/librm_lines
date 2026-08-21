#pragma once
#include <cstdint>
#include <cstddef>

namespace RMLinesRenderer {
    class ImageBuffer;
}

namespace LayerInfo {
    struct ImageInfo;
}

namespace AdvancedMath {
    struct Vector;
}

struct ImageRef;

struct Backdrop {
    const uint8_t *data = nullptr;
    size_t size = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t stride = 0;
};

uint32_t sampleBackdropNearest(const Backdrop &backdrop, const float u, const float v);

namespace RendererImage {
    void renderImage(RMLinesRenderer::ImageBuffer &buffer, const ImageRef &texture,
                     const LayerInfo::ImageInfo &imageInfo, const AdvancedMath::Vector &position,
                     const AdvancedMath::Vector &frameSize,
                     const AdvancedMath::Vector &scale);

    void renderImageError(RMLinesRenderer::ImageBuffer &buffer,
                          const LayerInfo::ImageInfo &imageInfo, const AdvancedMath::Vector &position,
                          const AdvancedMath::Vector &frameSize,
                          const AdvancedMath::Vector &scale);

    void renderBackdrop(RMLinesRenderer::ImageBuffer &buffer, const Backdrop &backdrop,
                        const AdvancedMath::Vector &position,
                        const AdvancedMath::Vector &frameSize,
                        const AdvancedMath::Vector &scale);
}

