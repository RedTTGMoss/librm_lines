#pragma once

#include "common/data_types.h"
#include "common/crdt_sequence_item.h"
#include "common/scene_items.h"

struct Text;
struct TextFormattingOptions;
constexpr std::array<std::pair<ParagraphStyle, int>, 12> StyleHeights = {
    {
        {BASIC, 100},
        {PlainText, 71},
        {Title, 150},
        {Sub, 70},
        {Bullet, 71},
        {BulletTab, 71},
        {CheckBox, 71},
        {CheckBoxChecked, 71},
        {CheckBoxTab, 71},
        {CheckBoxTabChecked, 71},
        {Numbered, 71},
        {NumberedTab, 71}
    }
};

constexpr std::array<std::pair<ParagraphStyle, int>, 12> LineHeights = {
    {
        {BASIC, 42.666666666666664},
        {PlainText, 42.666666666666664},
        {Title, 85.33333333333333},
        {Sub, 48},
        {Bullet, 42.666666666666664},
        {BulletTab, 42.666666666666664},
        {CheckBox, 42.666666666666664},
        {CheckBoxChecked, 42.666666666666664},
        {CheckBoxTab, 42.666666666666664},
        {CheckBoxTabChecked, 42.666666666666664},
        {Numbered, 42.666666666666664},
        {NumberedTab, 42.666666666666664}
    }
};

constexpr std::array<std::pair<ParagraphStyle, int>, 12> StyleWeights = {
    {
        {BASIC, 400},
        {Sub, 500},
    }
};
constexpr std::array<std::pair<ParagraphStyle, int>, 12> StyleWeightsItalic = {
    {
        {BASIC, 400},
    }
};
constexpr std::array<std::pair<ParagraphStyle, int>, 12> StyleWeightsBold = {
    {
        {BASIC, 700},
        {Sub, 800},
    }
};

constexpr float getStyleHeight(const ParagraphStyle style) {
    for (const auto &[key, value]: StyleHeights) {
        if (key == style)
            return static_cast<float>(value);
    }
    return static_cast<float>(StyleHeights[0].second);
}

constexpr float getLineHeight(const ParagraphStyle style) {
    for (const auto &[key, value]: LineHeights) {
        if (key == style)
            return static_cast<float>(value);
    }
    return static_cast<float>(LineHeights[0].second);
}

float getStyleWeight(const ParagraphStyle style, const TextFormattingOptions formatting);

enum FormattingOptions {
    BOLD_ON = 1,
    BOLD_OFF = 2,
    ITALIC_ON = 3,
    ITALIC_OFF = 4,
};

struct FontStyle {
    bool italic;
    float weight;
};

constexpr CrdtId ANCHOR_ID_START(0, 281474976710654);
constexpr CrdtId ANCHOR_ID_END(0, 281474976710655);

struct TextFormattingOptions {
    bool bold = false;
    bool italic = false;
    int deletedLength = 0;

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
    std::shared_ptr<Text> text;
    std::vector<Paragraph> paragraphs;

    void fromText(const std::shared_ptr<Text> &_text);

    Text toText() const;

    std::string repr() const;
};

TextItem createDeletedLength(CrdtId start, CrdtId end);

TextItem createFormatting(CrdtId id, FormattingOptions option);

TextItem createTextItem(CrdtId id, const std::string &text);
