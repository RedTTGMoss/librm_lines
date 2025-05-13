#include "common/data_types.h"

#include <memory>
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

Group::Group(const CrdtId nodeId)
    : nodeId(nodeId),
      label(LwwItem<std::string>(CrdtId(0, 0), "")),
      visible(LwwItem<bool>(CrdtId(0, 0), true)) {
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
    return alpha << 24 | blue << 16 | green << 8 | red;
}

Color Color::fromRGBA(const uint32_t *rgbaColor) {
    return {
        static_cast<uint8_t>((*rgbaColor >> 16) & 0xFF),
        static_cast<uint8_t>((*rgbaColor >> 8) & 0xFF),
        static_cast<uint8_t>(*rgbaColor & 0xFF),
        static_cast<uint8_t>((*rgbaColor >> 24) & 0xFF)
    };
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
