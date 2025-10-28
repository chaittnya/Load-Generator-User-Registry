#pragma once
#include <curl/curl.h>
#include <string>

namespace lg {

class HttpClient {
  CURL* h_{nullptr};
  struct curl_slist* hdrs_{nullptr};
  static size_t sink(char* ptr, size_t sz, size_t nm, void* ud) {
    auto* s = static_cast<std::string*>(ud);
    s->append(ptr, sz * nm);
    return sz * nm;
  }
  static long status(CURL* h) {
    long code = 0; curl_easy_getinfo(h, CURLINFO_RESPONSE_CODE, &code); return code;
  }
public:
  HttpClient() {
    h_ = curl_easy_init();
    hdrs_ = curl_slist_append(hdrs_, "Content-Type: application/json");
    curl_easy_setopt(h_, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(h_, CURLOPT_CONNECTTIMEOUT_MS, 2000L);
    curl_easy_setopt(h_, CURLOPT_TIMEOUT_MS, 10000L);
    curl_easy_setopt(h_, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(h_, CURLOPT_HTTPHEADER, hdrs_);
    curl_easy_setopt(h_, CURLOPT_WRITEFUNCTION, sink);
  }
  ~HttpClient() {
    if (hdrs_) curl_slist_free_all(hdrs_);
    if (h_) curl_easy_cleanup(h_);
  }
  bool get(const std::string& url, std::string& out, long& code) {
    out.clear();
    curl_easy_setopt(h_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(h_, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(h_, CURLOPT_WRITEDATA, &out);
    auto rc = curl_easy_perform(h_); 
    code = status(h_);
    return rc == CURLE_OK;
  }
  bool post(const std::string& url, const std::string& body, std::string& out, long& code) {
    out.clear();
    curl_easy_setopt(h_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(h_, CURLOPT_POST, 1L);
    curl_easy_setopt(h_, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(h_, CURLOPT_POSTFIELDSIZE, (long)body.size());
    curl_easy_setopt(h_, CURLOPT_WRITEDATA, &out);
    auto rc = curl_easy_perform(h_); code = status(h_);
    return rc == CURLE_OK;
  }
};

} // namespace lg
