// GoogleTest
#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <filesystem>
#include <csignal>
#include <functional>
#include <cstdlib>
#include <unistd.h>

// このヘッダだけ private→public 化（parseCgiAndBuildResponse を直接叩くため）
#define private public
#include "http/handler/file/cgi.hpp"
#undef private

#include "config/context/documentRootConfig.hpp"
#include "config/context/locationContext.hpp"
#include "config/context/serverContext.hpp"
#include "http/status.hpp"

#include "config.hpp"
#include "serverContext.hpp"
#include "token.hpp"
#include "tokenizer.hpp"

using namespace std;
namespace fs = std::filesystem;

// 無名名前空間に入れて再定義を防止
namespace {

// CgiHandler の簡単生成（DocumentRootConfigの最小初期化でOK）
static http::CgiHandler MakeHandler() {
    DocumentRootConfig conf;     // 必要なら conf.setRoot("/tmp"); 等
    return http::CgiHandler(conf);
}

// 本文取得（Option<string> を素直に読む）
static std::string GetBody(const http::Response& resp) {
    const types::Option<std::string>& opt = resp.getBody();
    return opt.isSome() ? opt.unwrap() : std::string();
}

// ヘッダ取得（toString() を軽く解析。専用getterがあれば差し替え可）
static std::string GetHeader(const http::Response& resp, const std::string& nameLower) {
    std::string s = const_cast<http::Response&>(resp).toString();

    std::string::size_type end = s.find("\r\n\r\n");
    if (end == std::string::npos) end = s.find("\n\n");
    if (end == std::string::npos) return std::string();

    std::string::size_type cur = 0;
    while (cur < end) {
        std::string::size_type eol = s.find('\n', cur);
        if (eol == std::string::npos || eol > end) eol = end;

        std::string line = s.substr(cur, eol - cur);
        if (!line.empty() && line[line.size() - 1] == '\r') line.erase(line.size() - 1);

        std::string::size_type colon = line.find(':');
        if (colon != std::string::npos) {
            std::string key = line.substr(0, colon);
            std::string val = line.substr(colon + 1);
            while (!val.empty() && (val[0] == ' ' || val[0] == '\t')) val.erase(0, 1);
            while (!val.empty() && (val[val.size() - 1] == ' ' || val[val.size() - 1] == '\t'))
                val.erase(val.size() - 1);
            for (std::size_t i = 0; i < key.size(); ++i) {
                char c = key[i];
                if ('A' <= c && c <= 'Z') key[i] = static_cast<char>(c - 'A' + 'a');
            }
            if (key == nameLower) return val;
        }

        if (eol == end) break;
        cur = eol + 1;
    }
    return std::string();
}

// 外部実行を避けて純粋に CGI 出力→HTTP 変換だけ検証
static http::Response RunWithCgiOut(const http::CgiHandler& h, const std::string& cgiOut) {
    return h.parseCgiAndBuildResponse(cgiOut);
}

} // namespace

// ===================== テスト =====================

TEST(CgiHandlerTest, NoHeaders_AllAsBody_200TextPlain) {
    http::CgiHandler h = MakeHandler();

    // ヘッダ無し CGI 出力
    const std::string cgiOut = "hello world";

    // 外部実行せずパースだけ検証
    http::Response resp = RunWithCgiOut(h, cgiOut);

    EXPECT_EQ(resp.getStatusCode(), http::kStatusOk);
    EXPECT_EQ(GetHeader(resp, "content-type"), "text/plain");
    EXPECT_EQ(GetHeader(resp, "content-length"), "11");
    EXPECT_EQ(GetBody(resp), "hello world");
}

TEST(CgiHandlerTest, Status404FromHeader) {
    http::CgiHandler h = MakeHandler();
    const std::string cgiOut =
        "Status: 404 Not Found\r\n"
        "Content-Type: text/plain\r\n"
        "\r\n"
        "nope";

    http::Response resp = RunWithCgiOut(h, cgiOut);

    EXPECT_EQ(resp.getStatusCode(), http::kStatusNotFound);
    EXPECT_EQ(GetHeader(resp, "content-type"), "text/plain");
    EXPECT_EQ(GetHeader(resp, "content-length"), "4");
    EXPECT_EQ(GetBody(resp), "nope");
}

TEST(CgiHandlerTest, LocationOnlyImplies302) {
    http::CgiHandler h = MakeHandler();
    const std::string cgiOut =
        "Location: https://example.com/\r\n"
        "\r\n";

    http::Response resp = RunWithCgiOut(h, cgiOut);

    EXPECT_EQ(resp.getStatusCode(), http::kStatusFound); // 302
    EXPECT_EQ(GetHeader(resp, "location"), "https://example.com/");
    EXPECT_EQ(GetHeader(resp, "content-length"), "0");
    EXPECT_TRUE(GetBody(resp).empty());
}

TEST(CgiHandlerTest, ContentTypePreservedAndLengthRecalculated) {
    http::CgiHandler h = MakeHandler();
    const std::string cgiOut =
        "Content-Type: text/html\r\n"
        "\r\n"
        "<body>";

    http::Response resp = RunWithCgiOut(h, cgiOut);

    EXPECT_EQ(resp.getStatusCode(), http::kStatusOk);
    EXPECT_EQ(GetHeader(resp, "content-type"), "text/html");
    EXPECT_EQ(GetHeader(resp, "content-length"), "6");
    EXPECT_EQ(GetBody(resp), "<body>");
}

