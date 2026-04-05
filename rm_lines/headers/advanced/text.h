#pragma once

#include "common/data_types.h"
#include "common/crdt_sequence_item.h"
#include "common/scene_items.h"

struct Text;
constexpr std::array<std::pair<ParagraphStyle, int>, 12> LineHeights = {
    {
        {BASIC, 100},
        {PlainText, 71},
        {Title, 150},
        {Sub, 70},
        {Bullet, 35},
        {BulletTab, 35},
        {CheckBox, 100},
        {CheckBoxChecked, 100},
        {CheckBoxTab, 100},
        {CheckBoxTabChecked, 100},
        {Numbered, 100}, // TODO: Update line height for numbered
        {NumberedTab, 100}
    }
};

constexpr float getStyleHeight(const ParagraphStyle style) {
    for (const auto &[key, value]: LineHeights) {
        if (key == style)
            return static_cast<float>(value);
    }
    return static_cast<float>(LineHeights[0].second);
}

enum FormattingOptions {
    BOLD_ON = 1,
    BOLD_OFF = 2,
    ITALIC_ON = 3,
    ITALIC_OFF = 4,
};

constexpr CrdtId ANCHOR_ID_START(0, 281474976710654);
constexpr CrdtId ANCHOR_ID_END(0, 281474976710655);

struct TextFormattingOptions {
    bool bold = false;
    bool italic = false;

    void updateUsingFormattingValue(const FormattingOptions option) {
        switch (option) {
            case BOLD_ON:
                bold = true;
                break;
            case BOLD_OFF:
                bold = false;
                break;
            case ITALIC_ON:
                italic = true;
                break;
            case ITALIC_OFF:
                italic = false;
                break;
            default:
                throw std::runtime_error(std::format("Unhandled formatting option {}", static_cast<int>(option)));
        }
    }

    void updateUsingFormattingValue(const int option) {
        const auto formattingOption = static_cast<FormattingOptions>(option);
        updateUsingFormattingValue(formattingOption);
    }

    void updateUsingFormattingValue(const TextItem &item) {
        if (std::holds_alternative<uint32_t>(item.value.value())) {
            updateUsingFormattingValue(std::get<uint32_t>(item.value.value()));
        } else {
            throw std::runtime_error("TextItem value is not a formatting option");
        }
    }

    bool operator==(const TextFormattingOptions other) const {
        return bold == other.bold && italic == other.italic;
    }

    json toJson() const;
};

template<>
struct std::hash<TextFormattingOptions> {
    size_t operator()(const TextFormattingOptions &options) const noexcept {
        return std::hash<bool>{}(options.bold) ^ (std::hash<bool>{}(options.italic) << 1);
    }
};

struct FormattedText {
    std::string text;
    std::vector<CrdtId> characterIDs;

    // Formatting options on this text
    TextFormattingOptions formatting;

    json toJson() const;
};

struct Paragraph {
    // Representing a single line which can contain multiple texts
    std::vector<FormattedText> contents;
    CrdtId startId;
    LwwItem<ParagraphStyleNew> style = LwwItem(ParagraphStyleNew(PlainText));

    std::string repr() const;

    json toJson() const;
};

struct TextDocument {
    Text text;
    std::vector<Paragraph> paragraphs;

    void fromText(Text &text);

    std::string repr() const;
};
