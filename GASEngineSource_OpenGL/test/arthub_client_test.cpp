#include "../app/common/arthub_client.h"
#include <filesystem>
#include <iostream>
#include <memory>
#include <vector>
#include "assert.h"
#include "set_locale.h"

int test_token_oa();
int test_user_oa();

int main() {
    SetLocale("zh_CN.UTF-8");

    std::filesystem::path test("http://testurl.com/1/2/3/4.png?test=1&test32=32#1234");
    std::cout << std::endl << test.extension();
    std::cout << std::endl << test.filename();
    std::cout << std::endl << test.parent_path();
    std::cout << std::endl << test.relative_path();
    std::cout << std::endl << test.root_directory();
    std::cout << std::endl << test.root_name();
    std::cout << std::endl;

    test_token_oa();
    test_user_oa();

    return 0;
}

int test_token_oa() {
    std::shared_ptr<ArthubConfig> config = std::make_shared<ArthubConfig>();
    config->cred.account_name = "333151e00019";
    config->cred.credential_type = kArthubCredential_Token;
    config->network_environment = kNetworkCondition_OA;

    DetermineEndpoint(*config);
    CheckProxy(*config);
    ArthubClient client(config);

    auto r1 = client.GetFileList("AssetHub_Atc", 39730020959);
    assert(r1.size() == 1);

    std::vector<GetSignatureReqItem> req;
    req.emplace_back();
    req.back().object_id = 39730020959;
    req.back().object_meta = "display_url";
    auto r2 = client.GetDownloadSign("AssetHub_Atc", req);
    assert(r2.result.items.size() == 1 && !r2.result.items[0].signed_url.empty());

    return 0;
}

int test_user_oa() {
    std::shared_ptr<ArthubConfig> config = std::make_shared<ArthubConfig>();
    config->cred.credential_type = kArthubCredential_Username;
    config->network_environment = kNetworkCondition_OA;

    DetermineEndpoint(*config);
    CheckProxy(*config);
    ArthubClient client(config);

    auto r1 = client.GetFileList("AssetHub_Atc", 39730020959);
    assert(r1.size() == 1);

    std::vector<GetSignatureReqItem> req;
    req.emplace_back();
    req.back().object_id = 39730020959;
    req.back().object_meta = "display_url";
    auto r2 = client.GetDownloadSign("AssetHub_Atc", req);
    assert(r2.result.items.size() == 1 && !r2.result.items[0].signed_url.empty());

    return 0;
}