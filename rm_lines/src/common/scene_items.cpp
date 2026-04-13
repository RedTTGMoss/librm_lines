#include <format>
#include <library.h>
#include <common/scene_items.h>
#include <reader/tagged_block_reader.h>
#include <common/blocks.h>

#include "writer/tagged_block_writer.h"

size_t getPointSizeSerialized(uint8_t version) {
    switch (version) {
        case 1:
            return 0x18;
        case 2:
            return 0x0E;
        default:
            logError(std::format("Unknown line version {}", version));
            return -1;
    }
}

bool Line::read(TaggedBlockReader *reader, const uint8_t version) {
    uint32_t toolId;
    uint32_t colorId;
    if (!reader->readInt(1, &toolId)) return false;
    if (!reader->readInt(2, &colorId)) return false;
    tool = static_cast<PenTool>(toolId);
    color = static_cast<PenColor>(colorId);
    if (!reader->readDouble(3, &thicknessScale)) return false;
    if (!reader->readFloat(4, &startingLength)) return false;

    SubBlockInfo subBlockInfo;
    reader->getTag();
    if (!reader->readSubBlock(5, subBlockInfo)) return false;
    size_t pointSizeSerialized = getPointSizeSerialized(version);
    if (subBlockInfo.size % pointSizeSerialized != 0) {
        logError(std::format("Point data size mismatch: {} is not multiple of {}", subBlockInfo.size,
                             pointSizeSerialized));
        return false;
    }

    int pointCount = subBlockInfo.size / pointSizeSerialized;
    points = std::vector<Point>(pointCount);

    for (int i = 0; i < pointCount; i++) {
        Point &point = points[i];
        if (!point.read(reader, version)) {
            logError(std::format("Failed to read point {}", i));
            return false;
        }
    }

    if (!reader->readId(6, &timestamp)) return false;

    // Optionally read moveId
    if (reader->remainingBytes() < 3) return true; // No more data to read
    reader->getTag();
    if (reader->checkTag(7, TagType::ID)) {
        CrdtId _moveId;
        if (!reader->readId(&_moveId)) return false;
        moveId = _moveId;
    }

    if (reader->remainingBytes() < 4) return true; // No more data to read
    reader->getTag();

    // Optionally read argbColor, this is only for new highlighters
    if (color == ARGB && reader->checkTag(8, TagType::Byte4)) {
        Color _argbColor;
        if (!reader->readColor(&_argbColor)) return false;
        argbColor = _argbColor;
    }
    this->version = version;
    return true;
}

bool Line::write(TaggedBlockWriter *writer) const {
    const uint32_t toolId = static_cast<uint32_t>(tool);
    const uint32_t colorId = static_cast<uint32_t>(color);
    if (!writer->writeInt(1, &toolId)) return false;
    if (!writer->writeInt(2, &colorId)) return false;
    if (!writer->writeDouble(3, &thicknessScale)) return false;
    if (!writer->writeFloat(4, &startingLength)) return false;

    // The points
    uint32_t subBlockStart;
    if (subBlockStart = writer->writeSubBlockStart(5); subBlockStart == 0) return false;
    for (const auto &point: points) {
        point.write(writer, version);
    }
    if (!writer->writeSubBlockEnd(subBlockStart)) return false;

    // Write timestamp
    if (!writer->writeId(6, &timestamp)) return false;

    // if we have moveId, write it
    if (moveId.has_value()) {
        if (!writer->writeId(7, &moveId.value())) return false;
    }

    // if we have argb color, write it
    if (argbColor.has_value()) {
        if (!writer->writeColor(8, &argbColor.value())) return false;
    }

    return true;
}

json Line::toJson() const {
    std::vector<json> pointsJson;
    for (auto &point: points) {
        pointsJson.push_back(point.toJson());
    }

    return {
        {"tool", tool},
        {"color", color},
        {"thicknessScale", thicknessScale},
        {"startingLength", startingLength},
        {"points", pointsJson},
        {"timestamp", timestamp.toJson()},
        {"moveId", moveId.has_value() ? moveId->toJson() : nullptr},
        {"argbColor", argbColor.has_value() ? argbColor->toJson() : nullptr}
    };
}

