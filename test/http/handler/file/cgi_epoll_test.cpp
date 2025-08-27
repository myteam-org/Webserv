// // cgi_epoll_test.cpp
// // テスト専用：CGI の最小ケース（ヘッダ無しで "hello" を返す）
// // - 既存の repo 配下に tmp/ があればそこを使い、無ければ /tmp に一時生成して掃除します
// // - CgiHandler::serve(...) を利用（serveInternal は使いません）

// #include <gtest/gtest.h>
// #include <signal.h>
// #include <fstream>
// #include <sstream>
// #include <string>
// #include <vector>
// #include <cstdio>
// #include <cerrno>
// #include <sys/stat.h>
// #include <unistd.h>

// #if __has_include(<filesystem>)
//   #include <filesystem>
//   namespace fs = std::filesystem;
//   #define WS_HAVE_FS 1
// #else
//   #define WS_HAVE_FS 0
// #endif

// // ===== あなたのプロジェクトのヘッダ =====
// #include "config/config.hpp"                         // Config
// #include "config/context/serverContext.hpp"          // ServerContext
// #include "config/context/locationContext.hpp"        // LocationContext
// #include "config/context/documentRootConfig.hpp"     // DocumentRootConfig
// #include "http/handler/file/cgi.hpp"                 // http::CgiHandler
// #include "http/request/request.hpp"                  // http::Request
// #include "http/response/response.hpp"                // http::Response
// #include "http/response/builder.hpp"                 // ResponseBuilder
// // Either / IAction / RawHeaders が別ヘッダなら追加インクルードしてください。
// // #include "utils/either.hpp"
// // #include "http/request/raw_headers.hpp"

// namespace { // ====== 共通ユーティリティ（テストからのみ使用） ======

// // レスポンス本文（Option を unwrap。None の場合は空文字）
// static std::string GetBody(const http::Response& resp) {
//     const types::Option<std::string>& opt = resp.getBody();
//     return opt.isSome() ? opt.unwrap() : std::string();
// }

// // レスポンスヘッダを軽量に取得（nameLower は小文字で渡す）
// static std::string GetHeader(const http::Response& resp, const std::string& nameLower) {
//     std::string text = const_cast<http::Response&>(resp).toString();

//     std::string::size_type headerEnd = text.find("\r\n\r\n");
//     if (headerEnd == std::string::npos) headerEnd = text.find("\n\n");
//     if (headerEnd == std::string::npos) return std::string();

//     std::string::size_type cur = 0;
//     while (cur < headerEnd) {
//         std::string::size_type eol = text.find('\n', cur);
//         if (eol == std::string::npos || eol > headerEnd) eol = headerEnd;

//         std::string line = text.substr(cur, eol - cur);
//         if (!line.empty() && line[line.size() - 1] == '\r') {
//             line.erase(line.size() - 1);
//         }

//         std::string::size_type colon = line.find(':');
//         if (colon != std::string::npos) {
//             std::string key = line.substr(0, colon);
//             std::string val = line.substr(colon + 1);

//             while (!val.empty() && (val[0] == ' ' || val[0] == '\t')) {
//                 val.erase(0, 1);
//             }
//             while (!val.empty() && (val[val.size() - 1] == ' ' || val[val.size() - 1] == '\t')) {
//                 val.erase(val.size() - 1);
//             }
//             for (std::size_t i = 0; i < key.size(); ++i) {
//                 char ch = key[i];
//                 if ('A' <= ch && ch <= 'Z') key[i] = static_cast<char>(ch - 'A' + 'a');
//             }
//             if (key == nameLower) return val;
//         }
//         if (eol == headerEnd) break;
//         cur = eol + 1;
//     }
//     return std::string();
// }

// struct TmpRoot {
// #if WS_HAVE_FS
//     fs::path path;
// #else
//     std::string path;
// #endif
//     bool owned = false;
//     ~TmpRoot() {
//         if (!owned) return;
// #if WS_HAVE_FS
//         std::error_code ec;
//         fs::remove_all(path, ec);
// #else
//         // POSIX fallback:
//         // 再帰削除は安全配慮で省略（このテストでは conf と hello.cgi のみ作成する前提）
//         // 必要なら独自の再帰削除を実装してください。
// #endif
//     }
// };

// // tmp/ があればそれを使う。無ければ /tmp 下に新規作成してテスト後に掃除。
// static TmpRoot GetTmpRoot() {
//     TmpRoot root;
// #if WS_HAVE_FS
//     if (fs::exists("tmp") && fs::is_directory("tmp")) {
//         root.path  = fs::absolute("tmp");
//         root.owned = false; // 既存は消さない
//     } else {
//         fs::path created = fs::temp_directory_path() /
//                            ("ws_cgi_test_" + std::to_string(getpid()));
//         fs::create_directories(created);
//         root.path  = created;
//         root.owned = true; // 後始末する
//     }
// #else
//     char buf[256];
//     snprintf(buf, sizeof(buf), "/tmp/ws_cgi_test_%d", static_cast<int>(getpid()));
//     mkdir(buf, 0700); // 既に存在していてもOK
//     root.path  = buf;
//     root.owned = true;
// #endif
//     return root;
// }

// static bool WriteTextFile(const std::string& filePath, const std::string& text) {
//     std::ofstream ofs(filePath.c_str());
//     if (!ofs.is_open()) return false;
//     ofs << text;
//     return true;
// }

