//
// Created by 甘尧 on 2023/7/5.
//

#include "base/logging.h"
#include "coro_httpclient.h"
#include "fmt/format.h"
#include "sdk/platform.h"

const static auto DEFAULT_CA_PEM =
        "-----BEGIN CERTIFICATE-----\n"
        "MIIDKDCCAhACCQDHu0UVVUEr4DANBgkqhkiG9w0BAQsFADBWMQswCQYDVQQGEwJD\n"
        "TjEVMBMGA1UEBwwMRGVmYXVsdCBDaXR5MRwwGgYDVQQKDBNEZWZhdWx0IENvbXBh\n"
        "bnkgTHRkMRIwEAYDVQQDDAlsb2NhbGhvc3QwHhcNMjIxMDI1MDM1NzMwWhcNMzIx\n"
        "MDIyMDM1NzMwWjBWMQswCQYDVQQGEwJDTjEVMBMGA1UEBwwMRGVmYXVsdCBDaXR5\n"
        "MRwwGgYDVQQKDBNEZWZhdWx0IENvbXBhbnkgTHRkMRIwEAYDVQQDDAlsb2NhbGhv\n"
        "c3QwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCr6iWgRRYJ9QfKSUPT\n"
        "nbw2rKZRlSBqnLeLdPam+s8RUA1p+YPoH2HJqIdxcfYmToz5t6G5OX8TFhAssShw\n"
        "PalRlQm5QHp4pL7nqPV79auB3PYKv6TgOumwDUpoBxcu0l9di9fjYbC2LmpVJeVz\n"
        "WQxCo+XO/g5YjXN1nPPeBgmZVkRvXLIYCTKshLlUa0nW7hj7Sl8CAV8OBNMBFkf1\n"
        "2vgcTqhs3yW9gnIwIoCFZvsdAsSbwR6zF1z96MeAYDIZWeyzUXkoZa4OCWwAhqzo\n"
        "+0JWukuNuHhsQhIJDvIZWHEblT0GlentP8HPXjFnJHYGUAjx3Fj1mH8mFG0fEXXN\n"
        "06qlAgMBAAEwDQYJKoZIhvcNAQELBQADggEBAGbKTy1mfSlJF012jKuIue2valI2\n"
        "CKz8X619jmxxIzk0k7wcmAlUUrUSFIzdIddZj92wYbBC1YNOWQ4AG5zpFo3NAQaZ\n"
        "kYGnlt+d2pNHLaT4IV9JM4iwTqPyi+FsOwTjUGHgaOr+tfK8fZmPbDmAE46OlC/a\n"
        "VVqNPmjaJiM2c/pJOs+HV9PvEOFmV9p5Yjjz4eV3jwqHdOcxZuLJl28/oqz65uCu\n"
        "LQiivkdVCuwc1IlpRFejkrbkrk28XCCJwokLt03EQj4xs0sjoTKgd92fpjls/tt+\n"
        "rw+7ILsAsuoWPIdiuCArCU1LXJDz3FDHafX/dxzdVBzpfVgP0rNpS050Mls=\n"
        "-----END CERTIFICATE-----";

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

CoroHttpClient::CoroHttpClient(const char* host, bool https) : TapHttpClient(host, https) {}

std::shared_ptr<cinatra::coro_http_client> CoroHttpClient::AcquireClient() {
    auto create_client = [&] {
        auto client = std::make_shared<cinatra::coro_http_client>();
        for (auto& [key, value] : headers) {
            client->add_header(key, value);
        }
        for (auto& [key, value] : params) {
            client->add_str_part(key, value);
        }
        client->set_req_timeout(std::chrono::milliseconds(http_timeout_ms));
        client->set_conn_timeout(std::chrono::milliseconds(http_timeout_ms));
        if (https) {
            auto cur_device = platform::Device::GetCurrent();
            auto ca_cert_path = cur_device ? cur_device->GetCaCertDir() : "";
            client->set_·(host);
            if (!ca_cert_path.empty()) {
                client->init_ssl(ca_cert_path);
            } else {
                client->init_ssl_content(DEFAULT_CA_PEM);
            }
        }
        return client;
    };
    std::unique_lock guard(client_lock);
    if (client_pool.empty()) {
        guard.unlock();
        return create_client();
    } else {
        auto client = client_pool.front();
        client_pool.pop();
        client->reset();
        guard.unlock();
        return client;
    }
}

void CoroHttpClient::RecycleClient(const std::shared_ptr<cinatra::coro_http_client>& client) {
    constexpr auto max_pool_size = 10;
    std::unique_lock guard(client_lock);
    if (client_pool.size() < max_pool_size) {
        client_pool.push(client);
    }
}

void CoroHttpClient::CommonHeader(const char* key, const char* value) {
    ASSERT(key && value);
    headers[key] = value;
}

void CoroHttpClient::CommonParam(const char* key, const char* value) {
    ASSERT(key && value);
    params[key] = value;
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
    auto co_client = AcquireClient();
    co_client->async_request(parent / path, co_type, std::move(ctx), ToHeader(headers))
            .start([co_client, this, success, failed](
                           async_simple::Try<cinatra::resp_data> result) {
                if (result.available()) {
                    auto& value = result.value();
                    if (value.status == 200) {
                        ResultWrap tap_res{value.resp_body};
                        if (tap_res.GetCode() == 0) {
                            success(tap_res.GetContent());
                        } else {
                            if (failed) {
                                failed(200, tap_res.GetCode(), tap_res.GetMsg());
                            }
                        }
                    } else {
                        if (failed) {
                            failed(value.status,
                                   value.net_err.value(),
                                   !value.resp_body.empty() ? value.resp_body.data() : "");
                        }
                    }
                } else {
                    if (failed) {
                        failed(-1, -1, "Unk!");
                    }
                }
                RecycleClient(co_client);
            });
}

ResultAsync<Json> CoroHttpClient::RequestAsync(HttpType type,
                                               const WebPath& path,
                                               Headers headers,
                                               Params params,
                                               const Json& content) {
    WebPath parent{https ? "https://" + host : "http://" + host};
    auto co_type = type == GET ? cinatra::http_method::GET : cinatra::http_method::POST;
    cinatra::req_context<> ctx{
            content.empty() ? cinatra::req_content_type::none : cinatra::req_content_type::json,
            ToParam(params),
            content.empty() ? "" : content.dump()};
    auto co_client = AcquireClient();
    auto value = co_await co_client->async_request(
            parent / path, co_type, std::move(ctx), ToHeader(headers));
    RecycleClient(co_client);
    if (value.status == 200) {
        ResultWrap tap_res{value.resp_body};
        if (tap_res.GetCode() == 0) {
            co_return tap_res.GetContent();
        } else {
            co_return MakeError(200, tap_res.GetCode(), tap_res.GetMsg());
        }
    } else {
        co_return MakeError(value.status,
                            value.net_err.value(),
                            !value.resp_body.empty() ? value.resp_body.data() : value.net_err.message());
    }
}

}  // namespace tapsdk::net
