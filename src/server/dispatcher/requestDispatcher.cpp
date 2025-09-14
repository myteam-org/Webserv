#include "server/dispatcher/RequestDispatcher.hpp"

// ===== あなた側の Connection に依存する最小アダプタ =====
// 1) 直列化した文字列をどこへ積むか：WriteBuffer 相当の API 名に合わせて差し替えてください。
namespace {
    void pushRawToWriteBuffer(Connection& c, const std::string& bytes) {
        // 例: c.getWriteBuffer().append(bytes);
        // なければ一時的に:
        // c.debugAppendWrite(bytes);
    }

    // 2) Request 情報の最小取得：実プロジェクトの API に合わせて差し替え
    std::string getMethod(const Connection& c);          // "GET","HEAD","POST","DELETE"
    std::string getRequestTarget(const Connection& c);   // "/index.html" 等
    std::string getHttpVersion(const Connection& c);     // "HTTP/1.1"
    bool        headerConnectionClose(const Connection& c); // Connection: close 判定
}

// ===== RequestDispatcher 実装 =====
DispatchResult RequestDispatcher::step(Connection& c) {
    if (!c.hasPending()) {
        return DispatchResult::kNone;
    }
    http::Request& req = c.front(); // pending_ の先頭を参照
    return dispatchNext(c, req);
}

void RequestDispatcher::ensureVhost(Connection& /*c*/) {
    // いまはモック：何もしない
}

DispatchResult RequestDispatcher::dispatchNext(Connection& c, http::Request& req) {
    VirtualServer vserver = chooseVServer(req.getServer());
    Either<IAction*, http::Response> responseRes = vserver.getRouter().serve(req);
    if (responseRes.isRight()) {
        enqueue(responseRes.unwrapRight());        // WriteBufferへ
        return DispatchResult::ArmOut();
    }
}

DispatchResult RequestDispatcher::handleGet(Connection& c, const http::Request& req) {
    const std::string fs = resolveFilesystemPath(req);

    http::ResponseBuilder rb;
    rb.file(fs, http::kStatusOk); // your builder: 404/CT/CL を面倒見てくれる実装
    if (shouldClose(req)) rb.header("Connection", "close");
    enqueueResponse(c, rb.build());
    return DispatchResult::kArmOut;
}

// DispatchResult RequestDispatcher::handleHead(Connection& c, const http::Request& req) {
//     const std::string fs = resolveFilesystemPath(req);
//     size_t sz = 0;
//     http::ResponseBuilder rb;

//     // HEAD: GET と同じヘッダ + ボディ無し
//     if (statFileSize(fs, sz)) {
//         rb.status(http::kStatusOk)
//           .header("Content-Type", detectMime(fs))
//           .header("Content-Length", utils::toString(sz)); // ★先にCLを入れる
//         // body は none のまま（ResponseBuilder::build が CL を維持する）
//     } else {
//         rb.status(http::kStatusNotFound)
//           .header("Content-Type", "text/plain; charset=UTF-8")
//           .header("Content-Length", "0"); // ボディ無し
//     }

//     if (shouldClose(req)) rb.header("Connection", "close");
//     enqueueResponse(c, rb.build());
//     return DispatchResult::kArmOut;
// }

DispatchResult RequestDispatcher::handlePost(Connection& c, const http::Request& req) {
    // 本番は upload_path に書き出す/CGI へ渡す。
    // まずは通電目的のモック: 201 Created
    http::ResponseBuilder rb;
    rb.status(http::kStatusCreated)
      .text("created", http::kStatusCreated);
    if (shouldClose(req)) rb.header("Connection", "close");
    enqueueResponse(c, rb.build());
    return DispatchResult::kArmOut;
}

DispatchResult RequestDispatcher::handleDelete(Connection& c, const http::Request& req) {
    // 本番は unlink()。まずはモック: 200 OK
    http::ResponseBuilder rb;
    rb.status(http::kStatusOk)
      .text("deleted", http::kStatusOk);
    if (shouldClose(req)) rb.header("Connection", "close");
    enqueueResponse(c, rb.build());
    return DispatchResult::kArmOut;
}

DispatchResult RequestDispatcher::startCgi(Connection& /*c*/, const std::string& /*scriptPath*/) {
    // ここでは本当に fork/exec しない。Server 側配線だけ先に動かすためのモック。
    CgiFds fds; fds.stdin_fd = -1; fds.stdout_fd = -1;
    return DispatchResult::StartCgi(fds);
}

DispatchResult RequestDispatcher::onCgiStdout(Connection& c) {
    // 本来は CGI 出力からヘッダを切り出して Response へ。モックでは 200 固定で返す。
    http::ResponseBuilder rb;
    rb.status(http::kStatusOk)
      .header("Content-Type", "text/plain")
      .text("cgi output", http::kStatusOk);
    enqueueResponse(c, rb.build());
    return DispatchResult::ArmOut();
}

DispatchResult RequestDispatcher::onCgiStdin(Connection& /*c*/) {
    // 本来はリクエストボディを stdin へ。モックでは何もしない
    return DispatchResult::kNone;
}

void RequestDispatcher::enqueueResponse(Connection& c, const http::Response& resp) {
    const std::string raw = const_cast<http::Response&>(resp).toString();
    pushRawToWriteBuffer(c, raw);
}

// bool RequestDispatcher::shouldKeepAlive(Connection& c) const {
//     const std::string ver = getHttpVersion(c);
//     if (ver == "HTTP/1.1") return !headerConnectionClose(c);
//     // HTTP/1.0 はデフォルト close。Keep-Alive ヘッダがあるなら維持（モックでは非対応）
//     return false;
// }

// std::string RequestDispatcher::resolveFilesystemPath(const http::Request& req) const {
//     const DocumentRootConfig* docRoot = req.getDocumentRoot();
//     std::string base = docRoot ? docRoot->getRoot() : "./www";
//     std::string path = req.getPath(); // 例: "/index.html" or "/docs/"

//     // ディレクトリなら index.html を補う
//     if (!path.empty() && path[path.size()-1] == '/') {
//         const std::string indexFile = docRoot ? docRoot->getIndex() : "index.html";
//         path += indexFile;
//     }
//     // ディレクトリトラバーサル防止: "../" を潰す
//     path = utils::normalizePath(path);
//     return base + path;
// }


// std::string RequestDispatcher::resolveTargetPath(Connection& c) const {
//     // 通常は vhost + location でルート解決
//     const std::string uri = getRequestTarget(c);
//     if (uri == "/" || uri.empty()) return "./www/index.html";
//     return std::string("./www") + uri; // モック
// }

bool RequestDispatcher::wantsCgi(Connection& c) const {
    const std::string uri = getRequestTarget(c);
    // 超簡易：.cgi とか .py なら CGI とみなす
    const std::string::size_type dot = uri.rfind('.');
    if (dot == std::string::npos) return false;
    const std::string ext = uri.substr(dot);
    return (ext == ".cgi" || ext == ".py" || ext == ".php");
}

bool RequestDispatcher::isHead(Connection& c) const {
    return getMethod(c) == "HEAD";
}
