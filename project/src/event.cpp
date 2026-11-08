#include "event.h"

#include <charconv>
#include <format>
#include <stdexcept>
#include <system_error>

namespace nano_edr {

Event::Event(const EventParts& parts) {
    uint64_t ms = 0;
    const auto [ptr, ec] = std::from_chars(
        parts.ts.data(), parts.ts.data() + parts.ts.size(), ms
    );

    if (ptr != parts.ts.data() + parts.ts.size() || ec != std::errc{}) {
        throw std::invalid_argument("неверное время события: " + parts.ts);
    }

    if (parts.type.empty()) {
        throw std::invalid_argument("неверный тип события: " + parts.type);
    }

    raw_ts_ = parts.ts;
    ts_ = Timestamp{ms};
    type_ = parts.type;
    event_type_ = EventTypeFromString(type_);
    pid_ = parts.pid;
    fields_ = parts.fields;
}

std::string ToString(const Event& event) {
    std::string result = std::format(
        "ts={} type={} pid={}", event.raw_ts(), event.type(), event.pid()
    );

    for (const auto& field : event.fields()) {
        result += std::format(" {}={}", field.key, field.value);
    }

    return result;
}

enum EventType EventTypeFromString(const std::string& type) {
    if (type == "process_start") return EventType::kProcessStart;
    if (type == "process_end") return EventType::kProcessEnd;
    if (type == "file_create") return EventType::kFileCreate;
    if (type == "file_write") return EventType::kFileWrite;
    if (type == "file_delete") return EventType::kFileDelete;
    if (type == "file_move") return EventType::kFileMove;
    if (type == "net_connect") return EventType::kNetConnect;
    return EventType::kOther;

}

std::ostream& operator<<(std::ostream& out, const Event& event) {
    out << ToString(event);
    return out;
}

}  // namespace nano_edr
