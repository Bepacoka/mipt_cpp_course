#include <stdexcept>
#include <string>
#include <print>

#include "alert_sink.h"
#include "dedup_sink.h"
#include "detection.h"

namespace nano_edr {

// --- ConsoleSink -----------------------------------------------------------

void ConsoleSink::operator()(const Detection& detection) const {
    print("{}\n", ToString(detection));
}

// --- FileSink --------------------------------------------------------------

FileSink::FileSink(const std::string& path) : out_(path) {
    if (!out_.is_open()) {
        throw std::runtime_error("не удалось открыть файл: " + path);
    }
}

void FileSink::operator()(const Detection& detection) {
    // а тут то есть cout использовать так святое дело, да? а как же print, мы же на современных плюсах общаемся, а не на этом допотопном синтаксисе вплоть с C++98
    out_ << ToString(detection) << '\n';
}

// --- CountingSink ----------------------------------------------------------

void CountingSink::operator()(const Detection& detection) const {
    ++counter_->total;
    counter_->seen.push_back(detection);
}

// --- FanOutSink ------------------------------------------------------------

void FanOutSink::Add(AlertSink sink) {
    sinks_.push_back(std::move(sink));
}

void FanOutSink::operator()(const Detection& detection) const {
    for (const auto& sink: sinks_) {
        sink(detection);
    }
}

// --- DedupSink -------------------------------------------------------------

DedupSink::DedupSink(AlertSink sink, uint64_t window_ms, std::size_t* suppressed) : sink_(std::move(sink)), window_ms_(window_ms), suppressed_(suppressed) {}

void DedupSink::operator()(const Detection& detection) {
    if (!last_detect_.count({detection.rule, detection.pid}) || detection.ts.ms - last_detect_[{detection.rule, detection.pid}] > window_ms_) {
        last_detect_[{detection.rule, detection.pid}] = detection.ts.ms;
        sink_(detection);
        return;
    } else if (suppressed_) (*suppressed_)++;
}

}  // namespace nano_edr
