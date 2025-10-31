#include "workloads.hpp"
#include <random>
#include <sstream>
#include <iostream>
#include <chrono>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

using namespace std;


bool seed_get_all_ids(Config &cfg, int n, std::vector<std::string> &ids_out)
{
  ids_out.clear();
  if (n <= 0)
    return false;

  HttpClient c;
  long code = 0;
  std::string resp;

  // /users/random?n=<n>
  std::ostringstream url;
  url << url_join(cfg.base, "/users/random") << "?n=" << n;

  if (!c.get(url.str(), resp, code) || code != 200)
  {
    std::cerr << "seed_get_all_ids: request failed or code != 200\n";
    return false;
  }

  json j = json::parse(resp, nullptr, false);
  if (j.is_discarded())
  {
    std::cerr << "seed_get_all_ids: invalid JSON\n";
    return false;
  }

  if (!j.contains("ids") || !j["ids"].is_array())
  {
    std::cerr << "seed_get_all_ids: JSON missing 'ids' array\n";
    return false;
  }
  cout << "IDs received : " << j["ids"].size() << endl;
  ids_out.reserve(j["ids"].size());
  for (auto &x : j["ids"])
  {
    if (x.is_string())
      ids_out.push_back(x.get<std::string>());
    else if (x.is_number())
      ids_out.push_back(std::to_string(x.get<long long>()));
  }

  return !ids_out.empty();
}


void worker_put_all(Config &cfg, atomic<bool> &stop, GlobalStats &gs, int tid)
{
  HttpClient c;
  uint64_t seq = 0;
  long code = 0;
  string out;
  while (!stop.load(memory_order_relaxed))
  {
    ostringstream name, mobile, payload;
    name << "load_" << tid << "_" << seq;
    mobile << "77" << tid << (100000 + (seq % 900000));
    payload << "{\"name\":\"" << name.str() << "\",\"mobile\":\"" << mobile.str() << "\"}";
    auto url = url_join(cfg.base, "/users");

    auto t0 = chrono::high_resolution_clock::now();
    bool ok = c.post(url, payload.str(), out, code);
    auto t1 = chrono::high_resolution_clock::now();

    if (ok && (code == 201 || code == 200))
    {
      gs.ok.fetch_add(1, memory_order_relaxed);
      gs.total_ns.fetch_add(ns_between(t0, t1), memory_order_relaxed);
    }
    else
      gs.fail.fetch_add(1, memory_order_relaxed);
    ++seq;
  }
}

void worker_get_all(Config &cfg, atomic<bool> &stop, GlobalStats &gs, IdFeeder &feeder)
{
  if (feeder.ids.empty())
    return;
  HttpClient c;
  long code = 0;
  string out;
  auto N = feeder.ids.size();
  while (!stop.load(memory_order_relaxed))
  {
    auto i = feeder.next.fetch_add(1, memory_order_relaxed);
    auto &id = feeder.ids[i % N];
    auto url = url_join(cfg.base, "/users/" + id);

    auto t0 = chrono::high_resolution_clock::now();
    bool ok = c.get(url, out, code);
    auto t1 = chrono::high_resolution_clock::now();

    if (ok && code == 200)
    {
      gs.ok.fetch_add(1, memory_order_relaxed);
      gs.total_ns.fetch_add(ns_between(t0, t1), memory_order_relaxed);

      if (out.find("\"source\":\"cache\"") != std::string::npos)
        gs.fromcache.fetch_add(1, std::memory_order_relaxed);
      else if (out.find("\"source\":\"db\"") != std::string::npos)
        gs.fromdb.fetch_add(1, std::memory_order_relaxed);
      gs.fail.fetch_add(1, memory_order_relaxed);
    }
  }
}

void worker_get_put(Config &cfg, atomic<bool> &stop, GlobalStats &gs, IdFeeder &feeder)
{
  HttpClient c;
  long code = 0;
  string out;
  mt19937 rng(random_device{}());
  uniform_real_distribution<double> coin(0.0, 1.0);
  auto N = feeder.ids.size();
  uint64_t seq = 0;

  while (!stop.load(memory_order_relaxed))
  {
    bool do_read = coin(rng) < cfg.read_ratio && N > 0;
    if (do_read)
    {
      auto i = feeder.next.fetch_add(1, memory_order_relaxed);
      auto &id = feeder.ids[i % N];
      auto url = url_join(cfg.base, "/users/" + id);

      auto t0 = chrono::high_resolution_clock::now();
      bool ok = c.get(url, out, code);
      auto t1 = chrono::high_resolution_clock::now();

      if (ok && code == 200)
      {
        gs.ok.fetch_add(1, memory_order_relaxed);
        gs.total_ns.fetch_add(ns_between(t0, t1), memory_order_relaxed);
      }
      else
      {
        gs.fail.fetch_add(1, memory_order_relaxed);
      }
    }
    else
    {
      ostringstream name, mobile, payload;
      name << "mix_" << seq;
      mobile << "+9166" << (100000 + (seq % 900000));
      payload << "{\"name\":\"" << name.str() << "\",\"mobile\":\"" << mobile.str() << "\"}";
      auto url = url_join(cfg.base, "/users");

      auto t0 = chrono::high_resolution_clock::now();
      bool ok = c.post(url, payload.str(), out, code);
      auto t1 = chrono::high_resolution_clock::now();

      if (ok && (code == 201 || code == 200))
      {
        gs.ok.fetch_add(1, memory_order_relaxed);
        gs.total_ns.fetch_add(ns_between(t0, t1), memory_order_relaxed);
      }
      else
        gs.fail.fetch_add(1, memory_order_relaxed);
      ++seq;
    }
  }
}


void worker_token(Config &cfg, atomic<bool> &stop, GlobalStats &gs, int tid)
{
  HttpClient c;
  long code = 0;
  string out;
  uint64_t seq = 0;
  while (!stop.load(memory_order_relaxed))
  {
    ostringstream payload;
    payload << "{\"mobile\":\"99" << tid << (100000 + (seq % 900000)) << "\","
            << "\"iterations\":" << cfg.token_iterations << "}";

    auto url = url_join(cfg.base, "/token");
    auto t0 = chrono::high_resolution_clock::now();
    bool ok = c.post(url, payload.str(), out, code);
    auto t1 = chrono::high_resolution_clock::now();

    if (ok && code == 200)
    {
      gs.ok.fetch_add(1, memory_order_relaxed);
      gs.total_ns.fetch_add(ns_between(t0, t1), memory_order_relaxed);
    }
    else
    {
      gs.fail.fetch_add(1, memory_order_relaxed);
    }

    ++seq;
  }
}
