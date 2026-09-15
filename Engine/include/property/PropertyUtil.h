#pragma once

#include <string>
#include <cctype>

inline std::string camelToWords(const std::string& s) {
    if (s.empty()) return s;

    std::string result;
    for (int i = 0; i < s.size(); i++) {
        if (std::isupper(static_cast<unsigned char>(s[i])) && i > 0)
            result += ' ';
        result += s[i];
    }
    result[0] = std::toupper(static_cast<unsigned char>(result[0]));
    return result;
}
