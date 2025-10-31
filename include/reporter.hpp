#pragma once
#include <atomic>
#include <cstdint>
#include "metrics.hpp"

using namespace std;

struct Snapshot
{
  uint64_t ok, fail;
  long long total_ns;
};

void stats_reporter(atomic<bool> &stop, GlobalStats &gs);