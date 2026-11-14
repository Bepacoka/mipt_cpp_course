#include "agent.h"

#include "agent_rules.h"
#include "dedup_sink.h"

#include <format>
#include <print>

namespace nano_edr {

Agent::Agent(bool quiet, uint64_t dedup_period_ms)
    : window_(), engine_(BuildAgentRules(&model_)), quiet_(quiet), dedup_period_ms_(dedup_period_ms) {
        engine_.SetModel(&model_);
    }

void Agent::HandleEvent(const Event& event) {
    model_.Observe(event);
    ctx_printed_ = false;
    std::vector<const IRule*> fired; // ...а зачем он нам теперь?
    engine_.ProcessEvent(event, &fired);
    window_.PushBack({event.ts(), event.type(), event.pid()});
    ++total_events_;
    ++counts_[event.type()];
}

void Agent::OnSkipped() {
    skipped_events_ += 1;    
}

void Agent::OnBroken() {
    broken_events_ += 1;
}

void Agent::OnTick() {
    
}

void Agent::PrintSummary() const {
    if (quiet_) return;

    std::print("[STAT] событий всего {}\n", total_events_);
    std::print("[STAT] пропущенных событий {}\n", skipped_events_);
    std::print("[STAT] сломанных событий {} \n", broken_events_);
    for (const auto& [type, count] : counts_) {
        std::print("[STAT] событий типа {} всего {}\n", type, count);
    }

    std::print("[STAT] правила, детектов:\n");
    for (size_t i = 0; i < engine_.size(); i++) {
        std::print("  {}  {}\n", engine_.rule(i).id(), engine_.rule(i).hits());
    }
    std::print("[STAT] в окне занято {} из 256 мест\n", window_.size()); // зачем, ну ладно
    std::print("[STAT] в модели проццессов {}, файлов {}", model_.process_count(), model_.file_count());
    if (suppressed_ > 0) {
        std::print("[STAT] подавленных дубликатов: {} (срок {} мс)", suppressed_, dedup_period_ms_);
    }
}

void Agent::Warn(const std::string& message) const noexcept {
    if (quiet_) return;

    try {
        std::print(stderr, "[WARN] {}\n", message);
    } catch (...) {
    }
}

void Agent::PrintContextOnce() {    
    if (quiet_ || ctx_printed_) return;
    if (window_.size() >= 2) {
        std::print("[CTX] -2: {} {} pid={}\n",
                   window_[window_.size() - 2].ts.ms, window_[window_.size() - 2].type, window_[window_.size() - 2].pid);
    }
    if (!window_.empty()) {
        std::print("[CTX] -1: {} {} pid={}\n",
            window_.back().ts.ms, window_.back().type, window_.back().pid);
    }
    ctx_printed_ = true;
}

void Agent::Subscribe(AlertSink sink) {
    FanOutSink all;
    all.Add([this](const Detection&) { PrintContextOnce(); });
    all.Add(std::move(sink));
    engine_.Subscribe(DedupSink(std::move(all), dedup_period_ms_, &suppressed_));
}

}  // namespace nano_edr
