#include "renderer/image_renderer.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

#include "advanced/layer_info.h"
#include "advanced/math.h"
#include "renderer/image_ref.h"
#include "renderer/rm_lines_stroker/imagebuffer.h"

namespace {
    struct ImageVertex {
        float x;
        float y;
        float u;
        float v;
    };

    float edgeFunction(const float ax, const float ay, const float bx, const float by, const float cx, const float cy) {
        return (cx - ax) * (by - ay) - (cy - ay) * (bx - ax);
    }

    uint8_t blendChannel(const uint8_t srcChannel, const uint8_t dstChannel,
                         const uint32_t srcAlpha, const uint32_t dstAlpha,
                         const uint32_t outAlpha, const uint32_t invSrcAlpha) {
        const uint64_t numerator = static_cast<uint64_t>(srcChannel) * srcAlpha * 255ULL +
                                   static_cast<uint64_t>(dstChannel) * dstAlpha * invSrcAlpha;
        const uint64_t denominator = static_cast<uint64_t>(outAlpha) * 255ULL;
        return static_cast<uint8_t>(numerator / denominator);
    }

    uint32_t blendOver(const uint32_t dst, const uint32_t src) {
        const uint32_t srcAlpha = (src >> 24) & 0xFF;
        if (srcAlpha == 0) {
            return dst;
        }

        const uint32_t dstAlpha = (dst >> 24) & 0xFF;
        const uint32_t invSrcAlpha = 255 - srcAlpha;
        const uint32_t outAlpha = srcAlpha + (dstAlpha * invSrcAlpha + 127) / 255;
        if (outAlpha == 0) {
            return 0;
        }

        const uint8_t srcR = static_cast<uint8_t>((src >> 16) & 0xFF);
        const uint8_t srcG = static_cast<uint8_t>((src >> 8) & 0xFF);
        const uint8_t srcB = static_cast<uint8_t>(src & 0xFF);
        const uint8_t dstR = static_cast<uint8_t>((dst >> 16) & 0xFF);
        const uint8_t dstG = static_cast<uint8_t>((dst >> 8) & 0xFF);
        const uint8_t dstB = static_cast<uint8_t>(dst & 0xFF);

        const uint8_t outR = blendChannel(srcR, dstR, srcAlpha, dstAlpha, outAlpha, invSrcAlpha);
        const uint8_t outG = blendChannel(srcG, dstG, srcAlpha, dstAlpha, outAlpha, invSrcAlpha);
        const uint8_t outB = blendChannel(srcB, dstB, srcAlpha, dstAlpha, outAlpha, invSrcAlpha);
        return (outAlpha << 24) | (static_cast<uint32_t>(outR) << 16) | (static_cast<uint32_t>(outG) << 8) |
               static_cast<uint32_t>(outB);
    }

    uint32_t sampleTextureNearest(const ImageRef &texture, const float u, const float v) {
        if (!texture.data || texture.w <= 0 || texture.h <= 0) {
            return 0;
        }

        const float clampedU = u < 0.0f ? 0.0f : (u > 1.0f ? 1.0f : u);
        const float clampedV = v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);
        int px = static_cast<int>(clampedU * static_cast<float>(texture.w - 1) + 0.5f);
        int py = static_cast<int>(clampedV * static_cast<float>(texture.h - 1) + 0.5f);
        if (px < 0) px = 0;
        if (py < 0) py = 0;
        if (px >= texture.w) px = texture.w - 1;
        if (py >= texture.h) py = texture.h - 1;

