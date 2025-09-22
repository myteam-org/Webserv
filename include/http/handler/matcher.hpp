#pragma once

#include "utils/types/option.hpp"
#include "utils/string.hpp"
#include <map>
#include <string>

namespace http {
    template <typename ValueType>
    class Matcher {
    private:
        typedef std::map<std::string, ValueType> PathToValueMap;
        PathToValueMap pathToValueMap_;

    public:
        explicit Matcher(const PathToValueMap& inputMap) : pathToValueMap_(inputMap) {}
        ~Matcher() {}

        types::Option<ValueType> match(const std::string& searchKey) const {
            if (searchKey.empty()) {
                return types::none<ValueType>();
            }
            types::Option<std::string> bestMatchKey = types::none<std::string>();
            for (typename PathToValueMap::const_iterator iter = pathToValueMap_.begin(); iter != pathToValueMap_.end(); ++iter) {
                const std::string& candidateKey = iter->first;
                if (utils::startsWith(searchKey, candidateKey) &&
                    (bestMatchKey.isNone() || candidateKey.size() > bestMatchKey.unwrap().size())) {
                    bestMatchKey = types::some<std::string>(candidateKey);
                }
            }
            if (bestMatchKey.isNone()) {
                return types::none<ValueType>();
            }
            return types::some<ValueType>(pathToValueMap_.at(bestMatchKey.unwrap()));
        }
    };
} // namespace http

// パスやメソッドのマッチング処理
