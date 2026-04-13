#pragma once

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

namespace RendererImage {
    void renderImage(RMLinesRenderer::ImageBuffer &buffer, const ImageRef &texture,
                     const LayerInfo::ImageInfo &imageInfo, const AdvancedMath::Vector &position,
                     const AdvancedMath::Vector &frameSize,
                     const AdvancedMath::Vector &scale);
}

