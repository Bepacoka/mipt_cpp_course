#include "field_traits.h"
#include <os.h>
#include <string>

namespace nano_edr {

bool FieldTraits<std::string>::Parse(const std::string& text, std::string *out) {
    *out = text;
    return true;
}

bool FieldTraits<uint64_t>::Parse(const std::string& text, uint64_t *out) {
    uint64_t old_out = *out; // типа это от меня хотят или что?
    const auto [ptr, ec] = std::from_chars(
        text.data(), text.data() + text.size(), *out
    );
    bool res = ptr == text.data() + text.size() && ec == std::errc{};
    if (!res) *out = old_out;
    return res;
}

char tolower(char c) {
    if ('A' <= c && c <= 'Z') return c + ('a' - 'A');
    return c;
}

bool FieldTraits<bool>::Parse(const std::string& text, bool *out) {
    // как олпрогер, учился у лучших
    // https://github.com/blackav/ejudge/blob/master/gen/new_server_match.c
    if (text == "1" || (text.size() == 3 && tolower(text[0]) == 'y' && tolower(text[1]) == 'e' && tolower(text[2]) == 's') || (text.size() == 4 && tolower(text[0]) == 't' && tolower(text[1]) == 'r' && tolower(text[2]) == 'u' && tolower(text[3]) == 'e')) return (*out = true);
    if (text == "0" || (text.size() == 2 && tolower(text[0]) == 'n' && tolower(text[1]) == 'o') || (text.size() == 5 && tolower(text[0]) == 'f' && tolower(text[1]) == 'a' && tolower(text[2]) == 'l' && tolower(text[3]) == 's' && tolower(text[4]) == 'e')) return !(*out = false);
    return false;
}

}
