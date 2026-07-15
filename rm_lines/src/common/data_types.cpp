#include "common/data_types.h"

#include <memory>
#include <sstream>
#include <common/crdt_sequence_item.h>

#include "advanced/text.h"
#include "advanced/text_scale.h"

std::string formatTextItem(const TextItem &textItem) {
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

std::string reprTextItem(const TextItem &textItem) {
    auto value = textItem.value.value();
    std::string valueDebug = formatTextItem(textItem);

    return std::format(
        "DEBUG Text item:"
        "ItemId({}, {}), "
        "LeftId({}, {}), "
        "RightId({}, {}), "
        "DeletedLength = {}, "
        "value = {}",
        textItem.itemId.first, textItem.itemId.second,
        textItem.leftId.first, textItem.leftId.second,
        textItem.rightId.first, textItem.rightId.second,
        textItem.deletedLength,
        valueDebug
    );
}

std::strong_ordering CrdtId::operator<=>(const CrdtId &other) const {
    return std::tie(first, second) <=> std::tie(other.first, other.second);
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

CrdtId CrdtId::operator+(const int i) const {
    return CrdtId(first, second + i);
}

CrdtId CrdtId::operator-(const int i) const {
    return CrdtId(first, second - i);
}

CrdtId &CrdtId::operator+=(const int i) {
    second += i;
    return *this;
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
    : nodeId(nodeId), parentId(CrdtId{0, 0}),
      label(LwwItem<std::string>(CrdtId(0, 0), "")),
      visible(LwwItem(CrdtId{0, 0}, true)) {
}

Group::Group(const CrdtId nodeId, const CrdtId parentId)
    : nodeId(nodeId), parentId(parentId),
      label(LwwItem<std::string>(CrdtId(0, 0), "")),
      visible(LwwItem(CrdtId{0, 0}, true)) {
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

int ParagraphStyleNew::tabbed() const {
    if (baseStyle == 3) {
        switch (legacy) {
            case BulletTab:
                return styleProperties;
            case CheckBoxTab:
            case CheckBoxTabChecked:
                return styleProperties - 16;
            case NumberedTab:
                return styleProperties - 32;
            default:
                return 0; // Not tabbed
        }
    }
    return 0; // Not tabbed
}

float ParagraphStyleNew::styleHeight(const ParagraphStyle against) const {
    return getStyleHeight(against, getStyle());
}

float ParagraphStyleNew::styleMargin() const {
    return getStyleMargin(getStyle());
}

float ParagraphStyleNew::fontSize() const {
    return getFontSize(getStyle());
}

float ParagraphStyleNew::getTabOffset() const {
    // TODO: Might include to include additional tab offset for some styles
    // These styles might account for check marks since they use a graphic
    // The dotted and numbered styles can be integrated with special glyph adding code
    return TAB_LENGTH * tabbed();
}

std::string ParagraphStyleNew::styleLabel() const {
    switch (getStyle()) {
        case MISSING:
            return "MISSING";
        case BASIC:
            return "BASIC";
        case PlainText:
            return "PlainText";
        case Title:
            return "Title";
        case Sub:
            return "Sub";
        case Bullet:
            return "Bullet";
        case BulletTab:
            return "BulletTab";
        case CheckBox:
            return "CheckBox";
        case CheckBoxChecked:
            return "CheckBoxChecked";
        case CheckBoxTab:
            return "CheckBoxTab";
        case CheckBoxTabChecked:
            return "CheckBoxTabChecked";
        case Numbered:
            return "Numbered";
        case NumberedTab:
            return "NumberedTab";
        default:
            return "UNKNOWN";
    }
    return "UNKNOWN";
}

std::string Color::repr() const {
    return std::format("Color(r={}, g={}, b={}, a={})", red, green, blue, alpha);
}

uint32_t Color::toRGBA() const {
    // Writing to buffer (ABGR)
    return alpha << 24 | blue << 16 | green << 8 | red;
}

uint32_t Color::toARGB() const {
    // Writing to file (ARGB)
    return alpha << 24 | red << 16 | green << 8 | blue;
}

Color Color::operator*(const Color &other) const {
    return Color(
        static_cast<uint8_t>(red / 255.0f * other.red / 255.0f * 255),
        static_cast<uint8_t>(green / 255.0f * other.green / 255.0f * 255),
        static_cast<uint8_t>(blue / 255.0f * other.blue / 255.0f * 255),
        static_cast<uint8_t>(alpha / 255.0f * other.alpha / 255.0f * 255)
    );
}

Color Color::operator*(float other) const {
    return Color(
        static_cast<uint8_t>(red * other),
        static_cast<uint8_t>(green * other),
        static_cast<uint8_t>(blue * other),
        static_cast<uint8_t>(alpha * other)
    );
}

Color Color::operator+(const Color &other) const {
    return Color(
        std::min<uint8_t>(red + other.red, 255),
        std::min<uint8_t>(green + other.green, 255),
        std::min<uint8_t>(blue + other.blue, 255),
        std::min<uint8_t>(alpha + other.alpha, 255)
    );
}


Color Color::fromRGBA(const uint32_t *rgbaColor) {
    // Reading from buffer (RGBA)
    return Color(
        static_cast<uint8_t>(*rgbaColor & 0xFF),
        static_cast<uint8_t>((*rgbaColor >> 8) & 0xFF),
        static_cast<uint8_t>((*rgbaColor >> 16) & 0xFF),
        static_cast<uint8_t>((*rgbaColor >> 24) & 0xFF)
    );
}

Color Color::fromARGB(const uint32_t *argbColor) {
    // Reading from buffer (RGBA)
    return Color(
        static_cast<uint8_t>((*argbColor >> 16) & 0xFF),
        static_cast<uint8_t>((*argbColor >> 8) & 0xFF),
        static_cast<uint8_t>((*argbColor) & 0xFF),
        static_cast<uint8_t>((*argbColor >> 24) & 0xFF)
    );
}

Color Color::random() {
    Color c;
    c.red = static_cast<uint8_t>(rand() % 255);
    c.green = static_cast<uint8_t>(rand() % 255);
    c.blue = static_cast<uint8_t>(rand() % 255);
    c.alpha = 255;
    return c;
}

void Color::inplaceFromARGB(const uint32_t *rgbaColor) {
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
    std::string parentIsStr;
    switch (parentIs) {
        case TREE:
            parentIsStr = "TREE";
            break;
        case NODE:
            parentIsStr = "NODE";
            break;
        case TEXT:
            parentIsStr = "TEXT";
            break;
        default:
            parentIsStr = "UNKNOWN";
    }
    return {
        {"label", label.toJson()},
        {"visible", visible.toJson()},
        {"parentId", parentId.toJson()},
        {"parentIs", parentIsStr},
        {"anchorId", anchorId ? anchorId->toJson() : nullptr},
        {"anchorType", anchorType ? anchorType->toJson() : nullptr},
        {"anchorThreshold", anchorThreshold ? anchorThreshold->toJson() : nullptr},
        {"anchorOriginX", anchorOriginX ? anchorOriginX->toJson() : nullptr}
    };
}

json ParagraphStyleNew::toJson() const {
    return {
        {"legacyStyle", legacy},
        {"baseStyle", baseStyle},
        {"styleProperties", styleProperties},
        {"isLegacy", isLegacy},
        {"tabbed", tabbed()},
        {"tabOffset", getTabOffset()},
        {"_styleLabel", styleLabel()},

        {"_styleHeight", styleHeight()},
        {"_fontSize", fontSize()},
    };
}

FontType ParagraphStyleNew::getFont() const {
    switch (getStyle()) {
        case Title:
            return Serif;
        default:
            return Sans;
    }
    return Sans;
}

ParagraphStyle ParagraphStyleNew::getStyle() const {
    // TODO: Support Header style sub style (which uses serif)
    // T (T) T t . . .
    // ^ Style marker on tablet ^
    return legacy;
}

template<>
json LwwItem<ParagraphStyleNew>::valueToJson() const {
    return value.toJson();
}

template<>
json LwwItem<CrdtId>::valueToJson() const {
    return value.toJson();
}

template<>
json LwwItem<RectPair>::valueToJson() const {
    return {
        {"x", value.x},
        {"y", value.y},
        {"w", value.w},
        {"h", value.h}
    };
}

template<>
json LwwItem<DoublePair>::valueToJson() const {
    return {
        {"first", value.first},
        {"second", value.second}
    };
}

template<>
json LwwItem<IntPair>::valueToJson() const {
    return {
        {"first", value.first},
        {"second", value.second}
    };
}
