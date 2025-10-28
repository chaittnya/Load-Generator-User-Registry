#pragma once
#include <string>
#include <regex>
#include <chrono>

namespace lg {

inline std::string url_join(const std::string& base, const std::string& path) {
  if (base.empty()) return path;
  if (base.back() == '/' && !path.empty() && path.front() == '/') return base + path.substr(1);
  if (base.back() != '/' && !path.empty() && path.front() != '/') return base + "/" + path;
  return base + path;
}

inline std::string json_extract_id(const std::string& body) {
  std::regex re("\"id\"\\s*:\\s*\"([^\"]+)\"");
  std::smatch m;
  if (std::regex_search(body, m, re) && m.size() > 1) return m[1].str();
  return {};
}

inline long long ns_between(std::chrono::high_resolution_clock::time_point a,
                            std::chrono::high_resolution_clock::time_point b) {
  return std::chrono::duration_cast<std::chrono::nanoseconds>(b - a).count();
}

}
