#include "arthub_client.h"
#include <chrono>
#include <sstream>
#include <thread>
#include "httplib.h"
#include "url.h"
#include "utils/json_maker.h"
#include "utils/logger.h"
#include "utils/string_util.h"

constexpr int total_retry = 5;

ArthubClient::ArthubClient(const std::shared_ptr<ArthubConfig>& in_config)
    : config(in_config),
      http_client(std::make_shared<httplib::Client>(in_config->endpoint.c_str())) {
    if (!config->http_proxy_host.empty()) {
        http_client->set_proxy(config->http_proxy_host.c_str(), config->http_proxy_port);
        LOG_INFO("using proxy: %s:%d", config->http_proxy_host.c_str(), config->http_proxy_port);
    }
    LOG_INFO("create client to: %s", config->endpoint.c_str());
}

std::vector<std::string> ArthubClient::GetFileList(const std::string& depot, uint64_t id,
                                                   const std::string& meta) {
    std::stringstream url;
    url << "/" << depot << "/openapi/v1/core/get-file-list-by-id-meta";

    std::string body;
    {
        json::JsonDoc doc;
        doc.SetArray();
        rapidjson::Value v;
        v.SetObject();
        doc.AddMemberTo(v, "object_id", id);
        doc.AddMemberTo(v, "object_meta", "display_url");
        doc.PushBackTo(doc.GetDocument(), v);
        body = doc.GetString();
    }

    auto resp = SendArthubRequest(url.str(), body);
    if (resp == nullptr) {
        return {};
    }

    rapidjson::Document reply_doc;
    reply_doc.Parse(resp->body.data(), resp->body.size());
    if (!reply_doc.IsObject()) {
        return {};
    }
    int code = -1;
    json::GetMember(reply_doc, "code", code);
    if (code != 0) {
        return {};
    }
    rapidjson::Value* v = rapidjson::Pointer("/result/items/0/files").Get(reply_doc);
    if (v == nullptr || !v->IsArray()) {
        return {};
    }
    std::vector<std::string> ret;
    json::GetElement(*v, ret);
    return ret;
}

GetDownloadSignatureReplyJson ArthubClient::GetDownloadSign(
    const std::string& depot, std::vector<GetSignatureReqItem>& param) {
    std::stringstream url;
    url << "/" << depot << "/openapi/v1/core/get-download-signature";

    for (auto& m : param) {
        if (EndsWith(m.file_name, ".gz")) {
            m.content_encoding = "gzip";
        }
    }
    json::JsonDoc doc;
    doc.AddMember("items", param);
    std::string body = doc.GetString();

    GetDownloadSignatureReplyJson reply;
    std::vector<std::string> ret(param.size(), "");
    auto resp = SendArthubRequest(url.str(), body);
    if (resp == nullptr) {
        return reply;
    }

    reply.FromJson(resp->body);
    return reply;
}

GetDownloadSignatureReplyJson ArthubClient::GetDownloadSign(const std::string& depot,
                                                            const GetSignatureReqItem& param) {
    std::vector<GetSignatureReqItem> vec(1, param);
    return GetDownloadSign(depot, vec);
}

GetUploadSignatureReplyJson ArthubClient::GetUploadSign(
    const std::string& depot, const std::vector<GetSignatureReqItem>& param) {
    std::stringstream url;
    url << "/" << depot << "/openapi/v1/core/get-upload-signature";

    json::JsonDoc doc;
    doc.AddMember("items", param);
    std::string body = doc.GetString();

    GetDownloadSignatureReplyJson reply;
    std::vector<std::string> ret(param.size(), "");
    auto resp = SendArthubRequest(url.str(), body);
    if (resp == nullptr) {
        return reply;
    }

    reply.FromJson(resp->body);
    return reply;
}

GetUploadSignatureReplyJson ArthubClient::GetUploadSign(const std::string& depot,
                                                        const GetSignatureReqItem& param) {
    std::vector<GetSignatureReqItem> vec(1, param);
    return GetUploadSign(depot, vec);
}

UpdateAssetByIDReplyJson ArthubClient::UpdateAssetByID(
    const std::string& depot, const std::vector<UpdateAssetByIDReqItem>& param) {
    std::stringstream url;
    url << "/" << depot << "/openapi/v1/core/update-asset-by-id";

    json::JsonDoc doc;
    doc.AddMember("items", param);
    std::string body = doc.GetString();

    UpdateAssetByIDReplyJson reply;
    std::vector<std::string> ret(param.size(), "");
    auto resp = SendArthubRequest(url.str(), body);
    if (resp == nullptr) {
        return reply;
    }

    reply.FromJson(resp->body);
    return reply;
}

std::string ArthubClient::DownloadContent(const std::string& depot,
                                          const GetSignatureReqItem& param) {
    auto resp = GetDownloadSign(depot, param);
    if (resp.code != 0 || resp.result.items.empty()) {
        return std::string();
    }
    std::string universal_identifier =
        config->endpoint_protocol + ":" + resp.result.items.front().signed_url;
    Url url(universal_identifier);
    std::string domain = url.protocol_ + "://" + url.host_;
    httplib::Client download(domain.c_str());
    auto res = download.Get(url.path_with_param_.c_str());
    if (res && res->status == 200)
        return res->body;
    else
        return std::string();
}

