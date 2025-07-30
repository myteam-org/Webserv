#include <gtest/gtest.h>
#include "http/request/http_request.hpp"
#include "config/context/serverContext.hpp"
#include "config/context/locationContext.hpp"
#include "config/context/documentRootConfig.hpp"
#include "config/config.hpp"
#include "http/config/config_resolver.hpp"
#include "http/request/read/context.hpp"  // ReadContext
#include "http/request/read/state.hpp"    // IState

// ---- ダミー Resolver ----
struct DummyResolver : public http::config::IConfigResolver {
    types::Result<const ServerContext*, error::AppError>
    choseServer(const std::string&) const override {
        return types::ok(static_cast<const ServerContext*>(NULL));
    }

    types::Result<const LocationContext*, error::AppError>
    choseLocation(const std::string&, const ServerContext&) const {
        return types::ok(static_cast<const LocationContext*>(NULL));
    }
};

// ---- ダミー ReadContext ----
struct DummyReadContext : public http::ReadContext {
    DummyReadContext()
        : http::ReadContext(staticResolver, NULL) {}  // NULL = dummy IState*

    static DummyResolver staticResolver;
};

// ---- static resolver インスタンス定義 ----
DummyResolver DummyReadContext::staticResolver;

// ---- テスト ----
TEST(HttpRequestTest, CanSetServerAndLocationPointersFromConfig) {
    ASSERT_TRUE(std::ifstream("test_config/test.conf").good()) 
    << "Missing test_config/test.conf (maybe not copied correctly in CI)";
    DummyReadContext dummyCtx;
    http::HttpRequest req;

    const std::string confFile = "./test_config/test.conf";
    Config config(confFile);

    const std::vector<ServerContext>& servers = config.getParser().getServer();
    ASSERT_FALSE(servers.empty());
    const ServerContext& server = servers[0];

    const std::vector<LocationContext>& locations = server.getLocation();
    ASSERT_FALSE(locations.empty());
    const LocationContext& location = locations[0];

    const DocumentRootConfig& docRoot = location.getDocumentRootConfig();

    req.setServer(server);
    req.setLocation(location);
    req.setDocumentRootConfig(docRoot);

    EXPECT_EQ(req.getServer(), &server);
    EXPECT_EQ(req.getLocation(), &location);
    EXPECT_EQ(req.getDocumentRoot(), &docRoot);
}
