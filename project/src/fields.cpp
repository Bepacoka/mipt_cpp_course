#include "fields.h"

#include "text.h"

#include <charconv>
#include <cctype>
#include <format>
#include <stdexcept>
#include <string>

namespace nano_edr {

const std::string* FindField(const Event& event, const std::string& key) {
    if (key == "ts") return &event.raw_ts();
    if (key == "type") return &event.type();
    if (key == "pid") return &event.pid();

    for (const auto& field : event.fields()) {
        if (field.key == key) return &field.value;
    }

    return nullptr;
}

const std::string& GetRequiredField(const Event& event, const std::string& key) {
    const auto* field = FindField(event, key);
    if (!field) {
        throw std::invalid_argument(std::format("нет поля {} в строке", key));
    }
    return *field;
}

bool GetIntField(const Event& event, const std::string& key, uint64_t* out) {
    const auto* field = FindField(event, key);
    if (!field || !out) return false;

    const auto [ptr, ec] = std::from_chars(
        field->data(), field->data() + field->size(), *out
    );
    return ptr == field->data() + field->size() && ec == std::errc{};
}

uint64_t GetIntField(const Event& event, const std::string& key,
                     uint64_t fallback) {
    uint64_t out = fallback;
    if (!GetIntField(event, key, &out)) return fallback;
    return out;
}

bool IsProcessStart(const Event& event) {
    return event.type() == "process_start";
}

bool IsFileWrite(const Event& event) {
    return event.type() == "file_write";
}

bool IsNetConnect(const Event& event) {
    return event.type() == "net_connect";
}

bool PathEndsWith(const Event& event, const std::string& suffix) {
    const auto* field = FindField(event, "path");
    if (!field) return false;
    return TextEndsWith(NormalizePath(*field), NormalizePath(suffix));
}

bool CommandLineContains(const Event& event, const std::string& needle) {
    const auto* field = FindField(event, "cmdline");
    if (!field) return false;
    return TextContains(NormalizePath(*field), NormalizePath(needle));
}

std::string NormalizePath(const std::string& path) {
    std::string result;
    result.reserve(path.size());

    for (char c : path) {
        if (c == '/') c = '\\';
        if (!result.empty() && result.back() == '\\' && c == '\\') continue;
        result += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }

    for (const std::string token : {"%temp%", "%tmp%"}) {
        size_t i = result.find(token);
        while (i != std::string::npos) {
            result.replace(i, token.size(), R"(\appdata\local\temp)");
            i = result.find(token, i + 1);
        }
    }

    return result;
}

}  // namespace nano_edr
