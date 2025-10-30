#pragma once
#include <string>

using namespace std;

enum class Workload { PutAll, GetAll, GetPopular, GetPut, Token };

struct Config {
  string base;         // e.g. http://localhost:8080
  int threads{8};
  int duration_s{30};
  Workload wl{Workload::GetAll};
  int popular_n{32};        // for /popular/seed
  double read_ratio{0.7};   // get_put mix
  int seed_get_all{2000};   // seed size for get_all/get_put
  int token_iterations{700000}; // PBKDF2 iterations
};

