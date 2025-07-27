#include "mime.hpp"
#include <algorithm>
#include <map>
#include <string>
#include <cstddef>

namespace http {

namespace {

const std::pair<const char*, const char*> mimeTypePairs[] = {
    std::make_pair("html", "text/html"),
    std::make_pair("htm", "text/html"),
    std::make_pair("css", "text/css"),
    std::make_pair("js", "application/javascript"),
    std::make_pair("json", "application/json"),
    std::make_pair("png", "image/png"),
    std::make_pair("jpg", "image/jpeg"),
    std::make_pair("jpeg", "image/jpeg"),
    std::make_pair("txt", "text/plain")
};

std::map<std::string, std::string> createMimeTypeMap() {
    std::map<std::string, std::string> mimeTypes;
    const std::size_t mimeTypeCount = sizeof(mimeTypePairs) / sizeof(mimeTypePairs[0]);
    for (std::size_t index = 0; index < mimeTypeCount; ++index) {
        mimeTypes[mimeTypePairs[index].first] = mimeTypePairs[index].second;
    }
    return mimeTypes;
}

} // namespace

std::string getMimeType(const std::string &fileName) {
    const static std::map<std::string, std::string> mimeTypes = createMimeTypeMap();

    const std::size_t lastDotPosition = fileName.find_last_of('.');
    if (lastDotPosition == std::string::npos) {
        return "application/octet-stream";
    }

    std::string fileExtension = fileName.substr(lastDotPosition + 1);
    std::transform(fileExtension.begin(), fileExtension.end(), fileExtension.begin(), ::tolower);

    const std::map<std::string, std::string>::const_iterator found = mimeTypes.find(fileExtension);
    if (found != mimeTypes.end()) {
        return found->second;
    }

    return "application/octet-stream";
}

} // namespace http
