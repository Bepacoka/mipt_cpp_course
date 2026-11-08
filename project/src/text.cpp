#include "text.h"

#include <algorithm>
#include <cctype>

namespace nano_edr {
namespace {

char Lower(char c) {
    return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
}

bool SameChar(char left, char right) {
    return Lower(left) == Lower(right);
}

}  // namespace

bool TextEquals(const std::string& left, const std::string& right) {
    return left.size() == right.size() &&
           std::equal(left.begin(), left.end(), right.begin(), SameChar);
}

bool TextContains(const std::string& text, const std::string& needle) {
    if (needle.empty()) return true;
    if (needle.size() > text.size()) return false;

    return std::search(text.begin(), text.end(), needle.begin(), needle.end(),
                       SameChar) != text.end();
}

bool TextEndsWith(const std::string& text, const std::string& suffix) {
    if (suffix.size() > text.size()) return false;
    return std::equal(suffix.rbegin(), suffix.rend(), text.rbegin(), SameChar);
}

}  // namespace nano_edr
