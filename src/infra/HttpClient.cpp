#include "HttpClient.h"

#include "../core/Settings.h"

#include <QCoreApplication>
#include <QString>
#include <QByteArray>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wininet.h>

#pragma comment(lib, "wininet.lib")

#include <vector>
#include <chrono>
#include <thread>

namespace autoflow {
namespace HttpClient {

// 用 WinINET 发送 HTTP/HTTPS 请求（和 PowerShell/.NET 用同一套 API，代理自动处理）
static HttpResponse wininetRequest(const std::string& method, const std::string& url,
                                    const std::string& body, const std::string& contentType,
                                    int timeoutMs,
                                    const std::map<std::string, std::string>& extraHeaders) {
    HttpResponse out;

    // 解析 URL
    URL_COMPONENTS uc = {};
    uc.dwStructSize = sizeof(uc);
    wchar_t host[256] = {};
    wchar_t path[2048] = {};
    uc.lpszHostName = host;
    uc.dwHostNameLength = 256;
    uc.lpszUrlPath = path;
    uc.dwUrlPathLength = 2048;

    std::wstring wurl = QString::fromStdString(url).toStdWString();
    if (!InternetCrackUrl(wurl.c_str(), 0, 0, &uc)) {
        out.error = QCoreApplication::translate("HttpClient", "URL 格式错误: %1")
                        .arg(QString::fromStdString(url)).toStdString();
        return out;
    }

    // 创建 session（直连，不走系统代理）
    HINTERNET hSession = InternetOpenW(L"AutoFlow/1.0", INTERNET_OPEN_TYPE_DIRECT,
                                        nullptr, nullptr, 0);
    if (!hSession) {
        out.error = QCoreApplication::translate("HttpClient", "InternetOpen 失败").toStdString();
        return out;
    }

    // 设置超时
    InternetSetOptionW(hSession, INTERNET_OPTION_CONNECT_TIMEOUT, &timeoutMs, sizeof(timeoutMs));
    InternetSetOptionW(hSession, INTERNET_OPTION_SEND_TIMEOUT, &timeoutMs, sizeof(timeoutMs));
    InternetSetOptionW(hSession, INTERNET_OPTION_RECEIVE_TIMEOUT, &timeoutMs, sizeof(timeoutMs));

    // 忽略 SSL 证书错误（某些自签名证书网关需要）
    DWORD flags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE;
    if (uc.nScheme == INTERNET_SCHEME_HTTPS) flags |= INTERNET_FLAG_SECURE;

    // 连接服务器
    HINTERNET hConnect = InternetConnectW(hSession, host, uc.nPort,
                                            nullptr, nullptr,
                                            INTERNET_SERVICE_HTTP, flags, 0);
    if (!hConnect) {
        out.error = QCoreApplication::translate("HttpClient", "无法连接到服务器: %1")
                        .arg(QString::fromWCharArray(host)).toStdString();
        InternetCloseHandle(hSession);
        return out;
    }

    // 创建请求
    std::wstring wmethod = QString::fromStdString(method).toUpper().toStdWString();
    HINTERNET hRequest = HttpOpenRequestW(hConnect, wmethod.c_str(), path,
                                            nullptr, nullptr, nullptr, flags, 0);
    if (!hRequest) {
        out.error = QCoreApplication::translate("HttpClient", "HttpOpenRequest 失败").toStdString();
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hSession);
        return out;
    }

    // 忽略 SSL 证书错误
    DWORD secFlags = SECURITY_FLAG_IGNORE_UNKNOWN_CA |
                     SECURITY_FLAG_IGNORE_CERT_DATE_INVALID |
                     SECURITY_FLAG_IGNORE_CERT_CN_INVALID |
                     SECURITY_FLAG_IGNORE_WRONG_USAGE;
    InternetSetOptionW(hRequest, INTERNET_OPTION_SECURITY_FLAGS, &secFlags, sizeof(secFlags));

    // 添加 headers
    std::wstring headers;
    if (!contentType.empty()) {
        headers += L"Content-Type: " + QString::fromStdString(contentType).toStdWString() + L"\r\n";
    }
    for (const auto& kv : extraHeaders) {
        headers += QString::fromStdString(kv.first).toStdWString() + L": " +
                   QString::fromStdString(kv.second).toStdWString() + L"\r\n";
    }

    // 发送请求
    const char* bodyData = body.empty() ? nullptr : body.c_str();
    DWORD bodySize = (DWORD)body.size();
    BOOL bSend = HttpSendRequestW(hRequest,
                                   headers.empty() ? nullptr : headers.c_str(),
                                   (DWORD)headers.size(),
                                   (LPVOID)bodyData, bodySize);
    if (!bSend) {
        DWORD err = GetLastError();
        out.error = QCoreApplication::translate("HttpClient", "HTTP 请求失败（错误码 %1）")
                        .arg((int)err).toStdString();
        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hSession);
        return out;
    }

    // 获取状态码
    DWORD statusCode = 0;
    DWORD statusCodeSize = sizeof(statusCode);
    HttpQueryInfoW(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER,
                    &statusCode, &statusCodeSize, nullptr);
    out.status = (int)statusCode;

    // 读取响应 body
    DWORD bytesAvailable = 0;
    DWORD qflags = 0;
    DWORD_PTR qctx = 0;
    while (InternetQueryDataAvailable(hRequest, &bytesAvailable, qflags, qctx) && bytesAvailable > 0) {
        std::vector<char> buf(bytesAvailable);
        DWORD bytesRead = 0;
        if (InternetReadFile(hRequest, buf.data(), bytesAvailable, &bytesRead) && bytesRead > 0) {
            out.body.append(buf.data(), (size_t)bytesRead);
        } else {
            break;
        }
    }

    out.ok = (out.status >= 200 && out.status < 300);
    if (out.ok) {
        out.error.clear();
    } else if (out.body.empty()) {
        out.error = QCoreApplication::translate("HttpClient", "HTTP 状态码 %1").arg(out.status).toStdString();
    } else {
        out.error = out.body;   // 服务器返回的错误 JSON
    }

    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hSession);
    return out;
}

HttpResponse request(const std::string& method, const std::string& url,
                     const std::string& body, const std::string& contentType, int timeoutMs) {
    return request(method, url, body, contentType, timeoutMs, {});
}

HttpResponse request(const std::string& method, const std::string& url,
                     const std::string& body, const std::string& contentType, int timeoutMs,
                     const std::map<std::string, std::string>& extraHeaders) {
    if (timeoutMs <= 0) timeoutMs = Settings::instance().httpTimeoutMs();
    const int maxRetries = Settings::instance().httpRetry();

    for (int attempt = 0;; ++attempt) {
        HttpResponse out = wininetRequest(method, url, body, contentType, timeoutMs, extraHeaders);
        if (out.ok) return out;
        if (attempt >= maxRetries) return out;
        std::this_thread::sleep_for(std::chrono::milliseconds(200 * (attempt + 1)));
    }
}

HttpResponse get(const std::string& url, int timeoutMs) {
    return request("GET", url, "", "", timeoutMs);
}

HttpResponse post(const std::string& url, const std::string& body,
                  const std::string& contentType, int timeoutMs) {
    return request("POST", url, body, contentType, timeoutMs);
}

} // namespace HttpClient
} // namespace autoflow
