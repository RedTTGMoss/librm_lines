#pragma once
#include <cstdint>
#include <string>

struct NumberingCounter {
    uint16_t num = 0;
    uint16_t numTab = 0;
    uint16_t numTabTab = 0;

    void reset() {
        num = 0;
        numTab = 0;
        numTabTab = 0;
    }

    void numPlus() {
        num++;
        numTab = 0;
        numTabTab = 0;
    }

    void numTabPlus() {
        numTab++;
        numTabTab = 0;
    }

    void numTabTabPlus() {
        numTabTab++;
    }

    std::string getNum() const {
        return std::to_string(num) + ".";
    }

    std::string getNumTab() const {
        // Letters from a to z, then aa, ab, ac, etc.
        std::string result;
        int n = numTab - 1;
        do {
            result = static_cast<char>('a' + (n % 26)) + result;
            n = n / 26 - 1;
        } while (n >= 0);
        return result + ".";
    }

    std::string getNumTabTab() const {
        // Roman numerals from i to x, then xi, xii, xiii, etc.
        int n = numTabTab;
        std::string result;

        static constexpr std::pair<int, const char *> numerals[] = {
            {1000, "m"},
            {900, "cm"},
            {500, "d"},
            {400, "cd"},
            {100, "c"},
            {90, "xc"},
            {50, "l"},
            {40, "xl"},
            {10, "x"},
            {9, "ix"},
            {5, "v"},
            {4, "iv"},
            {1, "i"},
        };

        for (const auto &[value, numeral]: numerals) {
            while (n >= value) {
                result += numeral;
                n -= value;
            }
        }

        return result + ".";
    }
};
