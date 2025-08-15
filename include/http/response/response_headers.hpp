#pragma once

#include "response_header_types.hpp"

namespace http {

    class Headers {
       public:
        Headers();
        explicit Headers(const ResponseHeaderFields& init);
        ~Headers();

        void setField(const std::string& name, const std::string& value);
        bool hasField(const std::string& name) const;
        const std::string& getField(const std::string& name) const;
        const ResponseHeaderFields& getMembers() const;
        bool removeField(const std::string& name);

       private:
        ResponseHeaderFields member_;
    };

} // namespace http