json Text::toJson() const {
    std::vector<json> stylesJson;
    for (const auto &style: styles) {
        stylesJson.push_back(textFormatToJson(style));
    }
    return {
        {"items", items.toJson()},
        {"styles", stylesJson},
        {"posX", posX},
        {"posY", posY},
        {"width", width.toJson()},
    };
}

bool Point::read(TaggedBlockReader *reader, uint8_t version) {
    if (!reader->readFloat(&x)) return false;
    if (!reader->readFloat(&y)) return false;
    if (version == 1) {
        float _speed;
        if (!reader->readFloat(&_speed)) return false;
        speed = _speed * 4;

        float _direction;
        if (!reader->readFloat(&_direction)) return false;
        direction = 255.0 * _direction / (PI * 2);

        float _width;
        if (!reader->readFloat(&_width)) return false;
        width = _width * 4;

        float _pressure;
        if (!reader->readFloat(&_pressure)) return false;
        pressure = _pressure * 255;
    } else if (version == 2) {
        uint16_t _speed;
        if (!reader->readBytes(sizeof(uint16_t), &_speed)) return false;
        // ReSharper disable once CppLocalVariableMightNotBeInitialized
        speed = _speed;

        uint16_t _width;
        if (!reader->readBytes(sizeof(uint16_t), &_width)) return false;
        // ReSharper disable once CppLocalVariableMightNotBeInitialized
        width = _width;

        uint8_t _direction;
        if (!reader->readBytes(sizeof(uint8_t), &_direction)) return false;
        // ReSharper disable once CppLocalVariableMightNotBeInitialized
        direction = _direction;

        uint8_t _pressure;
        if (!reader->readBytes(sizeof(uint8_t), &_pressure)) return false;
        // ReSharper disable once CppLocalVariableMightNotBeInitialized
        pressure = _pressure;
    } else {
        logError(std::format("Unknown line version {}", version));
        return false;
    }
    return true;
}

bool Point::write(TaggedBlockWriter *writer, const uint8_t version) const {
    if (!writer->writeFloat(&x)) return false;
    if (!writer->writeFloat(&y)) return false;
    if (version == 1) {
        // Old style, values are in different formats
        const float _speed = speed / 4.0;
        if (!writer->writeFloat(&_speed)) return false;

        const float _direction = direction * (PI * 2) / 255.0;
        if (!writer->writeFloat(&_direction)) return false;

        const float _width = width / 4.0;
        if (!writer->writeFloat(&_width)) return false;

        const float _pressure = pressure / 255.0;
        if (!writer->writeFloat(&_pressure)) return false;
    } else if (version == 2) {
        // ReSharper disable once CppTooWideScopeInitStatement
        const uint16_t _speed = speed;
        if (!writer->writeObj(_speed)) return false;
        // ReSharper disable once CppTooWideScopeInitStatement
        const uint16_t _width = width;
        if (!writer->writeObj(_width)) return false;

        // ReSharper disable once CppTooWideScopeInitStatement
        const uint8_t _direction = direction;
        if (!writer->writeObj(_direction)) return false;

        // ReSharper disable once CppTooWideScopeInitStatement
        const uint8_t _pressure = pressure;
        if (!writer->writeObj(_pressure)) return false;
    }
    return true;
}

json Point::toJson() const {
    return {
        {"x", x},
        {"y", y},
        {"speed", speed},
        {"direction", direction},
        {"width", width},
        {"pressure", pressure}
    };
}