// static bool MakeExecutable(const std::string& filePath) {
// #if WS_HAVE_FS
//     fs::permissions(filePath,
//         fs::perms::owner_exec|fs::perms::owner_read|fs::perms::owner_write|
//         fs::perms::group_exec|fs::perms::group_read|
//         fs::perms::others_exec,
//         fs::perm_options::add);
//     return true;
// #else
//     struct stat st;
//     if (stat(filePath.c_str(), &st) != 0) return false;
//     mode_t mode = st.st_mode | S_IXUSR | S_IRUSR | S_IWUSR | S_IXGRP | S_IXOTH;
//     return chmod(filePath.c_str(), mode) == 0;
// #endif
// }

// // Either<IAction*, Response> を最終的な Response まで解決するヘルパ。
// // 非同期アクション(IAction)を1ステップずつ実行して Right<Response> になるまで回します。
// static http::Response
// RunToResponse(Either<IAction*, http::Response> result) {
//     // 予防：無限ループ防止（十分大きめ）
//     int step_guard = 1024;

//     while (step_guard-- > 0) {
//         // あなたの Either API に合わせて修正
//         // 例1: isRight()/unwrapRight()/unwrapLeft()
//         if (result.isRight()) {
//             return result.unwrapRight();
//         }
//         IAction* act = result.unwrapLeft();

//         // ---- ここだけあなたの IAction API 名に合わせてください ----
//         // 例A: IAction::run() が Either<IAction*, Response> を返す
//         result = act->run();
//         // 例B: IAction::step() や IAction::execute() などなら、上の1行を置き換える
//         // result = act->step();
//         // result = act->execute();
//         // --------------------------------------------------------

//         // 注意：act の所有権（delete が必要か）はあなたの設計に従ってください。
//         // フレームワーク側が管理するなら delete しないこと。
//         // delete act; // 必要なら有効化
//     }

//     // ここに到達したら未収束
//     ADD_FAILURE() << "Action chain did not resolve to Response (likely needs API name tweak).";
//     const std::string msg = "Test failed: IAction chain did not reach Response";
//     return http::ResponseBuilder()
//         .header("Content-Type", "text/plain")
//         .body(msg, http::kStatusInternalServerError)
//         .build();
// }


// } // namespace（テスト用ユーティリティ終わり）

// // ===================== テスト本体 =====================

// TEST(CgiHandlerEpollTest, SimpleGet_NoHeaders) {
//     // write(2) での EPIPE に備えて SIGPIPE を無視
//     signal(SIGPIPE, SIG_IGN);

//     TmpRoot guard = GetTmpRoot();

// #if WS_HAVE_FS
//     const std::string dirPath = guard.path.string();
//     const std::string cgiPath = (guard.path / "hello.cgi").string();
// #else
//     const std::string dirPath = guard.path;
//     const std::string cgiPath = dirPath + "/hello.cgi";
// #endif

//     // 1) テスト用 CGI を作成（ヘッダ無しの "hello" を出力）
//     ASSERT_TRUE(WriteTextFile(cgiPath,
//         "#!/usr/bin/env sh\n"
//         "printf 'hello'"));
//     ASSERT_TRUE(MakeExecutable(cgiPath));

//     // 2) 一時 conf を作成（あなたのパーサ仕様に合わせて調整可）
// #if WS_HAVE_FS
//     const std::string confPath = (fs::temp_directory_path() / "ws_conf_test.conf").string();
// #else
//     const std::string confPath = "/tmp/ws_conf_test.conf";
// #endif

//     {
//         std::ostringstream oss;
//         oss << "server {\n"
//             << "  listen 8080;\n"
//             << "  host localhost;\n"
//             << "  location / {\n"
//             << "    allow_method GET;\n"
//             << "    root " << dirPath << ";\n"
//             << "    index index.html;\n"
//             << "  }\n"
//             << "}\n";
//         ASSERT_TRUE(WriteTextFile(confPath, oss.str()));
//     }

//     // 3) 設定読み込み → ドキュメントルート取得
//     Config config(confPath);
//     const std::vector<ServerContext>& servers = config.getParser().getServer();
//     ASSERT_FALSE(servers.empty()) << "no server blocks parsed";

//     const std::vector<LocationContext>& locations = servers[0].getLocation();
//     ASSERT_FALSE(locations.empty()) << "no location blocks parsed";

//     const DocumentRootConfig& doc = locations[0].getDocumentRootConfig();
//     ASSERT_EQ(doc.getRoot(), dirPath);

//     // 4) ハンドラとリクエストの用意
//     http::CgiHandler handler(doc);

//     RawHeaders headers;           // 実装に合わせて適宜初期化
//     std::vector<char> body;       // 今回は空
//     http::Request req(http::kMethodGet,
//                       "/hello.cgi",     // request target（path）
//                       "/hello.cgi",     // normalized path が別なら合わせて
//                       "",               // query などがあれば指定
//                       headers,
//                       body,
//                       /*server*/ NULL,
//                       /*location*/ &locations[0]);

//     // 5) 実行：公開 API の serve(...) を呼ぶ
//     const Either<IAction*, http::Response> result = handler.serve(req);
//     const http::Response resp = TakeRightOrFail(result); // Right 前提

//     // 6) 検証
//     EXPECT_EQ(resp.getStatusCode(), http::kStatusOk);
//     EXPECT_EQ(GetHeader(resp, "content-type"), "text/plain");
//     EXPECT_EQ(GetHeader(resp, "content-length"), "5");
//     EXPECT_EQ(GetBody(resp), "hello");

//     // 7) 一時 conf の掃除（repo の tmp は残す）
//     std::remove(confPath.c_str());
// }
