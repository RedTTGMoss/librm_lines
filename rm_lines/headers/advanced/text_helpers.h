#pragma once
#include "simdutf.h"

static std::vector<std::string_view> splitTextIntoLines(std::string_view text) {
    std::vector<std::string_view> lines;

    size_t start = 0;
    while (start < text.size()) {
        size_t end = text.find('\n', start);

        if (end == std::string_view::npos) {
            lines.emplace_back(text.data() + start, text.size() - start);
            break;
        }

        lines.emplace_back(text.data() + start, end - start + 1);
        start = end + 1;
    }

    return lines;
}

static std::string sanitizeUtf8(const std::string &input) {
    std::string output;
    output.reserve(input.size());

    size_t offset = 0;

    while (offset < input.size()) {
        auto result = simdutf::validate_utf8_with_errors(
            input.data() + offset,
            input.size() - offset
        );

        if (result.error == simdutf::error_code::SUCCESS) {
            output.append(input, offset, input.size() - offset);
            break;
        }

        // Copy everything before the bad byte
        output.append(input, offset, result.count);

        // Replace the invalid byte/sequence
        output += "\xEF\xBF\xBD"; // U+FFFD �

        // Skip the offending byte and continue
        offset += result.count + 1;
    }

    return output;
}