bool GlyphRange::read(TaggedBlockReader *reader) {
    // Optionally read start
    reader->getTag();
    if (reader->checkTag(2, TagType::Byte4)) {
        reader->claimTag();
        uint32_t _start;
        if (!reader->readInt(&_start)) return false;
        start = _start;
    }

    // Optionally read length
    reader->getTag();
    if (reader->checkTag(3, TagType::Byte4)) {
        reader->claimTag();
        uint32_t _length;
        if (!reader->readInt(&_length)) return false;
        length = _length;
    }

    uint32_t colorId;
    if (!reader->readInt(4, &colorId)) return false;
    color = static_cast<PenColor>(colorId);

    if (!reader->readString(5, &text)) return false;

    reader->getTag();
    if (!reader->readSubBlock(6)) return false; // Rects

    uint64_t numberOfRects;
    if (!reader->readValuint(numberOfRects)) return false;

    rects = std::vector<Rect>(numberOfRects);

    for (uint64_t i = 0; i < numberOfRects; i++) {
        double rect[4];
        if (!reader->readBytes(sizeof(double) * 4, rect)) return false;
        rects[i].x = rect[0];
        rects[i].y = rect[1];
        rects[i].w = rect[2];
        rects[i].h = rect[3];
    }
    int colorIndex = 8;

    reader->getTag();
    if (reader->checkTag(7, TagType::ID)) {
        if (!reader->readId(7, &firstId)) return false;
        colorIndex = 8;
    }
    reader->getTag();
    if (reader->checkTag(8, TagType::ID)) {
        if (!reader->readId(8, &lastId)) return false;
        colorIndex = 9;
    }
    reader->getTag();
    if (reader->checkTag(9, TagType::Byte1)) {
        if (!reader->readBool(9, &includeLastId)) return false;
        colorIndex = 10;
    }

    // Optionally read argbColor, this is only for new highlighters
    reader->getTag();
    if (color == ARGB && reader->checkTag(colorIndex, TagType::Byte4)) {
        Color _argbColor;
        if (!reader->readColor(colorIndex, &_argbColor)) return false;
        argbColor = _argbColor;
    }

    return true;
}

bool GlyphRange::write(TaggedBlockWriter *writer) const {
    // Write optional start
    if (start.has_value()) {
        if (!writer->writeInt(2, &start.value())) return false;
    }

    // Write optional length
    if (length.has_value()) {
        if (!writer->writeInt(3, &length.value())) return false;
    }

    // Write color and text
    const uint32_t colorId = static_cast<uint32_t>(color);
    if (!writer->writeInt(4, &colorId)) return false;
    if (!writer->writeString(5, &text)) return false;

    // Write rects
    uint32_t subBlockStart;
    if (subBlockStart = writer->writeSubBlockStart(6); subBlockStart == 0) return false;

    const uint64_t numberOfRects = rects.size();
    if (!writer->writeValuint(numberOfRects)) return false;

    for (const auto &rect: rects) {
        double rectData[4] = {rect.x, rect.y, rect.w, rect.h};
        if (!writer->writeBytes(sizeof(rectData), rectData)) return false;
    }
    if (!writer->writeSubBlockEnd(subBlockStart)) return false;

    // Write the rest of the text data
    int colorIndex = 8;
    if (firstId != END_MARKER) {
        if (!writer->writeId(7, &firstId)) return false;
        if (lastId != END_MARKER) {
            if (!writer->writeId(8, &lastId)) return false;
        }
        if (!writer->writeBool(9, includeLastId)) return false;
        colorIndex = 10;
    }

    // Write argbColor if needed
    if (color == ARGB) {
        if (!writer->writeColor(colorIndex, &argbColor)) return false;
    }

    return true;
}

json GlyphRange::toJson() const {
    std::vector<json> rectsJson;
    for (const auto &rect: rects) {
        rectsJson.push_back(rect.toJson());
    }
    return {
        {"start", start},
        {"length", length},
        {"color", color},
        {"argbColor", argbColor.toJson()},
        {"text", text},
        {"rects", rectsJson}
    };
}

json Image::toJson() const {
    return {
        {"imageRef", imageRef.toJson()},
        {"boundsTimestamp", boundsTimestamp.toJson()},
        {"vertices", vertices},
        {"indices", indices}
    };
}
