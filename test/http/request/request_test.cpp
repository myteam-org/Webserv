#include "http/request/request.hpp"

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "config/context/documentRootConfig.hpp"
#include "config/context/locationContext.hpp"
#include "config/context/serverContext.hpp"
#include "http/method.hpp"  // kMethodGet / kMethodPost
#include "raw_headers.hpp"
#include "utils/types/option.hpp"

// target を path / query に分割（C++98）
static void splitTarget(const std::string& target,
                        std::string& pathOnly,
                        std::string& queryString) {
    std::size_t pos = target.find('?');
    if (pos == std::string::npos) {
        pathOnly = target;
        queryString.clear();
    } else {
        pathOnly = target.substr(0, pos);
        queryString = target.substr(pos + 1);
    }
}

// 文字列 -> vector<char>
static std::vector<char> toVec(const std::string& s) {
    return std::vector<char>(s.begin(), s.end());
}

TEST(RequestTest, ConstructorAndGetters_FullArgs) {
    ::ServerContext server("server");
    ::LocationContext location("location");

    const http::HttpMethod method = http::kMethodGet;
    const std::string requestTarget = "/index.html?x=1";

    std::string pathOnly;
    std::string queryString;
    splitTarget(requestTarget, pathOnly, queryString); // "/index.html", "x=1"

    const std::string version = "HTTP/1.1"; // 実装側で固定なら検証用途のみ

    RawHeaders headers;
    headers.insert(std::make_pair("host", "example.com"));
    headers.insert(std::make_pair("content-length", "12"));

    const std::string bodyStr = "body content";
    std::vector<char> body = toVec(bodyStr);

    // ★ 新シグネチャ（pathOnly / queryString を渡す）
    http::Request req(method, requestTarget, pathOnly, queryString,
                      headers, body, &server, &location);

    EXPECT_EQ(req.getMethod(), method);
    EXPECT_EQ(req.getRequestTarget(), requestTarget);
    EXPECT_EQ(req.getPath(), pathOnly);
    EXPECT_EQ(req.getQueryString(), queryString);
    EXPECT_EQ(req.getHttpVersion(), version);
    EXPECT_EQ(std::string(req.getBody().begin(), req.getBody().end()),
              "body content");

    types::Option<std::string> h = req.getHeader("host");
    ASSERT_TRUE(h.isSome());
    EXPECT_EQ(h.unwrap(), "example.com");
}

// ヘルパ：新シグネチャ対応の Request 生成
static http::Request mkReq(::ServerContext& server, ::LocationContext& location,
                           http::HttpMethod method, const std::string& target,
                           const std::string& bodyStr,
                           const RawHeaders& headers /*= RawHeaders()*/) {
    std::string pathOnly, queryString;
    splitTarget(target, pathOnly, queryString);
    return http::Request(method, target, pathOnly, queryString,
                         headers, toVec(bodyStr), &server, &location);
}

TEST(RequestTest, EqualityOperator) {
    ::ServerContext server("server");
    ::LocationContext location("location");
    RawHeaders empty;  // 空ヘッダ

    // 同値
    http::Request req1 = mkReq(server, location, http::kMethodGet, "/test", "", empty);
    http::Request req2 = mkReq(server, location, http::kMethodGet, "/test", "", empty);

    // 差分（method / target / body）
    http::Request req3 = mkReq(server, location, http::kMethodPost, "/test", "", empty);
    http::Request req4 = mkReq(server, location, http::kMethodGet, "/other", "", empty);
    http::Request req6 = mkReq(server, location, http::kMethodGet, "/test", "body", empty);

    EXPECT_TRUE(req1 == req2);
    EXPECT_FALSE(req1 == req3);
    EXPECT_FALSE(req1 == req4);
    EXPECT_FALSE(req1 == req6);
}

TEST(RequestTest, GetHeader_NoneByDefault) {
    ServerContext server("server");
    LocationContext location("location");

    RawHeaders headers;
    std::vector<char> body;
    http::Request req(http::kMethodGet,
                      "/",              // requestTarget
                      "/",              // pathOnly
                      "",               // queryString
                      headers,
                      body,
                      &server,
                      &location);

    types::Option<std::string> h = req.getHeader("host");
    EXPECT_TRUE(h.isNone());
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}