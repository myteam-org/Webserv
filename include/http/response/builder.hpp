#pragma once

#include "response.hpp"
#include "response_headers.hpp"
#include "http/status.hpp"
#include "utils/types/option.hpp"
#include <sstream>

namespace http {
    class ResponseBuilder {
    public:
        ResponseBuilder();
        ~ResponseBuilder();

        Response build();

        ResponseBuilder &status(HttpStatusCode status);
        ResponseBuilder &header(const std::string &name, const std::string &value);

        ResponseBuilder &text(const std::string &body, HttpStatusCode status = kStatusOk);
        ResponseBuilder &html(const std::string &body, HttpStatusCode status = kStatusOk);
        ResponseBuilder &redirect(const std::string &location, HttpStatusCode status = kStatusFound);
        ResponseBuilder &file(const std::string &path, HttpStatusCode status = kStatusOk);
        ResponseBuilder &body(const std::string &body, HttpStatusCode status);

    private:
        HttpStatusCode status_;
        std::string httpVersion_;
        Headers headers_;
        types::Option<std::string> body_;

    };
} //namespace http
