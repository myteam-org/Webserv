#include <gtest/gtest.h>
#include <vector>
#include <string>
#include "config/config_resolver.hpp"
#include "config/config.hpp"
#include "config/context/serverContext.hpp"
#include "utils/types/result.hpp"
#include "utils/types/error.hpp"
#include <stdexcept>  // ← runtime_error 用

// ServerContextのテスト用モック実装
class DummyServerContext : public ServerContext {
 private:
  std::string host_;

 public:
  explicit DummyServerContext(const std::string& host) : host_(host) {}

  virtual std::string getHost() const {
    return host_;
  }
};

// テスト対象：ConfigResolver::chooseServer の挙動

TEST(ConfigResolverTest, ReturnsServerWhenExactHostMatches) {
  std::vector<ServerContext> servers;
  servers.push_back(DummyServerContext("example.com"));
  servers.push_back(DummyServerContext("localhost"));

  http::config::ConfigResolver resolver(servers);

  types::Result<const ServerContext*, error::AppError> result =
      resolver.chooseServer("example.com");

  EXPECT_TRUE(result.isOk());
  EXPECT_EQ(result.unwrap()->getHost(), "example.com");
}

TEST(ConfigResolverTest, ReturnsServerWhenHostMatchesIgnoringPort) {
  std::vector<ServerContext> servers;
  servers.push_back(DummyServerContext("example.com"));

  http::config::ConfigResolver resolver(servers);

  types::Result<const ServerContext*, error::AppError> result =
      resolver.chooseServer("example.com:8080");

  EXPECT_TRUE(result.isOk());
  EXPECT_EQ(result.unwrap()->getHost(), "example.com");
}

TEST(ConfigResolverTest, ReturnsErrorWhenHostNotFound) {
  std::vector<ServerContext> servers;
  servers.push_back(DummyServerContext("localhost"));

  http::config::ConfigResolver resolver(servers);

  types::Result<const ServerContext*, error::AppError> result =
      resolver.chooseServer("unknown.com");

  EXPECT_TRUE(result.isErr());
  EXPECT_EQ(result.unwrapErr(), error::kBadRequest);
}

TEST(ConfigResolverTest, ReturnsErrorWhenHostIsEmpty) {
  std::vector<ServerContext> servers;
  servers.push_back(DummyServerContext("localhost"));

  http::config::ConfigResolver resolver(servers);

  types::Result<const ServerContext*, error::AppError> result =
      resolver.chooseServer("");

  EXPECT_TRUE(result.isErr());
  EXPECT_EQ(result.unwrapErr(), error::kBadRequest);
}

TEST(ConfigParserTest, NoListenInServerError) {
  EXPECT_THROW({
    Config config("test/test_configs/no_listen.conf");
  }, std::runtime_error);
}
