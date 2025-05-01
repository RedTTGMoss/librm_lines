#include "advanced/text_sequence.h"

std::vector<std::string> splitUtf8(const std::string &str) {
    std::vector<std::string> result;
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
    for (const std::u32string u32 = conv.from_bytes(str); const char32_t ch : u32) {
        result.push_back(conv.to_bytes(ch));
    }
    return result;
}

void TextSequence::expandTextItems() {
    if (expanded)
        return;
    expanded = true;

    // We move the current sequence and create the new sequence with characters
    for (auto oldSequence = std::move(sequence); TextItem &textItem: oldSequence | std::views::values) {
        // Get the variant of string / int of the text item
        auto value = textItem.value.value();

        // Check if we can skip the text item
        if (
            std::holds_alternative<uint32_t>(value) // Matches the definition of a text format item
        ) {
            // Move to the new sequence
            add(std::move(textItem));
            continue;
        }

        // Otherwise begin by getting the string
        auto rawString = splitUtf8(std::get<std::string>(value));

        // Initialize IDs for registering the characters
        auto itemId = textItem.itemId;
        auto leftId = textItem.leftId;

        if (textItem.deletedLength > 0) {
            // If the text is deleted we want to create one deleted character for the length of the deleted items
            for (int i = 0; i < textItem.deletedLength; i++) {
                const auto rightId = i == textItem.deletedLength - 1
                                         ? textItem.rightId
                                         : CrdtId{itemId.first, itemId.second + 1};
                add(TextItem{
                    itemId,
                    leftId,
                    rightId,
                    1, "" // One BLANK character, not a space
                });

                // Move the IDs forward for the next character
                leftId = itemId;
                itemId = rightId;
            }
        } else {
            // If the text is valid we want to split each char into a new item
            for (int i = 0; i < rawString.size(); i++) {
                const auto rightId = i == rawString.size() - 1 ? textItem.rightId : CrdtId{itemId.first, itemId.second + 1};
                add(TextItem{
                    itemId,
                    leftId,
                    rightId,
                    0, // This is no longer a blank character
                    rawString[i]
                });

                // Move the IDs forward for the next character
                leftId = itemId;
                itemId = rightId;
            }
        }
    }
}
