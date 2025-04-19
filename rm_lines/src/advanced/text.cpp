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

std::string Paragraph::repr() const {
    std::string final = "";
    switch (style.value) {
        case BASIC:
            // I can't find this being used for any of the texts?
            break;
        case PLAIN:
            // This is paragraph text
            break;
        case HEADING:
            final += "# ";
            break;
        case BOLD:
            final += "## ";
            break;
        case BULLET:
            final += "- ";
            break;
        case BULLET2:
            // I can't find this being used anywhere too, but it's some variant or version of the bullet style
            final += "-- ";
            break;
        case CHECKBOX:
            final += "☐ ";
            break;
        case CHECKBOX_CHECKED:
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

void TextDocument::fromText(Text &text) {
    this->text = text;
    text.items.expandTextItems();
    paragraphs.clear();
    const auto characterIDs = text.items.getSortedIds();

    TextFormattingOptions formatting;

    int i = 0;

    while (i < characterIDs.size()) {
        // Initiate a new paragraph
        Paragraph paragraph;

        if (checkString(text.items[characterIDs[i]], "\n")) {
            paragraph.startId = characterIDs[i];
            i++;
        } else {
            paragraph.startId = END_MARKER;
        }

        FormattedText currentText;
        while (i < characterIDs.size()) {
            if (
                auto characterItem = text.items[characterIDs[i]];
                std::holds_alternative<uint32_t>(characterItem.value.value())
            ) {
                formatting.updateUsingFormattingValue(characterItem);
            } else {
                auto characterString = std::get<std::string>(characterItem.value.value());
                if (characterString == "\n") {
                    break; // Time for the next paragraph
                }
                assert(characterString.size() <= 1);
                if (currentText.text.empty()) {
                    // If the current text is empty, we can just add the formatting information
                    currentText.formatting = formatting;
                } else if (currentText.formatting != formatting) {
                    // Push the current text to the paragraph and create a new one
                    paragraph.contents.push_back(std::move(currentText));
                    currentText.formatting = formatting;
                }
                // Add the character to the current text
                currentText.text += characterString;
                currentText.characterIDs.push_back(characterIDs[i]);
            }
            i++;
        }
        if (!currentText.text.empty()) {
            paragraph.contents.push_back(std::move(currentText));
        }
        if (text.styleMap.contains(paragraph.startId)) {
            paragraph.style = text.styleMap[paragraph.startId];
        }
        paragraphs.push_back(std::move(paragraph));
    }
}

std::string TextDocument::repr() const {
    std::string final;

    for (const auto &paragraph: paragraphs) {
        final += paragraph.repr();
    }

    return final;
}
