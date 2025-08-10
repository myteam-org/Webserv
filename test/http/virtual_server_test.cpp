#include <gtest/gtest.h>
#include <map>
#include <vector>
#include <string>
#include "http/virtual_server.hpp"
#include "config/context/serverContext.hpp"
#include "config/context/locationContext.hpp"
#include "config/context/documentRootConfig.hpp"
#include "http/request/request.hpp"
#include "http/status.hpp"
#include "router/middleware/error_page.hpp"  // ErrorPageMap
#include "raw_headers.hpp"
#include "action.hpp"


TEST(VirtualServerTest, Constructor) {
    ServerContext serverContext("");
    VirtualServer virtualServer(serverContext, "127.0.0.1");
    EXPECT_EQ(virtualServer.getServerConfig().getValue(), serverContext.getValue());
}

// テスト専用・期待動作と同じロジック（後勝ち）
static ErrorPageMap MergeErrorPages_ForTest(
    const std::vector<std::map<int, std::string> >& vec) {
    ErrorPageMap out;
    for (std::vector<std::map<int, std::string> >::const_iterator it = vec.begin();
         it != vec.end(); ++it) {
        for (std::map<int, std::string>::const_iterator jt = it->begin();
             jt != it->end(); ++jt) {
            out[static_cast<http::HttpStatusCode>(jt->first)] = jt->second;
        }
    }
    return out;
}

TEST(ErrorPageMergeTest, MergeDistinctKeys) {
    std::vector<std::map<int, std::string> > v(1);
    v[0][403] = "403.html";
    v[0][500] = "500.html";

    ErrorPageMap m = MergeErrorPages_ForTest(v);
    ASSERT_EQ(m.size(), 2u);
    EXPECT_EQ(m[http::kStatusForbidden] , "403.html");
    EXPECT_EQ(m[http::kStatusInternalServerError], "500.html");
}

TEST(ErrorPageMergeTest, LastWinsOnDuplicateKey) {
    std::vector<std::map<int, std::string> > v(2);
    v[0][403] = "403-a.html";
    v[1][403] = "403-b.html"; // 後勝ちで上書きされる想定

    ErrorPageMap m = MergeErrorPages_ForTest(v);
    ASSERT_EQ(m.size(), 1u);
    EXPECT_EQ(m[http::kStatusForbidden], "403-b.html");
}

// --- ヘルパ: Location を組み立てる（必要最低限）
static LocationContext MakeLoc(const std::string& path,
                               bool allow_get, bool allow_post, bool allow_delete) {
    LocationContext loc(/*text*/"");
    loc.setPath(path);
    if (allow_get)    loc.setMethod(GET);     // AllowedMethod の列挙子（GET/POST/DELETE）
    if (allow_post)   loc.setMethod(POST);
    if (allow_delete) loc.setMethod(DELETE);
    // DocumentRootConfig は今回のテストでは未使用（405 判定が主目的）
    return loc;
}

// --- ヘルパ: Request を作る（最低限のヘッダ/ボディでOK）
static http::Request MakeReq(http::HttpMethod m,
                             const std::string& target,
                             const ServerContext* sc,
                             const LocationContext* lc) {
    RawHeaders headers;           // 型は request.hpp の定義に依存（空でOK）
    std::vector<char> body;             // 空でOK
    return http::Request(m, target, headers, body, sc, lc);
}

// コンストラクタが ServerContext を保持しているか（既存の基本テスト）
TEST(VirtualServerTest, ConstructorKeepsServerConfig) {
    ServerContext sc("");
    VirtualServer vs(sc, "127.0.0.1");
    EXPECT_EQ(vs.getServerConfig().getValue(), sc.getValue());
}

// ルート "/" は GET のみ → POST は 405 になること
TEST(VirtualServerRouteTest, PostToRootIs405WhenOnlyGetAllowed) {
    // server { location / { allow_method GET; } }
    ServerContext sc("");
    LocationContext root = MakeLoc("/", /*GET*/true, /*POST*/false, /*DELETE*/false);
    sc.addLocation(root);

    VirtualServer vs(sc, "127.0.0.1");

    // Request: POST /
    http::Request req = MakeReq(http::kMethodPost, "/", &sc, &root);
    Either<IAction*, http::Response> res = vs.getRouter().serve(req);

    ASSERT_TRUE(res.isRight());
    EXPECT_EQ(res.unwrapRight().getStatusCode(), http::kStatusMethodNotAllowed);
}

// /upload は POST を Upload に通す → 405 にはならないこと
TEST(VirtualServerRouteTest, PostToUploadIsNot405WhenUploadLocationExists) {
    ServerContext sc("");
    // "/" は GET のみ（おまけ）。"/upload" は POST 許可
    LocationContext root   = MakeLoc("/",       /*GET*/true,  /*POST*/false, /*DELETE*/false);
    LocationContext upload = MakeLoc("/upload", /*GET*/false, /*POST*/true,  /*DELETE*/false);
    sc.addLocation(root);
    sc.addLocation(upload);

    VirtualServer vs(sc, "127.0.0.1");

    // Request: POST /upload
    http::Request req = MakeReq(http::kMethodPost, "/upload", &sc, &upload);
    Either<IAction*, http::Response> res = vs.getRouter().serve(req);

    ASSERT_TRUE(res.isRight());
    // UploadFileHandler の具体挙動は環境依存なので、ここでは「405じゃない」ことだけを検証
    EXPECT_NE(res.unwrapRight().getStatusCode(), http::kStatusMethodNotAllowed);
}

// （任意）GET / 未登録時の 404 を確認したい場合
TEST(VirtualServerRouteTest, GetUnknownPathIs404) {
    ServerContext sc("");
    // 何も location を追加しない、または "/" 以外のみ追加
    LocationContext other = MakeLoc("/other", /*GET*/true, /*POST*/false, /*DELETE*/false);
    sc.addLocation(other);

    VirtualServer vs(sc, "127.0.0.1");

    http::Request req = MakeReq(http::kMethodGet, "/", &sc, &other);
    Either<IAction*, http::Response> res = vs.getRouter().serve(req);

    ASSERT_TRUE(res.isRight());
    EXPECT_EQ(res.unwrapRight().getStatusCode(), http::kStatusNotFound);
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
