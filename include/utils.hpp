#pragma once
#include <string>
#include <regex>
#include <chrono>

using namespace std;

inline string url_join(const string& base, const string& path) {
  if (base.empty()) return path;
  if (base.back() == '/' && !path.empty() && path.front() == '/') return base + path.substr(1);
  if (base.back() != '/' && !path.empty() && path.front() != '/') return base + "/" + path;
  return base + path;
}

inline string json_extract_id(const string& body) {
  regex re("\"id\"\\s*:\\s*\"([^\"]+)\"");
  smatch m;
  if (regex_search(body, m, re) && m.size() > 1) return m[1].str();
  return {};
}

inline long long ns_between(chrono::high_resolution_clock::time_point a,
                            chrono::high_resolution_clock::time_point b) {
  return chrono::duration_cast<chrono::nanoseconds>(b - a).count();
}
