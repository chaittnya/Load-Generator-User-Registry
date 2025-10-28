#pragma once
#include "config.hpp"
#include "metrics.hpp"
#include "http_client.hpp"
#include "utils.hpp"
#include <atomic>
#include <thread>
#include <vector>
#include <string>

namespace lg {

// Seeding helpers
bool seed_popular(const Config& cfg);
bool seed_get_all_ids(const Config& cfg, int n, std::vector<std::string>& ids_out);

// Workers
void worker_put_all(const Config& cfg, std::atomic<bool>& stop, GlobalStats& gs, int tid);
struct IdFeeder { std::vector<std::string> ids; std::atomic<size_t> next{0}; };
void worker_get_all(const Config& cfg, std::atomic<bool>& stop, GlobalStats& gs, IdFeeder& feeder);
void worker_get_popular(const Config& cfg, std::atomic<bool>& stop, GlobalStats& gs);
void worker_get_put(const Config& cfg, std::atomic<bool>& stop, GlobalStats& gs, IdFeeder& feeder);

// NEW: token workload
void worker_token(const Config& cfg, std::atomic<bool>& stop, GlobalStats& gs, int tid);

} // namespace lg
