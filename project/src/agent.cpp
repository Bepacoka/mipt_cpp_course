#include "agent.h"

#include "agent_rules.h"
#include "rules.h"

#include <format>
#include <print>

namespace nano_edr {

Agent::Agent(bool quiet)
    : window_(), engine_(BuildAgentRules()), quiet_(quiet) {}

void Agent::HandleEvent(const Event& event) {
    std::vector<const IRule*> fired;
    engine_.ProcessEvent(event, &fired);
    if (!fired.empty() && !quiet_) {
        if (window_.size() >= 2) {
            std::print("[CTX] -2: ts={} type={} pid={}\n",
                        window_[window_.size() - 2].ts.ms, window_[window_.size() - 2].type, window_[window_.size() - 2].pid);
        }
        if (!window_.empty()) {
            std::print("[CTX] -1: ts={} type={} pid={}\n",
                window_.back().ts.ms, window_.back().type, window_.back().pid);
        }
    }
    for (const auto& rule : fired) {
        std::print("[DETECT] {}  {}  ts={} pid={}\n",
                       SeverityName(rule->severity()), rule->id(),
                       event.raw_ts(), event.pid());
    }
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
}

void Agent::Warn(const std::string& message) const noexcept {
    if (quiet_) return;

    try {
        std::print(stderr, "[WARN] {}\n", message);
    } catch (...) {
    }
}

}  // namespace nano_edr
