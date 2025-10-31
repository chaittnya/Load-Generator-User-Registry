#pragma once
#include <iostream>
#include <curl/curl.h>
#include <string>
#include <unordered_map>

using namespace std;

struct HttpResp
{
  long code = 0;
  string body;
  unordered_map<string, string> headers;
};

class HttpClient
{
  CURL *request{nullptr};
  struct curl_slist *headers{nullptr};

  static size_t sink(char *ptr, size_t sz, size_t nm, void *ud)
  {
    auto *s = static_cast<string *>(ud);
    s->append(ptr, sz * nm);
    return sz * nm;
  }

  static long status(CURL *h)
  {
    long code = 0;
    curl_easy_getinfo(h, CURLINFO_RESPONSE_CODE, &code);
    return code;
  }

public:
  HttpClient()
  {
    request = curl_easy_init();
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(request, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(request, CURLOPT_CONNECTTIMEOUT_MS, 2000L);
    curl_easy_setopt(request, CURLOPT_TIMEOUT_MS, 10000L);
    curl_easy_setopt(request, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(request, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(request, CURLOPT_WRITEFUNCTION, sink);
  }
  ~HttpClient()
  {
    if (headers)
      curl_slist_free_all(headers);
    if (request)
      curl_easy_cleanup(request);
  }
  bool get(const string &url, string &out, long &code)
  {
    out.clear();
    curl_easy_setopt(request, CURLOPT_URL, url.c_str());
    curl_easy_setopt(request, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(request, CURLOPT_WRITEDATA, &out);
    auto rc = curl_easy_perform(request);
    code = status(request);
    return rc == CURLE_OK;
  }
  bool post(string &url, const string &body, string &out, long &code)
  {
    out.clear();
    curl_easy_setopt(request, CURLOPT_URL, url.c_str());
    curl_easy_setopt(request, CURLOPT_POST, 1L);
    curl_easy_setopt(request, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(request, CURLOPT_POSTFIELDSIZE, (long)body.size());
    curl_easy_setopt(request, CURLOPT_WRITEDATA, &out);
    auto rc = curl_easy_perform(request);
    code = status(request);
    return rc == CURLE_OK;
  }
};
