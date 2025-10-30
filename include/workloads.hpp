#pragma once
#include "config.hpp"
#include "metrics.hpp"
#include "http_client.hpp"
#include "utils.hpp"
#include <atomic>
#include <thread>
#include <vector>
#include <string>

using namespace std;

struct IdFeeder { 
    vector<string> ids; 
    atomic<size_t> next{0}; 
};

// Seeding helpers
bool seed_popular(const Config& cfg);
bool seed_get_all_ids(const Config& cfg, int n, vector<string>& ids_out);

// Workers
void worker_put_all(const Config& cfg, atomic<bool>& stop, GlobalStats& gs, int tid);
void worker_get_all(const Config& cfg, atomic<bool>& stop, GlobalStats& gs, IdFeeder& feeder);
void worker_get_popular(const Config& cfg, atomic<bool>& stop, GlobalStats& gs);
void worker_get_put(const Config& cfg, atomic<bool>& stop, GlobalStats& gs, IdFeeder& feeder);

// NEW: token workload
void worker_token(const Config& cfg, atomic<bool>& stop, GlobalStats& gs, int tid);

