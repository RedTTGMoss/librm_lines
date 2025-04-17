#include "advanced/text.h"

void expandTextItems(CrdtSequence<TextItem> &textSequence) {
    CrdtSequence<TextItem> characterSequence;

    for (TextItem& text : textSequence | std::views::values ) {
        // Get the variant of string / int of the text item
        auto value = text.value.value();

        // Check if we can skip the text item
        if (
            std::holds_alternative<uint32_t>(value) // Matches the definition of a text format item
        ) {
            // Move to the new sequence
            characterSequence.add(std::move(text));
            continue;
        }

        // Otherwise begin by getting the string
        auto rawString = std::get<std::string>(value);



        // Initialize IDs for registering the characters
        auto itemId = text.itemId;
        auto leftId = text.leftId;

        if (text.deletedLength > 0) {
            // If the text is deleted we want to create one deleted character for the length of the deleted items
            for (int i = 0; i < text.deletedLength; i++) {
                const auto rightId = i == text.deletedLength-1 ? text.rightId : CrdtId{itemId.first, itemId.second + 1};
                characterSequence.add(TextItem{
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
                const auto rightId = i == rawString.size()-1 ? text.rightId : CrdtId{itemId.first, itemId.second + 1};
                characterSequence.add(TextItem{
                    itemId,
                    leftId,
                    rightId,
                    0, // This is no longer a blank character
                    std::string(1, rawString[i])
                });

                // Move the IDs forward for the next character
                leftId = itemId;
                itemId = rightId;
            }
        }
    }

    // Replace the original text sequence with the new one
    textSequence = characterSequence;
}
