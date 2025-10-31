#pragma once
#include <string>

using namespace std;

enum class Workload
{
  PutAll,
  GetAll,
  GetPut,
  Token
};

struct Config
{
  string base;
  int threads{8};
  int duration_s{30};
  Workload wl{Workload::GetAll};
  int popular_n{32};
  double read_ratio{0.7};
  int seed_get_all{2000};
  int token_iterations{700000};
};
