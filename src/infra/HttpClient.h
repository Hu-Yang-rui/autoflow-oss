#pragma once
#include "../common.h"

namespace autoflow {

// HTTP 客户端（cpp-httplib 封装）
struct HttpResponse {
    int status = 0;
    std::string body;
    bool ok = false;
    std::string error;
};

namespace HttpClient {

// timeoutMs <= 0 时使用 Settings::httpTimeoutMs()；失败时按 Settings::httpRetry() 重试
HttpResponse get(const std::string& url, int timeoutMs = 0);
HttpResponse post(const std::string& url, const std::string& body,
                  const std::string& contentType = "application/json", int timeoutMs = 0);
HttpResponse request(const std::string& method, const std::string& url,
                     const std::string& body, const std::string& contentType, int timeoutMs);
HttpResponse request(const std::string& method, const std::string& url,
                     const std::string& body, const std::string& contentType, int timeoutMs,
                     const std::map<std::string, std::string>& extraHeaders);

} // namespace HttpClient
} // namespace autoflow
