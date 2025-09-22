#include "http/response/response_headers.hpp"
#include "utils/string.hpp"

namespace http {

    Headers::Headers() {}
    Headers::Headers(const ResponseHeaderFields& init) : member_(init) {}
    Headers::~Headers() {}


    void Headers::setField(const std::string& name, const std::string& value) {
        const std::string key = utils::toLower(name);
        member_[key] = value;
    }

    bool Headers::hasField(const std::string& name) const {
        const std::string key = utils::toLower(name);
        return member_.find(key) != member_.end();
    }

    const std::string& Headers::getField(const std::string& name) const {
        static const std::string kEmpty;
        const std::string key = utils::toLower(name);
        const ResponseHeaderFields::const_iterator it = member_.find(key);
        if (it == member_.end()) {
            return kEmpty;
        }
        return it->second;
    }

    const ResponseHeaderFields& Headers::getMembers() const {
        return member_;
    }

    bool Headers::removeField(const std::string& name) {
        const std::string key = utils::toLower(name);
        if (member_.find(key) == member_.end()) {
            return false;
        }
        member_.erase(key);
        return true;
    }

} // namespace http