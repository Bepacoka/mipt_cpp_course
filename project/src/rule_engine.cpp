#include "rule_engine.h"
#include "detection.h"

namespace nano_edr {

void RuleEngine::Subscribe(AlertSink sink) {
  sinks_.push_back(std::move(sink));
}

Detection RuleEngine::MakeDetection(const IRule& rule, const Event& event) const {
  return Detection(rule.id(), rule.severity(), event.ts(), event.pid(), model_ ? model_->DescribeChain(event.pid()) : "", rule.actions());
}

void RuleEngine::AddRule(std::unique_ptr<IRule> rule) {
  rules_.push_back(std::move(rule));
}

size_t RuleEngine::ProcessEvent(const Event& event, std::vector <const IRule*> *fired) {
  size_t cnt = 0;
  for (const auto &rule : rules_) {
    if (rule->Check(event)) {
      cnt += 1;
      Detection detect = MakeDetection(*rule, event);
      for (const auto& sink: sinks_) {
        sink(detect);
      }
      if (fired) fired->push_back(rule.get());
    }
  }
  return cnt;
}

}
