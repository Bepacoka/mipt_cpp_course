#include "rule.h"
#include <chrono>
#include <ranges>

namespace nano_edr {

RuleBase::RuleBase(const std::string& id, Severity severity)
    : id_(id), severity_(severity) {}

void RuleBase::AddAction(const std::string& action) {
    actions_.push_back(action);
}

MatchRule::MatchRule(const std::string& id, Severity severity)
    : RuleBase(id, severity) {}

void MatchRule::AddCondition(std::unique_ptr<ICondition> condition) {
    conditions_.push_back(std::move(condition));
}

bool MatchRule::Check(const Event& event) {
    for (const auto& condition : conditions_) {
        if (!condition->Matches(event)) return false;
    }

    CountHit();
    return true;
}

SequenceRule::SequenceRule(const std::string& id, Severity severity, uint64_t window_ms) : RuleBase(id, severity), window_ms_(window_ms) {}
void SequenceRule::SetFirst(std::unique_ptr<ICondition> first) {
    first_ = std::move(first);
}
void SequenceRule::SetSecond(std::unique_ptr<ICondition> second) {
    second_ = std::move(second);
}
bool SequenceRule::Check(const Event& event) {
    if (!first_ || !second_) return false;
    while (!pending_.empty() && event.ts().ms - pending_[0].ts_ms > window_ms_) pending_.erase(pending_.begin()); // world if std::deque
    if (second_->Matches(event)) {
        for (auto it = pending_.begin(); it != pending_.end(); ++it) {
            if (!same_pid_ || it->pid == event.pid()) {
                pending_.erase(it);
                CountHit();
                return true;
            }
        }
    }
    if (first_->Matches(event)) {
        pending_.push_back({event.ts().ms, event.pid()});
    }
    return false;
}

}  // namespace nano_edr
