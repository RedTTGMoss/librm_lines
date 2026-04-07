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

std::vector<CrdtId> TextSequence::getSortedTextIds() const {
    // Skip all the processing ahead, if the sequence is empty
    if (sequence.empty())
        return {};

    // Helper function to calculate the "length" of a text item (how many virtual positions it occupies)
    auto getItemLength = [](const TextItem &item) -> uint32_t {
        // Skip invalid items with no value
        if (!item.value.has_value())
            return 0;

        auto value = item.value.value();

        // Format items (uint32_t) always count as 1 position
        if (std::holds_alternative<uint32_t>(value))
            return 1;

        // Deleted items: deletedLength itself is the count (1 means just itemId, 2 means itemId + one extra)
        if (item.deletedLength > 0)
            return item.deletedLength;

        // String items: string length determines virtual positions
        // NOTE: Using string.length() for now. If different UTF-8 characters cause issues in the future,
        // consider using splitUtf8(str).size() to count actual UTF-8 characters instead.
        return std::get<std::string>(value).length();
    };

    // Helper to check if a CrdtId falls within an item's virtual range
    auto idInRange = [&getItemLength](const CrdtId &targetId, const TextItem &item) -> bool {
        uint32_t length = getItemLength(item);
        if (length == 0)
            return false;

        // Check if targetId is within [itemId, itemId + length - 1]
        if (item.itemId.first != targetId.first)
            return false;

        return targetId.second >= item.itemId.second &&
               targetId.second < item.itemId.second + length;
    };

    // Allocate size for the final sorted keys
    std::vector<CrdtId> sortedIds;
    sortedIds.reserve(sequence.size());

    // Build dependency graph
    std::map<CrdtId, std::unordered_set<CrdtId> > graph;

    for (const auto &[itemId, item]: sequence) {
        // Skip invalid items with no value
        if (!item.value.has_value())
            continue;

        uint32_t length = getItemLength(item);
        if (length == 0)
            continue;

        // Initialize this item in the graph
        graph[itemId] = {};

        // Check leftId dependency
        if (item.leftId != END_MARKER) {
            // Find which item contains the leftId position
            for (const auto &[otherId, otherItem]: sequence) {
                if (otherId == itemId)
                    continue;

                // Check if leftId falls within this other item's range
                if (idInRange(item.leftId, otherItem)) {
                    graph[itemId].insert(otherId);
                    break;
                }
            }
        }

        // Check rightId dependencies: items that have this item in their virtual range should come after
        for (const auto &[otherId, otherItem]: sequence) {
            if (otherId == itemId)
                continue;

            if (!otherItem.value.has_value())
                continue;

            // If another item's rightId falls within our virtual range, we must come before it
            if (otherItem.rightId != END_MARKER && idInRange(otherItem.rightId, item)) {
                graph[otherId].insert(itemId);
            }
        }
    }

    // Topological sort with tie-breaking by itemId
    std::vector<CrdtId> nextIds;
    while (!graph.empty()) {
        for (auto it = graph.begin(); it != graph.end();) {
            // If the item has no dependencies, we can add it to the next items
            if (it->second.empty()) {
                nextIds.push_back(it->first);
                it = graph.erase(it);
            } else {
                ++it;
            }
        }

        if (nextIds.empty() && !graph.empty()) {
            // If we have no next items but the graph is not finished, we have a cycle
            std::string debugGraph = "Here's the dependency graph, while sorting the text sequence: ";
            for (const auto &[key, value]: graph) {
                debugGraph += std::format("\nItem: {} -> ", key.repr());
                for (const auto &dep: value) {
                    debugGraph += std::format("{} ", dep.repr());
                }
            }
            logError(debugGraph);
            throw std::runtime_error("Cyclic dependency in text sequence");
        }

        // Sort ascending by itemId to break ties consistently
        std::sort(nextIds.begin(), nextIds.end(), [](const CrdtId &a, const CrdtId &b) {
            return a < b;
        });

        for (const auto &itemId: nextIds) {
            // Add the item to the sorted keys
            sortedIds.push_back(itemId);

            // Remove the item as a dependency of all other items in the graph
            for (auto &dependencies: graph | std::views::values)
                dependencies.erase(itemId);
        }
        // Erase the list of items for the next loop cycle
        nextIds.clear();
    }

    return sortedIds;
}
