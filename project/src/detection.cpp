#include "detection.h"

namespace nano_edr {

std::string ToString(const Detection& detection) {
    if (detection.chain.empty()) {
        return format("[DETECT] {}  {}  ts={} pid={}", SeverityName(detection.severity), detection.rule, detection.ts.ms, detection.pid);
    }
    return format("[DETECT] {}  {}  ts={} pid={} chain={}", SeverityName(detection.severity), detection.rule, detection.ts.ms, detection.pid, detection.chain);
}
  
}
