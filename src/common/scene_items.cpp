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

    if (reader->remainingBytes() > 3) {
        CrdtId _moveId;
        if (!reader->readId(7, &_moveId)) return false;
        moveId = _moveId;
    }

    return true;
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

