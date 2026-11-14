#ifndef NANO_EDR_AGENT_H
#define NANO_EDR_AGENT_H

#include <cstddef>
#include <map>
#include <string>

#include "event.h"
#include "ring_buffer.h"
#include "event_source.h"
#include "rule_engine.h"

namespace nano_edr {

class Agent : public IEventHandler {
  public:
    struct EventRecord {
        Timestamp ts;
        std::string type;
        std::string pid;
    };
    
    Agent(bool, uint64_t);

    void HandleEvent(const Event& event) override;
    void OnSkipped() override;
    void OnBroken() override;
    void OnTick() override;
    void PrintSummary() const;
    void Warn(const std::string& message) const noexcept;
    void PrintContextOnce();
    void Subscribe(AlertSink);

  private:
    RingBuffer<EventRecord, 256> window_;
    EntityModel model_;
    RuleEngine engine_;
    bool quiet_;
    bool ctx_printed_ = false;
    size_t total_events_ = 0;
    size_t skipped_events_ = 0;
    size_t broken_events_ = 0;
    size_t suppressed_ = 0;
    uint64_t dedup_period_ms_ = 0;
    std::map<std::string, std::size_t> counts_;
};

}  // namespace nano_edr

#endif  // NANO_EDR_AGENT_H
