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
    
    Agent(bool quiet);

    void HandleEvent(const Event& event) override;
    void OnSkipped() override;
    void OnBroken() override;
    void OnTick() override;
    void PrintSummary() const;
    void Warn(const std::string& message) const noexcept;

  private:
    RingBuffer<EventRecord, 256> window_;
    RuleEngine engine_;
    bool quiet_;
    std::size_t total_events_ = 0;
    std::size_t skipped_events_ = 0;
    std::size_t broken_events_ = 0;
    std::map<std::string, std::size_t> counts_;
};

}  // namespace nano_edr

#endif  // NANO_EDR_AGENT_H
