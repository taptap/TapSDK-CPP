//
// Created by 甘尧 on 2023/7/5.
//

#include "base/logging.h"
#include "coro_httpclient.h"
#include "fmt/format.h"

namespace tapsdk::net {

static std::unordered_map<std::string, std::string> ToHeader(Headers& headers) {
    std::unordered_map<std::string, std::string> res{};
    for (auto& [key, value] : headers) {
        res[key] = value;
    }
    return std::move(res);
}

static std::string ToParam(Params& params) {
    std::string res{};
    bool first{true};
    for (auto& [key, value] : params) {
        if (!first) {
            res.append("?");
            first = false;
        }
        res.append(fmt::format("{}={}", key, value));
    }
    return std::move(res);
}

CoroHttpClient::CoroHttpClient(const char* host, bool https) : TapHttpClient(host, https) {
    co_client.set_req_timeout(std::chrono::milliseconds(http_timeout_ms));
    co_client.set_conn_timeout(std::chrono::milliseconds(http_timeout_ms));
}

void CoroHttpClient::CommonHeader(const char* key, const char* value) {
    ASSERT(key && value);
    co_client.add_header(key, value);
}

void CoroHttpClient::CommonParam(const char* key, const char* value) {
    ASSERT(key && value);
    co_client.add_str_part(key, value);
}

void CoroHttpClient::RequestAsync(HttpType type,
                                  const WebPath& path,
                                  Headers headers,
                                  Params params,
                                  OnReturn success,
                                  OnFailed failed) {
    WebPath parent{https ? "https://" + host : "http://" + host};
    auto co_type = type == GET ? cinatra::http_method::GET : cinatra::http_method::POST;
    cinatra::req_context<> ctx{cinatra::req_content_type::string, ToParam(params), ""};
    co_client.async_request(parent / path, co_type, std::move(ctx), ToHeader(headers))
            .start([success, failed](async_simple::Try<cinatra::resp_data> result) {
                if (result.available()) {
                    auto& value = result.value();
                    if (value.status == 200) {
                        ResultWrap tap_res{value.resp_body.data()};
                        if (tap_res.GetCode() == 0) {
                            success(tap_res.GetContent());
                        } else {
                            if (failed) {
                                failed(200, tap_res.GetCode(), tap_res.GetMsg());
                            }
                        }
                    } else {
                        if (failed) {
                            failed(value.status, value.net_err.value(), value.resp_body.data());
                        }
                    }
                } else {
                    if (failed) {
                        failed(-1, -1, "Unk!");
                    }
                }
            });
}

ResultAsync<std::string> CoroHttpClient::RequestAsync(tapsdk::net::HttpType type,
                                                      const tapsdk::WebPath& path,
                                                      tapsdk::net::Headers headers,
                                                      tapsdk::net::Params params) {
    WebPath parent{https ? "https://" + host : "http://" + host};
    auto co_type = type == GET ? cinatra::http_method::GET : cinatra::http_method::POST;
    cinatra::req_context<> ctx{cinatra::req_content_type::string, ToParam(params), ""};
    auto value = co_await co_client.async_request(parent / path, co_type, std::move(ctx), ToHeader(headers));
    if (value.status == 200) {
        ResultWrap tap_res{value.resp_body.data()};
        if (tap_res.GetCode() == 0) {
            co_return tap_res.GetContent();
        } else {
            co_return MakeError(200, tap_res.GetCode(), tap_res.GetMsg());
        }
    } else {
        co_return MakeError(value.status, value.net_err.value(), value.resp_body.data());
    }
}

}  // namespace tapsdk::net
