#include "common/data_types.h"

#include <memory>
#include <sstream>
#include <common/crdt_sequence_item.h>

std::string formatTextItem(TextItem textItem) {
    if (textItem.value.has_value()) {
        if (std::holds_alternative<std::string>(textItem.value.value())) {
            std::string rawString = std::get<std::string>(textItem.value.value());
            std::ostringstream oss;

            // Replace escape sequences with their literal representations
            for (char c: rawString) {
                switch (c) {
                    case '\n': oss << "\\n";
                        break;
                    case '\t': oss << "\\t";
                        break;
                    case '\\': oss << "\\\\";
                        break;
                    case '\"': oss << "\\\"";
                        break;
                    default: oss << c;
                        break;
                }
            }

            return "\"" + oss.str() + "\"";
        } else if (std::holds_alternative<uint32_t>(textItem.value.value())) {
            return std::format("Format: {}", std::get<uint32_t>(textItem.value.value()));
        }
        return "Couldn't format TextItem";
    }
    return "TextItem has no value";
}

std::string reprTextItem(TextItem textItem) {
    auto value = textItem.value.value();
    std::string valueDebug = formatTextItem(textItem);

    return std::format(
        "DEBUG Text item /w python: "
        "CrdtSequenceItem("
        "CrdtId({}, {}), "
        "CrdtId({}, {}), "
        "CrdtId({}, {}), {}, "
        "{}"
        ")", textItem.itemId.first, textItem.itemId.second,
        textItem.leftId.first, textItem.leftId.second,
        textItem.rightId.first, textItem.rightId.second,
        textItem.deletedLength,
        valueDebug
    );
}

std::string CrdtId::repr() const {
    return std::format("CrdtId[{}:{}]", first, second);
}

json CrdtId::toJson() const {
    return std::format("{}:{}", first, second);
}

CrdtId CrdtId::operator++(int) {
    return CrdtId(first, ++second);
}

CrdtId::CrdtId(const char *id) : CrdtId(std::string(id)) {
}

CrdtId::CrdtId(const std::string &string) {
    const auto pos = string.find(':');
    if (pos == std::string::npos) {
        throw std::invalid_argument("Invalid CrdtId format, expected 'first:second'");
    }
    try {
        first = static_cast<uint8_t>(std::stoul(string.substr(0, pos)));
        second = std::stoull(string.substr(pos + 1));
    } catch (const std::exception &e) {
        throw std::invalid_argument(std::format("Failed to parse CrdtId from string '{}': {}", string, e.what()));
    }
}

Group::Group(const CrdtId nodeId)
    : nodeId(nodeId),
      label(LwwItem<std::string>(CrdtId(0, 0), "")),
      visible(LwwItem<bool>(CrdtId{0, 0}, true)) {
}

json textFormatToJson(const TextFormat &textFormat) {
    return {
        {"nodeId", textFormat.first.toJson()},
        {"format", textFormat.second.toJson()}
    };
}

json Color::toJson() const {
    return {alpha, red, green, blue};
}

uint32_t Color::toRGBA() const {
    // Writing to buffer (RGBA)
    return alpha << 24 | blue << 16 | green << 8 | red;
}

Color Color::operator*(const Color &other) const {
    return {
        static_cast<uint8_t>(red / 255.0f * other.red / 255.0f * 255),
        static_cast<uint8_t>(green / 255.0f * other.green / 255.0f * 255),
        static_cast<uint8_t>(blue / 255.0f * other.blue / 255.0f * 255),
        static_cast<uint8_t>(alpha / 255.0f * other.alpha / 255.0f * 255)
    };
}

Color Color::operator*(float other) const {
    return {
        static_cast<uint8_t>(red * other),
        static_cast<uint8_t>(green * other),
        static_cast<uint8_t>(blue * other),
        static_cast<uint8_t>(alpha * other)
    };
}

Color Color::operator+(const Color &other) const {
    return {
        std::min<uint8_t>(red + other.red, 255),
        std::min<uint8_t>(green + other.green, 255),
        std::min<uint8_t>(blue + other.blue, 255),
        std::min<uint8_t>(alpha + other.alpha, 255)
    };
}

Color Color::fromRGBA(const uint32_t *rgbaColor) {
    // Reading from buffer (RGBA)
    return {
        static_cast<uint8_t>(*rgbaColor & 0xFF),
        static_cast<uint8_t>((*rgbaColor >> 8) & 0xFF),
        static_cast<uint8_t>((*rgbaColor >> 16) & 0xFF),
        static_cast<uint8_t>((*rgbaColor >> 24) & 0xFF)
    };
}

void Color::inplaceFromRGBA(const uint32_t *rgbaColor) {
    // Reading from rM file (ARGB)
    red = static_cast<uint8_t>((*rgbaColor >> 16) & 0xFF);
    green = static_cast<uint8_t>((*rgbaColor >> 8) & 0xFF);
    blue = static_cast<uint8_t>(*rgbaColor & 0xFF);
    alpha = static_cast<uint8_t>((*rgbaColor >> 24) & 0xFF);
}

json Group::toJson() const {
    json j = toJsonNoItem();
    j["nodeId"] = nodeId.toJson();
    return j;
}

json Group::toJsonNoItem() const {
    return {
        {"label", label.toJson()},
        {"visible", visible.toJson()},
        {"anchorId", anchorId ? anchorId->toJson() : nullptr},
        {"anchorType", anchorType ? anchorType->toJson() : nullptr},
        {"anchorThreshold", anchorThreshold ? anchorThreshold->toJson() : nullptr},
        {"anchorOriginX", anchorOriginX ? anchorOriginX->toJson() : nullptr}
    };
}

template<>
json LwwItem<ParagraphStyle>::toJson() const {
    return {
        {"characterId", timestamp.toJson()},
        {"value", value}
    };
}

template<>
json LwwItem<CrdtId>::toJson() const {
    return value.toJson();
}
