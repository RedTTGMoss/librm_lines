#include "common/data_types.h"

#include <memory>
#include <common/crdt_sequence_item.h>

std::string formatTextItem(TextItem textItem) {
    if (textItem.value.has_value()) {
        if (std::holds_alternative<std::string>(textItem.value.value())) {
            return std::get<std::string>(textItem.value.value());
        } else if (std::holds_alternative<uint32_t>(textItem.value.value())) {
            return std::format("Format: {}", std::get<uint32_t>(textItem.value.value()));
        }
        return "Couldn't format TextItem";
    }
    return "TextItem has no value";
}

std::string CrdtId::repr() const {
    return std::format("CrdtId[{}:{}]", first, second);
}

json CrdtId::toJson() const {
    return std::format("{}:{}", first, second);
}

Group::Group(const CrdtId nodeId)
    : nodeId(nodeId),
      label(LwwItem<std::string>(CrdtId(0, 0), "")),
      visible(LwwItem<bool>(CrdtId(0, 0), true))
{}

json textFormatToJson(const TextFormat &textFormat) {
    return {
        {"nodeId", textFormat.first.toJson()},
        {"format", textFormat.second.toJson()}
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

template <>
json LwwItem<ParagraphStyle>::toJson() const {
    return {
                {"characterId", itemId.toJson()},
                {"value", value}
    };
}

template <>
json LwwItem<CrdtId>::toJson() const {
    return value.toJson();
}
