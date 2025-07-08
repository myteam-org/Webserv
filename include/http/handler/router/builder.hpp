#pragma once

namespace http {
    class RouterBuilder {
    public:
        RouterBuilder();
        ~RouterBuilder();
        
        RouterBuilder& route(HttpMethod method, const std::string& path, IHandler* handler);
        RouterBuilder& route(const std::vector<HttpMethod>& methods, const std::string& path, IHandler* handler);
        RouterBuilder& middleware(IMiddleware* middleware);
        
        Router* build();
        
    private:
        Router* router_;
        bool built_;
    };
} //namespace http