TEST(CgiHandlerTest, Nph204DropsBody) {
    http::CgiHandler h = MakeHandler();
    const std::string cgiOut =
        "HTTP/1.1 204 No Content\r\n"
        "X-Debug: a\r\n"
        "\r\n"
        "SHOULD_BE_DROPPED";

    http::Response resp = RunWithCgiOut(h, cgiOut);

    EXPECT_EQ(resp.getStatusCode(), http::kStatusNoContent);
    EXPECT_EQ(GetHeader(resp, "content-length"), "0");
    EXPECT_TRUE(GetBody(resp).empty());
}

TEST(CgiHandlerTest, NphNonNumericCodeTreatedAs200) {
    http::CgiHandler h = MakeHandler();
    const std::string cgiOut =
        "HTTP/1.1 OK something\r\n"
        "Content-Type: text/plain\r\n"
        "\r\n"
        "ok";

    http::Response resp = RunWithCgiOut(h, cgiOut);

    EXPECT_EQ(resp.getStatusCode(), http::kStatusOk);
    EXPECT_EQ(GetHeader(resp, "content-length"), "2");
    EXPECT_EQ(GetBody(resp), "ok");
}

TEST(CgiHandlerTest, MultiSetCookieAreAllKept) {
    http::CgiHandler h = MakeHandler();
    const std::string cgiOut =
        "Content-Type: text/plain\r\n"
        "Set-Cookie: a=1\r\n"
        "Set-Cookie: b=2; Path=/\r\n"
        "\r\n"
        "ok";

    http::Response resp = RunWithCgiOut(h, cgiOut);

    // このテストではサイズだけ最低限確認（詳細はヘッダ列挙APIがあればそちらで）
    EXPECT_EQ(GetHeader(resp, "content-length"), "2");
    EXPECT_EQ(GetBody(resp), "ok");
}

TEST(CgiHandlerTest, TransferEncodingIsStripped) {
    http::CgiHandler h = MakeHandler();
    const std::string cgiOut =
        "Content-Type: text/plain\r\n"
        "Transfer-Encoding: chunked\r\n"
        "\r\n"
        "abc";

    http::Response resp = RunWithCgiOut(h, cgiOut);

    // Transfer-Encoding は出力レスポンスから除去される想定
    EXPECT_EQ(GetHeader(resp, "transfer-encoding"), "");
    EXPECT_EQ(GetHeader(resp, "content-length"), "3");
    EXPECT_EQ(GetBody(resp), "abc");
}

// struct TmpRoot {
//   fs::path path;
//   bool owned = false;
//   ~TmpRoot(){ if (owned) { std::error_code ec; fs::remove_all(path, ec); } }
// };

// static TmpRoot getTmpRoot() {
//   TmpRoot tr;
//   if (fs::exists("tmp") && fs::is_directory("tmp")) {
//     tr.path  = fs::absolute("tmp");      // 既存の repo 配下 tmp を使用
//     tr.owned = false;                    // 消さない
//   } else {
//     fs::path p = fs::temp_directory_path() /
//                  ("ws_cgi_test_" + std::to_string(getpid()));
//     fs::create_directories(p);
//     tr.path  = p;                        // /tmp 以下に作成
//     tr.owned = true;                     // テスト終了時に掃除
//   }
//   return tr;
// }

// TEST(CgiHandlerEpollTest, SimpleGet_NoHeaders) {
//   signal(SIGPIPE, SIG_IGN);

//   TmpRoot guard = getTmpRoot();
//   const std::string dir = guard.path.string();

//   // hello.cgi を作る（改行なし）
//   fs::path script = guard.path / "hello.cgi";
//   { std::ofstream sh(script); ASSERT_TRUE(sh.is_open());
//     sh << "#!/usr/bin/env sh\n" << "printf 'hello'"; }
//   fs::permissions(script,
//     fs::perms::owner_exec|fs::perms::owner_read|fs::perms::owner_write|
//     fs::perms::group_exec|fs::perms::group_read|
//     fs::perms::others_exec, fs::perm_options::add);

//   // conf を /tmp 側に保存（どこでも良い）
//   fs::path confFile = fs::temp_directory_path() / "ws_conf_test.conf";
//   { std::ofstream ofs(confFile); ASSERT_TRUE(ofs.is_open());
//     ofs << "server {\n"
//         << "  listen 8080;\n"
//         << "  host localhost;\n"
//         << "  location / {\n"
//         << "    allow_method GET;\n"
//         << "    root " << dir << ";\n"
//         << "    index index.html;\n"
//         << "  }\n"
//         << "}\n"; }

//   Config config(confFile.string());
//   const std::vector<ServerContext>& servers = config.getParser().getServer();
//   ASSERT_FALSE(servers.empty());
//   const std::vector<LocationContext>& locs = servers[0].getLocation();
//   ASSERT_FALSE(locs.empty());
//   const DocumentRootConfig& doc = locs[0].getDocumentRootConfig();
//   ASSERT_EQ(doc.getRoot(), dir);

//   http::CgiHandler h(doc);

//   RawHeaders hdr; std::vector<char> body;
//   http::Request req(http::kMethodGet, "/hello.cgi", "/hello.cgi", "",
//                     hdr, body, /*server*/ NULL, /*location*/ &locs[0]);

//   http::Response resp = h.serveInternal(req);
//   EXPECT_EQ(resp.getStatusCode(), http::kStatusOk);
//   EXPECT_EQ(GetHeader(resp, "content-type"), "text/plain");
//   EXPECT_EQ(GetHeader(resp, "content-length"), "5");
//   EXPECT_EQ(GetBody(resp), "hello");

//   std::remove(confFile.string().c_str()); // conf の掃除（repoの tmp は残す）
// }
