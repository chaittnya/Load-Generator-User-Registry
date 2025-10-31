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

struct IdFeeder
{
    vector<string> ids;
    atomic<size_t> next{0};
};

bool seed_get_all_ids(Config &cfg, int n, vector<string> &ids_out);

// Workers
void worker_put_all(Config &cfg, atomic<bool> &stop, GlobalStats &gs, int tid);
void worker_get_all(Config &cfg, atomic<bool> &stop, GlobalStats &gs, IdFeeder &feeder);
void worker_get_put(Config &cfg, atomic<bool> &stop, GlobalStats &gs, IdFeeder &feeder);
void worker_token(Config &cfg, atomic<bool> &stop, GlobalStats &gs, int tid);
