#pragma once
#include <atomic>
#include <cstdint>

using namespace std;

struct GlobalStats
{
  atomic<uint64_t> ok{0};
  atomic<uint64_t> fail{0};
  atomic<uint64_t> fromdb{0};
  atomic<uint64_t> fromcache{0};
  atomic<uint64_t> fromunknown{0};
  atomic<long long> total_ns{0};
};
