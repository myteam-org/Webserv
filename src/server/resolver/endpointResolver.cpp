#include "server/resolver/EndpointResolver.hpp"

EndpointResolver::EndpointResolver(const std::map<std::string, VirtualServer*>& vsByKey)
      : vsByKey_(vsByKey) {}

VirtualServer* EndpointResolver::resolveByFd(int cfd) const {
    // 1) 厳密一致キーを作る（cfd のローカル側）
    std::string exact = endpointKeyFromFd(cfd); // "192.168.0.5:8080" など
    std::map<std::string, VirtualServer*>::const_iterator it = vsByKey_.find(exact);
    if (it != vsByKey_.end()) {
        return it->second;
    }
    // 2) ANY(0.0.0.0:port) へフォールバック
    const std::string::size_type c = exact.find(':');
    if (c != std::string::npos) {
        const std::string anyKey = std::string("0.0.0.0:") + exact.substr(c + 1);
        it = vsByKey_.find(anyKey);
        if (it != vsByKey_.end()) return it->second;
    }
    return 0; // 未登録: 設定ミス扱い or グローバルデフォルト
}

// connection fd から、
std::string EndpointResolver::endpointKeyFromFd(int fd) {
    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    std::memset(&sin, 0, sizeof(sin));
    if (::getsockname(fd, reinterpret_cast<struct sockaddr*>(&sin), &len) != 0) {
        return "0.0.0.0:0"; // フェイルセーフ
    }
    char ip[INET_ADDRSTRLEN]; // C++98
    if (!::inet_ntop(AF_INET, &sin.sin_addr, ip, sizeof(ip))) {
        return "0.0.0.0:0";
    }
    const unsigned short port = ntohs(sin.sin_port);
    std::ostringstream oss; oss << ip << ":" << port;
    return oss.str();
}
