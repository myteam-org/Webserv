#pragma once

#include "option.hpp"
#include "method.hpp"
#include <map>
#include <vector>
#include <string>


namespace http {
	typedef std::vector<std::string> RawHeaders;
    class Request {
    public:
        explicit Request(
            HttpMethod method,
            const std::string &requestTarget,
            const std::string &httpVersion = "HTTP/1.1",
            // const Headers &headers = Headers(),
            const std::string &body = ""
        );

        bool operator==(const Request &other) const;

        HttpMethod getMethod() const;
        const std::string &getRequestTarget() const;
        const std::string &getHttpVersion() const;
		types::Option<std::string> getHeader(const std::string &key) const;
        const std::string &getBody() const;

    private:
        HttpMethod method_;
        std::string requestTarget_;
        std::string httpVersion_;
        // Headers headers_;
        std::string body_;
    };
} // namespace http
