#include "config.hpp"
#include "metrics.hpp"
#include "utils.hpp"
#include "workloads.hpp"
#include <curl/curl.h>
#include <iostream>
#include <thread>
#include <vector>
#include <optional>
#include <chrono>

using namespace std;

static void usage(const char* prog) {
  cerr <<
    "Usage: " << prog
    << " --base http://localhost:8080 --threads 8 --duration 30"
    << " --workload [put_all|get_all|get_popular|get_put|token]"
    << " [--popular_n 32] [--read_ratio 0.7] [--seed_get_all 2000] [--token_iterations 700000]\n";
}

static optional<Config> parse_args(int argc, char** argv) {
  Config cfg;
  for (int i=1; i<argc; ++i) {
    string a = argv[i];
    auto need = [&](const char*){ if (i+1>=argc) { usage(argv[0]); exit(1);} return string(argv[++i]); };
    if (a=="--base") cfg.base = need("--base");
    else if (a=="--threads") cfg.threads = stoi(need("--threads"));
    else if (a=="--duration") cfg.duration_s = stoi(need("--duration"));
    else if (a=="--workload") {
      string w = need("--workload");
      if (w=="put_all") cfg.wl = Workload::PutAll;
      else if (w=="get_all") cfg.wl = Workload::GetAll;
      else if (w=="get_popular") cfg.wl = Workload::GetPopular;
      else if (w=="get_put") cfg.wl = Workload::GetPut;
      else if (w=="token") cfg.wl = Workload::Token;
      else { cerr << "Unknown workload\n"; return nullopt; }
    }
    else if (a=="--popular_n") cfg.popular_n = stoi(need("--popular_n"));
    else if (a=="--read_ratio") cfg.read_ratio = stod(need("--read_ratio"));
    else if (a=="--seed_get_all") cfg.seed_get_all = stoi(need("--seed_get_all"));
    else if (a=="--token_iterations") cfg.token_iterations = stoi(need("--token_iterations"));
    else { cerr << "Unknown arg: " << a << "\n"; return nullopt; }
  }
  if (cfg.base.empty()) { usage(argv[0]); return nullopt; }
  if (cfg.threads <= 0) cfg.threads = 1;
  if (cfg.duration_s <= 0) cfg.duration_s = 10;
  if (cfg.read_ratio < 0) cfg.read_ratio = 0;
  if (cfg.read_ratio > 1) cfg.read_ratio = 1;
  if (cfg.token_iterations < 1000) cfg.token_iterations = 1000;
  return cfg;
}

int main(int argc, char** argv) {
  auto cfgopt = parse_args(argc, argv);
  if (!cfgopt) return 1;
  Config cfg = *cfgopt;

  curl_global_init(CURL_GLOBAL_DEFAULT);

  // Pre-seed per workload
  IdFeeder feeder;
  if (cfg.wl == Workload::GetAll || cfg.wl == Workload::GetPut) {
    if (!seed_get_all_ids(cfg, cfg.seed_get_all, feeder.ids))
      cerr << "Warning: seeding ids failed\n";
    else
      cerr << "Seeded " << feeder.ids.size() << " ids\n";
  }

  GlobalStats gs;
  atomic<bool> stop{false};
  vector<thread> ts;
  ts.reserve(cfg.threads);

  auto start = chrono::steady_clock::now();
  auto end_at = start + chrono::seconds(cfg.duration_s);

  for (int t=0; t<cfg.threads; ++t) {
    ts.emplace_back([&, t](){
      switch (cfg.wl) {
        case Workload::PutAll:     worker_put_all(cfg, stop, gs, t); break;
        case Workload::GetAll:     worker_get_all(cfg, stop, gs, feeder); break;
        case Workload::GetPut:     worker_get_put(cfg, stop, gs, feeder); break;
        case Workload::Token:      worker_token(cfg, stop, gs, t); break;
      }
    });
  }

  while (chrono::steady_clock::now() < end_at)
    this_thread::sleep_for(chrono::milliseconds(100));
  stop.store(true, memory_order_relaxed);

  for (auto& th : ts) th.join();

  auto end = chrono::steady_clock::now();
  double secs = chrono::duration<double>(end - start).count();
  auto ok = gs.ok.load();
  auto fail = gs.fail.load();
  auto fromdb = gs.fromdb.load();
  auto fromcache = gs.fromcache.load();
  auto total_ns = gs.total_ns.load();
  double thr = ok / secs;
  double avg_ms = ok ? (total_ns / 1e6) / ok : 0.0;

  cout << "==== Load Results ====\n";
  cout << "Workload         : ";
  switch (cfg.wl) {
    case Workload::PutAll:     cout << "put_all"; break;
    case Workload::GetAll:     cout << "get_all"; break;
    case Workload::GetPopular: cout << "get_popular"; break;
    case Workload::GetPut:     cout << "get_put"; break;
    case Workload::Token:      cout << "token"; break;
  }
  cout << "\nThreads          : " << cfg.threads
            << "\nDuration (s)     : " << cfg.duration_s
            << "\nSuccess          : " << ok
            << "\nFailures         : " << fail
            << "\nFetched from DB  : " << fromdb
            << "\nFetched from Cache: " << fromcache
            << "\nThroughput (req/s): " << thr
            << "\nAvg latency (ms) : " << avg_ms
            << "\n======================\n";

  curl_global_cleanup();
  return 0;
}