std::string ArthubClient::UploadContent(const std::string& depot, const GetSignatureReqItem& param,
                                        const std::string& file_content,
                                        const std::string& content_type) {
    auto resp = GetUploadSign(depot, param);
    if (resp.code != 0 || resp.result.items.empty()) {
        return std::string();
    }
    std::string universal_identifier =
        config->endpoint_protocol + ":" + resp.result.items.front().signed_url;
    Url url(universal_identifier);
    std::string domain = url.protocol_ + "://" + url.host_;
    httplib::Client download(domain.c_str());
    auto res = download.Put(url.path_with_param_.c_str(), file_content, content_type.c_str());
    if (res && res->status < 300)
        return resp.result.items.front().origin_url;
    else
        return std::string();
}

std::unique_ptr<httplib::Response> ArthubClient::SendArthubRequest(const std::string& api_url,
                                                                   const std::string& body) {
    // 传入的path不许加参数
    assert(api_url.find('?') == std::string::npos);

    httplib::Request req;
    req.method = "POST";
    req.path = api_url;
    req.headers.insert({"Content-Type", "application/json"});
    req.body = body;

    if (config->cred.credential_type == kArthubCredential_Token) {
        req.headers.insert({"publictoken", config->cred.account_name});
    } else if (config->cred.credential_type == kArthubCredential_PCG_Token) {
        req.headers.insert({"pcgtoken", config->cred.account_name});
    } else if (config->cred.credential_type == kArthubCredential_Username) {
        if (config->network_environment == kNetworkCondition_Normal && !config->cookie.empty()) {
            req.headers.insert({"cookie", config->cookie});
        }
    }
    if (!config->mac_address.empty()) req.headers.insert({"MacAddress", config->mac_address});

    if (req.params.size() > 0) req.path += "?" + httplib::detail::params_to_query_str(req.params);

    LOG_DEBUG("%s %s", req.method.c_str(), req.path.c_str());
    for (const auto& m : req.headers) LOG_DEBUG("%s: %s", m.first.c_str(), m.second.c_str());
    LOG_DEBUG("%s", req.body.c_str());

    auto res = std::make_unique<httplib::Response>();
    int retry_times = 0;
    int retry_timespan = 200;
    bool success = false;
    do {
        success = http_client->send(req, *res);
        retry_times++;
        retry_timespan *= 2;
        if (!success && retry_times <= total_retry) {
            std::this_thread::sleep_for(std::chrono::milliseconds(retry_timespan));
        }
    } while (!success && retry_times <= total_retry);

    LOG_DEBUG("%d %s", res->status, res->version.c_str());
    for (const auto& m : res->headers) LOG_DEBUG("%s: %s", m.first.c_str(), m.second.c_str());
    LOG_DEBUG("%s", res->body.c_str());

    if (res->status == 302 || res->status == 401) {
        LOG_ERROR("credential may be invalid, http req to %s, status code is %d.", api_url.c_str(), res->status);
    }

    if (!success) {
        res.reset();
    }

    return res;
}

bool CheckProxy(ArthubConfig& config) {
    // check proxy
    bool has_proxy = false;
    if (!config.http_proxy_host.empty()) {
        std::string proxy_url =
            config.http_proxy_host + ":" + std::to_string(config.http_proxy_port);
        auto proxy_resp = httplib::Client(proxy_url.c_str()).Get("/");
        if (proxy_resp) {
            has_proxy = true;
            LOG_ERROR("the configured proxy(%s) works.", proxy_url.c_str());
        } else {
            LOG_ERROR("the configured proxy(%s) may not work.", proxy_url.c_str());
        }
    }
    return has_proxy;
}

bool CheckHostDirectConnectDomain(ArthubConfig& config) {
    // check proxy
    bool has_proxy = false;
    if (!config.endpoint.empty()) {
        std::string proxy_url = config.endpoint;
        auto cli = httplib::Client(proxy_url.c_str());
        cli.set_connection_timeout(0, 5000000); 
        cli.set_read_timeout(5, 0);
        auto domain_resp = cli.Get("/");
        if (domain_resp && domain_resp->status != 401) {
            has_proxy = true;
            LOG_ERROR("the configured domain(%s) can be accessed directly.", proxy_url.c_str());
        } else {
            LOG_ERROR("the configured domain(%s) may not be accessed directly.", proxy_url.c_str());
        }
    }
    return has_proxy;
}

bool DetermineEndpoint(ArthubConfig& config) {
    config.endpoint_protocol = "https";
    if (config.network_environment == kNetworkCondition_OA) {
        if (config.cred.credential_type == kArthubCredential_Token) {
            config.endpoint = "http://api.arthub.oa.com";  // 此域名没有https
            config.endpoint_protocol = "http";
        } else if(config.cred.credential_type == kArthubCredential_Username) {
            config.endpoint = "https://arthub.oa.com";
            config.endpoint_protocol = "https";
        } else {
            LOG_ERROR("pcg_token is not allowed for oa environment");
            return false;
        }
        // force a ioa proxy
        if (!CheckHostDirectConnectDomain(config)) {
            config.http_proxy_host = "127.0.0.1";
            config.http_proxy_port = 12639;
        }
        if (!CheckProxy(config)) {
            config.http_proxy_host.clear();
            config.http_proxy_port = 0;
        }
    } else if (config.network_environment == kNetworkCondition_Normal) {
        if (config.cred.credential_type == kArthubCredential_Username) {
            LOG_ERROR("pcg_token is not allowed in normal environment");
            return false;
        }
        config.endpoint = "https://arthub.qq.com";
        config.endpoint_protocol = "https";
    } else if (config.network_environment == kNetworkCondition_IDC) {
        if (config.cred.credential_type != kArthubCredential_PCG_Token) {
            LOG_ERROR("pcg_token is the only credential supported in idc environment");
            return false;
        }
        config.endpoint = "http://arthub.oa.com";
        config.endpoint_protocol = "http";
    }
    return true;
}