        const auto *pixels = texture.data.get();
        const size_t index = (static_cast<size_t>(py) * static_cast<size_t>(texture.w) + static_cast<size_t>(px)) * 4;
        const uint32_t r = pixels[index + 2];
        const uint32_t g = pixels[index + 1];
        const uint32_t b = pixels[index];
        const uint32_t a = pixels[index + 3];
        return (a << 24) | (r << 16) | (g << 8) | b;
    }

    void blendPixel(RMLinesRenderer::ImageBuffer &buffer, const int x, const int y, const uint32_t src) {
        if (x < 0 || y < 0 || x >= static_cast<int>(buffer.width) || y >= static_cast<int>(buffer.height)) {
            return;
        }

        auto *dst = buffer.scanline(static_cast<size_t>(y)) + x;
        *dst = blendOver(*dst, src);
    }

    void rasterizeTexturedTriangle(RMLinesRenderer::ImageBuffer &buffer, const ImageRef &texture,
                                   const ImageVertex &v0, const ImageVertex &v1, const ImageVertex &v2) {
        const float area = edgeFunction(v0.x, v0.y, v1.x, v1.y, v2.x, v2.y);
        if (std::abs(area) < 0.00001f) {
            return;
        }

        float minXValue = std::min({v0.x, v1.x, v2.x});
        float maxXValue = std::max({v0.x, v1.x, v2.x});
        float minYValue = std::min({v0.y, v1.y, v2.y});
        float maxYValue = std::max({v0.y, v1.y, v2.y});

        const float minXf = std::floor(minXValue);
        const float maxXf = std::ceil(maxXValue);
        const float minYf = std::floor(minYValue);
        const float maxYf = std::ceil(maxYValue);

        const int minX = std::max(0, static_cast<int>(minXf));
        const int maxX = std::min(static_cast<int>(buffer.width) - 1, static_cast<int>(maxXf));
        const int minY = std::max(0, static_cast<int>(minYf));
        const int maxY = std::min(static_cast<int>(buffer.height) - 1, static_cast<int>(maxYf));

        if (minX > maxX || minY > maxY) {
            return;
        }

        const float invArea = 1.0f / area;
        for (int y = minY; y <= maxY; ++y) {
            const float py = static_cast<float>(y) + 0.5f;
            for (int x = minX; x <= maxX; ++x) {
                const float px = static_cast<float>(x) + 0.5f;

                const float w0 = edgeFunction(v1.x, v1.y, v2.x, v2.y, px, py);
                const float w1 = edgeFunction(v2.x, v2.y, v0.x, v0.y, px, py);
                const float w2 = edgeFunction(v0.x, v0.y, v1.x, v1.y, px, py);

                if (!((w0 >= 0.0f && w1 >= 0.0f && w2 >= 0.0f) || (w0 <= 0.0f && w1 <= 0.0f && w2 <= 0.0f))) {
                    continue;
                }

                const float b0 = w0 * invArea;
                const float b1 = w1 * invArea;
                const float b2 = w2 * invArea;
                const float u = b0 * v0.u + b1 * v1.u + b2 * v2.u;
                const float v = b0 * v0.v + b1 * v1.v + b2 * v2.v;
                const uint32_t texel = sampleTextureNearest(texture, u, v);
                if ((texel >> 24) == 0) {
                    continue;
                }

                blendPixel(buffer, x, y, texel);
            }
        }
    }
}

namespace RendererImage {
    void renderImage(RMLinesRenderer::ImageBuffer &buffer, const ImageRef &texture,
                     const LayerInfo::ImageInfo &imageInfo, const AdvancedMath::Vector &position,
                     const AdvancedMath::Vector &frameSize,
                     const AdvancedMath::Vector &scale) {
        if (imageInfo.image.vertices.size() < 16) {
            return;
        }

        constexpr size_t vertexCount = 4;
        ImageVertex vertices[vertexCount]{};
        for (size_t i = 0; i < vertexCount; ++i) {
            const size_t base = i * 4;
            vertices[i].x = (position.x + imageInfo.image.vertices[base] + imageInfo.offsetX + frameSize.x / 2.0f) *
                            scale.x;
            vertices[i].y = (position.y + imageInfo.image.vertices[base + 1] + imageInfo.offsetY) * scale.y;
            vertices[i].u = imageInfo.image.vertices[base + 2];
            vertices[i].v = imageInfo.image.vertices[base + 3];
        }

        for (size_t i = 0; i + 2 < imageInfo.image.indices.size(); i += 3) {
            const auto i0 = imageInfo.image.indices[i];
            const auto i1 = imageInfo.image.indices[i + 1];
            const auto i2 = imageInfo.image.indices[i + 2];
            if (i0 >= vertexCount || i1 >= vertexCount || i2 >= vertexCount) {
                continue;
            }

            rasterizeTexturedTriangle(buffer, texture, vertices[i0], vertices[i1], vertices[i2]);
        }
    }
}

