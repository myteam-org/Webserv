#include <sstream>
#include <map>
#include <vector>
#include <string>

#include "cgi.hpp"
#include "http/response/builder.hpp"
#include "string.hpp"

namespace http {

namespace {

const int kNphLength = 8;
const int kOk = 200;

bool splitHeadersBody(const std::string& cgiOut, std::string* headers,
                      std::string* body);
bool parseNphStatus(const std::string& headers, int* codeOut);
typedef std::map<std::string, std::vector<std::string> > HeaderMap;
void parseHeaderLines(const std::string& headers, HeaderMap* out);
http::HttpStatusCode decideStatus(const HeaderMap& headerLines, int nphCode,
                                  bool* hasLocation);
void normalizeHeaders(bool hasLoc, bool willBody, HeaderMap* headerLines);
Response buildResponse(http::HttpStatusCode status,
                       const HeaderMap& headerLines, const std::string& body);
}  // namespace

Response CgiHandler::parseCgiAndBuildResponse(const std::string& cgiOut) const {
    std::string headers;
    std::string body;
    if (!splitHeadersBody(cgiOut, &headers, &body)) {
        return ResponseBuilder()
            .header("Content-Type", "text/plain")
            .header("Content-Length", utils::toString(cgiOut.size()))
            .body(cgiOut, kStatusOk)
            .build();
    }
    int nphCode = 0;
    (void)parseNphStatus(headers, &nphCode);

    HeaderMap headerLines;
    parseHeaderLines(headers, &headerLines);

    bool hasLoc = false;
    const http::HttpStatusCode status = decideStatus(headerLines, nphCode, &hasLoc);

    const bool willBody =
        (status != kStatusNoContent && status != kStatusNotModified);
    if (!willBody) {
        body.clear();
    }
    normalizeHeaders(hasLoc, willBody, &headerLines);
    return buildResponse(status, headerLines, body);
}

namespace {
bool splitHeadersBody(const std::string& cgiOut, std::string* headers,
                      std::string* body) {
    if (!headers || !body) {
        return false;
    }
    const std::string::size_type posCRLFCRLF = cgiOut.find("\r\n\r\n");
    if (posCRLFCRLF != std::string::npos) {
        *headers = cgiOut.substr(0, posCRLFCRLF);
        *body = cgiOut.substr(posCRLFCRLF + 4);
        return true;
    }
    const std::string::size_type posLFLF = cgiOut.find("\n\n");
    if (posLFLF != std::string::npos) {
        *headers = cgiOut.substr(0, posLFLF);
        *body = cgiOut.substr(posLFLF + 2);
        return true;
    }
    return false;
}

// NPH: 先頭行が "HTTP/1.x <code> ..."
bool parseNphStatus(const std::string& headers, int* codeOut) {
    if (!codeOut) {
        return false;
    }
    std::istringstream iss(headers);
    std::string line;
    if (!std::getline(iss, line)) {
        return false;
    }
    if (!line.empty() && line[line.size() - 1] == '\r') {
        line.erase(line.size() - 1);
    }
    // 最低でも "HTTP/1.x" (8文字) は必要。数値が無ければ 200 として続行
    if (line.size() < kNphLength) {
        return false;
    }
    if (line.compare(0, 7, "HTTP/1.") != 0) {
        return false;
    }
    std::istringstream lss(line);
    std::string httpver;
    int code = 200;
    if (!(lss >> httpver >> code)) {
        // 数字が読めないなら 200 として進める
        *codeOut = kOk;
        return true;
    }
    *codeOut = code;
    return true;
}

// ヘッダ行パース（多重値対応）
void parseHeaderLines(const std::string& headers, HeaderMap* out) {
    if (!out) {
        return;
    }
    out->clear();
    std::istringstream iss(headers);
    std::string line;
    while (std::getline(iss, line)) {
        if (!line.empty() &&
            line[line.size() - 1] == '\r') {  // \nは取れているので
            line.erase(line.size() - 1);
        }
        const std::string::size_type chr = line.find(':');
        if (chr == std::string::npos) {
            continue;
        }
        std::string key = line.substr(0, chr);
        std::string value = line.substr(chr + 1);
        key = utils::toLower(utils::trim(key));
        value = utils::trim(value);
        if (value.find('\n') != std::string::npos) {
            continue;
        }
        if (value.find('\r') != std::string::npos) {
            continue;
        }
        (*out)[key].push_back(value);
    }
}

// ステータス決定（Status/Location/NPH）
http::HttpStatusCode decideStatus(const HeaderMap& headerLines, int nphCode,
                                  bool* hasLocation) {
    if (hasLocation) {
        *hasLocation = headerLines.find("location") != headerLines.end();
    }
    int code = kOk;
    const HeaderMap::const_iterator it = headerLines.find("status");
    if (it != headerLines.end() && !it->second.empty()) {
        std::istringstream iss(it->second.front());
        int tmp = 0;
        if (iss >> tmp) {
            code = tmp;
        }
    } else if (nphCode > 0) {
        code = nphCode;
    } else if (hasLocation && *hasLocation) {
        code = static_cast<int>(kStatusFound);
    }
    return static_cast<http::HttpStatusCode>(code);
}

// ヘッダ正規化（危険/不要削除と補完）
void normalizeHeaders(bool hasLoc, bool willBody, HeaderMap* headerLines) {
    if (!headerLines) {
        return;
    }
    // Responseに不要な行を削除
    headerLines->erase("transfer-encoding");
    headerLines->erase("status");
    // Content-Typeの補完（本文があり、かつリダイレクトでない）
    if (willBody && !hasLoc) {
        if (headerLines->find("content-type") == headerLines->end()) {
            (*headerLines)["content-type"].push_back("text/plain");
        }
    }
    // Content-Lengthはあとで
    headerLines->erase("content-length");
}

Response buildResponse(http::HttpStatusCode status,
                       const HeaderMap& headerLines, const std::string& body) {
    ResponseBuilder rBuilder = ResponseBuilder();
    for (HeaderMap::const_iterator it = headerLines.begin(); it != headerLines.end(); ++it) {
        const std::string& key = it->first;
        for (std::size_t i = 0; i < it->second.size(); ++i) {
            rBuilder = rBuilder.header(key, it->second[i]);
        }
    }
    rBuilder = rBuilder.header("Content-Length", utils::toString(body.size()));
    rBuilder = rBuilder.header("Connection", "close");
    return rBuilder.body(body, status).build();
}

}  // namespace

}  // namespace http
