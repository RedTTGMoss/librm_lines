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

json CrdtId::asJson() const {
    return {
        first, second,
    };
}

Group::Group(const CrdtId nodeId)
    : nodeId(nodeId),
      label(LwwItem<std::string>(CrdtId(0, 0), "")),
      visible(LwwItem<bool>(CrdtId(0, 0), true))
{}
