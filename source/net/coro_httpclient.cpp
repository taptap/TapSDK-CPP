//
// Created by 甘尧 on 2023/7/5.
//

#include <regex>
#include "base/logging.h"
#include "coro_httpclient.h"
#include "fmt/format.h"
#include "sdk/platform.h"
#include "net/network.h"

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

static std::string ToParam(Params& params) {
    std::string res{};
    bool first{true};
    for (auto& [key, value] : params) {
        if (!first) {
            res.append("&");
        } else {
            res.append("?");
            first = false;
        }
        res.append(fmt::format("{}={}", key, value));
    }
    return std::move(res);
}

static std::string GetHost(const std::string &url) {
    std::regex host_regex(R"(^(?:https?:\/\/)?([^\/\?#]+))");
    std::smatch match;

    if (std::regex_search(url, match, host_regex) && match.size() > 1) {
        std::string host = match.str(1);
        return host;
    } else {
        return {};
    }
}

CoroHttpClient::CoroHttpClient(const char* host, bool https) : TapHttpClient(host, https) {}

std::shared_ptr<cinatra::coro_http_client> CoroHttpClient::AcquireClient() {
    auto create_client = [&] {
        auto client = std::make_shared<cinatra::coro_http_client>();
        client->set_req_timeout(std::chrono::milliseconds(http_timeout_ms));
        client->set_conn_timeout(std::chrono::milliseconds(http_timeout_ms));
        if (https) {
            auto cur_device = platform::Device::GetCurrent();
            auto ca_cert_path = cur_device ? cur_device->GetCaCertDir() : "";
            client->set_sni_hostname(host);
            if (!ca_cert_path.empty()) {
                client->init_ssl(ca_cert_path);
            } else {
                client->init_ssl_content(DEFAULT_CA_PEM);
            }
        }
        return client;
    };
    std::unique_lock guard(client_lock);
    std::shared_ptr<cinatra::coro_http_client> client{};
    if (client_pool.empty()) {
        guard.unlock();
        client = create_client();
    } else {
        client = client_pool.front();
        client_pool.pop();
        client->reset();
        guard.unlock();
    }
    for (auto& [key, value] : common_params) {
        client->add_str_part(key, value);
    }
    for (auto& [key, value] : common_headers) {
        client->add_header(key, value);
    }
    return client;
}

void CoroHttpClient::RecycleClient(const std::shared_ptr<cinatra::coro_http_client>& client) {
    constexpr auto max_pool_size = 10;
    std::unique_lock guard(client_lock);
    if (client_pool.size() < max_pool_size) {
        client_pool.push(client);
    }
}

void CoroHttpClient::CommonHeader(std::string_view key, std::string_view value) {
    ASSERT(!key.empty() && !value.empty());
    common_headers[key.data()] = value;
}

void CoroHttpClient::CommonParam(std::string_view key, std::string_view value) {
    ASSERT(!key.empty() && !value.empty());
    common_params[key.data()] = value;
}

void CoroHttpClient::RequestAsync(HttpType type,
                                  WebPath path,
                                  Headers headers,
                                  Params params,
                                  OnReturn success,
                                  OnFailed failed) {
    WebPath parent{https ? "https://" + host : "http://" + host};
    auto co_type = type == GET ? cinatra::http_method::GET : cinatra::http_method::POST;
    cinatra::req_context<> ctx{cinatra::req_content_type::string, ToParam(params), ""};
    auto co_client = AcquireClient();
    for (auto& [key, value] : headers) {
        co_client->add_header(key, value);
    }
    co_client->async_request(parent / path, co_type, std::move(ctx))
            .start([co_client, this, success, failed](
                           async_simple::Try<cinatra::resp_data> result) {
                if (result.available()) {
                    auto& value = result.value();
                    if (value.status == 200) {
                        auto tap_res = unwrap_result(value.resp_body);
                        if (tap_res.code == 0) {
                            success(tap_res.content);
                        } else {
                            if (failed) {
                                failed(200, tap_res.code, tap_res.msg);
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
                                               WebPath path,
                                               Headers headers,
                                               Params params,
                                               Content content,
                                               ContentType content_type) {
    WebPath parent{https ? "https://" + host : "http://" + host};
    auto co_type = type == GET ? cinatra::http_method::GET : cinatra::http_method::POST;
    auto cina_content_type = cinatra::req_content_type::none;
    if (content_type == ContentType::FORM) {
        cina_content_type = cinatra::req_content_type::form_url_encode;
    } else if (content_type == ContentType::JSON) {
        cina_content_type = cinatra::req_content_type::json;
    }
    cinatra::req_context<Content> ctx{cina_content_type, ToParam(params), content};
    auto co_client = AcquireClient();
    for (auto& [key, value] : headers) {
        co_client->add_header(key, value);
    }
    auto value = co_await co_client->async_request(
            parent / path, co_type, std::move(ctx));
    RecycleClient(co_client);
    if (value.status == 200) {
        auto tap_res = unwrap_result(value.resp_body);
        if (tap_res.code == 0) {
            co_return tap_res.content;
        } else {
            co_return MakeError(200, tap_res.code, tap_res.msg);
        }
    } else {
        co_return MakeError(
                value.status,
                value.net_err.value(),
                !value.resp_body.empty() ? value.resp_body.data() : value.net_err.message());
    }
}

ResultAsync<DownloadResult> DownloadAsync(const char *url, const char *path) {
    cinatra::coro_http_client http_client{};
    auto cur_device = platform::Device::GetCurrent();
    auto ca_cert_path = cur_device ? cur_device->GetCaCertDir() : "";
    http_client.set_req_timeout(std::chrono::milliseconds(http_timeout_ms));
    http_client.set_conn_timeout(std::chrono::milliseconds(http_timeout_ms));
    http_client.set_sni_hostname(GetHost(url));
    if (!ca_cert_path.empty()) {
        http_client.init_ssl(ca_cert_path);
    } else {
        http_client.init_ssl_content(DEFAULT_CA_PEM);
    }
    auto value = co_await http_client.async_download(url, path);
    if (value.status == 200) {
        co_return DownloadResult{};
    } else {
        co_return MakeError(
                value.status,
                value.net_err.value(),
                !value.resp_body.empty() ? value.resp_body.data() : value.net_err.message());
    }
}

}  // namespace tapsdk::net
