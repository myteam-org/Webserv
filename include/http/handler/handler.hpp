#include "http/request/request.hpp"
#include "either.hpp"
#include "action/action.hpp"
#include "http/response/response.hpp"

namespace http {
    class IHandler {
    public:
        virtual ~IHandler() {}
        virtual Either<IAction *, Response> serve(const Request &request) = 0;
    };
} //namespace http
