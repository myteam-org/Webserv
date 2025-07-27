#include "mime.hpp"
#include <algorithm>
#include <map>

namespace http {
    namespace {
        std::map<std::string, std::string> createMimeTypeMap() {
            std::map<std::string, std::string> mimeTypes;
            mimeTypes["html"] = "text/html";
            mimeTypes["htm"] = "text/html";
            mimeTypes["css"] = "text/css";
            mimeTypes["js"] = "application/javascript";
            mimeTypes["json"] = "application/json";
            mimeTypes["png"] = "image/png";
            mimeTypes["jpg"] = "image/jpeg";
            mimeTypes["jpeg"] = "image/jpeg";
            mimeTypes["txt"] = "text/plain";
            return mimeTypes;
        }
    } // namespace

    std::string getMimeType(const std::string &fileName) {
        const static std::map<std::string, std::string> mimeTypes = createMimeTypeMap();

        const std::size_t lastDot = fileName.find_last_of('.');
        if (lastDot == std::string::npos) {
            return "application/octet-stream";
        }

        std::string extension = fileName.substr(lastDot + 1);
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

        const std::map<std::string, std::string>::const_iterator it = mimeTypes.find(extension);
        if (it != mimeTypes.end()) {
            return it->second;
        }

        return "application/octet-stream";
    }
} //namespace http
