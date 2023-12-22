#include "http/url.h"
#include "gtest/gtest.h"

TEST(URL_TEST, parse_url) {
    Url url;
    url.ParseUrl("/test?1=2&3=4#top");
    EXPECT_EQ(url.url_, "/test");
    EXPECT_EQ(url.parameters_.size(), 2);
    EXPECT_EQ(url.GetParameter("1"), "2");
    EXPECT_EQ(url.GetParameter("3"), "4");
    EXPECT_EQ(url.fragment_, "top");
}

TEST(URL_TEST, parse_url2) {
    Url url;
    url.ParseUrl("/test?1=2&3=4");
    EXPECT_EQ(url.url_, "/test");
    EXPECT_EQ(url.parameters_.size(), 2);
    EXPECT_EQ(url.GetParameter("1"), "2");
    EXPECT_EQ(url.GetParameter("3"), "4");
    EXPECT_EQ(url.fragment_, "");
}

TEST(URL_TEST, parse_url2_depot) {
    Url url;
    url.ParseUrl("/AssetHub_Atc/test?1=2&3=4");
    EXPECT_EQ(url.url_, "/AssetHub_Atc/test");
    EXPECT_EQ(url.depot_name_, "AssetHub_Atc");
    EXPECT_EQ(url.parameters_.size(), 2);
    EXPECT_EQ(url.GetParameter("1"), "2");
    EXPECT_EQ(url.GetParameter("3"), "4");
    EXPECT_EQ(url.fragment_, "");
}

TEST(URL_TEST, parse_url3) {
    Url url;
    url.ParseUrl("/test?#test");
    EXPECT_EQ(url.url_, "/test");
    EXPECT_EQ(url.parameters_.size(), 0);
    EXPECT_EQ(url.fragment_, "test");
}

TEST(URL_TEST, parse_url4) {
    Url url;
    url.ParseUrl("/test#test");
    EXPECT_EQ(url.url_, "/test");
    EXPECT_EQ(url.parameters_.size(), 0);
    EXPECT_EQ(url.fragment_, "test");
}

TEST(URL_TEST, parse_url5) {
    Url url;
    url.ParseUrl("/test#");
    EXPECT_EQ(url.url_, "/test");
    EXPECT_EQ(url.parameters_.size(), 0);
    EXPECT_EQ(url.fragment_, "");
}

TEST(URL_TEST, parse_url6) {
    Url url;
    url.ParseUrl("/test?#");
    EXPECT_EQ(url.url_, "/test");
    EXPECT_EQ(url.parameters_.size(), 0);
    EXPECT_EQ(url.fragment_, "");
}

TEST(URL_TEST, parse_url7) {
    Url url;
    url.ParseUrl("/test?1=&3=4");
    EXPECT_EQ(url.url_, "/test");
    EXPECT_EQ(url.parameters_.size(), 2);
    EXPECT_EQ(url.GetParameter("1"), "");
    EXPECT_EQ(url.GetParameter("3"), "4");
    EXPECT_EQ(url.fragment_, "");
}

class UrlCoderForTest : public Url {
 public:
    using Url::Decode;
    using Url::Encode;

    // 仅处理一位的FromHex
    using Url::FromHex;
    // 仅处理一位的ToHex
    using Url::ToHex;
};

TEST(URL_TEST, ToHex) {
    // value (0-15) 转到 ascii表示的 16进制。
    for (int i = 0; i < 10; ++i) ASSERT_EQ(i + '0', UrlCoderForTest::ToHex(i));
    for (int i = 10; i < 16; ++i) ASSERT_EQ(i - 10 + 'A', UrlCoderForTest::ToHex(i));
}

TEST(URL_TEST, FromHex) {
    // value (0-15) 转到 ascii表示的 16进制。
    for (int i = 0; i < 10; ++i) ASSERT_EQ(i, UrlCoderForTest::FromHex(i + '0'));
    for (int i = 10; i < 16; ++i) ASSERT_EQ(i, UrlCoderForTest::FromHex(i - 10 + 'A'));
    for (int i = 10; i < 16; ++i) ASSERT_EQ(i, UrlCoderForTest::FromHex(i - 10 + 'a'));
}

TEST(URL_TEST, Decode) {
    ASSERT_EQ(UrlCoderForTest::Decode("%E6%88%91%E6%98%AF%E4%B8%AD%E6%96%87"), "我是中文");
    ASSERT_EQ(UrlCoderForTest::Decode("%GH"), "%GH");  // 测试失败情况，返回原字符串
    ASSERT_EQ(UrlCoderForTest::Decode("http%3A%2F%2Ftest.com%2Findex.html%3Ftest%3Dhttp"),
              "http://test.com/index.html?test=http");
}

TEST(URL_TEST, Encode) {
    ASSERT_EQ("%E6%88%91%E6%98%AF%E4%B8%AD%E6%96%87", UrlCoderForTest::Encode("我是中文"));
    ASSERT_EQ("%25GH", UrlCoderForTest::Encode("%GH"));
    ASSERT_EQ("http%3A%2F%2Ftest.com%2Findex.html%3Ftest%3Dhttp",
              UrlCoderForTest::Encode("http://test.com/index.html?test=http"));
    // 自定义字符忽略
    ASSERT_EQ("http://test.com/index.html%3Ftest%3Dhttp",
              UrlCoderForTest::Encode("http://test.com/index.html?test=http", ":/."));
}