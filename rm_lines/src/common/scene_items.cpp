#include <format>
#include <library.h>
#include <common/scene_items.h>
#include <reader/tagged_block_reader.h>
#include <common/blocks.h>

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

bool Line::read(TaggedBlockReader *reader, uint8_t version) {
    if (!reader->readInt(1, &toolId)) return false;
    if (!reader->readInt(2, &colorId)) return false;
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
    if (color == HIGHLIGHT && reader->checkTag(8, TagType::Byte4)) {
        Color _argbColor;
        if (!reader->readColor(&_argbColor)) return false;
        argbColor = _argbColor;
    }

    return true;
}

json Line::toJson() const {
    return {
        {"toolId", toolId},
        {"colorId", colorId},
        {"color", color},
        {"thicknessScale", thicknessScale},
        {"startingLength", startingLength},
        // {"points", items.toJson()},
        {"timestamp", timestamp.toJson()},
        {"moveId", moveId.has_value() ? moveId->toJson() : nullptr},
        // {"argbColor", argbColor.has_value() ? argbColor->toJson() : nullptr}
    };
}

json Text::toJson() {
    std::vector<json> stylesJson;
    for (const auto &style : styles) {
        stylesJson.push_back(textFormatToJson(style));
    }
    return {
        {"items", items.toJson()},
        {"styles", stylesJson},
        {"posX", posX},
        {"posY", posY},
        {"width", width},
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
        direction = 255 * _direction / (PI * 2);

        float _width;
        if (!reader->readFloat(&_width)) return false;
        width = _width * 4;

        float _pressure;
        if (!reader->readFloat(&_pressure)) return false;
        pressure = _pressure * 255;
    } else if (version == 2) {
        uint16_t _speed;
        if (!reader->readBytes(sizeof(uint16_t), &speed)) return false;
        // ReSharper disable once CppLocalVariableMightNotBeInitialized
        speed = _speed;

        uint16_t _width;
        if (!reader->readBytes(sizeof(uint16_t), &width)) return false;
        // ReSharper disable once CppLocalVariableMightNotBeInitialized
        width = _width;

        uint8_t _direction;
        if (!reader->readBytes(sizeof(uint8_t), &direction)) return false;
        // ReSharper disable once CppLocalVariableMightNotBeInitialized
        direction = _direction;

        uint8_t _pressure;
        if (!reader->readBytes(sizeof(uint8_t), &pressure)) return false;
        // ReSharper disable once CppLocalVariableMightNotBeInitialized
        pressure = _pressure;
    } else {
        logError(std::format("Unknown line version {}", version));
        return false;
    }
    return true;
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

    // If we didn't get the length, set it to the length of the text
    if (!length.has_value()) {
        length = text.length();
    }

    reader->getTag();
    if (!reader->readSubBlock(6)) return false; // Rects

    uint64_t numberOfRects;
    if (!reader->readValuint(numberOfRects)) return false;

    rects = std::vector<Rect>(numberOfRects);

    for (uint64_t i = 0; i < numberOfRects; i++) {
        if (!reader->readBytes(sizeof(Rect), &rects[i])) return false;
    }

    // Optionally read argbColor, this is only for new highlighters
    reader->getTag();
    if (color == HIGHLIGHT && reader->checkTag(8, TagType::Byte4)) {
        Color _argbColor;
        if (!reader->readColor(8, &_argbColor)) return false;
        argbColor = _argbColor;
    }
    if (reader->hasBytesRemaining()) {
        // TODO: Read unknown
        logError("Unknown bytes remaining in GlyphRange");
        // Skip for now...
        reader->seekTo(reader->currentBlockInfo.offset + reader->currentBlockInfo.size);
    }

    return true;
}