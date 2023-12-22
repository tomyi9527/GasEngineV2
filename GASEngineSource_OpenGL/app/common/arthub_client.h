#pragma once
#include <memory>
#include <string>
#include <vector>
#include "arthub_client_format.h"
#define CPPHTTPLIB_OPENSSL_SUPPORT

enum ArthubCredentialType { kArthubCredential_Username, kArthubCredential_Token, kArthubCredential_PCG_Token };
enum NetworkConditionType { kNetworkCondition_OA, kNetworkCondition_IDC, kNetworkCondition_Normal };

namespace httplib {
struct Response;
struct Client;
}  // namespace httplib

class ArthubCredential {
 public:
    ArthubCredentialType credential_type = kArthubCredential_Username;
    std::string account_name;
};

class ArthubConfig {
 public:
    NetworkConditionType network_environment = kNetworkCondition_Normal;
    std::string http_proxy_host;
    int http_proxy_port = 0;
    std::string cookie;
    std::string mac_address;
    std::string endpoint;
    std::string endpoint_protocol;
    ArthubCredential cred;

 public:
    static std::shared_ptr<ArthubConfig> GetGlobalConfig() {
        static std::shared_ptr<ArthubConfig> config = std::make_shared<ArthubConfig>();
        return config;
    }
};

bool CheckProxy(ArthubConfig& output);
bool DetermineEndpoint(ArthubConfig& output);  // by network_environment and credential_type

class ArthubClient {
 public:
    ArthubClient(const std::shared_ptr<ArthubConfig>& in_config);

    // TODO(beanpliu): 将GetFileList的输出也改为 xxxReplyJson 的形式
    std::vector<std::string> GetFileList(const std::string& depot, uint64_t id,
                                         const std::string& meta = "display_url");

    GetDownloadSignatureReplyJson GetDownloadSign(const std::string& depot,
                                                  std::vector<GetSignatureReqItem>& param);
    GetDownloadSignatureReplyJson GetDownloadSign(const std::string& depot,
                                                  const GetSignatureReqItem& param);
    GetUploadSignatureReplyJson GetUploadSign(const std::string& depot,
                                              const std::vector<GetSignatureReqItem>& param);
    GetUploadSignatureReplyJson GetUploadSign(const std::string& depot,
                                              const GetSignatureReqItem& param);

    UpdateAssetByIDReplyJson UpdateAssetByID(const std::string& depot,
                                             const std::vector<UpdateAssetByIDReqItem>& param);

    std::string DownloadContent(const std::string& depot, const GetSignatureReqItem& param);
    // return origin_url
    std::string UploadContent(const std::string& depot, const GetSignatureReqItem& param,
                              const std::string& file_content, const std::string& content_type);

 protected:
    std::unique_ptr<httplib::Response> SendArthubRequest(const std::string& api_url,
                                                         const std::string& param);

 protected:
    std::shared_ptr<ArthubConfig> config;
    std::shared_ptr<httplib::Client> http_client;
};