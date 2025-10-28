#pragma once
#include <atomic>
#include <cstdint>

namespace lg {

struct GlobalStats {
  std::atomic<uint64_t> ok{0};
  std::atomic<uint64_t> fail{0};
  std::atomic<long long> total_ns{0};
};

} // namespace lg
