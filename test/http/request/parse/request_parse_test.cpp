#include <gtest/gtest.h>
#include "http/request/parse/request_parser.hpp"
#include "http/config/config_resolver.hpp"
#include "http/request/read/context.hpp"
#include "context/locationContext.hpp"
#include "config/context/serverContext.hpp"
#include "http/request/http_request.hpp"

namespace {

class DummyResolver : public http::config::IConfigResolver {
 public:
  types::Result<const ServerContext*, error::AppError>
  chooseServer(const std::string&) const override {
    static ServerContext dummy("server");
    return types::ok(static_cast<const ServerContext*>(&dummy));
  }
};

RawHeaders makeValidHeaders() {
  RawHeaders headers;
  headers["Host"] = "localhost";
  headers["Content-Length"] = "5";
  return headers;
}

}  // namespace


// parseRequestLine
TEST(RequestParserTest, ParsesValidRequestLine) {
  DummyResolver resolver;
  http::ReadContext ctx(resolver, 0);

  ctx.setRequestLine("GET /index.html HTTP/1.1\r\n");
  http::parse::RequestParser parser(ctx);
  types::Result<types::Unit, error::AppError> result = parser.parseRequestLine();

  EXPECT_TRUE(result.isOk());
}

// parseHeaders 正常系
TEST(RequestParserTest, ParsesValidHeaders) {
  DummyResolver resolver;
  http::ReadContext ctx(resolver, NULL);
  ctx.setHeaders(makeValidHeaders());

  http::parse::RequestParser parser(ctx);
  types::Result<types::Unit, error::AppError> result = parser.parseHeaders();

  EXPECT_TRUE(result.isOk());
}

// parseHeaders 異常系（Host 欠如）
TEST(RequestParserTest, ReturnsErrorWhenHostMissing) {
  DummyResolver resolver;
  http::ReadContext ctx(resolver, NULL);

  RawHeaders headers;
  headers["Content-Length"] = "10";  // Host がない
  ctx.setHeaders(headers);

  http::parse::RequestParser parser(ctx);
  types::Result<types::Unit, error::AppError> result = parser.parseHeaders();

  EXPECT_TRUE(result.isErr());
  EXPECT_EQ(result.unwrapErr(), error::kMissingHost);
}

// テスト：parseBody
TEST(RequestParserTest, ParsesBodyCorrectly) {
  DummyResolver resolver;
  http::ReadContext ctx(resolver, NULL);
  ctx.setBody("ABCDE");

  http::parse::RequestParser parser(ctx);
  types::Result<types::Unit, error::AppError> result = parser.parseBody();

  EXPECT_TRUE(result.isOk());
}

// テスト：chooseLocation
TEST(RequestParserTest, ChoosesCorrectLocation) {
  DummyResolver resolver;
  http::ReadContext ctx(resolver, NULL);

  // ServerContext に LocationContext を追加
  ServerContext server("server");

  LocationContext rootLoc("server");
  rootLoc.setPath("/");  // ← マッチさせたい prefix にする
  server.addLocation(rootLoc);  // ← 適切な関数名に置き換えてください

  ctx.setServer(server);

  http::parse::RequestParser parser(ctx);
  std::string uri = "/";

  types::Result<const LocationContext*, error::AppError> result = parser.chooseLocation(uri);

  ASSERT_TRUE(result.isOk());
  EXPECT_EQ(result.unwrap()->getPath(), "/");
}
