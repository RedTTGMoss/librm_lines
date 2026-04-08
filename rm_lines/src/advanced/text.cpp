#include "advanced/text.h"
#include "common/scene_items.h"

bool checkString(const TextItem &item, const std::string &str) {
    if (!item.value.has_value())
        return false;
    if (!std::holds_alternative<std::string>(item.value.value()))
        return false;
    return std::get<std::string>(item.value.value()) == str;
}

bool operator==(const std::string &str, const char rhs) {
    return str.size() == 1 && *str.begin() == rhs;
}

json TextFormattingOptions::toJson() const {
    return {
        {"bold", bold},
        {"italic", italic},
        {"deletedLength", deletedLength}
    };
}

json FormattedText::toJson() const {
    json j;
    j["text"] = text;
    j["formatting"] = formatting.toJson();

    return j;
}

std::string Paragraph::repr() const {
    std::string final = "";
    switch (style.value.legacy) {
        case BASIC:
            // I can't find this being used for any of the texts?
            break;
        case PlainText:
            // This is paragraph text
            break;
        case Title:
            final += "# ";
            break;
        case Sub:
            final += "## ";
            break;
        case Bullet:
            final += "- ";
            break;
        case BulletTab:
            // I can't find this being used anywhere too, but it's some variant or version of the bullet style
            final += "-- ";
            break;
        case CheckBox:
            final += "☐ ";
            break;
        case CheckBoxChecked:
            final += "☑ ";
            break;
        default:
            break;
    };
    for (const auto &text: contents) {
        final += text.text;
    }
    return final + "\n";
}

json Paragraph::toJson() const {
    json j;
    j["startId"] = startId.toJson();
    j["style"] = style.value.toJson();
    j["contents"] = json::array();
    for (const auto &text: contents) {
        j["contents"].push_back(text.toJson());
    }

    return j;
}

void TextDocument::fromText(const std::shared_ptr<Text> &_text) {
    text = _text;
    text->items.expandTextItems();
    paragraphs.clear();
    const auto characterIDs = text->items.getSortedIds();

    TextFormattingOptions formatting;

    int i = 0;

    while (i < characterIDs.size()) {
        // Initiate a new paragraph
        Paragraph paragraph;

        if (checkString(text->items[characterIDs[i]], "\n")) {
            paragraph.startId = characterIDs[i];
            i++;
        } else {
            paragraph.startId = END_MARKER;
        }

        FormattedText currentText;
        currentText.formatting = formatting;
        while (i < characterIDs.size()) {
            if (
                auto characterItem = text->items[characterIDs[i]];
                std::holds_alternative<uint32_t>(characterItem.value.value())
            ) {
                formatting.updateUsingFormattingValue(characterItem);
            } else {
                auto characterString = std::get<std::string>(characterItem.value.value());
                if (characterString == "\n") {
                    break; // Time for the next paragraph
                }
                // assert(characterString.size() <= 1); This is not ideal due to utf8 encoding multiple bytes
                if (currentText.formatting != formatting) {
                    // New formatting
                    // Push the current text to the paragraph and create a new one
                    paragraph.contents.push_back(std::move(currentText));
                    formatting.deletedLength = 0;
                    currentText.formatting = formatting;
                }
                if (characterItem.deletedLength > 0 && currentText.formatting.deletedLength == 0) {
                    // Start new deletedLength
                    paragraph.contents.push_back(std::move(currentText));
                    formatting.deletedLength = characterItem.deletedLength;
                    currentText.formatting = formatting;
                } else if (characterItem.deletedLength > 0 && currentText.formatting.deletedLength > 0) {
                    // We can just increase it
                    currentText.formatting.deletedLength += characterItem.deletedLength;
                }
                if (characterItem.deletedLength == 0 && currentText.formatting.deletedLength > 0) {
                    // If the formatting has a deleted length, we need to push the current text and create a new one with the formatting moved forward
                    paragraph.contents.push_back(std::move(currentText));
                    formatting.deletedLength = 0;
                    currentText.formatting = formatting;
                }
                // Add the character to the current text
                currentText.text += characterString;
                currentText.characterIDs.push_back(characterIDs[i]);
            }
            i++;
        }
        if (!currentText.text.empty() || currentText.formatting.deletedLength > 0) {
            paragraph.contents.push_back(std::move(currentText));
        }
        if (text->styleMap.contains(paragraph.startId)) {
            paragraph.style = text->styleMap[paragraph.startId];
        }
        paragraphs.push_back(std::move(paragraph));
    }
}

Text TextDocument::toText() const {
    if (!this->text) {
        throw std::invalid_argument("Text is null");
    }
    // We copy the internal text object, compact the text items and we're done!
    auto newText = *this->text;
    newText.items.compactTextItems();
    return newText;
}

std::string TextDocument::repr() const {
    std::string final;

    for (const auto &paragraph: paragraphs) {
        final += paragraph.repr();
    }

    return final;
}

TextItem createDeletedLength(const CrdtId start, const CrdtId end) {
    // A deleted chunk that takes up anchor IDs
    TextItem item;
    item.itemId = start + 1;
    item.leftId = start;
    item.deletedLength = end.second - start.second;
    item.value = std::string("");
    return item;
}

TextItem createFormatting(const CrdtId id, const FormattingOptions option) {
    TextItem item;
    item.itemId = id + 1;
    item.leftId = id;
    item.deletedLength = 0;
    item.value = static_cast<uint32_t>(option);
    return item;
}

TextItem createTextItem(const CrdtId id, const std::string &text) {
    TextItem item;
    item.itemId = id + 1;
    item.leftId = id;
    item.deletedLength = 0;
    item.value = text;
    return item;
}
