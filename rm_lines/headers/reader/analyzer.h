#pragma once
#include <stack>
#include <reader/tagged_block_reader.h>

inline bool isValidString(const std::string &s) {
    for (const unsigned char c: s) {
        // Check ASCII range
        if (c < 32 || c > 126) return false;
    }
    return true;
}

class Analyzer {
public:
    bool analyzeTag(TaggedBlockReader *reader, bool skipGettingTag = false, bool skipClaim = false);

    void continuousRead(TaggedBlockReader *reader);

    void analyzeData(TaggedBlockReader *reader);

    static void analyze(TaggedBlockReader *reader);

private:
    void setPadding(const int amount) {
        padding = amount;
        paddingStr = std::string(amount * 4, ' ');
        paddingStrSub = std::string((amount + 1) * 4, ' ');
    }

    void incPadding() {
        setPadding(padding + 1);
    }

    void decPadding() {
        setPadding(padding - 1);
    }

    int getCurrentSubBlockIndex() const {
        return subBlockIndexes.empty() ? -1 : subBlockIndexes.top();
    }

    std::optional<SubBlockInfo> getSubBlockInfo() const {
        return subBlockInfos.empty() ? std::nullopt : std::make_optional(subBlockInfos.top());
    }

    std::stack<SubBlockInfo> subBlockInfos;
    std::stack<int> subBlockIndexes;
    int padding = 0;
    std::string paddingStr = "";
    std::string paddingStrSub = "";
};
