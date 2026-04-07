#include "advanced/text_sequence.h"
#include <utf8.h>

std::vector<std::string> splitUtf8(const std::string &str) {
    std::vector<std::string> result;
    auto it = str.begin();
    while (it != str.end()) {
        auto next = it;
        utf8::next(next, str.end());
        result.emplace_back(it, next);
        it = next;
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
                                         : CrdtId(itemId.first, itemId.second + 1);
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
                const auto rightId = i == rawString.size() - 1
                                         ? textItem.rightId
                                         : CrdtId(itemId.first, itemId.second + 1);
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

void TextSequence::compactTextItems() {
    if (!expanded)
        return;
    expanded = false;

    // Get sorted IDs before moving the sequence
    auto sortedIds = getSortedIds();

    // We move the current sequence and create the new sequence with merged strings
    auto oldSequence = std::move(sequence);

    int i = 0;
    while (i < sortedIds.size()) {
        auto &firstItem = oldSequence[sortedIds[i]];
        auto value = firstItem.value.value();

        // Check if we can skip the text item (format items)
        if (std::holds_alternative<uint32_t>(value)) {
            // Move format items directly to the new sequence
            add(std::move(firstItem));
            i++;
            continue;
        }

        // Start building a merged item
        std::string mergedString;
        uint32_t mergedDeletedLength = 0;
        auto itemId = firstItem.itemId;
        auto leftId = firstItem.leftId;
        CrdtId rightId = firstItem.rightId;

        // Determine if we're handling deleted items
        bool isDeleted = firstItem.deletedLength > 0;

        // Collect consecutive items that can be merged
        int j = i;
        while (j < sortedIds.size()) {
            auto &currentItem = oldSequence[sortedIds[j]];
            auto currentValue = currentItem.value.value();

            // Stop if it's a format item
            if (std::holds_alternative<uint32_t>(currentValue)) {
                break;
            }

            // Stop if deleted status doesn't match
            if ((currentItem.deletedLength > 0) != isDeleted) {
                break;
            }

            // For items beyond the first, check if they form a sequential chain
            if (j > i) {
                auto &prevItem = oldSequence[sortedIds[j - 1]];
                // Check if the current item's itemId matches the previous item's rightId
                // AND if the current item's leftId matches the previous item's itemId
                if (currentItem.itemId != prevItem.rightId || currentItem.leftId != prevItem.itemId) {
                    break;
                }
            }

            // Add to merged data
            if (isDeleted) {
                mergedDeletedLength += currentItem.deletedLength;
            } else {
                mergedString += std::get<std::string>(currentValue);
            }

            rightId = currentItem.rightId;
            j++;
        }

        // Create the merged item
        if (isDeleted) {
            add(TextItem{
                itemId,
                leftId,
                rightId,
                mergedDeletedLength,
                ""
            });
        } else {
            add(TextItem{
                itemId,
                leftId,
                rightId,
                0,
                mergedString
            });
        }

        // Move to the next unprocessed item
        i = j;
    }
}
