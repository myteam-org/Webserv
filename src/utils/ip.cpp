#include "utils/ip.hpp"

namespace {
    inline bool isDigit(const char c) {
        return c >= '0' && c <= '9';
    }
}

namespace utils {

// "正規10進表記"の IPv4 を num[4] に格納して true
// 先頭ゼロは不可（"0"単独のみ可）、各オクテットは 0..255
bool isRegularNumber(const std::string& ipAddress, int num[4]) {
    const char* c = ipAddress.c_str();

    for (int part = 0; part < 4; ++part) {
        if (*c == '\0' || !isDigit(*c)) {
            return false;
        }
        // 先頭ゼロの扱い
        if (*c == '0') {
            num[part] = 0;
            ++c;
            if (isDigit(*c)) {
                return false;
            }
        } else {
            int val = 0;
            while (isDigit(*c)) {
                val = val * 10 + (*c - '0');
                ++c;
                if (val > IP_MAX) {
                    return false;
                }
            }
            if (val == 0) {
                return false;
            }
            num[part] = val;
        }
        if (part < 3) {
            if (*c != '.') {
                return false;
            }
            ++c;
        }
    }
    return *c == '\0';
}

bool isCanonicalDecimalIPv4(const std::string& ipAddress) {
    int num[4];

    if (!isRegularNumber(ipAddress, num)) {
        return false;
    }
    if (num[0] != BITS7) {
        return false;
    }
    // 127.0.0.0 を除外
    if (num[1] == 0 && num[2] == 0 && num[3] == 0) {
        return false;
    }
    // 127.255.255.255 を除外
    if (num[1] == IP_MAX && num[2] == IP_MAX && num[3] == IP_MAX) {
        return false;
    }
    return true;
}

} // namespace utils