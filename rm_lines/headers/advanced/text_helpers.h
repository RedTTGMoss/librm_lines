#pragma once

std::vector<std::string_view> splitTextIntoLines(std::string_view text) {
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