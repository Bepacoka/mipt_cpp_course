#include "parse.h"

#include <string>

namespace nano_edr {

bool IsBlankOrComment(const std::string& line) {
    const size_t start = line.find_first_not_of(" \t");
    return start == std::string::npos || line[start] == '#' || line[start] == ';';
}

bool ParseEventParts(const std::string& line, EventParts* out) {
    if (!out || IsBlankOrComment(line)) return false;

    EventParts parts;
    bool seen_ts = false;
    bool seen_type = false;
    bool seen_pid = false;
    size_t i = line.find_first_not_of(" \t");

    while (i != std::string::npos) {
        const size_t equal = line.find('=', i);
        const size_t whitespace = line.find_first_of(" \t", i);

        if (equal == std::string::npos || equal == i ||
            (whitespace != std::string::npos && whitespace < equal)) {
            return false;
        }

        const std::string key = line.substr(i, equal - i);
        const size_t value_start = equal + 1;
        std::string value;

        if (value_start < line.size() && line[value_start] == '"') {
            const size_t content_start = value_start + 1;
            const size_t quote = line.find('"', content_start);
            if (quote == std::string::npos) return false;

            value = line.substr(content_start, quote - content_start);
            i = quote + 1;
            if (i < line.size() && line[i] != ' ' && line[i] != '\t') {
                return false;
            }
        } else {
            const size_t end = line.find_first_of(" \t", value_start);
            if (end == std::string::npos) {
                value = line.substr(value_start);
                i = std::string::npos;
            } else {
                value = line.substr(value_start, end - value_start);
                i = end;
            }
        }

        if (key == "ts" && !seen_ts) {
            parts.ts = value;
            seen_ts = true;
        } else if (key == "type" && !seen_type) {
            parts.type = value;
            seen_type = true;
        } else if (key == "pid" && !seen_pid) {
            parts.pid = value;
            seen_pid = true;
        } else {
            parts.fields.push_back({key, value});
        }

        if (i != std::string::npos) {
            i = line.find_first_not_of(" \t", i);
        }
    }

    *out = parts;
    return true;
}

}  // namespace nano_edr
