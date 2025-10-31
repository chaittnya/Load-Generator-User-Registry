#include <thread>
#include <atomic>
#include <chrono>
#include <iostream>
#include "reporter.hpp"

using namespace std;

using namespace chrono_literals;

static inline Snapshot snap(GlobalStats &gs)
{
  return {
      gs.ok.load(memory_order_relaxed),
      gs.fail.load(memory_order_relaxed),
      gs.total_ns.load(memory_order_relaxed)};
}

void stats_reporter(atomic<bool> &stop, GlobalStats &gs)
{
  using clock = chrono::steady_clock;
  auto prev = snap(gs);
  auto prev_t = clock::now();

  while (!stop.load(memory_order_relaxed))
  {
    this_thread::sleep_for(1s);

    auto now = clock::now();
    auto cur = snap(gs);
    chrono::duration<double> dt = now - prev_t;
    double secs = dt.count();

    uint64_t d_ok = cur.ok - prev.ok;
    uint64_t d_fail = cur.fail - prev.fail;
    long long d_ns = cur.total_ns - prev.total_ns;

    double throughput_rps = (d_ok + d_fail) / secs;
    double avg_ms = d_ok ? ((double)d_ns / 1e6) / (double)d_ok : 0.0;

    cout.setf(ios::fixed);
    cout.precision(2);
    cout << "throughput=" << throughput_rps << " req/s, "
         << "avg=" << avg_ms << " ms, "
         << "ok=" << d_ok << ", fail=" << d_fail << '\n';

    prev = cur;
    prev_t = now;
  }
}